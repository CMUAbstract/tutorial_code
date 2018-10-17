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
#include "helper.h"

#define ROWS 4
#define COLS 513
#define DCOLS 1

/*
* If 0, print result using UART. If 1, print result using radio
*/
#define USE_RADIO 0

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

/**** Add more task code here ****/


/**** Do not need to touch anything below ****/
void task_print() {
#if USE_RADIO == 0
	print_result(ROWS, DCOLS, TS(result));
#else
	send_radio(ROWS, DCOLS, TS(result));
#endif
	TRANSITION_TO(task_init);
}
