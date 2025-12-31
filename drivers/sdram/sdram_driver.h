#ifndef SDRAM_DRIVER_H
#define SDRAM_DRIVER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * SDRAM driver for STM32H743 + W9825G6KH.
 * NOTE: Assumes FMC SDCLK = 100MHz. No MPU/cache setup is performed here.
 */

#define SDRAM_BASE_ADDRESS      0xC0000000UL
#define SDRAM_SIZE_BYTES        (32UL * 1024UL * 1024UL)
#define SDRAM_DMA_ALIGNMENT     32U
#define SDRAM_ALIGN_ATTRIBUTE   __attribute__((aligned(SDRAM_DMA_ALIGNMENT)))

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert((SDRAM_SIZE_BYTES % sizeof(uint32_t)) == 0U,
               "SDRAM size must be word aligned.");
_Static_assert((SDRAM_DMA_ALIGNMENT & (SDRAM_DMA_ALIGNMENT - 1U)) == 0U,
               "SDRAM DMA alignment must be a power of two.");
#endif

typedef enum {
  SDRAM_OK = 0,
  SDRAM_ERROR = 1
} sdram_result_t;

typedef struct {
  uint32_t *base;
  size_t size_bytes;
  size_t errors;
  const volatile bool *abort_flag;
  uint32_t yield_interval;
} sdram_bist_context_t;

sdram_result_t sdram_init(void);
sdram_result_t sdram_start(void);
void sdram_stop(void);

sdram_result_t sdram_bist_quick(void);
sdram_result_t sdram_bist_full(sdram_bist_context_t *ctx);

#ifdef __cplusplus
}
#endif

#endif /* SDRAM_DRIVER_H */
