#include "ch.h"
#include "hal.h"
#include "drivers/drivers.h"
#include "drv_hall.h"
#include <stdio.h>

static THD_WORKING_AREA(waHallTask, 512);

static THD_FUNCTION(hallTask, arg) {
  (void)arg;
  chRegSetThreadName("HallTask");

  while (!chThdShouldTerminateX()) {
    drv_hall_task();
    chThdSleepMilliseconds(5);
  }

  chThdExit(MSG_OK);
}

static void draw_hall_states(void) {
  char line[16];

  drv_display_clear();

  for (uint8_t row = 0; row < 8; ++row) {
    uint8_t left = row;
    uint8_t right = (uint8_t)(row + 8);
    uint8_t y = (uint8_t)(row * 8);

    snprintf(line, sizeof(line), "B%u:%s", (unsigned)(left + 1U),
             drv_hall_is_pressed(left) ? "ON " : "OFF");
    drv_display_draw_text(0, y, line);

    snprintf(line, sizeof(line), "B%u:%s", (unsigned)(right + 1U),
             drv_hall_is_pressed(right) ? "ON " : "OFF");
    drv_display_draw_text(64, y, line);
  }
}

int main(void) {

  halInit();
  chSysInit();
  drivers_init_all();

  drv_hall_init();
  chThdCreateStatic(waHallTask, sizeof(waHallTask), NORMALPRIO, hallTask, NULL);

  while (true) {
    draw_hall_states();
    chThdSleepMilliseconds(100);
  }
}
