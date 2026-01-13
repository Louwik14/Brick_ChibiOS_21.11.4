/*
 * =========================================================
 * External SDRAM driver (STM32H7 + FMC, x16 bus)
 * =========================================================
 *
 * This driver provides a LOGICAL 32-bit access API to an
 * external SDRAM connected using a 16-bit FMC bus.
 *
 * On STM32H7, the FMC swaps the two 16-bit halfwords on
 * 32-bit accesses in x16 mode.
 *
 * This driver compensates this behavior by swapping the
 * halfwords back ON READ.
 *
 * Writes are performed RAW.
 *
 * =========================================================
 */

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
    FMC_SDCRx_CAS_1 |             /* CAS = 2 */
    FMC_SDCRx_SDCLK_1 |           /* HCLK / 2 */
    FMC_SDCRx_RBURST |            /* Read burst enable */
    FMC_SDCRx_RPIPE_0,            /* Read pipe delay = 1 */

  .sdtr =
    ((2U - 1U) << FMC_SDTRx_TMRD_Pos) |
    ((7U - 1U) << FMC_SDTRx_TXSR_Pos) |
    ((4U - 1U) << FMC_SDTRx_TRAS_Pos) |
    ((7U - 1U) << FMC_SDTRx_TRC_Pos)  |
    ((2U - 1U) << FMC_SDTRx_TWR_Pos)  |
    ((2U - 1U) << FMC_SDTRx_TRP_Pos)  |
    ((2U - 1U) << FMC_SDTRx_TRCD_Pos),

  .sdcmr =
    ((4U - 1U) << FMC_SDCMR_NRFS_Pos) |
    (0
      | (1U << 0)   /* burst length = 2 */
      | (0U << 3)   /* sequential */
      | (2U << 4)   /* CAS = 2 */
      | (0U << 7)   /* standard op */
      | (1U << 9)   /* single write burst */
    ),

  .sdrtr = (603U << 1)
};


/* =========================================================
 * INTERNAL UTIL
 * ========================================================= */

/*
 * Swap the two 16-bit halfwords of a 32-bit word.
 *
 * ABCD EFGH (32-bit) -> EFGH ABCD
 */
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

  /*
   * RAW write.
   * The FMC will internally reorder halfwords, but we do not
   * compensate here. The logical view is fixed on read.
   */
  mem[index] = value;
}

uint32_t sdram_ext_read32(uint32_t index) {
  volatile uint32_t *mem = (uint32_t *)SDRAM_EXT_BASE;

  /*
   * RAW read + halfword swap compensation.
   * This restores the logical 32-bit value expected by software.
   */
  return swap16(mem[index]);
}
