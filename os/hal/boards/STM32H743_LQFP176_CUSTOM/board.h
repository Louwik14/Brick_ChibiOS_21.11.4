/*
    ChibiOS - Copyright (C) 2006..2025 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/*
 * Custom board definition for STM32H743 LQFP176 (REV XY).
 */

#ifndef BOARD_H
#define BOARD_H

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/*
 * Board identifier.
 */
#define BOARD_STM32H743_LQFP176_CUSTOM
#define BOARD_NAME                  "STM32H743 LQFP176 Custom"

#define FMC_PIN_MODE(pin)           PIN_MODE_ALTERNATE(pin)
#define FMC_PIN_OTYPE(pin)          PIN_OTYPE_PUSHPULL(pin)
#define FMC_PIN_OSPEED(pin)         PIN_OSPEED_HIGH(pin)
#define FMC_PIN_PUPDR(pin)          PIN_PUPDR_FLOATING(pin)
#define FMC_PIN_ODR(pin)            PIN_ODR_LOW(pin)
#define FMC_PIN_AF(pin)             PIN_AFIO_AF(pin, 12U)

#if defined(STM32_I2C_USE_I2C3) && (STM32_I2C_USE_I2C3 == TRUE)
#error "I2C3 uses PH7 (SCL) which conflicts with the LED on PH7."
#endif

/*
 * Board oscillators-related settings.
 */
#if !defined(STM32_LSECLK)
#define STM32_LSECLK                32768U
#endif

#define STM32_LSEDRV                (3U << 3U)

#if !defined(STM32_HSECLK)
#define STM32_HSECLK                25000000U
#endif


/*
 * MCU type as defined in the ST header.
 */
#undef STM32H743xx
#define STM32H743xx

/*
 * IO pins assignments.
 */
#define GPIOA_PIN0                  0U
#define GPIOA_PIN1                  1U
#define GPIOA_PIN2                  2U
#define GPIOA_PIN3                  3U
#define GPIOA_PIN4                  4U
#define GPIOA_PIN5                  5U
#define GPIOA_PIN6                  6U
#define GPIOA_PIN7                  7U
#define GPIOA_PIN8                  8U
#define GPIOA_PIN9                  9U
#define GPIOA_PIN10                 10U
#define GPIOA_PIN11                 11U
#define GPIOA_PIN12                 12U
#define GPIOA_PIN13                 13U
#define GPIOA_PIN14                 14U
#define GPIOA_PIN15                 15U

#define GPIOB_PIN0                  0U
#define GPIOB_PIN1                  1U
#define GPIOB_PIN2                  2U
#define GPIOB_PIN3                  3U
#define GPIOB_PIN4                  4U
#define GPIOB_PIN5                  5U
#define GPIOB_PIN6                  6U
#define GPIOB_PIN7                  7U
#define GPIOB_PIN8                  8U
#define GPIOB_PIN9                  9U
#define GPIOB_PIN10                 10U
#define GPIOB_PIN11                 11U
#define GPIOB_PIN12                 12U
#define GPIOB_PIN13                 13U
#define GPIOB_PIN14                 14U
#define GPIOB_PIN15                 15U

#define GPIOC_PIN0                  0U
#define GPIOC_PIN1                  1U
#define GPIOC_PIN2                  2U
#define GPIOC_PIN3                  3U
#define GPIOC_PIN4                  4U
#define GPIOC_PIN5                  5U
#define GPIOC_PIN6                  6U
#define GPIOC_PIN7                  7U
#define GPIOC_PIN8                  8U
#define GPIOC_PIN9                  9U
#define GPIOC_PIN10                 10U
#define GPIOC_PIN11                 11U
#define GPIOC_PIN12                 12U
#define GPIOC_PIN13                 13U
#define GPIOC_PIN14                 14U
#define GPIOC_PIN15                 15U

#define GPIOD_PIN0                  0U
#define GPIOD_PIN1                  1U
#define GPIOD_PIN2                  2U
#define GPIOD_PIN3                  3U
#define GPIOD_PIN4                  4U
#define GPIOD_PIN5                  5U
#define GPIOD_PIN6                  6U
#define GPIOD_PIN7                  7U
#define GPIOD_PIN8                  8U
#define GPIOD_PIN9                  9U
#define GPIOD_PIN10                 10U
#define GPIOD_PIN11                 11U
#define GPIOD_PIN12                 12U
#define GPIOD_PIN13                 13U
#define GPIOD_PIN14                 14U
#define GPIOD_PIN15                 15U

#define GPIOE_PIN0                  0U
#define GPIOE_PIN1                  1U
#define GPIOE_PIN2                  2U
#define GPIOE_PIN3                  3U
#define GPIOE_PIN4                  4U
#define GPIOE_PIN5                  5U
#define GPIOE_PIN6                  6U
#define GPIOE_PIN7                  7U
#define GPIOE_PIN8                  8U
#define GPIOE_PIN9                  9U
#define GPIOE_PIN10                 10U
#define GPIOE_PIN11                 11U
#define GPIOE_PIN12                 12U
#define GPIOE_PIN13                 13U
#define GPIOE_PIN14                 14U
#define GPIOE_PIN15                 15U

#define GPIOF_PIN0                  0U
#define GPIOF_PIN1                  1U
#define GPIOF_PIN2                  2U
#define GPIOF_PIN3                  3U
#define GPIOF_PIN4                  4U
#define GPIOF_PIN5                  5U
#define GPIOF_PIN6                  6U
#define GPIOF_PIN7                  7U
#define GPIOF_PIN8                  8U
#define GPIOF_PIN9                  9U
#define GPIOF_PIN10                 10U
#define GPIOF_PIN11                 11U
#define GPIOF_PIN12                 12U
#define GPIOF_PIN13                 13U
#define GPIOF_PIN14                 14U
#define GPIOF_PIN15                 15U

#define GPIOG_PIN0                  0U
#define GPIOG_PIN1                  1U
#define GPIOG_PIN2                  2U
#define GPIOG_PIN3                  3U
#define GPIOG_PIN4                  4U
#define GPIOG_PIN5                  5U
#define GPIOG_PIN6                  6U
#define GPIOG_PIN7                  7U
#define GPIOG_PIN8                  8U
#define GPIOG_PIN9                  9U
#define GPIOG_PIN10                 10U
#define GPIOG_PIN11                 11U
#define GPIOG_PIN12                 12U
#define GPIOG_PIN13                 13U
#define GPIOG_PIN14                 14U
#define GPIOG_PIN15                 15U

#define GPIOH_PIN0                  0U
#define GPIOH_PIN1                  1U
#define GPIOH_PIN2                  2U
#define GPIOH_PIN3                  3U
#define GPIOH_PIN4                  4U
#define GPIOH_PIN5                  5U
#define GPIOH_PIN6                  6U
#define GPIOH_PIN7                  7U
#define GPIOH_PIN8                  8U
#define GPIOH_PIN9                  9U
#define GPIOH_PIN10                 10U
#define GPIOH_PIN11                 11U
#define GPIOH_PIN12                 12U
#define GPIOH_PIN13                 13U
#define GPIOH_PIN14                 14U
#define GPIOH_PIN15                 15U

#define GPIOI_PIN0                  0U
#define GPIOI_PIN1                  1U
#define GPIOI_PIN2                  2U
#define GPIOI_PIN3                  3U
#define GPIOI_PIN4                  4U
#define GPIOI_PIN5                  5U
#define GPIOI_PIN6                  6U
#define GPIOI_PIN7                  7U
#define GPIOI_PIN8                  8U
#define GPIOI_PIN9                  9U
#define GPIOI_PIN10                 10U
#define GPIOI_PIN11                 11U
#define GPIOI_PIN12                 12U
#define GPIOI_PIN13                 13U
#define GPIOI_PIN14                 14U
#define GPIOI_PIN15                 15U

#define GPIOJ_PIN0                  0U
#define GPIOJ_PIN1                  1U
#define GPIOJ_PIN2                  2U
#define GPIOJ_PIN3                  3U
#define GPIOJ_PIN4                  4U
#define GPIOJ_PIN5                  5U
#define GPIOJ_PIN6                  6U
#define GPIOJ_PIN7                  7U
#define GPIOJ_PIN8                  8U
#define GPIOJ_PIN9                  9U
#define GPIOJ_PIN10                 10U
#define GPIOJ_PIN11                 11U
#define GPIOJ_PIN12                 12U
#define GPIOJ_PIN13                 13U
#define GPIOJ_PIN14                 14U
#define GPIOJ_PIN15                 15U

#define GPIOK_PIN0                  0U
#define GPIOK_PIN1                  1U
#define GPIOK_PIN2                  2U
#define GPIOK_PIN3                  3U
#define GPIOK_PIN4                  4U
#define GPIOK_PIN5                  5U
#define GPIOK_PIN6                  6U
#define GPIOK_PIN7                  7U
#define GPIOK_PIN8                  8U
#define GPIOK_PIN9                  9U
#define GPIOK_PIN10                 10U
#define GPIOK_PIN11                 11U
#define GPIOK_PIN12                 12U
#define GPIOK_PIN13                 13U
#define GPIOK_PIN14                 14U
#define GPIOK_PIN15                 15U

#define GPIOA_SAI2_SD_B              GPIOA_PIN0
#define GPIOA_MXP_S2                 GPIOA_PIN1
#define GPIOA_MXH_S2                 GPIOA_PIN2
#define GPIOA_MXH_S1                 GPIOA_PIN5
#define GPIOA_MXH_S0                 GPIOA_PIN6
#define GPIOA_ENC2_A                 GPIOA_PIN8
#define GPIOA_USART1_TX              GPIOA_PIN9
#define GPIOA_USART1_RX              GPIOA_PIN10
#define GPIOA_USB_DM                 GPIOA_PIN11
#define GPIOA_USB_DP                 GPIOA_PIN12
#define GPIOA_SWDIO                  GPIOA_PIN13
#define GPIOA_SWCLK                  GPIOA_PIN14

#define GPIOB_MXP_ANALOG             GPIOB_PIN1
#define GPIOB_SPI3_SCK               GPIOB_PIN3
#define GPIOB_SPI3_MISO              GPIOB_PIN4
#define GPIOB_SPI3_MOSI              GPIOB_PIN5
#define GPIOB_SPI3_CS                GPIOB_PIN7
#define GPIOB_UART5_RX               GPIOB_PIN12
#define GPIOB_UART5_TX               GPIOB_PIN13
#define GPIOB_USB2_DM                GPIOB_PIN14
#define GPIOB_USB2_DP                GPIOB_PIN15

#define GPIOC_MXH_ANALOG2            GPIOC_PIN4
#define GPIOC_MXH_ANALOG1            GPIOC_PIN5
/* SD over SPI mapping using former SDMMC pins:
 * CMD -> MOSI (PD2), CLK -> SCK (PC12), DAT0 -> MISO (PC8), DAT3 -> CS (PC11).
 */
#define GPIOC_SD_SPI_MISO            GPIOC_PIN8
#define GPIOC_SD_SPI_D1_UNUSED       GPIOC_PIN9
#define GPIOC_SD_SPI_D2_UNUSED       GPIOC_PIN10
#define GPIOC_SD_SPI_CS              GPIOC_PIN11
#define GPIOC_SD_SPI_SCK             GPIOC_PIN12
#define GPIOC_ENC3_B                 GPIOC_PIN13

#define GPIOD_FMC_D2                 GPIOD_PIN0
#define GPIOD_FMC_D3                 GPIOD_PIN1
#define GPIOD_SD_SPI_MOSI            GPIOD_PIN2
#define GPIOD_SPI6_CS                GPIOD_PIN6
#define GPIOD_SPI1_MISO              GPIOD_PIN7
#define GPIOD_FMC_D13                GPIOD_PIN8
#define GPIOD_FMC_D14                GPIOD_PIN9
#define GPIOD_FMC_D15                GPIOD_PIN10
#define GPIOD_SPI5_DC                GPIOD_PIN11
#define GPIOD_FMC_D0                 GPIOD_PIN14
#define GPIOD_FMC_D1                 GPIOD_PIN15

#define GPIOE_FMC_NBL0               GPIOE_PIN0
#define GPIOE_FMC_NBL1               GPIOE_PIN1
#define GPIOE_SPI5_CS                GPIOE_PIN2
#define GPIOE_FMC_D4                 GPIOE_PIN7
#define GPIOE_FMC_D5                 GPIOE_PIN8
#define GPIOE_FMC_D6                 GPIOE_PIN9
#define GPIOE_FMC_D7                 GPIOE_PIN10
#define GPIOE_FMC_D8                 GPIOE_PIN11
#define GPIOE_FMC_D9                 GPIOE_PIN12
#define GPIOE_FMC_D10                GPIOE_PIN13
#define GPIOE_FMC_D11                GPIOE_PIN14
#define GPIOE_FMC_D12                GPIOE_PIN15

