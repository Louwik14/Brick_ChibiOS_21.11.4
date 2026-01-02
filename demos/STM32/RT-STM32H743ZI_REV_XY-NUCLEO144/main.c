#include "ch.h"
#include "hal.h"
#include "chprintf.h"

static const SerialConfig uart_cfg = {
  115200, 0, USART_CR2_STOP1_BITS, 0
};

int main(void) {
  uint8_t buffer[512];
  sdcflags_t errors;

  halInit();
  chSysInit();

  sdStart(&SD1, &uart_cfg);
  chprintf((BaseSequentialStream *)&SD1,
           "\r\n=== SDMMC1 RAW BLOCK TEST ===\r\n");

  sdcStart(&SDCD1, NULL);
  if (sdcConnect(&SDCD1) != HAL_SUCCESS) {
    errors = sdcGetAndClearErrors(&SDCD1);
    chprintf((BaseSequentialStream *)&SD1,
             "sdcConnect failed (errors=0x%08lX)\r\n", errors);
    while (true) {
      chThdSleepMilliseconds(1000);
    }
  }

  if (sdcRead(&SDCD1, 0, buffer, 1) != HAL_SUCCESS) {
    errors = sdcGetAndClearErrors(&SDCD1);
    chprintf((BaseSequentialStream *)&SD1,
             "sdcRead failed (errors=0x%08lX)\r\n", errors);
    while (true) {
      chThdSleepMilliseconds(1000);
    }
  }

  chprintf((BaseSequentialStream *)&SD1, "LBA0[0..31]:\r\n");
  for (size_t i = 0; i < 32; i++) {
    chprintf((BaseSequentialStream *)&SD1, "%02X ", buffer[i]);
    if (((i + 1U) % 16U) == 0U) {
      chprintf((BaseSequentialStream *)&SD1, "\r\n");
    }
  }
  chprintf((BaseSequentialStream *)&SD1, "\r\n");

  while (true) {
    chThdSleepMilliseconds(1000);
  }
}
