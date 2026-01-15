#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include <string.h>

/* UART config */
static SerialConfig uart_cfg = {
  115200,
  0,
  USART_CR2_STOP1_BITS,
  0
};

/* Buffers MUST be aligned and non-cached on H7 */
__attribute__((section(".nocache"), aligned(32)))
static uint8_t txbuf[512];

__attribute__((section(".nocache"), aligned(32)))
static uint8_t rxbuf[512];

/* SD pins */
static void sd_pins_init(void) {
  palSetPadMode(GPIOC,  8, PAL_MODE_ALTERNATE(12) | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_PUPDR_PULLUP);
  palSetPadMode(GPIOC,  9, PAL_MODE_ALTERNATE(12) | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_PUPDR_PULLUP);
  palSetPadMode(GPIOC, 10, PAL_MODE_ALTERNATE(12) | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_PUPDR_PULLUP);
  palSetPadMode(GPIOC, 11, PAL_MODE_ALTERNATE(12) | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_PUPDR_PULLUP);
  palSetPadMode(GPIOC, 12, PAL_MODE_ALTERNATE(12) | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_PUPDR_PULLUP);
  palSetPadMode(GPIOD,  2, PAL_MODE_ALTERNATE(12) | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_PUPDR_PULLUP);
}


int main(void) {

  halInit();
  RCC->AHB3ENR |= (1 << 16);      // SDMMC1EN
  RCC->D1CCIPR = (RCC->D1CCIPR & ~(3 << 16)) | (1 << 16); // kernel = PLL1Q

  RCC->D1CCIPR |= (1 << 16);

  chSysInit();

  /* Start UART */
  sdStart(&SD1, &uart_cfg);
  BaseSequentialStream *chp = (BaseSequentialStream *)&SD1;

  chprintf(chp, "\r\n============================\r\n");
  chprintf(chp, "  MINIMAL SDMMC TEST\r\n");
  chprintf(chp, "============================\r\n");

  /* Init pins */
  sd_pins_init();
  chThdSleepMilliseconds(10);

  /* Start SDC driver */
  chprintf(chp, "Starting SDC...\r\n");
  sdcStart(&SDCD1, NULL);
  SDMMC1->POWER = 3;
  SDMMC1->CLKCR = 0x100 | 120;   // enable clock + div
  chprintf(chp, "CLKCR = %08X\r\n", SDMMC1->CLKCR);
  chprintf(chp, "POWER = %08X\r\n", SDMMC1->POWER);
  SDMMC1->ARG = 0;
  SDMMC1->CMD = 0 | (1<<10) | (1<<9);  // CMD0 CPSMEN | WAITRESP=0
  chThdSleepMilliseconds(10);
  chprintf(chp, "STA = %08X\r\n", SDMMC1->STA);

  SDMMC1->ARG = 0;
  SDMMC1->CMD = 0 | (1<<10); // CPSMEN

  chThdSleepMilliseconds(10);

  uint32_t sta = SDMMC1->STA;
  uint32_t icr = SDMMC1->ICR;

  chprintf(chp, "STA after manual CMD0 = %08lx\r\n", sta);

  chprintf(chp, "Connecting card...\r\n");
  if (sdcConnect(&SDCD1)) {
    chprintf(chp, "❌ sdcConnect() FAILED\r\n");
    while (1) chThdSleepMilliseconds(1000);
  }

  chprintf(chp, "✅ Card initialized\r\n");

  /* Prepare test pattern */
  for (int i = 0; i < 512; i++) {
    txbuf[i] = (uint8_t)i;
    rxbuf[i] = 0;
  }

  chprintf(chp, "Writing block 0...\r\n");
  if (sdcWrite(&SDCD1, 0, txbuf, 1)) {
    chprintf(chp, "❌ Write failed\r\n");
    while (1) chThdSleepMilliseconds(1000);
  }

  chprintf(chp, "Reading block 0...\r\n");
  if (sdcRead(&SDCD1, 0, rxbuf, 1)) {
    chprintf(chp, "❌ Read failed\r\n");
    while (1) chThdSleepMilliseconds(1000);
  }

  /* Compare */
  if (memcmp(txbuf, rxbuf, 512) == 0) {
    chprintf(chp, "✅ READ/WRITE OK\r\n");
  } else {
    chprintf(chp, "❌ DATA MISMATCH\r\n");
  }

  while (1) {
    chThdSleepMilliseconds(1000);
  }
}