#define GPIOF_FMC_A0                 GPIOF_PIN0
#define GPIOF_FMC_A1                 GPIOF_PIN1
#define GPIOF_FMC_A2                 GPIOF_PIN2
#define GPIOF_FMC_A3                 GPIOF_PIN3
#define GPIOF_FMC_A4                 GPIOF_PIN4
#define GPIOF_FMC_A5                 GPIOF_PIN5
#define GPIOF_SPI5_RESET             GPIOF_PIN6
#define GPIOF_SPI5_SCK               GPIOF_PIN7
#define GPIOF_SPI5_MISO              GPIOF_PIN8
#define GPIOF_SPI5_MOSI              GPIOF_PIN9
#define GPIOF_ENC4_A                 GPIOF_PIN10
#define GPIOF_FMC_SDNRAS             GPIOF_PIN11
#define GPIOF_FMC_A6                 GPIOF_PIN12
#define GPIOF_FMC_A7                 GPIOF_PIN13
#define GPIOF_FMC_A8                 GPIOF_PIN14
#define GPIOF_FMC_A9                 GPIOF_PIN15

#define GPIOG_FMC_A10                GPIOG_PIN0
#define GPIOG_FMC_A11                GPIOG_PIN1
#define GPIOG_FMC_A12                GPIOG_PIN2
#define GPIOG_FMC_BA0                GPIOG_PIN4
#define GPIOG_FMC_BA1                GPIOG_PIN5
#define GPIOG_ENC1_A                 GPIOG_PIN6
#define GPIOG_FMC_SDCLK              GPIOG_PIN8
#define GPIOG_SPI1_MOSI              GPIOG_PIN9
#define GPIOG_SPI1_CS                GPIOG_PIN10
#define GPIOG_SPI1_SCK               GPIOG_PIN11
#define GPIOG_SPI6_MISO              GPIOG_PIN12
#define GPIOG_SPI6_SCK               GPIOG_PIN13
#define GPIOG_SPI6_MOSI              GPIOG_PIN14
#define GPIOG_FMC_SDNCAS             GPIOG_PIN15

#define GPIOH_FMC_SDCKE0             GPIOH_PIN2
#define GPIOH_FMC_SDNE0              GPIOH_PIN3
#define GPIOH_FMC_SDNWE              GPIOH_PIN5

#define GPIOI_SPI2_SCK               GPIOI_PIN1
#define GPIOI_SPI2_MOSI              GPIOI_PIN3
#define GPIOI_SAI2_MCLK_A            GPIOI_PIN4
#define GPIOI_SAI2_SCK_A             GPIOI_PIN5
#define GPIOI_SAI2_SD_A              GPIOI_PIN6
#define GPIOI_SAI2_FS_A              GPIOI_PIN7
#define GPIOI_MXP_S1                 GPIOI_PIN8
#define GPIOI_ENC4_B                 GPIOI_PIN9
#define GPIOI_MXP_S0                 GPIOI_PIN11

/*
 * IO lines assignments.
 */
#define LINE_SAI2_SD_B               PAL_LINE(GPIOA, 0U)
#define LINE_MXP_S2                  PAL_LINE(GPIOA, 1U)
#define LINE_MXH_S2                  PAL_LINE(GPIOA, 2U)
#define LINE_MXH_S1                  PAL_LINE(GPIOA, 5U)
#define LINE_MXH_S0                  PAL_LINE(GPIOA, 6U)
#define LINE_ENC2_A                  PAL_LINE(GPIOA, 8U)
#define LINE_USART1_TX               PAL_LINE(GPIOA, 9U)
#define LINE_USART1_RX               PAL_LINE(GPIOA, 10U)
#define LINE_USB_DM                  PAL_LINE(GPIOA, 11U)
#define LINE_USB_DP                  PAL_LINE(GPIOA, 12U)
#define LINE_SWDIO                   PAL_LINE(GPIOA, 13U)
#define LINE_SWCLK                   PAL_LINE(GPIOA, 14U)

#define LINE_MXP_ANALOG              PAL_LINE(GPIOB, 1U)
#define LINE_SPI3_SCK                PAL_LINE(GPIOB, 3U)
#define LINE_SPI3_MISO               PAL_LINE(GPIOB, 4U)
#define LINE_SPI3_MOSI               PAL_LINE(GPIOB, 5U)
#define LINE_SPI3_CS                 PAL_LINE(GPIOB, 7U)
#define LINE_UART5_RX                PAL_LINE(GPIOB, 12U)
#define LINE_UART5_TX                PAL_LINE(GPIOB, 13U)
#define LINE_USB2_DM                 PAL_LINE(GPIOB, 14U)
#define LINE_USB2_DP                 PAL_LINE(GPIOB, 15U)

#define LINE_MXH_ANALOG2             PAL_LINE(GPIOC, 4U)
#define LINE_MXH_ANALOG1             PAL_LINE(GPIOC, 5U)
#define LINE_SD_SPI_MISO             PAL_LINE(GPIOC, 8U)
#define LINE_SD_SPI_CS               PAL_LINE(GPIOC, 11U)
#define LINE_SD_SPI_SCK              PAL_LINE(GPIOC, 12U)
#define LINE_ENC3_B                  PAL_LINE(GPIOC, 13U)

#define LINE_FMC_D2                  PAL_LINE(GPIOD, 0U)
#define LINE_FMC_D3                  PAL_LINE(GPIOD, 1U)
#define LINE_SD_SPI_MOSI             PAL_LINE(GPIOD, 2U)
#define LINE_SPI6_CS                 PAL_LINE(GPIOD, 6U)
#define LINE_SPI1_MISO               PAL_LINE(GPIOD, 7U)
#define LINE_FMC_D13                 PAL_LINE(GPIOD, 8U)
#define LINE_FMC_D14                 PAL_LINE(GPIOD, 9U)
#define LINE_FMC_D15                 PAL_LINE(GPIOD, 10U)
#define LINE_SPI5_DC                 PAL_LINE(GPIOD, 11U)
#define LINE_FMC_D0                  PAL_LINE(GPIOD, 14U)
#define LINE_FMC_D1                  PAL_LINE(GPIOD, 15U)

#define LINE_FMC_NBL0                PAL_LINE(GPIOE, 0U)
#define LINE_FMC_NBL1                PAL_LINE(GPIOE, 1U)
#define LINE_SPI5_CS                 PAL_LINE(GPIOE, 2U)
#define LINE_FMC_D4                  PAL_LINE(GPIOE, 7U)
#define LINE_FMC_D5                  PAL_LINE(GPIOE, 8U)
#define LINE_FMC_D6                  PAL_LINE(GPIOE, 9U)
#define LINE_FMC_D7                  PAL_LINE(GPIOE, 10U)
#define LINE_FMC_D8                  PAL_LINE(GPIOE, 11U)
#define LINE_FMC_D9                  PAL_LINE(GPIOE, 12U)
#define LINE_FMC_D10                 PAL_LINE(GPIOE, 13U)
#define LINE_FMC_D11                 PAL_LINE(GPIOE, 14U)
#define LINE_FMC_D12                 PAL_LINE(GPIOE, 15U)

#define LINE_FMC_A0                  PAL_LINE(GPIOF, 0U)
#define LINE_FMC_A1                  PAL_LINE(GPIOF, 1U)
#define LINE_FMC_A2                  PAL_LINE(GPIOF, 2U)
#define LINE_FMC_A3                  PAL_LINE(GPIOF, 3U)
#define LINE_FMC_A4                  PAL_LINE(GPIOF, 4U)
#define LINE_FMC_A5                  PAL_LINE(GPIOF, 5U)
#define LINE_SPI5_RESET              PAL_LINE(GPIOF, 6U)
#define LINE_SPI5_SCK                PAL_LINE(GPIOF, 7U)
#define LINE_SPI5_MISO               PAL_LINE(GPIOF, 8U)
#define LINE_SPI5_MOSI               PAL_LINE(GPIOF, 9U)

/* OLED aliases for display driver. */
#define LINE_SPI5_CS_OLED            LINE_SPI5_CS
#define LINE_SPI5_DC_OLED            LINE_SPI5_DC
#define LINE_SPI5_RES_OLED           LINE_SPI5_RESET
#define LINE_ENC4_A                  PAL_LINE(GPIOF, 10U)
#define LINE_FMC_SDNRAS              PAL_LINE(GPIOF, 11U)
#define LINE_FMC_A6                  PAL_LINE(GPIOF, 12U)
#define LINE_FMC_A7                  PAL_LINE(GPIOF, 13U)
#define LINE_FMC_A8                  PAL_LINE(GPIOF, 14U)
#define LINE_FMC_A9                  PAL_LINE(GPIOF, 15U)

#define LINE_FMC_A10                 PAL_LINE(GPIOG, 0U)
#define LINE_FMC_A11                 PAL_LINE(GPIOG, 1U)
#define LINE_FMC_A12                 PAL_LINE(GPIOG, 2U)
#define LINE_FMC_BA0                 PAL_LINE(GPIOG, 4U)
#define LINE_FMC_BA1                 PAL_LINE(GPIOG, 5U)
#define LINE_ENC1_A                  PAL_LINE(GPIOG, 6U)
#define LINE_FMC_SDCLK               PAL_LINE(GPIOG, 8U)
#define LINE_SPI1_MOSI               PAL_LINE(GPIOG, 9U)
#define LINE_SPI1_CS                 PAL_LINE(GPIOG, 10U)
#define LINE_SPI1_SCK                PAL_LINE(GPIOG, 11U)
#define LINE_SPI6_MISO               PAL_LINE(GPIOG, 12U)
#define LINE_SPI6_SCK                PAL_LINE(GPIOG, 13U)
#define LINE_SPI6_MOSI               PAL_LINE(GPIOG, 14U)
#define LINE_FMC_SDNCAS              PAL_LINE(GPIOG, 15U)

#define LINE_FMC_SDCKE0              PAL_LINE(GPIOH, 2U)
#define LINE_FMC_SDNE0               PAL_LINE(GPIOH, 3U)
#define LINE_FMC_SDNWE               PAL_LINE(GPIOH, 5U)

#define LINE_SPI2_SCK                PAL_LINE(GPIOI, 1U)
#define LINE_SPI2_MOSI               PAL_LINE(GPIOI, 3U)
#define LINE_SAI2_MCLK_A             PAL_LINE(GPIOI, 4U)
#define LINE_SAI2_SCK_A              PAL_LINE(GPIOI, 5U)
#define LINE_SAI2_SD_A               PAL_LINE(GPIOI, 6U)
#define LINE_SAI2_FS_A               PAL_LINE(GPIOI, 7U)
#define LINE_MXP_S1                  PAL_LINE(GPIOI, 8U)
#define LINE_ENC4_B                  PAL_LINE(GPIOI, 9U)
#define LINE_MXP_S0                  PAL_LINE(GPIOI, 11U)

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*
 * I/O ports initial setup, this configuration is established soon after reset
 * in the initialization code.
 * Please refer to the STM32 Reference Manual for details.
 */
#define PIN_MODE_INPUT(n)           (0U << ((n) * 2U))
#define PIN_MODE_OUTPUT(n)          (1U << ((n) * 2U))
#define PIN_MODE_ALTERNATE(n)       (2U << ((n) * 2U))
#define PIN_MODE_ANALOG(n)          (3U << ((n) * 2U))
#define PIN_ODR_LOW(n)              (0U << (n))
#define PIN_ODR_HIGH(n)             (1U << (n))
#define PIN_OTYPE_PUSHPULL(n)       (0U << (n))
#define PIN_OTYPE_OPENDRAIN(n)      (1U << (n))
#define PIN_OSPEED_VERYLOW(n)       (0U << ((n) * 2U))
#define PIN_OSPEED_LOW(n)           (1U << ((n) * 2U))
#define PIN_OSPEED_MEDIUM(n)        (2U << ((n) * 2U))
#define PIN_OSPEED_HIGH(n)          (3U << ((n) * 2U))
#define PIN_PUPDR_FLOATING(n)       (0U << ((n) * 2U))
#define PIN_PUPDR_PULLUP(n)         (1U << ((n) * 2U))
#define PIN_PUPDR_PULLDOWN(n)       (2U << ((n) * 2U))
#define PIN_AFIO_AF(n, v)           ((v) << (((n) % 8U) * 4U))

/*
 * GPIOA setup:
 *
 * PA0  - SAI2_SD_B                 (alternate 10).
 * PA1  - MXP_S2                    (output pushpull).
 * PA2  - MXH_S2                    (output pushpull).
 * PA3  - PIN3                      (analog).
 * PA4  - PIN4                      (analog).
 * PA5  - MXH_S1                    (output pushpull).
 * PA6  - MXH_S0                    (output pushpull).
 * PA7  - PIN7                      (analog).
 * PA8  - ENC2_A                    (input).
 * PA9  - USART1_TX                 (alternate 7).
 * PA10 - USART1_RX                 (alternate 7).
 * PA11 - USB_DM                    (alternate 10).
 * PA12 - USB_DP                    (alternate 10).
 * PA13 - SWDIO                     (alternate 0).
 * PA14 - SWCLK                     (alternate 0).
 * PA15 - PIN15                     (analog).
 */
