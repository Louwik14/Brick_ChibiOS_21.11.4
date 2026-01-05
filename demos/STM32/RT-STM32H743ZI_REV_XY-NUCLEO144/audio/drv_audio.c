/**
 * @file drv_audio.c
 * @brief Gestion complète du pipeline audio TDM (SAI + DMA + callbacks DSP).
 */

#include "drv_audio.h"
#include "audio_codec_ada1979.h"
#include "audio_codec_pcm4104.h"
#include "mpu_config.h"
#include "chprintf.h"
#include <string.h>

/* -------------------------------------------------------------------------- */
/* Buffers ping/pong                                                          */
/* -------------------------------------------------------------------------- */
/*
 * Placés en .ram_d2 non cacheable (voir linker/MPU) pour éliminer toute
 * incohérence D-Cache avec le DMA SAI/SPI. Le linker n'est pas modifié ici,
 * seule la documentation rappelle la contrainte de section.
 */

static volatile int32_t AUDIO_DMA_BUFFER_ATTR audio_in_buffers[2][AUDIO_FRAMES_PER_BUFFER][AUDIO_NUM_INPUT_CHANNELS];
static volatile int32_t AUDIO_DMA_BUFFER_ATTR audio_out_buffers[2][AUDIO_FRAMES_PER_BUFFER][AUDIO_NUM_OUTPUT_CHANNELS];

static volatile spilink_audio_block_t AUDIO_DMA_BUFFER_ATTR spi_in_buffers;
static volatile spilink_audio_block_t AUDIO_DMA_BUFFER_ATTR spi_out_buffers;

static volatile uint8_t audio_in_ready_index = 0xFFU;
static volatile uint8_t audio_out_ready_index = 0xFFU;

/* Synchronisation stricte RX/TX : les deux DMA doivent signaler le même demi-buffer. */
#define AUDIO_SYNC_FLAG_RX    0x01U
#define AUDIO_SYNC_FLAG_TX    0x02U
static volatile uint8_t audio_sync_mask = 0U;
static volatile uint8_t audio_sync_half = 0xFFU;

/* SPI-LINK callbacks. */
static drv_spilink_pull_cb_t spilink_pull_cb = NULL;
static drv_spilink_push_cb_t spilink_push_cb = NULL;

typedef struct {
    float gain_main;
    float gain_cue;
    bool  to_main;
    bool  to_cue;
} audio_route_t;

typedef struct {
    float            master_volume;
    audio_route_t    routes[4];
} audio_control_snapshot_t;

static struct {
    mutex_t                 lock;
    audio_control_snapshot_t state;
} audio_control;

static audio_control_snapshot_t audio_control_cached;

/* -------------------------------------------------------------------------- */
/* Synchronisation des DMA                                                    */
/* -------------------------------------------------------------------------- */

static binary_semaphore_t audio_dma_sem;

static SAIDriver *audio_sai_rx = &AUDIO_SAI_RX_DRIVER;
static SAIDriver *audio_sai_tx = &AUDIO_SAI_TX_DRIVER;
static thread_t *audio_thread = NULL;
static drv_audio_state_t audio_state = DRV_AUDIO_STOPPED;
static bool audio_initialized = false;

/* Nombre d'échantillons transférés par transaction (ping + pong). */
#define AUDIO_DMA_IN_SAMPLES   (AUDIO_FRAMES_PER_BUFFER * AUDIO_NUM_INPUT_CHANNELS * 2U)
#define AUDIO_DMA_OUT_SAMPLES  (AUDIO_FRAMES_PER_BUFFER * AUDIO_NUM_OUTPUT_CHANNELS * 2U)
#define AUDIO_INT24_MAX_F      8388607.0f
/*
 * Les tableaux [2][frames][channels] sont vus par le DMA comme un buffer
 * linéaire unique : interruption Half-Transfer => index 0 (ping),
 * interruption Transfer-Complete => index 1 (pong).
 */

/* -------------------------------------------------------------------------- */
/* Configuration SAI LLD                                                     */
/* -------------------------------------------------------------------------- */

