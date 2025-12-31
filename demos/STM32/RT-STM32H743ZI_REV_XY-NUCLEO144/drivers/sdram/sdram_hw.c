/*
 * NOTE:
 * - SDRAM timings/refresh are calibrated for SDCLK = 100 MHz (HCLK = 200 MHz)
 *   on W9825G6KH.
 * - MPU and cache configuration are intentionally disabled at this project stage.
 */
#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "stm32h7xx.h"
#include "stm32h743xx.h"
#include "sdram_driver_priv.h"

#define SDRAM_REFRESH_COUNT       (761u) /* Valid only for SDCLK = 100 MHz (W9825G6KH, 64 ms / 8192 rows). */
#define SDRAM_TIMEOUT_CYCLES      (0x3FFFFu)
#define SDRAM_SDSR_BUSY           (1u << 5) /* RM0455: SDSR busy flag (bit 5) */

/* W9825G6KH Mode Register: BL=1, sequential, CAS=3, standard op, single write. */
#define SDRAM_MODE_BURST_LENGTH_1    (0x0u)
#define SDRAM_MODE_BURST_TYPE_SEQ    (0x0u << 3)
#define SDRAM_MODE_CAS_LATENCY_3     (0x3u << 4)
#define SDRAM_MODE_STANDARD_OPERATION (0x0u << 7)
#define SDRAM_MODE_WRITEBURST_SINGLE (0x1u << 9)
#define SDRAM_MODE_REGISTER_VALUE    (SDRAM_MODE_BURST_LENGTH_1 |        \
                                      SDRAM_MODE_BURST_TYPE_SEQ |        \
                                      SDRAM_MODE_CAS_LATENCY_3 |         \
                                      SDRAM_MODE_STANDARD_OPERATION |    \
                                      SDRAM_MODE_WRITEBURST_SINGLE)

#define SDRAM_CMD_NORMAL         (0u)
#define SDRAM_CMD_CLK_ENABLE     (1u)
#define SDRAM_CMD_PALL           (2u)
#define SDRAM_CMD_AUTOREFRESH    (3u)
#define SDRAM_CMD_LOAD_MODE      (4u)

/* FMC Bank1 (SDNE0/SDCKE0) selection for this board. */
#define SDRAM_FMC_TARGET_BANK    FMC_SDCMR_CTB1
#define SDRAM_SDSR_MODE_MASK     FMC_SDSR_MODES1_Msk
#define SDRAM_SDSR_MODE_SHIFT    FMC_SDSR_MODES1_Pos

static uint32_t fmc_current_mode(void);

static bool fmc_wait_while_busy(uint32_t timeout)
{
  while ((FMC_Bank5_6_R->SDSR & SDRAM_SDSR_BUSY) != 0u) {
    if (timeout-- == 0u) {
      return false;
    }
  }

  return true;
}

static bool fmc_wait_for_mode(uint32_t expected_mode, uint32_t timeout)
{
  while (fmc_current_mode() != expected_mode) {
    if (timeout-- == 0u) {
      return false;
    }
  }

  return true;
}

static bool fmc_issue_command(uint32_t mode, uint32_t auto_refresh, uint32_t mode_reg)
{
  const uint32_t command = ((mode << FMC_SDCMR_MODE_Pos) & FMC_SDCMR_MODE_Msk) |
                           SDRAM_FMC_TARGET_BANK |
                           ((((auto_refresh > 0u) ? (auto_refresh - 1u) : 0u) << FMC_SDCMR_NRFS_Pos) &
                            FMC_SDCMR_NRFS_Msk) |
                           ((mode_reg << FMC_SDCMR_MRD_Pos) & FMC_SDCMR_MRD_Msk);

  FMC_Bank5_6_R->SDCMR = command;
  return fmc_wait_while_busy(SDRAM_TIMEOUT_CYCLES);
}

static uint32_t fmc_current_mode(void)
{
  return (FMC_Bank5_6_R->SDSR & SDRAM_SDSR_MODE_MASK) >> SDRAM_SDSR_MODE_SHIFT;
}