#define VAL_GPIOA_MODER             (PIN_MODE_ALTERNATE(GPIOA_SAI2_SD_B) |  \
                                     PIN_MODE_OUTPUT(GPIOA_MXP_S2) |        \
                                     PIN_MODE_OUTPUT(GPIOA_MXH_S2) |        \
                                     PIN_MODE_ANALOG(GPIOA_PIN3) |          \
                                     PIN_MODE_ANALOG(GPIOA_PIN4) |          \
                                     PIN_MODE_OUTPUT(GPIOA_MXH_S1) |        \
                                     PIN_MODE_OUTPUT(GPIOA_MXH_S0) |        \
                                     PIN_MODE_ANALOG(GPIOA_PIN7) |          \
                                     PIN_MODE_INPUT(GPIOA_ENC2_A) |         \
                                     PIN_MODE_ALTERNATE(GPIOA_USART1_TX) |  \
                                     PIN_MODE_ALTERNATE(GPIOA_USART1_RX) |  \
                                     PIN_MODE_ALTERNATE(GPIOA_USB_DM) |     \
                                     PIN_MODE_ALTERNATE(GPIOA_USB_DP) |     \
                                     PIN_MODE_ALTERNATE(GPIOA_SWDIO) |      \
                                     PIN_MODE_ALTERNATE(GPIOA_SWCLK) |      \
                                     PIN_MODE_ANALOG(GPIOA_PIN15))
#define VAL_GPIOA_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOA_SAI2_SD_B) |   \
                                     PIN_OTYPE_PUSHPULL(GPIOA_MXP_S2) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOA_MXH_S2) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOA_PIN3) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOA_PIN4) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOA_MXH_S1) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOA_MXH_S0) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOA_PIN7) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOA_ENC2_A) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOA_USART1_TX) |  \
                                     PIN_OTYPE_PUSHPULL(GPIOA_USART1_RX) |  \
                                     PIN_OTYPE_PUSHPULL(GPIOA_USB_DM) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOA_USB_DP) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOA_SWDIO) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOA_SWCLK) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOA_PIN15))
#define VAL_GPIOA_OSPEEDR           (PIN_OSPEED_HIGH(GPIOA_SAI2_SD_B) |     \
                                     PIN_OSPEED_LOW(GPIOA_MXP_S2) |         \
                                     PIN_OSPEED_LOW(GPIOA_MXH_S2) |         \
                                     PIN_OSPEED_VERYLOW(GPIOA_PIN3) |       \
                                     PIN_OSPEED_VERYLOW(GPIOA_PIN4) |       \
                                     PIN_OSPEED_LOW(GPIOA_MXH_S1) |         \
                                     PIN_OSPEED_LOW(GPIOA_MXH_S0) |         \
                                     PIN_OSPEED_VERYLOW(GPIOA_PIN7) |       \
                                     PIN_OSPEED_LOW(GPIOA_ENC2_A) |         \
                                     PIN_OSPEED_HIGH(GPIOA_USART1_TX) |     \
                                     PIN_OSPEED_HIGH(GPIOA_USART1_RX) |     \
                                     PIN_OSPEED_HIGH(GPIOA_USB_DM) |        \
                                     PIN_OSPEED_HIGH(GPIOA_USB_DP) |        \
                                     PIN_OSPEED_HIGH(GPIOA_SWDIO) |         \
                                     PIN_OSPEED_HIGH(GPIOA_SWCLK) |         \
                                     PIN_OSPEED_VERYLOW(GPIOA_PIN15))
#define VAL_GPIOA_PUPDR             (PIN_PUPDR_FLOATING(GPIOA_SAI2_SD_B) |  \
                                     PIN_PUPDR_FLOATING(GPIOA_MXP_S2) |     \
                                     PIN_PUPDR_FLOATING(GPIOA_MXH_S2) |     \
                                     PIN_PUPDR_FLOATING(GPIOA_PIN3) |       \
                                     PIN_PUPDR_FLOATING(GPIOA_PIN4) |       \
                                     PIN_PUPDR_FLOATING(GPIOA_MXH_S1) |     \
                                     PIN_PUPDR_FLOATING(GPIOA_MXH_S0) |     \
                                     PIN_PUPDR_FLOATING(GPIOA_PIN7) |       \
                                     PIN_PUPDR_FLOATING(GPIOA_ENC2_A) |     \
                                     PIN_PUPDR_FLOATING(GPIOA_USART1_TX) | \
                                     PIN_PUPDR_FLOATING(GPIOA_USART1_RX) | \
                                     PIN_PUPDR_FLOATING(GPIOA_USB_DM) |    \
                                     PIN_PUPDR_FLOATING(GPIOA_USB_DP) |    \
                                     PIN_PUPDR_FLOATING(GPIOA_SWDIO) |     \
                                     PIN_PUPDR_FLOATING(GPIOA_SWCLK) |     \
                                     PIN_PUPDR_FLOATING(GPIOA_PIN15))
#define VAL_GPIOA_ODR               (PIN_ODR_LOW(GPIOA_SAI2_SD_B) |         \
                                     PIN_ODR_LOW(GPIOA_MXP_S2) |           \
                                     PIN_ODR_LOW(GPIOA_MXH_S2) |           \
                                     PIN_ODR_LOW(GPIOA_PIN3) |             \
                                     PIN_ODR_LOW(GPIOA_PIN4) |             \
                                     PIN_ODR_LOW(GPIOA_MXH_S1) |           \
                                     PIN_ODR_LOW(GPIOA_MXH_S0) |           \
                                     PIN_ODR_LOW(GPIOA_PIN7) |             \
                                     PIN_ODR_LOW(GPIOA_ENC2_A) |           \
                                     PIN_ODR_LOW(GPIOA_USART1_TX) |        \
                                     PIN_ODR_LOW(GPIOA_USART1_RX) |        \
                                     PIN_ODR_LOW(GPIOA_USB_DM) |           \
                                     PIN_ODR_LOW(GPIOA_USB_DP) |           \
                                     PIN_ODR_HIGH(GPIOA_SWDIO) |           \
                                     PIN_ODR_HIGH(GPIOA_SWCLK) |           \
                                     PIN_ODR_LOW(GPIOA_PIN15))
#define VAL_GPIOA_AFRL              (PIN_AFIO_AF(GPIOA_SAI2_SD_B, 10U) |    \
                                     PIN_AFIO_AF(GPIOA_MXP_S2, 0U) |        \
                                     PIN_AFIO_AF(GPIOA_MXH_S2, 0U) |        \
                                     PIN_AFIO_AF(GPIOA_PIN3, 0U) |          \
                                     PIN_AFIO_AF(GPIOA_PIN4, 0U) |          \
                                     PIN_AFIO_AF(GPIOA_MXH_S1, 0U) |        \
                                     PIN_AFIO_AF(GPIOA_MXH_S0, 0U) |        \
                                     PIN_AFIO_AF(GPIOA_PIN7, 0U))
#define VAL_GPIOA_AFRH              (PIN_AFIO_AF(GPIOA_ENC2_A, 0U) |        \
                                     PIN_AFIO_AF(GPIOA_USART1_TX, 7U) |     \
                                     PIN_AFIO_AF(GPIOA_USART1_RX, 7U) |     \
                                     PIN_AFIO_AF(GPIOA_USB_DM, 10U) |       \
                                     PIN_AFIO_AF(GPIOA_USB_DP, 10U) |       \
                                     PIN_AFIO_AF(GPIOA_SWDIO, 0U) |         \
                                     PIN_AFIO_AF(GPIOA_SWCLK, 0U) |         \
                                     PIN_AFIO_AF(GPIOA_PIN15, 0U))

/*
 * GPIOB setup:
 *
 * PB0  - PIN0                      (analog).
 * PB1  - MXP_ANALOG                (analog).
 * PB2  - PIN2                      (analog).
 * PB3  - SPI3_SCK                  (alternate 6).
 * PB4  - SPI3_MISO                 (alternate 6).
 * PB5  - SPI3_MOSI                 (alternate 6).
 * PB6  - PIN6                      (analog).
 * PB7  - SPI3_CS                   (output pushpull).
 * PB8  - PIN8                      (analog).
 * PB9  - PIN9                      (analog).
 * PB10 - PIN10                     (analog).
 * PB11 - PIN11                     (analog).
 * PB12 - UART5_RX                  (alternate 8).
 * PB13 - UART5_TX                  (alternate 8).
 * PB14 - USB2_DM                   (alternate 12).
 * PB15 - USB2_DP                   (alternate 12).
 */
#define VAL_GPIOB_MODER             (PIN_MODE_ANALOG(GPIOB_PIN0) |         \
                                     PIN_MODE_ANALOG(GPIOB_MXP_ANALOG) |    \
                                     PIN_MODE_ANALOG(GPIOB_PIN2) |         \
                                     PIN_MODE_ALTERNATE(GPIOB_SPI3_SCK) |  \
                                     PIN_MODE_ALTERNATE(GPIOB_SPI3_MISO) | \
                                     PIN_MODE_ALTERNATE(GPIOB_SPI3_MOSI) | \
                                     PIN_MODE_ANALOG(GPIOB_PIN6) |         \
                                     PIN_MODE_OUTPUT(GPIOB_SPI3_CS) |      \
                                     PIN_MODE_ANALOG(GPIOB_PIN8) |         \
                                     PIN_MODE_ANALOG(GPIOB_PIN9) |         \
                                     PIN_MODE_ANALOG(GPIOB_PIN10) |        \
                                     PIN_MODE_ANALOG(GPIOB_PIN11) |        \
                                     PIN_MODE_ALTERNATE(GPIOB_UART5_RX) |  \
                                     PIN_MODE_ALTERNATE(GPIOB_UART5_TX) |  \
                                     PIN_MODE_ALTERNATE(GPIOB_USB2_DM) |   \
                                     PIN_MODE_ALTERNATE(GPIOB_USB2_DP))
#define VAL_GPIOB_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOB_PIN0) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOB_MXP_ANALOG) |\
                                     PIN_OTYPE_PUSHPULL(GPIOB_PIN2) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOB_SPI3_SCK) |   \
                                     PIN_OTYPE_PUSHPULL(GPIOB_SPI3_MISO) |  \
                                     PIN_OTYPE_PUSHPULL(GPIOB_SPI3_MOSI) |  \
                                     PIN_OTYPE_PUSHPULL(GPIOB_PIN6) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOB_SPI3_CS) |    \
                                     PIN_OTYPE_PUSHPULL(GPIOB_PIN8) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOB_PIN9) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOB_PIN10) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOB_PIN11) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOB_UART5_RX) |   \
                                     PIN_OTYPE_PUSHPULL(GPIOB_UART5_TX) |   \
                                     PIN_OTYPE_PUSHPULL(GPIOB_USB2_DM) |    \
                                     PIN_OTYPE_PUSHPULL(GPIOB_USB2_DP))
#define VAL_GPIOB_OSPEEDR           (PIN_OSPEED_VERYLOW(GPIOB_PIN0) |      \
                                     PIN_OSPEED_VERYLOW(GPIOB_MXP_ANALOG) |\
                                     PIN_OSPEED_VERYLOW(GPIOB_PIN2) |      \
                                     PIN_OSPEED_HIGH(GPIOB_SPI3_SCK) |     \
                                     PIN_OSPEED_HIGH(GPIOB_SPI3_MISO) |    \
                                     PIN_OSPEED_HIGH(GPIOB_SPI3_MOSI) |    \
                                     PIN_OSPEED_VERYLOW(GPIOB_PIN6) |      \
                                     PIN_OSPEED_LOW(GPIOB_SPI3_CS) |       \
                                     PIN_OSPEED_VERYLOW(GPIOB_PIN8) |      \
                                     PIN_OSPEED_VERYLOW(GPIOB_PIN9) |      \
                                     PIN_OSPEED_VERYLOW(GPIOB_PIN10) |     \
                                     PIN_OSPEED_VERYLOW(GPIOB_PIN11) |     \
                                     PIN_OSPEED_HIGH(GPIOB_UART5_RX) |     \
                                     PIN_OSPEED_HIGH(GPIOB_UART5_TX) |     \
                                     PIN_OSPEED_HIGH(GPIOB_USB2_DM) |      \
                                     PIN_OSPEED_HIGH(GPIOB_USB2_DP))
#define VAL_GPIOB_PUPDR             (PIN_PUPDR_FLOATING(GPIOB_PIN0) |      \
                                     PIN_PUPDR_FLOATING(GPIOB_MXP_ANALOG) |\
                                     PIN_PUPDR_FLOATING(GPIOB_PIN2) |      \
                                     PIN_PUPDR_FLOATING(GPIOB_SPI3_SCK) | \
                                     PIN_PUPDR_FLOATING(GPIOB_SPI3_MISO) |\
                                     PIN_PUPDR_FLOATING(GPIOB_SPI3_MOSI) |\
                                     PIN_PUPDR_FLOATING(GPIOB_PIN6) |      \
                                     PIN_PUPDR_FLOATING(GPIOB_SPI3_CS) |   \
                                     PIN_PUPDR_FLOATING(GPIOB_PIN8) |      \
                                     PIN_PUPDR_FLOATING(GPIOB_PIN9) |      \
                                     PIN_PUPDR_FLOATING(GPIOB_PIN10) |     \
                                     PIN_PUPDR_FLOATING(GPIOB_PIN11) |     \
                                     PIN_PUPDR_FLOATING(GPIOB_UART5_RX) |  \
                                     PIN_PUPDR_FLOATING(GPIOB_UART5_TX) |  \
                                     PIN_PUPDR_FLOATING(GPIOB_USB2_DM) |   \
                                     PIN_PUPDR_FLOATING(GPIOB_USB2_DP))
