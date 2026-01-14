#include "ch.h"
#include "hal.h"
#include "chprintf.h"

/* ================= UART ================= */

static SerialConfig uart_cfg = {
  115200,
  0,
  USART_CR2_STOP1_BITS,
  0
};

/* ================= UTILS ================= */

static void hexdump(BaseSequentialStream *chp, const uint8_t *buf, uint32_t len) {
  for (uint32_t i = 0; i < len; i++) {
    if ((i & 15) == 0) {
      chprintf(chp, "\r\n%04lX: ", i);
    }
    chprintf(chp, "%02X ", buf[i]);
  }
  chprintf(chp, "\r\n");
}

/* ================= MAIN ================= */

int main(void) {

  halInit();
  chSysInit();

  sdStart(&SD1, &uart_cfg);
  BaseSequentialStream *chp = (BaseSequentialStream*)&SD1;

  chprintf(chp, "\r\n==============================\r\n");
  chprintf(chp, " SDMMC RAW DRIVER TEST (NO FS)\r\n");
  chprintf(chp, "==============================\r\n");

  chprintf(chp, "Connecting to SD card...\r\n");

  if (sdcConnect(&SDCD1) != HAL_SUCCESS) {
    chprintf(chp, "❌ sdcConnect() failed\r\n");
    while (1) chThdSleepMilliseconds(1000);
  }

  chprintf(chp, "✅ Card connected\r\n");

  uint32_t sectors = SDCD1.capacity;
  uint64_t size_mb = ((uint64_t)sectors * 512) / (1024 * 1024);

  chprintf(chp, "Card capacity: %lu sectors = %llu MB\r\n", sectors, size_mb);

  static uint8_t block[512] __attribute__((aligned(32)));

  chprintf(chp, "\r\nReading block 0...\r\n");

  if (sdcRead(&SDCD1, 0, block, 1) != HAL_SUCCESS) {
    chprintf(chp, "❌ Read block 0 failed\r\n");
    while (1);
  }

  chprintf(chp, "Block 0 content:");
  hexdump(chp, block, 128);

  chprintf(chp, "\r\nReading block 1...\r\n");

  if (sdcRead(&SDCD1, 1, block, 1) != HAL_SUCCESS) {
    chprintf(chp, "❌ Read block 1 failed\r\n");
    while (1);
  }

  chprintf(chp, "Block 1 content:");
  hexdump(chp, block, 128);

  chprintf(chp, "\r\n✅ SDMMC RAW TEST DONE\r\n");

  while (1) {
    chThdSleepMilliseconds(1000);
  }
}
