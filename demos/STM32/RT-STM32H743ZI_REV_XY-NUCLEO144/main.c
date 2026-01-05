#include "ch.h"
#include "hal.h"
#include "audio/drv_audio.h"
#include "drivers/drivers.h"

int main(void) {
  halInit();
  chSysInit();

  drivers_init_all();

  drv_audio_init();
  drv_audio_start();

  while (true) {
    chThdSleepSeconds(1);
  }
}