static const SAIConfig audio_sai_rx_cfg = {
    .tx_buffer = NULL,
    .rx_buffer = (void *)audio_in_buffers,
    .size = AUDIO_DMA_IN_SAMPLES,
    .end_cb = audio_sai_rx_cb,
    .gcr = 0U,
    .cr1 = (SAI_xCR1_MODE_0 | SAI_xCR1_PRTCFG_0 | SAI_xCR1_DS_4 | SAI_xCR1_DS_2 |
            (3U << 20U) | SAI_xCR1_CKSTR | SAI_xCR1_OUTDRIV | SAI_xCR1_MCKEN),
    .cr2 = SAI_xCR2_FTH_0,
    .frcr = ((256U - 1U) << SAI_xFRCR_FRL_Pos) |
            ((128U - 1U) << SAI_xFRCR_FSALL_Pos) |
            SAI_xFRCR_FSDEF | SAI_xFRCR_FSOFF,
    .slotr = (0U << SAI_xSLOTR_FBOFF_Pos) |
             SAI_xSLOTR_SLOTSZ_1 |
             ((AUDIO_NUM_INPUT_CHANNELS - 1U) << SAI_xSLOTR_NBSLOT_Pos) |
             0x00FFU,
    .dma_mode = STM32_DMA_CR_PSIZE_WORD | STM32_DMA_CR_MSIZE_WORD,
};

static const SAIConfig audio_sai_tx_cfg = {
    .tx_buffer = (const void *)audio_out_buffers,
    .rx_buffer = NULL,
    .size = AUDIO_DMA_OUT_SAMPLES,
    .end_cb = audio_sai_tx_cb,
    .gcr = 0U,
    .cr1 = (SAI_xCR1_MODE_1 | SAI_xCR1_PRTCFG_0 | SAI_xCR1_DS_4 | SAI_xCR1_DS_2 |
            SAI_xCR1_SYNCEN_0),
    .cr2 = SAI_xCR2_FTH_0,
    .frcr = ((128U - 1U) << SAI_xFRCR_FRL_Pos) |
            ((64U - 1U) << SAI_xFRCR_FSALL_Pos) |
            SAI_xFRCR_FSDEF | SAI_xFRCR_FSOFF,
    .slotr = (0U << SAI_xSLOTR_FBOFF_Pos) |
             SAI_xSLOTR_SLOTSZ_1 |
             ((AUDIO_NUM_OUTPUT_CHANNELS - 1U) << SAI_xSLOTR_NBSLOT_Pos) |
             0x000FU,
    .dma_mode = STM32_DMA_CR_PSIZE_WORD | STM32_DMA_CR_MSIZE_WORD,
};

/* -------------------------------------------------------------------------- */
/* Prototypes internes                                                        */
/* -------------------------------------------------------------------------- */

static void audio_hw_configure_sai(void);
static void audio_dma_start(void);
static void audio_dma_stop(void);
static void audio_routes_reset_defaults(void);
static void audio_control_init(void);
static void audio_control_get_snapshot(audio_control_snapshot_t *dst);
static float soft_clip(float x);
static void audio_dma_sync_mark(uint8_t half, uint8_t flag);

static void audio_sai_rx_cb(SAIDriver *saip, bool half);
static void audio_sai_tx_cb(SAIDriver *saip, bool half);

/* -------------------------------------------------------------------------- */
/* Configuration SAI LLD                                                     */
/* -------------------------------------------------------------------------- */

