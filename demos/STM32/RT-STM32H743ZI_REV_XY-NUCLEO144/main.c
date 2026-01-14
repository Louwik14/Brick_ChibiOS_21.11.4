#include "ch.h"
#include "hal.h"
#include "chprintf.h"

/* UART config */
static SerialConfig uart_cfg = {
  115200,
  0,
  USART_CR2_STOP1_BITS,
  0
};

/* SD pins */
static void sd_pins_init(void) {
  palSetPadMode(GPIOC,  8, PAL_MODE_ALTERNATE(12) | PAL_STM32_OSPEED_HIGHEST);
  palSetPadMode(GPIOC,  9, PAL_MODE_ALTERNATE(12) | PAL_STM32_OSPEED_HIGHEST);
  palSetPadMode(GPIOC, 10, PAL_MODE_ALTERNATE(12) | PAL_STM32_OSPEED_HIGHEST);
  palSetPadMode(GPIOC, 11, PAL_MODE_ALTERNATE(12) | PAL_STM32_OSPEED_HIGHEST);
  palSetPadMode(GPIOC, 12, PAL_MODE_ALTERNATE(12) | PAL_STM32_OSPEED_HIGHEST);
  palSetPadMode(GPIOD,  2, PAL_MODE_ALTERNATE(12) | PAL_STM32_OSPEED_HIGHEST);
}

int main(void) {

  halInit();
  chSysInit();

  SCB_DisableICache();
  SCB_DisableDCache();

  sdStart(&SD1, &uart_cfg);
  BaseSequentialStream *chp = (BaseSequentialStream *)&SD1;

  chprintf(chp, "\r\n============================\r\n");
  chprintf(chp, "  SDMMC SDC low-level test\r\n");
  chprintf(chp, "============================\r\n");

  sd_pins_init();
  chThdSleepMilliseconds(10);

  chprintf(chp, "Starting SDC...\r\n");
  sdcStart(&SDCD1, NULL);

  chprintf(chp, "Connecting card...\r\n");

  if (sdcConnect(&SDCD1)) {
    chprintf(chp, "❌ sdcConnect() FAILED\r\n");
  } else {
    chprintf(chp, "✅ Card initialized!\r\n");
  }

  /* Dump some registers for sanity */
  chprintf(chp, "SDMMC1 POWER = %08lx\r\n", SDMMC1->POWER);
  chprintf(chp, "SDMMC1 CLKCR = %08lx\r\n", SDMMC1->CLKCR);
  chprintf(chp, "SDMMC1 STA   = %08lx\r\n", SDMMC1->STA);

  while (true) {
    chThdSleepMilliseconds(1000);
  }
}
