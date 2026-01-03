#include "ch.h"
#include "hal.h"
#include "drv_display.h"

/* ===================== RANDOM WAVE ===================== */

#define WAVE_WIDTH    128
#define WAVE_CENTER_Y 32

static int8_t wave[WAVE_WIDTH];

/* PRNG simple et rapide */
static uint32_t rng = 0x12345678;

static int8_t rand8(void) {
    rng ^= rng << 13;
    rng ^= rng >> 17;
    rng ^= rng << 5;
    return (int8_t)(rng & 0x7F) - 64;
}

/* Initialise une onde douce */
static void init_wave(void) {
    int8_t v = 0;
    for (int i = 0; i < WAVE_WIDTH; i++) {
        v += rand8() >> 3;      /* lissage */
        if (v > 32)  v = 32;
        if (v < -32) v = -32;
        wave[i] = v;
    }
}

/* ===================== DRAW ===================== */

static void draw_wave(void) {

    for (int x = 0; x < WAVE_WIDTH; x++) {

        int y = WAVE_CENTER_Y - wave[x];

        /* onde épaisse */
        drv_display_draw_pixel(x, y - 1, true);
        drv_display_draw_pixel(x, y,     true);
        drv_display_draw_pixel(x, y + 1, true);
    }
}

/* ===================== MAIN ===================== */

int main(void) {

    halInit();
    chSysInit();

    drv_display_start();
    init_wave();

    while (true) {

        drv_display_clear();

        drv_display_draw_text(0, 0, "NOISE WAVE");

        draw_wave();

        /* === animation === */

        /* décale tout à gauche */
        for (int i = 0; i < WAVE_WIDTH - 1; i++) {
            wave[i] = wave[i + 1];
        }

        /* génère une nouvelle valeur */
        int8_t v = wave[WAVE_WIDTH - 2];
        v += rand8() >> 3;
        if (v > 32)  v = 32;
        if (v < -32) v = -32;
        wave[WAVE_WIDTH - 1] = v;

        chThdSleepMilliseconds(33); /* ~30 FPS */
    }
}
