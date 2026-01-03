#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "mpu_config.h"

/* UART */
static const SerialConfig uart_cfg = {
  115200,
  0,
  USART_CR2_STOP1_BITS,
  0
};

/* Buffer SDMMC IDMA : NON CACHEABLE + aligné */
__attribute__((section(".nocache"), aligned(32)))
static uint8_t sdc_buf[512];

int main(void) {

  halInit();
  chSysInit();

  /* UART */
  sdStart(&SD1, &uart_cfg);
  BaseSequentialStream *chp = (BaseSequentialStream *)&SD1;

  chprintf(chp, "\r\n=== SDMMC1 TEST (H7 + SDMMCv2 + MPU) ===\r\n");

  /* MPU */
  if (!mpu_config_init_once()) {
    chprintf(chp, "MPU init FAILED\r\n");
    while (true) chThdSleepMilliseconds(1000);
  }
  chprintf(chp, "MPU OK\r\n");

  /* SD driver start (NO config struct on H7) */
  chprintf(chp, "sdcStart...\r\n");
  sdcStart(&SDCD1, NULL);

  /* Connect card */
  chprintf(chp, "sdcConnect...\r\n");
  if (sdcConnect(&SDCD1) != HAL_SUCCESS) {
    sdcflags_t e = sdcGetAndClearErrors(&SDCD1);
    chprintf(chp, "sdcConnect FAILED: 0x%08lX\r\n", (uint32_t)e);
    while (true) chThdSleepMilliseconds(1000);
  }
  chprintf(chp, "sdcConnect OK\r\n");

  /* Read first block */
  chprintf(chp, "Reading LBA0...\r\n");
  if (sdcRead(&SDCD1, 0, sdc_buf, 1) != HAL_SUCCESS) {
    sdcflags_t e = sdcGetAndClearErrors(&SDCD1);
    chprintf(chp, "sdcRead FAILED: 0x%08lX\r\n", (uint32_t)e);
    while (true) chThdSleepMilliseconds(1000);
  }

  chprintf(chp, "READ OK\r\n");
  chprintf(chp, "LBA0 first 16 bytes:\r\n");

  for (int i = 0; i < 16; i++) {
    chprintf(chp, "%02X ", sdc_buf[i]);
  }
  chprintf(chp, "\r\n");

  while (true) {
    chprintf(chp, "Alive\r\n");
    chThdSleepMilliseconds(1000);
  }
}
