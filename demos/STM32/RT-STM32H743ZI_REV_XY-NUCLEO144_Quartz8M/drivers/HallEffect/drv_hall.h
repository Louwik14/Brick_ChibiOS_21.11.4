#ifndef DRV_HALL_H
#define DRV_HALL_H

#include <stdint.h>
#include <stdbool.h>

void hall_init(void);
void hall_update(void);
uint16_t hall_get(uint8_t index);
bool hall_get_note_on(uint8_t index);
bool hall_get_note_off(uint8_t index);
uint8_t hall_get_velocity(uint8_t index);
uint8_t hall_get_pressure(uint8_t index);
uint8_t hall_get_midi_value(uint8_t index);

#endif /* DRV_HALL_H */
