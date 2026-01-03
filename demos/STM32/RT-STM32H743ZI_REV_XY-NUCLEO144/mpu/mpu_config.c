/**
 * @file mpu_config.c
 * @brief MPU configuration: mark .nocache as non-cacheable (ChibiOS-safe)
 */

#include "mpu_config.h"

#include "ch.h"
#include "hal.h"

extern uint8_t __nocache_base__;
extern uint8_t __nocache_end__;

static bool initialized = false;

bool mpu_config_init_once(void) {

  if (initialized) {
    return true;
  }
  initialized = true;

#if CORTEX_MODEL == 7

  uintptr_t base = (uintptr_t)&__nocache_base__;
  size_t size    = (size_t)(&__nocache_end__ - &__nocache_base__);

  /* Minimum MPU region size */
  if (size < 32U) {
    return false;
  }

  /* Compute power-of-two region size */
  size_t region_size = 32U;
  uint32_t rasr_size = MPU_RASR_SIZE_32;

  while (region_size < size) {
    region_size <<= 1;
    rasr_size++;
  }

  /* Disable MPU before configuration */
  mpuDisable();

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
