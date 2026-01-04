#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "drv_audio.h"

static const SerialConfig uart_cfg = {
  115200,
  0,
  0,
  0
};

int main(void) {
  halInit();
  chSysInit();

  sdStart(&SD1, &uart_cfg);
  chprintf((BaseSequentialStream *)&SD1, "\r\nSAI HAL debug (no DMA, no codec)\r\n");
  chprintf((BaseSequentialStream *)&SD1, "SAI HAL debug start\r\n");

  drv_audio_run_sai_hal_debug();

  chprintf((BaseSequentialStream *)&SD1, "SAI HAL debug done\r\n");

  while (true) {
    chThdSleepSeconds(1);
  }
}
