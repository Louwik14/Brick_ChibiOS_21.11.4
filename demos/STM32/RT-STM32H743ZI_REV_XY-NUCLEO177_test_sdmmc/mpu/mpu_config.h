/**
 * @file mpu_config.h
 * @brief Configuration minimale commune du MPU pour les buffers non-cacheables.
 */

#ifndef MPU_CONFIG_H
#define MPU_CONFIG_H

#include <stdbool.h>
#include <stdint.h>

#include "mpu_map.h"

/*
 * Limites exportées par le script LD pour la section .nocache (SRAM D2 / SRAM3).
 * Utilisées pour dériver la fenêtre MPU non-cacheable compatible DMA.
 */
extern uint8_t __nocache_base__;
extern uint8_t __nocache_end__;

bool mpu_config_init_once(void);

#endif /* MPU_CONFIG_H */
