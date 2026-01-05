#include "ui_model.h"

static volatile uint16_t ui_hall_mask;

void ui_model_set_hall_mask(uint16_t mask) {
    __atomic_store_n(&ui_hall_mask, mask, __ATOMIC_RELAXED);
}

uint16_t ui_model_get_hall_mask(void) {
    return __atomic_load_n(&ui_hall_mask, __ATOMIC_RELAXED);
}
