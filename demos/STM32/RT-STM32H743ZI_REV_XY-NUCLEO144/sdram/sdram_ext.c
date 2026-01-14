#include "ch.h"
#include "hal.h"
#include "hal_sdram.h"
#include "sdram_ext.h"

/* ================= SDRAM CONFIG ================= */

static const SDRAMConfig sdram_cfg = {
  .sdcr = (uint32_t)(
    FMC_ColumnBits_Number_9b |
    FMC_RowBits_Number_13b |
    FMC_SDMemory_Width_16b |
    FMC_InternalBank_Number_4 |
    FMC_CAS_Latency_2 |
    FMC_Write_Protection_Disable |
    FMC_SDClock_Period_2 |
    FMC_Read_Burst_Enable |
    FMC_ReadPipe_Delay_1
  ),

  .sdtr = (uint32_t)(
    (2 - 1) |
    (7 << 4) |
    (4 << 8) |
    (7 << 12) |
    (2 << 16) |
    (2 << 20) |
    (2 << 24)
  ),

  .sdcmr = (uint32_t)(
    ((4 - 1) << 5) |
    ((FMC_SDCMR_MRD_BURST_LENGTH_2 |
      FMC_SDCMR_MRD_BURST_TYPE_SEQUENTIAL |
      FMC_SDCMR_MRD_CAS_LATENCY_2 |
      FMC_SDCMR_MRD_OPERATING_MODE_STANDARD |
      FMC_SDCMR_MRD_WRITEBURST_MODE_SINGLE) << 9)
  ),

  .sdrtr = (uint32_t)(603 << 1),
};

void sdram_ext_init(void) {
  sdramInit();
  sdramStart(&SDRAMD1, &sdram_cfg);
}

/* ================= API ================= */

void sdram_ext_write32(uint32_t index, uint32_t value) {
  volatile uint32_t *p = (volatile uint32_t *)SDRAM_EXT_BASE;
  p[index] = value;
}

uint32_t sdram_ext_read32(uint32_t index) {
  volatile uint32_t *p = (volatile uint32_t *)SDRAM_EXT_BASE;
  return p[index];
}

void sdram_ext_clean_cache(void *addr, uint32_t size) {
  SCB_CleanDCache_by_Addr((uint32_t *)addr, size);
}

void sdram_ext_invalidate_cache(void *addr, uint32_t size) {
  SCB_InvalidateDCache_by_Addr((uint32_t *)addr, size);
}
