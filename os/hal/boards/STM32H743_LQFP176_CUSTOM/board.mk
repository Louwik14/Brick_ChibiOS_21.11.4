# List of all the board related files.
BOARDSRC = $(CHIBIOS)/os/hal/boards/STM32H743_LQFP176_CUSTOM/board.c

# Required include directories
BOARDINC = $(CHIBIOS)/os/hal/boards/STM32H743_LQFP176_CUSTOM

# Shared variables
ALLCSRC += $(BOARDSRC)
ALLINC  += $(BOARDINC)
