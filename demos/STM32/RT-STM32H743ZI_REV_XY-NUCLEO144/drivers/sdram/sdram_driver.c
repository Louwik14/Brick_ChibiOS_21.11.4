#include "ch.h"
#include "hal.h"

#include "sdram_driver.h"
#include "sdram_driver_priv.h"
#include "sdram_layout.h"

/* Global driver context */
sdram_driver_ctx_t sdram_ctx = {
  .state = SDRAM_NOT_INITIALIZED,
  .last_error = SDRAM_ERR_NONE,
  .bist_running = false,
  .last_bist_result = {0}
};

MUTEX_DECL(sdram_ctx_mtx);

/* ---- internal helpers --------------------------------------------------- */

static void sdram_set_fault_locked(sdram_error_t error) {
  sdram_ctx.state = SDRAM_FAULT;
  sdram_ctx.last_error = error;
}

static void sdram_clear_region_info(sdram_region_info_t *info) {
  if (!info) return;
  info->id = SDRAM_REGION_INVALID;
  info->base = 0u;
  info->size_bytes = 0u;
  info->flags = 0u;
  info->alignment_bytes = 0u;
}

/* ---- public API ---------------------------------------------------------- */

void sdram_init(bool run_quick_bist) {

  /* Single init guard */
  chMtxLock(&sdram_ctx_mtx);
  if (sdram_ctx.state != SDRAM_NOT_INITIALIZED) {
    chMtxUnlock(&sdram_ctx_mtx);
    return;
  }
  sdram_ctx.state = SDRAM_INITIALIZING;
  sdram_ctx.last_error = SDRAM_ERR_NONE;
  sdram_ctx.bist_running = false;
  chMtxUnlock(&sdram_ctx_mtx);

  /* ---- FMC / SDRAM HW init ---- */
  if (!sdram_hw_init_sequence()) {
    chMtxLock(&sdram_ctx_mtx);
    sdram_set_fault_locked(SDRAM_ERR_FMC_TIMEOUT);
    chMtxUnlock(&sdram_ctx_mtx);
    return;
  }

  /* ✅ IMPORTANT :
   * SDRAM is USABLE immediately after HW init on STM32H7
   */
  chMtxLock(&sdram_ctx_mtx);
  sdram_ctx.state = SDRAM_READY;
  sdram_ctx.last_error = SDRAM_ERR_NONE;
  chMtxUnlock(&sdram_ctx_mtx);

  /* ---- Optional QUICK BIST ---- */
  if (!run_quick_bist) {
    return;
  }

  sdram_bist_context_t bist_ctx = {0};
  bist_ctx.mode = SDRAM_BIST_MODE_QUICK;
  bist_ctx.abort_flag = NULL;

  chMtxLock(&sdram_ctx_mtx);
  sdram_ctx.bist_running = true;
  chMtxUnlock(&sdram_ctx_mtx);

  bool ok = sdram_bist_start(&bist_ctx);

  chMtxLock(&sdram_ctx_mtx);
  sdram_ctx.bist_running = false;
  sdram_ctx.last_bist_result = bist_ctx.result;

  if (!ok || bist_ctx.result.status != SDRAM_BIST_PASS) {
    sdram_ctx.state = SDRAM_DEGRADED;     /* ⚠️ usable but warn */
    sdram_ctx.last_error = SDRAM_ERR_BIST_FAIL;
  }
  chMtxUnlock(&sdram_ctx_mtx);
}

bool sdram_is_initialized(void) {
  chMtxLock(&sdram_ctx_mtx);
  bool ok = (sdram_ctx.state == SDRAM_READY ||
             sdram_ctx.state == SDRAM_DEGRADED);
  chMtxUnlock(&sdram_ctx_mtx);
  return ok;
}

sdram_state_t sdram_status(void) {
  chMtxLock(&sdram_ctx_mtx);
  sdram_state_t st = sdram_ctx.state;
  chMtxUnlock(&sdram_ctx_mtx);
  return st;
}

sdram_error_t sdram_get_error(void) {
  chMtxLock(&sdram_ctx_mtx);
  sdram_error_t err = sdram_ctx.last_error;
  chMtxUnlock(&sdram_ctx_mtx);
  return err;
}

bool sdram_run_bist(sdram_bist_mode_t mode, sdram_bist_result_t *out_result) {

  chMtxLock(&sdram_ctx_mtx);
  if (sdram_ctx.bist_running || sdram_ctx.state == SDRAM_NOT_INITIALIZED) {
    chMtxUnlock(&sdram_ctx_mtx);
    return false;
  }
  sdram_ctx.bist_running = true;
  chMtxUnlock(&sdram_ctx_mtx);

  sdram_bist_context_t ctx = {0};
  ctx.mode = mode;
  ctx.abort_flag = NULL;

  bool ok = sdram_bist_start(&ctx);

  chMtxLock(&sdram_ctx_mtx);
  sdram_ctx.bist_running = false;
  sdram_ctx.last_bist_result = ctx.result;

  if (!ok || ctx.result.status != SDRAM_BIST_PASS) {
    sdram_ctx.state = (mode == SDRAM_BIST_MODE_FULL)
                        ? SDRAM_FAULT
                        : SDRAM_DEGRADED;
    sdram_ctx.last_error = SDRAM_ERR_BIST_FAIL;
  }

  if (out_result) {
    *out_result = ctx.result;
  }
  chMtxUnlock(&sdram_ctx_mtx);

  return ok;
}

bool sdram_get_region(sdram_region_id_t region_id,
                      sdram_region_info_t *out_info) {

  if (!out_info) return false;

  chMtxLock(&sdram_ctx_mtx);
  if (sdram_ctx.state == SDRAM_FAULT || sdram_ctx.bist_running) {
    chMtxUnlock(&sdram_ctx_mtx);
    sdram_clear_region_info(out_info);
    return false;
  }
  chMtxUnlock(&sdram_ctx_mtx);

  if (!sdram_query_region_descriptor(region_id, out_info)) {
    sdram_clear_region_info(out_info);
    return false;
  }

  return true;
}
