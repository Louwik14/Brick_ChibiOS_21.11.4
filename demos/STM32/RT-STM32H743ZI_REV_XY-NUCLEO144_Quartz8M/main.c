#include "ch.h"
#include "hal.h"
#include "drivers/drivers.h"
#include "drv_display.h"
#include "drivers/HallEffect/drv_hall.h"
#include "midi/midi.h"
#include "usb/usb_device.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

int main(void) {

  halInit();
  chSysInit();

  drivers_init_all();
  drv_display_init();
  hall_init();
  usb_device_start();
  midi_init();

  char line[32];

  const uint8_t base_note = 60U;
  const uint8_t sensor_index = 3U;   /* <-- change la touche à observer ici */

  bool note_active[16];
  memset(note_active, 0, sizeof(note_active));

  while (true) {

    drv_display_clear();

    for (uint8_t i = 0U; i < 16U; i++) {

      /* Ces getters consomment (read+clear) les events de façon atomique */
      bool note_on  = hall_get_note_on(i);
      bool note_off = hall_get_note_off(i);
      uint16_t raw  = hall_get(i);

      uint8_t note_number = (uint8_t)(base_note + i);

      if (note_on) {
        note_active[i] = true;
        midi_note_on(MIDI_DEST_BOTH, 0U, note_number, 100U); /* velocity fixe */
      }

      if (note_off) {
        note_active[i] = false;
        midi_note_off(MIDI_DEST_BOTH, 0U, note_number, 0U);
      }

      if (i == sensor_index) {

        uint16_t min = hall_dbg_get_min(i);
        uint16_t max = hall_dbg_get_max(i);
        uint16_t lo  = hall_dbg_get_trig_lo(i);
        uint16_t hi  = hall_dbg_get_trig_hi(i);
        uint8_t  st  = hall_dbg_get_state(i);

        uint16_t range = (max >= min) ? (max - min) : 0;

        uint8_t y = 0;

        snprintf(line, sizeof(line), "RAW   = %5u", raw);
        drv_display_draw_text(0, y, line); y += 10;

        snprintf(line, sizeof(line), "MIN   = %5u", min);
        drv_display_draw_text(0, y, line); y += 10;

        snprintf(line, sizeof(line), "MAX   = %5u", max);
        drv_display_draw_text(0, y, line); y += 10;

        snprintf(line, sizeof(line), "RANGE = %5u", range);
        drv_display_draw_text(0, y, line); y += 10;

        snprintf(line, sizeof(line), "LO    = %5u", lo);
        drv_display_draw_text(0, y, line); y += 10;

        snprintf(line, sizeof(line), "HI    = %5u", hi);
        drv_display_draw_text(0, y, line); y += 10;

        snprintf(line, sizeof(line), "STATE = %s", st ? "ON " : "OFF");
        drv_display_draw_text(0, y, line); y += 10;

        snprintf(line, sizeof(line), "ON  = %d", note_on ? 1 : 0);
        drv_display_draw_text(0, y, line); y += 10;

        snprintf(line, sizeof(line), "OFF = %d", note_off ? 1 : 0);
        drv_display_draw_text(0, y, line); y += 10;
      }
    }

    drv_display_update();

    chThdSleepMilliseconds(10);
  }
}
