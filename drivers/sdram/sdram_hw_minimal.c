#include "ch.h"
#include "hal.h"
#include "stm32h7xx.h"

#include "sdram_hw_minimal.h"

/*
 * Minimal SDRAM init for STM32H743 + W9825G6KH.
 * SDCLK = 100 MHz, MPU/cache intentionally not configured here.
 */

#define SDRAM_REFRESH_COUNT         761u

#define SDRAM_CMD_CLK_ENABLE        0x1u
#define SDRAM_CMD_PALL              0x2u
#define SDRAM_CMD_AUTO_REFRESH      0x3u
#define SDRAM_CMD_LOAD_MODE         0x4u

#define SDRAM_TARGET_BANK1          FMC_SDCMR_CTB1

#define SDRAM_MODE_BURST_LENGTH_1   0x0u
#define SDRAM_MODE_BURST_TYPE_SEQ   0x0u
#define SDRAM_MODE_CAS_LATENCY_3    (0x3u << 4)
#define SDRAM_MODE_STANDARD         0x0u
#define SDRAM_MODE_WRITEBURST_SINGLE (0x1u << 9)

#define SDRAM_TMRD_CYCLES           2u
#define SDRAM_TXSR_CYCLES           7u
#define SDRAM_TRAS_CYCLES           4u
#define SDRAM_TRC_CYCLES            7u
#define SDRAM_TWR_CYCLES            2u
#define SDRAM_TRP_CYCLES            2u
#define SDRAM_TRCD_CYCLES           2u

static void sdram_send_command(uint32_t mode, uint32_t auto_refresh, uint32_t mode_reg) {
  uint32_t refresh = 0u;

  if (auto_refresh > 0u) {
    refresh = (auto_refresh - 1u) << FMC_SDCMR_NRFS_Pos;
  }

  uint32_t command = (mode << FMC_SDCMR_MODE_Pos) |
                     SDRAM_TARGET_BANK1 |
                     (refresh & FMC_SDCMR_NRFS_Msk) |
                     ((mode_reg << FMC_SDCMR_MRD_Pos) & FMC_SDCMR_MRD_Msk);

  FMC_Bank5_6_R->SDCMR = command;
}

bool sdram_init_minimal(void) {
  if ((RCC->AHB3ENR & RCC_AHB3ENR_FMCEN) == 0u) {
    RCC->AHB3ENR |= RCC_AHB3ENR_FMCEN;
    (void)RCC->AHB3ENR;
  }

  uint32_t sdcr = FMC_SDCRx_NC_0 |
                  FMC_SDCRx_NR_1 |
                  FMC_SDCRx_MWID_0 |
                  FMC_SDCRx_NB |
                  ((3u - 1u) << FMC_SDCRx_CAS_Pos) |
                  FMC_SDCRx_SDCLK_1;

  uint32_t sdtr = ((SDRAM_TMRD_CYCLES - 1u) << FMC_SDTRx_TMRD_Pos) |
                  ((SDRAM_TXSR_CYCLES - 1u) << FMC_SDTRx_TXSR_Pos) |
                  ((SDRAM_TRAS_CYCLES - 1u) << FMC_SDTRx_TRAS_Pos) |
                  ((SDRAM_TRC_CYCLES - 1u) << FMC_SDTRx_TRC_Pos) |
                  ((SDRAM_TWR_CYCLES - 1u) << FMC_SDTRx_TWR_Pos) |
                  ((SDRAM_TRP_CYCLES - 1u) << FMC_SDTRx_TRP_Pos) |
                  ((SDRAM_TRCD_CYCLES - 1u) << FMC_SDTRx_TRCD_Pos);

  FMC_Bank5_6_R->SDCR[0] = sdcr;
  FMC_Bank5_6_R->SDTR[0] = sdtr;

  sdram_send_command(SDRAM_CMD_CLK_ENABLE, 0u, 0u);
  chThdSleepMicroseconds(100u);

  sdram_send_command(SDRAM_CMD_PALL, 0u, 0u);
  sdram_send_command(SDRAM_CMD_AUTO_REFRESH, 2u, 0u);
  sdram_send_command(SDRAM_CMD_LOAD_MODE,
                     0u,
                     SDRAM_MODE_BURST_LENGTH_1 |
                       SDRAM_MODE_BURST_TYPE_SEQ |
                       SDRAM_MODE_CAS_LATENCY_3 |
                       SDRAM_MODE_STANDARD |
                       SDRAM_MODE_WRITEBURST_SINGLE);

  FMC_Bank5_6_R->SDRTR = (SDRAM_REFRESH_COUNT << FMC_SDRTR_COUNT_Pos);

  return true;
}
