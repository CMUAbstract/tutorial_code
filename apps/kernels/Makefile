OBJECTS ?= sort.o
EXEC = tutorial

DEPS = libmspbuiltins:gcc libio:gcc libfixed:gcc \
        libcapybara:gcc libmspware:gcc libfxl:gcc \
        libmsp:gcc libpacarana:gcc libmat:gcc

INPUT_SIZE ?= 1
BLOCK_SIZE ?= 64

LOCAL_CFLAGS += -DBLOCK_SIZE=$(BLOCK_SIZE) \
                -DINPUT_SIZE=$(INPUT_SIZE)

CFLAGS += LOCAL_CFLAGS
export CFLAGS
include tools/maker/Makefile
