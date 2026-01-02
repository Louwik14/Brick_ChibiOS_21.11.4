#include "ch.h"
#include "hal.h"
#include "chprintf.h"

#include "sdram_hw_minimal.h"

/* UART1 / SD1 */
static const SerialConfig uart_cfg = {
    115200,
    0,
    USART_CR2_STOP1_BITS,
    0
};

static BaseSequentialStream *chp = (BaseSequentialStream *)&SD1;

int main(void)
{
    halInit();
    chSysInit();

    sdStart(&SD1, &uart_cfg);
    chThdSleepMilliseconds(200);

    chprintf(chp, "MAIN START\r\n");

    chprintf(chp, "BEFORE SDRAM INIT\r\n");
    bool ok = sdram_init_minimal();
    chprintf(chp, "AFTER SDRAM INIT: %d\r\n", ok ? 1 : 0);

    chprintf(chp, "BEFORE POINTER\r\n");
    volatile uint32_t *sdram =
        (volatile uint32_t *)0xC0000000u;
    chprintf(chp, "AFTER POINTER\r\n");

    chprintf(chp, "BEFORE WRITE\r\n");
    __DSB();
    __ISB();
    *sdram = 0x12345678u;
    __DSB();
    __ISB();
    chprintf(chp, "AFTER WRITE\r\n");

    chprintf(chp, "BEFORE READ\r\n");
    __DSB();
    __ISB();
    uint32_t v = *sdram;
    __DSB();
    __ISB();
    chprintf(chp, "AFTER READ: 0x%08lX\r\n", v);

    if (v == 0x12345678u) {
        chprintf(chp, "SDRAM OK\r\n");
    } else {
        chprintf(chp, "SDRAM FAIL\r\n");
    }

    while (true) {
        chprintf(chp, "ALIVE\r\n");
        chThdSleepMilliseconds(1000);
    }
}