#define VAL_GPIOB_ODR               (PIN_ODR_LOW(GPIOB_PIN0) |            \
                                     PIN_ODR_LOW(GPIOB_MXP_ANALOG) |       \
                                     PIN_ODR_LOW(GPIOB_PIN2) |             \
                                     PIN_ODR_LOW(GPIOB_SPI3_SCK) |         \
                                     PIN_ODR_LOW(GPIOB_SPI3_MISO) |        \
                                     PIN_ODR_LOW(GPIOB_SPI3_MOSI) |        \
                                     PIN_ODR_LOW(GPIOB_PIN6) |             \
                                     PIN_ODR_HIGH(GPIOB_SPI3_CS) |         \
                                     PIN_ODR_LOW(GPIOB_PIN8) |             \
                                     PIN_ODR_LOW(GPIOB_PIN9) |             \
                                     PIN_ODR_LOW(GPIOB_PIN10) |            \
                                     PIN_ODR_LOW(GPIOB_PIN11) |            \
                                     PIN_ODR_LOW(GPIOB_UART5_RX) |         \
                                     PIN_ODR_LOW(GPIOB_UART5_TX) |         \
                                     PIN_ODR_LOW(GPIOB_USB2_DM) |          \
                                     PIN_ODR_LOW(GPIOB_USB2_DP))
#define VAL_GPIOB_AFRL              (PIN_AFIO_AF(GPIOB_PIN0, 0U) |         \
                                     PIN_AFIO_AF(GPIOB_MXP_ANALOG, 0U) |    \
                                     PIN_AFIO_AF(GPIOB_PIN2, 0U) |         \
                                     PIN_AFIO_AF(GPIOB_SPI3_SCK, 6U) |     \
                                     PIN_AFIO_AF(GPIOB_SPI3_MISO, 6U) |    \
                                     PIN_AFIO_AF(GPIOB_SPI3_MOSI, 6U) |    \
                                     PIN_AFIO_AF(GPIOB_PIN6, 0U) |         \
                                     PIN_AFIO_AF(GPIOB_SPI3_CS, 0U))
#define VAL_GPIOB_AFRH              (PIN_AFIO_AF(GPIOB_PIN8, 0U) |         \
                                     PIN_AFIO_AF(GPIOB_PIN9, 0U) |         \
                                     PIN_AFIO_AF(GPIOB_PIN10, 0U) |        \
                                     PIN_AFIO_AF(GPIOB_PIN11, 0U) |        \
                                     PIN_AFIO_AF(GPIOB_UART5_RX, 8U) |     \
                                     PIN_AFIO_AF(GPIOB_UART5_TX, 8U) |     \
                                     PIN_AFIO_AF(GPIOB_USB2_DM, 12U) |      \
                                     PIN_AFIO_AF(GPIOB_USB2_DP, 12U))

/*
 * GPIOC setup:
 *
 * PC0  - PIN0                      (analog).
 * PC1  - PIN1                      (analog).
 * PC2  - PIN2                      (analog).
 * PC3  - PIN3                      (analog).
 * PC4  - MXH_ANALOG2               (analog).
 * PC5  - MXH_ANALOG1               (analog).
 * PC6  - PIN6                      (analog).
 * PC7  - PIN7                      (analog).
 * PC8  - SD_SPI_MISO               (alternate 6).
 * PC9  - SD_SPI_D1_UNUSED          (analog).
 * PC10 - SD_SPI_D2_UNUSED          (analog).
 * PC11 - SD_SPI_CS                 (output pushpull).
 * PC12 - SD_SPI_SCK                (alternate 6).
 * PC13 - ENC3_B                    (input).
 * PC14 - PIN14                     (analog).
 * PC15 - PIN15                     (analog).
 */
#define VAL_GPIOC_MODER             (PIN_MODE_ANALOG(GPIOC_PIN0) |         \
                                     PIN_MODE_ANALOG(GPIOC_PIN1) |         \
                                     PIN_MODE_ANALOG(GPIOC_PIN2) |         \
                                     PIN_MODE_ANALOG(GPIOC_PIN3) |         \
                                     PIN_MODE_ANALOG(GPIOC_MXH_ANALOG2) |  \
                                     PIN_MODE_ANALOG(GPIOC_MXH_ANALOG1) |  \
                                     PIN_MODE_ANALOG(GPIOC_PIN6) |         \
                                     PIN_MODE_ANALOG(GPIOC_PIN7) |         \
                                     PIN_MODE_ALTERNATE(GPIOC_SD_SPI_MISO) |\
                                     PIN_MODE_ANALOG(GPIOC_SD_SPI_D1_UNUSED) |\
                                     PIN_MODE_ANALOG(GPIOC_SD_SPI_D2_UNUSED) |\
                                     PIN_MODE_OUTPUT(GPIOC_SD_SPI_CS) |    \
                                     PIN_MODE_ALTERNATE(GPIOC_SD_SPI_SCK) |\
                                     PIN_MODE_INPUT(GPIOC_ENC3_B) |        \
                                     PIN_MODE_ANALOG(GPIOC_PIN14) |        \
                                     PIN_MODE_ANALOG(GPIOC_PIN15))
#define VAL_GPIOC_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOC_PIN0) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN1) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN2) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN3) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_MXH_ANALOG2) |\
                                     PIN_OTYPE_PUSHPULL(GPIOC_MXH_ANALOG1) |\
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN6) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN7) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_SD_SPI_MISO) |\
                                     PIN_OTYPE_PUSHPULL(GPIOC_SD_SPI_D1_UNUSED) |\
                                     PIN_OTYPE_PUSHPULL(GPIOC_SD_SPI_D2_UNUSED) |\
                                     PIN_OTYPE_PUSHPULL(GPIOC_SD_SPI_CS) | \
                                     PIN_OTYPE_PUSHPULL(GPIOC_SD_SPI_SCK) |\
                                     PIN_OTYPE_PUSHPULL(GPIOC_ENC3_B) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN14) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN15))
#define VAL_GPIOC_OSPEEDR           (PIN_OSPEED_VERYLOW(GPIOC_PIN0) |      \
                                     PIN_OSPEED_VERYLOW(GPIOC_PIN1) |      \
                                     PIN_OSPEED_VERYLOW(GPIOC_PIN2) |      \
                                     PIN_OSPEED_VERYLOW(GPIOC_PIN3) |      \
                                     PIN_OSPEED_VERYLOW(GPIOC_MXH_ANALOG2) |\
                                     PIN_OSPEED_VERYLOW(GPIOC_MXH_ANALOG1) |\
                                     PIN_OSPEED_VERYLOW(GPIOC_PIN6) |      \
                                     PIN_OSPEED_VERYLOW(GPIOC_PIN7) |      \
                                     PIN_OSPEED_HIGH(GPIOC_SD_SPI_MISO) |  \
                                     PIN_OSPEED_VERYLOW(GPIOC_SD_SPI_D1_UNUSED) |\
                                     PIN_OSPEED_VERYLOW(GPIOC_SD_SPI_D2_UNUSED) |\
                                     PIN_OSPEED_LOW(GPIOC_SD_SPI_CS) |     \
                                     PIN_OSPEED_HIGH(GPIOC_SD_SPI_SCK) |   \
                                     PIN_OSPEED_LOW(GPIOC_ENC3_B) |        \
                                     PIN_OSPEED_VERYLOW(GPIOC_PIN14) |     \
                                     PIN_OSPEED_VERYLOW(GPIOC_PIN15))
#define VAL_GPIOC_PUPDR             (PIN_PUPDR_FLOATING(GPIOC_PIN0) |      \
                                     PIN_PUPDR_FLOATING(GPIOC_PIN1) |      \
                                     PIN_PUPDR_FLOATING(GPIOC_PIN2) |      \
                                     PIN_PUPDR_FLOATING(GPIOC_PIN3) |      \
                                     PIN_PUPDR_FLOATING(GPIOC_MXH_ANALOG2) |\
                                     PIN_PUPDR_FLOATING(GPIOC_MXH_ANALOG1) |\
                                     PIN_PUPDR_FLOATING(GPIOC_PIN6) |      \
                                     PIN_PUPDR_FLOATING(GPIOC_PIN7) |      \
                                     PIN_PUPDR_PULLUP(GPIOC_SD_SPI_MISO) | \
                                     PIN_PUPDR_FLOATING(GPIOC_SD_SPI_D1_UNUSED) |\
                                     PIN_PUPDR_FLOATING(GPIOC_SD_SPI_D2_UNUSED) |\
                                     PIN_PUPDR_FLOATING(GPIOC_SD_SPI_CS) | \
                                     PIN_PUPDR_FLOATING(GPIOC_SD_SPI_SCK) |\
                                     PIN_PUPDR_FLOATING(GPIOC_ENC3_B) |    \
                                     PIN_PUPDR_FLOATING(GPIOC_PIN14) |     \
                                     PIN_PUPDR_FLOATING(GPIOC_PIN15))
#define VAL_GPIOC_ODR               (PIN_ODR_LOW(GPIOC_PIN0) |            \
                                     PIN_ODR_LOW(GPIOC_PIN1) |            \
                                     PIN_ODR_LOW(GPIOC_PIN2) |            \
                                     PIN_ODR_LOW(GPIOC_PIN3) |            \
                                     PIN_ODR_LOW(GPIOC_MXH_ANALOG2) |     \
                                     PIN_ODR_LOW(GPIOC_MXH_ANALOG1) |     \
                                     PIN_ODR_LOW(GPIOC_PIN6) |            \
                                     PIN_ODR_LOW(GPIOC_PIN7) |            \
                                     PIN_ODR_LOW(GPIOC_SD_SPI_MISO) |      \
                                     PIN_ODR_LOW(GPIOC_SD_SPI_D1_UNUSED) | \
                                     PIN_ODR_LOW(GPIOC_SD_SPI_D2_UNUSED) | \
                                     PIN_ODR_HIGH(GPIOC_SD_SPI_CS) |       \
                                     PIN_ODR_LOW(GPIOC_SD_SPI_SCK) |       \
                                     PIN_ODR_LOW(GPIOC_ENC3_B) |           \
                                     PIN_ODR_LOW(GPIOC_PIN14) |           \
                                     PIN_ODR_LOW(GPIOC_PIN15))
#define VAL_GPIOC_AFRL              (PIN_AFIO_AF(GPIOC_PIN0, 0U) |         \
                                     PIN_AFIO_AF(GPIOC_PIN1, 0U) |         \
                                     PIN_AFIO_AF(GPIOC_PIN2, 0U) |         \
                                     PIN_AFIO_AF(GPIOC_PIN3, 0U) |         \
                                     PIN_AFIO_AF(GPIOC_MXH_ANALOG2, 0U) |  \
                                     PIN_AFIO_AF(GPIOC_MXH_ANALOG1, 0U) |  \
                                     PIN_AFIO_AF(GPIOC_PIN6, 0U) |         \
                                     PIN_AFIO_AF(GPIOC_PIN7, 0U))
#define VAL_GPIOC_AFRH              (PIN_AFIO_AF(GPIOC_SD_SPI_MISO, 6U) |  \
                                     PIN_AFIO_AF(GPIOC_SD_SPI_D1_UNUSED, 0U) |\
                                     PIN_AFIO_AF(GPIOC_SD_SPI_D2_UNUSED, 0U) |\
                                     PIN_AFIO_AF(GPIOC_SD_SPI_CS, 0U) |    \
                                     PIN_AFIO_AF(GPIOC_SD_SPI_SCK, 6U) |   \
                                     PIN_AFIO_AF(GPIOC_ENC3_B, 0U) |       \
                                     PIN_AFIO_AF(GPIOC_PIN14, 0U) |        \
                                     PIN_AFIO_AF(GPIOC_PIN15, 0U))

/*
 * GPIOD setup:
 *
 * PD0  - FMC_D2                    (alternate 12).
 * PD1  - FMC_D3                    (alternate 12).
 * PD2  - SD_SPI_MOSI               (alternate 6).
 * PD3  - PIN3                      (analog).
 * PD4  - PIN4                      (analog).
 * PD5  - PIN5                      (analog).
 * PD6  - SPI6_CS                   (output pushpull).
 * PD7  - SPI1_MISO                 (alternate 5).
 * PD8  - FMC_D13                   (alternate 12).
 * PD9  - FMC_D14                   (alternate 12).
 * PD10 - FMC_D15                   (alternate 12).
 * PD11 - SPI5_DC                   (output pushpull).
 * PD12 - PIN12                     (analog).
 * PD13 - PIN13                     (analog).
 * PD14 - FMC_D0                    (alternate 12).
 * PD15 - FMC_D1                    (alternate 12).
 */
