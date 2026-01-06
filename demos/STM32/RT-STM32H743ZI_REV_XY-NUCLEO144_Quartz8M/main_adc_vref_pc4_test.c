#include "ch.h"
#include "hal.h"
#include "drivers/drivers.h"
#include "drv_display.h"
#include <stdio.h>

static const ADCConfig adccfg = {
  .difsel = 0U,
  .calibration = 0U,
};

static const ADCConversionGroup adcgrpcfg = {
  .circular = false,
  .num_channels = 2,
  .end_cb = NULL,
  .error_cb = NULL,
  .cfgr = ADC_CFGR_RES_16BITS,
  .cfgr2 = 0U,
  .ccr = ADC_CCR_VREFEN,
  .pcsel = ADC_SELMASK_IN18 | ADC_SELMASK_IN4,
  .ltr1 = 0U,
  .htr1 = 0U,
  .ltr2 = 0U,
  .htr2 = 0U,
  .ltr3 = 0U,
  .htr3 = 0U,
  .awd2cr = 0U,
  .awd3cr = 0U,
  .smpr = {
    ADC_SMPR1_SMP_AN4(ADC_SMPR_SMP_64P5),
    ADC_SMPR2_SMP_AN18(ADC_SMPR_SMP_64P5),
  },
  .sqr = {
    ADC_SQR1_SQ1_N(ADC_CHANNEL_IN18) | ADC_SQR1_SQ2_N(ADC_CHANNEL_IN4),
    0U,
    0U,
    0U
  }
};

int main(void) {
  halInit();
  chSysInit();

  drivers_init_all();
  drv_display_init();

  palSetPadMode(GPIOC, 4, PAL_MODE_INPUT_ANALOG);

  adcStart(&ADCD1, &adccfg);
  chThdSleepMilliseconds(2);

  adcsample_t samples[2];
  char line[32];

  while (true) {
    adcConvert(&ADCD1, &adcgrpcfg, samples, 1);

    drv_display_clear();
    snprintf(line, sizeof(line), "VREFINT: %u", (unsigned)samples[0]);
    drv_display_draw_text(0, 0, line);
    snprintf(line, sizeof(line), "PC4:     %u", (unsigned)samples[1]);
    drv_display_draw_text(0, 8, line);
    drv_display_update();

    chThdSleepMilliseconds(200);
  }
}
