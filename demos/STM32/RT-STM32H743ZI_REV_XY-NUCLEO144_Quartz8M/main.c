#include "ch.h"
#include "hal.h"
#include "drivers/drivers.h"
#include "drv_display.h"
#include <stdio.h>

/* ADC1 IN7 (PA7) low-level register configuration. */
#define ADC_CHANNEL            7U
#define ADC_CHANNEL_MASK       (1UL << ADC_CHANNEL)
#define ADC_SAMPLE_TIME_BITS   (ADC_SMPR1_SMP7_2 | ADC_SMPR1_SMP7_1 | ADC_SMPR1_SMP7_0) /* 810.5 cycles */

static void adc1_gpio_init(void) {
  /* Enable GPIOA clock. */
  RCC->AHB4ENR |= RCC_AHB4ENR_GPIOAEN;
  (void)RCC->AHB4ENR;

  /* PA7 analog mode, no pull. */
  GPIOA->MODER &= ~(GPIO_MODER_MODE7_Msk);
  GPIOA->MODER |= (3UL << GPIO_MODER_MODE7_Pos);
  GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPD7_Msk);
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

  /* Continuous conversion. */
  ADC1->CFGR |= ADC_CFGR_CONT;

  /* Clear ready flag and enable ADC. */
  ADC1->ISR |= ADC_ISR_ADRDY;
  ADC1->CR |= ADC_CR_ADEN;
  while ((ADC1->ISR & ADC_ISR_ADRDY) == 0U) {
  }

  /* Start conversions. */
  ADC1->CR |= ADC_CR_ADSTART;
}

static uint16_t adc1_read_polling(void) {
  while ((ADC1->ISR & ADC_ISR_EOC) == 0U) {
  }
  uint16_t value = (uint16_t)(ADC1->DR & 0x0FFFU);
  ADC1->ISR |= ADC_ISR_EOC;
  return value;
}



int main(void) {
  halInit();
  chSysInit();

  drivers_init_all();
  drv_display_init();

  adc1_gpio_init();
  adc1_init();

  char line1[32];
  char line2[32];

  uint32_t counter = 0;

  while (true) {
    uint16_t v = adc1_read_polling();

    counter++;

    drv_display_clear();
    drv_display_draw_text(0, 0, "ADC1 POLL OK");

    snprintf(line1, sizeof(line1), "VAL = %u", (unsigned)v);
    drv_display_draw_text(0, 12, line1);

    snprintf(line2, sizeof(line2), "CNT = %lu", (unsigned long)counter);
    drv_display_draw_text(0, 24, line2);

    drv_display_update();

    chThdSleepMilliseconds(200);
  }
}