#define VAL_GPIOD_MODER             (FMC_PIN_MODE(GPIOD_FMC_D2) |          \
                                     FMC_PIN_MODE(GPIOD_FMC_D3) |          \
                                     PIN_MODE_ALTERNATE(GPIOD_SD_SPI_MOSI) |\
                                     PIN_MODE_ANALOG(GPIOD_PIN3) |         \
                                     PIN_MODE_ANALOG(GPIOD_PIN4) |         \
                                     PIN_MODE_ANALOG(GPIOD_PIN5) |         \
                                     PIN_MODE_OUTPUT(GPIOD_SPI6_CS) |      \
                                     PIN_MODE_ALTERNATE(GPIOD_SPI1_MISO) | \
                                     FMC_PIN_MODE(GPIOD_FMC_D13) |         \
                                     FMC_PIN_MODE(GPIOD_FMC_D14) |         \
                                     FMC_PIN_MODE(GPIOD_FMC_D15) |         \
                                     PIN_MODE_OUTPUT(GPIOD_SPI5_DC) |      \
                                     PIN_MODE_ANALOG(GPIOD_PIN12) |        \
                                     PIN_MODE_ANALOG(GPIOD_PIN13) |        \
                                     FMC_PIN_MODE(GPIOD_FMC_D0) |          \
                                     FMC_PIN_MODE(GPIOD_FMC_D1))
#define VAL_GPIOD_OTYPER            (FMC_PIN_OTYPE(GPIOD_FMC_D2) |          \
                                     FMC_PIN_OTYPE(GPIOD_FMC_D3) |          \
                                     PIN_OTYPE_PUSHPULL(GPIOD_SD_SPI_MOSI) | \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN3) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN4) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN5) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOD_SPI6_CS) |    \
                                     PIN_OTYPE_PUSHPULL(GPIOD_SPI1_MISO) |  \
                                     FMC_PIN_OTYPE(GPIOD_FMC_D13) |         \
                                     FMC_PIN_OTYPE(GPIOD_FMC_D14) |         \
                                     FMC_PIN_OTYPE(GPIOD_FMC_D15) |         \
                                     PIN_OTYPE_PUSHPULL(GPIOD_SPI5_DC) |    \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN12) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN13) |      \
                                     FMC_PIN_OTYPE(GPIOD_FMC_D0) |          \
                                     FMC_PIN_OTYPE(GPIOD_FMC_D1))
#define VAL_GPIOD_OSPEEDR           (FMC_PIN_OSPEED(GPIOD_FMC_D2) |         \
                                     FMC_PIN_OSPEED(GPIOD_FMC_D3) |         \
                                     PIN_OSPEED_HIGH(GPIOD_SD_SPI_MOSI) |  \
                                     PIN_OSPEED_VERYLOW(GPIOD_PIN3) |      \
                                     PIN_OSPEED_VERYLOW(GPIOD_PIN4) |      \
                                     PIN_OSPEED_VERYLOW(GPIOD_PIN5) |      \
                                     PIN_OSPEED_LOW(GPIOD_SPI6_CS) |       \
                                     PIN_OSPEED_HIGH(GPIOD_SPI1_MISO) |    \
                                     FMC_PIN_OSPEED(GPIOD_FMC_D13) |        \
                                     FMC_PIN_OSPEED(GPIOD_FMC_D14) |        \
                                     FMC_PIN_OSPEED(GPIOD_FMC_D15) |        \
                                     PIN_OSPEED_LOW(GPIOD_SPI5_DC) |       \
                                     PIN_OSPEED_VERYLOW(GPIOD_PIN12) |     \
                                     PIN_OSPEED_VERYLOW(GPIOD_PIN13) |     \
                                     FMC_PIN_OSPEED(GPIOD_FMC_D0) |         \
                                     FMC_PIN_OSPEED(GPIOD_FMC_D1))
#define VAL_GPIOD_PUPDR             (FMC_PIN_PUPDR(GPIOD_FMC_D2) |          \
                                     FMC_PIN_PUPDR(GPIOD_FMC_D3) |          \
                                     PIN_PUPDR_PULLUP(GPIOD_SD_SPI_MOSI) | \
                                     PIN_PUPDR_FLOATING(GPIOD_PIN3) |      \
                                     PIN_PUPDR_FLOATING(GPIOD_PIN4) |      \
                                     PIN_PUPDR_FLOATING(GPIOD_PIN5) |      \
                                     PIN_PUPDR_FLOATING(GPIOD_SPI6_CS) |   \
                                     PIN_PUPDR_FLOATING(GPIOD_SPI1_MISO) | \
                                     FMC_PIN_PUPDR(GPIOD_FMC_D13) |         \
                                     FMC_PIN_PUPDR(GPIOD_FMC_D14) |         \
                                     FMC_PIN_PUPDR(GPIOD_FMC_D15) |         \
                                     PIN_PUPDR_FLOATING(GPIOD_SPI5_DC) |   \
                                     PIN_PUPDR_FLOATING(GPIOD_PIN12) |     \
                                     PIN_PUPDR_FLOATING(GPIOD_PIN13) |     \
                                     FMC_PIN_PUPDR(GPIOD_FMC_D0) |          \
                                     FMC_PIN_PUPDR(GPIOD_FMC_D1))
#define VAL_GPIOD_ODR               (FMC_PIN_ODR(GPIOD_FMC_D2) |            \
                                     FMC_PIN_ODR(GPIOD_FMC_D3) |            \
                                     PIN_ODR_LOW(GPIOD_SD_SPI_MOSI) |      \
                                     PIN_ODR_LOW(GPIOD_PIN3) |             \
                                     PIN_ODR_LOW(GPIOD_PIN4) |             \
                                     PIN_ODR_LOW(GPIOD_PIN5) |             \
                                     PIN_ODR_HIGH(GPIOD_SPI6_CS) |         \
                                     PIN_ODR_LOW(GPIOD_SPI1_MISO) |        \
                                     FMC_PIN_ODR(GPIOD_FMC_D13) |           \
                                     FMC_PIN_ODR(GPIOD_FMC_D14) |           \
                                     FMC_PIN_ODR(GPIOD_FMC_D15) |           \
                                     PIN_ODR_LOW(GPIOD_SPI5_DC) |          \
                                     PIN_ODR_LOW(GPIOD_PIN12) |            \
                                     PIN_ODR_LOW(GPIOD_PIN13) |            \
                                     FMC_PIN_ODR(GPIOD_FMC_D0) |            \
                                     FMC_PIN_ODR(GPIOD_FMC_D1))
#define VAL_GPIOD_AFRL              (FMC_PIN_AF(GPIOD_FMC_D2) |             \
                                     FMC_PIN_AF(GPIOD_FMC_D3) |             \
                                     PIN_AFIO_AF(GPIOD_SD_SPI_MOSI, 6U) |  \
                                     PIN_AFIO_AF(GPIOD_PIN3, 0U) |         \
                                     PIN_AFIO_AF(GPIOD_PIN4, 0U) |         \
                                     PIN_AFIO_AF(GPIOD_PIN5, 0U) |         \
                                     PIN_AFIO_AF(GPIOD_SPI6_CS, 0U) |      \
                                     PIN_AFIO_AF(GPIOD_SPI1_MISO, 5U))
#define VAL_GPIOD_AFRH              (FMC_PIN_AF(GPIOD_FMC_D13) |            \
                                     FMC_PIN_AF(GPIOD_FMC_D14) |            \
                                     FMC_PIN_AF(GPIOD_FMC_D15) |            \
                                     PIN_AFIO_AF(GPIOD_SPI5_DC, 0U) |      \
                                     PIN_AFIO_AF(GPIOD_PIN12, 0U) |        \
                                     PIN_AFIO_AF(GPIOD_PIN13, 0U) |        \
                                     FMC_PIN_AF(GPIOD_FMC_D0) |            \
                                     FMC_PIN_AF(GPIOD_FMC_D1))

/*
 * GPIOE setup:
 *
 * PE0  - FMC_NBL0                  (alternate 12).
 * PE1  - FMC_NBL1                  (alternate 12).
 * PE2  - SPI5_CS                   (output pushpull).
 * PE3  - PIN3                      (analog).
 * PE4  - PIN4                      (analog).
 * PE5  - PIN5                      (analog).
 * PE6  - PIN6                      (analog).
 * PE7  - FMC_D4                    (alternate 12).
 * PE8  - FMC_D5                    (alternate 12).
 * PE9  - FMC_D6                    (alternate 12).
 * PE10 - FMC_D7                    (alternate 12).
 * PE11 - FMC_D8                    (alternate 12).
 * PE12 - FMC_D9                    (alternate 12).
 * PE13 - FMC_D10                   (alternate 12).
 * PE14 - FMC_D11                   (alternate 12).
 * PE15 - FMC_D12                   (alternate 12).
 */
#define VAL_GPIOE_MODER             (FMC_PIN_MODE(GPIOE_FMC_NBL0) |        \
                                     FMC_PIN_MODE(GPIOE_FMC_NBL1) |        \
                                     PIN_MODE_OUTPUT(GPIOE_SPI5_CS) |      \
                                     PIN_MODE_ANALOG(GPIOE_PIN3) |         \
                                     PIN_MODE_ANALOG(GPIOE_PIN4) |         \
                                     PIN_MODE_ANALOG(GPIOE_PIN5) |         \
                                     PIN_MODE_ANALOG(GPIOE_PIN6) |         \
                                     FMC_PIN_MODE(GPIOE_FMC_D4) |          \
                                     FMC_PIN_MODE(GPIOE_FMC_D5) |          \
                                     FMC_PIN_MODE(GPIOE_FMC_D6) |          \
                                     FMC_PIN_MODE(GPIOE_FMC_D7) |          \
                                     FMC_PIN_MODE(GPIOE_FMC_D8) |          \
                                     FMC_PIN_MODE(GPIOE_FMC_D9) |          \
                                     FMC_PIN_MODE(GPIOE_FMC_D10) |         \
                                     FMC_PIN_MODE(GPIOE_FMC_D11) |         \
                                     FMC_PIN_MODE(GPIOE_FMC_D12))
#define VAL_GPIOE_OTYPER            (FMC_PIN_OTYPE(GPIOE_FMC_NBL0) |        \
                                     FMC_PIN_OTYPE(GPIOE_FMC_NBL1) |        \
                                     PIN_OTYPE_PUSHPULL(GPIOE_SPI5_CS) |    \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN3) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN4) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN5) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN6) |       \
                                     FMC_PIN_OTYPE(GPIOE_FMC_D4) |          \
                                     FMC_PIN_OTYPE(GPIOE_FMC_D5) |          \
                                     FMC_PIN_OTYPE(GPIOE_FMC_D6) |          \
                                     FMC_PIN_OTYPE(GPIOE_FMC_D7) |          \
                                     FMC_PIN_OTYPE(GPIOE_FMC_D8) |          \
                                     FMC_PIN_OTYPE(GPIOE_FMC_D9) |          \
                                     FMC_PIN_OTYPE(GPIOE_FMC_D10) |         \
                                     FMC_PIN_OTYPE(GPIOE_FMC_D11) |         \
                                     FMC_PIN_OTYPE(GPIOE_FMC_D12))
#define VAL_GPIOE_OSPEEDR           (FMC_PIN_OSPEED(GPIOE_FMC_NBL0) |       \
                                     FMC_PIN_OSPEED(GPIOE_FMC_NBL1) |       \
                                     PIN_OSPEED_LOW(GPIOE_SPI5_CS) |       \
                                     PIN_OSPEED_VERYLOW(GPIOE_PIN3) |      \
                                     PIN_OSPEED_VERYLOW(GPIOE_PIN4) |      \
                                     PIN_OSPEED_VERYLOW(GPIOE_PIN5) |      \
                                     PIN_OSPEED_VERYLOW(GPIOE_PIN6) |      \
                                     FMC_PIN_OSPEED(GPIOE_FMC_D4) |         \
                                     FMC_PIN_OSPEED(GPIOE_FMC_D5) |         \
                                     FMC_PIN_OSPEED(GPIOE_FMC_D6) |         \
                                     FMC_PIN_OSPEED(GPIOE_FMC_D7) |         \
                                     FMC_PIN_OSPEED(GPIOE_FMC_D8) |         \
                                     FMC_PIN_OSPEED(GPIOE_FMC_D9) |         \
                                     FMC_PIN_OSPEED(GPIOE_FMC_D10) |        \
                                     FMC_PIN_OSPEED(GPIOE_FMC_D11) |        \
                                     FMC_PIN_OSPEED(GPIOE_FMC_D12))
#define VAL_GPIOE_PUPDR             (FMC_PIN_PUPDR(GPIOE_FMC_NBL0) |        \
                                     FMC_PIN_PUPDR(GPIOE_FMC_NBL1) |        \
                                     PIN_PUPDR_FLOATING(GPIOE_SPI5_CS) |   \
                                     PIN_PUPDR_FLOATING(GPIOE_PIN3) |      \
                                     PIN_PUPDR_FLOATING(GPIOE_PIN4) |      \
                                     PIN_PUPDR_FLOATING(GPIOE_PIN5) |      \
                                     PIN_PUPDR_FLOATING(GPIOE_PIN6) |      \
                                     FMC_PIN_PUPDR(GPIOE_FMC_D4) |          \
                                     FMC_PIN_PUPDR(GPIOE_FMC_D5) |          \
                                     FMC_PIN_PUPDR(GPIOE_FMC_D6) |          \
                                     FMC_PIN_PUPDR(GPIOE_FMC_D7) |          \
                                     FMC_PIN_PUPDR(GPIOE_FMC_D8) |          \
                                     FMC_PIN_PUPDR(GPIOE_FMC_D9) |          \
                                     FMC_PIN_PUPDR(GPIOE_FMC_D10) |         \
                                     FMC_PIN_PUPDR(GPIOE_FMC_D11) |         \
                                     FMC_PIN_PUPDR(GPIOE_FMC_D12))
