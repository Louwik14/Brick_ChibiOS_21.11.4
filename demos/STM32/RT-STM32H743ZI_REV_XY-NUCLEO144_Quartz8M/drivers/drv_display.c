
#include "drv_display.h"
#include "ch.h"
#include "hal.h"
#include "brick_config.h"
#include <string.h>
#include <stdio.h>

/* ====================================================================== */
/*                        CONFIGURATION MATÉRIELLE                        */
/* ====================================================================== */

/*
 * Port SPI2 demandé :
 *  - SCK  = PI1 (AF5)
 *  - MOSI = PI3 (AF5)
 *
 * Si ces lignes sont déjà définies ailleurs (board.h, etc.), ce #ifndef évite
 * les redéfinitions.
 */
#ifndef LINE_SPI2_SCK
#define LINE_SPI2_SCK                PAL_LINE(GPIOI, 1U)
#endif

#ifndef LINE_SPI2_MOSI
#define LINE_SPI2_MOSI               PAL_LINE(GPIOI, 3U)
#endif

/* SPI2 - Configuration compatible ChibiOS récent */
static const SPIConfig spicfg = {
    .circular = false,
    .slave    = false,
    .data_cb  = NULL,
    .error_cb = NULL,

    /* ~10–15 MHz selon ton clock SPI */
    .cfg1 = SPI_CFG1_MBR_2 |               /* Prescaler */
            SPI_CFG1_DSIZE_VALUE(7),      /* 8 bits */

    .cfg2 = SPI_CFG2_MASTER |
            SPI_CFG2_SSM
};

/* ====================================================================== */
/*                             VARIABLES INTERNES                         */
/* ====================================================================== */

static uint8_t buffer[BRICK_OLED_WIDTH * BRICK_OLED_HEIGHT / 8];
static const font_t *current_font = NULL;
static thread_t *display_tp = NULL;

/* ====================================================================== */
/*                              UTILITAIRES GPIO                          */
/* ====================================================================== */

/* On laisse CS/DC/RES tels quels (déjà câblés et définis chez toi) */
static inline void cs_low(void)  { palClearLine(LINE_SPI5_CS_OLED); }
static inline void cs_high(void) { palSetLine  (LINE_SPI5_CS_OLED); }
static inline void dc_cmd(void)  { palClearLine(LINE_SPI5_DC_OLED); }
static inline void dc_data(void) { palSetLine  (LINE_SPI5_DC_OLED); }

/* ====================================================================== */
/*                              UTILITAIRES SPI                           */
/* ====================================================================== */

static void send_cmd(uint8_t cmd) {
    dc_cmd();
    cs_low();
    spiPolledExchange(&SPID2, cmd);
    cs_high();
}

static void send_data(const uint8_t *data, size_t len) {
    dc_data();
    for (size_t i = 0; i < len; i++) {
        cs_low();
        spiPolledExchange(&SPID2, data[i]);
        cs_high();
    }
}

/* ====================================================================== */
/*                              DIRTY STATE                               */
/* ====================================================================== */

static volatile bool display_dirty = false;
/* 1 bit par page (0..7) */
static uint8_t dirty_pages = 0;

/* ====================================================================== */
/*                              FRAMEBUFFER                               */
/* ====================================================================== */

uint8_t* drv_display_get_buffer(void) {
    return buffer;
}

/* ====================================================================== */
/*                              PIXELS                                    */
/* ====================================================================== */

static inline void set_pixel(int x, int y, bool on) {
    if (x < 0 || x >= BRICK_OLED_WIDTH || y < 0 || y >= BRICK_OLED_HEIGHT)
        return;

    const int index = x + (y >> 3) * BRICK_OLED_WIDTH;
    const uint8_t mask = (uint8_t)(1U << (y & 7));

    uint8_t old = buffer[index];

    if (on) buffer[index] |= mask;
    else    buffer[index] &= (uint8_t)~mask;

    /* Ne marquer dirty QUE si le pixel change vraiment */
    if (buffer[index] != old) {
        display_dirty = true;
        dirty_pages |= (1U << (y >> 3));
    }
}

void drv_display_draw_pixel(int x, int y, bool on) {
    set_pixel(x, y, on);
}

void drv_display_draw_rect(int x, int y, int w, int h) {

    if (w <= 0 || h <= 0)
        return;

    /* Top & bottom */
    for (int ix = 0; ix < w; ix++) {
        drv_display_draw_pixel(x + ix, y, true);
        drv_display_draw_pixel(x + ix, y + h - 1, true);
    }

    /* Left & right */
    for (int iy = 0; iy < h; iy++) {
        drv_display_draw_pixel(x, y + iy, true);
        drv_display_draw_pixel(x + w - 1, y + iy, true);
    }
}

void drv_display_fill_rect(int x, int y, int w, int h) {

    if (w <= 0 || h <= 0)
        return;

    for (int iy = 0; iy < h; iy++) {
        for (int ix = 0; ix < w; ix++) {
            drv_display_draw_pixel(x + ix, y + iy, true);
        }
    }
}

void drv_display_clear_rect(int x, int y, int w, int h) {

    if (w <= 0 || h <= 0)
        return;

    for (int iy = 0; iy < h; iy++) {
        for (int ix = 0; ix < w; ix++) {
            drv_display_draw_pixel(x + ix, y + iy, false);
        }
    }
}

/* ====================================================================== */
/*                      INITIALISATION OLED                               */
/* ====================================================================== */

