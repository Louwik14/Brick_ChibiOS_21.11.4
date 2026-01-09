#include "drv_hall.h"

#include "ch.h"
#include "hal.h"

#include <limits.h>
#include <stdint.h>
#include <stdbool.h>

/*
 * drv_hall.c — STM32H743 + ChibiOS
 *
 * Acquisition:
 *   GPT (TIM4_TRGO) -> ADC (ext trigger) -> DMA circulaire -> adc_cb() IRQ
 *   adc_cb(): lit A/B, map index capteur, avance MUX, appelle hall_process_channel()
 *
 * Logique "Grid-like" (bouton auto-calibrant):
 *   - Min/Max auto-observés par capteur
 *   - Seuils Schmitt dynamiques exprimés en % de la plage [min..max]
 *   - NOTE ON à la montée (raw monte quand on appuie)
 *   - NOTE OFF à la descente
 *
 * Thread-safety:
 *   - Les flags note_on/off sont écrits en IRQ.
 *   - Le main les consomme via getters read+clear atomiques (chSysLock()).
 *   - hall_update() est un no-op pour éviter d'effacer des événements en course.
 */

/* -------------------- Paramètres -------------------- */

#define ADC_NUM_CHANNELS     2
#define ADC_DMA_DEPTH        1

#define HALL_SENSOR_COUNT    16U

/* Fréquence globale de déclenchement ADC. Chaque capteur est vu à F/8. */
#define HALL_ADC_TRIGGER_HZ  5000U

/* -------------------- Grid-like engine params (RAW 16-bit) --------------------
 *
 * threshold/hysteresis exprimés en millièmes de la plage [min..max].
 * Exemple: TH=700, HYST=120 -> ON à 76%, OFF à 64% (car hyst/2 = 60)
 *
 * Ajuste ces valeurs en fonction de ta mécanique (course utile, feel, etc).
 */
#define HALL_THRESHOLD_PPM    700U   /* 0..1000 */
#define HALL_HYST_PPM         120U   /* 0..1000 */
#define HALL_MIN_RANGE        500U   /* plage minimale requise avant de considérer la calibration "valide" */

/* MUX pins */
#define MUX_S0_PORT GPIOA
#define MUX_S0_PIN  5
#define MUX_S1_PORT GPIOA
#define MUX_S1_PIN  4
#define MUX_S2_PORT GPIOA
#define MUX_S2_PIN  6

/* ADC channels */
#define ADC_CH_MUXA   ADC_CHANNEL_IN4
#define ADC_CH_MUXB   ADC_CHANNEL_IN7
#define ADC_PCSEL     (ADC_SELMASK_IN4 | ADC_SELMASK_IN7)

/* -------------------- DMA buffer -------------------- */

__attribute__((section(".ramd2")))
static adcsample_t adc_buffer[ADC_DMA_DEPTH * ADC_NUM_CHANNELS];

/* -------------------- État capteurs -------------------- */

/* Valeur brute par capteur (RAW 16-bit) */
static volatile uint16_t hall_values[HALL_SENSOR_COUNT];

/* Flags impulsionnels écrits en IRQ, consommés par le main */
static volatile bool hall_note_on[HALL_SENSOR_COUNT];
static volatile bool hall_note_off[HALL_SENSOR_COUNT];

/* Stubs API (à implémenter plus tard) */
static volatile uint8_t hall_velocity[HALL_SENSOR_COUNT];
static volatile uint8_t hall_pressure[HALL_SENSOR_COUNT];
static volatile uint8_t hall_midi_value[HALL_SENSOR_COUNT];

static bool hall_initialized;
static uint8_t hall_mux_index;

/* -------------------- Grid-like button state (par capteur) -------------------- */

typedef struct {
  uint16_t min;
  uint16_t max;
  uint16_t trig_lo;
  uint16_t trig_hi;

  uint16_t prev_in;
  uint16_t curr_in;

  uint8_t  prev_out; /* 0=OFF, 1=ON */
  uint8_t  curr_out; /* 0=OFF, 1=ON */
  uint8_t locked;   // 0 = apprend, 1 = figé pendant appui
} hall_button_t;

static hall_button_t hall_btn[HALL_SENSOR_COUNT];

/* -------------------- Forward decl -------------------- */

static void mux_select(uint8_t ch);
static void hall_process_channel(uint8_t index, uint16_t raw);
static void hall_update_triggers(hall_button_t *b);
static bool hall_range_valid(const hall_button_t *b);

/* -------------------- GPT (TIM4_TRGO) -------------------- */

/*
 * TIM4 est utilisé comme source de trigger ADC (TRGO sur Update Event).
 * EXTSEL_SRC(12) correspond à TIM4_TRGO dans les configs STM32H7 ChibiOS.
 */
static const GPTConfig hall_gptcfg = {
  .frequency = STM32_TIMCLK1,
  .callback  = NULL,
  .cr2       = TIM_CR2_MMS_1, /* MMS=010: TRGO on Update */
  .dier      = 0U
};

