# List of all the board related files.
BOARDSRC = $(CHIBIOS)/os/hal/boards/STM32H743_LQFP176_CUSTOM_Q_8M/board.c

# Required include directories
BOARDINC = $(CHIBIOS)/os/hal/boards/STM32H743_LQFP176_CUSTOM_Q_8M

# Shared variables
ALLCSRC += $(BOARDSRC)
ALLINC  += $(BOARDINC)
