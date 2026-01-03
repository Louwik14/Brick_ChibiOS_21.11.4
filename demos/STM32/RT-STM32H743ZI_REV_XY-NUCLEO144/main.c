#include "ch.h"
#include "hal.h"
#include <string.h>

/* ===================== OLED PINS (TON PINOUT) ===================== */

#define OLED_CS_PORT   GPIOE
#define OLED_CS_PAD    2U

#define OLED_DC_PORT   GPIOD
#define OLED_DC_PAD    11U

#define OLED_RST_PORT  GPIOF
#define OLED_RST_PAD   6U

/* ===================== SPI CONFIG ===================== */
/* SPI MODE0, polling, CS manuel */
static const SPIConfig spicfg_oled = {
  .circular = false,
  .slave    = false,
  .data_cb  = NULL,
  .error_cb = NULL,
  .cfg1     = SPI_CFG1_MBR_2 |           /* prescaler */
              SPI_CFG1_DSIZE_VALUE(7),  /* 8 bits */
  .cfg2     = SPI_CFG2_MASTER |
              SPI_CFG2_SSM               /* NSS software */
};

/* ===================== GPIO SPI FORCE ===================== */
/* >>> CRITIQUE SUR STM32H7 <<< */

static void force_spi5_pins(void) {
  /* SPI5 = PF7 (SCK), PF8 (MISO), PF9 (MOSI), AF5 */
  palSetPadMode(GPIOF, 7U, PAL_MODE_ALTERNATE(5) | PAL_STM32_OSPEED_HIGHEST);
  palSetPadMode(GPIOF, 8U, PAL_MODE_ALTERNATE(5) | PAL_STM32_OSPEED_HIGHEST);
  palSetPadMode(GPIOF, 9U, PAL_MODE_ALTERNATE(5) | PAL_STM32_OSPEED_HIGHEST);
}

/* ===================== LOW LEVEL ===================== */

static inline void cs_low(void)  { palClearPad(OLED_CS_PORT, OLED_CS_PAD); }
static inline void cs_high(void) { palSetPad  (OLED_CS_PORT, OLED_CS_PAD); }
static inline void dc_cmd(void)  { palClearPad(OLED_DC_PORT, OLED_DC_PAD); }
static inline void dc_data(void) { palSetPad  (OLED_DC_PORT, OLED_DC_PAD); }

static void oled_cmd(uint8_t c) {
  dc_cmd();
  cs_low();
  spiPolledExchange(&SPID5, c);
  cs_high();
}

static void oled_data(uint8_t d) {
  dc_data();
  cs_low();
  spiPolledExchange(&SPID5, d);
  cs_high();
}

/* ===================== SSD1309 INIT ===================== */

static void oled_init(void) {
  palClearPad(OLED_RST_PORT, OLED_RST_PAD);
  chThdSleepMilliseconds(50);
  palSetPad(OLED_RST_PORT, OLED_RST_PAD);
  chThdSleepMilliseconds(50);

  oled_cmd(0xAE);
  oled_cmd(0xD5); oled_cmd(0x80);
  oled_cmd(0xA8); oled_cmd(0x3F);
  oled_cmd(0xD3); oled_cmd(0x00);
  oled_cmd(0x40);
  oled_cmd(0x8D); oled_cmd(0x14);
  oled_cmd(0x20); oled_cmd(0x00);
  oled_cmd(0xA1);
  oled_cmd(0xC8);
  oled_cmd(0xDA); oled_cmd(0x12);
  oled_cmd(0xAF);
}

static void oled_clear(void) {
  for (uint8_t page = 0; page < 8; page++) {
    oled_cmd(0xB0 + page);
    oled_cmd(0x00);
    oled_cmd(0x10);
    for (uint8_t i = 0; i < 128; i++) {
      oled_data(0x00);
    }
  }
}

/* ===================== FONT MINIMAL ===================== */

static const uint8_t font_5x7[][5] = {
  {0x7F,0x08,0x08,0x08,0x7F}, /* H */
  {0x7F,0x49,0x49,0x49,0x41}, /* E */
  {0x7F,0x01,0x01,0x01,0x01}, /* L */
  {0x3E,0x41,0x41,0x41,0x3E}, /* O */
  {0x7F,0x02,0x04,0x02,0x7F}, /* W */
  {0x7F,0x48,0x4C,0x4A,0x31}, /* R */
  {0x7F,0x41,0x41,0x22,0x1C}  /* D */
};

static void oled_char(uint8_t page, uint8_t col, const uint8_t *g) {
  oled_cmd(0xB0 + page);
  oled_cmd(col & 0x0F);
  oled_cmd(0x10 | (col >> 4));
  for (uint8_t i = 0; i < 5; i++) oled_data(g[i]);
  oled_data(0x00);
}

/* ===================== MAIN ===================== */

int main(void) {

  halInit();
  chSysInit();

  /* GPIO OLED */
  palSetPadMode(OLED_CS_PORT,  OLED_CS_PAD,  PAL_MODE_OUTPUT_PUSHPULL);
  palSetPadMode(OLED_DC_PORT,  OLED_DC_PAD,  PAL_MODE_OUTPUT_PUSHPULL);
  palSetPadMode(OLED_RST_PORT, OLED_RST_PAD, PAL_MODE_OUTPUT_PUSHPULL);

  cs_high();
  dc_data();
  palSetPad(OLED_RST_PORT, OLED_RST_PAD);

  /* >>> LIGNE CRITIQUE <<< */
  force_spi5_pins();

  spiStart(&SPID5, &spicfg_oled);

  oled_init();
  oled_clear();

  /* HELLO WORLD */
  oled_char(2,  0, font_5x7[0]);
  oled_char(2,  6, font_5x7[1]);
  oled_char(2, 12, font_5x7[2]);
  oled_char(2, 18, font_5x7[2]);
  oled_char(2, 24, font_5x7[3]);
  oled_char(2, 36, font_5x7[4]);
  oled_char(2, 42, font_5x7[3]);
  oled_char(2, 48, font_5x7[5]);
  oled_char(2, 54, font_5x7[2]);
  oled_char(2, 60, font_5x7[6]);

  while (true) {
    chThdSleepMilliseconds(1000);
  }
}
