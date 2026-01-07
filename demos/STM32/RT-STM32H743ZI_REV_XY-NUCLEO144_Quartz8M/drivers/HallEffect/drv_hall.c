#include "drv_hall.h"

#include "ch.h"
#include "hal.h"

#define ADC_NUM_CHANNELS        2
#define ADC_DMA_DEPTH          16

#define MUX_S0_PORT GPIOA
#define MUX_S0_PIN  5
#define MUX_S1_PORT GPIOA
#define MUX_S1_PIN  4
#define MUX_S2_PORT GPIOA
#define MUX_S2_PIN  6

#define ADC_CH_MUXA   ADC_CHANNEL_IN4
#define ADC_CH_MUXB   ADC_CHANNEL_IN7

#define ADC_PCSEL     (ADC_SELMASK_IN4 | ADC_SELMASK_IN7)

__attribute__((section(".ramd2")))
static adcsample_t adc_buffer[ADC_DMA_DEPTH * ADC_NUM_CHANNELS];

static uint16_t hall_values[16];
static bool hall_initialized;

static void adc_cb(ADCDriver *adcp) {
  (void)adcp;
}

static const ADCConversionGroup adcgrpcfg = {
  .circular     = true,
  .num_channels = ADC_NUM_CHANNELS,
  .end_cb       = adc_cb,
  .error_cb     = NULL,

  .cfgr         = ADC_CFGR_CONT,
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

static void mux_select(uint8_t ch) {
  palWritePad(MUX_S0_PORT, MUX_S0_PIN, (ch >> 0) & 1U);
  palWritePad(MUX_S1_PORT, MUX_S1_PIN, (ch >> 1) & 1U);
  palWritePad(MUX_S2_PORT, MUX_S2_PIN, (ch >> 2) & 1U);
}

static void get_last_samples(uint16_t *a, uint16_t *b) {
  size_t idx = (ADC_DMA_DEPTH - 1U) * ADC_NUM_CHANNELS;
  *a = adc_buffer[idx + 0U];
  *b = adc_buffer[idx + 1U];
}

void hall_init(void) {
  if (hall_initialized) {
    return;
  }

  palSetPadMode(GPIOA, 4, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPadMode(GPIOA, 5, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPadMode(GPIOA, 6, PAL_MODE_OUTPUT_PUSHPULL);

  palSetPadMode(GPIOC, 4, PAL_MODE_INPUT_ANALOG);
  palSetPadMode(GPIOA, 7, PAL_MODE_INPUT_ANALOG);

  for (unsigned i = 0; i < ADC_DMA_DEPTH * ADC_NUM_CHANNELS; i++) {
    adc_buffer[i] = 0x1234;
  }

  adcStart(&ADCD1, NULL);
  adcStartConversion(&ADCD1, &adcgrpcfg, adc_buffer, ADC_DMA_DEPTH);

  hall_initialized = true;
}

void hall_update(void) {
  for (uint8_t mux_ch = 0; mux_ch < 8U; mux_ch++) {
    mux_select(mux_ch);
    chThdSleepMilliseconds(2);

    uint16_t vA;
    uint16_t vB;
    get_last_samples(&vA, &vB);

    hall_values[mux_ch + 0U] = vA;
    hall_values[mux_ch + 8U] = vB;
  }
}

uint16_t hall_get(uint8_t index) {
  if (index >= 16U) {
    return 0;
  }
  return hall_values[index];
}
