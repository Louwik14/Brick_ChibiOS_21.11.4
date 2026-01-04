/*
 * Minimal STM32H7 HAL core stubs for ChibiOS integration.
 *
 * This keeps HAL timebase calls functional without touching SysTick,
 * clock tree, or NVIC configuration owned by ChibiOS.
 */

#include "ch.h"
#include "osal.h"
#include "halconf.h"
#include "mcuconf.h"
#include "board.h"
#include "hal_lld.h"
#include "stm32h7xx_hal.h"

#define __STM32H7xx_HAL_VERSION_MAIN   (0x01UL)
#define __STM32H7xx_HAL_VERSION_SUB1   (0x0BUL)
#define __STM32H7xx_HAL_VERSION_SUB2   (0x05UL)
#define __STM32H7xx_HAL_VERSION_RC     (0x00UL)
#define __STM32H7xx_HAL_VERSION        ((__STM32H7xx_HAL_VERSION_MAIN << 24)  \
                                        |(__STM32H7xx_HAL_VERSION_SUB1 << 16) \
                                        |(__STM32H7xx_HAL_VERSION_SUB2 << 8)  \
                                        |(__STM32H7xx_HAL_VERSION_RC))

__IO uint32_t uwTick = 0U;
uint32_t uwTickPrio = 0U;
HAL_TickFreqTypeDef uwTickFreq = HAL_TICK_FREQ_DEFAULT;

HAL_StatusTypeDef HAL_Init(void) {
  return HAL_OK;
}

HAL_StatusTypeDef HAL_DeInit(void) {
  return HAL_OK;
}

HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority) {
  (void)TickPriority;
  return HAL_OK;
}

void HAL_IncTick(void) {
  /* ChibiOS owns the system tick; keep this as a no-op. */
}

uint32_t HAL_GetTick(void) {
  uwTick = TIME_I2MS(chVTGetSystemTimeX());
  return uwTick;
}

void HAL_Delay(uint32_t Delay) {
  if (Delay == 0U) {
    return;
  }
  chThdSleepMilliseconds(Delay);
}

HAL_StatusTypeDef HAL_SetTickFreq(HAL_TickFreqTypeDef Freq) {
  uwTickFreq = Freq;
  return HAL_OK;
}

HAL_TickFreqTypeDef HAL_GetTickFreq(void) {
  return uwTickFreq;
}

uint32_t HAL_GetTickPrio(void) {
  return uwTickPrio;
}

uint32_t HAL_GetHalVersion(void) {
  return __STM32H7xx_HAL_VERSION;
}

uint32_t HAL_GetREVID(void) {
  return ((DBGMCU->IDCODE) >> 16U);
}

uint32_t HAL_GetDEVID(void) {
  return (DBGMCU->IDCODE & 0x00000FFFU);
}

uint32_t HAL_GetUIDw0(void) {
  return READ_REG(*((uint32_t *)UID_BASE));
}

uint32_t HAL_GetUIDw1(void) {
  return READ_REG(*((uint32_t *)(UID_BASE + 4U)));
}

uint32_t HAL_GetUIDw2(void) {
  return READ_REG(*((uint32_t *)(UID_BASE + 8U)));
}

uint32_t HAL_RCCEx_GetPeriphCLKFreq(uint64_t PeriphClk) {
  switch (PeriphClk) {
    case RCC_PERIPHCLK_SAI1:
      return STM32_SAI1CLK;
#if defined(RCC_PERIPHCLK_SAI2)
    case RCC_PERIPHCLK_SAI2:
      return STM32_SAI2CLK;
#endif
#if defined(RCC_PERIPHCLK_SAI2A)
    case RCC_PERIPHCLK_SAI2A:
      return STM32_SAI2ACLK;
#endif
#if defined(RCC_PERIPHCLK_SAI2B)
    case RCC_PERIPHCLK_SAI2B:
      return STM32_SAI2BCLK;
#endif
#if defined(RCC_PERIPHCLK_SAI3) && !defined(RCC_PERIPHCLK_SAI2)
    case RCC_PERIPHCLK_SAI3:
      return STM32_SAI3CLK;
#endif
#if defined(RCC_PERIPHCLK_SAI4A)
    case RCC_PERIPHCLK_SAI4A:
      return STM32_SAI4ACLK;
#endif
#if defined(RCC_PERIPHCLK_SAI4B)
    case RCC_PERIPHCLK_SAI4B:
      return STM32_SAI4BCLK;
#endif
    default:
      return 0U;
  }
}
