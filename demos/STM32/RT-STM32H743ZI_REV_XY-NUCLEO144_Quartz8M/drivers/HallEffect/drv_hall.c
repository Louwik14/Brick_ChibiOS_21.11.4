#include "drv_hall.h"

#include "ch.h"
#include "hal.h"

#include <limits.h>

#define ADC_NUM_CHANNELS        2
#define ADC_DMA_DEPTH           1

#define HALL_SENSOR_COUNT       16U
/* Paramètres réglables (échelle ADC 16-bit). */
#define HALL_RAW_REST           36000U
#define HALL_RAW_PRESSED        46000U
#define HALL_ON_THRESHOLD       40000U
#define HALL_HYSTERESIS         1500U
#define HALL_RETRIGGER_DELTA    900U
#define HALL_VELOCITY_RATE_MIN  500U
#define HALL_VELOCITY_RATE_MAX  200000U
#define HALL_TRIGGER_RATE       2500U

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
static systime_t hall_prev_times[HALL_SENSOR_COUNT];
static bool hall_gate[HALL_SENSOR_COUNT];
static bool hall_armed[HALL_SENSOR_COUNT];
static bool hall_note_on[HALL_SENSOR_COUNT];
static bool hall_note_off[HALL_SENSOR_COUNT];
static uint8_t hall_velocity[HALL_SENSOR_COUNT];
static uint8_t hall_pressure[HALL_SENSOR_COUNT];
static uint8_t hall_midi_value[HALL_SENSOR_COUNT];
static int16_t hall_offsets[HALL_SENSOR_COUNT] = {0};
static bool hall_initialized;
static uint8_t hall_mux_index;
static uint16_t hall_dbg_adjusted[HALL_SENSOR_COUNT];
static uint16_t hall_dbg_on_threshold[HALL_SENSOR_COUNT];
static uint16_t hall_dbg_off_threshold[HALL_SENSOR_COUNT];
static uint16_t hall_dbg_retrigger_threshold[HALL_SENSOR_COUNT];
static uint32_t hall_dbg_rate[HALL_SENSOR_COUNT];
static bool hall_dbg_armed[HALL_SENSOR_COUNT];
static bool hall_dbg_gate[HALL_SENSOR_COUNT];

static void mux_select(uint8_t ch);
static void hall_process_channel(uint8_t index, uint16_t raw, systime_t now);

/*
 * "Timer clock in Hz." and "TIM CR2 register initialization data."
 * (os/hal/ports/STM32/LLD/TIMv1/hal_gpt_lld.h).
 * "MMS = 010 = TRGO on Update Event." (testhal/STM32/multi/ADC/cfg/stm32h743zi_nucleo144/portab.c)
 */
static const GPTConfig hall_gptcfg = {
  .frequency    = STM32_TIMCLK1,
  .callback     = NULL,
  .cr2          = TIM_CR2_MMS_1,
  .dier         = 0U
};

static const gptcnt_t hall_gpt_interval = (STM32_TIMCLK1 / 20000U);

static void adc_cb(ADCDriver *adcp) {
  (void)adcp;
  uint16_t vA = adc_buffer[0U];
  uint16_t vB = adc_buffer[1U];
  uint8_t mux_ch = hall_mux_index;

  hall_values[mux_ch + 0U] = vA;
  hall_values[mux_ch + 8U] = vB;

  /* "This function can be called from any context" (os/rt/include/chvt.h). */
  systime_t now = chVTGetSystemTimeX();
  hall_process_channel(mux_ch + 0U, vA, now);
  hall_process_channel(mux_ch + 8U, vB, now);

  hall_mux_index = (uint8_t)((mux_ch + 1U) & 0x7U);
  mux_select(hall_mux_index);
}

