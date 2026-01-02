#include "ch.h"
#include "hal.h"
#include "chprintf.h"

/* UART config (SD1) */
static const SerialConfig uart_cfg = {
  115200,
  0,
  USART_CR2_STOP1_BITS,
  0
};

/* SDRAM base address */
#define SDRAM_BASE  0xC0000000U

/* Minimal, safe SDRAM configuration */
static const SDRAMConfig sdram_cfg = {
  .sdcr =
    /* 8 column bits (CORRECT for W9825G6KH) */
    0 |
    FMC_SDCRx_NR_1 |            /* 13 row bits */
    FMC_SDCRx_MWID_0 |          /* 16-bit bus */
    FMC_SDCRx_NB |              /* 4 banks */
    (2U << FMC_SDCRx_CAS_Pos) | /* CAS = 3 */
    FMC_SDCRx_SDCLK_1,          /* HCLK/2 */


  .sdtr =
    ((2U - 1U) << FMC_SDTRx_TMRD_Pos) |
    ((7U - 1U) << FMC_SDTRx_TXSR_Pos) |
    ((5U - 1U) << FMC_SDTRx_TRAS_Pos) |
    ((7U - 1U) << FMC_SDTRx_TRC_Pos)  |
    ((2U - 1U) << FMC_SDTRx_TWR_Pos)  |
    ((2U - 1U) << FMC_SDTRx_TRP_Pos)  |
    ((2U - 1U) << FMC_SDTRx_TRCD_Pos),

  .sdcmr = 0, /* handled by HAL */
  .sdrtr = (761U << FMC_SDRTR_COUNT_Pos)
};

int main(void) {
  halInit();
  chSysInit();

  sdStart(&SD1, &uart_cfg);
  chprintf((BaseSequentialStream *)&SD1,
           "\r\n=== SDRAM TEST START ===\r\n");

  /* Init SDRAM subsystem */
  sdramInit();
  chprintf((BaseSequentialStream *)&SD1, "sdramInit OK\r\n");

  /* Start SDRAM */
  sdramStart(&SDRAMD1, &sdram_cfg);
  chprintf((BaseSequentialStream *)&SD1, "sdramStart OK\r\n");

  volatile uint16_t *sdram = (volatile uint16_t *)SDRAM_BASE;

  sdram[0] = 0x1234;
  sdram[1] = 0xABCD;

  uint16_t a = sdram[0];
  uint16_t b = sdram[1];

  chprintf((BaseSequentialStream *)&SD1,
           "R0=0x%04X R1=0x%04X\r\n", a, b);

  if (a == 0x1234 && b == 0xABCD) {
    chprintf((BaseSequentialStream *)&SD1, "SDRAM BASIC OK\r\n");
  } else {
    chprintf((BaseSequentialStream *)&SD1, "SDRAM BASIC FAIL\r\n");
  }

  while (true) {
    chThdSleepMilliseconds(1000);
  }
}
