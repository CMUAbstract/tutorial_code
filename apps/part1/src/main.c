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
#include <libcapybara/board.h>
#include <libcapybara/reconfig.h>

#include "sample.h"
#include "weights.h"

#define ROWS 4
#define COLS 513
#define DCOLS 1

enum {CORRECT_ANSWER, INCORRECT_ANSWER};

/*
* If 0, print result using UART. If 1, print result using radio
*/
#define USE_RADIO 0

#if USE_RADIO == 1
#include <libradio/radio.h>

#define zero 0
#define one 1
#define two 2
#define three 3
#define four 4

capybara_task_cfg_t pwr_configs[4] = {
  CFG_ROW(zero, CONFIGD, LOWP2,LOWP2),
  CFG_ROW(one, PREBURST, LOWP2,MEDHIGHP),
  CFG_ROW(two, BURST, MEDHIGHP, MEDHIGHP),
  CFG_ROW(three, CONFIGD, MEDP,MEDP),
};
#endif

void init();

/**** You do not need to touch anything above. ****/
/**** Your code starts here ****/

/**** Define more tasks here ****/
TASK(task_init);
TASK(task_print);

/**** Define more task shared variables here ****/
TASK_SHARED(fixed, result, ROWS * DCOLS); // place the final result here

ENTRY_TASK(task_init);
INIT_FUNC(init);

void init() {
	capybara_init();
}

void task_init() {
	PRINTF("\r\n Starting...");
}











/**** Do not need to touch anything below ****/
void task_print() {
#if USE_RADIO == 0
	for(uint16_t i = 0; i < ROWS; i++) {
		for(uint16_t k = 0; k < DCOLS; k++) {
			PRINTF("%u ", TS(result, i * DCOLS + k));
		}
	}
	PRINTF("\r\n");
#else
	// Reconfigure bank
	capybara_transition(3);
	// Send header. Our header is 0xAA1234
	radio_buff[0] = 0xAA;
	radio_buff[1] = 0x12;
	radio_buff[2] = 0x34;
	// Send data. I'll just send 0x01
	if (TS(result, 0) == 65216 && TS(result, 1) == 105
			&& TS(result, 2) == 65414 && TS(result, 3) == 65476) {
		// Correct
		radio_buff[4] = CORRECT_ANSWER;
	} else {
		// Incorrect
		radio_buff[4] = INCORRECT_ANSWER;
	}
	// Send it!
	radio_send();
#endif
	TRANSITION_TO(task_init);
}