static const SAIConfig audio_sai_rx_cfg = {
    .tx_buffer = NULL,
    .rx_buffer = (void *)audio_in_buffers,
    .size = AUDIO_DMA_IN_SAMPLES,
    .end_cb = audio_sai_rx_cb,
    .gcr = 0U,
    .cr1 = (SAI_xCR1_MODE_0 | SAI_xCR1_PRTCFG_0 | SAI_xCR1_DS_2 |
            (3U << 20U) | SAI_xCR1_CKSTR | SAI_xCR1_OUTDRIV | SAI_xCR1_MCKEN),
    .cr2 = SAI_xCR2_FTH_0,
    .frcr = ((256U - 1U) << SAI_xFRCR_FRL_Pos) |
            ((128U - 1U) << SAI_xFRCR_FSALL_Pos) |
            SAI_xFRCR_FSDEF | SAI_xFRCR_FSOFF,
    .slotr = (0U << SAI_xSLOTR_FBOFF_Pos) |
             SAI_xSLOTR_SLOTSZ_1 |
             ((AUDIO_NUM_INPUT_CHANNELS - 1U) << SAI_xSLOTR_NBSLOT_Pos) |
             0x00FFU,
    .dma_mode = STM32_DMA_CR_PSIZE_WORD | STM32_DMA_CR_MSIZE_WORD,
};

static const SAIConfig audio_sai_tx_cfg = {
    .tx_buffer = (const void *)audio_out_buffers,
    .rx_buffer = NULL,
    .size = AUDIO_DMA_OUT_SAMPLES,
    .end_cb = audio_sai_tx_cb,
    .gcr = 0U,
    .cr1 = (SAI_xCR1_MODE_1 | SAI_xCR1_PRTCFG_0 | SAI_xCR1_DS_2 |
            SAI_xCR1_SYNCEN_0),
    .cr2 = SAI_xCR2_FTH_0,
    .frcr = ((128U - 1U) << SAI_xFRCR_FRL_Pos) |
            ((64U - 1U) << SAI_xFRCR_FSALL_Pos) |
            SAI_xFRCR_FSDEF | SAI_xFRCR_FSOFF,
    .slotr = (0U << SAI_xSLOTR_FBOFF_Pos) |
             SAI_xSLOTR_SLOTSZ_1 |
             ((AUDIO_NUM_OUTPUT_CHANNELS - 1U) << SAI_xSLOTR_NBSLOT_Pos) |
             0x000FU,
    .dma_mode = STM32_DMA_CR_PSIZE_WORD | STM32_DMA_CR_MSIZE_WORD,
};

/* -------------------------------------------------------------------------- */
/* Callbacks DMA SAI                                                         */
/* -------------------------------------------------------------------------- */

static void audio_sai_rx_cb(SAIDriver *saip, bool half) {
    (void)saip;
    uint8_t half_index = half ? 0U : 1U;
    if (half) {
        drv_audio_dma_half_cb(half_index);
    } else {
        drv_audio_dma_full_cb(half_index);
    }
    audio_dma_sync_mark(half_index, AUDIO_SYNC_FLAG_RX);
}

static void audio_sai_tx_cb(SAIDriver *saip, bool half) {
    (void)saip;
    uint8_t half_index = half ? 0U : 1U;
    audio_dma_sync_mark(half_index, AUDIO_SYNC_FLAG_TX);
}

static THD_WORKING_AREA(audioThreadWA, AUDIO_THREAD_STACK_SIZE);
static THD_FUNCTION(audioThread, arg);

#if defined(STM32H7xx) && defined(SAI_xCR1_MODE_0)
static void audio_sai_dump_regs(BaseSequentialStream *chp,
                                const char *tag,
                                const SAI_Block_TypeDef *block);
#endif
static void audio_sai_dump_state(BaseSequentialStream *chp);

/* -------------------------------------------------------------------------- */
/* API publique                                                               */
/* -------------------------------------------------------------------------- */

void drv_audio_init(void) {
    if (audio_initialized) {
        return;
    }

    if (!mpu_config_init_once()) {
        audio_state = DRV_AUDIO_FAULT;
        return;
    }

    chBSemObjectInit(&audio_dma_sem, FALSE);

    audio_control_init();

    /* Prépare le bus I2C et les codecs. */
#if AUDIO_ALLOW_NO_CODEC
    msg_t codec_status = HAL_RET_SUCCESS;
#else
    msg_t codec_status = adau1979_init();
#endif
    if (codec_status != HAL_RET_SUCCESS) {
        audio_state = DRV_AUDIO_FAULT;
        return;
    }
    audio_codec_pcm4104_init();

    audio_in_ready_index = 0xFFU;
    audio_out_ready_index = 0xFFU;
    audio_sync_mask = 0U;
    audio_sync_half = 0xFFU;

    memset((void *)audio_in_buffers, 0, sizeof(audio_in_buffers));
    memset((void *)audio_out_buffers, 0, sizeof(audio_out_buffers));
    memset((void *)spi_in_buffers, 0, sizeof(spi_in_buffers));
    memset((void *)spi_out_buffers, 0, sizeof(spi_out_buffers));
    audio_routes_reset_defaults();

    /* Les GPIO SAI sont déjà configurés via board.h. */
    audio_hw_configure_sai();
    if (audio_state == DRV_AUDIO_FAULT) {
        return;
    }

    audio_state = DRV_AUDIO_READY;
    audio_initialized = true;
}

