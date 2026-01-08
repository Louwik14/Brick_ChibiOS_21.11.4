#include "ch.h"
#include "hal.h"
#include "drivers/drivers.h"
#include "drv_display.h"
#include "drivers/HallEffect/drv_hall.h"
#include "midi/midi.h"
#include "usb/usb_device.h"
#include <stdio.h>
#include <string.h>

/* Polices déclarées dans le driver display */
extern const font_t FONT_4X6;
extern const font_t FONT_5X7;

int main(void) {

  halInit();
  chSysInit();

  drivers_init_all();
  drv_display_init();
  hall_init();
  usb_device_start();
  midi_init();

  char line[32];
  const uint8_t sensor_index = 4U;
  const uint8_t base_note = 60U;
  const uint8_t debug_x = 86U;
  const uint8_t debug_y = 0U;
  const uint8_t debug_line_height = 8U;
  bool note_active[16];
  uint8_t last_pressure[16];

  memset(note_active, 0, sizeof(note_active));
  memset(last_pressure, 0, sizeof(last_pressure));

  while (true) {
    hall_update();

    drv_display_clear();

    /* Police normale */
    drv_display_set_font(&FONT_5X7);
    drv_display_draw_text(0, 0, "HALL B5 DEBUG");

    for (uint8_t i = 0U; i < 16U; i++) {
      bool note_on = hall_get_note_on(i);
      bool note_off = hall_get_note_off(i);
      uint8_t velocity = hall_get_velocity(i);
      uint8_t pressure = hall_get_pressure(i);
      uint8_t note_number = (uint8_t)(base_note + i);

      if (note_on) {
        note_active[i] = true;
        midi_note_on(MIDI_DEST_BOTH, 0U, note_number, velocity);
      }
      if (note_off) {
        note_active[i] = false;
        midi_note_off(MIDI_DEST_BOTH, 0U, note_number, 0U);
      }
      if (note_active[i]) {
        if (pressure != last_pressure[i]) {
          midi_poly_aftertouch(MIDI_DEST_BOTH, 0U, note_number, pressure);
          last_pressure[i] = pressure;
        }
      } else {
        last_pressure[i] = 0U;
      }
    }

    uint16_t raw = hall_get(sensor_index);
    uint8_t midi_value = hall_get_midi_value(sensor_index);
    uint8_t velocity = hall_get_velocity(sensor_index);
    uint8_t pressure = hall_get_pressure(sensor_index);

    snprintf(line, sizeof(line), "RAW = %4u", raw);
    drv_display_draw_text(0, 12, line);

    snprintf(line, sizeof(line), "MIDI = %3u", midi_value);
    drv_display_draw_text(0, 20, line);

    snprintf(line, sizeof(line), "ON/OFF = %s", note_active[sensor_index] ? "ON " : "OFF");
    drv_display_draw_text(0, 28, line);

    snprintf(line, sizeof(line), "VEL = %3u", velocity);
    drv_display_draw_text(0, 36, line);

    snprintf(line, sizeof(line), "PRES = %3u", pressure);
    drv_display_draw_text(0, 44, line);

    /* --- Colonne debug droite en petite police --- */
    drv_display_set_font(&FONT_4X6);

    snprintf(line, sizeof(line), "MUX:%u", hall_debug_get_mux_index());
    drv_display_draw_text(debug_x, debug_y, line);

    snprintf(line, sizeof(line), "H0:%5u", hall_get(0));
    drv_display_draw_text(debug_x, debug_y + debug_line_height, line);

    snprintf(line, sizeof(line), "H1:%5u", hall_get(1));
    drv_display_draw_text(debug_x, debug_y + 2 * debug_line_height, line);

    snprintf(line, sizeof(line), "H8:%5u", hall_get(8));
    drv_display_draw_text(debug_x, debug_y + 3 * debug_line_height, line);

    snprintf(line, sizeof(line), "H9:%5u", hall_get(9));
    drv_display_draw_text(debug_x, debug_y + 4 * debug_line_height, line);

    /* Retour police normale */
    drv_display_set_font(&FONT_5X7);

    drv_display_update();

    chThdSleepMilliseconds(100);
  }
}
