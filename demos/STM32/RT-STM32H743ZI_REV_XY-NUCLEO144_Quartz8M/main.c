#include "ch.h"
#include "hal.h"
#include "drivers/drivers.h"
#include "drv_display.h"
#include <stdio.h>

#define MUX_S0_LINE LINE_MXH_S0
#define MUX_S1_LINE LINE_MXH_S1
#define MUX_S2_LINE LINE_MXH_S2

#define ADC_MUX_CHANNEL ADC_CHANNEL_IN7

static const ADCConfig adccfg = {
  .difsel = 0U,
  .calibration = 0U,
};

static const ADCConversionGroup adcgrpcfg = {
  .circular = false,
  .num_channels = 1U,
  .end_cb = NULL,
  .error_cb = NULL,
  .cfgr = ADC_CFGR_RES_16BITS,
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
    ADC_SMPR1_SMP_AN7(ADC_SMPR_SMP_810P5),
    0U,
  },
  .sqr = {
    ADC_SQR1_SQ1_N(ADC_MUX_CHANNEL),
    0U,
    0U,
    0U,
  }
};

static void mux_set(uint8_t sel) {
  palWriteLine(MUX_S0_LINE, (sel & 0x01U) ? PAL_HIGH : PAL_LOW);
  palWriteLine(MUX_S1_LINE, (sel & 0x02U) ? PAL_HIGH : PAL_LOW);
  palWriteLine(MUX_S2_LINE, (sel & 0x04U) ? PAL_HIGH : PAL_LOW);
}

int main(void) {
  halInit();
  chSysInit();

  drivers_init_all();
  drv_display_init();

  palSetPadMode(GPIOA, 7, PAL_MODE_INPUT_ANALOG);
  palSetLineMode(MUX_S0_LINE, PAL_MODE_OUTPUT_PUSHPULL);
  palSetLineMode(MUX_S1_LINE, PAL_MODE_OUTPUT_PUSHPULL);
  palSetLineMode(MUX_S2_LINE, PAL_MODE_OUTPUT_PUSHPULL);

  adcStart(&ADCD1, &adccfg);
  chThdSleepMilliseconds(2);

  adcsample_t sample;
  char line1[32];
  char line2[32];

  while (true) {
    for (uint8_t ch = 0; ch < 8U; ch++) {
      mux_set(ch);
      chThdSleepMilliseconds(5);

      adcConvert(&ADCD1, &adcgrpcfg, &sample, 1);

      drv_display_clear();
      snprintf(line1, sizeof(line1), "MUX CH: %u", (unsigned)ch);
      drv_display_draw_text(0, 0, line1);
      snprintf(line2, sizeof(line2), "ADC: %u", (unsigned)sample);
      drv_display_draw_text(0, 12, line2);
      drv_display_update();

      chThdSleepMilliseconds(200);
    }
  }
}
