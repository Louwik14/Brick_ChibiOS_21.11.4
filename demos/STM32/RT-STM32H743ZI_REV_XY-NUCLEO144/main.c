#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "ff.h"
#include <string.h>

/* UART */
static const SerialConfig uart_cfg = {
  115200,
  0,
  USART_CR2_STOP1_BITS,
  0
};

/* MMC_SPI driver instance (required by FatFS bindings). */
MMCDriver MMCD1;

/* Low speed SPI configuration for SD init (very conservative). */
static SPIConfig sd_spi_cfg_low = {
  .circular = false,
  .slave    = false,
  .data_cb  = NULL,
  .error_cb = NULL,
  .ssport   = GPIOC,
  .sspad    = GPIOC_SD_SPI_CS,
  .cr1      = SPI_CR1_BR_2 | SPI_CR1_BR_1 | SPI_CR1_BR_0,
  .cr2      = 0U
};

/* High speed SPI configuration for SD transfers (<= 12 MHz target). */
static SPIConfig sd_spi_cfg_high = {
  .circular = false,
  .slave    = false,
  .data_cb  = NULL,
  .error_cb = NULL,
  .ssport   = GPIOC,
  .sspad    = GPIOC_SD_SPI_CS,
  .cr1      = SPI_CR1_BR_2 | SPI_CR1_BR_1,
  .cr2      = 0U
};

/* MMC/SD over SPI configuration. */
static MMCConfig mmc_cfg = {&SPID3, &sd_spi_cfg_low, &sd_spi_cfg_high};

static void fatal(BaseSequentialStream *chp, const char *msg, uint32_t code) {
  chprintf(chp, "%s: 0x%08lX\r\n", msg, (uint32_t)code);
  while (true) {
    chprintf(chp, "FAILED\r\n");
    chThdSleepMilliseconds(1000);
  }
}

static void sdram_test(BaseSequentialStream *chp) {
  const uint32_t test_index = 0U;
  const uint32_t test_value = 0x11223344U;
  uint32_t read_value = 0U;
  uint32_t expected = 0U;

  /* SDRAM x16 swaps halfwords on 32-bit accesses; compare logically. */
  expected = (test_value >> 16) | (test_value << 16);

  /* SDRAM init + minimal functional test (32-bit only, wrapper mandatory). */
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

  chprintf(chp, "\r\n=== SPI-SD TEST (MMC_SPI + FatFS) ===\r\n");

  chprintf(chp, "SPI-SD init...\r\n");
  mmcStart(&MMCD1, &mmc_cfg);

  chprintf(chp, "SPI-SD connect...\r\n");
  if (mmcConnect(&MMCD1)) {
    fatal(chp, "SPI-SD connect FAILED", 1U);
  }

  chprintf(chp, "SD mount...\r\n");
  FATFS fs;
  FRESULT fr = f_mount(&fs, "/", 1);
  if (fr != FR_OK) {
    fatal(chp, "SD mount FAILED", fr);
  }
  chprintf(chp, "SD mount OK\r\n");

  FIL file;
  UINT bytes = 0;
  const char *msg = "hello spi sd\r\n";
  char read_buf[32] = {0};

  chprintf(chp, "Write test.txt...\r\n");
  fr = f_open(&file, "test.txt", FA_WRITE | FA_CREATE_ALWAYS);
  if (fr != FR_OK) {
    fatal(chp, "Write open FAILED", fr);
  }
  fr = f_write(&file, msg, strlen(msg), &bytes);
  if (fr != FR_OK || bytes == 0U) {
    fatal(chp, "Write FAILED", fr);
  }
  f_close(&file);
  chprintf(chp, "Write OK\r\n");

  chprintf(chp, "Read test.txt...\r\n");
  fr = f_open(&file, "test.txt", FA_READ);
  if (fr != FR_OK) {
    fatal(chp, "Read open FAILED", fr);
  }
  fr = f_read(&file, read_buf, sizeof(read_buf) - 1U, &bytes);
  if (fr != FR_OK) {
    fatal(chp, "Read FAILED", fr);
  }
  f_close(&file);
  read_buf[sizeof(read_buf) - 1U] = '\0';

  chprintf(chp, "Read OK\r\n");
  chprintf(chp, "Content: \"%s\"\r\n", read_buf);

  while (true) {
    chprintf(chp, "Alive\r\n");
    chThdSleepMilliseconds(1000);
  }
}
