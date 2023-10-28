# *****************************************************************************
# NEORV32 FreeRTOS Makefile
# *****************************************************************************

# FreeRTOS home folder
FREERTOS_HOME ?= ../FreeRTOS-Kernel

# NEORV32 home folder
NEORV32_HOME ?= ../neorv32


# User's application sources (*.c, *.cpp, *.s, *.S); add additional files here
APP_SRC = $(wildcard ./*.c) $(wildcard ./*.s) $(wildcard ./*.cpp) $(wildcard ./*.S)

# User's application include folders (don't forget the '-I' before each entry)
APP_INC = -I .

# User's application include folders - for assembly files only (don't forget the '-I' before each entry)
ASM_INC = -I .

# User flags for additional configuration (will be added to compiler flags)
USER_FLAGS =

# -----------------------------------------------------------------------------
# FreeRTOS
# -----------------------------------------------------------------------------

# Kernel
APP_SRC += $(wildcard $(FREERTOS_HOME)/*.c)
APP_INC += -I $(FREERTOS_HOME)/include

# RISC-V specifics
APP_SRC += $(wildcard  $(FREERTOS_HOME)/portable/GCC/RISC-V/*.c)
APP_SRC +=  $(FREERTOS_HOME)/portable/GCC/RISC-V/portASM.S
APP_INC += -I  $(FREERTOS_HOME)/portable/GCC/RISC-V

# Heap management
APP_SRC += $(wildcard  $(FREERTOS_HOME)/portable/MemMang/heap_4.c)

# -----------------------------------------------------------------------------
# NEORV32
# -----------------------------------------------------------------------------

# Set maximum heap size
override USER_FLAGS += "-Wl,--defsym,__neorv32_heap_size=3500"

# Chip-specific configuration
APP_INC += -I chip_specific_extensions/neorv32
ASM_INC += -I chip_specific_extensions/neorv32

# Software framework, HAL, build environment, etc.
include $(NEORV32_HOME)/sw/common/common.mk
