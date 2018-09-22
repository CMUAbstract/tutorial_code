TOOLS = alpaca
TOOLCHAINS = gcc clang alpaca

export BOARD = launchpad
export DEVICE = msp430fr5994

EXEC = tutorial

OBJECTS = main.o

DEPS += libmspbuiltins:gcc libio:gcc libmsp:gcc libfixed:gcc libmat:gcc \
        libmspware:gcc libfxl:gcc libmspuartlink:gcc

export MAIN_CLOCK_FREQ = 16000000

export CLOCK_FREQ_ACLK = 32768
export CLOCK_FREQ_SMCLK = $(MAIN_CLOCK_FREQ)
export CLOCK_FREQ_MCLK = $(MAIN_CLOCK_FREQ)

export LIBMSP_CLOCK_SOURCE = DCO
export LIBMSP_DCO_FREQ = $(MAIN_CLOCK_FREQ)

export LIBMSPUARTLINK_UART_IDX = 1
export LIBMSPUARTLINK_PIN_TX = 2.5
export LIBMSPUARTLINK_BAUDRATE = 115200
export LIBMSPUARTLINK_CLK = SMCLK

CONSOLE ?= 1

ifneq ($(CONSOLE),)
export VERBOSE = 1
export LIBMSP_SLEEP = 1
export LIBIO_BACKEND = hwuart
export LIBMSP_UART_IDX = 0
export LIBMSP_UART_PIN_TX = 2.0
export LIBMSP_UART_BAUDRATE = 115200
export LIBMSP_UART_CLOCK = SMCLK
override CFLAGS += -DCONSOLE=1
endif

export CFLAGS
include tools/maker/Makefile