#define VAL_GPIOE_ODR               (FMC_PIN_ODR(GPIOE_FMC_NBL0) |          \
                                     FMC_PIN_ODR(GPIOE_FMC_NBL1) |          \
                                     PIN_ODR_HIGH(GPIOE_SPI5_CS) |         \
                                     PIN_ODR_LOW(GPIOE_PIN3) |             \
                                     PIN_ODR_LOW(GPIOE_PIN4) |             \
                                     PIN_ODR_LOW(GPIOE_PIN5) |             \
                                     PIN_ODR_LOW(GPIOE_PIN6) |             \
                                     FMC_PIN_ODR(GPIOE_FMC_D4) |            \
                                     FMC_PIN_ODR(GPIOE_FMC_D5) |            \
                                     FMC_PIN_ODR(GPIOE_FMC_D6) |            \
                                     FMC_PIN_ODR(GPIOE_FMC_D7) |            \
                                     FMC_PIN_ODR(GPIOE_FMC_D8) |            \
                                     FMC_PIN_ODR(GPIOE_FMC_D9) |            \
                                     FMC_PIN_ODR(GPIOE_FMC_D10) |           \
                                     FMC_PIN_ODR(GPIOE_FMC_D11) |           \
                                     FMC_PIN_ODR(GPIOE_FMC_D12))
#define VAL_GPIOE_AFRL              (FMC_PIN_AF(GPIOE_FMC_NBL0) |           \
                                     FMC_PIN_AF(GPIOE_FMC_NBL1) |           \
                                     PIN_AFIO_AF(GPIOE_SPI5_CS, 0U) |      \
                                     PIN_AFIO_AF(GPIOE_PIN3, 0U) |         \
                                     PIN_AFIO_AF(GPIOE_PIN4, 0U) |         \
                                     PIN_AFIO_AF(GPIOE_PIN5, 0U) |         \
                                     PIN_AFIO_AF(GPIOE_PIN6, 0U) |         \
                                     FMC_PIN_AF(GPIOE_FMC_D4))
#define VAL_GPIOE_AFRH              (FMC_PIN_AF(GPIOE_FMC_D5) |             \
                                     FMC_PIN_AF(GPIOE_FMC_D6) |             \
                                     FMC_PIN_AF(GPIOE_FMC_D7) |             \
                                     FMC_PIN_AF(GPIOE_FMC_D8) |             \
                                     FMC_PIN_AF(GPIOE_FMC_D9) |             \
                                     FMC_PIN_AF(GPIOE_FMC_D10) |            \
                                     FMC_PIN_AF(GPIOE_FMC_D11) |            \
                                     FMC_PIN_AF(GPIOE_FMC_D12))

/*
 * GPIOF setup:
 *
 * PF0  - FMC_A0                    (alternate 12).
 * PF1  - FMC_A1                    (alternate 12).
 * PF2  - FMC_A2                    (alternate 12).
 * PF3  - FMC_A3                    (alternate 12).
 * PF4  - FMC_A4                    (alternate 12).
 * PF5  - FMC_A5                    (alternate 12).
 * PF6  - SPI5_RESET                (output pushpull).
 * PF7  - SPI5_SCK                  (alternate 5).
 * PF8  - SPI5_MISO                 (alternate 5).
 * PF9  - SPI5_MOSI                 (alternate 5).
 * PF10 - ENC4_A                    (input).
 * PF11 - FMC_SDNRAS                (alternate 12).
 * PF12 - FMC_A6                    (alternate 12).
 * PF13 - FMC_A7                    (alternate 12).
 * PF14 - FMC_A8                    (alternate 12).
 * PF15 - FMC_A9                    (alternate 12).
 */
#define VAL_GPIOF_MODER             (FMC_PIN_MODE(GPIOF_FMC_A0) |          \
                                     FMC_PIN_MODE(GPIOF_FMC_A1) |          \
                                     FMC_PIN_MODE(GPIOF_FMC_A2) |          \
                                     FMC_PIN_MODE(GPIOF_FMC_A3) |          \
                                     FMC_PIN_MODE(GPIOF_FMC_A4) |          \
                                     FMC_PIN_MODE(GPIOF_FMC_A5) |          \
                                     PIN_MODE_OUTPUT(GPIOF_SPI5_RESET) |   \
                                     PIN_MODE_ALTERNATE(GPIOF_SPI5_SCK) |  \
                                     PIN_MODE_ALTERNATE(GPIOF_SPI5_MISO) | \
                                     PIN_MODE_ALTERNATE(GPIOF_SPI5_MOSI) | \
                                     PIN_MODE_INPUT(GPIOF_ENC4_A) |        \
                                     FMC_PIN_MODE(GPIOF_FMC_SDNRAS) |      \
                                     FMC_PIN_MODE(GPIOF_FMC_A6) |          \
                                     FMC_PIN_MODE(GPIOF_FMC_A7) |          \
                                     FMC_PIN_MODE(GPIOF_FMC_A8) |          \
                                     FMC_PIN_MODE(GPIOF_FMC_A9))
#define VAL_GPIOF_OTYPER            (FMC_PIN_OTYPE(GPIOF_FMC_A0) |          \
                                     FMC_PIN_OTYPE(GPIOF_FMC_A1) |          \
                                     FMC_PIN_OTYPE(GPIOF_FMC_A2) |          \
                                     FMC_PIN_OTYPE(GPIOF_FMC_A3) |          \
                                     FMC_PIN_OTYPE(GPIOF_FMC_A4) |          \
                                     FMC_PIN_OTYPE(GPIOF_FMC_A5) |          \
                                     PIN_OTYPE_PUSHPULL(GPIOF_SPI5_RESET) |\
                                     PIN_OTYPE_PUSHPULL(GPIOF_SPI5_SCK) |  \
                                     PIN_OTYPE_PUSHPULL(GPIOF_SPI5_MISO) | \
                                     PIN_OTYPE_PUSHPULL(GPIOF_SPI5_MOSI) | \
                                     PIN_OTYPE_PUSHPULL(GPIOF_ENC4_A) |     \
                                     FMC_PIN_OTYPE(GPIOF_FMC_SDNRAS) |      \
                                     FMC_PIN_OTYPE(GPIOF_FMC_A6) |          \
                                     FMC_PIN_OTYPE(GPIOF_FMC_A7) |          \
                                     FMC_PIN_OTYPE(GPIOF_FMC_A8) |          \
                                     FMC_PIN_OTYPE(GPIOF_FMC_A9))
#define VAL_GPIOF_OSPEEDR           (FMC_PIN_OSPEED(GPIOF_FMC_A0) |         \
                                     FMC_PIN_OSPEED(GPIOF_FMC_A1) |         \
                                     FMC_PIN_OSPEED(GPIOF_FMC_A2) |         \
                                     FMC_PIN_OSPEED(GPIOF_FMC_A3) |         \
                                     FMC_PIN_OSPEED(GPIOF_FMC_A4) |         \
                                     FMC_PIN_OSPEED(GPIOF_FMC_A5) |         \
                                     PIN_OSPEED_LOW(GPIOF_SPI5_RESET) |     \
                                     PIN_OSPEED_HIGH(GPIOF_SPI5_SCK) |     \
                                     PIN_OSPEED_HIGH(GPIOF_SPI5_MISO) |    \
                                     PIN_OSPEED_HIGH(GPIOF_SPI5_MOSI) |    \
                                     PIN_OSPEED_LOW(GPIOF_ENC4_A) |        \
                                     FMC_PIN_OSPEED(GPIOF_FMC_SDNRAS) |     \
                                     FMC_PIN_OSPEED(GPIOF_FMC_A6) |         \
                                     FMC_PIN_OSPEED(GPIOF_FMC_A7) |         \
                                     FMC_PIN_OSPEED(GPIOF_FMC_A8) |         \
                                     FMC_PIN_OSPEED(GPIOF_FMC_A9))
#define VAL_GPIOF_PUPDR             (FMC_PIN_PUPDR(GPIOF_FMC_A0) |          \
                                     FMC_PIN_PUPDR(GPIOF_FMC_A1) |          \
                                     FMC_PIN_PUPDR(GPIOF_FMC_A2) |          \
                                     FMC_PIN_PUPDR(GPIOF_FMC_A3) |          \
                                     FMC_PIN_PUPDR(GPIOF_FMC_A4) |          \
                                     FMC_PIN_PUPDR(GPIOF_FMC_A5) |          \
                                     PIN_PUPDR_FLOATING(GPIOF_SPI5_RESET) |\
                                     PIN_PUPDR_FLOATING(GPIOF_SPI5_SCK) |  \
                                     PIN_PUPDR_FLOATING(GPIOF_SPI5_MISO) | \
                                     PIN_PUPDR_FLOATING(GPIOF_SPI5_MOSI) | \
                                     PIN_PUPDR_FLOATING(GPIOF_ENC4_A) |    \
                                     FMC_PIN_PUPDR(GPIOF_FMC_SDNRAS) |      \
                                     FMC_PIN_PUPDR(GPIOF_FMC_A6) |          \
                                     FMC_PIN_PUPDR(GPIOF_FMC_A7) |          \
                                     FMC_PIN_PUPDR(GPIOF_FMC_A8) |          \
                                     FMC_PIN_PUPDR(GPIOF_FMC_A9))
#define VAL_GPIOF_ODR               (FMC_PIN_ODR(GPIOF_FMC_A0) |            \
                                     FMC_PIN_ODR(GPIOF_FMC_A1) |            \
                                     FMC_PIN_ODR(GPIOF_FMC_A2) |            \
                                     FMC_PIN_ODR(GPIOF_FMC_A3) |            \
                                     FMC_PIN_ODR(GPIOF_FMC_A4) |            \
                                     FMC_PIN_ODR(GPIOF_FMC_A5) |            \
                                     PIN_ODR_HIGH(GPIOF_SPI5_RESET) |      \
                                     PIN_ODR_LOW(GPIOF_SPI5_SCK) |         \
                                     PIN_ODR_LOW(GPIOF_SPI5_MISO) |        \
                                     PIN_ODR_LOW(GPIOF_SPI5_MOSI) |        \
                                     PIN_ODR_LOW(GPIOF_ENC4_A) |           \
                                     FMC_PIN_ODR(GPIOF_FMC_SDNRAS) |        \
                                     FMC_PIN_ODR(GPIOF_FMC_A6) |            \
                                     FMC_PIN_ODR(GPIOF_FMC_A7) |            \
                                     FMC_PIN_ODR(GPIOF_FMC_A8) |            \
                                     FMC_PIN_ODR(GPIOF_FMC_A9))
#define VAL_GPIOF_AFRL              (FMC_PIN_AF(GPIOF_FMC_A0) |             \
                                     FMC_PIN_AF(GPIOF_FMC_A1) |             \
                                     FMC_PIN_AF(GPIOF_FMC_A2) |             \
                                     FMC_PIN_AF(GPIOF_FMC_A3) |             \
                                     FMC_PIN_AF(GPIOF_FMC_A4) |             \
                                     FMC_PIN_AF(GPIOF_FMC_A5) |             \
                                     PIN_AFIO_AF(GPIOF_SPI5_RESET, 0U) |   \
                                     PIN_AFIO_AF(GPIOF_SPI5_SCK, 5U))
#define VAL_GPIOF_AFRH              (PIN_AFIO_AF(GPIOF_SPI5_MISO, 5U) |    \
                                     PIN_AFIO_AF(GPIOF_SPI5_MOSI, 5U) |    \
                                     PIN_AFIO_AF(GPIOF_ENC4_A, 0U) |       \
                                     FMC_PIN_AF(GPIOF_FMC_SDNRAS) |        \
                                     FMC_PIN_AF(GPIOF_FMC_A6) |            \
                                     FMC_PIN_AF(GPIOF_FMC_A7) |            \
                                     FMC_PIN_AF(GPIOF_FMC_A8) |            \
                                     FMC_PIN_AF(GPIOF_FMC_A9))

/*
 * GPIOG setup:
 *
 * PG0  - FMC_A10                   (alternate 12).
 * PG1  - FMC_A11                   (alternate 12).
 * PG2  - FMC_A12                   (alternate 12).
 * PG3  - PIN3                      (analog).
 * PG4  - FMC_BA0                   (alternate 12).
 * PG5  - FMC_BA1                   (alternate 12).
 * PG6  - ENC1_A                    (input).
 * PG7  - PIN7                      (analog).
 * PG8  - FMC_SDCLK                 (alternate 12).
 * PG9  - SPI1_MOSI                 (alternate 5).
 * PG10 - SPI1_CS                   (output pushpull).
 * PG11 - SPI1_SCK                  (alternate 5).
 * PG12 - SPI6_MISO                 (alternate 5).
 * PG13 - SPI6_SCK                  (alternate 5).
 * PG14 - SPI6_MOSI                 (alternate 5).
 * PG15 - FMC_SDNCAS                (alternate 12).
 */