static const gptcnt_t hall_gpt_interval = (STM32_TIMCLK1 / HALL_ADC_TRIGGER_HZ);

/* -------------------- ADC callback (IRQ context) -------------------- */

static void adc_cb(ADCDriver *adcp) {
  (void)adcp;

  /* Buffer depth = 1 => 2 samples: A puis B. */
  uint16_t vA = (uint16_t)adc_buffer[0U];
  uint16_t vB = (uint16_t)adc_buffer[1U];

  uint8_t mux = hall_mux_index;

  /* Stocke brut (debug/telemetry) */
  hall_values[mux + 0U] = vA;
  hall_values[mux + 8U] = vB;

  /* Moteur Grid-like par capteur */
  hall_process_channel((uint8_t)(mux + 0U), vA);
  hall_process_channel((uint8_t)(mux + 8U), vB);

  /* Next mux channel (0..7) */
  hall_mux_index = (uint8_t)((hall_mux_index + 1U) & 7U);
  mux_select(hall_mux_index);
}

/* -------------------- ADC config -------------------- */

static const ADCConversionGroup adcgrpcfg = {
  .circular     = true,
  .num_channels = ADC_NUM_CHANNELS,
  .end_cb       = adc_cb,
  .error_cb     = NULL,

  /* External trigger on rising edge: TIM4_TRGO */
  .cfgr  = ADC_CFGR_EXTEN_RISING | ADC_CFGR_EXTSEL_SRC(12),
  .cfgr2 = 0,

  .ltr1 = 0, .htr1 = 0,
  .ltr2 = 0, .htr2 = 0,
  .ltr3 = 0, .htr3 = 0,

  .awd2cr = 0,
  .awd3cr = 0,

  .pcsel = ADC_PCSEL,

  .smpr = {
    ADC_SMPR1_SMP_AN4(ADC_SMPR_SMP_64P5) |
    ADC_SMPR1_SMP_AN7(ADC_SMPR_SMP_64P5),
    0
  },

  .sqr = {
    ADC_SQR1_SQ1_N(ADC_CH_MUXA) |
    ADC_SQR1_SQ2_N(ADC_CH_MUXB),
    0, 0, 0
  }
};

/* -------------------- MUX -------------------- */

static void mux_select(uint8_t ch) {
  palWritePad(MUX_S0_PORT, MUX_S0_PIN, (ch >> 0) & 1U);
  palWritePad(MUX_S1_PORT, MUX_S1_PIN, (ch >> 1) & 1U);
  palWritePad(MUX_S2_PORT, MUX_S2_PIN, (ch >> 2) & 1U);
}

/* -------------------- Grid-like helpers -------------------- */

static bool hall_range_valid(const hall_button_t *b) {
  if (b->max <= b->min) {
    return false;
  }
  uint16_t range = (uint16_t)(b->max - b->min);
  return range >= (uint16_t)HALL_MIN_RANGE;
}

static void hall_update_triggers(hall_button_t *b) {
  /* Si plage invalide, on met des triggers "neutres" */
  if (!hall_range_valid(b)) {
    b->trig_lo = b->min;
    b->trig_hi = b->max;
    return;
  }

  uint32_t range = (uint32_t)(b->max - b->min);

  uint32_t half_hyst = (uint32_t)HALL_HYST_PPM / 2U;
  uint32_t lo_ppm = (uint32_t)HALL_THRESHOLD_PPM;
  uint32_t hi_ppm = (uint32_t)HALL_THRESHOLD_PPM;

  /* évite underflow/overflow */
  if (lo_ppm > half_hyst) {
    lo_ppm -= half_hyst;
  } else {
    lo_ppm = 0U;
  }

  hi_ppm += half_hyst;
  if (hi_ppm > 1000U) {
    hi_ppm = 1000U;
  }

  b->trig_lo = (uint16_t)(b->min + (range * lo_ppm) / 1000U);
  b->trig_hi = (uint16_t)(b->min + (range * hi_ppm) / 1000U);
}

/* -------------------- Logique Grid-like ON/OFF (IRQ context) --------------------
 *
 * Hypothèse hardware: raw MONTE quand on appuie.
 * -> NOTE ON quand raw dépasse trig_hi (montée)
 * -> NOTE OFF quand raw redescend sous trig_lo (descente)
 */
