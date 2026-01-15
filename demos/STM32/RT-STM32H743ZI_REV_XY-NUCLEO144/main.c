#include "ch.h"
#include "hal.h"
#include <string.h>

/* Buffer MUST be aligned and non-cached on H7 */
__attribute__((section(".nocache"), aligned(32)))
static uint8_t txbuf[512];

int main(void) {

  halInit();
  chSysInit();

  /* Start SDC */
  sdcStart(&SDCD1, NULL);

  /* Try connect */
  if (sdcConnect(&SDCD1)) {
    /* Failure: hang here */
    while (1) {
      chThdSleepMilliseconds(1000);
    }
  }

  /* Fill buffer */
  for (int i = 0; i < 512; i++) {
    txbuf[i] = 0xA5;
  }

  /* Write block 0 */
  if (sdcWrite(&SDCD1, 0, txbuf, 1)) {
    /* Write failed: hang here */
    while (1) {
      chThdSleepMilliseconds(1000);
    }
  }

  /* Done: stay here forever */
  while (1) {
    chThdSleepMilliseconds(1000);
  }
}
