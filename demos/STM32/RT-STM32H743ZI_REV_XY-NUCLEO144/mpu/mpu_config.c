/**
 * @file mpu_config.c
 * @brief MPU configuration: Phase 1 bring-up (no D-Cache, minimal MPU)
 */

#include "mpu_config.h"

#include "ch.h"
#include "hal.h"
#include "sdram_ext.h"

extern uint8_t __nocache_base__;
extern uint8_t __nocache_end__;

static bool initialized = false;

static bool mpu_compute_rasr_size(size_t size, uint32_t *rasr_size) {
  size_t region_size = 32U;
  uint32_t rasr      = MPU_RASR_SIZE_32;

  if (size < 32U) {
    return false;
  }

  while (region_size < size) {
    region_size <<= 1;
    rasr++;
  }

  *rasr_size = rasr;
  return true;
}

bool mpu_config_init_once(void) {

  if (initialized) {
    return true;
  }
  initialized = true;

#if CORTEX_MODEL == 7

  uintptr_t base = (uintptr_t)&__nocache_base__;
  size_t size    = (size_t)(&__nocache_end__ - &__nocache_base__);
  uint32_t rasr_size = 0U;

  /* Minimum MPU region size */
  if (!mpu_compute_rasr_size(size, &rasr_size)) {
    return false;
  }

  /* Disable MPU before configuration */
  mpuDisable();

  /*
   * Configure SDRAM as non-cacheable (Phase 1 contract).
   * 32MB @ 0xC0000000.
   */
  mpuConfigureRegion(
      MPU_REGION_SDRAM_MAIN,
      SDRAM_EXT_BASE,
      MPU_RASR_SIZE_32M |
      MPU_RASR_ATTR_NON_CACHEABLE
  );

  /*
   * Configure ONE region for .nocache
   * - non-cacheable
   * - background region still enabled
   */
  mpuConfigureRegion(
      MPU_REGION_D2_NOCACHE,
      base,
      rasr_size |
      MPU_RASR_ATTR_NON_CACHEABLE
  );

  /* Re-enable MPU with default memory map */
  mpuEnable(MPU_CTRL_PRIVDEFENA_Msk);

#endif

  return true;
}
