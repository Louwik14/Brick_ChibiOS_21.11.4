/*
 * Minimal SDRAM bring-up for STM32H743 + W9825G6KH-6I
 *
 * SAFE MODE:
 * - SDCLK = 100 MHz (HCLK/2)
 * - 16-bit bus
 * - CAS = 3
 * - No read burst
 * - No read pipeline
 * - No cache
 * - No MPU
 *
 * Tested assumptions:
 * - FMC GPIO pinout already configured by ChibiOS board files
 * - SDRAM connected on FMC SDRAM Bank1 (NE0)
 * - Base address: 0xC0000000
 */

#include "ch.h"
#include "hal.h"

#include "stm32h7xx.h"
#include "stm32h743xx.h"

#include "sdram_hw_minimal.h"

/* --- Constants ----------------------------------------------------------- */

#define SDRAM_REFRESH_COUNT      761u
#define FMC_SDSR_BUSY_BIT        (1u << 5)

/* FMC SDRAM commands (SDCMR.MODE) */
#define SDRAM_CMD_CLK_ENABLE     1u
#define SDRAM_CMD_PALL           2u
#define SDRAM_CMD_AUTOREFRESH    3u
#define SDRAM_CMD_LOAD_MODE      4u

/* Target SDRAM bank: Bank1 (NE0) */
#define SDRAM_FMC_TARGET_BANK    FMC_SDCMR_CTB1

/*
 * SDRAM Mode Register:
 * - Burst Length = 1
 * - Sequential
 * - CAS Latency = 3
 * - Standard operation
 * - Single write burst
 */
#define SDRAM_MODE_REGISTER_VALUE ( \
    (0x0u) |        /* BL = 1 */ \
    (0x0u << 3) |   /* Sequential */ \
    (0x3u << 4) |   /* CAS = 3 */ \
    (0x0u << 7) |   /* Standard */ \
    (0x1u << 9))    /* Single write burst */

/* --- Internal helpers ---------------------------------------------------- */

static void sdram_wait_while_busy(void)
{
    while ((FMC_Bank5_6_R->SDSR & FMC_SDSR_BUSY_BIT) != 0u) {
        __NOP();
    }
}

static void sdram_send_command(uint32_t mode,
                               uint32_t auto_refresh,
                               uint32_t mode_reg)
{
    sdram_wait_while_busy();

    FMC_Bank5_6_R->SDCMR =
        ((mode << FMC_SDCMR_MODE_Pos) & FMC_SDCMR_MODE_Msk) |
        SDRAM_FMC_TARGET_BANK |
        ((((auto_refresh > 0u) ? (auto_refresh - 1u) : 0u)
          << FMC_SDCMR_NRFS_Pos) & FMC_SDCMR_NRFS_Msk) |
        ((mode_reg << FMC_SDCMR_MRD_Pos) & FMC_SDCMR_MRD_Msk);
}

/* --- Public API ---------------------------------------------------------- */

bool sdram_init_minimal(void)
{
    /* Enable FMC peripheral clock */
    RCC->AHB3ENR |= RCC_AHB3ENR_FMCEN;
    (void)RCC->AHB3ENR;
    __DSB();
    __ISB();

    /*
     * SDCR configuration (SAFE MODE)
     * - 9 column bits
     * - 13 row bits
     * - 16-bit data bus
     * - 4 internal banks
     * - CAS latency = 3
     * - SDCLK = HCLK / 2
     */
    const uint32_t sdcr =
        FMC_SDCRx_NC_0 |                  /* 9 columns */
        FMC_SDCRx_NR_1 |                  /* 13 rows */
        FMC_SDCRx_MWID_0 |                /* 16-bit */
        FMC_SDCRx_NB |                    /* 4 banks */
        ((2u << FMC_SDCRx_CAS_Pos) & FMC_SDCRx_CAS_Msk) | /* CAS = 3 */
        FMC_SDCRx_SDCLK_1;                /* SDCLK = HCLK/2 */

    /*
     * Conservative SDRAM timings for 100 MHz SDCLK
     */
    const uint32_t sdtr =
        ((2u - 1u) << FMC_SDTRx_TMRD_Pos) |
        ((7u - 1u) << FMC_SDTRx_TXSR_Pos) |
        ((5u - 1u) << FMC_SDTRx_TRAS_Pos) |
        ((7u - 1u) << FMC_SDTRx_TRC_Pos)  |
        ((2u - 1u) << FMC_SDTRx_TWR_Pos)  |
        ((2u - 1u) << FMC_SDTRx_TRP_Pos)  |
        ((2u - 1u) << FMC_SDTRx_TRCD_Pos);

    /* Apply SDRAM control and timing registers */
    FMC_Bank5_6_R->SDCR[0] = sdcr;
    FMC_Bank5_6_R->SDTR[0] = sdtr;

    /*
     * 🔴 CRITICAL STEP (STM32H7):
     * Enable FMC via Bank1 control register
     * Without this, all FMC accesses will fault.
     */
    FMC_Bank1_R->BTCR[0] |= FMC_BCR1_FMCEN;
    __DSB();
    __ISB();

    /* --- JEDEC SDRAM initialization sequence --- */

    /* Clock enable */
    sdram_send_command(SDRAM_CMD_CLK_ENABLE, 0u, 0u);
    chThdSleepMicroseconds(200u);

    /* Precharge all */
    sdram_send_command(SDRAM_CMD_PALL, 0u, 0u);

    /* Auto-refresh (2 cycles, twice) */
    sdram_send_command(SDRAM_CMD_AUTOREFRESH, 2u, 0u);
    sdram_send_command(SDRAM_CMD_AUTOREFRESH, 2u, 0u);

    /* Load mode register */
    sdram_send_command(SDRAM_CMD_LOAD_MODE, 0u,
                       SDRAM_MODE_REGISTER_VALUE);

    /* Program refresh rate counter */
    FMC_Bank5_6_R->SDRTR =
        (SDRAM_REFRESH_COUNT << FMC_SDRTR_COUNT_Pos) &
        FMC_SDRTR_COUNT_Msk;

    return true;
}
