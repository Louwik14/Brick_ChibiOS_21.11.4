#include "ch.h"
#include "hal.h"
#include "chprintf.h"

#include "sdram_hw_minimal.h"

/* UART1 / SD1 : debug central du projet */
static const SerialConfig uart_cfg = {
    115200,
    0,
    USART_CR2_STOP1_BITS,
    0
};

static BaseSequentialStream *chp = (BaseSequentialStream *)&SD1;

int main(void) {

    halInit();
    chSysInit();

    /* Démarrage UART debug */
    sdStart(&SD1, &uart_cfg);

    chThdSleepMilliseconds(200);

    while (true) {
        bool ok = sdram_init_minimal();
        if (!ok) {
            chprintf(chp, "FAIL\r\n");
            chThdSleepMilliseconds(1000);
            continue;
        }

        volatile uint16_t *sdram_base = (volatile uint16_t *)0xC0000000u;
        const uint16_t test_value = 0xA55Au;
        *sdram_base = test_value;
        chprintf(chp, "WRITE OK\r\n");

        if (*sdram_base == test_value) {
            chprintf(chp, "READ OK\r\n");
        } else {
            chprintf(chp, "FAIL\r\n");
        }

        chThdSleepMilliseconds(1000);
    }
}
