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
#include <libpacarana/pacarana.h>
#include <libapds/proximity.h>


#define zero 0
#define one 1
#define two 2
#define three 3
#define four 4

#ifdef PACARANA
REGISTER(radio);
REGISTER(gyro);
REGISTER(photores);
#endif

capybara_task_cfg_t pwr_configs[4] = {
  CFG_ROW(zero, CONFIGD, LOWP2,LOWP2),
  CFG_ROW(one, PREBURST, LOWP2,MEDHIGHP),
  CFG_ROW(two, BURST, MEDHIGHP, MEDHIGHP),
  CFG_ROW(three, CONFIGD, MEDP,MEDP),
};

#define PACA_SIZE 16

typedef struct {
  uint8_t fxl_byte;
  int num_funcs;
  void (*pointertable[PACA_SIZE])(void);
} pacarana_cfg_t;
#define PACA_CFG_ROW(fxlByte,numFuncs,...) \
  {fxlByte,numFuncs,{__VA_ARGS__}}

void print_proof() {
  printf("Reboot!\r\n");
}


pacarana_cfg_t inits[2] = {
  // This goes FXL config then a variable number of functions
  PACA_CFG_ROW(BIT_PHOTO_SW | BIT_SENSE_SW,2, print_proof,enable_photoresistor),
  PACA_CFG_ROW(0,0,NULL)
};


void init();

TASK(task_init);
TASK(task_init1);
TASK(task_init2);
TASK(task_delay);
TASK(task_send);
TASK(task_filter);

ENTRY_TASK(task_init);
INIT_FUNC(init);

void init() {
  capybara_init();
  // Sanity check our number
  if(curctx->pacaCfg > sizeof(inits)/sizeof(pacarana_cfg_t)) {
    printf("Error! invalid pacaCfg number\n");
    while(1);
  }
  // Set up the board-side stuff
  fxl_set(inits[curctx->pacaCfg].fxl_byte);
  // TODO figure out a solution for delay cycles
  //__delay_cycles(160000);
  // Walk through the init functions
  PRINTF("Running init!\r\n");
  for(int i = 0; i < inits[curctx->pacaCfg].num_funcs; i++) {
    inits[curctx->pacaCfg].pointertable[i]();
  }

 /* fxl_set(BIT_SENSE_SW);
  STATE_CHANGE(photores, 0x0);
  __delay_cycles(160000);
  lsm_reset();
  */
}

__nv int test;
__nv uint8_t num_samps;
__nv uint16_t samples[24];

void task_init() {
  PRINTF("Task init\r\n");
  uint16_t proxVal = read_photoresistor();
  PRINTF("level: %u\r\n",proxVal);
  if (proxVal < ALERT_THRESH && proxVal > 10) {
    PACA_TRANSITION_TO(task_init1,0);
  }
  else {
    PACA_TRANSITION_TO(task_init2,0);
  }
}

void task_init1() {
  PRINTF("Task init 1\r\n");
  uint16_t proxVal = read_photoresistor();

  PRINTF("level: %u\r\n",proxVal);
  if (proxVal < ALERT_THRESH && proxVal > 10) {
  }
  else {
    PACA_TRANSITION_TO(task_init2,0);
  }
  // We may have detected an object, set high sampling freq
  gyro_init_data_rate(0x60);
  STATE_CHANGE(gyro, 0x2);
  for( int i = 0; i < 8; i++) {
    read_raw_accel(samples + 3*i + 0,samples + 3*i + 1, samples + 3*i + 2);
  }
  num_samps = 8;
  disable_photoresistor();
#ifdef PACARANA
  STATE_CHANGE(photores, 0x0);
#endif
  lsm_disable();
  STATE_CHANGE(gyro,0x0);
  // Move our bunch of samples to task filter
  PACA_TRANSITION_TO(task_filter,1)
}

void task_init2() {
  PRINTF("Task init 2\r\n");
  uint16_t proxVal = read_photoresistor();
  if (proxVal < ALERT_THRESH && proxVal > 10) {
    PRINTF("level: %u\r\n",20);
    PACA_TRANSITION_TO(task_init1,0)
  }
  // No oject, go with low sampling freq
  gyro_init_data_rate(0x30);
  STATE_CHANGE(gyro, 0x1);
  // Only nab a couple samples
  for( int i = 0; i < 4; i++) {
    read_raw_accel(samples + 3*i + 0,samples + 3*i + 1, samples + 3*i + 2);
  }
  num_samps = 4;
  disable_photoresistor();
  STATE_CHANGE(photores, 0x0);
  lsm_disable();
  STATE_CHANGE(gyro,0x0);
  // Move our bunch of samples to task filter
  PACA_TRANSITION_TO(task_filter,0)
}

void task_filter() {
  float avg[3];
  double sum;
  for (int j = 0; j < 3; j++) {
    sum = 0;
    for (int i = 0; i < num_samps; i++) {
      sum += samples[i*3 + j];
    }
    avg[j] = sum / num_samps;
  }
  // If anomaly condition detected
  if (avg[0] > (avg[1] + avg[2])) {
    PACA_TRANSITION_TO(task_send,0);
  }
  else {
    PACA_TRANSITION_TO(task_delay,0);
  }
}

void task_delay() {
  for (unsigned i = 0; i < 1000; ++i) {
      __delay_cycles(4000);
  }
  PACA_TRANSITION_TO(task_init,0);
}

void task_send() {
  // Reconfigure bank
  capybara_transition(3);
  // Send header. Our header is 0xAA1234
  radio_buff[0] = 0xAA;
  radio_buff[1] = 0x12;
  radio_buff[2] = 0x34;
  PRINTF("Sending\r\n");
  // Send data. I'll just send 0x01
  for(int i = 3; i < LIBRADIO_BUFF_LEN; i++) {
      radio_buff[i] = 0x01;
  }
  // Send it!
  radio_send();
  PRINTF("Sent\r\n");
  test = 3 + test;
  PACA_TRANSITION_TO(task_delay,1);
}



