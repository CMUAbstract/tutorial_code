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

#include "sample.h"
#include "weights.h"

#define ROWS 4
#define COLS 513
#define DCOLS 1


void init();
void task_init();
void task_compute();
void task_dot_product();
void task_finish();

TASK(task_init);
TASK(task_compute);
TASK(task_dot_product);
TASK(task_finish);

ENTRY_TASK(task_init);
INIT_FUNC(init);

GLOBAL_SB(uint16_t, row_idx);
GLOBAL_SB(fixed[ROWS * DCOLS], result);

static void init_hw() {
    msp_watchdog_disable();
    msp_gpio_unlock();
    msp_clock_setup();
#ifdef CONSOLE
#warning console
    INIT_CONSOLE();
#endif
    __enable_interrupt();
}

void init() {
	init_hw();
}

void task_init() {
	PRINTF("\r\n Starting...");
	GV(row_idx) = 0;
	TRANSITION_TO(task_compute);
}

void task_compute() {
	if(GV(row_idx) < ROWS) {
		TRANSITION_TO(task_dot_product);
	}
	TRANSITION_TO(task_finish);
}

void task_dot_product() {
	fixed w = 0;
	for(uint16_t i = 0; i < COLS; i++) {
		fixed tmp = F_MUL(sample[i], weights[GV(row_idx) * COLS + i]);
		w = F_ADD(w, tmp);
	}
	GV(result)[GV(row_idx)] = w;
	GV(row_idx)++;
	TRANSITION_TO(task_compute);
}

void task_finish() {
#ifdef CONSOLE
	for(uint16_t i = 0; i < ROWS; i++) {
			PRINTF("\r\n %i ", GV(result)[i]);
	}
#endif
		exit(0);
}