bool sdram_hw_init_sequence(void)
{
  BaseSequentialStream *chp = (BaseSequentialStream *)&SD1;

  chprintf(chp, "[SDRAM] INIT: FMC/SDRAM sequence start\r\n");

  chprintf(chp, "[SDRAM] STEP: FMC clock enable\r\n");
  if ((RCC->AHB3ENR & RCC_AHB3ENR_FMCEN) == 0u) {
    RCC->AHB3ENR |= RCC_AHB3ENR_FMCEN;
    (void)RCC->AHB3ENR;
  }
  chprintf(chp, "[SDRAM] STEP: FMC clock enable done\r\n");

  const uint32_t sdcr = FMC_SDCRx_NC_0 | /* 9 columns */
                        FMC_SDCRx_NR_1 | /* 13 rows    */
                        FMC_SDCRx_MWID_0 | /* 16-bit bus */
                        FMC_SDCRx_NB | /* 4 internal banks */
                        ((2u << FMC_SDCRx_CAS_Pos) & FMC_SDCRx_CAS_Msk) | /* CAS latency = 3 */
                        FMC_SDCRx_SDCLK_1 | /* 2 HCLK period */
                        FMC_SDCRx_RBURST; /* enable read burst */

  /* Timing values below are calibrated for W9825G6KH at SDCLK = 100 MHz. */
  const uint32_t sdtr = ((2u - 1u) << FMC_SDTRx_TMRD_Pos) |  /* tMRD */
                        ((8u - 1u) << FMC_SDTRx_TXSR_Pos) |  /* tXSR */
                        ((6u - 1u) << FMC_SDTRx_TRAS_Pos) |  /* tRAS */
                        ((6u - 1u) << FMC_SDTRx_TRC_Pos) |   /* tRC  */
                        ((3u - 1u) << FMC_SDTRx_TWR_Pos) |   /* tWR  */
                        ((3u - 1u) << FMC_SDTRx_TRP_Pos) |   /* tRP  */
                        ((3u - 1u) << FMC_SDTRx_TRCD_Pos);   /* tRCD */

  chprintf(chp, "[SDRAM] STEP: Configure SDCR/SDTR\r\n");
  FMC_Bank5_6_R->SDCR[0] = sdcr;
  FMC_Bank5_6_R->SDTR[0] = sdtr;
  chprintf(chp, "[SDRAM] STEP: SDCR/SDTR configured\r\n");

  chThdSleepMicroseconds(200u);

  chprintf(chp, "[SDRAM] CMD: CLK_ENABLE\r\n");
  if (!fmc_issue_command(SDRAM_CMD_CLK_ENABLE, 1u, 0u)) {
    chprintf(chp, "[SDRAM][ERR] TIMEOUT: CLK_ENABLE (FMC busy)\r\n");
    return false;
  }
  chprintf(chp, "[SDRAM] WAIT: CLK_ENABLE done (FMC not busy)\r\n");

  chThdSleepMilliseconds(1);

  chprintf(chp, "[SDRAM] CMD: PALL\r\n");
  if (!fmc_issue_command(SDRAM_CMD_PALL, 1u, 0u)) {
    chprintf(chp, "[SDRAM][ERR] TIMEOUT: PALL (FMC busy)\r\n");
    return false;
  }
  chprintf(chp, "[SDRAM] WAIT: PALL done (FMC not busy)\r\n");

  chprintf(chp, "[SDRAM] CMD: AUTO_REFRESH\r\n");
  if (!fmc_issue_command(SDRAM_CMD_AUTOREFRESH, 8u, 0u)) {
    chprintf(chp, "[SDRAM][ERR] TIMEOUT: AUTO_REFRESH (FMC busy)\r\n");
    return false;
  }
  chprintf(chp, "[SDRAM] WAIT: AUTO_REFRESH done (FMC not busy)\r\n");

  chprintf(chp, "[SDRAM] CMD: LOAD_MODE\r\n");
  if (!fmc_issue_command(SDRAM_CMD_LOAD_MODE, 1u, SDRAM_MODE_REGISTER_VALUE)) {
    chprintf(chp, "[SDRAM][ERR] TIMEOUT: LOAD_MODE (FMC busy)\r\n");
    return false;
  }
  chprintf(chp, "[SDRAM] WAIT: LOAD_MODE done (FMC not busy)\r\n");

  chprintf(chp, "[SDRAM] STEP: Refresh counter set\r\n");
  uint32_t sdrtr = FMC_Bank5_6_R->SDRTR;
  sdrtr &= ~FMC_SDRTR_COUNT_Msk;
  sdrtr |= (SDRAM_REFRESH_COUNT << FMC_SDRTR_COUNT_Pos);
  FMC_Bank5_6_R->SDRTR = sdrtr;
  chprintf(chp, "[SDRAM] STEP: Refresh counter set done\r\n");

  chprintf(chp, "[SDRAM] WAIT: FMC not busy after refresh update\r\n");
  if (!fmc_wait_while_busy(SDRAM_TIMEOUT_CYCLES)) {
    chprintf(chp, "[SDRAM][ERR] TIMEOUT: Refresh update (FMC busy)\r\n");
    return false;
  }
  chprintf(chp, "[SDRAM] WAIT: FMC not busy (refresh update done)\r\n");

  const uint32_t status = FMC_Bank5_6_R->SDSR;
  if ((status & FMC_SDSR_RE) != 0u) {
    chprintf(chp, "[SDRAM][ERR] Refresh error flagged (SDSR.RE)\r\n");
    return false;
  }

  chprintf(chp, "[SDRAM] CMD: NORMAL\r\n");
  if (!fmc_issue_command(SDRAM_CMD_NORMAL, 0u, 0u)) {
    chprintf(chp, "[SDRAM][ERR] TIMEOUT: NORMAL (FMC busy)\r\n");
    return false;
  }
  chprintf(chp, "[SDRAM] WAIT: NORMAL command done (FMC not busy)\r\n");

  chprintf(chp, "[SDRAM] WAIT: NORMAL mode\r\n");
  if (!fmc_wait_for_mode(SDRAM_CMD_NORMAL, SDRAM_TIMEOUT_CYCLES)) {
    chprintf(chp, "[SDRAM][ERR] TIMEOUT: NORMAL mode (SDSR.MODE)\r\n");
    return false;
  }
  const uint32_t mode = fmc_current_mode();
  if (mode != SDRAM_CMD_NORMAL) {
    chprintf(chp, "[SDRAM][ERR] Mode mismatch after init: %lu\r\n", (unsigned long)mode);
    return false;
  }
  chprintf(chp, "[SDRAM] STEP: NORMAL mode active\r\n");

  return true;
}