#define VAL_GPIOG_MODER             (FMC_PIN_MODE(GPIOG_FMC_A10) |         \
                                     FMC_PIN_MODE(GPIOG_FMC_A11) |         \
                                     FMC_PIN_MODE(GPIOG_FMC_A12) |         \
                                     PIN_MODE_ANALOG(GPIOG_PIN3) |         \
                                     FMC_PIN_MODE(GPIOG_FMC_BA0) |         \
                                     FMC_PIN_MODE(GPIOG_FMC_BA1) |         \
                                     PIN_MODE_INPUT(GPIOG_ENC1_A) |        \
                                     PIN_MODE_ANALOG(GPIOG_PIN7) |         \
                                     FMC_PIN_MODE(GPIOG_FMC_SDCLK) |       \
                                     PIN_MODE_ALTERNATE(GPIOG_SPI1_MOSI) |\
                                     PIN_MODE_OUTPUT(GPIOG_SPI1_CS) |      \
                                     PIN_MODE_ALTERNATE(GPIOG_SPI1_SCK) | \
                                     PIN_MODE_ALTERNATE(GPIOG_SPI6_MISO) |\
                                     PIN_MODE_ALTERNATE(GPIOG_SPI6_SCK) | \
                                     PIN_MODE_ALTERNATE(GPIOG_SPI6_MOSI) |\
                                     FMC_PIN_MODE(GPIOG_FMC_SDNCAS))
#define VAL_GPIOG_OTYPER            (FMC_PIN_OTYPE(GPIOG_FMC_A10) |         \
                                     FMC_PIN_OTYPE(GPIOG_FMC_A11) |         \
                                     FMC_PIN_OTYPE(GPIOG_FMC_A12) |         \
                                     PIN_OTYPE_PUSHPULL(GPIOG_PIN3) |       \
                                     FMC_PIN_OTYPE(GPIOG_FMC_BA0) |         \
                                     FMC_PIN_OTYPE(GPIOG_FMC_BA1) |         \
                                     PIN_OTYPE_PUSHPULL(GPIOG_ENC1_A) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOG_PIN7) |       \
                                     FMC_PIN_OTYPE(GPIOG_FMC_SDCLK) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOG_SPI1_MOSI) | \
                                     PIN_OTYPE_PUSHPULL(GPIOG_SPI1_CS) |   \
                                     PIN_OTYPE_PUSHPULL(GPIOG_SPI1_SCK) |  \
                                     PIN_OTYPE_PUSHPULL(GPIOG_SPI6_MISO) | \
                                     PIN_OTYPE_PUSHPULL(GPIOG_SPI6_SCK) |  \
                                     PIN_OTYPE_PUSHPULL(GPIOG_SPI6_MOSI) | \
                                     FMC_PIN_OTYPE(GPIOG_FMC_SDNCAS))
#define VAL_GPIOG_OSPEEDR           (FMC_PIN_OSPEED(GPIOG_FMC_A10) |        \
                                     FMC_PIN_OSPEED(GPIOG_FMC_A11) |        \
                                     FMC_PIN_OSPEED(GPIOG_FMC_A12) |        \
                                     PIN_OSPEED_VERYLOW(GPIOG_PIN3) |      \
                                     FMC_PIN_OSPEED(GPIOG_FMC_BA0) |        \
                                     FMC_PIN_OSPEED(GPIOG_FMC_BA1) |        \
                                     PIN_OSPEED_LOW(GPIOG_ENC1_A) |         \
                                     PIN_OSPEED_VERYLOW(GPIOG_PIN7) |      \
                                     FMC_PIN_OSPEED(GPIOG_FMC_SDCLK) |      \
                                     PIN_OSPEED_HIGH(GPIOG_SPI1_MOSI) |    \
                                     PIN_OSPEED_LOW(GPIOG_SPI1_CS) |        \
                                     PIN_OSPEED_HIGH(GPIOG_SPI1_SCK) |     \
                                     PIN_OSPEED_HIGH(GPIOG_SPI6_MISO) |    \
                                     PIN_OSPEED_HIGH(GPIOG_SPI6_SCK) |     \
                                     PIN_OSPEED_HIGH(GPIOG_SPI6_MOSI) |    \
                                     FMC_PIN_OSPEED(GPIOG_FMC_SDNCAS))
#define VAL_GPIOG_PUPDR             (FMC_PIN_PUPDR(GPIOG_FMC_A10) |         \
                                     FMC_PIN_PUPDR(GPIOG_FMC_A11) |         \
                                     FMC_PIN_PUPDR(GPIOG_FMC_A12) |         \
                                     PIN_PUPDR_FLOATING(GPIOG_PIN3) |      \
                                     FMC_PIN_PUPDR(GPIOG_FMC_BA0) |         \
                                     FMC_PIN_PUPDR(GPIOG_FMC_BA1) |         \
                                     PIN_PUPDR_FLOATING(GPIOG_ENC1_A) |    \
                                     PIN_PUPDR_FLOATING(GPIOG_PIN7) |      \
                                     FMC_PIN_PUPDR(GPIOG_FMC_SDCLK) |       \
                                     PIN_PUPDR_FLOATING(GPIOG_SPI1_MOSI) |\
                                     PIN_PUPDR_FLOATING(GPIOG_SPI1_CS) |   \
                                     PIN_PUPDR_FLOATING(GPIOG_SPI1_SCK) |  \
                                     PIN_PUPDR_FLOATING(GPIOG_SPI6_MISO) | \
                                     PIN_PUPDR_FLOATING(GPIOG_SPI6_SCK) |  \
                                     PIN_PUPDR_FLOATING(GPIOG_SPI6_MOSI) | \
                                     FMC_PIN_PUPDR(GPIOG_FMC_SDNCAS))
#define VAL_GPIOG_ODR               (FMC_PIN_ODR(GPIOG_FMC_A10) |           \
                                     FMC_PIN_ODR(GPIOG_FMC_A11) |           \
                                     FMC_PIN_ODR(GPIOG_FMC_A12) |           \
                                     PIN_ODR_LOW(GPIOG_PIN3) |             \
                                     FMC_PIN_ODR(GPIOG_FMC_BA0) |           \
                                     FMC_PIN_ODR(GPIOG_FMC_BA1) |           \
                                     PIN_ODR_LOW(GPIOG_ENC1_A) |           \
                                     PIN_ODR_LOW(GPIOG_PIN7) |             \
                                     FMC_PIN_ODR(GPIOG_FMC_SDCLK) |         \
                                     PIN_ODR_LOW(GPIOG_SPI1_MOSI) |        \
                                     PIN_ODR_HIGH(GPIOG_SPI1_CS) |         \
                                     PIN_ODR_LOW(GPIOG_SPI1_SCK) |         \
                                     PIN_ODR_LOW(GPIOG_SPI6_MISO) |        \
                                     PIN_ODR_LOW(GPIOG_SPI6_SCK) |         \
                                     PIN_ODR_LOW(GPIOG_SPI6_MOSI) |        \
                                     FMC_PIN_ODR(GPIOG_FMC_SDNCAS))
#define VAL_GPIOG_AFRL              (FMC_PIN_AF(GPIOG_FMC_A10) |            \
                                     FMC_PIN_AF(GPIOG_FMC_A11) |            \
                                     FMC_PIN_AF(GPIOG_FMC_A12) |            \
                                     PIN_AFIO_AF(GPIOG_PIN3, 0U) |         \
                                     FMC_PIN_AF(GPIOG_FMC_BA0) |            \
                                     FMC_PIN_AF(GPIOG_FMC_BA1) |            \
                                     PIN_AFIO_AF(GPIOG_ENC1_A, 0U) |        \
                                     PIN_AFIO_AF(GPIOG_PIN7, 0U))
#define VAL_GPIOG_AFRH              (FMC_PIN_AF(GPIOG_FMC_SDCLK) |          \
                                     PIN_AFIO_AF(GPIOG_SPI1_MOSI, 5U) |    \
                                     PIN_AFIO_AF(GPIOG_SPI1_CS, 0U) |      \
                                     PIN_AFIO_AF(GPIOG_SPI1_SCK, 5U) |     \
                                     PIN_AFIO_AF(GPIOG_SPI6_MISO, 5U) |    \
                                     PIN_AFIO_AF(GPIOG_SPI6_SCK, 5U) |     \
                                     PIN_AFIO_AF(GPIOG_SPI6_MOSI, 5U) |    \
                                     FMC_PIN_AF(GPIOG_FMC_SDNCAS))

/*
 * GPIOH setup:
 *
 * PH0  - PIN0                      (analog).
 * PH1  - PIN1                      (analog).
 * PH2  - FMC_SDCKE0                (alternate 12).
 * PH3  - FMC_SDNE0                 (alternate 12).
 * PH4  - PIN4                      (analog).
 * PH5  - FMC_SDNWE                 (alternate 12).
 * PH6  - PIN6                      (analog).
 * PH7  - LED                       (output push-pull).
 *        (ancienne config: PIN7    (analog).)
 *        NOTE: PH7 maps to I2C3_SCL (AF4). Keep I2C3 disabled or remap.
 * PH8  - PIN8                      (analog).
 * PH9  - PIN9                      (analog).
 * PH10 - PIN10                     (analog).
 * PH11 - PIN11                     (analog).
 * PH12 - PIN12                     (analog).
 * PH13 - PIN13                     (analog).
 * PH14 - PIN14                     (analog).
 * PH15 - PIN15                     (analog).
 */
#define VAL_GPIOH_MODER             (PIN_MODE_ANALOG(GPIOH_PIN0) |         \
                                     PIN_MODE_ANALOG(GPIOH_PIN1) |         \
                                     FMC_PIN_MODE(GPIOH_FMC_SDCKE0) |      \
                                     FMC_PIN_MODE(GPIOH_FMC_SDNE0) |       \
                                     PIN_MODE_ANALOG(GPIOH_PIN4) |         \
                                     FMC_PIN_MODE(GPIOH_FMC_SDNWE) |       \
                                     PIN_MODE_ANALOG(GPIOH_PIN6) |         \
                                     PIN_MODE_OUTPUT(GPIOH_PIN7) |         \
                                     PIN_MODE_ANALOG(GPIOH_PIN8) |         \
                                     PIN_MODE_ANALOG(GPIOH_PIN9) |         \
                                     PIN_MODE_ANALOG(GPIOH_PIN10) |        \
                                     PIN_MODE_ANALOG(GPIOH_PIN11) |        \
                                     PIN_MODE_ANALOG(GPIOH_PIN12) |        \
                                     PIN_MODE_ANALOG(GPIOH_PIN13) |        \
                                     PIN_MODE_ANALOG(GPIOH_PIN14) |        \
                                     PIN_MODE_ANALOG(GPIOH_PIN15))
#define VAL_GPIOH_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOH_PIN0) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN1) |       \
                                     FMC_PIN_OTYPE(GPIOH_FMC_SDCKE0) |      \
                                     FMC_PIN_OTYPE(GPIOH_FMC_SDNE0) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN4) |       \
                                     FMC_PIN_OTYPE(GPIOH_FMC_SDNWE) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN6) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN7) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN8) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN9) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN10) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN11) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN12) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN13) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN14) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN15))
#define VAL_GPIOH_OSPEEDR           (PIN_OSPEED_VERYLOW(GPIOH_PIN0) |      \
                                     PIN_OSPEED_VERYLOW(GPIOH_PIN1) |      \
                                     FMC_PIN_OSPEED(GPIOH_FMC_SDCKE0) |     \
                                     FMC_PIN_OSPEED(GPIOH_FMC_SDNE0) |      \
                                     PIN_OSPEED_VERYLOW(GPIOH_PIN4) |      \
                                     FMC_PIN_OSPEED(GPIOH_FMC_SDNWE) |      \
                                     PIN_OSPEED_VERYLOW(GPIOH_PIN6) |      \
                                     PIN_OSPEED_LOW(GPIOH_PIN7) |          \
                                     PIN_OSPEED_VERYLOW(GPIOH_PIN8) |      \
                                     PIN_OSPEED_VERYLOW(GPIOH_PIN9) |      \
                                     PIN_OSPEED_VERYLOW(GPIOH_PIN10) |     \
                                     PIN_OSPEED_VERYLOW(GPIOH_PIN11) |     \
                                     PIN_OSPEED_VERYLOW(GPIOH_PIN12) |     \
                                     PIN_OSPEED_VERYLOW(GPIOH_PIN13) |     \
                                     PIN_OSPEED_VERYLOW(GPIOH_PIN14) |     \
                                     PIN_OSPEED_VERYLOW(GPIOH_PIN15))
#define VAL_GPIOH_PUPDR             (PIN_PUPDR_FLOATING(GPIOH_PIN0) |      \
                                     PIN_PUPDR_FLOATING(GPIOH_PIN1) |      \
                                     FMC_PIN_PUPDR(GPIOH_FMC_SDCKE0) |      \
                                     FMC_PIN_PUPDR(GPIOH_FMC_SDNE0) |       \
                                     PIN_PUPDR_FLOATING(GPIOH_PIN4) |      \
                                     FMC_PIN_PUPDR(GPIOH_FMC_SDNWE) |       \
                                     PIN_PUPDR_FLOATING(GPIOH_PIN6) |      \
                                     PIN_PUPDR_FLOATING(GPIOH_PIN7) |      \
                                     PIN_PUPDR_FLOATING(GPIOH_PIN8) |      \
                                     PIN_PUPDR_FLOATING(GPIOH_PIN9) |      \
                                     PIN_PUPDR_FLOATING(GPIOH_PIN10) |     \
                                     PIN_PUPDR_FLOATING(GPIOH_PIN11) |     \
                                     PIN_PUPDR_FLOATING(GPIOH_PIN12) |     \
                                     PIN_PUPDR_FLOATING(GPIOH_PIN13) |     \
                                     PIN_PUPDR_FLOATING(GPIOH_PIN14) |     \
                                     PIN_PUPDR_FLOATING(GPIOH_PIN15))
