#ifndef SDRAM_EXT_H
#define SDRAM_EXT_H

#include <stdint.h>

/* Base et taille SDRAM */
#define SDRAM_EXT_BASE 0xC0000000U
#define SDRAM_EXT_SIZE (32 * 1024 * 1024U) /* 32 MB */

#ifdef __cplusplus
extern "C" {
#endif

void sdram_ext_init(void);

void sdram_ext_write32(uint32_t index, uint32_t value);
uint32_t sdram_ext_read32(uint32_t index);

void sdram_ext_clean_cache(void *addr, uint32_t size);
void sdram_ext_invalidate_cache(void *addr, uint32_t size);

#ifdef __cplusplus
}
#endif

#endif
