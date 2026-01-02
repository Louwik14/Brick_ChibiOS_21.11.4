#ifndef SDRAM_HW_MINIMAL_H
#define SDRAM_HW_MINIMAL_H

#include <stdbool.h>
#include <stdint.h>

/*
 * Initialize external SDRAM connected to FMC SDRAM Bank1 (NE0).
 *
 * Assumptions:
 * - FMC GPIO pinout already configured by ChibiOS board files
 * - FMC clock enabled internally
 * - No MPU, no cache
 *
 * Returns:
 * - true  : initialization sequence completed
 * - false : (reserved for future error handling)
 */
bool sdram_init_minimal(void);

#endif /* SDRAM_HW_MINIMAL_H */
