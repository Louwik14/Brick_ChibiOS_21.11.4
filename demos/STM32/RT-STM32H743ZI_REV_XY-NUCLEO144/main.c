#include "ch.h"
#include "hal.h"
#include "audio/drv_audio.h"
#include "drivers/drivers.h"
#include <stdio.h>

static THD_WORKING_AREA(oledThreadWA, THD_WORKING_AREA_SIZE(512));
static THD_FUNCTION(oledThread, arg) {
  (void)arg;
  chRegSetThreadName("oledAudioStats");

  while (!chThdShouldTerminateX()) {
    uint32_t rx_ht = drv_audio_get_rx_half_count();
    uint32_t rx_tc = drv_audio_get_rx_full_count();
    uint32_t tx_ht = drv_audio_get_tx_half_count();
    uint32_t tx_tc = drv_audio_get_tx_full_count();
    uint32_t cs0 = drv_audio_get_rx_checksum(0U);
    uint32_t cs1 = drv_audio_get_rx_checksum(1U);
    uint32_t sync_err = drv_audio_get_sync_error_count();
    uint32_t sr_live = drv_audio_get_sai_sr_live();

    char line[32];
    drv_display_clear();
    drv_display_draw_text(0, 0, "SAI AUDIO TEST");

    snprintf(line, sizeof(line), "RX HT:%lu TC:%lu",
             (unsigned long)rx_ht, (unsigned long)rx_tc);
    drv_display_draw_text(0, 8, line);

    snprintf(line, sizeof(line), "TX HT:%lu TC:%lu",
             (unsigned long)tx_ht, (unsigned long)tx_tc);
    drv_display_draw_text(0, 16, line);

    snprintf(line, sizeof(line), "RX CS0: %08lX", (unsigned long)cs0);
    drv_display_draw_text(0, 24, line);

    snprintf(line, sizeof(line), "RX CS1: %08lX", (unsigned long)cs1);
    drv_display_draw_text(0, 32, line);

    snprintf(line, sizeof(line), "SYNC ERR: %lu", (unsigned long)sync_err);
    drv_display_draw_text(0, 40, line);

    snprintf(line, sizeof(line), "SR: 0x%08lX", (unsigned long)sr_live);
    drv_display_draw_text(0, 48, line);

    chThdSleepMilliseconds(300);
  }
}

int main(void) {
  halInit();
  chSysInit();

  drivers_init_all();

  chThdCreateStatic(oledThreadWA,
                    sizeof(oledThreadWA),
                    NORMALPRIO - 2,
                    oledThread,
                    NULL);

  drv_audio_init();
  drv_audio_reset_stats();
  drv_audio_start();

  while (true) {
    chThdSleepSeconds(1);
  }
}
