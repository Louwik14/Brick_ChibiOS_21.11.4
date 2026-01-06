#include "ch.h"
#include "hal.h"
#include "drivers/drivers.h"
#include "drv_display.h"
#include <stdio.h>

#define MUX_S0_LINE LINE_MXH_S0
#define MUX_S1_LINE LINE_MXH_S1
#define MUX_S2_LINE LINE_MXH_S2

static void mux_set(uint8_t sel) {
  palWriteLine(MUX_S0_LINE, (sel & 0x01U) ? PAL_HIGH : PAL_LOW);
  palWriteLine(MUX_S1_LINE, (sel & 0x02U) ? PAL_HIGH : PAL_LOW);
  palWriteLine(MUX_S2_LINE, (sel & 0x04U) ? PAL_HIGH : PAL_LOW);
}

int main(void) {
  halInit();
  chSysInit();

  drivers_init_all();
  drv_display_init();

  palSetLineMode(MUX_S0_LINE, PAL_MODE_OUTPUT_PUSHPULL);
  palSetLineMode(MUX_S1_LINE, PAL_MODE_OUTPUT_PUSHPULL);
  palSetLineMode(MUX_S2_LINE, PAL_MODE_OUTPUT_PUSHPULL);

  char line[32];

  while (true) {
    for (uint8_t ch = 0; ch < 8U; ch++) {
      mux_set(ch);
      chThdSleepMilliseconds(20);

      drv_display_clear();
      snprintf(line, sizeof(line), "MUX SEL: %u", (unsigned)ch);
      drv_display_draw_text(0, 0, line);
      snprintf(line, sizeof(line), "S2 S1 S0: %u%u%u",
               (unsigned)palReadLine(MUX_S2_LINE),
               (unsigned)palReadLine(MUX_S1_LINE),
               (unsigned)palReadLine(MUX_S0_LINE));
      drv_display_draw_text(0, 8, line);
      drv_display_update();
      chThdSleepMilliseconds(200);
    }
  }
}
