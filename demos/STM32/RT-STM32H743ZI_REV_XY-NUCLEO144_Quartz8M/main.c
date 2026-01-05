#include "ch.h"
#include "hal.h"
#include "audio/audio_conf.h"
#include "drivers/drivers.h"
#include <stdio.h>

/*
 * Hardware-dependent: verify GPIO lines and AF numbers against the board
 * pinout and RM0433 before first flash.
 */
#define LINE_SAI2A_SCK  LINE_SAI2_SCK_A
#define LINE_SAI2A_FS   LINE_SAI2_FS_A
#define LINE_SAI2A_SD   LINE_SAI2_SD_A

#define LINE_DBG_HT     PAL_LINE(GPIOH, 7U)  /* PH7 LED on board (confirm). */
#define LINE_DBG_TC     PAL_LINE(GPIOH, 8U)  /* Free pin: confirm on board. */

#define SAI_DMA_ITEMS   256U

static volatile uint32_t ht_count = 0U;
static volatile uint32_t tc_count = 0U;
static volatile uint32_t dma_error_count = 0U;
static volatile uint32_t sai_error_flags = 0U;

static uint32_t sai_tx_buffer[SAI_DMA_ITEMS];

static THD_WORKING_AREA(waOledThread, 512);

static void sai_test_callback(SAIDriver *saip, bool half) {
  (void)saip;

  if (half) {
    ht_count++;
    palToggleLine(LINE_DBG_HT);
  }
  else {
    tc_count++;
    palToggleLine(LINE_DBG_TC);
  }
}

void sai_dma_error_hook(SAIDriver *saip) {
  (void)saip;
  dma_error_count++;
}

static const char *sai_state_label(saistate_t state) {
  switch (state) {
    case SAI_READY:
      return "READY";
    case SAI_ACTIVE:
      return "ACTIVE";
    default:
      return "OTHER";
  }
}

static void sai_gpio_init(void) {
  palSetLineMode(LINE_SAI2A_SCK, PAL_MODE_ALTERNATE(10) | PAL_STM32_OSPEED_HIGHEST);
  palSetLineMode(LINE_SAI2A_FS,  PAL_MODE_ALTERNATE(10) | PAL_STM32_OSPEED_HIGHEST);
  palSetLineMode(LINE_SAI2A_SD,  PAL_MODE_ALTERNATE(10) | PAL_STM32_OSPEED_HIGHEST);

  palSetLineMode(LINE_DBG_HT, PAL_MODE_OUTPUT_PUSHPULL);
  palSetLineMode(LINE_DBG_TC, PAL_MODE_OUTPUT_PUSHPULL);
  palClearLine(LINE_DBG_HT);
  palClearLine(LINE_DBG_TC);
}

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
         (SAI_xCR1_DS_0 | SAI_xCR1_DS_1 | SAI_xCR1_DS_2),
  .cr2 = SAI_xCR2_FTH_1,
  .frcr = (63U << SAI_xFRCR_FRL_Pos) |
          (31U << SAI_xFRCR_FSALL_Pos),
  .slotr = (0U << SAI_xSLOTR_FBOFF_Pos) |
           (SAI_xSLOTR_SLOTSZ_1) |
           (1U << SAI_xSLOTR_NBSLOT_Pos) |
           (0x3U << SAI_xSLOTR_SLOTEN_Pos),
  .dma_mode = STM32_DMA_CR_PSIZE_WORD | STM32_DMA_CR_MSIZE_WORD
};

static THD_FUNCTION(oledThread, arg) {
  (void)arg;
  chRegSetThreadName("OLED SAI");

  while (true) {
    char line[32];
    uint32_t local_ht = ht_count;
    uint32_t local_tc = tc_count;
    uint32_t local_dma_err = dma_error_count;
    uint32_t local_sr = sai_error_flags;
    saistate_t local_state = SAID2A.state;

    drv_display_clear();
    drv_display_draw_text(0, 0, "SAI TEST");

    snprintf(line, sizeof(line), "HT:%lu", (unsigned long)local_ht);
    drv_display_draw_text(0, 8, line);

    snprintf(line, sizeof(line), "TC:%lu", (unsigned long)local_tc);
    drv_display_draw_text(0, 16, line);

    snprintf(line, sizeof(line), "DMAE:%lu", (unsigned long)local_dma_err);
    drv_display_draw_text(0, 24, line);

    snprintf(line, sizeof(line), "SR:%08lX", (unsigned long)local_sr);
    drv_display_draw_text(0, 32, line);

    snprintf(line, sizeof(line), "ST:%s", sai_state_label(local_state));
    drv_display_draw_text(0, 40, line);

    chThdSleepMilliseconds(500);
  }
}

int main(void) {
  halInit();
  chSysInit();

  drivers_init_all();

  sai_gpio_init();
  sai_fill_silence();

  saiStart(&SAID2A, &sai_cfg);
  saiSetBuffers(&SAID2A, sai_tx_buffer, NULL, SAI_DMA_ITEMS);
  saiStartExchange(&SAID2A);

  chThdCreateStatic(waOledThread, sizeof(waOledThread),
                    NORMALPRIO - 1, oledThread, NULL);

  while (true) {
    chThdSleepSeconds(5);

    saiStopExchange(&SAID2A);
    /* CMSIS (stm32h743xx.h): status flags are in SAI_Block_TypeDef::SR. */
    sai_error_flags = SAID2A.blockp->SR;

    chThdSleepSeconds(1);

    saiStartExchange(&SAID2A);
  }
}
