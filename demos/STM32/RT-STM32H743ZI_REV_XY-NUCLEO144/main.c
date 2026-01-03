#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "drv_audio.h"

static volatile uint32_t audio_dma_half_count = 0U;
static volatile uint32_t audio_dma_full_count = 0U;

void drv_audio_dma_half_cb(uint8_t half) {
  (void)half;
  audio_dma_half_count++;
}

void drv_audio_dma_full_cb(uint8_t half) {
  (void)half;
  audio_dma_full_count++;
}

static const SerialConfig uart_cfg = {
  115200,
  0,
  0,
  0
};

static const char *audio_state_name(drv_audio_state_t state) {
  switch (state) {
    case DRV_AUDIO_STOPPED:
      return "STOPPED";
    case DRV_AUDIO_READY:
      return "READY";
    case DRV_AUDIO_RUNNING:
      return "RUNNING";
    case DRV_AUDIO_FAULT:
      return "FAULT";
    default:
      return "UNKNOWN";
  }
}

static THD_WORKING_AREA(logThreadWA, 512);
static THD_FUNCTION(logThread, arg) {
  BaseSequentialStream *chp = (BaseSequentialStream *)arg;
  uint32_t last_half = 0U;
  uint32_t last_full = 0U;

  chRegSetThreadName("audioLog");

  while (true) {
    chThdSleepSeconds(1);

    uint32_t half = audio_dma_half_count;
    uint32_t full = audio_dma_full_count;
    uint32_t half_delta = half - last_half;
    uint32_t full_delta = full - last_full;
    last_half = half;
    last_full = full;

    chprintf(chp,
             "audio irq/s half=%lu full=%lu state=%s\r\n",
             (unsigned long)half_delta,
             (unsigned long)full_delta,
             audio_state_name(drv_audio_get_state()));
  }
}

int main(void) {
  halInit();
  chSysInit();

  sdStart(&SD1, &uart_cfg);
  chprintf((BaseSequentialStream *)&SD1, "\r\nAudio bring-up test (no codec)\r\n");

  drv_audio_init();
  drv_audio_start();

  chThdCreateStatic(logThreadWA,
                    sizeof(logThreadWA),
                    LOWPRIO,
                    logThread,
                    (void *)&SD1);

  while (true) {
    chThdSleepSeconds(5);
  }
}
