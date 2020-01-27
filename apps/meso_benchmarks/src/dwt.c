#include <stdlib.h>
#include <stdint.h>
//#include <string.h>
#include <stdio.h>

// Built-in libraries for the msp430
#include <msp430.h>
#include <libmspbuiltins/builtins.h>
#include <libmsp/clock.h>
#include <libmsp/watchdog.h>
#include <libmsp/gpio.h>
#include <libio/console.h>
// Functions for the capybara board
#include <libcapybara/board.h>
// Definitions supporting the alpaca language
#include <libalpaca/alpaca.h>
// Definitions for pacarana specifics
#include <libpacarana/pacarana.h>
// Libraries for handling matrices and fixed point calculations
#include <libfixed/fixed.h>
#include <libmat/mat.h>


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////Tasks///////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void task_init();
void task_compute();
void task_exit();
TASK(task_init);
TASK(task_compute);
TASK(task_exit);

ENTRY_TASK(task_init)
INIT_FUNC(init)

void init() {
	capybara_init();
	PRINTF("Init'd\r\n");
	}

////////////////////////////////////////////////////////////////////////////////
///////////////////////////////Weights Matrices/////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#include "common/mat.h"

////////////////////////////////////////////////////////////////////////////////
///////////////////////////////Implementation///////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void task_init() {
	MAT_RESHAPE(b1, MAT_SIZE, MAT_SIZE);
	MAT_RESHAPE(b2, MAT_SIZE, MAT_SIZE);
	MAT_RESHAPE(b3, MAT_SIZE, MAT_SIZE);
	mat_t *mat_input_ptr = &mat_a_dense;
	for(uint16_t i = 0; i < MAT_SIZE; i++) {
		for(uint16_t j = 0; j < MAT_SIZE; j++) {
			MAT_SET(b1, MAT_GET(mat_input_ptr, i, j), i, j);
		}
	}
	TRANSITION_TO(task_compute);
}

__nv uint8_t enable_copy = 1;
__nv uint16_t row_idx = 0;
__nv uint16_t col_idx = 0;
__nv uint16_t stride = 0;
__nv uint16_t num_elements = 0;
__nv mat_t *src;
__nv mat_t *dest;
__nv mat_t *inter;
__nv uint16_t i_glob = 0, j_glob = 0;
#include "dwt.h"

void task_compute() {
	// PLACE CODE HERE
	src = b1;
	dest = b2;
	inter = b3;
	uint16_t rows = MAT_GET_DIM(dest, 0);
	uint16_t cols = MAT_GET_DIM(dest, 1);

	uint16_t i = i_glob;
	while(i < rows) {
		enable_copy = 1;
		num_elements = cols; 
		stride = 1; 
		row_idx = i;
    i_glob++;
		TRANSITION_TO(task_dwt1d);
	}
	
	// MAT_RESHAPE(b2, 1, rows, cols);
	// MAT_DUMP(b2, 0);
	// exit(0);	
	uint16_t j = j_glob;
	while(j < cols) {
		enable_copy = 0;
		num_elements = rows;
		stride = cols;
		row_idx = 0;
		col_idx = j;
    j_glob++;
		TRANSITION_TO(task_dwt1d);
	}
	
	TRANSITION_TO(task_exit);
}

void task_exit() {
	while(1) {
#ifdef CONSOLE
	uint16_t rows = MAT_GET_DIM(dest, 0);
	uint16_t cols = MAT_GET_DIM(dest, 1);
	MAT_RESHAPE(b2, 1, rows, cols);
	MAT_DUMP(b2, 0);
#endif
	}
}
