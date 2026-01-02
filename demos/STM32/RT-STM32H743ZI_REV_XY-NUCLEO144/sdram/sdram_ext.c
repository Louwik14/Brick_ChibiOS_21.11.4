#include "ch.h"
#include "hal.h"
#include "sdram_ext.h"

/* =========================================================
 * SDRAM CONFIGURATION (STM32H7 + W9825G6KH, x16)
 * ========================================================= */

static const SDRAMConfig sdram_cfg = {
  .sdcr =
    FMC_SDCRx_NC_0 |              /* 9 column bits */
    FMC_SDCRx_NR_1 |              /* 13 row bits */
    FMC_SDCRx_MWID_0 |            /* x16 */
    FMC_SDCRx_NB |                /* 4 banks */
    (2U << FMC_SDCRx_CAS_Pos) |   /* CAS latency = 3 */
    FMC_SDCRx_SDCLK_1,            /* HCLK / 2 */

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

/* =========================================================
 * INTERNAL UTIL
 * ========================================================= */

/* Swap halfwords (x16 SDRAM + 32-bit access) */
static inline uint32_t swap16(uint32_t v) {
  return (v >> 16) | (v << 16);
}

/* =========================================================
 * PUBLIC API
 * ========================================================= */

void sdram_ext_init(void) {

  /* Initialize SDRAM subsystem */
  sdramInit();

  /* Start SDRAM with fixed configuration */
  sdramStart(&SDRAMD1, &sdram_cfg);
}

void sdram_ext_write32(uint32_t index, uint32_t value) {

  volatile uint32_t *mem = (uint32_t *)SDRAM_EXT_BASE;

  /* Write with halfword swap */
  mem[index] = swap16(value);
}

uint32_t sdram_ext_read32(uint32_t index) {

  volatile uint32_t *mem = (uint32_t *)SDRAM_EXT_BASE;

  /* Read and unswap */
  return swap16(mem[index]);
}