#define VAL_GPIOH_ODR               (PIN_ODR_LOW(GPIOH_PIN0) |            \
                                     PIN_ODR_LOW(GPIOH_PIN1) |            \
                                     FMC_PIN_ODR(GPIOH_FMC_SDCKE0) |        \
                                     FMC_PIN_ODR(GPIOH_FMC_SDNE0) |         \
                                     PIN_ODR_LOW(GPIOH_PIN4) |             \
                                     FMC_PIN_ODR(GPIOH_FMC_SDNWE) |         \
                                     PIN_ODR_LOW(GPIOH_PIN6) |             \
                                     PIN_ODR_HIGH(GPIOH_PIN7) |            \
                                     PIN_ODR_LOW(GPIOH_PIN8) |             \
                                     PIN_ODR_LOW(GPIOH_PIN9) |             \
                                     PIN_ODR_LOW(GPIOH_PIN10) |            \
                                     PIN_ODR_LOW(GPIOH_PIN11) |            \
                                     PIN_ODR_LOW(GPIOH_PIN12) |            \
                                     PIN_ODR_LOW(GPIOH_PIN13) |            \
                                     PIN_ODR_LOW(GPIOH_PIN14) |            \
                                     PIN_ODR_LOW(GPIOH_PIN15))
#define VAL_GPIOH_AFRL              (PIN_AFIO_AF(GPIOH_PIN0, 0U) |         \
                                     PIN_AFIO_AF(GPIOH_PIN1, 0U) |         \
                                     FMC_PIN_AF(GPIOH_FMC_SDCKE0) |        \
                                     FMC_PIN_AF(GPIOH_FMC_SDNE0) |         \
                                     PIN_AFIO_AF(GPIOH_PIN4, 0U) |         \
                                     FMC_PIN_AF(GPIOH_FMC_SDNWE) |         \
                                     PIN_AFIO_AF(GPIOH_PIN6, 0U) |         \
                                     PIN_AFIO_AF(GPIOH_PIN7, 0U))
#define VAL_GPIOH_AFRH              (PIN_AFIO_AF(GPIOH_PIN8, 0U) |         \
                                     PIN_AFIO_AF(GPIOH_PIN9, 0U) |         \
                                     PIN_AFIO_AF(GPIOH_PIN10, 0U) |        \
                                     PIN_AFIO_AF(GPIOH_PIN11, 0U) |        \
                                     PIN_AFIO_AF(GPIOH_PIN12, 0U) |        \
                                     PIN_AFIO_AF(GPIOH_PIN13, 0U) |        \
                                     PIN_AFIO_AF(GPIOH_PIN14, 0U) |        \
                                     PIN_AFIO_AF(GPIOH_PIN15, 0U))

/*
 * GPIOI setup:
 *
 * PI0  - PIN0                      (analog).
 * PI1  - SPI2_SCK                  (alternate 5).
 * PI2  - PIN2                      (analog).
 * PI3  - SPI2_MOSI                 (alternate 5).
 * PI4  - SAI2_MCLK_A               (alternate 10).
 * PI5  - SAI2_SCK_A                (alternate 10).
 * PI6  - SAI2_SD_A                 (alternate 10).
 * PI7  - SAI2_FS_A                 (alternate 10).
 * PI8  - MXP_S1                    (output pushpull).
 * PI9  - ENC4_B                    (input).
 * PI10 - PIN10                     (analog).
 * PI11 - MXP_S0                    (output pushpull).
 * PI12 - PIN12                     (analog).
 * PI13 - PIN13                     (analog).
 * PI14 - PIN14                     (analog).
 * PI15 - PIN15                     (analog).
 */
#define VAL_GPIOI_MODER             (PIN_MODE_ANALOG(GPIOI_PIN0) |         \
                                     PIN_MODE_ALTERNATE(GPIOI_SPI2_SCK) |  \
                                     PIN_MODE_ANALOG(GPIOI_PIN2) |         \
                                     PIN_MODE_ALTERNATE(GPIOI_SPI2_MOSI) | \
                                     PIN_MODE_ALTERNATE(GPIOI_SAI2_MCLK_A) |\
                                     PIN_MODE_ALTERNATE(GPIOI_SAI2_SCK_A) |\
                                     PIN_MODE_ALTERNATE(GPIOI_SAI2_SD_A) | \
                                     PIN_MODE_ALTERNATE(GPIOI_SAI2_FS_A) | \
                                     PIN_MODE_OUTPUT(GPIOI_MXP_S1) |        \
                                     PIN_MODE_INPUT(GPIOI_ENC4_B) |        \
                                     PIN_MODE_ANALOG(GPIOI_PIN10) |        \
                                     PIN_MODE_OUTPUT(GPIOI_MXP_S0) |        \
                                     PIN_MODE_ANALOG(GPIOI_PIN12) |        \
                                     PIN_MODE_ANALOG(GPIOI_PIN13) |        \
                                     PIN_MODE_ANALOG(GPIOI_PIN14) |        \
                                     PIN_MODE_ANALOG(GPIOI_PIN15))
#define VAL_GPIOI_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOI_PIN0) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOI_SPI2_SCK) |  \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN2) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOI_SPI2_MOSI) | \
                                     PIN_OTYPE_PUSHPULL(GPIOI_SAI2_MCLK_A) |\
                                     PIN_OTYPE_PUSHPULL(GPIOI_SAI2_SCK_A) |\
                                     PIN_OTYPE_PUSHPULL(GPIOI_SAI2_SD_A) | \
                                     PIN_OTYPE_PUSHPULL(GPIOI_SAI2_FS_A) | \
                                     PIN_OTYPE_PUSHPULL(GPIOI_MXP_S1) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOI_ENC4_B) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN10) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOI_MXP_S0) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN12) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN13) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN14) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN15))
#define VAL_GPIOI_OSPEEDR           (PIN_OSPEED_VERYLOW(GPIOI_PIN0) |      \
                                     PIN_OSPEED_HIGH(GPIOI_SPI2_SCK) |     \
                                     PIN_OSPEED_VERYLOW(GPIOI_PIN2) |      \
                                     PIN_OSPEED_HIGH(GPIOI_SPI2_MOSI) |    \
                                     PIN_OSPEED_HIGH(GPIOI_SAI2_MCLK_A) |  \
                                     PIN_OSPEED_HIGH(GPIOI_SAI2_SCK_A) |   \
                                     PIN_OSPEED_HIGH(GPIOI_SAI2_SD_A) |    \
                                     PIN_OSPEED_HIGH(GPIOI_SAI2_FS_A) |    \
                                     PIN_OSPEED_LOW(GPIOI_MXP_S1) |         \
                                     PIN_OSPEED_LOW(GPIOI_ENC4_B) |         \
                                     PIN_OSPEED_VERYLOW(GPIOI_PIN10) |     \
                                     PIN_OSPEED_LOW(GPIOI_MXP_S0) |         \
                                     PIN_OSPEED_VERYLOW(GPIOI_PIN12) |     \
                                     PIN_OSPEED_VERYLOW(GPIOI_PIN13) |     \
                                     PIN_OSPEED_VERYLOW(GPIOI_PIN14) |     \
                                     PIN_OSPEED_VERYLOW(GPIOI_PIN15))
#define VAL_GPIOI_PUPDR             (PIN_PUPDR_FLOATING(GPIOI_PIN0) |      \
                                     PIN_PUPDR_FLOATING(GPIOI_SPI2_SCK) |  \
                                     PIN_PUPDR_FLOATING(GPIOI_PIN2) |      \
                                     PIN_PUPDR_FLOATING(GPIOI_SPI2_MOSI) | \
                                     PIN_PUPDR_FLOATING(GPIOI_SAI2_MCLK_A) |\
                                     PIN_PUPDR_FLOATING(GPIOI_SAI2_SCK_A) |\
                                     PIN_PUPDR_FLOATING(GPIOI_SAI2_SD_A) | \
                                     PIN_PUPDR_FLOATING(GPIOI_SAI2_FS_A) | \
                                     PIN_PUPDR_FLOATING(GPIOI_MXP_S1) |    \
                                     PIN_PUPDR_FLOATING(GPIOI_ENC4_B) |    \
                                     PIN_PUPDR_FLOATING(GPIOI_PIN10) |     \
                                     PIN_PUPDR_FLOATING(GPIOI_MXP_S0) |    \
                                     PIN_PUPDR_FLOATING(GPIOI_PIN12) |     \
                                     PIN_PUPDR_FLOATING(GPIOI_PIN13) |     \
                                     PIN_PUPDR_FLOATING(GPIOI_PIN14) |     \
                                     PIN_PUPDR_FLOATING(GPIOI_PIN15))
#define VAL_GPIOI_ODR               (PIN_ODR_LOW(GPIOI_PIN0) |            \
                                     PIN_ODR_LOW(GPIOI_SPI2_SCK) |         \
                                     PIN_ODR_LOW(GPIOI_PIN2) |             \
                                     PIN_ODR_LOW(GPIOI_SPI2_MOSI) |        \
                                     PIN_ODR_LOW(GPIOI_SAI2_MCLK_A) |      \
                                     PIN_ODR_LOW(GPIOI_SAI2_SCK_A) |       \
                                     PIN_ODR_LOW(GPIOI_SAI2_SD_A) |        \
                                     PIN_ODR_LOW(GPIOI_SAI2_FS_A) |        \
                                     PIN_ODR_LOW(GPIOI_MXP_S1) |           \
                                     PIN_ODR_LOW(GPIOI_ENC4_B) |           \
                                     PIN_ODR_LOW(GPIOI_PIN10) |            \
                                     PIN_ODR_LOW(GPIOI_MXP_S0) |           \
                                     PIN_ODR_LOW(GPIOI_PIN12) |            \
                                     PIN_ODR_LOW(GPIOI_PIN13) |            \
                                     PIN_ODR_LOW(GPIOI_PIN14) |            \
                                     PIN_ODR_LOW(GPIOI_PIN15))
#define VAL_GPIOI_AFRL              (PIN_AFIO_AF(GPIOI_PIN0, 0U) |         \
                                     PIN_AFIO_AF(GPIOI_SPI2_SCK, 5U) |     \
                                     PIN_AFIO_AF(GPIOI_PIN2, 0U) |         \
                                     PIN_AFIO_AF(GPIOI_SPI2_MOSI, 5U) |    \
                                     PIN_AFIO_AF(GPIOI_SAI2_MCLK_A, 10U) | \
                                     PIN_AFIO_AF(GPIOI_SAI2_SCK_A, 10U) |  \
                                     PIN_AFIO_AF(GPIOI_SAI2_SD_A, 10U) |   \
                                     PIN_AFIO_AF(GPIOI_SAI2_FS_A, 10U))
#define VAL_GPIOI_AFRH              (PIN_AFIO_AF(GPIOI_MXP_S1, 0U) |       \
                                     PIN_AFIO_AF(GPIOI_ENC4_B, 0U) |       \
                                     PIN_AFIO_AF(GPIOI_PIN10, 0U) |        \
                                     PIN_AFIO_AF(GPIOI_MXP_S0, 0U) |       \
                                     PIN_AFIO_AF(GPIOI_PIN12, 0U) |        \
                                     PIN_AFIO_AF(GPIOI_PIN13, 0U) |        \
                                     PIN_AFIO_AF(GPIOI_PIN14, 0U) |        \
                                     PIN_AFIO_AF(GPIOI_PIN15, 0U))

/*
 * GPIOJ setup:
 *
 * All pins analog.
 */
#define VAL_GPIOJ_MODER             0xFFFFFFFFU
#define VAL_GPIOJ_OTYPER            0x00000000U
#define VAL_GPIOJ_OSPEEDR           0x00000000U
#define VAL_GPIOJ_PUPDR             0x00000000U
#define VAL_GPIOJ_ODR               0x00000000U
#define VAL_GPIOJ_AFRL              0x00000000U
#define VAL_GPIOJ_AFRH              0x00000000U

/*
 * GPIOK setup:
 *
 * All pins analog.
 */
#define VAL_GPIOK_MODER             0xFFFFFFFFU
#define VAL_GPIOK_OTYPER            0x00000000U
#define VAL_GPIOK_OSPEEDR           0x00000000U
#define VAL_GPIOK_PUPDR             0x00000000U
#define VAL_GPIOK_ODR               0x00000000U
#define VAL_GPIOK_AFRL              0x00000000U
#define VAL_GPIOK_AFRH              0x00000000U

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if !defined(_FROM_ASM_)
#ifdef __cplusplus
extern "C" {
#endif
  void boardInit(void);
#ifdef __cplusplus
}
#endif
#endif /* _FROM_ASM_ */

#endif /* BOARD_H */
