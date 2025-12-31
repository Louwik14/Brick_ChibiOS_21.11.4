#include "ch.h"
#include "hal.h"
#include "chprintf.h"

/*
 * CLOCK + UART + LED validation
 * UART : USART1 (SD1)
 * LED  : PH7 (active-low)
 */

int main(void) {

  halInit();
  chSysInit();

  /* UART1 explicit configuration */
  static const SerialConfig uart_cfg = {
    115200,                 /* baudrate */
    0,
    USART_CR2_STOP1_BITS,
    0
  };
  sdStart(&SD1, &uart_cfg);

  /* LED PH7 (active-low) */
  palSetPadMode(GPIOH, 7, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPad(GPIOH, 7); /* LED OFF */

  chprintf((BaseSequentialStream *)&SD1,
    "\r\n=== CLOCK VALIDATION ===\r\n"
    "SYS_CK = %lu Hz\r\n"
    "HCLK   = %lu Hz\r\n"
    "PCLK1  = %lu Hz\r\n"
    "PCLK2  = %lu Hz\r\n"
    "PCLK3  = %lu Hz\r\n"
    "PCLK4  = %lu Hz\r\n"
    "========================\r\n\r\n",
    (uint32_t)STM32_SYS_CK,
    (uint32_t)STM32_HCLK,
    (uint32_t)STM32_PCLK1,
    (uint32_t)STM32_PCLK2,
    (uint32_t)STM32_PCLK3,
    (uint32_t)STM32_PCLK4
  );

  uint32_t counter = 0;

  while (true) {
    palTogglePad(GPIOH, 7);
    chprintf((BaseSequentialStream *)&SD1,
             "Tick %lu - system alive\r\n",
             counter++);
    chThdSleepMilliseconds(1000);
  }
}
