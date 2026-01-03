#include "ch.h"
#include "hal.h"
#include "midi.h"
#include "usb_device.h"

int main(void) {
  halInit();
  chSysInit();

  usb_device_start();
  midi_init();

  while (!usb_device_active()) {
    chThdSleepMilliseconds(10);
  }

  while (true) {
    midi_note_on(MIDI_DEST_USB, 0, 60, 100);
    chThdSleepMilliseconds(1000);
    midi_note_off(MIDI_DEST_USB, 0, 60, 0);
    chThdSleepMilliseconds(1000);
  }
}
