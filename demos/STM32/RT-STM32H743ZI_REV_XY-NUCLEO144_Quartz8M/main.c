#include "ch.h"
#include "hal.h"
#include "drivers/drivers.h"
#include "drv_display.h"
#include <stdio.h>

#define MUX_S0_LINE LINE_MXH_S0
#define MUX_S1_LINE LINE_MXH_S1
#define MUX_S2_LINE LINE_MXH_S2

static const ADCConfig adccfg = {
  .difsel = 0U,
  .calibration = 0U,
};

static const ADCConversionGroup adcgrpcfg = {
  .circular = false,
  .num_channels = 3,
  .end_cb = NULL,
  .error_cb = NULL,
  .cfgr = ADC_CFGR_RES_16BITS,
  .cfgr2 = 0U,
  .ccr = 0U,
  .pcsel = ADC_SELMASK_IN4 | ADC_SELMASK_IN7 | ADC_SELMASK_IN9,
  .ltr1 = 0U,
  .htr1 = 0U,
  .ltr2 = 0U,
  .htr2 = 0U,
  .ltr3 = 0U,
  .htr3 = 0U,
  .awd2cr = 0U,
  .awd3cr = 0U,
  .smpr = {
    ADC_SMPR1_SMP_AN4(ADC_SMPR_SMP_810P5) |
    ADC_SMPR1_SMP_AN7(ADC_SMPR_SMP_810P5) |
    ADC_SMPR1_SMP_AN9(ADC_SMPR_SMP_810P5),
    0U
  },
  .sqr = {
    ADC_SQR1_SQ1_N(ADC_CHANNEL_IN4) |
    ADC_SQR1_SQ2_N(ADC_CHANNEL_IN7) |
    ADC_SQR1_SQ3_N(ADC_CHANNEL_IN9),
    0U,
    0U,
    0U
  }
};

static void mux_set(uint8_t sel) {
  palWriteLine(MUX_S0_LINE, (sel & 0x01U) ? PAL_HIGH : PAL_LOW);
  palWriteLine(MUX_S1_LINE, (sel & 0x02U) ? PAL_HIGH : PAL_LOW);
  palWriteLine(MUX_S2_LINE, (sel & 0x04U) ? PAL_HIGH : PAL_LOW);
}

static void adc_read_triplet(adcsample_t *dest) {
  adcsample_t dummy[3];

  adcConvert(&ADCD1, &adcgrpcfg, dummy, 1);
  chThdSleepMicroseconds(20);
  adcConvert(&ADCD1, &adcgrpcfg, dest, 1);
}

int main(void) {
  halInit();
  chSysInit();

  drivers_init_all();
  drv_display_init();

  palSetLineMode(MUX_S0_LINE, PAL_MODE_OUTPUT_PUSHPULL);
  palSetLineMode(MUX_S1_LINE, PAL_MODE_OUTPUT_PUSHPULL);
  palSetLineMode(MUX_S2_LINE, PAL_MODE_OUTPUT_PUSHPULL);

  palSetPadMode(GPIOC, 4, PAL_MODE_INPUT_ANALOG);
  palSetPadMode(GPIOA, 7, PAL_MODE_INPUT_ANALOG);
  palSetPadMode(GPIOB, 1, PAL_MODE_INPUT_ANALOG);

  adcStart(&ADCD1, &adccfg);

  uint16_t values[8][3];
  char line[32];

  while (true) {
    for (uint8_t ch = 0; ch < 8U; ch++) {
      mux_set(ch);
      chThdSleepMicroseconds(10);
      adcsample_t samples[3];
      adc_read_triplet(samples);
      values[ch][0] = samples[0];
      values[ch][1] = samples[1];
      values[ch][2] = samples[2];
    }

    drv_display_clear();
    drv_display_draw_text(0, 0, "MUX PC4/PA7/PB1");
    for (uint8_t i = 0; i < 8U; i++) {
      snprintf(line, sizeof(line), "C%u:%5u %5u %5u",
               (unsigned)i,
               (unsigned)values[i][0],
               (unsigned)values[i][1],
               (unsigned)values[i][2]);
      drv_display_draw_text(0, 8U * (i + 1U), line);
    }
    drv_display_update();

    chThdSleepMilliseconds(200);
  }
}
