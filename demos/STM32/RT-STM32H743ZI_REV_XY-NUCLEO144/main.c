/*
 * Minimal SAI bring-up: STM32H743 + ChibiOS SAI LLD (SAI2A TX master).
 * - 48 kHz, stereo, 32-bit slots
 * - DMA HT/TC callback = audio metronome
 * - No heap, no cache/MPU usage in application code
 */

#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include <math.h>

/* -------------------------------------------------------------------------- */
/* Clock/format assumptions (validated against mcuconf.h + RCC setup)         */
/* -------------------------------------------------------------------------- */
/*
 * SAI2 kernel clock source: STM32_SAI23SEL = PLL2_P (mcuconf.h)
 * PLL2: HSE=25 MHz, DIVM=5, DIVN=98, FRACN=2494, DIVP=10
 *   => f_SAI2 ≈ 49.152 MHz (fractional PLL), suitable for 48 kHz audio.
 *
 * Frame: 2 slots × 32 bits = 64 bits/frame
 * BCLK = 48 kHz × 64 = 3.072 MHz
 * MCKDIV = 15 => SCK = f_SAI2 / (MCKDIV+1) ≈ 49.152/16 = 3.072 MHz
 *
 * RM0433: PCLK_APB2 > 2 × BCLK requirement.
 */

#define AUDIO_SAMPLE_RATE_HZ      48000U
#define AUDIO_FRAME_SAMPLES       64U
#define AUDIO_CHANNELS            2U
#define AUDIO_SLOT_BITS           32U
#define AUDIO_FRAME_BITS          (AUDIO_CHANNELS * AUDIO_SLOT_BITS)
#define AUDIO_BCLK_HZ             (AUDIO_SAMPLE_RATE_HZ * AUDIO_FRAME_BITS)
#define AUDIO_BUFFER_HALVES       2U

#define SAI_MCKDIV                15U

#define SINE_FREQ_HZ              1000U
#define SINE_TABLE_SIZE           (AUDIO_SAMPLE_RATE_HZ / SINE_FREQ_HZ)
#define SINE_AMPLITUDE            0x007FFFFF
#define AUDIO_TWO_PI              6.2831853071795864769f

#if defined(STM32_PCLK2)
#if STM32_PCLK2 < (2U * AUDIO_BCLK_HZ)
#error "PCLK2 must be >= 2x BCLK per RM0433"
#endif
#endif

_Static_assert((AUDIO_SAMPLE_RATE_HZ % SINE_FREQ_HZ) == 0U,
               "Sine table must be integer length");
_Static_assert(AUDIO_FRAME_BITS == 64U, "Expected 64 bits per audio frame");

/* -------------------------------------------------------------------------- */
/* UART1 (SD1)                                                                */
/* -------------------------------------------------------------------------- */

static SerialConfig sercfg = {
  115200,
  0,
  USART_CR2_STOP1_BITS,
  0
};

/* -------------------------------------------------------------------------- */
/* DMA-safe buffers (D2 SRAM, aligned >= 32 bytes)                            */
/* -------------------------------------------------------------------------- */

#define AUDIO_DMA_BUFFER_ATTR __attribute__((section(".ram_d2"), aligned(32)))

static int32_t AUDIO_DMA_BUFFER_ATTR
audio_tx_buffer[AUDIO_BUFFER_HALVES][AUDIO_FRAME_SAMPLES][AUDIO_CHANNELS];

/* -------------------------------------------------------------------------- */
/* Sine generation                                                            */
/* -------------------------------------------------------------------------- */

static int32_t sine_table[SINE_TABLE_SIZE];
static uint32_t sine_index = 0U;

/* -------------------------------------------------------------------------- */
/* Diagnostics                                                                */
/* -------------------------------------------------------------------------- */

static volatile uint32_t audio_ht_count = 0U;
static volatile uint32_t audio_tc_count = 0U;
static volatile uint32_t audio_dma_error_count = 0U;
static volatile uint32_t audio_sai_error_count = 0U;
static volatile uint32_t audio_sai_error_repeats = 0U;

/* -------------------------------------------------------------------------- */
/* Forward declarations                                                       */
/* -------------------------------------------------------------------------- */

static void fill_half_buffer(uint8_t half);
static void sai_tx_end_cb(SAIDriver *saip, bool half);

/* -------------------------------------------------------------------------- */
/* SAI DMA error hook (called inside DMA ISR)                                 */
/* -------------------------------------------------------------------------- */

void sai_dma_error_hook(SAIDriver *saip) {
  (void)saip;
  audio_dma_error_count++;
}

