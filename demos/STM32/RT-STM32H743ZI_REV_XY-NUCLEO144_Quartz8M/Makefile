##############################################################################
# Build global options
# NOTE: Can be overridden externally.
#

ifeq ($(USE_OPT),)
  USE_OPT = -O0 -ggdb
endif


ifeq ($(USE_COPT),)
  USE_COPT =
endif

ifeq ($(USE_CPPOPT),)
  USE_CPPOPT = -fno-rtti
endif

ifeq ($(USE_LINK_GC),)
  USE_LINK_GC = yes
endif

ifeq ($(USE_LDOPT),)
  USE_LDOPT =
endif

ifeq ($(USE_LTO),)
  USE_LTO = no
endif

ifeq ($(USE_VERBOSE_COMPILE),)
  USE_VERBOSE_COMPILE = no
endif

ifeq ($(USE_SMART_BUILD),)
  USE_SMART_BUILD = yes
endif

##############################################################################
# Architecture or project specific options
#

ifeq ($(USE_PROCESS_STACKSIZE),)
  USE_PROCESS_STACKSIZE = 0x400
endif

ifeq ($(USE_EXCEPTIONS_STACKSIZE),)
  USE_EXCEPTIONS_STACKSIZE = 0x400
endif

ifeq ($(USE_FPU),)
  USE_FPU = no
endif

ifeq ($(USE_FPU_OPT),)
  USE_FPU_OPT = -mfloat-abi=$(USE_FPU) -mfpu=fpv5-d16
endif

##############################################################################
# Project, target, sources and paths
#

PROJECT = ch
MCU     = cortex-m7

# Paths
CHIBIOS := ../../..
CHIBIOS_CONTRIB := $(CHIBIOS)/ChibiOS-Contrib-chibios-21.11.x

CONFDIR  := ./cfg
BUILDDIR := ./build
DEPDIR   := ./.dep

##############################################################################
# ChibiOS core
#

include $(CHIBIOS)/os/license/license.mk
include $(CHIBIOS)/os/common/startup/ARMCMx/compilers/GCC/mk/startup_stm32h7xx.mk

include $(CHIBIOS_CONTRIB)/os/hal/hal.mk
include $(CHIBIOS)/os/hal/lib/streams/streams.mk
include $(CHIBIOS_CONTRIB)/os/hal/ports/STM32/STM32H7xx/platform.mk
include $(CHIBIOS)/os/hal/boards/STM32H743_LQFP176_CUSTOM/board.mk
include $(CHIBIOS)/os/hal/osal/rt-nil/osal.mk

include $(CHIBIOS)/os/rt/rt.mk
include $(CHIBIOS)/os/common/ports/ARMv7-M/compilers/GCC/mk/port.mk

include $(CHIBIOS)/tools/mk/autobuild.mk

include $(CHIBIOS)/os/test/test.mk
include $(CHIBIOS)/test/rt/rt_test.mk
include $(CHIBIOS)/test/oslib/oslib_test.mk

##############################################################################
# Linker script
#

LDSCRIPT = $(STARTUPLD)/STM32H743xI.ld

##############################################################################
# Sources
#

CSRC = $(ALLCSRC) \
       $(TESTSRC) \
       $(CHIBIOS)/os/various/syscalls.c \
       main.c \
       $(wildcard drivers/*.c)\
       $(wildcard midi/*.c)\
       $(wildcard mpu/*.c)\
       $(wildcard usb/*.c)\
       $(wildcard ui/*.c)\
       $(wildcard sdram/*.c) \
       $(wildcard audio/*.c) \
       drivers/stm32h7xx-hal-driver/Src/stm32h7xx_hal_dma.c \
       drivers/stm32h7xx-hal-driver/Src/stm32h7xx_hal_gpio.c \
       drivers/stm32h7xx-hal-driver/Src/stm32h7xx_hal_sai.c \
       drivers/stm32h7xx-hal-driver/Src/stm32h7xx_hal_sai_ex.c
       
CPPSRC = $(ALLCPPSRC)
ASMSRC = $(ALLASMSRC)
ASMXSRC = $(ALLXASMSRC)

##############################################################################
# Includes
#

INCDIR = $(CONFDIR) $(ALLINC) $(TESTINC)
INCDIR += drivers
INCDIR += midi
INCDIR += sdram
INCDIR += mpu
INCDIR += usb
INCDIR += ui
INCDIR += audio
INCDIR += drivers/stm32h7xx-hal-driver/Inc
INCDIR += drivers/stm32h7xx-hal-driver/Inc/Legacy


##############################################################################
# Warnings
#

CWARN   = -Wall -Wextra -Wundef -Wstrict-prototypes
CPPWARN = -Wall -Wextra -Wundef

##############################################################################
# User section
#

UDEFS  = -DSTM32_ENFORCE_H7_REV_XY -DSTM32H743xx -DSTM32_HSECLK=8000000U
UADEFS =

UINCDIR += $(CHIBIOS)/os/various

ULIBDIR =
ULIBS   =

##############################################################################
# Rules
#

RULESPATH = $(CHIBIOS)/os/common/startup/ARMCMx/compilers/GCC/mk
include $(RULESPATH)/arm-none-eabi.mk
include $(RULESPATH)/rules.mk
