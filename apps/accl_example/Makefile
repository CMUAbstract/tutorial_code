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

ifneq ($(WORKLOAD_CYCLES),)
LOCAL_CFLAGS += -DWORKLOAD_CYCLES=$(WORKLOAD_CYCLES)
endif

ifeq ($(DISABLE_ALL),1)
LOCAL_CFLAGS += -DDISABLE_ALL
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

# Use the PROF_COMPUTE flag to test the power consumption while the device
# performs adds and writes to non-volatile memory
ifneq ($(PROF_COMPUTE),)
LOCAL_CFLAGS += -DPROF_COMPUTE
endif

ifneq ($(TEST_NO_LIM),)
LOCAL_CFLAGS += -DTEST_NO_LIM
endif

ifneq ($(ONLYGYRO),)
LOCAL_CFLAGS += -DONLYGYRO
endif

CFLAGS += LOCAL_CFLAGS
export CFLAGS
include tools/maker/Makefile
