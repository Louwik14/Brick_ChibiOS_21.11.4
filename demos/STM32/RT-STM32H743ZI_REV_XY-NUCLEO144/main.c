#include "ch.h"
#include "hal.h"
#include "chprintf.h"

/* UART config */
static SerialConfig sercfg = {
  115200,
  0,
  USART_CR2_STOP1_BITS,
  0
};

#define SDMMC_BASE 0x52007000UL

#define SDMMC_POWER   (*(volatile uint32_t *)(SDMMC_BASE + 0x00))
#define SDMMC_CLKCR   (*(volatile uint32_t *)(SDMMC_BASE + 0x04))
#define SDMMC_ARG     (*(volatile uint32_t *)(SDMMC_BASE + 0x08))
#define SDMMC_CMD     (*(volatile uint32_t *)(SDMMC_BASE + 0x0C))
#define SDMMC_RESPCMD (*(volatile uint32_t *)(SDMMC_BASE + 0x10))
#define SDMMC_RESP1   (*(volatile uint32_t *)(SDMMC_BASE + 0x14))
#define SDMMC_RESP2   (*(volatile uint32_t *)(SDMMC_BASE + 0x18))
#define SDMMC_RESP3   (*(volatile uint32_t *)(SDMMC_BASE + 0x1C))
#define SDMMC_RESP4   (*(volatile uint32_t *)(SDMMC_BASE + 0x20))
#define SDMMC_DTIMER  (*(volatile uint32_t *)(SDMMC_BASE + 0x24))
#define SDMMC_DLEN    (*(volatile uint32_t *)(SDMMC_BASE + 0x28))
#define SDMMC_DCTRL   (*(volatile uint32_t *)(SDMMC_BASE + 0x2C))
#define SDMMC_STA     (*(volatile uint32_t *)(SDMMC_BASE + 0x34))
#define SDMMC_ICR     (*(volatile uint32_t *)(SDMMC_BASE + 0x38))
#define SDMMC_MASK    (*(volatile uint32_t *)(SDMMC_BASE + 0x3C))

static void dump_sdmmc(BaseSequentialStream *chp) {
  chprintf(chp, "SDMMC_POWER = 0x%08X\r\n", SDMMC_POWER);
  chprintf(chp, "SDMMC_CLKCR = 0x%08X\r\n", SDMMC_CLKCR);
  chprintf(chp, "SDMMC_CMD   = 0x%08X\r\n", SDMMC_CMD);
  chprintf(chp, "SDMMC_ARG   = 0x%08X\r\n", SDMMC_ARG);
  chprintf(chp, "SDMMC_RESP1 = 0x%08X\r\n", SDMMC_RESP1);
  chprintf(chp, "SDMMC_STA   = 0x%08X\r\n", SDMMC_STA);
  chprintf(chp, "SDMMC_DCTRL = 0x%08X\r\n", SDMMC_DCTRL);
}

int main(void) {

  halInit();
  chSysInit();

  /* Start UART */
  sdStart(&SD1, &sercfg);
  chprintf((BaseSequentialStream *)&SD1, "\r\n=== STM32H743 SDMMC MINIMAL TEST ===\r\n");

  chprintf((BaseSequentialStream *)&SD1, "Starting SDC driver...\r\n");
  sdcStart(&SDCD1, NULL);

  /* Important on H7: let clocks & power settle */
  chThdSleepMilliseconds(50);

  chprintf((BaseSequentialStream *)&SD1, "Connecting to SD card...\r\n");

  if (sdcConnect(&SDCD1)) {
    chprintf((BaseSequentialStream *)&SD1, "\r\n❌ ERROR: sdcConnect() failed\r\n");
    chprintf((BaseSequentialStream *)&SD1, "Dumping SDMMC registers:\r\n");
    dump_sdmmc((BaseSequentialStream *)&SD1);

    while (true) {
      chThdSleepMilliseconds(1000);
    }
  }

  chprintf((BaseSequentialStream *)&SD1, "\r\n✅ SD card detected and initialized successfully!\r\n");

  dump_sdmmc((BaseSequentialStream *)&SD1);

  while (true) {
    chThdSleepMilliseconds(1000);
  }
}
