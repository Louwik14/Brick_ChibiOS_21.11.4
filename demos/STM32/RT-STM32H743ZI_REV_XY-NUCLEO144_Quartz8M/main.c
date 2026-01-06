#include "ch.h"
#include "hal.h"
#include "drivers/drivers.h"
#include "drv_display.h"
#include <stdio.h>

/* ADC configuration for ADC1 IN7 (PA7). */
#define ADC_CHANNEL            ADC_CHANNEL_IN7
#define ADC_CHANNEL_MASK       (1U << 7)

#define ADC_BUF_DEPTH          1U

/* DMA buffer: MUST be 32-byte aligned and in DMA-safe memory. */
static uint16_t adc_buf[1] __attribute__((section(".dma"), aligned(32)));

static const ADCConfig adccfg = {
  .difsel = 0U,
};

static const ADCConversionGroup adcgrpcfg = {
  .circular     = TRUE,
  .num_channels = 1,
  .end_cb       = NULL,
  .error_cb     = NULL,

  .cfgr  = ADC_CFGR_CONT,
  .cfgr2 = 0,

  // Channel enable mask: bit 7 = ADC channel 7
  .pcsel = (1U << 7),

  // Sampling time for channel 7
  .smpr = {
    ADC_SMPR1_SMP_AN7(ADC_SMPR_SMP_64P5),
    0
  },

  // Sequence: SQ1 = channel 7
  .sqr = {
    ADC_SQR1_SQ1_N(7),
    0, 0, 0
  }
};



int main(void) {
  halInit();
  chSysInit();

  drivers_init_all();
  drv_display_init();

  /* PA7 as ADC1_IN7 analog input. */
  palSetPadMode(GPIOA, 7, PAL_MODE_INPUT_ANALOG);

  /* Start ADC driver. */
  msg_t ret = adcStart(&ADCD1, &adccfg);
  if (ret != HAL_RET_SUCCESS) {
    drv_display_clear();
    drv_display_draw_text(0, 0, "ADC START FAIL");
    drv_display_update();
    while (true) {
      chThdSleepMilliseconds(1000);
    }
  }

  /* Clean buffer before DMA writes (cache). */
  cacheBufferFlush(adc_buf, sizeof(adc_buf));

  /* Start continuous conversion with DMA (circular). */
  adcStartConversion(&ADCD1, &adcgrpcfg, adc_buf, ADC_BUF_DEPTH);

  char line1[32];
  char line2[32];

  uint32_t counter = 0;

  while (true) {
    /* Invalidate cache before reading DMA buffer. */
    cacheBufferInvalidate(adc_buf, sizeof(adc_buf));

    uint16_t v = adc_buf[0] & 0x0FFF;  /* 12-bit mask */

    counter++;

    drv_display_clear();
    drv_display_draw_text(0, 0, "ADC1 DMA OK");

    snprintf(line1, sizeof(line1), "VAL = %u", (unsigned)v);
    drv_display_draw_text(0, 12, line1);

    snprintf(line2, sizeof(line2), "CNT = %lu", (unsigned long)counter);
    drv_display_draw_text(0, 24, line2);

    drv_display_update();

    chThdSleepMilliseconds(200);
  }
}
