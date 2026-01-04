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

#if HAL_USE_SAI || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

typedef struct {
  __IO uint32_t *cr1;
  __IO uint32_t *cr2;
  __IO uint32_t *frcr;
  __IO uint32_t *slotr;
  __IO uint32_t *im;
  __IO uint32_t *clrfr;
  __IO uint32_t *dr;
} sai_block_regs_t;

#define SAI_MODE_RX_MASK (SAI_xCR1_MODE_0 | SAI_xCR1_MODE_1)

#define SAI1A_DMA_REQUEST STM32_DMAMUX1_SAI1_A
#define SAI1B_DMA_REQUEST STM32_DMAMUX1_SAI1_B
#define SAI2A_DMA_REQUEST STM32_DMAMUX1_SAI2_A
#define SAI2B_DMA_REQUEST STM32_DMAMUX1_SAI2_B

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

static void sai_lld_get_block_regs(const SAIDriver *saip,
                                   sai_block_regs_t *regs) {

  if (saip->block == STM32_SAI_BLOCK_A) {
    regs->cr1 = &saip->sai->ACR1;
    regs->cr2 = &saip->sai->ACR2;
    regs->frcr = &saip->sai->AFRCR;
    regs->slotr = &saip->sai->ASLOTR;
    regs->im = &saip->sai->AIM;
    regs->clrfr = &saip->sai->ACLRFR;
    regs->dr = &saip->sai->ADR;
  }
  else {
    regs->cr1 = &saip->sai->BCR1;
    regs->cr2 = &saip->sai->BCR2;
    regs->frcr = &saip->sai->BFRCR;
    regs->slotr = &saip->sai->BSLOTR;
    regs->im = &saip->sai->BIM;
    regs->clrfr = &saip->sai->BCLRFR;
    regs->dr = &saip->sai->BDR;
  }
}

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

#if defined(STM32_SAI_DMA_ERROR_HOOK)
  if ((flags & (STM32_DMA_ISR_TEIF | STM32_DMA_ISR_DMEIF)) != 0U) {
    STM32_SAI_DMA_ERROR_HOOK(saip);
  }
#else
  (void)flags;
#endif

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

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

void sai_lld_init(void) {

#if STM32_SAI_USE_SAI1A
  saiObjectInit(&SAID1A);
  SAID1A.sai = SAI1;
  SAID1A.block = STM32_SAI_BLOCK_A;
#endif

#if STM32_SAI_USE_SAI1B
  saiObjectInit(&SAID1B);
  SAID1B.sai = SAI1;
  SAID1B.block = STM32_SAI_BLOCK_B;
#endif

#if STM32_SAI_USE_SAI2A
  saiObjectInit(&SAID2A);
  SAID2A.sai = SAI2;
  SAID2A.block = STM32_SAI_BLOCK_A;
#endif

#if STM32_SAI_USE_SAI2B
  saiObjectInit(&SAID2B);
  SAID2B.sai = SAI2;
  SAID2B.block = STM32_SAI_BLOCK_B;
#endif
}

void sai_lld_start(SAIDriver *saip) {
  sai_block_regs_t regs;
  uint32_t cr1;
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

  sai_lld_get_block_regs(saip, &regs);

  cr1 = saip->config->cr1 & ~(SAI_xCR1_SAIEN | SAI_xCR1_DMAEN);
  *regs.cr1 &= ~SAI_xCR1_SAIEN;
  while ((*regs.cr1 & SAI_xCR1_SAIEN) != 0U) {
  }
  *regs.cr1 &= ~SAI_xCR1_DMAEN;
  *regs.cr1 &= ~SAI_xCR1_DMAEN;

  /* RM0433: SAI_GCR must be programmed when both blocks are disabled. */
  saip->sai->GCR = saip->config->gcr;
  *regs.cr1 = cr1;
  *regs.cr2 = saip->config->cr2;
  *regs.frcr = saip->config->frcr;
  *regs.slotr = saip->config->slotr;
  *regs.im = 0U;
  *regs.clrfr = 0xFFFFFFFFU;
}

void sai_lld_stop(SAIDriver *saip) {
  sai_block_regs_t regs;

  sai_lld_get_block_regs(saip, &regs);

  *regs.cr1 &= ~SAI_xCR1_SAIEN;
  while ((*regs.cr1 & SAI_xCR1_SAIEN) != 0U) {
  }

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
}

void sai_lld_set_buffers(SAIDriver *saip,
                         const void *tx_buffer,
                         void *rx_buffer,
                         size_t size) {
  sai_block_regs_t regs;
  uint32_t request;
  bool is_rx;

  osalDbgCheck(size > 0U);
  osalDbgCheck(size <= STM32_DMA_MAX_TRANSFER);
  osalDbgCheck((size & 1U) == 0U);

  saip->txbuf = tx_buffer;
  saip->rxbuf = rx_buffer;
  saip->bufsize = size;

  sai_lld_get_block_regs(saip, &regs);
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
    dmaStreamSetPeripheral(saip->dmarx, regs.dr);
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
    dmaStreamSetPeripheral(saip->dmatx, regs.dr);
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
  sai_block_regs_t regs;

  sai_lld_get_block_regs(saip, &regs);

  osalDbgAssert(saip->bufsize > 0U, "buffers not set");

  *regs.cr1 = (*regs.cr1 | SAI_xCR1_DMAEN) & ~SAI_xCR1_SAIEN;
  *regs.cr1 |= SAI_xCR1_SAIEN;
}

void sai_lld_stop_exchange(SAIDriver *saip) {
  sai_block_regs_t regs;

  sai_lld_get_block_regs(saip, &regs);

  *regs.cr1 &= ~SAI_xCR1_SAIEN;
  while ((*regs.cr1 & SAI_xCR1_SAIEN) != 0U) {
  }
  *regs.cr1 &= ~SAI_xCR1_DMAEN;

  if (saip->dmarx != NULL) {
    dmaStreamDisable(saip->dmarx);
  }
  if (saip->dmatx != NULL) {
    dmaStreamDisable(saip->dmatx);
  }
}

#endif /* HAL_USE_SAI */

/** @} */
