/*
 * Minimal SAI bring-up: STM32H743 + ChibiOS SAI LLD (SAI1A TX master).
 * - 48 kHz, stereo, 32-bit slots
 * - DMA HT/TC callback = audio metronome
 * - No heap, no cache/MPU usage in application code
 */

#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "audio_codec_ada1979.h"
#include "audio_codec_pcm4104.h"

/* -------------------------------------------------------------------------- */
/* Clock/format assumptions (validated against mcuconf.h + RCC setup)         */
/* -------------------------------------------------------------------------- */
/*
 * SAI1 kernel clock source: STM32_SAI1SEL = PLL2_P (mcuconf.h)
 * PLL2: HSE=25 MHz, DIVM=5, DIVN=98, FRACN=2494, DIVP=10
 *   => f_SAI1 ≈ 49.152 MHz (fractional PLL), suitable for 48 kHz audio.
 *
 * Frame: 2 slots × 32 bits = 64 bits/frame (FRL+1 = 64, power of two)
 * BCLK = 48 kHz × 64 = 3.072 MHz
 * With MCKEN and NODIV=0, MCKDIV uses:
 *   MCKDIV = f_SAI1 / (FS * 256) ≈ 4 (HAL formula for 256×FS MCLK)
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

#define SAI_MCKDIV                4U

#define SINE_FREQ_HZ              1000U
#define SINE_TABLE_SIZE           (AUDIO_SAMPLE_RATE_HZ / SINE_FREQ_HZ)

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

static const int32_t sine_table[SINE_TABLE_SIZE] = {
  0, 1094933, 2171131, 3210181, 4194303, 5106660, 5931641, 6655129,
  7264747, 7750062, 8102772, 8316841, 8388607, 8316841, 8102772, 7750062,
  7264747, 6655129, 5931641, 5106660, 4194303, 3210181, 2171131, 1094933,
  0, -1094933, -2171131, -3210181, -4194303, -5106660, -5931641, -6655129,
  -7264747, -7750062, -8102772, -8316841, -8388607, -8316841, -8102772, -7750062,
  -7264747, -6655129, -5931641, -5106660, -4194304, -3210181, -2171131, -1094933
};
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
static void dump_rcc_clocks(BaseSequentialStream *chp);
static void dump_sai_registers(BaseSequentialStream *chp, const char *tag);
static void dump_dma_registers(BaseSequentialStream *chp, const SAIDriver *saip);
static void codec_diagnostics(BaseSequentialStream *chp);

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
         SAI_xCR1_DS_1 | SAI_xCR1_DS_2 |
         SAI_xCR1_OUTDRIV |
         SAI_xCR1_MCKEN |
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
/* RCC / PLL / SAI clock diagnostics                                          */
/* -------------------------------------------------------------------------- */

static uint32_t rcc_get_pll_source_hz(void) {
  uint32_t src = (RCC->PLLCKSELR & RCC_PLLCKSELR_PLLSRC_Msk) >> RCC_PLLCKSELR_PLLSRC_Pos;

  switch (src) {
  case 0U:
    return STM32_HSI_CK;
  case 1U:
    return STM32_CSI_CK;
  case 2U:
    return STM32_HSE_CK;
  default:
    return 0U;
  }
}

static void pll_dump(BaseSequentialStream *chp,
                     const char *name,
                     uint32_t divm,
                     uint32_t divr,
                     uint32_t fracr) {
  uint32_t m = (divm == 0U) ? 1U : divm;
  uint32_t n = ((divr & RCC_PLL1DIVR_N1_Msk) >> RCC_PLL1DIVR_N1_Pos) + 1U;
  uint32_t p = ((divr & RCC_PLL1DIVR_P1_Msk) >> RCC_PLL1DIVR_P1_Pos) + 1U;
  uint32_t q = ((divr & RCC_PLL1DIVR_Q1_Msk) >> RCC_PLL1DIVR_Q1_Pos) + 1U;
  uint32_t r = ((divr & RCC_PLL1DIVR_R1_Msk) >> RCC_PLL1DIVR_R1_Pos) + 1U;
  uint32_t frac = (fracr & RCC_PLL1FRACR_FRACN1_Msk) >> RCC_PLL1FRACR_FRACN1_Pos;
  uint32_t src_hz = rcc_get_pll_source_hz();
  uint64_t ref_hz = (m == 0U) ? 0U : (uint64_t)src_hz / (uint64_t)m;
  uint64_t vco = (ref_hz * ((uint64_t)n * 8192U + (uint64_t)frac)) / 8192U;
  uint64_t p_hz = (p == 0U) ? 0U : (vco / (uint64_t)p);
  uint64_t q_hz = (q == 0U) ? 0U : (vco / (uint64_t)q);
  uint64_t r_hz = (r == 0U) ? 0U : (vco / (uint64_t)r);

  chprintf(chp,
           "[PLL%s] M=%lu N=%lu P=%lu Q=%lu R=%lu FRAC=%lu | ref=%lu Hz vco=%lu Hz P=%lu Hz Q=%lu Hz R=%lu Hz\r\n",
           name,
           (unsigned long)m,
           (unsigned long)n,
           (unsigned long)p,
           (unsigned long)q,
           (unsigned long)r,
           (unsigned long)frac,
           (unsigned long)ref_hz,
           (unsigned long)vco,
           (unsigned long)p_hz,
           (unsigned long)q_hz,
           (unsigned long)r_hz);
}

