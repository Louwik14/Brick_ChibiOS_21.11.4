#include "ch.h"
#include "hal.h"
#include "chprintf.h"

/* ================= UART ================= */

static const SerialConfig uart_cfg = {
  115200,
  0,
  USART_CR2_STOP1_BITS,
  0
};

/* ================= SDRAM ================= */

#define SDRAM_BASE  0xC0000000U   /* FMC SDRAM Bank 1 */

static const SDRAMConfig sdram_cfg = {
  .sdcr =
    FMC_SDCRx_NC_0 |                /* 9 column bits */
    FMC_SDCRx_NR_1 |                /* 13 row bits */
    FMC_SDCRx_MWID_0 |              /* x16 */
    FMC_SDCRx_NB |                  /* 4 banks */
    (2U << FMC_SDCRx_CAS_Pos) |     /* CAS latency = 3 */
    FMC_SDCRx_SDCLK_1 |             /* SDCLK = HCLK / 2 */
    (1U << FMC_SDCRx_RPIPE_Pos),    /* Read pipe (important on H7) */

  .sdtr =
    ((2U - 1U) << FMC_SDTRx_TMRD_Pos) |
    ((7U - 1U) << FMC_SDTRx_TXSR_Pos) |
    ((5U - 1U) << FMC_SDTRx_TRAS_Pos) |
    ((7U - 1U) << FMC_SDTRx_TRC_Pos)  |
    ((2U - 1U) << FMC_SDTRx_TWR_Pos)  |
    ((2U - 1U) << FMC_SDTRx_TRP_Pos)  |
    ((2U - 1U) << FMC_SDTRx_TRCD_Pos),

  .sdcmr = 0,
  .sdrtr = (781U << FMC_SDRTR_COUNT_Pos)
};

int main(void) {

  halInit();
  chSysInit();

  sdStart(&SD1, &uart_cfg);
  chprintf((BaseSequentialStream *)&SD1,
           "\r\n=== SDRAM FMC TEST (ChibiOS H7) ===\r\n");

  /* ---------- Init SDRAM ---------- */

  sdramInit();
  sdramStart(&SDRAMD1, &sdram_cfg);

  chprintf((BaseSequentialStream *)&SD1,
           "SDRAM started\r\n");

  /* ============================================================
   * PROGRAM SDRAM MODE REGISTER (THE RIGHT WAY IN CHIBIOS)
   *
   * Burst Length = 1
   * Burst Type   = Sequential
   * CAS Latency  = 3
   * Write Mode   = Single
   *
   * Mode Register = 0x0230
   * ============================================================ */

  sdramSendCommand(
    &SDRAMD1,
    SDRAM_CMD_LOAD_MODE,
    SDRAM_BANK_1,
    1,
    0x0230
  );

  chprintf((BaseSequentialStream *)&SD1,
           "Mode Register programmed\r\n");

  /* ---------- SDRAM test ---------- */

  volatile uint16_t *sdram = (volatile uint16_t *)SDRAM_BASE;

  chprintf((BaseSequentialStream *)&SD1, "WRITE...\r\n");

  for (uint32_t i = 0; i < 32; i++) {
    sdram[i] = (uint16_t)(0xA500 + i);
  }

  chprintf((BaseSequentialStream *)&SD1, "READ...\r\n");

  bool ok = true;

  for (uint32_t i = 0; i < 32; i++) {
    uint16_t v = sdram[i];
    chprintf((BaseSequentialStream *)&SD1,
             "R[%lu] = 0x%04X\r\n", i, v);

    if (v != (uint16_t)(0xA500 + i)) {
      ok = false;
    }
  }

  chprintf((BaseSequentialStream *)&SD1,
           ok ? "SDRAM OK ✅\r\n" : "SDRAM FAIL ❌\r\n");

  while (true) {
    chThdSleepMilliseconds(1000);
  }
}
