#include "ch.h"
#include "hal.h"
#include "drivers/drivers.h"
#include "drv_display.h"
#include "drivers/HallEffect/drv_hall.h"
#include <stdio.h>

int main(void) {

  halInit();
  chSysInit();

  drivers_init_all();
  drv_display_init();
  hall_init();

  char line[32];

  while (true) {
    hall_update();

    drv_display_clear();
    drv_display_draw_text(0, 0, "HALL SENSORS:");

    for (uint8_t i = 0; i < 8; i++) {
      snprintf(line, sizeof(line),
               "B%-2u:%5u  B%-2u:%5u",
               i + 1, hall_get(i),
               i + 9, hall_get(i + 8));

      drv_display_draw_text(0, 12 + i * 8, line);
    }

    drv_display_update();

    chThdSleepMilliseconds(100);
  }
}
