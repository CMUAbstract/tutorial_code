OBJECTS ?= fft.o
EXEC = tutorial

DEPS = libmspbuiltins:gcc libio:gcc libfixed:gcc \
        libcapybara:gcc libmspware:gcc libfxl:gcc \
        libmsp:gcc libpacarana:gcc libmat:gcc liblsm:gcc

INPUT_SIZE ?= 1
BLOCK_SIZE ?= 64

LOCAL_CFLAGS += -DBLOCK_SIZE=$(BLOCK_SIZE) \
                -DINPUT_SIZE=$(INPUT_SIZE)


ifneq ($(WORKLOAD_CYCLES),)
LOCAL_CFLAGS += -DWORKLOAD_CYCLES=$(WORKLOAD_CYCLES)
endif

# Defines the HPVLP (high power vs low power) flag that will run the workload in
# an actual test. Without it we get print statements
ifeq ($(HPVLP),1)
LOCAL_CFLAGS += -DHPVLP
endif

# Defines the REENABLE flag which will trigger the reenable/disable functions
ifeq ($(REENABLE),1)
LOCAL_CFLAGS += -DREENABLE
endif

ifneq ($(RATE),)
LOCAL_CFLAGS += -DRATE=$(RATE)
endif

CFLAGS += LOCAL_CFLAGS
export CFLAGS
include tools/maker/Makefile