void drv_audio_run_sai_hal_debug(void) {
    BaseSequentialStream *chp = (BaseSequentialStream *)&SD1;
    audio_sai_dump_state(chp);
}

void drv_audio_start(void) {
    if (audio_state == DRV_AUDIO_RUNNING) {
        return;
    }

    if (!audio_initialized) {
        drv_audio_init();
    }

    if (audio_state == DRV_AUDIO_FAULT) {
        audio_codec_pcm4104_set_mute(true);
        return;
    }

    audio_codec_pcm4104_set_mute(true);

#if AUDIO_ALLOW_NO_CODEC
    msg_t codec_status = HAL_RET_SUCCESS;
#else
    msg_t codec_status = adau1979_set_default_config();
#endif
    if (codec_status != HAL_RET_SUCCESS) {
        audio_state = DRV_AUDIO_FAULT;
        return;
    }

    audio_in_ready_index = 0xFFU;
    audio_out_ready_index = 0xFFU;
    audio_sync_mask = 0U;
    audio_sync_half = 0xFFU;

    audio_dma_start();
    if (audio_state == DRV_AUDIO_FAULT) {
        audio_codec_pcm4104_set_mute(true);
        return;
    }
    if (audio_thread == NULL) {
        audio_thread = chThdCreateStatic(audioThreadWA,
                                         sizeof(audioThreadWA),
                                         AUDIO_THREAD_PRIORITY,
                                         audioThread,
                                         NULL);
    }
    audio_state = DRV_AUDIO_RUNNING;
    audio_codec_pcm4104_set_mute(false);
}

void drv_audio_stop(void) {
    if ((audio_state != DRV_AUDIO_RUNNING) && (audio_state != DRV_AUDIO_READY)) {
        return;
    }

    audio_codec_pcm4104_set_mute(true);
    audio_dma_stop();

    if (audio_thread != NULL) {
        chThdTerminate(audio_thread);
        chBSemSignal(&audio_dma_sem);
        chThdWait(audio_thread);
        audio_thread = NULL;
    }

    audio_state = DRV_AUDIO_STOPPED;
}

drv_audio_state_t drv_audio_get_state(void) {
    return audio_state;
}

const int32_t* drv_audio_get_input_buffer(uint8_t *index, size_t *frames) {
    chSysLock();
    uint8_t ready = audio_in_ready_index;
    audio_in_ready_index = 0xFFU;
    chSysUnlock();

    if (ready == 0xFFU) {
        return NULL;
    }
    if (index != NULL) {
        *index = ready;
    }
    if (frames != NULL) {
        *frames = AUDIO_FRAMES_PER_BUFFER;
    }
    return (const int32_t*)audio_in_buffers[ready];
}

int32_t* drv_audio_get_output_buffer(uint8_t *index, size_t *frames) {
    chSysLock();
    uint8_t ready = audio_out_ready_index;
    audio_out_ready_index = 0xFFU;
    chSysUnlock();

    if (ready == 0xFFU) {
        return NULL;
    }
    if (index != NULL) {
        *index = ready;
    }
    if (frames != NULL) {
        *frames = AUDIO_FRAMES_PER_BUFFER;
    }
    return (int32_t *)&audio_out_buffers[ready][0][0];
}

void drv_audio_release_buffers(uint8_t in_index, uint8_t out_index) {
    (void)in_index;
    (void)out_index;
    /* DMA circulaire : rien à faire, les buffers seront réécrits au prochain tour. */
}

