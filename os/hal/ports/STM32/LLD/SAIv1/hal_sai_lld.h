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
 * @file    SAIv1/hal_sai_lld.h
 * @brief   STM32 SAI subsystem low level driver header.
 *
 * @addtogroup SAI
 * @{
 */

#ifndef HAL_SAI_LLD_H
#define HAL_SAI_LLD_H

#if defined(HAL_USE_SAI) && (HAL_USE_SAI == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @name    SAI audio block identifiers
 * @{
 */
#define STM32_SAI_BLOCK_A                0U
#define STM32_SAI_BLOCK_B                1U
/** @} */

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Configuration options
 * @{
 */
/* Deterministic DMA stream allocation is a bring-up requirement (Safe Profile
 * & Hardening). Do not leave SAI DMA streams set to STM32_DMA_STREAM_ID_ANY. */
#if !defined(STM32_SAI_USE_SAI1A) || defined(__DOXYGEN__)
#define STM32_SAI_USE_SAI1A              FALSE
#endif

#if !defined(STM32_SAI_USE_SAI1B) || defined(__DOXYGEN__)
#define STM32_SAI_USE_SAI1B              FALSE
#endif

#if !defined(STM32_SAI_USE_SAI2A) || defined(__DOXYGEN__)
#define STM32_SAI_USE_SAI2A              FALSE
#endif

#if !defined(STM32_SAI_USE_SAI2B) || defined(__DOXYGEN__)
#define STM32_SAI_USE_SAI2B              FALSE
#endif

#if !defined(STM32_SAI_SAI1A_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define STM32_SAI_SAI1A_IRQ_PRIORITY     10
#endif

#if !defined(STM32_SAI_SAI1B_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define STM32_SAI_SAI1B_IRQ_PRIORITY     10
#endif

#if !defined(STM32_SAI_SAI2A_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define STM32_SAI_SAI2A_IRQ_PRIORITY     10
#endif

#if !defined(STM32_SAI_SAI2B_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define STM32_SAI_SAI2B_IRQ_PRIORITY     10
#endif

#if !defined(STM32_SAI_SAI1A_DMA_PRIORITY) || defined(__DOXYGEN__)
#define STM32_SAI_SAI1A_DMA_PRIORITY     1
#endif

#if !defined(STM32_SAI_SAI1B_DMA_PRIORITY) || defined(__DOXYGEN__)
#define STM32_SAI_SAI1B_DMA_PRIORITY     1
#endif

#if !defined(STM32_SAI_SAI2A_DMA_PRIORITY) || defined(__DOXYGEN__)
#define STM32_SAI_SAI2A_DMA_PRIORITY     1
#endif

#if !defined(STM32_SAI_SAI2B_DMA_PRIORITY) || defined(__DOXYGEN__)
#define STM32_SAI_SAI2B_DMA_PRIORITY     1
#endif

#if !defined(STM32_SAI_SAI1A_RX_DMA_STREAM) || defined(__DOXYGEN__)
#define STM32_SAI_SAI1A_RX_DMA_STREAM    STM32_DMA_STREAM_ID_ANY
#endif

#if !defined(STM32_SAI_SAI1A_TX_DMA_STREAM) || defined(__DOXYGEN__)
#define STM32_SAI_SAI1A_TX_DMA_STREAM    STM32_DMA_STREAM_ID_ANY
#endif

#if !defined(STM32_SAI_SAI1B_RX_DMA_STREAM) || defined(__DOXYGEN__)
#define STM32_SAI_SAI1B_RX_DMA_STREAM    STM32_DMA_STREAM_ID_ANY
#endif

#if !defined(STM32_SAI_SAI1B_TX_DMA_STREAM) || defined(__DOXYGEN__)
#define STM32_SAI_SAI1B_TX_DMA_STREAM    STM32_DMA_STREAM_ID_ANY
#endif

#if !defined(STM32_SAI_SAI2A_RX_DMA_STREAM) || defined(__DOXYGEN__)
#define STM32_SAI_SAI2A_RX_DMA_STREAM    STM32_DMA_STREAM_ID_ANY
#endif

#if !defined(STM32_SAI_SAI2A_TX_DMA_STREAM) || defined(__DOXYGEN__)
#define STM32_SAI_SAI2A_TX_DMA_STREAM    STM32_DMA_STREAM_ID_ANY
#endif

#if !defined(STM32_SAI_SAI2B_RX_DMA_STREAM) || defined(__DOXYGEN__)
#define STM32_SAI_SAI2B_RX_DMA_STREAM    STM32_DMA_STREAM_ID_ANY
#endif

#if !defined(STM32_SAI_SAI2B_TX_DMA_STREAM) || defined(__DOXYGEN__)
#define STM32_SAI_SAI2B_TX_DMA_STREAM    STM32_DMA_STREAM_ID_ANY
#endif

#if !defined(STM32_SAI_DMA_ERROR_HOOK) || defined(__DOXYGEN__)
#define STM32_SAI_DMA_ERROR_HOOK(saip)   osalSysHalt("DMA failure")
#endif

#if !defined(STM32_SAI_ERROR_HOOK) || defined(__DOXYGEN__)
#define STM32_SAI_ERROR_HOOK(saip, flags) do {                              \
  (void)(saip);                                                             \
  (void)(flags);                                                            \
} while (false)
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if STM32_SAI_USE_SAI1A &&                                                  \
    (STM32_SAI_SAI1A_RX_DMA_STREAM == STM32_DMA_STREAM_ID_ANY)
#if defined(BRING_UP_STRICT) && (BRING_UP_STRICT == TRUE)
#error "SAI1A RX requires a deterministic DMA stream (bring-up requirement)"
#else
#warning "SAI1A RX uses STM32_DMA_STREAM_ID_ANY; deterministic allocation is a bring-up requirement"
#endif
#endif

#if STM32_SAI_USE_SAI1A &&                                                  \
    (STM32_SAI_SAI1A_TX_DMA_STREAM == STM32_DMA_STREAM_ID_ANY)
#if defined(BRING_UP_STRICT) && (BRING_UP_STRICT == TRUE)
#error "SAI1A TX requires a deterministic DMA stream (bring-up requirement)"
#else
#warning "SAI1A TX uses STM32_DMA_STREAM_ID_ANY; deterministic allocation is a bring-up requirement"
#endif
#endif

#if STM32_SAI_USE_SAI1B &&                                                  \
    (STM32_SAI_SAI1B_RX_DMA_STREAM == STM32_DMA_STREAM_ID_ANY)
#if defined(BRING_UP_STRICT) && (BRING_UP_STRICT == TRUE)
#error "SAI1B RX requires a deterministic DMA stream (bring-up requirement)"
#else
#warning "SAI1B RX uses STM32_DMA_STREAM_ID_ANY; deterministic allocation is a bring-up requirement"
#endif
#endif

#if STM32_SAI_USE_SAI1B &&                                                  \
    (STM32_SAI_SAI1B_TX_DMA_STREAM == STM32_DMA_STREAM_ID_ANY)
#if defined(BRING_UP_STRICT) && (BRING_UP_STRICT == TRUE)
#error "SAI1B TX requires a deterministic DMA stream (bring-up requirement)"
#else
#warning "SAI1B TX uses STM32_DMA_STREAM_ID_ANY; deterministic allocation is a bring-up requirement"
#endif
#endif

#if STM32_SAI_USE_SAI2A &&                                                  \
    (STM32_SAI_SAI2A_RX_DMA_STREAM == STM32_DMA_STREAM_ID_ANY)
#if defined(BRING_UP_STRICT) && (BRING_UP_STRICT == TRUE)
#error "SAI2A RX requires a deterministic DMA stream (bring-up requirement)"
#else
#warning "SAI2A RX uses STM32_DMA_STREAM_ID_ANY; deterministic allocation is a bring-up requirement"
#endif
#endif

#if STM32_SAI_USE_SAI2A &&                                                  \
    (STM32_SAI_SAI2A_TX_DMA_STREAM == STM32_DMA_STREAM_ID_ANY)
#if defined(BRING_UP_STRICT) && (BRING_UP_STRICT == TRUE)
#error "SAI2A TX requires a deterministic DMA stream (bring-up requirement)"
#else
#warning "SAI2A TX uses STM32_DMA_STREAM_ID_ANY; deterministic allocation is a bring-up requirement"
#endif
#endif

#if STM32_SAI_USE_SAI2B &&                                                  \
    (STM32_SAI_SAI2B_RX_DMA_STREAM == STM32_DMA_STREAM_ID_ANY)
#if defined(BRING_UP_STRICT) && (BRING_UP_STRICT == TRUE)
#error "SAI2B RX requires a deterministic DMA stream (bring-up requirement)"
#else
#warning "SAI2B RX uses STM32_DMA_STREAM_ID_ANY; deterministic allocation is a bring-up requirement"
#endif
#endif

#if STM32_SAI_USE_SAI2B &&                                                  \
    (STM32_SAI_SAI2B_TX_DMA_STREAM == STM32_DMA_STREAM_ID_ANY)
#if defined(BRING_UP_STRICT) && (BRING_UP_STRICT == TRUE)
#error "SAI2B TX requires a deterministic DMA stream (bring-up requirement)"
#else
#warning "SAI2B TX uses STM32_DMA_STREAM_ID_ANY; deterministic allocation is a bring-up requirement"
#endif
#endif

#if STM32_SAI_USE_SAI1A &&                                                  \
    !STM32_DMA_IS_VALID_STREAM(STM32_SAI_SAI1A_RX_DMA_STREAM)
#error "Invalid DMA stream assigned to SAI1A RX"
#endif

#if STM32_SAI_USE_SAI1A &&                                                  \
    !STM32_DMA_IS_VALID_STREAM(STM32_SAI_SAI1A_TX_DMA_STREAM)
#error "Invalid DMA stream assigned to SAI1A TX"
#endif

#if STM32_SAI_USE_SAI1B &&                                                  \
    !STM32_DMA_IS_VALID_STREAM(STM32_SAI_SAI1B_RX_DMA_STREAM)
#error "Invalid DMA stream assigned to SAI1B RX"
#endif

#if STM32_SAI_USE_SAI1B &&                                                  \
    !STM32_DMA_IS_VALID_STREAM(STM32_SAI_SAI1B_TX_DMA_STREAM)
#error "Invalid DMA stream assigned to SAI1B TX"
#endif

#if STM32_SAI_USE_SAI2A &&                                                  \
    !STM32_DMA_IS_VALID_STREAM(STM32_SAI_SAI2A_RX_DMA_STREAM)
#error "Invalid DMA stream assigned to SAI2A RX"
#endif

#if STM32_SAI_USE_SAI2A &&                                                  \
    !STM32_DMA_IS_VALID_STREAM(STM32_SAI_SAI2A_TX_DMA_STREAM)
#error "Invalid DMA stream assigned to SAI2A TX"
#endif

#if STM32_SAI_USE_SAI2B &&                                                  \
    !STM32_DMA_IS_VALID_STREAM(STM32_SAI_SAI2B_RX_DMA_STREAM)
#error "Invalid DMA stream assigned to SAI2B RX"
#endif

#if STM32_SAI_USE_SAI2B &&                                                  \
    !STM32_DMA_IS_VALID_STREAM(STM32_SAI_SAI2B_TX_DMA_STREAM)
#error "Invalid DMA stream assigned to SAI2B TX"
#endif

#if !defined(STM32_DMA_REQUIRED)
#define STM32_DMA_REQUIRED
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/**
 * @brief   Low level fields of the SAI driver structure.
 */
#define sai_lld_driver_fields                                               \
  /* Pointer to the SAI global registers (CMSIS: stm32h743xx.h SAI_TypeDef).*/ \
  SAI_TypeDef              *sai;                                            \
  /* Pointer to the SAI block registers (CMSIS: stm32h743xx.h SAI_Block_TypeDef).*/ \
  SAI_Block_TypeDef        *blockp;                                         \
  /* Audio block selector (A/B).*/                                          \
  uint8_t                  block;                                          \
  /* Receive DMA stream.*/                                                  \
  const stm32_dma_stream_t *dmarx;                                          \
  /* Transmit DMA stream.*/                                                 \
  const stm32_dma_stream_t *dmatx;                                          \
  /* RX DMA mode bit mask.*/                                                \
  uint32_t                 rxdmamode;                                       \
  /* TX DMA mode bit mask.*/                                                \
  uint32_t                 txdmamode;                                       \
  /* TX buffer pointer.*/                                                   \
  const void               *txbuf;                                          \
  /* RX buffer pointer.*/                                                   \
  void                     *rxbuf;                                         \
  /* Buffer size in items.*/                                                \
  size_t                   bufsize;                                        \
  /* Latched error flags for bring-up policy (WCKCFG/OVRUDR).*/              \
  uint32_t                 error_flags;                                    \
  /* Repetition counter for persistent error detection.*/                   \
  uint32_t                 error_repeats;

/**
 * @brief   Low level fields of the SAI configuration structure.
 */
#define sai_lld_config_fields                                               \
  /* SAI global configuration register (GCR).*/                             \
  uint32_t                 gcr;                                             \
  /* SAI configuration register 1 (CR1) without SAIEN/DMAEN.*/              \
  uint32_t                 cr1;                                             \
  /* SAI configuration register 2 (CR2).*/                                  \
  uint32_t                 cr2;                                             \
  /* SAI frame configuration register (FRCR).*/                             \
  uint32_t                 frcr;                                            \
  /* SAI slot register (SLOTR).*/                                           \
  uint32_t                 slotr;                                           \
  /* DMA mode bits (PSIZE/MSIZE only, no DIR/CIRC/IRQ).*/                   \
  uint32_t                 dma_mode;

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if STM32_SAI_USE_SAI1A && !defined(__DOXYGEN__)
extern SAIDriver SAID1A;
#endif

#if STM32_SAI_USE_SAI1B && !defined(__DOXYGEN__)
extern SAIDriver SAID1B;
#endif

#if STM32_SAI_USE_SAI2A && !defined(__DOXYGEN__)
extern SAIDriver SAID2A;
#endif

#if STM32_SAI_USE_SAI2B && !defined(__DOXYGEN__)
extern SAIDriver SAID2B;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void sai_lld_init(void);
#if defined(SAI_LLD_ENHANCED_API)
  msg_t sai_lld_start(SAIDriver *saip);
  msg_t sai_lld_stop(SAIDriver *saip);
#else
  void sai_lld_start(SAIDriver *saip);
  void sai_lld_stop(SAIDriver *saip);
#endif
  void sai_lld_set_buffers(SAIDriver *saip,
                           const void *tx_buffer,
                           void *rx_buffer,
                           size_t size);
  void sai_lld_start_exchange(SAIDriver *saip);
#if defined(SAI_LLD_ENHANCED_API)
  msg_t sai_lld_stop_exchange(SAIDriver *saip);
#else
  void sai_lld_stop_exchange(SAIDriver *saip);
#endif
  void sai_dma_error_hook(SAIDriver *saip);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_SAI == TRUE */

#endif /* HAL_SAI_LLD_H */

/** @} */
