#include "ch.h"
#include "hal.h"
#include "drivers/drivers.h"
#include "drv_display.h"
#include <stdio.h>

/* ADC1 IN7 (PA7) low-level register configuration. */
#define ADC_CHANNEL            7U
#define ADC_CHANNEL_MASK       (1UL << ADC_CHANNEL)
#define ADC_SAMPLE_TIME_BITS   (ADC_SMPR1_SMP7_2 | ADC_SMPR1_SMP7_1 | ADC_SMPR1_SMP7_0) /* 810.5 cycles */

#define ADC_DMA_STREAM         DMA2_Stream0
#define ADC_DMAMUX_CHANNEL     DMAMUX1_Channel8
#define ADC_DMAMUX_REQ         STM32_DMAMUX1_ADC1
#define ADC_DMA_BUFFER_DEPTH   16U

/* DMA buffer: MUST be 32-byte aligned and in DMA-safe memory. */
static uint16_t adc_dma_buf[ADC_DMA_BUFFER_DEPTH]
    __attribute__((section(".dma"), aligned(32)));

static void adc1_gpio_init(void) {
  /* Enable GPIOA clock. */
  RCC->AHB4ENR |= RCC_AHB4ENR_GPIOAEN;
  (void)RCC->AHB4ENR;

  /* PA7 analog mode, no pull. */
  GPIOA->MODER &= ~(GPIO_MODER_MODE7_Msk);
  GPIOA->MODER |= (3UL << GPIO_MODER_MODE7_Pos);
  GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPD7_Msk);
}

static void adc1_dma_init(void) {
  /* Enable DMA2 clock. */
  RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;
  (void)RCC->AHB1ENR;

  /* Disable the stream before reconfiguration. */
  ADC_DMA_STREAM->CR &= ~DMA_SxCR_EN;
  while ((ADC_DMA_STREAM->CR & DMA_SxCR_EN) != 0U) {
  }

  /* Clear interrupt flags for stream 0. */
  DMA2->LIFCR = DMA_LIFCR_CTCIF0 | DMA_LIFCR_CHTIF0 | DMA_LIFCR_CTEIF0 |
                DMA_LIFCR_CDMEIF0 | DMA_LIFCR_CFEIF0;

  /* Configure DMAMUX request for ADC1 on DMA2 Stream0 (channel 8). */
  ADC_DMAMUX_CHANNEL->CCR = ADC_DMAMUX_REQ;

  /* Configure DMA stream for peripheral-to-memory circular transfers. */
  ADC_DMA_STREAM->PAR = (uint32_t)&ADC1->DR;
  ADC_DMA_STREAM->M0AR = (uint32_t)adc_dma_buf;
  ADC_DMA_STREAM->NDTR = ADC_DMA_BUFFER_DEPTH;

  ADC_DMA_STREAM->CR =
      DMA_SxCR_PL_1 |                 /* High priority */
      DMA_SxCR_MSIZE_0 |              /* 16-bit memory */
      DMA_SxCR_PSIZE_0 |              /* 16-bit peripheral */
      DMA_SxCR_MINC |                 /* Increment memory */
      DMA_SxCR_CIRC;                  /* Circular mode */

  ADC_DMA_STREAM->FCR = 0U;

  /* Clean buffer before DMA writes (cache). */
  cacheBufferFlush(adc_dma_buf, sizeof(adc_dma_buf));

  /* Enable DMA stream. */
  ADC_DMA_STREAM->CR |= DMA_SxCR_EN;
}

static void adc1_init(void) {
  /* Enable ADC12 clock. */
  RCC->AHB1ENR |= RCC_AHB1ENR_ADC12EN;
  (void)RCC->AHB1ENR;

  /* Ensure ADC is disabled before configuration. */
  if (ADC1->CR & ADC_CR_ADEN) {
    ADC1->CR |= ADC_CR_ADDIS;
    while (ADC1->CR & ADC_CR_ADEN) {
    }
  }

  /* Exit deep power down and enable regulator. */
  ADC1->CR &= ~ADC_CR_DEEPPWD;
  ADC1->CR |= ADC_CR_ADVREGEN;
  chThdSleepMicroseconds(20);

  /* Calibration in single-ended mode. */
  ADC1->CR &= ~ADC_CR_ADCALDIF;
  ADC1->CR |= ADC_CR_ADCAL;
  while (ADC1->CR & ADC_CR_ADCAL) {
  }
  chThdSleepMicroseconds(2);

  /* Configure channel preselection, sample time and sequence. */
  ADC1->PCSEL |= ADC_CHANNEL_MASK;
  ADC1->SMPR1 &= ~ADC_SMPR1_SMP7_Msk;
  ADC1->SMPR1 |= ADC_SAMPLE_TIME_BITS;

  ADC1->SQR1 &= ~(ADC_SQR1_L_Msk | ADC_SQR1_SQ1_Msk);
  ADC1->SQR1 |= (ADC_CHANNEL << ADC_SQR1_SQ1_Pos);

  /* Continuous conversion with DMA circular mode (DMNGT=0b10). */
  ADC1->CFGR &= ~ADC_CFGR_DMNGT_Msk;
  ADC1->CFGR |= ADC_CFGR_CONT | ADC_CFGR_DMNGT_1;

  /* Clear ready flag and enable ADC. */
  ADC1->ISR |= ADC_ISR_ADRDY;
  ADC1->CR |= ADC_CR_ADEN;
  while ((ADC1->ISR & ADC_ISR_ADRDY) == 0U) {
  }

  /* Start conversions. */
  ADC1->CR |= ADC_CR_ADSTART;
}

static uint16_t adc1_read_dma_latest(void) {
  cacheBufferInvalidate(adc_dma_buf, sizeof(adc_dma_buf));

  uint32_t ndtr = ADC_DMA_STREAM->NDTR;
  uint32_t write_index = (ADC_DMA_BUFFER_DEPTH - ndtr) % ADC_DMA_BUFFER_DEPTH;
  uint32_t read_index = (write_index == 0U) ? (ADC_DMA_BUFFER_DEPTH - 1U)
                                            : (write_index - 1U);

  return adc_dma_buf[read_index] & 0x0FFFU;
}



int main(void) {
  halInit();
  chSysInit();

  drivers_init_all();
  drv_display_init();

  adc1_gpio_init();
  adc1_dma_init();
  adc1_init();

  char line1[32];
  char line2[32];

  uint32_t counter = 0;

  while (true) {
    uint16_t v = adc1_read_dma_latest();

    counter++;

    drv_display_clear();
    drv_display_draw_text(0, 0, "ADC1 DMA OK");

    snprintf(line1, sizeof(line1), "VAL = %u", (unsigned)v);
    drv_display_draw_text(0, 12, line1);

    snprintf(line2, sizeof(line2), "CNT = %lu", (unsigned long)counter);
    drv_display_draw_text(0, 24, line2);

    drv_display_update();

    chThdSleepMilliseconds(200);
  }
}