int32_t (*drv_audio_get_spi_in_buffers(void))[AUDIO_FRAMES_PER_BUFFER][4] {
    return (int32_t (*)[AUDIO_FRAMES_PER_BUFFER][4])spi_in_buffers;
}

int32_t (*drv_audio_get_spi_out_buffers(void))[AUDIO_FRAMES_PER_BUFFER][4] {
    return (int32_t (*)[AUDIO_FRAMES_PER_BUFFER][4])spi_out_buffers;
}

size_t drv_audio_get_spi_frames(void) {
    return AUDIO_FRAMES_PER_BUFFER;
}

void drv_audio_register_spilink_pull(drv_spilink_pull_cb_t cb) {
    spilink_pull_cb = cb;
}

void drv_audio_register_spilink_push(drv_spilink_push_cb_t cb) {
    spilink_push_cb = cb;
}

void drv_audio_set_master_volume(float vol) {
    if (vol < 0.0f) {
        vol = 0.0f;
    }
    chMtxLock(&audio_control.lock);
    audio_control.state.master_volume = vol;
    chMtxUnlock(&audio_control.lock);
}

void drv_audio_set_route(uint8_t track, bool to_main, bool to_cue) {
    if (track >= 4U) {
        return;
    }

    chMtxLock(&audio_control.lock);
    audio_control.state.routes[track].to_main = to_main;
    audio_control.state.routes[track].to_cue = to_cue;
    chMtxUnlock(&audio_control.lock);
}

static float clamp_0_1(float v) {
    if (v < 0.0f) {
        return 0.0f;
    }
    if (v > 1.0f) {
        return 1.0f;
    }
    return v;
}

static void audio_control_init(void) {
    chMtxObjectInit(&audio_control.lock);
    audio_routes_reset_defaults();
}

static void audio_control_get_snapshot(audio_control_snapshot_t *dst) {
    if (dst == NULL) {
        return;
    }
    chMtxLock(&audio_control.lock);
    *dst = audio_control.state;
    chMtxUnlock(&audio_control.lock);
}

void drv_audio_set_route_gain(uint8_t track, float gain_main, float gain_cue) {
    if (track >= 4U) {
        return;
    }

    chMtxLock(&audio_control.lock);
    audio_control.state.routes[track].gain_main = clamp_0_1(gain_main);
    audio_control.state.routes[track].gain_cue = clamp_0_1(gain_cue);
    chMtxUnlock(&audio_control.lock);
}

static void audio_routes_reset_defaults(void) {
    chMtxLock(&audio_control.lock);
    audio_control.state.master_volume = 1.0f;
    for (uint8_t t = 0U; t < 4U; ++t) {
        audio_control.state.routes[t].gain_main = 1.0f;
        audio_control.state.routes[t].gain_cue = 1.0f;
        audio_control.state.routes[t].to_main = true;
        audio_control.state.routes[t].to_cue = false;
    }
    audio_control_cached = audio_control.state;
    chMtxUnlock(&audio_control.lock);
}

static float soft_clip(float x) {
    const float threshold = 0.95f;
    if (x > threshold) {
        const float excess = x - threshold;
        return threshold + (excess / (1.0f + (excess * excess)));
    }
    if (x < -threshold) {
        const float excess = x + threshold;
        return -threshold + (excess / (1.0f + (excess * excess)));
    }
    return x;
}

static void audio_dma_sync_mark(uint8_t half, uint8_t flag) {
    chSysLockFromISR();

    if (audio_sync_half != half) {
        audio_sync_half = half;
        audio_sync_mask = 0U;
    }

    audio_sync_mask |= flag;

    if ((audio_sync_mask & (AUDIO_SYNC_FLAG_RX | AUDIO_SYNC_FLAG_TX)) ==
        (AUDIO_SYNC_FLAG_RX | AUDIO_SYNC_FLAG_TX)) {
        audio_in_ready_index = half;
        audio_out_ready_index = half;
        audio_sync_mask = 0U;
        audio_sync_half = 0xFFU;
        chBSemSignalI(&audio_dma_sem);
    }

    chSysUnlockFromISR();
}

