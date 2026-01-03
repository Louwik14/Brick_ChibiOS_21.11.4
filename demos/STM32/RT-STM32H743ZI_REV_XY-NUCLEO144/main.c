#include "ch.h"
#include "hal.h"
#include "drivers.h"

int main(void) {
  halInit();
  chSysInit();

  drivers_init_all();

  drv_display_clear();
  drv_display_draw_text(0, 0, "Hello world");
  drv_display_update();

  while (true) {
    chThdSleepMilliseconds(1000);
  }
}
