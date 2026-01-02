#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "hal_fsmc.h"
#include "hal_sdram.h"

/* ------------------------------------------------------------------------- */
/* UART configuration (USART1 / SD1)                                          */
/* ------------------------------------------------------------------------- */

static const SerialConfig uart_cfg = {
  115200,
  0,
  USART_CR2_STOP1_BITS,
  0
};

static BaseSequentialStream *chp = (BaseSequentialStream *)&SD1;

/* ------------------------------------------------------------------------- */
/* SDRAM configuration (MINIMALE, SAFE)                                      */
/* ------------------------------------------------------------------------- */

static const SDRAMConfig sdram_cfg = {
  /* SDCR: configuration SDRAM */
  .sdcr =
    FMC_SDCRx_NC_0 |                  /* 9 column bits */
    FMC_SDCRx_NR_1 |                  /* 13 row bits */
    FMC_SDCRx_MWID_0 |                /* 16-bit data bus */
    FMC_SDCRx_NB |                    /* 4 internal banks */
    (2U << FMC_SDCRx_CAS_Pos) |       /* CAS latency = 3 */
    FMC_SDCRx_SDCLK_1,                /* SDRAM clock = HCLK / 2 */

  /* SDTR: timings très conservateurs */
  .sdtr =
    ((2U - 1U) << FMC_SDTRx_TMRD_Pos) |
    ((7U - 1U) << FMC_SDTRx_TXSR_Pos) |
    ((5U - 1U) << FMC_SDTRx_TRAS_Pos) |
    ((7U - 1U) << FMC_SDTRx_TRC_Pos)  |
    ((2U - 1U) << FMC_SDTRx_TWR_Pos)  |
    ((2U - 1U) << FMC_SDTRx_TRP_Pos)  |
    ((2U - 1U) << FMC_SDTRx_TRCD_Pos)
};

/* ------------------------------------------------------------------------- */
/* Main                                                                       */
/* ------------------------------------------------------------------------- */

int main(void)
{
  halInit();
  chSysInit();

  /* Start serial driver */
  sdStart(&SD1, &uart_cfg);
  chThdSleepMilliseconds(200);

  chprintf(chp, "\r\n");
  chprintf(chp, "===============================\r\n");
  chprintf(chp, " SDRAM DRIVER START TEST (H7)\r\n");
  chprintf(chp, "===============================\r\n");

  chprintf(chp, "BEFORE sdramStart()\r\n");
  sdramStart(&SDRAMD1, &sdram_cfg);
  chprintf(chp, "AFTER sdramStart()\r\n");

  while (true) {
    chprintf(chp, "ALIVE\r\n");
    chThdSleepMilliseconds(1000);
  }
}
