#include <msp430.h>
#include <stdlib.h>

#include <libio/console.h>
#include <libmspbuiltins/builtins.h>
#include <libmsp/mem.h>
#include <libmsp/periph.h>
#include <libmsp/clock.h>
#include <libmsp/watchdog.h>
#include <libmsp/gpio.h>
#include <libfixed/fixed.h>
#include <libalpaca/alpaca.h>
#include <libcapybara/reconfig.h>
#include <libcapybara/board.h>
#include <libapds/gesture.h>
#include <libradio/radio.h>

#include "weights.h"

#define ROWS 4
#define COLS 513
#define DCOLS 1

#define zero 0
#define one 1
#define two 2
#define three 3

capybara_task_cfg_t pwr_configs[4] = {
	CFG_ROW(zero, CONFIGD, LOWP2,LOWP2),
	CFG_ROW(one, PREBURST, LOWP2,MEDHIGHP),
	CFG_ROW(two, BURST, MEDHIGHP, MEDHIGHP),
	CFG_ROW(three, CONFIGD, MEDP,MEDP),
};

void init();
void task_init();
void task_compute();
void task_dot_product();
void task_sample();
void task_gesture();
void task_send();
void task_delay();
void task_finish();

TASK(task_init);
TASK(task_compute);
TASK(task_dot_product);
TASK(task_sample);
TASK(task_gesture);
TASK(task_send);
TASK(task_delay);
TASK(task_finish);

ENTRY_TASK(task_init);
INIT_FUNC(init);

GLOBAL_SB(fixed, result, ROWS * DCOLS);
GLOBAL_SB(fixed, sample, COLS);
GLOBAL_SB(uint16_t, row_idx);

void init() {
	capybara_init();
}

void task_init() {
	PRINTF("\r\n Starting...");
	GV(row_idx) = 0;
	capybara_transition(3);
  apds_settle();
	TRANSITION_TO(task_sample);
}

void task_sample() {
  capybara_transition(1);
  int16_t proxVal = 0;
  enable_photoresistor();
  proxVal = read_photoresistor();
  if((proxVal < ALERT_THRESH) && proxVal > LOW_OUTPUT){
    disable_photoresistor();
    PRINTF("Got value %u\r\n",proxVal);
    TRANSITION_TO(task_gesture);
  }
  WAIT_PHOTORESISTOR_DELAY;
  TRANSITION_TO(task_sample);
}

void task_gesture() {

}

void task_compute() {
	capybara_transition(3);
	if(GV(row_idx) < ROWS) {
		TRANSITION_TO(task_dot_product);
	}
#ifdef CONSOLE
	for(uint16_t i = 0; i < ROWS; i++) {
		PRINTF("\r\n %i ", GV(result, i));
	}
#endif
	TRANSITION_TO(task_send);
}

void task_dot_product() {
	fixed w = 0;
	for(uint16_t i = 0; i < COLS; i++) {
		fixed tmp = F_MUL(GV(sample, i), weights[COLS * GV(row_idx) + i]);
		w = F_ADD(w, tmp);
	}
	uint16_t row = GV(row_idx);
	GV(result, row) = w;
	GV(row_idx)++;
	TRANSITION_TO(task_compute);
}

void task_send() {
	// Reconfigure bank
  capybara_transition(3);
	// Send header. Our header is 0xAA1234
  radio_buff[0] = 0xAA;
  radio_buff[1] = 0x12;
  radio_buff[2] = 0x34;
  // Send data. I'll just send 0x01
	for(int i = 3; i < LIBRADIO_BUFF_LEN && i - 5 < ROWS; i++) {
      radio_buff[i] = GV(result, i - 3);
  }
  // Send it!
	radio_send();
	TRANSITION_TO(task_delay);
}

void task_delay() {
	for (unsigned i = 0; i < 100; ++i) {
		__delay_cycles(4000);
  }
	TRANSITION_TO(task_init);
}

void task_finish() {
#ifdef CONSOLE
	for(uint16_t i = 0; i < ROWS; i++) {
		PRINTF("\r\n %i ", GV(result, i));
	}
#endif
		exit(0);
}