static void dump_rcc_clocks(BaseSequentialStream *chp) {
  uint32_t sai1sel = (RCC->D2CCIP1R & RCC_D2CCIP1R_SAI1SEL_Msk) >> RCC_D2CCIP1R_SAI1SEL_Pos;
  const char *sai1src = "UNKNOWN";
  uint32_t sai1clk = 0U;

  chprintf(chp, "=== RCC CLOCKS ===\r\n");
  chprintf(chp, "SystemCoreClock=%lu Hz\r\n", (unsigned long)SystemCoreClock);
#ifdef STM32_HCLK
  chprintf(chp, "HCLK=%lu Hz\r\n", (unsigned long)STM32_HCLK);
#endif
#ifdef STM32_PCLK1
  chprintf(chp, "PCLK1=%lu Hz\r\n", (unsigned long)STM32_PCLK1);
#endif
#ifdef STM32_PCLK2
  chprintf(chp, "PCLK2=%lu Hz\r\n", (unsigned long)STM32_PCLK2);
#endif

  pll_dump(chp, "1",
           (RCC->PLLCKSELR & RCC_PLLCKSELR_DIVM1_Msk) >> RCC_PLLCKSELR_DIVM1_Pos,
           RCC->PLL1DIVR,
           RCC->PLL1FRACR);
  pll_dump(chp, "2",
           (RCC->PLLCKSELR & RCC_PLLCKSELR_DIVM2_Msk) >> RCC_PLLCKSELR_DIVM2_Pos,
           RCC->PLL2DIVR,
           RCC->PLL2FRACR);
  pll_dump(chp, "3",
           (RCC->PLLCKSELR & RCC_PLLCKSELR_DIVM3_Msk) >> RCC_PLLCKSELR_DIVM3_Pos,
           RCC->PLL3DIVR,
           RCC->PLL3FRACR);

  switch (sai1sel) {
  case 0U:
    sai1src = "PLL1_Q";
    sai1clk = STM32_PLL1_Q_CK;
    break;
  case 1U:
    sai1src = "PLL2_P";
    sai1clk = STM32_PLL2_P_CK;
    break;
  case 2U:
    sai1src = "PLL3_P";
    sai1clk = STM32_PLL3_P_CK;
    break;
  case 3U:
    sai1src = "I2S_CKIN";
    sai1clk = 0U;
    break;
  case 4U:
    sai1src = "PER_CK";
    sai1clk = STM32_PER_CK;
    break;
  default:
    break;
  }

  chprintf(chp, "SAI1SEL=%lu (%s) => %lu Hz\r\n",
           (unsigned long)sai1sel,
           sai1src,
           (unsigned long)sai1clk);
  chprintf(chp, "\r\n");
}

/* -------------------------------------------------------------------------- */
/* SAI + DMA register diagnostics                                             */
/* -------------------------------------------------------------------------- */

static void dump_sai_registers(BaseSequentialStream *chp, const char *tag) {
  chprintf(chp, "=== SAI1 REGISTERS (%s) ===\r\n", tag);
  chprintf(chp, "SAI1_GCR = 0x%08lx\r\n", (unsigned long)SAI1->GCR);
  chprintf(chp,
           "SAI1A: CR1=0x%08lx CR2=0x%08lx FRCR=0x%08lx SLOTR=0x%08lx\r\n",
           (unsigned long)SAI1_Block_A->CR1,
           (unsigned long)SAI1_Block_A->CR2,
           (unsigned long)SAI1_Block_A->FRCR,
           (unsigned long)SAI1_Block_A->SLOTR);
  chprintf(chp,
           "SAI1A: IMR=0x%08lx SR=0x%08lx CLRFR=0x%08lx DR=0x%08lx\r\n",
           (unsigned long)SAI1_Block_A->IMR,
           (unsigned long)SAI1_Block_A->SR,
           (unsigned long)SAI1_Block_A->CLRFR,
           (unsigned long)SAI1_Block_A->DR);
  chprintf(chp, "\r\n");
}

static void dump_dma_registers(BaseSequentialStream *chp, const SAIDriver *saip) {
  const stm32_dma_stream_t *dmastp = saip->dmatx;

  chprintf(chp, "=== DMA REGISTERS (SAI1A TX) ===\r\n");
  if (dmastp == NULL) {
    chprintf(chp, "DMA stream not allocated.\r\n\r\n");
    return;
  }

  chprintf(chp,
           "CR=0x%08lx NDTR=0x%08lx PAR=0x%08lx M0AR=0x%08lx FCR=0x%08lx\r\n",
           (unsigned long)dmastp->stream->CR,
           (unsigned long)dmastp->stream->NDTR,
           (unsigned long)dmastp->stream->PAR,
           (unsigned long)dmastp->stream->M0AR,
           (unsigned long)dmastp->stream->FCR);
#if STM32_DMA_SUPPORTS_DMAMUX == TRUE
  chprintf(chp, "DMAMUX_CCR=0x%08lx\r\n", (unsigned long)dmastp->mux->CCR);
#endif
  chprintf(chp, "\r\n");
}

