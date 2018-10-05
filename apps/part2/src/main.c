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
#include <libmat/mat.h>
#include <libalpaca/alpaca.h>

#include "sample.h"
#include "weights.h"

#define ROWS 4
#define COLS 513
#define DCOLS 1

__nv fixed result[ROWS * DCOLS];

__nv MAT_DECLARE(sample) = {
  .dims = {COLS, DCOLS},
	.strides = {DCOLS, 1},
  .len_dims = 2,
};

__nv MAT_DECLARE(weights) = {
  .dims = {ROWS, COLS},
	.strides = {COLS, 1},
  .len_dims = 2,
};

__nv MAT_DECLARE(result) = {
  .dims = {ROWS, DCOLS},
	.strides = {DCOLS, 1},
  .len_dims = 2,
};

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
  uint16_t rows = MAT_GET_DIM(weights, 0);
	if(GV(row_idx) < rows) {
		TRANSITION_TO(task_dot_product);
	}
	TRANSITION_TO(task_finish);
}

void task_dot_product() {
	uint16_t cols = MAT_GET_DIM(weights, 1);
	fixed w = 0;
	for(uint16_t i = 0; i < cols; i++) {
		fixed tmp = F_MUL(MAT_GET(sample, i, 0), MAT_GET(weights, GV(row_idx), i));
		w = F_ADD(w, tmp);
	}
	MAT_SET(result, w, GV(row_idx), 0);
	GV(row_idx)++;
	TRANSITION_TO(task_compute);
}

void task_finish() {
#ifdef CONSOLE
	for(uint16_t i = 0; i < ROWS; i++) {
		for(uint16_t j = 0; j < DCOLS; j++) {
			PRINTF("\r\n %i ", MAT_GET(result, i, j));
		}
	}
#endif
		exit(0);
}
