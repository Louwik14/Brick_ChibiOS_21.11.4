#include "ch.h"
#include "hal.h"
#include "drivers/drivers.h"
#include "drv_display.h"
#include <stdio.h>

/* ================= ADC ================= */

static adcsample_t adc_buf[1];

static const ADCConfig adccfg = {
  .difsel = 0U
};

static const ADCConversionGroup adcgrpcfg = {
  .circular     = TRUE,
  .num_channels = 1,
  .end_cb       = NULL,
  .error_cb     = NULL,
  .cfgr         = ADC_CFGR_RES_12BITS,
  .cfgr2        = 0U,
  .ccr          = 0U,
  .pcsel        = ADC_SELMASK_IN7,   // PA7 = ADC1_INP7
  .ltr1         = 0U,
  .htr1         = 0x03FFFFFFU,
  .ltr2         = 0U,
  .htr2         = 0U,
  .ltr3         = 0U,
  .htr3         = 0U,
  .awd2cr       = 0U,
  .awd3cr       = 0U,
  .smpr         = {
    ADC_SMPR1_SMP_AN7(ADC_SMPR_SMP_810P5),  // sample time long
    0
  },
  .sqr          = {
    ADC_SQR1_SQ1_N(ADC_CHANNEL_IN7),
    0,0,0
  }
};

/* ================= MAIN ================= */

int main(void) {

  halInit();
  chSysInit();

  drivers_init_all();
  drv_display_init();

  /* PA7 analog */
  palSetPadMode(GPIOA, 7, PAL_MODE_INPUT_ANALOG);

  /* Start ADC in continuous mode */
  adcStart(&ADCD1, &adccfg);
  adcStartConversion(&ADCD1, &adcgrpcfg, adc_buf, 1);

  char line[32];

  while (true) {

    uint16_t v = adc_buf[0];

    drv_display_clear();
    drv_display_draw_text(0, 0, "ADC1 PA7 TEST");
    snprintf(line, sizeof(line), "VAL = %u", v);
    drv_display_draw_text(0, 12, line);
    drv_display_update();

    chThdSleepMilliseconds(200);
  }
}
