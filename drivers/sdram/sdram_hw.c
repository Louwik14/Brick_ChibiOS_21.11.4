#include "ch.h"
#include "hal.h"

#include "sdram_driver.h"

/*
 * SDRAM hardware init for STM32H743 + W9825G6KH.
 * Fixed SDCLK=100MHz, no MPU/cache configuration in this module.
 */

#define SDRAM_SDCLK_HZ              100000000UL

/*
 * Timing values below are calibrated for SDCLK=100MHz and W9825G6KH.
 * FMC expects values as (cycles - 1).
 */
#define SDRAM_TMRD_CYCLES           2U
#define SDRAM_TXSR_CYCLES           7U
#define SDRAM_TRAS_CYCLES           4U
#define SDRAM_TRC_CYCLES            7U
#define SDRAM_TWR_CYCLES            2U
#define SDRAM_TRP_CYCLES            2U
#define SDRAM_TRCD_CYCLES           2U

/*
 * SDRAM refresh count for SDCLK=100MHz.
 * Formula: ((tREF / rows) * fSDCLK) - 20.
 * tREF = 64ms, rows = 8192 => 7.8125us.
 * 7.8125us * 100MHz = 781.25; 781.25 - 20 ~= 761.
 */
#define SDRAM_REFRESH_COUNT         761U

#define SDRAM_CMD_CLK_ENABLE        0x1U
#define SDRAM_CMD_PALL              0x2U
#define SDRAM_CMD_AUTO_REFRESH      0x3U
#define SDRAM_CMD_LOAD_MODE         0x4U

#define SDRAM_TARGET_BANK1          FMC_SDCMR_CTB1

#define SDRAM_MODE_BURST_LENGTH_1   0x0U
#define SDRAM_MODE_BURST_TYPE_SEQ   0x0U
#define SDRAM_MODE_CAS_LATENCY_3    (0x3U << 4)
#define SDRAM_MODE_STANDARD         0x0U
#define SDRAM_MODE_WRITEBURST_SINGLE (0x1U << 9)

static bool sdram_initialized = false;

static void sdram_delay_ms(uint32_t ms) {
  chThdSleepMilliseconds(ms);
}

static void sdram_send_command(uint32_t mode, uint32_t target, uint32_t auto_refresh, uint32_t mode_reg) {
  uint32_t command = (mode << FMC_SDCMR_MODE_Pos) |
                     target |
                     ((auto_refresh << FMC_SDCMR_NRFS_Pos) & FMC_SDCMR_NRFS) |
                     ((mode_reg << FMC_SDCMR_MRD_Pos) & FMC_SDCMR_MRD);

  FMC_Bank5_6_R->SDCMR = command;
  sdram_delay_ms(1U);
}

static void sdram_configure_timings(void) {
  uint32_t sdcr = (FMC_SDCRx_NC_0) |
                  (FMC_SDCRx_NR_1) |
                  (FMC_SDCRx_MWID_0) |
                  (FMC_SDCRx_NB) |
                  ((3U - 1U) << FMC_SDCRx_CAS_Pos) |
                  (FMC_SDCRx_SDCLK_1) |
                  (FMC_SDCRx_RBURST);

  uint32_t sdtr = ((SDRAM_TMRD_CYCLES - 1U) << FMC_SDTRx_TMRD_Pos) |
                  ((SDRAM_TXSR_CYCLES - 1U) << FMC_SDTRx_TXSR_Pos) |
                  ((SDRAM_TRAS_CYCLES - 1U) << FMC_SDTRx_TRAS_Pos) |
                  ((SDRAM_TRC_CYCLES - 1U) << FMC_SDTRx_TRC_Pos) |
                  ((SDRAM_TWR_CYCLES - 1U) << FMC_SDTRx_TWR_Pos) |
                  ((SDRAM_TRP_CYCLES - 1U) << FMC_SDTRx_TRP_Pos) |
                  ((SDRAM_TRCD_CYCLES - 1U) << FMC_SDTRx_TRCD_Pos);

  FMC_Bank5_6_R->SDCR[0] = sdcr;
  FMC_Bank5_6_R->SDTR[0] = sdtr;
}

sdram_result_t sdram_start(void) {
  rccEnableFSMC(true);
  rccResetFSMC();

  sdram_configure_timings();

  sdram_send_command(SDRAM_CMD_CLK_ENABLE, SDRAM_TARGET_BANK1, 0U, 0U);
  sdram_delay_ms(1U);

  sdram_send_command(SDRAM_CMD_PALL, SDRAM_TARGET_BANK1, 0U, 0U);
  sdram_send_command(SDRAM_CMD_AUTO_REFRESH, SDRAM_TARGET_BANK1, 8U, 0U);

  uint32_t mode = SDRAM_MODE_BURST_LENGTH_1 |
                  SDRAM_MODE_BURST_TYPE_SEQ |
                  SDRAM_MODE_CAS_LATENCY_3 |
                  SDRAM_MODE_STANDARD |
                  SDRAM_MODE_WRITEBURST_SINGLE;

  sdram_send_command(SDRAM_CMD_LOAD_MODE, SDRAM_TARGET_BANK1, 0U, mode);

  FMC_Bank5_6_R->SDRTR = (SDRAM_REFRESH_COUNT << FMC_SDRTR_COUNT_Pos);
  /* STM32H7: FMC can report BUSY/refresh activity after SDRTR update.
   * Do not poll or wait here; SDRAM is ready immediately after LOAD_MODE + SDRTR.
   */

  return SDRAM_OK;
}

void sdram_stop(void) {
  rccResetFSMC();
  rccDisableFSMC();
  sdram_initialized = false;
}

sdram_result_t sdram_init(void) {
  if (sdram_initialized) {
    return SDRAM_OK;
  }

  if (sdram_start() != SDRAM_OK) {
    return SDRAM_ERROR;
  }

  sdram_initialized = true;
  return SDRAM_OK;
}
