#include "ch.h"
#include "hal.h"
#include "drivers/drivers.h"
#include "drv_hall.h"
#include <stdio.h>

/* ====================================================================== */
/*                           THREAD HALL                                  */
/* ====================================================================== */

static THD_WORKING_AREA(waHallTask, 512);

static THD_FUNCTION(hallTask, arg) {
  (void)arg;
  chRegSetThreadName("HallTask");

  while (!chThdShouldTerminateX()) {
    drv_hall_task();
    chThdSleepMilliseconds(5);   /* lecture rapide ADC/MUX */
  }

  chThdExit(MSG_OK);
}

/* ====================================================================== */
/*                          AFFICHAGE OLED                                */
/* ====================================================================== */

static void draw_hall_states(void) {
  char line[16];

  drv_display_clear();

  for (uint8_t row = 0; row < 8; ++row) {
    uint8_t left  = row;
    uint8_t right = (uint8_t)(row + 8);
    uint8_t y     = (uint8_t)(row * 8);

    snprintf(line, sizeof(line),
             "B%u:%s",
             (unsigned)(left + 1U),
             drv_hall_is_pressed(left) ? "ON " : "OFF");
    drv_display_draw_text(0, y, line);

    snprintf(line, sizeof(line),
             "B%u:%s",
             (unsigned)(right + 1U),
             drv_hall_is_pressed(right) ? "ON " : "OFF");
    drv_display_draw_text(64, y, line);
  }

  /* 🔴 FLUSH EXPLICITE = PAS DE RACE CONDITION */
  drv_display_update();
}

/* ====================================================================== */
/*                                MAIN                                    */
/* ====================================================================== */

int main(void) {

  halInit();
  chSysInit();

  /* Init drivers globaux (GPIO, SPI, etc.) */
  drivers_init_all();

  /* Init + thread OLED */
  drv_display_start();

  /* Init Hall */
  //drv_hall_init();

  /* Thread Hall (lecture capteurs uniquement) */
  /*chThdCreateStatic(waHallTask,
                    sizeof(waHallTask),
                    NORMALPRIO,
                    hallTask,
                    NULL); */

  /* Boucle principale : affichage */
  while (true) {
    drv_display_clear();
    drv_display_draw_text(0, 0, "DISPLAY ONLY");
    drv_display_update();
    chThdSleepMilliseconds(500);
  }
}
