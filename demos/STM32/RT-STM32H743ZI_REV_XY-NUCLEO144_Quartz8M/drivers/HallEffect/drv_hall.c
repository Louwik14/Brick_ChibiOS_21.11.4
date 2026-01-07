#include "drv_hall.h"

#include "ch.h"
#include "hal.h"

#include <limits.h>

#define ADC_NUM_CHANNELS        2
#define ADC_DMA_DEPTH          16

#define HALL_SENSOR_COUNT      16U
#define HALL_RAW_REST          3600U
#define HALL_RAW_PRESSED       6400U
#define HALL_ON_THRESHOLD      4200U
#define HALL_OFF_THRESHOLD     4000U
#define HALL_VELOCITY_MAX_DELTA 2000U

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

static uint16_t hall_values[HALL_SENSOR_COUNT];
static uint16_t hall_prev_values[HALL_SENSOR_COUNT];
static bool hall_gate[HALL_SENSOR_COUNT];
static bool hall_note_on[HALL_SENSOR_COUNT];
static bool hall_note_off[HALL_SENSOR_COUNT];
static uint8_t hall_velocity[HALL_SENSOR_COUNT];
static uint8_t hall_pressure[HALL_SENSOR_COUNT];
static uint8_t hall_midi_value[HALL_SENSOR_COUNT];
static int16_t hall_offsets[HALL_SENSOR_COUNT] = {0};
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

static uint16_t clamp_u16(int32_t value) {
  if (value < 0) {
    return 0U;
  }
  if (value > UINT16_MAX) {
    return UINT16_MAX;
  }
  return (uint16_t)value;
}

static uint8_t clamp_u8(int32_t value) {
  if (value < 0) {
    return 0U;
  }
  if (value > UINT8_MAX) {
    return UINT8_MAX;
  }
  return (uint8_t)value;
}

static uint16_t hall_apply_offset(uint16_t raw, int16_t offset) {
  return clamp_u16((int32_t)raw + offset);
}

static uint8_t hall_map_to_midi(uint16_t value, uint16_t min, uint16_t max) {
  if (max <= min) {
    return 0U;
  }
  if (value <= min) {
    return 0U;
  }
  if (value >= max) {
    return 127U;
  }
  uint32_t span = (uint32_t)(max - min);
  uint32_t scaled = (uint32_t)(value - min) * 127U;
  return (uint8_t)(scaled / span);
}

static uint8_t hall_compute_velocity(uint16_t prev, uint16_t current) {
  uint16_t delta = (current > prev) ? (uint16_t)(current - prev) : 0U;
  uint32_t scaled = (uint32_t)delta * 127U / HALL_VELOCITY_MAX_DELTA;
  if (scaled == 0U) {
    scaled = 1U;
  }
  if (scaled > 127U) {
    scaled = 127U;
  }
  return (uint8_t)scaled;
}

static void hall_process_channel(uint8_t index, uint16_t raw) {
  int16_t offset = hall_offsets[index];
  uint16_t adjusted = hall_apply_offset(raw, offset);
  uint16_t prev = hall_prev_values[index];
  hall_prev_values[index] = adjusted;

  uint16_t on_threshold = clamp_u16((int32_t)HALL_ON_THRESHOLD + offset);
  uint16_t off_threshold = clamp_u16((int32_t)HALL_OFF_THRESHOLD + offset);
  uint16_t min_value = clamp_u16((int32_t)HALL_RAW_REST + offset);
  uint16_t max_value = clamp_u16((int32_t)HALL_RAW_PRESSED + offset);
  if (max_value <= min_value) {
    max_value = (uint16_t)(min_value + 1U);
  }

  hall_midi_value[index] = hall_map_to_midi(adjusted, min_value, max_value);

  if (!hall_gate[index]) {
    if (adjusted >= on_threshold) {
      hall_gate[index] = true;
      hall_note_on[index] = true;
      hall_velocity[index] = hall_compute_velocity(prev, adjusted);
    }
  } else {
    if (adjusted <= off_threshold) {
      hall_gate[index] = false;
      hall_note_off[index] = true;
    }
  }

  if (hall_gate[index]) {
    hall_pressure[index] = hall_map_to_midi(adjusted, min_value, max_value);
  } else {
    hall_pressure[index] = 0U;
  }
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

  for (uint8_t i = 0; i < HALL_SENSOR_COUNT; i++) {
    hall_values[i] = 0U;
    hall_prev_values[i] = 0U;
    hall_gate[i] = false;
    hall_note_on[i] = false;
    hall_note_off[i] = false;
    hall_velocity[i] = 0U;
    hall_pressure[i] = 0U;
    hall_midi_value[i] = 0U;
  }

  adcStart(&ADCD1, NULL);
  adcStartConversion(&ADCD1, &adcgrpcfg, adc_buffer, ADC_DMA_DEPTH);

  hall_initialized = true;
}

void hall_update(void) {
  for (uint8_t i = 0; i < HALL_SENSOR_COUNT; i++) {
    hall_note_on[i] = false;
    hall_note_off[i] = false;
  }

  for (uint8_t mux_ch = 0; mux_ch < 8U; mux_ch++) {
    mux_select(mux_ch);
    chThdSleepMilliseconds(2);

    uint16_t vA;
    uint16_t vB;
    get_last_samples(&vA, &vB);

    hall_values[mux_ch + 0U] = vA;
    hall_values[mux_ch + 8U] = vB;
    hall_process_channel(mux_ch + 0U, vA);
    hall_process_channel(mux_ch + 8U, vB);
  }
}

uint16_t hall_get(uint8_t index) {
  if (index >= HALL_SENSOR_COUNT) {
    return 0;
  }
  return hall_values[index];
}

bool hall_get_note_on(uint8_t index) {
  if (index >= HALL_SENSOR_COUNT) {
    return false;
  }
  return hall_note_on[index];
}

bool hall_get_note_off(uint8_t index) {
  if (index >= HALL_SENSOR_COUNT) {
    return false;
  }
  return hall_note_off[index];
}

uint8_t hall_get_velocity(uint8_t index) {
  if (index >= HALL_SENSOR_COUNT) {
    return 0U;
  }
  return hall_velocity[index];
}

uint8_t hall_get_pressure(uint8_t index) {
  if (index >= HALL_SENSOR_COUNT) {
    return 0U;
  }
  return hall_pressure[index];
}

uint8_t hall_get_midi_value(uint8_t index) {
  if (index >= HALL_SENSOR_COUNT) {
    return 0U;
  }
  return hall_midi_value[index];
}
