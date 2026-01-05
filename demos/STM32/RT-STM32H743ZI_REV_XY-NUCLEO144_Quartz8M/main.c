#include "ch.h"
#include "hal.h"
#include "drivers/drivers.h"
#include "drv_hall.h"
#include "ui/ui_model.h"
#include <stdio.h>

/* ====================================================================== */
/*                           THREAD HALL                                  */
/* ====================================================================== */

static THD_WORKING_AREA(waHallTask, 512);

enum {
  /* Scheduler is cooperative for equal priorities (CH_CFG_TIME_QUANTUM = 0).
     Cadences below guarantee UI execution windows even under Hall load. */
  HALL_TASK_PERIOD_MS = 20,
  UI_TASK_PERIOD_MS = 100,
  UI_FORCE_REFRESH_MS = 1000
};

static uint16_t build_hall_mask(void) {
  uint16_t mask = 0;
  for (uint8_t i = 0; i < 16U; ++i) {
    if (drv_hall_is_pressed(i)) {
      mask |= (uint16_t)(1U << i);
    }
  }
  return mask;
}

static THD_FUNCTION(hallTask, arg) {
  (void)arg;
  chRegSetThreadName("HallTask");
  systime_t next_wake = chVTGetSystemTimeX();

  while (!chThdShouldTerminateX()) {
    drv_hall_task();
    ui_model_set_hall_mask(build_hall_mask());
    next_wake = chThdSleepUntilWindowed(next_wake,
                                        next_wake + TIME_MS2I(HALL_TASK_PERIOD_MS));
  }

  chThdExit(MSG_OK);
}

/* ====================================================================== */
/*                          AFFICHAGE OLED                                */
/* ====================================================================== */

static void draw_hall_states(uint16_t hall_mask, uint32_t heartbeat) {
  char line[16];

  drv_display_clear();

  for (uint8_t row = 0; row < 8; ++row) {
    uint8_t left  = row;
    uint8_t right = (uint8_t)(row + 8);
    uint8_t y     = (uint8_t)(row * 8);

    snprintf(line, sizeof(line),
             "B%u:%s",
             (unsigned)(left + 1U),
             (hall_mask & (1U << left)) ? "ON " : "OFF");
    drv_display_draw_text(0, y, line);

    snprintf(line, sizeof(line),
             "B%u:%s",
             (unsigned)(right + 1U),
             (hall_mask & (1U << right)) ? "ON " : "OFF");
    drv_display_draw_text(64, y, line);
  }

  snprintf(line, sizeof(line), "U%lu", (unsigned long)heartbeat);
  drv_display_draw_text(96, 56, line);

  drv_display_update();
}

static THD_WORKING_AREA(waUiTask, 512);

static THD_FUNCTION(uiTask, arg) {
  (void)arg;
  chRegSetThreadName("UiTask");
  systime_t next_wake = chVTGetSystemTimeX();
  uint16_t last_mask = 0xFFFFU;
  uint8_t refresh_ticks = 0;
  uint32_t heartbeat = 0;

  while (!chThdShouldTerminateX()) {
    uint16_t hall_mask = ui_model_get_hall_mask();
    bool force_refresh = (refresh_ticks >= (UI_FORCE_REFRESH_MS / UI_TASK_PERIOD_MS));
    if (hall_mask != last_mask || force_refresh) {
      draw_hall_states(hall_mask, heartbeat);
      last_mask = hall_mask;
      refresh_ticks = 0;
    } else {
      refresh_ticks++;
    }

    heartbeat++;
    next_wake = chThdSleepUntilWindowed(next_wake,
                                        next_wake + TIME_MS2I(UI_TASK_PERIOD_MS));
  }

  chThdExit(MSG_OK);
}

/* ====================================================================== */
/*                                MAIN                                    */
/* ====================================================================== */

int main(void) {

  halInit();
  chSysInit();

  /* Init drivers globaux (GPIO, SPI, etc.) */
  drivers_init_all();

  /* Init Hall */
  drv_hall_init();

  /* Thread Hall (lecture capteurs uniquement) */
  chThdCreateStatic(waHallTask,
                    sizeof(waHallTask),
                    NORMALPRIO,
                    hallTask,
                    NULL);

  /* Thread UI (OLED best-effort, basse priorité) */
  chThdCreateStatic(waUiTask,
                    sizeof(waUiTask),
                    LOWPRIO,
                    uiTask,
                    NULL);

  while (true) {
    chThdSleepMilliseconds(1000);
  }
}
