#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "sdram_ext.h"

static const SerialConfig uart_cfg = {
  115200, 0, USART_CR2_STOP1_BITS, 0
};

int main(void) {

  halInit();
  chSysInit();

  sdStart(&SD1, &uart_cfg);
  chprintf((BaseSequentialStream *)&SD1,
           "\r\n=== SDRAM EXT MODULE TEST ===\r\n");

  /* Init external SDRAM */
  sdram_ext_init();
  chprintf((BaseSequentialStream *)&SD1,
           "SDRAM initialized\r\n");

  /* Simple test */
  sdram_ext_write32(0, 0x11223344);
  uint32_t v = sdram_ext_read32(0);

  chprintf((BaseSequentialStream *)&SD1,
           "Read value = 0x%08lX\r\n", v);

  while (true) {
    chprintf((BaseSequentialStream *)&SD1, "Alive\r\n");
    chThdSleepMilliseconds(1000);
  }
}
