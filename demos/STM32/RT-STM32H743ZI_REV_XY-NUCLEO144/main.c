#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "sdram_hw_minimal.h"

static const SerialConfig uart_cfg = {
    115200, 0, USART_CR2_STOP1_BITS, 0
};

int main(void) {
    halInit();
    chSysInit();

    sdStart(&SD1, &uart_cfg);
    chThdSleepMilliseconds(200);

    chprintf((BaseSequentialStream*)&SD1,
              "\r\n=== SDRAM MINIMAL TEST ===\r\n");

    if (!sdram_init_minimal()) {
        chprintf((BaseSequentialStream*)&SD1, "FAIL\r\n");
    } else {
        volatile uint32_t *sdram = (volatile uint32_t *)0xC0000000;
        const uint32_t test_value = 0x12345678;
        *sdram = test_value;
        chprintf((BaseSequentialStream*)&SD1, "WRITE OK\r\n");

        if (*sdram == test_value) {
            chprintf((BaseSequentialStream*)&SD1, "READ OK\r\n");
        } else {
            chprintf((BaseSequentialStream*)&SD1, "FAIL\r\n");
        }
    }

    while (true) {
        chThdSleepMilliseconds(1000);
    }
}
