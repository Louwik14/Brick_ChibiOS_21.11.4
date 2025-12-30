#include "ch.h"
#include "hal.h"

int main(void) {

  halInit();
  chSysInit();

  /* PH7 : LED active-low
     Forcer mode + état initial connu */
  palSetPadMode(GPIOH, 7, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPad(GPIOH, 7);   /* LED OFF */

  while (true) {
    /* LED ON */
    palClearPad(GPIOH, 7);
    chThdSleepMilliseconds(1000);

    /* LED OFF */
    palSetPad(GPIOH, 7);
    chThdSleepMilliseconds(1000);
  }
}
