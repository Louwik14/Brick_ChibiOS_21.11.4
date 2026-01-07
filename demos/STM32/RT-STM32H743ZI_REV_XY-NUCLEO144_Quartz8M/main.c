#include "ch.h"
#include "hal.h"
#include "drivers/drivers.h"
#include "drv_display.h"
#include <stdio.h>

// ================= CONFIG =================

#define ADC_NUM_CHANNELS        2
#define ADC_DMA_DEPTH          16   // nombre de paires stockées

// MUX select pins
#define MUX_S0_PORT GPIOA
#define MUX_S0_PIN  5
#define MUX_S1_PORT GPIOA
#define MUX_S1_PIN  4
#define MUX_S2_PORT GPIOA
#define MUX_S2_PIN  6

// Analog inputs
// PC4 = ADC1_INP4
// PA7 = ADC1_INP7

#define ADC_CH_MUXA   ADC_CHANNEL_IN4
#define ADC_CH_MUXB   ADC_CHANNEL_IN7

#define ADC_PCSEL     (ADC_SELMASK_IN4 | ADC_SELMASK_IN7)

// ================= DMA BUFFER =================

// Non-cacheable RAM
__attribute__((section(".ramd2")))
static adcsample_t adc_buffer[ADC_DMA_DEPTH * ADC_NUM_CHANNELS];

// ================= ADC CALLBACK =================

static void adc_cb(ADCDriver *adcp) {
  (void)adcp;
}

// ================= ADC CONFIG =================

static const ADCConversionGroup adcgrpcfg = {
  .circular     = true,
  .num_channels = ADC_NUM_CHANNELS,
  .end_cb       = adc_cb,
  .error_cb     = NULL,

  .cfgr         = ADC_CFGR_CONT,   // CONTINUOUS !!
  .cfgr2        = 0,

  .ltr1         = 0,
  .htr1         = 0,
  .ltr2         = 0,
  .htr2         = 0,
  .ltr3         = 0,
  .htr3         = 0,

  .awd2cr       = 0,
  .awd3cr       = 0,

  .pcsel        = ADC_PCSEL,

  .smpr         = {
    ADC_SMPR1_SMP_AN4(ADC_SMPR_SMP_384P5) |
    ADC_SMPR1_SMP_AN7(ADC_SMPR_SMP_384P5),
    0
  },

  .sqr          = {
    ADC_SQR1_SQ1_N(ADC_CH_MUXA) |
    ADC_SQR1_SQ2_N(ADC_CH_MUXB),
    0,
    0,
    0
  }
};

// ================= MUX =================

static void muxSelect(uint8_t ch) {
  palWritePad(MUX_S0_PORT, MUX_S0_PIN, (ch >> 0) & 1);
  palWritePad(MUX_S1_PORT, MUX_S1_PIN, (ch >> 1) & 1);
  palWritePad(MUX_S2_PORT, MUX_S2_PIN, (ch >> 2) & 1);
}

// ================= GET LAST SAMPLE =================

static void get_last_samples(uint16_t *a, uint16_t *b) {
  size_t idx = (ADC_DMA_DEPTH - 1) * ADC_NUM_CHANNELS;
  *a = adc_buffer[idx + 0];
  *b = adc_buffer[idx + 1];
}

// ================= HALL VALUES =================

static uint16_t hall_values[16];

// ================= MAIN =================

int main(void) {

  halInit();
  chSysInit();

  drivers_init_all();
  drv_display_init();

  // GPIO MUX
  palSetPadMode(GPIOA, 4, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPadMode(GPIOA, 5, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPadMode(GPIOA, 6, PAL_MODE_OUTPUT_PUSHPULL);

  // ADC pins
  palSetPadMode(GPIOC, 4, PAL_MODE_INPUT_ANALOG);
  palSetPadMode(GPIOA, 7, PAL_MODE_INPUT_ANALOG);

  // Init buffer sentinel
  for (unsigned i = 0; i < ADC_DMA_DEPTH * ADC_NUM_CHANNELS; i++)
    adc_buffer[i] = 0x1234;

  // Start ADC
  adcStart(&ADCD1, NULL);
  adcStartConversion(&ADCD1, &adcgrpcfg, adc_buffer, ADC_DMA_DEPTH);

  char line[32];

  while (true) {

    // -------- SCAN DES 8 POSITIONS --------
    for (uint8_t mux_ch = 0; mux_ch < 8; mux_ch++) {

      muxSelect(mux_ch);
      chThdSleepMilliseconds(2); // stabilisation analogique

      uint16_t vA, vB;
      get_last_samples(&vA, &vB);

      hall_values[mux_ch + 0] = vA;   // capteurs 1..8
      hall_values[mux_ch + 8] = vB;   // capteurs 9..16
    }

    // -------- AFFICHAGE --------

    drv_display_clear();
    drv_display_draw_text(0, 0, "HALL SENSORS:");

    // Affiche 8 lignes, 2 capteurs par ligne
    for (uint8_t i = 0; i < 8; i++) {
      snprintf(line, sizeof(line),
               "B%-2u:%5u  B%-2u:%5u",
               i + 1, hall_values[i],
               i + 9, hall_values[i + 8]);

      drv_display_draw_text(0, 12 + i * 8, line);
    }

    drv_display_update();

    chThdSleepMilliseconds(100);
  }
}