/* -------------------------------------------------------------------------- */
/* Hook DSP faible                                                            */
/* -------------------------------------------------------------------------- */

void __attribute__((weak)) drv_audio_process_block(const int32_t              *adc_in,
                                                   const spilink_audio_block_t spi_in,
                                                   int32_t                    *dac_out,
                                                   spilink_audio_block_t       spi_out,
                                                   size_t                      frames) {
    (void)spi_in;

    const float inv_scale = 1.0f / AUDIO_INT24_MAX_F;
    const audio_control_snapshot_t *ctrl = &audio_control_cached;
    float master = ctrl->master_volume;
    if (master < 0.0f) {
        master = 0.0f;
    }

    const int32_t *adc_ptr = adc_in;
    int32_t *dac_ptr = dac_out;

    for (size_t n = 0; n < frames; ++n) {
        float main_l = 0.0f;
        float main_r = 0.0f;
        float cue_l  = 0.0f;
        float cue_r  = 0.0f;

        for (uint8_t track = 0U; track < 4U; ++track) {
            const audio_route_t *route = &ctrl->routes[track];
            size_t base = (size_t)track * 2U;
            float in_l = (float)adc_ptr[base] * inv_scale;
            float in_r = (float)adc_ptr[base + 1U] * inv_scale;

            if (route->to_main) {
                main_l += in_l * route->gain_main;
                main_r += in_r * route->gain_main;
            }
            if (route->to_cue) {
                cue_l += in_l * route->gain_cue;
                cue_r += in_r * route->gain_cue;
            }
        }

        main_l = soft_clip(main_l);
        main_r = soft_clip(main_r);
        cue_l = soft_clip(cue_l);
        cue_r = soft_clip(cue_r);

        main_l = soft_clip(main_l * master);
        main_r = soft_clip(main_r * master);
        cue_l = soft_clip(cue_l * master);
        cue_r = soft_clip(cue_r * master);

        dac_ptr[0] = (int32_t)(main_l * AUDIO_INT24_MAX_F);
        dac_ptr[1] = (int32_t)(main_r * AUDIO_INT24_MAX_F);
        dac_ptr[2] = (int32_t)(cue_l * AUDIO_INT24_MAX_F);
        dac_ptr[3] = (int32_t)(cue_r * AUDIO_INT24_MAX_F);

        adc_ptr += AUDIO_NUM_INPUT_CHANNELS;
        dac_ptr += AUDIO_NUM_OUTPUT_CHANNELS;
    }

    if (spi_out != NULL) {
        memset(spi_out, 0, sizeof(spi_out_buffers));
    }
}

void __attribute__((weak)) drv_audio_dma_half_cb(uint8_t half) {
    (void)half;
}

void __attribute__((weak)) drv_audio_dma_full_cb(uint8_t half) {
    (void)half;
}

/* -------------------------------------------------------------------------- */
/* Thread audio : déclenché par les callbacks DMA                             */
/* -------------------------------------------------------------------------- */

static THD_FUNCTION(audioThread, arg) {
    (void)arg;
    chRegSetThreadName("audioProcess");

    while (!chThdShouldTerminateX()) {
        chBSemWait(&audio_dma_sem);

        uint8_t in_idx, out_idx;
        size_t frames = AUDIO_FRAMES_PER_BUFFER;

        chSysLock();
        if (audio_in_ready_index == 0xFFU || audio_out_ready_index == 0xFFU) {
            chSysUnlock();
            continue;
        }
        in_idx = audio_in_ready_index;
        out_idx = audio_out_ready_index;
        audio_in_ready_index = 0xFFU;
        audio_out_ready_index = 0xFFU;
        chSysUnlock();

        const int32_t *in_buf = (const int32_t *)audio_in_buffers[in_idx];
        int32_t *out_buf = (int32_t *)audio_out_buffers[out_idx];

        /* Récupère l'audio des cartouches si disponible. */
        if (spilink_pull_cb != NULL) {
            spilink_pull_cb((int32_t (*)[AUDIO_FRAMES_PER_BUFFER][4])spi_in_buffers, frames);
        } else {
            memset((void *)spi_in_buffers, 0, sizeof(spi_in_buffers));
        }

        audio_control_snapshot_t ctrl_snapshot;
        audio_control_get_snapshot(&ctrl_snapshot);
        audio_control_cached = ctrl_snapshot;

        drv_audio_process_block(in_buf,
                                 (int32_t (*)[AUDIO_FRAMES_PER_BUFFER][4])spi_in_buffers,
                                 out_buf,
                                 (int32_t (*)[AUDIO_FRAMES_PER_BUFFER][4])spi_out_buffers,
                                 frames);

        /* Exporte le flux vers les cartouches si besoin. */
        if (spilink_push_cb != NULL) {
            spilink_push_cb((int32_t (*)[AUDIO_FRAMES_PER_BUFFER][4])spi_out_buffers, frames);
        }
    }
}

