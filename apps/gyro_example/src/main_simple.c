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

TASK(task_init,0);
TASK(task_filter,1);

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
}

__nv int test;
__nv uint8_t num_samps;
__nv uint16_t samples[24];

void task_init() {
  STATE_CHECK(photores, 0x1);
  /*enable_photoresistor();
#ifdef PACARANA
  STATE_CHANGE(photores, 0x1);
#endif*/
  uint16_t proxVal;
  num_samps = 8;
  for(int i = 0; i < num_samps; i++) {
    proxVal = read_photoresistor();
    printf("%u\r\n",proxVal);
  }
  disable_photoresistor();
#ifdef PACARANA
  STATE_CHANGE(photores, 0x0);
#endif
  // Move our bunch of samples to task filter
  PACA_TRANSITION_TO(task_filter,1)
}


void task_filter() {
  // We purposely let the  gyro continue on in whatever mode because we want the
  // model to warn us about this
  float avg[3];
  double sum;
  for (int j = 0; j < 3; j++) {
    sum = 0;
    for (int i = 0; i < num_samps; i++) {
      sum += samples[i*3 + j];
    }
    avg[j] = sum / num_samps;
  }
  enable_photoresistor();
  STATE_CHANGE(photores,0x1);
  // If anomaly condition detected
  PACA_TRANSITION_TO(task_init,0);
}

