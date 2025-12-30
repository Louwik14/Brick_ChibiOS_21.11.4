#include "ch.h"
#include "hal.h"

int main(void) {

  halInit();
  chSysInit();

  /* PH7 as output */
  palSetPadMode(GPIOH, 7, PAL_MODE_OUTPUT_PUSHPULL);

  while (true) {
    /* LED ON (cas active-low) */
    palClearPad(GPIOH, 7);
    chThdSleepMilliseconds(1000);

    /* LED OFF */
    palSetPad(GPIOH, 7);
    chThdSleepMilliseconds(1000);
  }
}
