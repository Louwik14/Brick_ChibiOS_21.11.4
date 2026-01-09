#include "drv_encoders.h"
#include "ch.h"
#include "hal.h"
#include <stdint.h>
#include <stdbool.h>

/* ================== CONFIG GPIO (TES PINS) ================== */

typedef struct {
  ioportid_t portA;
  uint16_t   pinA;
  ioportid_t portB;
  uint16_t   pinB;
} enc_hw_t;

/* Mapping exact selon ce que tu m'as donné */
static const enc_hw_t enc_hw[ENCODER_COUNT] = {
  { GPIOB, 0,  GPIOB, 1  },   /* ENC1: PB0 / PB1 */
  { GPIOB, 10, GPIOB, 11 },   /* ENC2: PB10 / PB11 */
  { GPIOH, 10, GPIOH, 11 },   /* ENC3: PH10 / PH11 */
  { GPIOD, 12, GPIOD, 13 },   /* ENC4: PD12 / PD13 */
};

/* ================== ÉTAT ================== */

static int16_t enc_value[ENCODER_COUNT];
static uint8_t enc_prev_state[ENCODER_COUNT];
static bool enc_started = false;

/* Table de transition quadrature */
static const int8_t quad_table[16] = {
   0, -1, +1,  0,
  +1,  0,  0, -1,
  -1,  0,  0, +1,
   0, +1, -1,  0
};

/* ================== LOW LEVEL ================== */

static uint8_t enc_read_state(uint8_t i) {
  const enc_hw_t *h = &enc_hw[i];
  uint8_t a = palReadPad(h->portA, h->pinA) ? 1 : 0;
  uint8_t b = palReadPad(h->portB, h->pinB) ? 1 : 0;
  return (uint8_t)((a << 1) | b);
}

/* ================== API ================== */

void drv_encoders_start(void) {

  if (enc_started) return;

  for (int i = 0; i < ENCODER_COUNT; i++) {

    palSetPadMode(enc_hw[i].portA, enc_hw[i].pinA, PAL_MODE_INPUT_PULLUP);
    palSetPadMode(enc_hw[i].portB, enc_hw[i].pinB, PAL_MODE_INPUT_PULLUP);

    enc_value[i] = 0;
    enc_prev_state[i] = enc_read_state(i);
  }

  enc_started = true;
}

/* Poll interne */
static void enc_poll(uint8_t i) {
  uint8_t prev = enc_prev_state[i];
  uint8_t now  = enc_read_state(i);

  uint8_t idx = (uint8_t)((prev << 2) | now);
  int8_t delta = quad_table[idx];

  enc_value[i] += delta;
  enc_prev_state[i] = now;
}

int16_t drv_encoder_get(encoder_id_t id) {
  if (id >= ENCODER_COUNT) return 0;
  enc_poll(id);
  return enc_value[id];
}

int16_t drv_encoder_get_delta(encoder_id_t id) {
  static int16_t last[ENCODER_COUNT] = {0};

  if (id >= ENCODER_COUNT) return 0;

  enc_poll(id);

  int16_t now = enc_value[id];
  int16_t d = now - last[id];
  last[id] = now;
  return d;
}

int16_t drv_encoder_get_delta_accel(encoder_id_t id) {
  int16_t d = drv_encoder_get_delta(id);

  if (d > 5)       return d * 4;
  else if (d > 2)  return d * 2;
  else if (d < -5) return d * 4;
  else if (d < -2) return d * 2;
  else             return d;
}

void drv_encoder_reset(encoder_id_t id) {
  if (id >= ENCODER_COUNT) return;
  enc_value[id] = 0;
}
