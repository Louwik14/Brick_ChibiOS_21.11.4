#include "ch.h"
#include "hal.h"
#include "chprintf.h"

#include "sdram_driver.h"

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

        chprintf(chp, "\r\n");
        chprintf(chp, "========================================\r\n");
        chprintf(chp, " STM32H743 Clock & SDRAM Check\r\n");
        chprintf(chp, "========================================\r\n");

        /* Clocks */
        chprintf(chp, "SYS_CK  : %lu Hz\r\n", STM32_SYS_CK);
        chprintf(chp, "HCLK    : %lu Hz\r\n", STM32_HCLK);
        chprintf(chp, "PCLK1   : %lu Hz\r\n", STM32_PCLK1);
        chprintf(chp, "PCLK2   : %lu Hz\r\n", STM32_PCLK2);
        chprintf(chp, "PCLK3   : %lu Hz\r\n", STM32_PCLK3);
        chprintf(chp, "PCLK4   : %lu Hz\r\n", STM32_PCLK4);

        chprintf(chp, "Assumed FMC SDCLK : ~%lu Hz (HCLK / 2)\r\n",
                  STM32_HCLK / 2U);

        /* Vérification SDRAM */
        if (!sdram_is_initialized()) {
            chprintf(chp, "\r\n[INFO] SDRAM not initialized, starting init...\r\n");

            sdram_init(true);

            sdram_state_t state = sdram_status();
            if ((state != SDRAM_READY) && (state != SDRAM_DEGRADED)) {
                chprintf(chp, "[ERR ] SDRAM init FAILED\r\n");
            } else {
                chprintf(chp, "[OK  ] SDRAM init OK\r\n");

                if (!sdram_run_bist(SDRAM_BIST_MODE_QUICK, NULL)) {
                    chprintf(chp, "[ERR ] SDRAM QUICK_BIST FAILED\r\n");
                } else {
                    chprintf(chp, "[OK  ] SDRAM QUICK_BIST PASSED\r\n");
                }
            }
        } else {
            chprintf(chp, "\r\n[OK  ] SDRAM already initialized\r\n");
        }

        chprintf(chp, "========================================\r\n");

        /* Rafraîchissement toutes les 2 secondes */
        chThdSleepMilliseconds(2000);
    }
}
