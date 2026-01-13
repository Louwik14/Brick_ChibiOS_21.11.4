#include "ch.h"
#include "hal.h"

#include "sdram_ext.h"
#include "drv_display.h"

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

/* =========================================================
 * Main diagnostic SDRAM BUS
 * ========================================================= */

int main(void) {

  halInit();
  chSysInit();

  drv_display_init();
  sdram_ext_init();

  chThdSleepMilliseconds(500);

  char line[32];

  volatile uint16_t *m16 = (volatile uint16_t *)SDRAM_EXT_BASE;
  volatile uint32_t *m32 = (volatile uint32_t *)SDRAM_EXT_BASE;

  /* =====================================================
   * Phase 1 : test 16-bit direct
   * ===================================================== */

  drv_display_clear();
  drv_display_draw_text(0, 0, "SDRAM BUS TEST");
  drv_display_draw_text(0, 12, "16-bit test...");
  drv_display_update();

  /* Write pattern */
  m16[0] = 0x1111;
  m16[1] = 0x2222;
  m16[2] = 0x3333;
  m16[3] = 0x4444;

  /* Read back */
  uint16_t a = m16[0];
  uint16_t b = m16[1];
  uint16_t c = m16[2];
  uint16_t d = m16[3];

  chThdSleepMilliseconds(100);

  drv_display_clear();
  drv_display_draw_text(0, 0, "16-bit READ:");

  snprintf(line, sizeof(line), "0: %04X", a);
  drv_display_draw_text(0, 12, line);

  snprintf(line, sizeof(line), "1: %04X", b);
  drv_display_draw_text(0, 24, line);

  snprintf(line, sizeof(line), "2: %04X", c);
  drv_display_draw_text(0, 36, line);

  snprintf(line, sizeof(line), "3: %04X", d);
  drv_display_draw_text(0, 48, line);

  drv_display_update();
  chThdSleepMilliseconds(3000);

  /* =====================================================
   * Phase 2 : test bits individuels
   * ===================================================== */

  drv_display_clear();
  drv_display_draw_text(0, 0, "BIT TEST");

  m16[0] = 0x0001; // bit 0
  uint16_t t0 = m16[0];

  m16[0] = 0x0100; // bit 8
  uint16_t t8 = m16[0];

  snprintf(line, sizeof(line), "B0: %04X", t0);
  drv_display_draw_text(0, 16, line);

  snprintf(line, sizeof(line), "B8: %04X", t8);
  drv_display_draw_text(0, 32, line);

  drv_display_update();
  chThdSleepMilliseconds(3000);

  /* =====================================================
   * Phase 3 : test 32-bit RAW
   * ===================================================== */

  drv_display_clear();
  drv_display_draw_text(0, 0, "32-bit RAW");

  m32[0] = 0x11223344;
  uint32_t r = m32[0];

  snprintf(line, sizeof(line), "W:11223344");
  drv_display_draw_text(0, 16, line);

  snprintf(line, sizeof(line), "R:%08lX", r);
  drv_display_draw_text(0, 32, line);

  drv_display_update();
  chThdSleepMilliseconds(3000);

  /* =====================================================
   * Boucle infinie
   * ===================================================== */

  while (true) {
    chThdSleepMilliseconds(1000);
  }
}
