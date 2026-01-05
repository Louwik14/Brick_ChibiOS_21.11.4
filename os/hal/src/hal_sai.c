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
 * @file    hal_sai.c
 * @brief   SAI Driver code.
 *
 * @addtogroup SAI
 * @{
 */

#include "hal.h"

#if defined(HAL_USE_SAI) && (HAL_USE_SAI == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   SAI Driver initialization.
 * @note    This function is implicitly invoked by @p halInit().
 *
 * @init
 */
void saiInit(void) {

  sai_lld_init();
}

/**
 * @brief   Initializes the standard part of a @p SAIDriver structure.
 *
 * @param[out] saip     pointer to the @p SAIDriver object
 *
 * @init
 */
void saiObjectInit(SAIDriver *saip) {

  saip->state  = SAI_STOP;
  saip->config = NULL;
#if defined(SAI_LLD_ENHANCED_API)
  saip->start_error = HAL_RET_SUCCESS;
#endif
  saip->error_flags = 0U;
  saip->error_repeats = 0U;
}

/**
 * @brief   Configures and activates the SAI peripheral.
 *
 * @param[in] saip      pointer to the @p SAIDriver object
 * @param[in] config    pointer to the @p SAIConfig object
 * @return              The operation status.
 *
 * @api
 */
msg_t saiStart(SAIDriver *saip, const SAIConfig *config) {
  msg_t msg;

  osalDbgCheck((saip != NULL) && (config != NULL));

  osalSysLock();
  osalDbgAssert((saip->state == SAI_STOP) || (saip->state == SAI_READY),
                "invalid state");

  saip->config = config;

#if defined(SAI_LLD_ENHANCED_API)
  msg = sai_lld_start(saip);
  if (msg == HAL_RET_SUCCESS) {
    saip->state = SAI_READY;
    if (saip->start_error != HAL_RET_SUCCESS) {
      msg = saip->start_error;
      saip->start_error = HAL_RET_SUCCESS;
    }
  }
  else {
    saip->state = SAI_STOP;
  }
#else
  sai_lld_start(saip);
  saip->state = SAI_READY;
  msg = HAL_RET_SUCCESS;
#endif

  osalSysUnlock();

  return msg;
}

/**
 * @brief   Deactivates the SAI peripheral.
 *
 * @param[in] saip      pointer to the @p SAIDriver object
 *
 * @api
 */
void saiStop(SAIDriver *saip) {

  osalDbgCheck(saip != NULL);

  osalSysLock();
  osalDbgAssert((saip->state == SAI_STOP) || (saip->state == SAI_READY),
                "invalid state");

  sai_lld_stop(saip);
  saip->config = NULL;
  saip->state  = SAI_STOP;
#if defined(SAI_LLD_ENHANCED_API)
  saip->start_error = HAL_RET_SUCCESS;
#endif

  osalSysUnlock();
}

/**
 * @brief   Updates DMA buffers and size.
 *
 * @param[in] saip      pointer to the @p SAIDriver object
 * @param[in] tx_buffer pointer to TX buffer or NULL
 * @param[in] rx_buffer pointer to RX buffer or NULL
 * @param[in] size      buffer size in items
 *
 * @api
 */
void saiSetBuffers(SAIDriver *saip,
                   const void *tx_buffer,
                   void *rx_buffer,
                   size_t size) {

  osalDbgCheck(saip != NULL);

  osalSysLock();
  osalDbgAssert(saip->state == SAI_READY, "not ready");

  sai_lld_set_buffers(saip, tx_buffer, rx_buffer, size);

  osalSysUnlock();
}

/**
 * @brief   Starts an SAI data exchange.
 *
 * @param[in] saip      pointer to the @p SAIDriver object
 *
 * @api
 */
void saiStartExchange(SAIDriver *saip) {

  osalDbgCheck(saip != NULL);

  osalSysLock();
  osalDbgAssert(saip->state == SAI_READY, "not ready");
  saiStartExchangeI(saip);
  osalSysUnlock();
}

/**
 * @brief   Stops the ongoing data exchange.
 *
 * @param[in] saip      pointer to the @p SAIDriver object
 *
 * @api
 */
void saiStopExchange(SAIDriver *saip) {

  osalDbgCheck(saip != NULL);

  osalSysLock();
  osalDbgAssert(saip->state == SAI_ACTIVE, "not active");
  saiStopExchangeI(saip);
  osalSysUnlock();
}

#endif /* HAL_USE_SAI == TRUE */

/** @} */