/* -------------------------------------------------------------------------- */
/* Configuration SAI + DMA                                                    */
/* -------------------------------------------------------------------------- */

static void audio_hw_configure_sai(void) {
#if defined(STM32H7xx) && defined(SAI_xCR1_MODE_0)
#else
    audio_state = DRV_AUDIO_FAULT;
#endif
}

static void audio_dma_start(void) {
    msg_t rx_status = saiStart(audio_sai_rx, &audio_sai_rx_cfg);
    msg_t tx_status = saiStart(audio_sai_tx, &audio_sai_tx_cfg);

    if ((rx_status != HAL_RET_SUCCESS) || (tx_status != HAL_RET_SUCCESS)) {
        audio_state = DRV_AUDIO_FAULT;
        audio_dma_stop();
        return;
    }

    saiSetBuffers(audio_sai_rx, NULL, (void *)audio_in_buffers, AUDIO_DMA_IN_SAMPLES);
    saiSetBuffers(audio_sai_tx, (const void *)audio_out_buffers, NULL, AUDIO_DMA_OUT_SAMPLES);

    /* Démarre le TX esclave avant le RX maître pour éviter les glitches d'horloge. */
    saiStartExchange(audio_sai_tx);
    saiStartExchange(audio_sai_rx);

    if ((audio_sai_rx->state != SAI_ACTIVE) || (audio_sai_tx->state != SAI_ACTIVE)) {
        audio_state = DRV_AUDIO_FAULT;
        audio_dma_stop();
    }
}

static void audio_dma_stop(void) {
    if (audio_sai_rx->state == SAI_ACTIVE) {
        saiStopExchange(audio_sai_rx);
    }
    if (audio_sai_tx->state == SAI_ACTIVE) {
        saiStopExchange(audio_sai_tx);
    }

    if (audio_sai_rx->state == SAI_READY) {
        saiStop(audio_sai_rx);
    }
    if (audio_sai_tx->state == SAI_READY) {
        saiStop(audio_sai_tx);
    }
}

static void audio_sai_dump_state(BaseSequentialStream *chp) {
#if defined(STM32H7xx) && defined(SAI_xCR1_MODE_0)
    audio_sai_dump_regs(chp, "RX", AUDIO_SAI_RX_BLOCK);
    audio_sai_dump_regs(chp, "TX", AUDIO_SAI_TX_BLOCK);
#else
    (void)chp;
#endif
}

#if defined(STM32H7xx) && defined(SAI_xCR1_MODE_0)
static void audio_sai_dump_regs(BaseSequentialStream *chp,
                                const char *tag,
                                const SAI_Block_TypeDef *block) {
    chprintf(chp,
             "[SAI %s] CR1=0x%08lx CR2=0x%08lx FRCR=0x%08lx SLOTR=0x%08lx IMR=0x%08lx SR=0x%08lx\r\n",
             tag,
             (unsigned long)block->CR1,
             (unsigned long)block->CR2,
             (unsigned long)block->FRCR,
             (unsigned long)block->SLOTR,
             (unsigned long)block->IMR,
             (unsigned long)block->SR);
}
#endif
