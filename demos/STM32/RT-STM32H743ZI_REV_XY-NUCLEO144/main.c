#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "sdram_driver.h"

static const SerialConfig uart_cfg = {
    115200, 0, USART_CR2_STOP1_BITS, 0
};

int main(void) {
    halInit();
    chSysInit();

    sdStart(&SD1, &uart_cfg);
    chThdSleepMilliseconds(200);

    chprintf((BaseSequentialStream*)&SD1,
              "\r\n=== SDRAM INIT TEST ===\r\n");

    chprintf((BaseSequentialStream*)&SD1,
              "[TEST] Calling sdram_init(false)\r\n");

    sdram_init(false);

    chprintf((BaseSequentialStream*)&SD1,
              "[OK] Returned from sdram_init()\r\n");

    while (true) {
        chprintf((BaseSequentialStream*)&SD1,
                  "Main alive\r\n");
        chThdSleepMilliseconds(1000);
    }
}
