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

__nv fixed buf[ROWS * DCOLS];

__nv mat_t mat_inputs = {
  .dims = {COLS, DCOLS},
  .len_dims = 2,
  .data = sample
};

__nv mat_t mat_weights = {
  .dims = {ROWS, COLS},
  .len_dims = 2,
  .data = weight
};

__nv mat_t mat_result = {
  .dims = {ROWS, DCOLS},
  .len_dims = 2,
  .data = buf
};

__nv mat_t *input_ptr = &mat_inputs;
__nv mat_t *weight_ptr = &mat_weights;
__nv mat_t *result_ptr = &mat_result;

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
	GV(row_idx) = 0;
	TRANSITION_TO(task_compute);
}

void task_compute() {
  uint16_t rows = MAT_GET_DIM(weight_ptr, 0);
	if(GV(row_idx) < rows) {
		GV(row_idx)++;
		TRANSITION_TO(task_dot_product);
	}
	TRANSITION_TO(task_finish);
}

void task_dot_product() {
	uint16_t cols = MAT_GET_DIM(weight_ptr, 1);
	fixed w = 0;
	for(uint16_t i = 0; i < cols; i++) {
		fixed tmp = F_MUL(MAT_GET(input_ptr, i, 0), MAT_GET(weight_ptr, GV(row_idx), i));
		w = F_ADD(w, tmp);
	}
	MAT_SET(result_ptr, w, GV(row_idx), 0);
	TRANSITION_TO(task_compute);
}

void task_finish() {
#ifdef CONSOLE
    MAT_RESHAPE(result_ptr, 0, ROWS, DCOLS);
    MAT_DUMP(result, 0);
#endif
		exit(0);
}
