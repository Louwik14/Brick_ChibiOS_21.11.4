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

    sdStart(&SD1, &uart_cfg);

    chThdSleepMilliseconds(200);

    chprintf(chp, "MAIN START\r\n");

    while (true) {

        chprintf(chp, "BEFORE SDRAM\r\n");
        chThdSleepMilliseconds(500);

        bool ok = sdram_init_minimal();

        chprintf(chp, "AFTER SDRAM\r\n");

        if (!ok) {
            chprintf(chp, "SDRAM INIT FAIL\r\n");
            chThdSleepMilliseconds(1000);
            continue;
        }

        volatile uint16_t *sdram_base = (volatile uint16_t *)0xC0000000u;
        *sdram_base = 0xA55A;
        chprintf(chp, "WRITE DONE\r\n");

        if (*sdram_base == 0xA55A) {
            chprintf(chp, "READ OK\r\n");
        } else {
            chprintf(chp, "READ FAIL\r\n");
        }

        chThdSleepMilliseconds(2000);
    }
}
