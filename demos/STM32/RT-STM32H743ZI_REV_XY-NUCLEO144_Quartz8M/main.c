#include "ch.h"
#include "hal.h"
#include "drivers/drivers.h"
#include <stdio.h>

int main(void) {

  halInit();
  chSysInit();
  drivers_init_all();

  uint32_t idcode = DBGMCU->IDCODE;

  while (true) {
    char line[32];

    drv_display_clear();

    snprintf(line, sizeof(line), "IDCODE:");
    drv_display_draw_text(0, 0, line);

    snprintf(line, sizeof(line), "0x%08lX", (unsigned long)idcode);
    drv_display_draw_text(0, 16, line);

    chThdSleepMilliseconds(1000);
  }
}