/* -------------------------------------------------------------------------- */
/* SAI configuration (safe bring-up profile)                                  */
/* -------------------------------------------------------------------------- */

static const SAIConfig sai_tx_config = {
  .tx_buffer = audio_tx_buffer,
  .rx_buffer = NULL,
  .size = (AUDIO_FRAME_SAMPLES * AUDIO_CHANNELS * AUDIO_BUFFER_HALVES),
  .end_cb = sai_tx_end_cb,
  .gcr = 0U,
  .cr1 = SAI_xCR1_PRTCFG_0 |
         SAI_xCR1_DS_0 | SAI_xCR1_DS_1 | SAI_xCR1_DS_2 |
         SAI_xCR1_OUTDRIV |
         SAI_xCR1_NOMCK |
         (SAI_MCKDIV << SAI_xCR1_MCKDIV_Pos),
  .cr2 = SAI_xCR2_FTH_0,
  .frcr = ((AUDIO_FRAME_BITS - 1U) << SAI_xFRCR_FRL_Pos) |
          ((AUDIO_FRAME_BITS / 2U - 1U) << SAI_xFRCR_FSALL_Pos) |
          SAI_xFRCR_FSDEF | SAI_xFRCR_FSOFF,
  .slotr = (0U << SAI_xSLOTR_FBOFF_Pos) |
           SAI_xSLOTR_SLOTSZ_1 |
           ((AUDIO_CHANNELS - 1U) << SAI_xSLOTR_NBSLOT_Pos) |
           0x0003U,
  .dma_mode = STM32_DMA_CR_PSIZE_WORD | STM32_DMA_CR_MSIZE_WORD
};

/* -------------------------------------------------------------------------- */
/* Audio callback (DMA HT/TC)                                                 */
/* -------------------------------------------------------------------------- */

static void sai_tx_end_cb(SAIDriver *saip, bool half) {
  (void)saip;
  if (half) {
    audio_ht_count++;
    fill_half_buffer(0U);
  }
  else {
    audio_tc_count++;
    fill_half_buffer(1U);
  }
}

static void fill_half_buffer(uint8_t half) {
  size_t i;
  int32_t (*buf)[AUDIO_CHANNELS] = audio_tx_buffer[half];

  for (i = 0U; i < AUDIO_FRAME_SAMPLES; i++) {
    int32_t sample = sine_table[sine_index];
    sine_index++;
    if (sine_index >= SINE_TABLE_SIZE) {
      sine_index = 0U;
    }
    buf[i][0] = sample;
    buf[i][1] = sample;
  }
}

/* -------------------------------------------------------------------------- */
/* Main                                                                        */
/* -------------------------------------------------------------------------- */

int main(void) {
  size_t i;

  halInit();
  chSysInit();

  sdStart(&SD1, &sercfg);
  chprintf((BaseSequentialStream *)&SD1,
           "\r\n=== STM32H743 SAI2A TX BRING-UP (48 kHz, stereo) ===\r\n");

  for (i = 0U; i < SINE_TABLE_SIZE; i++) {
    float phase = (AUDIO_TWO_PI * (float)i) / (float)SINE_TABLE_SIZE;
    sine_table[i] = (int32_t)(sinf(phase) * (float)SINE_AMPLITUDE);
  }

  fill_half_buffer(0U);
  fill_half_buffer(1U);

  saiStart(&SAID2A, &sai_tx_config);
  saiSetBuffers(&SAID2A,
                audio_tx_buffer,
                NULL,
                AUDIO_FRAME_SAMPLES * AUDIO_CHANNELS * AUDIO_BUFFER_HALVES);
  saiStartExchange(&SAID2A);

  while (true) {
    uint32_t sai_flags;
    uint32_t sai_repeats;

    osalSysLock();
    sai_flags = SAID2A.error_flags;
    sai_repeats = SAID2A.error_repeats;
    SAID2A.error_flags = 0U;
    SAID2A.error_repeats = 0U;
    osalSysUnlock();

    if (sai_flags != 0U) {
      audio_sai_error_count++;
    }
    if (sai_repeats != 0U) {
      audio_sai_error_repeats += sai_repeats;
    }

    chprintf((BaseSequentialStream *)&SD1,
             "HT=%lu TC=%lu DMA_ERR=%lu SAI_ERR=%lu SAI_REPEAT=%lu SR=0x%08lx\r\n",
             (unsigned long)audio_ht_count,
             (unsigned long)audio_tc_count,
             (unsigned long)audio_dma_error_count,
             (unsigned long)audio_sai_error_count,
             (unsigned long)audio_sai_error_repeats,
             (unsigned long)SAID2A.blockp->SR);

    chThdSleepMilliseconds(1000);
  }
}
