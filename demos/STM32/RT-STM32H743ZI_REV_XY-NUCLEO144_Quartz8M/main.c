#include "ch.h"
#include "hal.h"
#include "drivers/drivers.h"
#include "drv_display.h"
#include <stdio.h>

/* ================= ADC ================= */

#define MUX_S0_LINE LINE_MXH_S0
#define MUX_S1_LINE LINE_MXH_S1
#define MUX_S2_LINE LINE_MXH_S2

#define ADC_MUX_CHANNEL         0U
#define ADC_EXTERNAL_CHANNEL    ADC_CHANNEL_IN7
#define ADC_EXTERNAL_SEL        ADC_SELMASK_IN7

static adcsample_t adc_buf[1] __attribute__((section(".nocache"), aligned(32)));

static const ADCConfig adccfg = {
  .difsel = 0U,
};

static const ADCConversionGroup adcgrpcfg = {
  .circular     = TRUE,
  .num_channels = 1,
  .end_cb       = NULL,
  .error_cb     = NULL,
  .cfgr         = ADC_CFGR_RES_12BITS | ADC_CFGR_CONT_ENABLED,
  .cfgr2        = 0U,
  .ccr          = 0U,
  .pcsel        = ADC_EXTERNAL_SEL,
  .ltr1         = 0U,
  .htr1         = 0x03FFFFFFU,
  .ltr2         = 0U,
  .htr2         = 0U,
  .ltr3         = 0U,
  .htr3         = 0U,
  .awd2cr       = 0U,
  .awd3cr       = 0U,
  .smpr         = {
    ADC_SMPR1_SMP_AN7(ADC_SMPR_SMP_64P5),
    0U
  },
  .sqr          = {
    ADC_SQR1_SQ1_N(ADC_EXTERNAL_CHANNEL),
    0U, 0U, 0U
  }
};

static void mux_set(uint8_t sel) {
  palWriteLine(MUX_S0_LINE, (sel & 0x01U) ? PAL_HIGH : PAL_LOW);
  palWriteLine(MUX_S1_LINE, (sel & 0x02U) ? PAL_HIGH : PAL_LOW);
  palWriteLine(MUX_S2_LINE, (sel & 0x04U) ? PAL_HIGH : PAL_LOW);
}

/* ================= MAIN ================= */

int main(void) {

  halInit();
  chSysInit();

  drivers_init_all();
  drv_display_init();

  palSetLineMode(MUX_S0_LINE, PAL_MODE_OUTPUT_PUSHPULL);
  palSetLineMode(MUX_S1_LINE, PAL_MODE_OUTPUT_PUSHPULL);
  palSetLineMode(MUX_S2_LINE, PAL_MODE_OUTPUT_PUSHPULL);
  mux_set(ADC_MUX_CHANNEL);

  palSetPadMode(GPIOA, 7, PAL_MODE_INPUT_ANALOG);

  /* Start ADC in continuous mode (DMA circular). */
  adcStart(&ADCD1, &adccfg);
  adcStartConversion(&ADCD1, &adcgrpcfg, adc_buf, 1);

  char line[32];

  while (true) {

    uint16_t v = adc_buf[0];

    drv_display_clear();
    drv_display_draw_text(0, 0, "ADC1 MUX PA7 DMA");
    snprintf(line, sizeof(line), "MUX CH = %u", (unsigned)ADC_MUX_CHANNEL);
    drv_display_draw_text(0, 10, line);
    snprintf(line, sizeof(line), "VAL = %u", v);
    drv_display_draw_text(0, 22, line);
    drv_display_update();

    chThdSleepMilliseconds(200);
  }
}
