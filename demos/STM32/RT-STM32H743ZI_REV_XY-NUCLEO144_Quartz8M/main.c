#include "ch.h"
#include "hal.h"
#include "drivers/drivers.h"
#include "drv_display.h"
#include "drivers/HallEffect/drv_hall.h"
#include "drv_encoders.h"
#include "midi/midi.h"
#include "usb/usb_device.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

/* Helpers affichage enums */

static const char *vel_mode_str[] = {
  "PEAK",
  "TIME",
  "ENERGY"
};

static const char *vel_curve_str[] = {
  "LIN",
  "SOFT",
  "HARD",
  "LOG",
  "EXP"
};

#define LOCAL_VEL_MODE_COUNT   (sizeof(vel_mode_str) / sizeof(vel_mode_str[0]))
#define LOCAL_VEL_CURVE_COUNT  (sizeof(vel_curve_str) / sizeof(vel_curve_str[0]))

int main(void) {

  halInit();
  chSysInit();

  drivers_init_all();
  drv_display_init();
  drv_encoders_start();
  hall_init();
  usb_device_start();
  midi_init();

  char line[32];

  const uint8_t base_note = 60U;
  const uint8_t sensor_index = 3U;   /* touche à observer */

  bool note_active[16];
  memset(note_active, 0, sizeof(note_active));

  uint8_t last_velocity = 0;

  while (true) {

    /* ----------- Lecture encodeurs ----------- */

    int d0 = drv_encoder_get_delta_accel(ENC3) ;
    int d1 = drv_encoder_get_delta_accel(ENC4) ;


    if (d0 != 0) {
      int mode = (int)hall_get_velocity_mode();
      mode += d0;

      if (mode < 0) mode = 0;
      if (mode >= (int)LOCAL_VEL_MODE_COUNT) mode = (int)LOCAL_VEL_MODE_COUNT - 1;

      hall_set_velocity_mode((uint8_t)mode);
    }

    if (d1 != 0) {
      int curve = (int)hall_get_velocity_curve();
      curve += d1;

      if (curve < 0) curve = 0;
      if (curve >= (int)LOCAL_VEL_CURVE_COUNT) curve = (int)LOCAL_VEL_CURVE_COUNT - 1;

      hall_set_velocity_curve((uint8_t)curve);
    }

    /* ----------- MIDI + état touches ----------- */

    for (uint8_t i = 0U; i < 16U; i++) {

      bool note_on  = hall_get_note_on(i);
      bool note_off = hall_get_note_off(i);

      uint8_t note_number = (uint8_t)(base_note + i);

      if (note_on) {
        uint8_t vel = hall_get_velocity(i);
        if (vel == 0) vel = 1;

        note_active[i] = true;
        if (i == sensor_index) {
          last_velocity = vel;
        }

        midi_note_on(MIDI_DEST_BOTH, 0U, note_number, vel);
      }

      if (note_off) {
        note_active[i] = false;
        midi_note_off(MIDI_DEST_BOTH, 0U, note_number, 0U);
      }
    }

    /* ----------- OLED ----------- */

    drv_display_clear();

    snprintf(line, sizeof(line), "STATE = %s", note_active[sensor_index] ? "ON " : "OFF");
    drv_display_draw_text(0, 0, line);

    snprintf(line, sizeof(line), "VEL   = %3u", last_velocity);
    drv_display_draw_text(0, 12, line);

    uint8_t mode  = hall_get_velocity_mode();
    uint8_t curve = hall_get_velocity_curve();

    /* Sécurité anti-crash */
    if (mode >= LOCAL_VEL_MODE_COUNT)  mode = 0;
    if (curve >= LOCAL_VEL_CURVE_COUNT) curve = 0;

    snprintf(line, sizeof(line), "MODE  = %s", vel_mode_str[mode]);
    drv_display_draw_text(0, 24, line);

    snprintf(line, sizeof(line), "CURVE = %s", vel_curve_str[curve]);
    drv_display_draw_text(0, 36, line);

    drv_display_update();

    chThdSleepMilliseconds(10);
  }
}
