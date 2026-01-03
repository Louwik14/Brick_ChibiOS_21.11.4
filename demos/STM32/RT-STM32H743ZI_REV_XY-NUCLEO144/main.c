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

int main(void) {

  halInit();
  chSysInit();

  /* UART */
  sdStart(&SD1, &uart_cfg);
  BaseSequentialStream *chp = (BaseSequentialStream *)&SD1;

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
