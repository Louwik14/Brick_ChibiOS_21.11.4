#include "ch.h"
#include "hal.h"
#include "drivers/drivers.h"
#include "drv_display.h"
#include <stdio.h>

/* ================== LOW LEVEL ADC ================== */

static void adc_lowlevel_init(void) {
  /* Enable ADC clock */
  RCC->AHB1ENR |= RCC_AHB1ENR_ADC12EN;
  __DSB();

  /* Exit deep power down, enable regulator */
  ADC1->CR &= ~ADC_CR_DEEPPWD;
  ADC1->CR |= ADC_CR_ADVREGEN;
  chThdSleepMilliseconds(1);

  /* Calibrate */
  ADC1->CR &= ~ADC_CR_ADEN;
  ADC1->CR |= ADC_CR_ADCAL;
  while (ADC1->CR & ADC_CR_ADCAL);

  /* Configure ADC */
  ADC1->CFGR = 0;
  ADC1->SMPR1 = ADC_SMPR1_SMP4_2 | ADC_SMPR1_SMP4_1; // long sample time ch4
  ADC1->SQR1 = (4 << ADC_SQR1_SQ1_Pos); // channel 4 = PC4

  /* Enable ADC */
  ADC1->ISR |= ADC_ISR_ADRDY;
  ADC1->CR |= ADC_CR_ADEN;
  while (!(ADC1->ISR & ADC_ISR_ADRDY));
}

static uint16_t adc_read_once(void) {
  ADC1->CR |= ADC_CR_ADSTART;
  while (!(ADC1->ISR & ADC_ISR_EOC));
  return ADC1->DR;
}

/* ================== MAIN ================== */

int main(void) {
  halInit();
  chSysInit();

  drivers_init_all();
  drv_display_init();

  /* PC4 analog */
  palSetPadMode(GPIOC, 4, PAL_MODE_INPUT_ANALOG);

  adc_lowlevel_init();

  char line[32];

  while (true) {
    uint16_t v = adc_read_once();

    drv_display_clear();
    snprintf(line, sizeof(line), "ADC PC4: %u", v);
    drv_display_draw_text(0, 0, line);
    drv_display_update();

    chThdSleepMilliseconds(200);
  }
}
