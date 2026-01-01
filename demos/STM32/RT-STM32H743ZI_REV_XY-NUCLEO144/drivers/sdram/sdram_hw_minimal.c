/*
 * Minimal SDRAM bring-up for STM32H743 + W9825G6KH.
 * - SDCLK = 100 MHz (HCLK/2)
 * - 16-bit bus, 9 columns, 13 rows, 4 banks, CAS=3
 * - Read burst disabled
 * - MPU/cache intentionally unused
 */
#include "ch.h"
#include "hal.h"
#include "stm32h7xx.h"
#include "stm32h743xx.h"

#include "sdram_hw_minimal.h"

#define SDRAM_REFRESH_COUNT (761u)

#define SDRAM_BUSY_TIMEOUT_US (1000u)

#define SDRAM_CMD_CLK_ENABLE  (1u)
#define SDRAM_CMD_PALL        (2u)
#define SDRAM_CMD_AUTOREFRESH (3u)
#define SDRAM_CMD_LOAD_MODE   (4u)

#define SDRAM_FMC_TARGET_BANK FMC_SDCMR_CTB1

#define SDRAM_MODE_BURST_LENGTH_1     (0x0u)
#define SDRAM_MODE_BURST_TYPE_SEQ     (0x0u << 3)
#define SDRAM_MODE_CAS_LATENCY_3      (0x3u << 4)
#define SDRAM_MODE_STANDARD_OPERATION (0x0u << 7)
#define SDRAM_MODE_WRITEBURST_SINGLE  (0x1u << 9)
#define SDRAM_MODE_REGISTER_VALUE     (SDRAM_MODE_BURST_LENGTH_1 |        \
                                       SDRAM_MODE_BURST_TYPE_SEQ |        \
                                       SDRAM_MODE_CAS_LATENCY_3 |         \
                                       SDRAM_MODE_STANDARD_OPERATION |    \
                                       SDRAM_MODE_WRITEBURST_SINGLE)

static void sdram_send_command(uint32_t mode, uint32_t auto_refresh, uint32_t mode_reg)
{
  const uint32_t command = ((mode << FMC_SDCMR_MODE_Pos) & FMC_SDCMR_MODE_Msk) |
                           SDRAM_FMC_TARGET_BANK |
                           ((((auto_refresh > 0u) ? (auto_refresh - 1u) : 0u) << FMC_SDCMR_NRFS_Pos) &
                            FMC_SDCMR_NRFS_Msk) |
                           ((mode_reg << FMC_SDCMR_MRD_Pos) & FMC_SDCMR_MRD_Msk);

  FMC_Bank5_6_R->SDCMR = command;
}

static bool sdram_wait_while_busy(uint32_t timeout_us)
{
  while ((FMC_Bank5_6_R->SDSR & FMC_SDSR_BUSY) != 0u) {
    if (timeout_us == 0u) {
      return false;
    }
    chThdSleepMicroseconds(1u);
    timeout_us--;
  }

  return true;
}

bool sdram_init_minimal(void)
{
  RCC->AHB3ENR |= RCC_AHB3ENR_FMCEN;
  (void)RCC->AHB3ENR;

  const uint32_t sdcr = FMC_SDCRx_NC_0 | /* 9 columns */
                        FMC_SDCRx_NR_1 | /* 13 rows */
                        FMC_SDCRx_MWID_0 | /* 16-bit bus */
                        FMC_SDCRx_NB | /* 4 internal banks */
                        ((2u << FMC_SDCRx_CAS_Pos) & FMC_SDCRx_CAS_Msk) | /* CAS latency = 3 */
                        FMC_SDCRx_SDCLK_1; /* SDCLK = HCLK/2 (2 HCLK periods) */

  const uint32_t sdtr = ((2u - 1u) << FMC_SDTRx_TMRD_Pos) |  /* tMRD = 2 cycles */
                        ((7u - 1u) << FMC_SDTRx_TXSR_Pos) |  /* tXSR = 7 cycles */
                        ((5u - 1u) << FMC_SDTRx_TRAS_Pos) |  /* tRAS = 5 cycles */
                        ((7u - 1u) << FMC_SDTRx_TRC_Pos) |   /* tRC = 7 cycles */
                        ((2u - 1u) << FMC_SDTRx_TWR_Pos) |   /* tWR = 2 cycles */
                        ((2u - 1u) << FMC_SDTRx_TRP_Pos) |   /* tRP = 2 cycles */
                        ((2u - 1u) << FMC_SDTRx_TRCD_Pos);   /* tRCD = 2 cycles */

  FMC_Bank5_6_R->SDCR[0] = sdcr;
  FMC_Bank5_6_R->SDTR[0] = sdtr;

  sdram_send_command(SDRAM_CMD_CLK_ENABLE, 0u, 0u);
  chThdSleepMicroseconds(100u);
  if (!sdram_wait_while_busy(SDRAM_BUSY_TIMEOUT_US)) {
    return false;
  }

  sdram_send_command(SDRAM_CMD_PALL, 0u, 0u);
  if (!sdram_wait_while_busy(SDRAM_BUSY_TIMEOUT_US)) {
    return false;
  }

  sdram_send_command(SDRAM_CMD_AUTOREFRESH, 8u, 0u);
  if (!sdram_wait_while_busy(SDRAM_BUSY_TIMEOUT_US)) {
    return false;
  }

  sdram_send_command(SDRAM_CMD_LOAD_MODE, 0u, SDRAM_MODE_REGISTER_VALUE);
  if (!sdram_wait_while_busy(SDRAM_BUSY_TIMEOUT_US)) {
    return false;
  }

  uint32_t sdrtr = FMC_Bank5_6_R->SDRTR;
  sdrtr &= ~FMC_SDRTR_COUNT_Msk;
  sdrtr |= (SDRAM_REFRESH_COUNT << FMC_SDRTR_COUNT_Pos) & FMC_SDRTR_COUNT_Msk;
  FMC_Bank5_6_R->SDRTR = sdrtr;
  if (!sdram_wait_while_busy(SDRAM_BUSY_TIMEOUT_US)) {
    return false;
  }

  return true;
}
