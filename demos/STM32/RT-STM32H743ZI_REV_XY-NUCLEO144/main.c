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

/* SD pins init */
static void sd_pins_init(void) {
  palSetPadMode(GPIOC,  8, PAL_MODE_ALTERNATE(12) | PAL_STM32_OSPEED_HIGHEST);
  palSetPadMode(GPIOC,  9, PAL_MODE_ALTERNATE(12) | PAL_STM32_OSPEED_HIGHEST);
  palSetPadMode(GPIOC, 10, PAL_MODE_ALTERNATE(12) | PAL_STM32_OSPEED_HIGHEST);
  palSetPadMode(GPIOC, 11, PAL_MODE_ALTERNATE(12) | PAL_STM32_OSPEED_HIGHEST);
  palSetPadMode(GPIOC, 12, PAL_MODE_ALTERNATE(12) | PAL_STM32_OSPEED_HIGHEST); // CK
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
  chprintf(chp, "  SDMMC H743 PLL UNLOCK TEST\r\n");
  chprintf(chp, "============================\r\n");

  /* ---- Dump RCC before ---- */
  chprintf(chp, "BEFORE:\r\n");
  chprintf(chp, "RCC->CR        = %08lx\r\n", RCC->CR);
  chprintf(chp, "RCC->PLLCFGR   = %08lx\r\n", RCC->PLLCFGR);

  /* =========================================================
     TURN OFF PLL1 TO ALLOW RECONFIG
     ========================================================= */
  chprintf(chp, "Disabling PLL1...\r\n");

  RCC->CR &= ~RCC_CR_PLL1ON;
  while (RCC->CR & RCC_CR_PLL1RDY);  // wait until really off

  /* =========================================================
     ENABLE PLL1 Q and R outputs
     Bits 17 = DIVQ1EN, 18 = DIVR1EN
     ========================================================= */
  chprintf(chp, "Enabling PLL1 Q/R outputs...\r\n");

  RCC->PLLCFGR |= (1U << 17) | (1U << 18);

  /* =========================================================
     TURN PLL1 BACK ON
     ========================================================= */
  chprintf(chp, "Re-enabling PLL1...\r\n");

  RCC->CR |= RCC_CR_PLL1ON;
  while (!(RCC->CR & RCC_CR_PLL1RDY));

  /* ---- Dump RCC after ---- */
  chprintf(chp, "AFTER:\r\n");
  chprintf(chp, "RCC->CR        = %08lx\r\n", RCC->CR);
  chprintf(chp, "RCC->PLLCFGR   = %08lx\r\n", RCC->PLLCFGR);

  /* =========================================================
     SELECT SDMMC kernel clock = PLL1Q
     ========================================================= */
  chprintf(chp, "Selecting SDMMC kernel clock = PLL1Q\r\n");

  RCC->D1CCIPR &= ~(3U << 16);
  RCC->D1CCIPR |=  (1U << 16);

  chprintf(chp, "RCC->D1CCIPR   = %08lx\r\n", RCC->D1CCIPR);

  /* =========================================================
     ENABLE AHB3 SDMMC1 CLOCK
     ========================================================= */
  chprintf(chp, "Enabling AHB3 clock for SDMMC1\r\n");
  RCC->AHB3ENR |= RCC_AHB3ENR_SDMMC1EN;
  __DSB();
  chprintf(chp, "RCC->AHB3ENR   = %08lx\r\n", RCC->AHB3ENR);

  /* =========================================================
     GPIO
     ========================================================= */
  sd_pins_init();
  chThdSleepMilliseconds(10);

  /* =========================================================
     RESET SDMMC
     ========================================================= */
  RCC->AHB3RSTR |= RCC_AHB3RSTR_SDMMC1RST;
  __DSB();
  RCC->AHB3RSTR &= ~RCC_AHB3RSTR_SDMMC1RST;
  __DSB();

  /* =========================================================
     BASIC SDMMC INIT
     ========================================================= */
  SDMMC1->POWER = 0;
  SDMMC1->CLKCR = 0;
  SDMMC1->ICR   = 0xFFFFFFFF;

  SDMMC1->POWER = 3;
  SDMMC1->CLKCR = 0x34;

  chprintf(chp, "After init:\r\n");
  chprintf(chp, "SDMMC1 POWER = %08lx\r\n", SDMMC1->POWER);
  chprintf(chp, "SDMMC1 CLKCR = %08lx\r\n", SDMMC1->CLKCR);
  chprintf(chp, "SDMMC1 STA   = %08lx\r\n", SDMMC1->STA);

  /* =========================================================
     SEND CMD0
     ========================================================= */
  chprintf(chp, "\r\nSending CMD0...\r\n");

  SDMMC1->ICR = 0xFFFFFFFF;
  SDMMC1->ARG = 0;
  SDMMC1->CMD = (0U) | (1U << 10);

  uint32_t sta;
  uint32_t timeout = 1000000;

  do {
    sta = SDMMC1->STA;
    timeout--;
  } while (((sta & (SDMMC_STA_CMDSENT | SDMMC_STA_CMDREND | SDMMC_STA_CTIMEOUT)) == 0U) && timeout);

  chprintf(chp, "After CMD0:\r\n");
  chprintf(chp, "SDMMC1 STA   = %08lx\r\n", SDMMC1->STA);

  if (sta & SDMMC_STA_CMDSENT) {
    chprintf(chp, "✅ CMDSENT -> SDMMC CLOCK IS RUNNING\r\n");
  } else if (sta & SDMMC_STA_CMDREND) {
    chprintf(chp, "✅ CMDREND -> SDMMC CLOCK IS RUNNING\r\n");
  } else if (sta & SDMMC_STA_CTIMEOUT) {
    chprintf(chp, "⚠️ CTIMEOUT -> CLOCK RUNNING, no card response\r\n");
  } else {
    chprintf(chp, "❌ NOTHING -> SDMMC STILL NOT CLOCKED\r\n");
  }

  while (1) {
    chThdSleepMilliseconds(1000);
  }
}
