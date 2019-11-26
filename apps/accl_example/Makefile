#OBJECTS = main_crunch_current_profile.o
OBJECTS = main_workload.o
EXEC = tutorial

DEPS = libmspbuiltins:gcc libio:gcc libfixed:gcc \
        libradio:gcc libcapybara:gcc liblsm:gcc \
        libmspuartlink:gcc libmspware:gcc \
        libapds:gcc libfxl:gcc libmsp:gcc \
        libfxdmath:gcc liblps:gcc libpacarana:gcc

# Defines the PROF flag which kicks us into the initialization profile task
ifeq ($(PROF),1)
LOCAL_CFLAGS += -DPROF
endif

# Defines the HPVLP (high power vs low power) flag that will run the workload
# task which we'll eventulaly use to compare the high power mode and low power
# mode energy consumption.
ifeq ($(HPVLP),1)
LOCAL_CFLAGS += -DHPVLP
endif

# Defines the REENABLE flag which will trigger the reenable/disable functions
ifeq ($(REENABLE),1)
LOCAL_CFLAGS += -DREENABLE
endif

ifeq ($(ACCEL),1)
LOCAL_CFLAGS += -DACCEL
endif


ifeq ($(ITER_INC),1)
LOCAL_CFLAGS += -DITER_INC
endif

ifneq ($(RATE),)
LOCAL_CFLAGS += -DRATE=$(RATE)
endif

ifneq ($(ITER_START),)
LOCAL_CFLAGS += -DITER_START=$(ITER_START)
endif

ifneq ($(ITER_STEP),)
LOCAL_CFLAGS += -DITER_STEP=$(ITER_STEP)
endif

ifneq ($(ITER_END),)
LOCAL_CFLAGS += -DITER_END=$(ITER_END)
endif


CFLAGS += LOCAL_CFLAGS
export CFLAGS
include tools/maker/Makefile
