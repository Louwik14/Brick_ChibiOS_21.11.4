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

/* SD pins (SDMMC1) */
static void sd_pins_init_sdmmc1(void) {
  palSetPadMode(GPIOC,  8, PAL_MODE_ALTERNATE(12) | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_PUPDR_PULLUP);
  palSetPadMode(GPIOC,  9, PAL_MODE_ALTERNATE(12) | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_PUPDR_PULLUP);
  palSetPadMode(GPIOC, 10, PAL_MODE_ALTERNATE(12) | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_PUPDR_PULLUP);
  palSetPadMode(GPIOC, 11, PAL_MODE_ALTERNATE(12) | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_PUPDR_PULLUP);
  palSetPadMode(GPIOC, 12, PAL_MODE_ALTERNATE(12) | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_PUPDR_PULLUP);
  palSetPadMode(GPIOD,  2, PAL_MODE_ALTERNATE(12) | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_PUPDR_PULLUP);
}

/* SD pins (SDMMC2 common mapping on ST boards) */
static void sd_pins_init_sdmmc2(void) {
  palSetPadMode(GPIOD, 7, PAL_MODE_ALTERNATE(11) | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_PUPDR_PULLUP); // CMD
  palSetPadMode(GPIOD, 6, PAL_MODE_ALTERNATE(11) | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_PUPDR_PULLUP); // CLK
  palSetPadMode(GPIOB,14, PAL_MODE_ALTERNATE(9)  | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_PUPDR_PULLUP); // D0
  palSetPadMode(GPIOB,15, PAL_MODE_ALTERNATE(9)  | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_PUPDR_PULLUP); // D1
  palSetPadMode(GPIOB, 3, PAL_MODE_ALTERNATE(9)  | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_PUPDR_PULLUP); // D2
  palSetPadMode(GPIOB, 4, PAL_MODE_ALTERNATE(9)  | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_PUPDR_PULLUP); // D3
}

static uint32_t test_sdmmc(volatile SDMMC_TypeDef *sdmmc, uint32_t ahb3en_bit) {
  /* Enable SDMMC clock */
  RCC->AHB3ENR |= (1U << ahb3en_bit);

  /* Force kernel clock = HCLK */
  RCC->D1CCIPR &= ~(3UL << 16);

  /* Reset peripheral */
  RCC->AHB3RSTR |=  (1U << ahb3en_bit);
  RCC->AHB3RSTR &= ~(1U << ahb3en_bit);

  /* Power and slow clock */
  sdmmc->POWER = 3;
  sdmmc->CLKCR = (1UL << 8) | 120;

  /* Send illegal command to force a flag */
  sdmmc->ICR = 0xFFFFFFFF;
  sdmmc->ARG = 0;
  sdmmc->CMD = (63 << 0) | (1 << 10) | (1 << 6); // CPSMEN + WAITRESP

  chThdSleepMilliseconds(20);

  return sdmmc->STA;
}

int main(void) {

  halInit();
  chSysInit();

  /* Start UART */
  sdStart(&SD1, &uart_cfg);
  BaseSequentialStream *chp = (BaseSequentialStream *)&SD1;

  chprintf(chp, "\r\n============================\r\n");
  chprintf(chp, "  STM32H7 AHB3 / SDMMC DIAG\r\n");
  chprintf(chp, "============================\r\n");

  chprintf(chp, "DBGMCU_IDCODE = %08lX\r\n", DBGMCU->IDCODE);

  /* Enable PWR + VDDIO2 just in case */
  RCC->APB4ENR |= (1U << 28);   // PWREN
  PWR->CR3 |= (1U << 9);        // VDDIO2EN
  chThdSleepMilliseconds(10);

  chprintf(chp, "PWR->CR3 = %08lX\r\n", PWR->CR3);

  /* ================= TEST AHB3 DOMAIN VIA MDMA ================= */

  chprintf(chp, "\r\n--- Testing AHB3 domain with MDMA ---\r\n");

  /* Enable MDMA clock (AHB3 bit 0) */
  RCC->AHB3ENR |= (1U << 0);

  chThdSleepMilliseconds(1);

  uint32_t mdma_gisr0 = MDMA->GISR0;

  chprintf(chp, "MDMA->GISR0 = %08lX\r\n", mdma_gisr0);

  /* ================= TEST SDMMC1 ================= */

  chprintf(chp, "\r\n--- Testing SDMMC1 ---\r\n");
  sd_pins_init_sdmmc1();

  uint32_t sta1 = test_sdmmc(SDMMC1, 16); // bit 16 = SDMMC1EN

  chprintf(chp, "SDMMC1->STA = %08lX\r\n", sta1);

  /* ================= TEST SDMMC2 ================= */

  chprintf(chp, "\r\n--- Testing SDMMC2 ---\r\n");
  sd_pins_init_sdmmc2();

  uint32_t sta2 = test_sdmmc(SDMMC2, 17); // bit 17 = SDMMC2EN

  chprintf(chp, "SDMMC2->STA = %08lX\r\n", sta2);

  /* ================= VERDICT ================= */

  chprintf(chp, "\r\n============================\r\n");

  if ((sta1 == 0) && (sta2 == 0)) {
    chprintf(chp, "❌ BOTH SDMMC BLOCKS ARE DEAD\r\n");
    chprintf(chp, "=> Clock tree / reset domain / system init issue\r\n");
  } else {
    if (sta1 != 0) chprintf(chp, "✅ SDMMC1 IS ALIVE\r\n");
    if (sta2 != 0) chprintf(chp, "✅ SDMMC2 IS ALIVE\r\n");
  }

  chprintf(chp, "============================\r\n");

  while (1) {
    chThdSleepMilliseconds(1000);
  }
}
