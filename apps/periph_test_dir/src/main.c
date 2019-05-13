#include <msp430.h>

#include <libmspbuiltins/builtins.h>
#include <libmsp/clock.h>
#include <libmsp/watchdog.h>
#include <libmsp/gpio.h>
#include <libio/console.h>
#include <libcapybara/reconfig.h>
#include <libcapybara/board.h>
#include <libradio/radio.h>
#include <libalpaca/alpaca.h>
#include <liblsm/gyro.h>
#include <libapds/proximity.h>
#include <libhmc/magnetometer.h>

#define SERIES_LEN 23
// Vars that are really internal to the radio code
typedef enum __attribute__((packed)) {
    RADIO_CMD_SUMMARY = 0,
    RADIO_CMD_ABOVE_WARNING = 1,
    RADIO_CMD_BELOW_WARNING = 2,
    RADIO_CMD_CAL_NEEDED = 3,
} radio_cmd_t;

typedef struct __attribute__((packed)) {
    radio_cmd_t cmd;
    uint8_t series[SERIES_LEN];
} radio_pkt_t;

static radio_pkt_t radio_pkt;

TASK(task_init,1)

__nv capybara_task_cfg_t pwr_configs[1] = {
  CFG_ROW(0, CONFIGD, LOWP, LOWP),
};


void hardware_under_test() {
#ifdef GYRO_INT
  // Just testing this config, we don't actually handle the interrupt
  __delay_cycles(160000);
  lsm_reset();
  gyro_init_fifo_tap();
  read_tap_src();
#elif defined(GYRO_LP)
  __delay_cycles(160000);
  lsm_reset();
  gyro_init_data_rate_hm(0x30,0);
  read_tap_src();
#elif defined(GYRO_HP)
  __delay_cycles(160000);
  lsm_reset();
  gyro_init_data_rate_hm(0x70,1);
  read_tap_src();
#elif defined(APDS_HP)
  __delay_cycles(5000);
  fxl_set(BIT_APDS_SW);
  __delay_cycles(5000);
  proximity_init();
  enableGesture();
#elif defined(APDS_LP)
  __delay_cycles(5000);
  fxl_set(BIT_APDS_SW);
  __delay_cycles(5000);
  proximity_init();
  enableProximitySensor();
#elif defined(RADIO_IDLE)
  radio_on();
#elif defined(RADIO_LP)
  radio_on();
  __delay_cycles(48000);
  uartlink_open_tx();
  for (int i = 0; i < LIBRADIO_BUFF_LEN; i++) {
    radio_buff[i] = 0x01;
  }
  uartlink_send(radio_buff, LIBRADIO_BUFF_LEN);
  uartlink_close();
#elif defined(RADIO_HP)
#elif defined(HMC_LP)
  __delay_cycles(48000);
	printf("Starting lp\r\n");
  magnetometer_init_lp();
	printf("finished lp!\r\n");
  //Omitting delay
#elif defined(HMC_HP)
  __delay_cycles(5000);
	printf("Starting hp\r\n");
  magnetometer_init_hp();
	printf("finished hp!\r\n");
  //Omitting delay
#endif
  return;
}

void init() {
  capybara_init();
  fxl_set(BIT_SENSE_SW);
  //TODO figure out how to better manage this delay
  P1OUT |= BIT0;
  P1DIR |= BIT0;
  P1OUT &= ~BIT0;
  // This function is defined above based on what we're testing
  hardware_under_test();
  P1OUT |= BIT0;
  P1DIR |= BIT0;
  P1OUT &= ~BIT0;
  LOG("init\r\n");
  TRANSITION_TO(task_init);
}

#define DELAY_COUNT 100
void task_init() {
  // We'll just trap the execution here
  uint32_t i;
  while(1) {
    P1OUT |= BIT1;
    P1DIR |= BIT1;
    P1OUT &= ~BIT1;
    i = 0;
    while(i < DELAY_COUNT) {
      i++;
    }
  }
}




ENTRY_TASK(task_init)
INIT_FUNC(init)



