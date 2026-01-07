#ifndef DRV_HALL_H
#define DRV_HALL_H

#include <stdint.h>

void hall_init(void);
void hall_update(void);
uint16_t hall_get(uint8_t index);

#endif /* DRV_HALL_H */
