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
 * @file    SAIv1/hal_sai_lld.c
 * @brief   STM32 SAI subsystem low level driver source.
 *
 * @addtogroup SAI
 * @{
 */

#include "hal.h"

#if defined(HAL_USE_SAI) && (HAL_USE_SAI == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

#define SAI_MODE_RX_MASK (SAI_xCR1_MODE_0 | SAI_xCR1_MODE_1)

#define SAI1A_DMA_REQUEST STM32_DMAMUX1_SAI1_A
#define SAI1B_DMA_REQUEST STM32_DMAMUX1_SAI1_B
#define SAI2A_DMA_REQUEST STM32_DMAMUX1_SAI2_A
#define SAI2B_DMA_REQUEST STM32_DMAMUX1_SAI2_B

/* Bring-up hardening: bound SAIEN clear waits per "SAI LLD — Bring-up Safe
   Profile & Hardening", aligning with RM expectations while avoiding lockups. */
#define SAI_LLD_SAIEN_CLEAR_TIMEOUT      10000U

/* Bring-up policy (SAI LLD — Hardware Bring-up Playbook / Safe Profile &
   Hardening):
   - tolerate: latch WCKCFG/OVRUDR once (bring-up without codec), clear flags.
   - signal: report latched flags via STM32_SAI_ERROR_HOOK() outside IRQ.
   - stop immediately: if WCKCFG/OVRUDR are persistent or repeated. */
#define SAI_LLD_ERROR_SR_MASK            (SAI_xSR_WCKCFG | SAI_xSR_OVRUDR)
#define SAI_LLD_ERROR_CLRFR_MASK         (SAI_xCLRFR_CWCKCFG | SAI_xCLRFR_COVRUDR)

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

#if STM32_SAI_USE_SAI1A
SAIDriver SAID1A;
#endif

#if STM32_SAI_USE_SAI1B
SAIDriver SAID1B;
#endif

#if STM32_SAI_USE_SAI2A
SAIDriver SAID2A;
#endif

#if STM32_SAI_USE_SAI2B
SAIDriver SAID2B;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

static uint32_t sai1_ref = 0U;
static uint32_t sai2_ref = 0U;

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

static bool sai_lld_is_rx(const SAIDriver *saip) {
  uint32_t mode = saip->config->cr1 & SAI_xCR1_MODE;

  return (mode == SAI_xCR1_MODE_0) || (mode == SAI_MODE_RX_MASK);
}

static uint32_t sai_lld_dma_request(const SAIDriver *saip) {

  if (saip->sai == SAI1) {
    return (saip->block == STM32_SAI_BLOCK_A) ?
           SAI1A_DMA_REQUEST : SAI1B_DMA_REQUEST;
  }

  return (saip->block == STM32_SAI_BLOCK_A) ?
         SAI2A_DMA_REQUEST : SAI2B_DMA_REQUEST;
}

static void sai_lld_serve_dma_interrupt(SAIDriver *saip, uint32_t flags) {
  uint32_t sr = saip->blockp->SR;
  uint32_t critical = sr & SAI_LLD_ERROR_SR_MASK;

#if defined(STM32_SAI_DMA_ERROR_HOOK)
  if ((flags & (STM32_DMA_ISR_TEIF | STM32_DMA_ISR_DMEIF)) != 0U) {
    STM32_SAI_DMA_ERROR_HOOK(saip);
  }
#else
  (void)flags;
#endif

  if (critical != 0U) {
    if ((saip->error_flags & critical) != 0U) {
      saip->error_repeats++;
    }
    saip->error_flags |= critical;
    saip->blockp->CLRFR = SAI_LLD_ERROR_CLRFR_MASK;
  }

  if ((flags & STM32_DMA_ISR_HTIF) != 0U) {
    _sai_isr_half_code(saip);
  }
  if ((flags & STM32_DMA_ISR_TCIF) != 0U) {
    _sai_isr_full_code(saip);
  }
}

static uint32_t sai_lld_stream_id(const SAIDriver *saip, bool is_rx) {

  if (saip->sai == SAI1) {
    if (saip->block == STM32_SAI_BLOCK_A) {
      return is_rx ? STM32_SAI_SAI1A_RX_DMA_STREAM : STM32_SAI_SAI1A_TX_DMA_STREAM;
    }
    return is_rx ? STM32_SAI_SAI1B_RX_DMA_STREAM : STM32_SAI_SAI1B_TX_DMA_STREAM;
  }

  if (saip->block == STM32_SAI_BLOCK_A) {
    return is_rx ? STM32_SAI_SAI2A_RX_DMA_STREAM : STM32_SAI_SAI2A_TX_DMA_STREAM;
  }

  return is_rx ? STM32_SAI_SAI2B_RX_DMA_STREAM : STM32_SAI_SAI2B_TX_DMA_STREAM;
}

static uint32_t sai_lld_irq_priority(const SAIDriver *saip) {

  if (saip->sai == SAI1) {
    return (saip->block == STM32_SAI_BLOCK_A) ? STM32_SAI_SAI1A_IRQ_PRIORITY
                                              : STM32_SAI_SAI1B_IRQ_PRIORITY;
  }

  return (saip->block == STM32_SAI_BLOCK_A) ? STM32_SAI_SAI2A_IRQ_PRIORITY
                                            : STM32_SAI_SAI2B_IRQ_PRIORITY;
}

static uint32_t sai_lld_dma_priority(const SAIDriver *saip) {

  if (saip->sai == SAI1) {
    return (saip->block == STM32_SAI_BLOCK_A) ? STM32_SAI_SAI1A_DMA_PRIORITY
                                              : STM32_SAI_SAI1B_DMA_PRIORITY;
  }

  return (saip->block == STM32_SAI_BLOCK_A) ? STM32_SAI_SAI2A_DMA_PRIORITY
                                            : STM32_SAI_SAI2B_DMA_PRIORITY;
}

static void sai_lld_report_errors(SAIDriver *saip) {
  uint32_t sr = saip->blockp->SR & SAI_LLD_ERROR_SR_MASK;
  uint32_t pending = saip->error_flags & SAI_LLD_ERROR_SR_MASK;

  if ((sr | pending) == 0U) {
    return;
  }

  if (sr != 0U) {
    saip->blockp->CLRFR = SAI_LLD_ERROR_CLRFR_MASK;
  }

  STM32_SAI_ERROR_HOOK(saip, (sr | pending));

  if ((saip->error_repeats != 0U) || (sr != 0U)) {
    osalSysHalt("SAI WCKCFG/OVRUDR persistent");
  }

  saip->error_flags = 0U;
  saip->error_repeats = 0U;
}

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

void sai_lld_init(void) {

#if STM32_SAI_USE_SAI1A
  saiObjectInit(&SAID1A);
  SAID1A.sai = SAI1;
  /* CMSIS mapping: stm32h743xx.h exposes SAI1_Block_A (SAI_Block_TypeDef). */
  SAID1A.blockp = SAI1_Block_A;
  SAID1A.block = STM32_SAI_BLOCK_A;
#endif

#if STM32_SAI_USE_SAI1B
  saiObjectInit(&SAID1B);
  SAID1B.sai = SAI1;
  /* CMSIS mapping: stm32h743xx.h exposes SAI1_Block_B (SAI_Block_TypeDef). */
  SAID1B.blockp = SAI1_Block_B;
  SAID1B.block = STM32_SAI_BLOCK_B;
#endif

#if STM32_SAI_USE_SAI2A
  saiObjectInit(&SAID2A);
  SAID2A.sai = SAI2;
  /* CMSIS mapping: stm32h743xx.h exposes SAI2_Block_A (SAI_Block_TypeDef). */
  SAID2A.blockp = SAI2_Block_A;
  SAID2A.block = STM32_SAI_BLOCK_A;
#endif

#if STM32_SAI_USE_SAI2B
  saiObjectInit(&SAID2B);
  SAID2B.sai = SAI2;
  /* CMSIS mapping: stm32h743xx.h exposes SAI2_Block_B (SAI_Block_TypeDef). */
  SAID2B.blockp = SAI2_Block_B;
  SAID2B.block = STM32_SAI_BLOCK_B;
#endif
}

#if defined(SAI_LLD_ENHANCED_API)
msg_t sai_lld_start(SAIDriver *saip) {
  msg_t msg = HAL_RET_SUCCESS;
#else
void sai_lld_start(SAIDriver *saip) {
#endif
  uint32_t cr1;
  uint32_t timeout;
  bool is_rx;

  if (saip->state == SAI_STOP) {
    if (saip->sai == SAI1) {
      if (sai1_ref++ == 0U) {
        rccEnableSAI1(true);
      }
    }
    else {
      if (sai2_ref++ == 0U) {
        rccEnableSAI2(true);
      }
    }

    is_rx = sai_lld_is_rx(saip);

    if (is_rx) {
      saip->dmarx = dmaStreamAllocI(sai_lld_stream_id(saip, true),
                                    sai_lld_irq_priority(saip),
                                    (stm32_dmaisr_t)sai_lld_serve_dma_interrupt,
                                    saip);
      osalDbgAssert(saip->dmarx != NULL, "DMA RX allocation failed");
      saip->dmatx = NULL;
    }
    else {
      saip->dmatx = dmaStreamAllocI(sai_lld_stream_id(saip, false),
                                    sai_lld_irq_priority(saip),
                                    (stm32_dmaisr_t)sai_lld_serve_dma_interrupt,
                                    saip);
      osalDbgAssert(saip->dmatx != NULL, "DMA TX allocation failed");
      saip->dmarx = NULL;
    }
  }

  saip->error_flags = 0U;
  saip->error_repeats = 0U;

  cr1 = saip->config->cr1 & ~(SAI_xCR1_SAIEN | SAI_xCR1_DMAEN);
  saip->blockp->CR1 &= ~SAI_xCR1_SAIEN;
  timeout = SAI_LLD_SAIEN_CLEAR_TIMEOUT;
  while (((saip->blockp->CR1 & SAI_xCR1_SAIEN) != 0U) && (timeout > 0U)) {
    timeout--;
  }
  if ((saip->blockp->CR1 & SAI_xCR1_SAIEN) != 0U) {
    /* Timeout keeps API behavior, but degrades state for robust bring-up. */
    saip->state = SAI_STOP;
#if defined(SAI_LLD_ENHANCED_API)
    msg = HAL_RET_HW_FAILURE;
    return msg;
#endif
  }
  saip->blockp->CR1 &= ~SAI_xCR1_DMAEN;
  saip->blockp->CR1 &= ~SAI_xCR1_DMAEN;

  /* RM0433: SAI_GCR must be programmed when both blocks are disabled. */
  /* CMSIS mapping (stm32h743xx.h): block registers are in SAI_Block_TypeDef. */
  saip->sai->GCR = saip->config->gcr;
  saip->blockp->CR1 = cr1;
  saip->blockp->CR2 = saip->config->cr2;
  saip->blockp->FRCR = saip->config->frcr;
  saip->blockp->SLOTR = saip->config->slotr;
  saip->blockp->IMR = 0U;
  saip->blockp->CLRFR = 0xFFFFFFFFU;
#if defined(SAI_LLD_ENHANCED_API)
  return msg;
#endif
}

#if defined(SAI_LLD_ENHANCED_API)
msg_t sai_lld_stop(SAIDriver *saip) {
  msg_t msg = HAL_RET_SUCCESS;
#else
void sai_lld_stop(SAIDriver *saip) {
#endif
  uint32_t timeout;

  saip->error_flags = 0U;
  saip->error_repeats = 0U;

  saip->blockp->CR1 &= ~SAI_xCR1_SAIEN;
  timeout = SAI_LLD_SAIEN_CLEAR_TIMEOUT;
  while (((saip->blockp->CR1 & SAI_xCR1_SAIEN) != 0U) && (timeout > 0U)) {
    timeout--;
  }
  if ((saip->blockp->CR1 & SAI_xCR1_SAIEN) != 0U) {
    /* Timeout keeps API behavior, but degrades state for robust bring-up. */
    saip->state = SAI_STOP;
#if defined(SAI_LLD_ENHANCED_API)
    msg = HAL_RET_HW_FAILURE;
#endif
  }
  saip->blockp->CR1 &= ~SAI_xCR1_DMAEN;

  if (saip->dmarx != NULL) {
    dmaStreamDisable(saip->dmarx);
    dmaStreamFreeI(saip->dmarx);
    saip->dmarx = NULL;
  }

  if (saip->dmatx != NULL) {
    dmaStreamDisable(saip->dmatx);
    dmaStreamFreeI(saip->dmatx);
    saip->dmatx = NULL;
  }

  if (saip->sai == SAI1) {
    if (--sai1_ref == 0U) {
      rccDisableSAI1();
    }
  }
  else {
    if (--sai2_ref == 0U) {
      rccDisableSAI2();
    }
  }

#if defined(SAI_LLD_ENHANCED_API)
  return msg;
#endif
}

void sai_lld_set_buffers(SAIDriver *saip,
                         const void *tx_buffer,
                         void *rx_buffer,
                         size_t size) {
  uint32_t request;
  bool is_rx;

  osalDbgCheck(size > 0U);
  osalDbgCheck(size <= STM32_DMA_MAX_TRANSFER);
  osalDbgCheck((size & 1U) == 0U);

  saip->txbuf = tx_buffer;
  saip->rxbuf = rx_buffer;
  saip->bufsize = size;

  request = sai_lld_dma_request(saip);
  is_rx = sai_lld_is_rx(saip);

  if (is_rx) {
    osalDbgCheck(rx_buffer != NULL);
    osalDbgCheck(tx_buffer == NULL);
    osalDbgCheck(saip->dmarx != NULL);
    dmaStreamDisable(saip->dmarx);
    dmaStreamClearInterrupt(saip->dmarx);
    (void)request;
#if STM32_DMA_SUPPORTS_DMAMUX == TRUE
    dmaSetRequestSource(saip->dmarx, request);
#endif
    dmaStreamSetPeripheral(saip->dmarx, &saip->blockp->DR);
    dmaStreamSetMemory0(saip->dmarx, rx_buffer);
    dmaStreamSetTransactionSize(saip->dmarx, size);
    saip->rxdmamode = saip->config->dma_mode | STM32_DMA_CR_DIR_P2M |
                      STM32_DMA_CR_MINC | STM32_DMA_CR_CIRC |
                      STM32_DMA_CR_TCIE | STM32_DMA_CR_HTIE |
                      STM32_DMA_CR_TEIE | STM32_DMA_CR_DMEIE |
                      STM32_DMA_CR_PL(sai_lld_dma_priority(saip));
    dmaStreamSetMode(saip->dmarx, saip->rxdmamode);
    dmaStreamEnable(saip->dmarx);
  }
  else {
    osalDbgCheck(tx_buffer != NULL);
    osalDbgCheck(rx_buffer == NULL);
    osalDbgCheck(saip->dmatx != NULL);
    dmaStreamDisable(saip->dmatx);
    dmaStreamClearInterrupt(saip->dmatx);
    (void)request;
#if STM32_DMA_SUPPORTS_DMAMUX == TRUE
    dmaSetRequestSource(saip->dmatx, request);
#endif
    dmaStreamSetPeripheral(saip->dmatx, &saip->blockp->DR);
    dmaStreamSetMemory0(saip->dmatx, tx_buffer);
    dmaStreamSetTransactionSize(saip->dmatx, size);
    saip->txdmamode = saip->config->dma_mode | STM32_DMA_CR_DIR_M2P |
                      STM32_DMA_CR_MINC | STM32_DMA_CR_CIRC |
                      STM32_DMA_CR_TCIE | STM32_DMA_CR_HTIE |
                      STM32_DMA_CR_TEIE | STM32_DMA_CR_DMEIE |
                      STM32_DMA_CR_PL(sai_lld_dma_priority(saip));
    dmaStreamSetMode(saip->dmatx, saip->txdmamode);
    dmaStreamEnable(saip->dmatx);
  }
}

void sai_lld_start_exchange(SAIDriver *saip) {
  osalDbgAssert(saip->bufsize > 0U, "buffers not set");

  /* Clear error flags before enabling to avoid IRQ storm from latent errors. */
  saip->blockp->CLRFR = SAI_LLD_ERROR_CLRFR_MASK;
  saip->blockp->CR1 = (saip->blockp->CR1 | SAI_xCR1_DMAEN) & ~SAI_xCR1_SAIEN;
  saip->blockp->CR1 |= SAI_xCR1_SAIEN;

  /* Bring-up policy: report errors outside IRQ, stop if persistent/repeated. */
  sai_lld_report_errors(saip);
}

#if defined(SAI_LLD_ENHANCED_API)
msg_t sai_lld_stop_exchange(SAIDriver *saip) {
  msg_t msg = HAL_RET_SUCCESS;
#else
void sai_lld_stop_exchange(SAIDriver *saip) {
#endif
  uint32_t timeout;

  saip->blockp->CR1 &= ~SAI_xCR1_SAIEN;
  timeout = SAI_LLD_SAIEN_CLEAR_TIMEOUT;
  while (((saip->blockp->CR1 & SAI_xCR1_SAIEN) != 0U) && (timeout > 0U)) {
    timeout--;
  }
  if ((saip->blockp->CR1 & SAI_xCR1_SAIEN) != 0U) {
    /* Timeout keeps API behavior, but degrades state for robust bring-up. */
    saip->blockp->CR1 &= ~SAI_xCR1_DMAEN;
    saip->state = SAI_READY;
    if (saip->dmarx != NULL) {
      dmaStreamDisable(saip->dmarx);
    }
    if (saip->dmatx != NULL) {
      dmaStreamDisable(saip->dmatx);
    }
#if defined(SAI_LLD_ENHANCED_API)
    msg = HAL_RET_HW_FAILURE;
#endif
  }
  saip->blockp->CR1 &= ~SAI_xCR1_DMAEN;

  if (saip->dmarx != NULL) {
    dmaStreamDisable(saip->dmarx);
  }
  if (saip->dmatx != NULL) {
    dmaStreamDisable(saip->dmatx);
  }

#if defined(SAI_LLD_ENHANCED_API)
  return msg;
#endif
}

#endif /* HAL_USE_SAI */

/** @} */