void drv_display_init(void) {

    /* GPIO OLED */
    palSetLineMode(LINE_SPI5_CS_OLED,  PAL_MODE_OUTPUT_PUSHPULL);
    palSetLineMode(LINE_SPI5_DC_OLED,  PAL_MODE_OUTPUT_PUSHPULL);
    palSetLineMode(LINE_SPI5_RES_OLED, PAL_MODE_OUTPUT_PUSHPULL);

    cs_high();
    dc_data();
    palSetLine(LINE_SPI5_RES_OLED);

    /* SPI2 pins forced to AF5 (STM32H7 specific) */
    palSetLineMode(LINE_SPI2_SCK,  PAL_MODE_ALTERNATE(5) | PAL_STM32_OSPEED_HIGHEST);
    palSetLineMode(LINE_SPI2_MOSI, PAL_MODE_ALTERNATE(5) | PAL_STM32_OSPEED_HIGHEST);

    spiStart(&SPID2, &spicfg);

    /* Reset OLED */
    palClearLine(LINE_SPI5_RES_OLED);
    chThdSleepMilliseconds(50);
    palSetLine(LINE_SPI5_RES_OLED);
    chThdSleepMilliseconds(50);

    /* Séquence init SSD1309 / SSD1306 */
    send_cmd(0xAE);
    send_cmd(0xD5); send_cmd(0x80);
    send_cmd(0xA8); send_cmd(0x3F);
    send_cmd(0xD3); send_cmd(0x00);
    send_cmd(0x40);
    send_cmd(0x8D); send_cmd(0x14);
    send_cmd(0x20); send_cmd(0x00);
    send_cmd(0xA1);
    send_cmd(0xC8);
    send_cmd(0xDA); send_cmd(0x12);
    send_cmd(0xAF);

    drv_display_clear();
    drv_display_update();

    extern const font_t FONT_5X7;
    current_font = &FONT_5X7;
}

/* ====================================================================== */
/*                               CLEAR                                    */
/* ====================================================================== */

void drv_display_clear(void) {
    memset(buffer, 0x00, sizeof(buffer));
    display_dirty = true;
    dirty_pages = 0xFF; /* toutes les pages */
}

/* ====================================================================== */
/*                               UPDATE                                   */
/* ====================================================================== */

void drv_display_update(void) {

    /* Rien à faire */
    if (!display_dirty)
        return;

    for (uint8_t page = 0; page < 8; page++) {

        if (!(dirty_pages & (1U << page)))
            continue;

        send_cmd(0xB0 + page);
        send_cmd(0x00);
        send_cmd(0x10);

        send_data(&buffer[page * BRICK_OLED_WIDTH],
                  BRICK_OLED_WIDTH);
    }

    /* Reset dirty state */
    display_dirty = false;
    dirty_pages = 0;
}

/* ====================================================================== */
/*                              TEXTE                                     */
/* ====================================================================== */

void drv_display_set_font(const font_t *font) {
    current_font = font;
}

static inline uint8_t font_advance(const font_t *f) {
    return (uint8_t)(f->width + f->spacing);
}

void drv_display_draw_char(uint8_t x, uint8_t y, char c) {

    if (!current_font) return;

    if ((uint8_t)c < current_font->first || (uint8_t)c > current_font->last)
        c = '?';

    for (uint8_t col = 0; col < current_font->width; col++) {
        uint8_t bits = current_font->get_col(c, col);
        for (uint8_t row = 0; row < current_font->height; row++) {
            if (bits & (1U << row))
                set_pixel(x + col, y + row, true);
        }
    }
}

void drv_display_draw_text(uint8_t x, uint8_t y, const char *txt) {

    if (!current_font || !txt) return;

    const uint8_t adv = font_advance(current_font);

    while (*txt && x < BRICK_OLED_WIDTH) {
        drv_display_draw_char(x, y, *txt++);
        x = (uint8_t)(x + adv);
    }
}

void drv_display_draw_text_with_font(const font_t *font,
                                     uint8_t x, uint8_t y,
                                     const char *txt) {

    if (!font || !txt) return;

    const font_t *save = current_font;
    current_font = font;
    drv_display_draw_text(x, y, txt);
    current_font = save;
}

void drv_display_draw_text_at_baseline(const font_t *font,
                                       uint8_t x, uint8_t baseline_y,
                                       const char *txt) {

    if (!font || !txt) return;

    const font_t *save = current_font;
    current_font = font;

    uint8_t y = (baseline_y >= font->height)
                ? (baseline_y - font->height)
                : 0;

    drv_display_draw_text(x, y, txt);
    current_font = save;
}

void drv_display_draw_number(uint8_t x, uint8_t y, int num) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%d   ", num);
    drv_display_draw_text(x, y, buf);
}

/* ====================================================================== */
/*                         THREAD DE RAFRAÎCHISSEMENT                      */
/* ====================================================================== */

static THD_WORKING_AREA(waDisplay, 512);

static THD_FUNCTION(displayThread, arg) {
    (void)arg;
    chRegSetThreadName("Display");

    while (!chThdShouldTerminateX()) {
        drv_display_update();
        chThdSleepMilliseconds(33);
    }

    chThdExit(MSG_OK);
}

void drv_display_start(void) {
    if (display_tp != NULL) {
        if (chThdTerminatedX(display_tp)) {
            chThdWait(display_tp);
            display_tp = NULL;
        } else {
            return;
        }
    }

    drv_display_init();

    display_tp = chThdCreateStatic(waDisplay, sizeof(waDisplay),
                                   NORMALPRIO, displayThread, NULL);
}

void drv_display_stop(void) {
    if (display_tp == NULL)
        return;

    chThdTerminate(display_tp);
    chThdWait(display_tp);
    display_tp = NULL;
}
