#include "ch.h"
#include "hal.h"
#include "drivers/drivers.h"
#include "drv_display.h"
#include <stdio.h>

#define ADC_DMA_BUFFER_DEPTH   16U

static adcsample_t adc_dma_buf[ADC_DMA_BUFFER_DEPTH]
    __attribute__((section(".dma"), aligned(32)));

static volatile size_t adc_latest_index = 0U;
static bool adc_half_phase = true;

static void adc_dma_callback(ADCDriver *adcp) {
  (void)adcp;

  if (adc_half_phase) {
    adc_latest_index = (ADC_DMA_BUFFER_DEPTH / 2U) - 1U;
  } else {
    adc_latest_index = ADC_DMA_BUFFER_DEPTH - 1U;
  }

  adc_half_phase = !adc_half_phase;
}

static const ADCConfig adccfg = {
  .difsel = 0U,
  .calibration = 0U,
};

static const ADCConversionGroup adcgrpcfg = {
  .circular = true,
  .num_channels = 1,
  .end_cb = adc_dma_callback,
  .error_cb = NULL,
  .cfgr = ADC_CFGR_CONT,
  .cfgr2 = 0U,
  .ccr = 0U,
  .pcsel = ADC_SELMASK_IN7,
  .ltr1 = 0U,
  .htr1 = 0U,
  .ltr2 = 0U,
  .htr2 = 0U,
  .ltr3 = 0U,
  .htr3 = 0U,
  .awd2cr = 0U,
  .awd3cr = 0U,
  .smpr = {
    ADC_SMPR1_SMP_AN7(ADC_SMPR_SMP_640P5),
    0U,
  },
  .sqr = {
    ADC_SQR1_SQ1_N(ADC_CHANNEL_IN7),
    0U,
    0U,
    0U
  }
};

static uint16_t adc1_read_dma_latest(void) {
  size_t index = adc_latest_index;

  return adc_dma_buf[index] & 0x0FFFU;
}



int main(void) {
  halInit();
  chSysInit();

  drivers_init_all();
  drv_display_init();

  palSetPadMode(GPIOA, 7U, PAL_MODE_INPUT_ANALOG);
  adcStart(&ADCD1, &adccfg);
  chThdSleepMilliseconds(2);
  adcStartConversion(&ADCD1, &adcgrpcfg, adc_dma_buf, ADC_DMA_BUFFER_DEPTH);

  char line1[32];
  char line2[32];

  uint32_t counter = 0;

  while (true) {
    uint16_t v = adc1_read_dma_latest();

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
