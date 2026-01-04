#include "ch.h"
#include "hal.h"
#include "audio/audio_conf.h"

/*
 * Hardware-dependent: verify GPIO lines and AF numbers against the board
 * pinout and RM0433 before first flash.
 */
#define LINE_SAI2A_SCK  LINE_SAI2_SCK_A
#define LINE_SAI2A_FS   LINE_SAI2_FS_A
#define LINE_SAI2A_SD   LINE_SAI2_SD_A

#define LINE_DBG_HT     PAL_LINE(GPIOH, 7U)  /* PH7 LED on board */
#define LINE_DBG_TC     PAL_LINE(GPIOH, 8U)  /* Free pin */

#define SAI_DMA_ITEMS   256U

static const SerialConfig uart_cfg = {
  115200,
  0,
  0,
  0
};

static volatile uint32_t ht_count = 0U;
static volatile uint32_t tc_count = 0U;
static volatile uint32_t dma_error_count = 0U;
static volatile uint32_t sai_error_flags = 0U;

static AUDIO_DMA_BUFFER_ATTR uint32_t sai_tx_buffer[SAI_DMA_ITEMS];

/* ==== DMA CALLBACK ======================================================= */

static void sai_test_callback(SAIDriver *saip, bool half) {
  (void)saip;

  if (half) {
    ht_count++;
    palToggleLine(LINE_DBG_HT);
  } else {
    tc_count++;
    palToggleLine(LINE_DBG_TC);
  }
}

void sai_dma_error_hook(SAIDriver *saip) {
  (void)saip;
  dma_error_count++;
}

/* ==== GPIO =============================================================== */

static void sai_gpio_init(void) {
  palSetLineMode(LINE_SAI2A_SCK,
                 PAL_MODE_ALTERNATE(10) | PAL_STM32_OSPEED_HIGHEST);
  palSetLineMode(LINE_SAI2A_FS,
                 PAL_MODE_ALTERNATE(10) | PAL_STM32_OSPEED_HIGHEST);
  palSetLineMode(LINE_SAI2A_SD,
                 PAL_MODE_ALTERNATE(10) | PAL_STM32_OSPEED_HIGHEST);

  palSetLineMode(LINE_DBG_HT, PAL_MODE_OUTPUT_PUSHPULL);
  palSetLineMode(LINE_DBG_TC, PAL_MODE_OUTPUT_PUSHPULL);

  palClearLine(LINE_DBG_HT);
  palClearLine(LINE_DBG_TC);
}

/* ==== AUDIO ============================================================= */

static void sai_fill_silence(void) {
  for (size_t i = 0; i < SAI_DMA_ITEMS; i++) {
    sai_tx_buffer[i] = 0U;
  }
}

static const SAIConfig sai_cfg = {
  .tx_buffer = sai_tx_buffer,
  .rx_buffer = NULL,
  .size = SAI_DMA_ITEMS,
  .end_cb = sai_test_callback,

  .gcr = 0U,

  .cr1 = SAI_xCR1_NOMCK |
         (SAI_xCR1_DS_0 | SAI_xCR1_DS_1 | SAI_xCR1_DS_2), /* 32-bit */

  .cr2 = SAI_xCR2_FTH_1, /* FIFO half */

  .frcr = (63U << SAI_xFRCR_FRL_Pos) |   /* 64 bits frame */
          (31U << SAI_xFRCR_FSALL_Pos), /* 32 bits FS */

  .slotr = (0U << SAI_xSLOTR_FBOFF_Pos) |
           (SAI_xSLOTR_SLOTSZ_1) |      /* 32-bit slots */
           (1U << SAI_xSLOTR_NBSLOT_Pos) |
           (0x3U << SAI_xSLOTR_SLOTEN_Pos),

  .dma_mode = STM32_DMA_CR_PSIZE_WORD |
              STM32_DMA_CR_MSIZE_WORD
};

/* ==== MAIN ============================================================== */

int main(void) {
  halInit();
  chSysInit();

  sdStart(&SD1, &uart_cfg);

  /* Use sdWrite(): zero formatting, minimal stack usage */
  sdWrite(&SD1, (const uint8_t *)"\r\nSAI LLD bring-up test\r\n", 25);
  sdWrite(&SD1, (const uint8_t *)"Verify SAI2A pins\r\n", 20);

  sai_gpio_init();
  sai_fill_silence();

  saiStart(&SAID2A, &sai_cfg);
  saiSetBuffers(&SAID2A, sai_tx_buffer, NULL, SAI_DMA_ITEMS);
  saiStartExchange(&SAID2A);

  sdWrite(&SD1, (const uint8_t *)"SAI started\r\n", 13);

  while (true) {
    chThdSleepSeconds(5);

    saiStopExchange(&SAID2A);

    /* CMSIS: status flags are in SAI_Block_TypeDef::SR */
    sai_error_flags = SAID2A.blockp->SR;

    /* Minimal numeric print: split to avoid formatter */
    sdWrite(&SD1, (const uint8_t *)"SAI stopped\r\n", 13);

    chThdSleepSeconds(1);

    saiStartExchange(&SAID2A);
    sdWrite(&SD1, (const uint8_t *)"SAI restarted\r\n", 15);
  }
}
