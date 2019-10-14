TOOLS_REL_ROOT = tools
TOOLS = alpaca
TOOLCHAINS = gcc clang alpaca

APPS = pacarana_test gyro_example periph_test_dir checkpt_test cntrl_flow \
accl_example apds_example

export BOARD = capybara
export BOARD_MAJOR = 2
export BOARD_MINOR = 0
export DEVICE = msp430fr5949

SHARED_DEPS = libmspbuiltins:gcc libio:gcc libfixed:gcc libmspuartlink:gcc \
        libapds:gcc libcapybara:gcc libfxl:gcc libmspware:gcc libmsp:gcc \
        libradio:gcc libfxdmath:gcc

export MAIN_CLOCK_FREQ = 8000000

export CLOCK_FREQ_ACLK = 32768
export CLOCK_FREQ_SMCLK = $(MAIN_CLOCK_FREQ)
export CLOCK_FREQ_MCLK = $(MAIN_CLOCK_FREQ)

export LIBMSP_CLOCK_SOURCE = DCO
export LIBMSP_DCO_FREQ = $(MAIN_CLOCK_FREQ)
export LIBMSP_SLEEP_TIMER = B.0.0
export LIBMSP_SLEEP_TIMER_CLK = ACLK
export LIBMSP_SLEEP_TIMER_DIV = 8*1

export LIBMSPUARTLINK_UART_IDX = 1
export LIBMSPUARTLINK_PIN_TX = 2.5
export LIBMSPUARTLINK_BAUDRATE = 115200
export LIBMSPUARTLINK_CLK = SMCLK

ifeq ($(BOARD_MAJOR).$(BOARD_MINOR),1.0)
export LIBCAPYBARA_PIN_VBOOST_OK = 2.3
else ifeq ($(BOARD_MAJOR).$(BOARD_MINOR),1.1)
export LIBCAPYBARA_PIN_VBOOST_OK = 3.4
else ifeq ($(BOARD_MAJOR).$(BOARD_MINOR),2.0)
  export LIBCAPYBARA_PIN_VBOOST_OK = 3.6
endif # BOARD_MAJOR.BOARD_MINOR

export LIBCAPYBARA_CONT_POWER ?= 1
export LIBCAPYBARA_VBANK_COMP_REF = 1.2 # V
export LIBCAPYBARA_VBANK_COMP_SETTLE_MS = 2
export LIBCAPYBARA_DEEP_DISCHARGE = 1.7 # V
export LIBCAPYBARA_PIN_VB1_DIV = 1.1

ifeq ($(BOARD_MAJOR),1)
export LIBCAPYBARA_PIN_BOOST_SW = 2.7
export LIBCAPYBARA_PIN_VBANK_OK = 2.2
export LIBCAPYBARA_VBANK_DIV = 10:20
export LIBCAPYBARA_VBANK_COMP_CHAN = E.11 # comparator type and channel for Vbank voltage
export LIBCAPYBARA_VBANK_COMP_PIN = 2.4


export LIBCAPYBARA_NUM_BANKS = 4

export LIBCAPYBARA_SWITCH_DESIGN = NO
export LIBCAPYBARA_SWITCH_CONTROL = ONE_PIN
export LIBCAPYBARA_BANK_PORT_0 = J.0
export LIBCAPYBARA_BANK_PORT_1 = J.2
export LIBCAPYBARA_BANK_PORT_2 = J.4
export LIBCAPYBARA_BANK_PORT_3 = 4.0

else ifeq ($(BOARD_MAJOR),2)

override LIBCAPYBARA_SWITCH_DESIGN = NC
override LIBCAPYBARA_SWITCH_CONTROL = ONE_PIN
override LIBCAPYBARA_PIN_VBOOST_OK = 3.6
override LIBCAPYBARA_PIN_VBANK_OK = 3.7
override LIBCAPYBARA_PIN_BOOST_SW = 4.5
override LIBCAPYBARA_VBANK_DIV = 10:4.22
override LIBCAPYBARA_VBANK_COMP_CHAN = E.13 # comparator type and channel for Vbank voltage
override LIBCAPYBARA_VBANK_COMP_PIN = 3.1
export LIBCAPYBARA_SWITCH_DESIGN = NC
export LIBCAPYBARA_SWITCH_CONTROL = ONE_PIN
export LIBCAPYBARA_PIN_VBOOST_OK = 3.6
export LIBCAPYBARA_PIN_VBANK_OK = 3.7
export LIBCAPYBARA_PIN_BOOST_SW = 4.5
export LIBCAPYBARA_VBANK_DIV = 10:4.22
export LIBCAPYBARA_VBANK_COMP_CHAN = E.13 # comparator type and channel for Vbank voltage
export LIBCAPYBARA_VBANK_COMP_PIN = 3.1
export LIBCAPYBARA_NUM_BANKS = 4

ifeq ($(LIBCAPYBARA_SWITCH_CONTROL),ONE_PIN)
override LIBCAPYBARA_BANK_PORT_0 = J.0
override LIBCAPYBARA_BANK_PORT_1 = J.1
override LIBCAPYBARA_BANK_PORT_2 = J.2
override LIBCAPYBARA_BANK_PORT_3 = J.3
export LIBCAPYBARA_BANK_PORT_0 = J.0
export LIBCAPYBARA_BANK_PORT_1 = J.1
export LIBCAPYBARA_BANK_PORT_2 = J.2
export LIBCAPYBARA_BANK_PORT_3 = J.3
else
$(error Given switch control type not supported on given board version)
endif # LIBCAPYBARA_SWITCH_CONTROL

endif # BOARD_MAJOR

export LIBCAPYBARA_PRECHG_HANDLING = IMP
export LIBCAPYBARA_FXL_OFF = 0

export RADIO_CAPTURE = 0

export VOLTAGE = 2400

CONSOLE ?= 0

ifneq ($(CONSOLE),)
export VERBOSE = 2
export LIBMSP_SLEEP = 1
export LIBIO_BACKEND = hwuart
export LIBMSP_UART_IDX = 0
export LIBMSP_UART_PIN_TX = 2.0
export LIBMSP_UART_BAUDRATE = 115200
export LIBMSP_UART_CLOCK = SMCLK
export LIBMAT_CONSOLE = 1
override CFLAGS += -DCONSOLE=0
endif


PACARANA ?= 1

ifneq ($(PACARANA),)
override CFLAGS += -DPACARANA
include tools/pacarana/Makefile
endif

SHARED_DEPS += libpacarana:gcc

export CFLAGS
include tools/maker/Makefile
