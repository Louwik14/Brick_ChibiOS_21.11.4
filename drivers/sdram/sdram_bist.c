#include "ch.h"
#include "hal.h"

#include "sdram_driver.h"

/*
 * SDRAM BIST helpers for STM32H743 + W9825G6KH.
 * Fixed SDCLK=100MHz, no MPU/cache configuration in this module.
 */

#define SDRAM_BIST_DEFAULT_YIELD_INTERVAL 4096U
#define SDRAM_BIST_QUICK_WORDS            1024U

static bool sdram_bist_should_abort(const sdram_bist_context_t *ctx) {
  return (ctx != NULL) && (ctx->abort_flag != NULL) && (*(ctx->abort_flag));
}

static void sdram_bist_yield_if_needed(const sdram_bist_context_t *ctx, size_t index) {
  uint32_t interval = SDRAM_BIST_DEFAULT_YIELD_INTERVAL;

  if (ctx != NULL && ctx->yield_interval > 0U) {
    interval = ctx->yield_interval;
  }

  if (interval > 0U && (index % interval) == 0U) {
    chThdYield();
  }
}

static sdram_result_t sdram_bist_run(uint32_t *base, size_t words, sdram_bist_context_t *ctx) {
  if (base == NULL || words == 0U) {
    return SDRAM_ERROR;
  }

  if (((uintptr_t)base % SDRAM_DMA_ALIGNMENT) != 0U) {
    return SDRAM_ERROR;
  }

  const uint32_t patterns[] = {
    0x00000000U,
    0xFFFFFFFFU,
    0xA5A5A5A5U,
    0x5A5A5A5AU
  };

  size_t errors = 0U;

  for (size_t p = 0U; p < (sizeof(patterns) / sizeof(patterns[0])); ++p) {
    uint32_t pattern = patterns[p];

    for (size_t i = 0U; i < words; ++i) {
      base[i] = pattern;
      if ((i & 0xFFU) == 0U) {
        sdram_bist_yield_if_needed(ctx, i);
        if (sdram_bist_should_abort(ctx)) {
          if (ctx != NULL) {
            ctx->errors = errors;
          }
          return SDRAM_ERROR;
        }
      }
    }

    for (size_t i = 0U; i < words; ++i) {
      if (base[i] != pattern) {
        ++errors;
      }
      if ((i & 0xFFU) == 0U) {
        sdram_bist_yield_if_needed(ctx, i);
        if (sdram_bist_should_abort(ctx)) {
          if (ctx != NULL) {
            ctx->errors = errors;
          }
          return SDRAM_ERROR;
        }
      }
    }
  }

  if (ctx != NULL) {
    ctx->errors = errors;
  }

  return (errors == 0U) ? SDRAM_OK : SDRAM_ERROR;
}

sdram_result_t sdram_bist_quick(void) {
  uint32_t *base = (uint32_t *)SDRAM_BASE_ADDRESS;
  size_t words = SDRAM_BIST_QUICK_WORDS;

  return sdram_bist_run(base, words, NULL);
}

sdram_result_t sdram_bist_full(sdram_bist_context_t *ctx) {
  if (ctx == NULL || ctx->base == NULL || ctx->size_bytes == 0U) {
    return SDRAM_ERROR;
  }

  if ((ctx->size_bytes % sizeof(uint32_t)) != 0U) {
    return SDRAM_ERROR;
  }

  return sdram_bist_run(ctx->base, ctx->size_bytes / sizeof(uint32_t), ctx);
}
