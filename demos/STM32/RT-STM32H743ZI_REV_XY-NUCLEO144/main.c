#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "mpu_config.h"
#include "sdram_ext.h"

/* UART */
static const SerialConfig uart_cfg = {
  115200,
  0,
  USART_CR2_STOP1_BITS,
  0
};

/* Buffer SDMMC IDMA : NON CACHEABLE + aligné */
__attribute__((section(".nocache"), aligned(32)))
static uint8_t sdc_buf_tx[512];

__attribute__((section(".nocache"), aligned(32)))
static uint8_t sdc_buf_rx[512];

static void sdram_test(BaseSequentialStream *chp) {
  const uint32_t test_index = 0U;
  const uint32_t test_value = 0x11223344U;
  uint32_t read_value = 0U;
  uint32_t expected = 0U;

  /* SDRAM x16 swaps halfwords on 32-bit accesses */
  expected = (test_value >> 16) | (test_value << 16);

  chprintf(chp, "SDRAM init...\r\n");
  sdram_ext_init();

  chprintf(chp, "SDRAM test write/read...\r\n");
  sdram_ext_write32(test_index, test_value);
  read_value = sdram_ext_read32(test_index);

  if (read_value != expected) {
    chprintf(chp, "SDRAM test FAILED: 0x%08lX\r\n", read_value);
    while (true) {
      chThdSleepMilliseconds(1000);
    }
  }

  chprintf(chp, "SDRAM test OK\r\n");
}

static void sdmmc_test(BaseSequentialStream *chp) {
  chprintf(chp, "\r\n=== SDMMC1 TEST (H7 + SDMMCv2 + MPU) ===\r\n");

  for (size_t i = 0; i < sizeof(sdc_buf_tx); i++) {
    sdc_buf_tx[i] = (uint8_t)(i & 0xFFU);
    sdc_buf_rx[i] = 0U;
  }

  chprintf(chp, "sdcStart...\r\n");
  sdcStart(&SDCD1, NULL);

  chprintf(chp, "sdcConnect...\r\n");
  if (sdcConnect(&SDCD1) != HAL_SUCCESS) {
    sdcflags_t e = sdcGetAndClearErrors(&SDCD1);
    chprintf(chp, "sdcConnect FAILED: 0x%08lX\r\n", (uint32_t)e);
    while (true) chThdSleepMilliseconds(1000);
  }
  chprintf(chp, "sdcConnect OK\r\n");

  chprintf(chp, "Writing LBA0...\r\n");
  if (sdcWrite(&SDCD1, 0, sdc_buf_tx, 1) != HAL_SUCCESS) {
    sdcflags_t e = sdcGetAndClearErrors(&SDCD1);
    chprintf(chp, "sdcWrite FAILED: 0x%08lX\r\n", (uint32_t)e);
    while (true) chThdSleepMilliseconds(1000);
  }

  chprintf(chp, "Reading LBA0...\r\n");
  if (sdcRead(&SDCD1, 0, sdc_buf_rx, 1) != HAL_SUCCESS) {
    sdcflags_t e = sdcGetAndClearErrors(&SDCD1);
    chprintf(chp, "sdcRead FAILED: 0x%08lX\r\n", (uint32_t)e);
    while (true) chThdSleepMilliseconds(1000);
  }

  for (size_t i = 0; i < sizeof(sdc_buf_tx); i++) {
    if (sdc_buf_rx[i] != sdc_buf_tx[i]) {
      chprintf(chp, "SDMMC verify FAILED at %u\r\n", (unsigned int)i);
      while (true) chThdSleepMilliseconds(1000);
    }
  }

  chprintf(chp, "SDMMC test OK\r\n");
}

int main(void) {

  halInit();
  chSysInit();

  sdStart(&SD1, &uart_cfg);
  BaseSequentialStream *chp = (BaseSequentialStream *)&SD1;

  if (!mpu_config_init_once()) {
    chprintf(chp, "MPU init FAILED\r\n");
    while (true) chThdSleepMilliseconds(1000);
  }
  chprintf(chp, "MPU OK\r\n");

  /* Phase 2 validation order */
  sdram_test(chp);
  sdmmc_test(chp);

  while (true) {
    chprintf(chp, "Alive\r\n");
    chThdSleepMilliseconds(1000);
  }
}