static void hall_process_channel(uint8_t index, uint16_t raw) {

  hall_button_t *b = &hall_btn[index];

  /* 1) update min/max auto-cal SEULEMENT SI NON LOCKÉ */
  if (b->locked == 0U) {

    if (raw < b->min) {
      b->min = raw;
    }
    if (raw > b->max) {
      b->max = raw;
    }
  }


  /* 2) recompute triggers */
  hall_update_triggers(b);

  /* 3) shift input history */
  b->prev_in = b->curr_in;
  b->curr_in = raw;

  /* 4) shift output history */
  b->prev_out = b->curr_out;

  /* 5) Tant que la plage n'est pas valide, on force OFF (pas d'événements) */
  if (!hall_range_valid(b)) {
    b->curr_out = 0;
    return;
  }

  /* 6) Schmitt trigger + direction check + LOCK */
  if ((b->prev_out == 0U) &&
      (b->curr_in >= b->trig_hi) &&
      (b->curr_in > b->prev_in)) {

    b->curr_out = 1U;
    b->locked = 1U;   /* 🔒 on fige la calibration pendant l'appui */
  }
  else if ((b->prev_out != 0U) &&
           (b->curr_in <= b->trig_lo) &&
           (b->curr_in < b->prev_in)) {

    b->curr_out = 0U;
    b->locked = 0U;   /* 🔓 on réautorise l'auto-cal au relâchement */
  }



  /* 7) Edge detect -> flags */
  if ((b->prev_out == 0U) && (b->curr_out == 1U)) {
    hall_note_on[index] = true;
  }
  else if ((b->prev_out == 1U) && (b->curr_out == 0U)) {
    hall_note_off[index] = true;
  }
}

/* -------------------- API -------------------- */

void hall_init(void) {
  if (hall_initialized) {
    return;
  }

  /* GPIO MUX */
  palSetPadMode(GPIOA, 4, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPadMode(GPIOA, 5, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPadMode(GPIOA, 6, PAL_MODE_OUTPUT_PUSHPULL);

  /* ADC pins */
  palSetPadMode(GPIOC, 4, PAL_MODE_INPUT_ANALOG); /* IN4 */
  palSetPadMode(GPIOA, 7, PAL_MODE_INPUT_ANALOG); /* IN7 */

  for (uint8_t i = 0U; i < HALL_SENSOR_COUNT; i++) {
    hall_values[i] = 0U;
    hall_note_on[i] = false;
    hall_note_off[i] = false;

    hall_velocity[i] = 0U;
    hall_pressure[i] = 0U;
    hall_midi_value[i] = 0U;

    hall_btn[i].min = UINT16_MAX;
    hall_btn[i].max = 0U;
    hall_btn[i].trig_lo = UINT16_MAX;
    hall_btn[i].trig_hi = 0U;
    hall_btn[i].prev_in = 0U;
    hall_btn[i].curr_in = 0U;
    hall_btn[i].prev_out = 0U;
    hall_btn[i].curr_out = 0U;
    hall_btn[i].locked = 0U;

  }

  hall_mux_index = 0U;
  mux_select(0U);

  gptStart(&GPTD4, &hall_gptcfg);

  adcStart(&ADCD1, NULL);
  adcStartConversion(&ADCD1, &adcgrpcfg, adc_buffer, ADC_DMA_DEPTH);

  gptStartContinuous(&GPTD4, hall_gpt_interval);

  hall_initialized = true;
}

/* Intentionnellement no-op:
 * Les événements sont consommés via getters read+clear atomiques.
 */
void hall_update(void) {
  /* no-op */
}

uint16_t hall_get(uint8_t index) {
  if (index >= HALL_SENSOR_COUNT) {
    return 0U;
  }
  return hall_values[index];
}

/* Read+clear atomique pour éviter perte d'événement si l'IRQ écrit pendant la lecture */
bool hall_get_note_on(uint8_t index) {
  if (index >= HALL_SENSOR_COUNT) {
    return false;
  }
  bool v;
  chSysLock();
  v = hall_note_on[index];
  hall_note_on[index] = false;
  chSysUnlock();
  return v;
}

bool hall_get_note_off(uint8_t index) {
  if (index >= HALL_SENSOR_COUNT) {
    return false;
  }
  bool v;
  chSysLock();
  v = hall_note_off[index];
  hall_note_off[index] = false;
  chSysUnlock();
  return v;
}

/* Stubs (compat header/main). */
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
/* -------------------- DEBUG API -------------------- */

uint16_t hall_dbg_get_min(uint8_t i) {
  if (i >= HALL_SENSOR_COUNT) return 0;
  return hall_btn[i].min;
}

uint16_t hall_dbg_get_max(uint8_t i) {
  if (i >= HALL_SENSOR_COUNT) return 0;
  return hall_btn[i].max;
}

uint16_t hall_dbg_get_trig_lo(uint8_t i) {
  if (i >= HALL_SENSOR_COUNT) return 0;
  return hall_btn[i].trig_lo;
}

uint16_t hall_dbg_get_trig_hi(uint8_t i) {
  if (i >= HALL_SENSOR_COUNT) return 0;
  return hall_btn[i].trig_hi;
}

uint8_t hall_dbg_get_state(uint8_t i) {
  if (i >= HALL_SENSOR_COUNT) return 0;
  return hall_btn[i].curr_out;
}
