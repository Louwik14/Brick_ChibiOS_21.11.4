/*
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/**
 * @file    hal_sai.h
 * @brief   SAI Driver macros and structures.
 *
 * @addtogroup SAI
 * @{
 */

#ifndef HAL_SAI_H
#define HAL_SAI_H

#if (HAL_USE_SAI == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @brief   SAI driver state machine possible states.
 */
typedef enum {
  SAI_UNINIT = 0,                  /**< Not initialized.                   */
  SAI_STOP = 1,                    /**< Stopped.                           */
  SAI_READY = 2,                   /**< Ready.                             */
  SAI_ACTIVE = 3                   /**< Active.                            */
} saistate_t;

/**
 * @brief   Type of a structure representing an SAI driver.
 */
typedef struct hal_sai_driver SAIDriver;

/**
 * @brief   Type of a structure representing an SAI driver configuration.
 */
typedef struct hal_sai_config SAIConfig;

/**
 * @brief   SAI notification callback type.
 *
 * @param[in] saip      pointer to the @p SAIDriver object
 * @param[in] half      true on half-transfer, false on full-transfer
 */
typedef void (*saicallback_t)(SAIDriver *saip, bool half);

/* Including the low level driver header, it exports information required
   for completing types.*/
#include "hal_sai_lld.h"

/**
 * @brief   Structure representing an SAI driver.
 */
struct hal_sai_driver {
  /**
   * @brief   Driver state.
   */
  saistate_t                state;
  /**
   * @brief   Current configuration data.
   */
  const SAIConfig           *config;
  /* End of the mandatory fields.*/
  sai_lld_driver_fields
};

/**
 * @brief   Driver configuration structure.
 */
struct hal_sai_config {
  /**
   * @brief   Transmission buffer pointer.
   * @note    Can be @p NULL if TX is not required.
   */
  const void                *tx_buffer;
  /**
   * @brief   Receive buffer pointer.
   * @note    Can be @p NULL if RX is not required.
   */
  void                      *rx_buffer;
  /**
   * @brief   TX and RX buffers size as number of items.
   */
  size_t                    size;
  /**
   * @brief   Callback function called at half/full transfer.
   */
  saicallback_t             end_cb;
  /* End of the mandatory fields.*/
  sai_lld_config_fields
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/**
 * @brief   Starts an SAI data exchange.
 *
 * @param[in] saip      pointer to the @p SAIDriver object
 *
 * @iclass
 */
#define saiStartExchangeI(saip) {                                           \
  sai_lld_start_exchange(saip);                                              \
  (saip)->state = SAI_ACTIVE;                                                \
}

/**
 * @brief   Stops the ongoing data exchange.
 *
 * @param[in] saip      pointer to the @p SAIDriver object
 *
 * @iclass
 */
#define saiStopExchangeI(saip) {                                            \
  sai_lld_stop_exchange(saip);                                               \
  (saip)->state = SAI_READY;                                                 \
}

/**
 * @brief   Common ISR code, half buffer event.
 * @note    This macro is meant to be used in the low level driver only.
 *
 * @param[in] saip      pointer to the @p SAIDriver object
 *
 * @notapi
 */
#define _sai_isr_half_code(saip) {                                          \
  if ((saip)->config->end_cb != NULL) {                                      \
    (saip)->config->end_cb(saip, true);                                      \
  }                                                                          \
}

/**
 * @brief   Common ISR code, full buffer event.
 * @note    This macro is meant to be used in the low level driver only.
 *
 * @param[in] saip      pointer to the @p SAIDriver object
 *
 * @notapi
 */
#define _sai_isr_full_code(saip) {                                          \
  if ((saip)->config->end_cb != NULL) {                                      \
    (saip)->config->end_cb(saip, false);                                     \
  }                                                                          \
}

#ifdef __cplusplus
extern "C" {
#endif
  void saiInit(void);
  void saiObjectInit(SAIDriver *saip);
  msg_t saiStart(SAIDriver *saip, const SAIConfig *config);
  void saiStop(SAIDriver *saip);
  void saiSetBuffers(SAIDriver *saip,
                     const void *tx_buffer,
                     void *rx_buffer,
                     size_t size);
  void saiStartExchange(SAIDriver *saip);
  void saiStopExchange(SAIDriver *saip);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_SAI == TRUE */

#endif /* HAL_SAI_H */

/** @} */