static const char *sai_fifo_level_name(uint32_t flvl) {
  switch (flvl) {
  case 0U:
    return "EMPTY";
  case 1U:
    return "1/4";
  case 2U:
    return "1/2";
  case 3U:
    return "3/4";
  case 4U:
    return "FULL";
  default:
    return "RESV";
  }
}

/* -------------------------------------------------------------------------- */
/* Codec diagnostics                                                          */
/* -------------------------------------------------------------------------- */

static void codec_diagnostics(BaseSequentialStream *chp) {
  msg_t st;
  uint32_t i2c_err;

  chprintf(chp, "=== CODEC INIT ===\r\n");
  adau1979_set_log_stream(chp);
  audio_codec_pcm4104_set_log_stream(chp);

  st = adau1979_init();
  i2c_err = i2cGetErrors(&AUDIO_I2C_DRIVER);

  chprintf(chp, "I2C init %s (state=%u err=0x%08lx)\r\n",
           (AUDIO_I2C_DRIVER.state != I2C_STOP) ? "OK" : "FAIL",
           (unsigned)AUDIO_I2C_DRIVER.state,
           (unsigned long)i2c_err);

  chprintf(chp, "ADAU1979 init %s\r\n", (st == HAL_RET_SUCCESS) ? "OK" : "FAIL");

  chprintf(chp, "Resetting PCM4104\r\n");
  audio_codec_pcm4104_init();
  chprintf(chp, "PCM4104 reg writes: N/A (hardware mode)\r\n");
  chprintf(chp, "Unmuting PCM4104\r\n");
  audio_codec_pcm4104_set_mute(false);

  st = adau1979_set_default_config();
  chprintf(chp, "ADAU1979 default config %s\r\n",
           (st == HAL_RET_SUCCESS) ? "OK" : "FAIL");
  adau1979_mute(false);

  chprintf(chp, "=== CODEC SUMMARY ===\r\n");
  chprintf(chp, "PCM4104: OK / UNMUTED\r\n");
  chprintf(chp, "ADAU1979: %s\r\n", (st == HAL_RET_SUCCESS) ? "OK" : "FAIL");
  chprintf(chp, "I2C: %s\r\n", (AUDIO_I2C_DRIVER.state != I2C_STOP) ? "OK" : "FAIL");
  chprintf(chp, "\r\n");
}

/* -------------------------------------------------------------------------- */
/* Main                                                                        */
/* -------------------------------------------------------------------------- */

int main(void) {
  BaseSequentialStream *chp;

int main(void) {
  halInit();
  chSysInit();

  sdStart(&SD1, &sercfg);
  chp = (BaseSequentialStream *)&SD1;

  chprintf(chp,
           "\r\n=== STM32H743 SAI1A TX BRING-UP (48 kHz, stereo, MCLK) ===\r\n");

  dump_rcc_clocks(chp);

  codec_diagnostics(chp);

  fill_half_buffer(0U);
  fill_half_buffer(1U);

  saiStart(&SAID1A, &sai_tx_config);
  dump_sai_registers(chp, "after saiStart");

  saiSetBuffers(&SAID1A,
                audio_tx_buffer,
                NULL,
                AUDIO_FRAME_SAMPLES * AUDIO_CHANNELS * AUDIO_BUFFER_HALVES);
  dump_dma_registers(chp, &SAID1A);

  saiStartExchange(&SAID1A);
  dump_sai_registers(chp, "after saiStartExchange");

  while (true) {
    uint32_t sai_flags;
    uint32_t sai_repeats;
    uint32_t sr;
    uint32_t flvl;

    osalSysLock();
    sai_flags = SAID1A.error_flags;
    sai_repeats = SAID1A.error_repeats;
    SAID1A.error_flags = 0U;
    SAID1A.error_repeats = 0U;
    sr = SAID1A.blockp->SR;
    osalSysUnlock();

    if (sai_flags != 0U) {
      audio_sai_error_count++;
    }
    if (sai_repeats != 0U) {
      audio_sai_error_repeats += sai_repeats;
    }

    flvl = (sr & SAI_xSR_FLVL_Msk) >> SAI_xSR_FLVL_Pos;

    chprintf(chp, "=== RUNTIME STATUS ===\r\n");
    chprintf(chp,
             "HT=%lu TC=%lu DMA_ERR=%lu SAI_ERR=%lu SAI_REPEAT=%lu\r\n",
             (unsigned long)audio_ht_count,
             (unsigned long)audio_tc_count,
             (unsigned long)audio_dma_error_count,
             (unsigned long)audio_sai_error_count,
             (unsigned long)audio_sai_error_repeats);
    chprintf(chp,
             "SAI_SR=0x%08lx FIFO=%s (0x%lx)\r\n\r\n",
             (unsigned long)sr,
             sai_fifo_level_name(flvl),
             (unsigned long)flvl);

    chThdSleepMilliseconds(1000);
  }

  return 0;
}
