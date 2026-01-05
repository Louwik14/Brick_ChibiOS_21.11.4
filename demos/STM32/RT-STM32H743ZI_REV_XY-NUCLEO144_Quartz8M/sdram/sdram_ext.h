/*
 * NOTE:
 * External SDRAM is x16.
 * All 32-bit accesses MUST use the provided helpers.
 * Direct raw access will observe halfword swapping.
 */

#ifndef SDRAM_EXT_H
#define SDRAM_EXT_H

#include <stdint.h>
#include <stdbool.h>

/* Base address of external SDRAM */
#define SDRAM_EXT_BASE   0xC0000000U

/* Initialize and start external SDRAM */
void sdram_ext_init(void);

/* 32-bit access helpers (recommended) */
void     sdram_ext_write32(uint32_t index, uint32_t value);
uint32_t sdram_ext_read32(uint32_t index);

#endif /* SDRAM_EXT_H */
