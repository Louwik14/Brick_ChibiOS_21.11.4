#include "ch.h"
#include "hal.h"
#include "stm32h7xx.h"

int main(void) {

  halInit();
  chSysInit();

  RCC->AHB4ENR |= RCC_AHB4ENR_GPIOHEN;

  GPIOH->MODER &= ~(3U << (7*2));
  GPIOH->MODER |=  (1U << (7*2));   // output
  GPIOH->OTYPER &= ~(1U << 7);

  while (1) {
    GPIOH->ODR ^= (1U << 7);
    for (volatile uint32_t i = 0; i < 1000000; i++);
  }
}