static const ADCConversionGroup adcgrpcfg = {
  .circular     = true,
  .num_channels = ADC_NUM_CHANNELS,
  .end_cb       = adc_cb,
  .error_cb     = NULL,

  /*
   * "Callback function associated to the group." (os/hal/include/hal_adc.h)
   * "NOTE: The bits ADC_CFGR_CONT or ADC_CFGR_DISCEN must be specified
   * in continuous mode or if the buffer depth is greater than one."
   * (os/hal/ports/STM32/LLD/ADCv4/hal_adc_lld.h)
   * "If circular buffer depth > 1, then the half transfer interrupt is
   * enabled in order to allow streaming processing."
   * (os/hal/ports/STM32/LLD/ADCv4/hal_adc_lld.c)
   */
  .cfgr         = ADC_CFGR_EXTEN_RISING |
                  ADC_CFGR_EXTSEL_SRC(12), /* "TIM4_TRGO" (testhal/.../portab.c) */
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
    ADC_SMPR1_SMP_AN4(ADC_SMPR_SMP_64P5) |
    ADC_SMPR1_SMP_AN7(ADC_SMPR_SMP_64P5),
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

static uint32_t hall_compute_rate(uint16_t prev, uint16_t current, systime_t prev_time, systime_t now) {
  uint16_t delta = (current > prev) ? (uint16_t)(current - prev) : 0U;
  systime_t diff = chTimeDiffX(prev_time, now);
  uint32_t elapsed_us = TIME_I2US(diff);
  if (elapsed_us == 0U) {
    elapsed_us = 1U;
  }
  uint64_t scaled = (uint64_t)delta * 1000000ULL;
  return (uint32_t)(scaled / elapsed_us);
}

static uint8_t hall_velocity_from_rate(uint32_t rate) {
  if (rate <= HALL_VELOCITY_RATE_MIN) {
    return 1U;
  }
  if (rate >= HALL_VELOCITY_RATE_MAX) {
    return 127U;
  }
  uint32_t span = HALL_VELOCITY_RATE_MAX - HALL_VELOCITY_RATE_MIN;
  uint32_t scaled = (rate - HALL_VELOCITY_RATE_MIN) * 126U / span;
  return (uint8_t)(scaled + 1U);
}

static void hall_process_channel(uint8_t index, uint16_t raw, systime_t now) {
  int16_t offset = hall_offsets[index];
  uint16_t adjusted = hall_apply_offset(raw, offset);
  uint16_t prev = hall_prev_values[index];
  systime_t prev_time = hall_prev_times[index];
  if (prev_time == 0U) {
    prev_time = now;
  }
  uint32_t rate = hall_compute_rate(prev, adjusted, prev_time, now);
  hall_prev_values[index] = adjusted;
  hall_prev_times[index] = now;

  uint16_t min_value = clamp_u16((int32_t)HALL_RAW_REST + offset);
  uint16_t max_value = clamp_u16((int32_t)HALL_RAW_PRESSED + offset);
  if (max_value <= min_value) {
    max_value = (uint16_t)(min_value + 1U);
  }
  uint16_t on_threshold = clamp_u16((int32_t)HALL_ON_THRESHOLD + offset);
  if (on_threshold <= min_value) {
    on_threshold = (uint16_t)(min_value + 1U);
  } else if (on_threshold >= max_value) {
    on_threshold = (uint16_t)(max_value - 1U);
  }

  uint16_t off_threshold = on_threshold;
  if (off_threshold > min_value + HALL_HYSTERESIS) {
    off_threshold = (uint16_t)(off_threshold - HALL_HYSTERESIS);
  } else {
    off_threshold = min_value;
  }

  uint16_t retrigger_threshold = on_threshold;
  if (retrigger_threshold > min_value + HALL_RETRIGGER_DELTA) {
    retrigger_threshold = (uint16_t)(retrigger_threshold - HALL_RETRIGGER_DELTA);
  } else {
    retrigger_threshold = min_value;
  }

  hall_midi_value[index] = hall_map_to_midi(adjusted, min_value, max_value);

  hall_dbg_adjusted[index] = adjusted;
  hall_dbg_on_threshold[index] = on_threshold;
  hall_dbg_off_threshold[index] = off_threshold;
  hall_dbg_retrigger_threshold[index] = retrigger_threshold;
  hall_dbg_rate[index] = rate;

  bool fast_rise = rate >= HALL_TRIGGER_RATE;
  if (!hall_gate[index] && (adjusted <= retrigger_threshold)) {
    hall_armed[index] = true;
  }

  if (!hall_gate[index] && hall_armed[index] &&
      (adjusted >= on_threshold || (fast_rise && adjusted >= retrigger_threshold))) {
    hall_gate[index] = true;
    hall_note_on[index] = true;
    hall_velocity[index] = hall_velocity_from_rate(rate);
    hall_armed[index] = false;
  }

  if (hall_gate[index] && (adjusted <= off_threshold)) {
    hall_gate[index] = false;
    hall_note_off[index] = true;
  }

  if (hall_gate[index]) {
    hall_pressure[index] = hall_map_to_midi(adjusted, min_value, max_value);
  } else {
    hall_pressure[index] = 0U;
  }

  hall_dbg_armed[index] = hall_armed[index];
  hall_dbg_gate[index] = hall_gate[index];
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
    hall_prev_times[i] = chVTGetSystemTimeX();
    hall_gate[i] = false;
    hall_armed[i] = true;
    hall_note_on[i] = false;
    hall_note_off[i] = false;
    hall_velocity[i] = 0U;
    hall_pressure[i] = 0U;
    hall_midi_value[i] = 0U;
  }

  hall_mux_index = 0U;
  mux_select(hall_mux_index);

  gptStart(&GPTD4, &hall_gptcfg);

  adcStart(&ADCD1, NULL);
  adcStartConversion(&ADCD1, &adcgrpcfg, adc_buffer, ADC_DMA_DEPTH);

  gptStartContinuous(&GPTD4, hall_gpt_interval);

  hall_initialized = true;
}

void hall_update(void) {
  for (uint8_t i = 0; i < HALL_SENSOR_COUNT; i++) {
    hall_note_on[i] = false;
    hall_note_off[i] = false;
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
