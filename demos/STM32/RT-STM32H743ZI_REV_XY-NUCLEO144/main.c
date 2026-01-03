#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "sdram_ext.h"

/* UART1 = SD1 */
static const SerialConfig uart_cfg = {
  115200,
  0,
  USART_CR2_STOP1_BITS,
  0
};

int main(void) {

  halInit();
  chSysInit();

  sdStart(&SD1, &uart_cfg);
  BaseSequentialStream *chp = (BaseSequentialStream *)&SD1;

  chprintf(chp, "\r\n=== SDRAM RAW OBSERVATION TEST ===\r\n");

  /* Init SDRAM (TON DRIVER, inchangé) */
  chprintf(chp, "Init SDRAM...\r\n");
  sdram_ext_init();
  chprintf(chp, "SDRAM init done\r\n");

  /* Valeurs de test */
  uint32_t values[] = {
    0x00000000,
    0xFFFFFFFF,
    0x11223344,
    0xA5A5A5A5,
    0x5A5A5A5A
  };

  /* On teste plusieurs index */
  for (uint32_t idx = 0; idx < 8; idx++) {

    chprintf(chp, "\r\n-- INDEX %lu --\r\n", idx);

    for (uint32_t v = 0; v < (sizeof(values)/sizeof(values[0])); v++) {

      uint32_t write_val = values[v];
      uint32_t read_val  = 0;

      sdram_ext_write32(idx, write_val);
      read_val = sdram_ext_read32(idx);

      chprintf(
        chp,
        "W: 0x%08lX  R: 0x%08lX\r\n",
        write_val,
        read_val
      );
    }
  }

  chprintf(chp, "\r\n=== END OF TEST ===\r\n");

  while (true) {
    chThdSleepMilliseconds(1000);
  }
}
