/*
 * =========================================================
 * External SDRAM access API (STM32H7 + FMC, x16 bus)
 * =========================================================
 *
 * CONTEXT
 * -------
 * The external SDRAM is connected to the STM32 FMC with a
 * 16-bit data bus (x16).
 *
 * The Cortex-M7 CPU performs 32-bit accesses.
 *
 * On STM32H7, when using FMC in x16 mode, each 32-bit access
 * is internally split into two 16-bit accesses and the FMC
 * returns the two halfwords in SWAPPED order.
 *
 * Observed RAW behavior:
 *   Write: 0x11223344
 *   Read : 0x33441122
 *
 * Therefore:
 *   Software MUST compensate this halfword swap to present
 *   a logical 32-bit memory view to the rest of the system.
 *
 *
 * DESIGN DECISION
 * ---------------
 * - Writes are performed RAW.
 * - Reads are corrected by swapping the two 16-bit halfwords.
 *
 * This yields a clean, logical 32-bit API for the rest of the
 * firmware.
 *
 *
 * IMPORTANT RULE
 * --------------
 * ❗ All 32-bit accesses to external SDRAM MUST go through
 *    this API.
 * ❗ Do NOT access the SDRAM using raw pointers elsewhere
 *    in the code.
 *
 * =========================================================
 */

#ifndef SDRAM_EXT_H
#define SDRAM_EXT_H

#include <stdint.h>
#include <stdbool.h>

/* Base address of external SDRAM (FMC bank) */
#define SDRAM_EXT_BASE   0xC0000000U

/* Initialize and start external SDRAM */
void sdram_ext_init(void);

/*
 * 32-bit access helpers (logical 32-bit view).
 *
 * index is a 32-bit word index (NOT a byte address).
 */
void     sdram_ext_write32(uint32_t index, uint32_t value);
uint32_t sdram_ext_read32(uint32_t index);

#endif /* SDRAM_EXT_H */
