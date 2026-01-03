/**
  ******************************************************************************
  * @file    stm32h7xx_hal_conf.h
  * @brief   Minimal HAL configuration for STM32H743 + ChibiOS.
  ******************************************************************************
  * This project uses the STM32H7 HAL only for SAI. System init, clocks,
  * SysTick, NVIC, USB/UART/I2C/SPI are handled by ChibiOS.
  ******************************************************************************
  */

#ifndef STM32H7xx_HAL_CONF_H
#define STM32H7xx_HAL_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/* ########################## Module Selection ############################## */
#define HAL_MODULE_ENABLED
#define HAL_GPIO_MODULE_ENABLED
#define HAL_RCC_MODULE_ENABLED
#define HAL_SAI_MODULE_ENABLED
#define HAL_DMA_MODULE_ENABLED
/* ########################## Oscillator Values ############################# */
#if !defined(HSE_VALUE)
#define HSE_VALUE (25000000UL)
#endif

#if !defined(HSE_STARTUP_TIMEOUT)
#define HSE_STARTUP_TIMEOUT (100UL)
#endif

#if !defined(CSI_VALUE)
#define CSI_VALUE (4000000UL)
#endif

#if !defined(HSI_VALUE)
#define HSI_VALUE (64000000UL)
#endif

#if !defined(LSE_VALUE)
#define LSE_VALUE (32768UL)
#endif

#if !defined(LSE_STARTUP_TIMEOUT)
#define LSE_STARTUP_TIMEOUT (5000UL)
#endif

#if !defined(LSI_VALUE)
#define LSI_VALUE (32000UL)
#endif

#if !defined(EXTERNAL_CLOCK_VALUE)
#define EXTERNAL_CLOCK_VALUE (12288000UL)
#endif

/* ########################### System Configuration ######################### */
#define VDD_VALUE         (3300UL)
#define TICK_INT_PRIORITY (0x0FUL)
#define USE_RTOS          0
#define USE_FULL_ASSERT   0U

#define USE_HAL_SAI_REGISTER_CALLBACKS 0U

/* ########################### assert_param ################################# */
#if (USE_FULL_ASSERT == 1U)
void assert_failed(uint8_t *file, uint32_t line);
#define assert_param(expr) ((expr) ? (void)0U : assert_failed((uint8_t *)__FILE__, __LINE__))
#else
#define assert_param(expr) ((void)0U)
#endif

#ifdef __cplusplus
}
#endif

#endif /* STM32H7xx_HAL_CONF_H */
