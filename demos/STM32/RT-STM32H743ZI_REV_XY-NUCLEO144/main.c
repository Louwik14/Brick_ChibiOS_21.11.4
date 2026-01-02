#include "ch.h"
#include "hal.h"
#include "chprintf.h"

#include "mpu_config.h"

static const SerialConfig uart_cfg = {
  115200,
  0,
  USART_CR2_STOP1_BITS,
  0
};

/* Buffer IDMA: section non-cacheable + alignement 32B. */
__attribute__((section(".nocache"), aligned(32)))
static uint8_t sdc_buf[512];

static void dump_hex(BaseSequentialStream *chp, const uint8_t *p, size_t n) {
  for (size_t i = 0; i < n; i++) {
    chprintf(chp, "%02X%s", p[i], ((i + 1) % 16 == 0) ? "\r\n" : " ");
  }
  if (n % 16) chprintf(chp, "\r\n");
}

int main(void) {
  halInit();
  chSysInit();

  sdStart(&SD1, &uart_cfg);
  BaseSequentialStream *chp = (BaseSequentialStream *)&SD1;

  if (!mpu_config_init_once()) {
    chprintf(chp, "MPU init failed (nocache region).\r\n");
    while (true) chThdSleepMilliseconds(1000);
  }

  chprintf(chp, "\r\n=== SDMMC1 RAW BLOCK TEST (NOCACHE BUF) ===\r\n");

  uint8_t *buf = sdc_buf;

  /* Sanity: card detect (si le LLD le supporte) */
  chprintf(chp, "Card inserted? %s\r\n", sdcIsCardInserted(&SDCD1) ? "YES" : "NO");

  chprintf(chp, "sdcStart...\r\n");
  sdcStart(&SDCD1, NULL);

  chprintf(chp, "sdcConnect...\r\n");
  if (sdcConnect(&SDCD1) != HAL_SUCCESS) {
    sdcflags_t e = sdcGetAndClearErrors(&SDCD1);
    chprintf(chp, "sdcConnect FAILED errors=0x%08lX\r\n", (uint32_t)e);
    while (true) chThdSleepMilliseconds(1000);
  }
  chprintf(chp, "sdcConnect OK\r\n");

  chprintf(chp, "Reading LBA0...\r\n");
  if (sdcRead(&SDCD1, 0, buf, 1) != HAL_SUCCESS) {
    sdcflags_t e = sdcGetAndClearErrors(&SDCD1);
    chprintf(chp, "sdcRead FAILED errors=0x%08lX\r\n", (uint32_t)e);
    while (true) chThdSleepMilliseconds(1000);
  }

#if CORTEX_MODEL == 7
  /* Si DCache est ON, invalidation pour voir ce que le DMA a écrit */
  SCB_InvalidateDCache_by_Addr((uint32_t *)((uint32_t)buf & ~31U), 512 + 32);
#endif

  chprintf(chp, "LBA0 first 32 bytes:\r\n");
  dump_hex(chp, buf, 32);

  while (true) {
    chprintf(chp, "Alive\r\n");
    chThdSleepMilliseconds(1000);
  }
}
