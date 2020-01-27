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

void init() {capybara_init();
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
	uint16_t rows = MAT_GET_DIM((&mat_a_dense), 0);
	uint16_t cols = MAT_GET_DIM((&mat_a_dense), 1);
	uint16_t frows = MAT_GET_DIM((&mat_c_dense), 2);
	uint16_t fcols = MAT_GET_DIM((&mat_c_dense), 3);
	MAT_RESHAPE(b1, 1, rows, cols);
	MAT_RESHAPE(b2, (rows - frows + 1), (cols - fcols + 1));
	MAT_RESHAPE(b3, (rows - frows + 1), (cols - fcols + 1));
	mat_t *mat_input_ptr = &mat_a_dense;
	for(uint16_t i = 0; i < rows; i++) {
		for(uint16_t j = 0; j < cols; j++) {
			MAT_SET(b1, MAT_GET(mat_input_ptr, i, j), 0, i, j);
		}
	}
	TRANSITION_TO(task_compute);
}

void task_compute() {
	// PLACE CODE HERE
	P1OUT |= BIT0;
	P1DIR |= BIT0;
	P1OUT &= ~BIT0;
	mat_t *src = b1;
	mat_t *dest = b2;
	mat_t *inter = b3;
	mat_t *filter = &mat_c_sparse;

	uint16_t rows = MAT_GET_DIM(dest, 0);
	uint16_t cols = MAT_GET_DIM(dest, 1);
	MAT_RESHAPE(inter, rows, cols);

	uint16_t frows = filter->sparse.dims[2];
	uint16_t fcols = filter->sparse.dims[3];
	// Only a 2D convolution
	uint16_t total_elements = filter->sparse.sizes[0] + 1;

	uint16_t fsize = frows * fcols;

	fixed *dest_ptr = dest->data;
	for(uint16_t i = 0; i < rows; i++) {
		for(uint16_t j = 0; j < cols; j++) {
			fixed w = 0;
			uint16_t pos = 0;
			uint16_t idx = filter->sparse.offsets[pos];
			fixed *filter_ptr = filter->data;
			while(pos < total_elements) {
				uint16_t k = idx / fsize;
				uint16_t l = (idx % fsize) / fcols;
				uint16_t m = idx % fcols;
				w = F_ADD(w, F_MUL(MAT_GET(src, k, i + l, j + m), *filter_ptr));
				pos++;
				idx += filter->sparse.offsets[pos];
				filter_ptr++;
			}
			*dest_ptr++ = w;
		}
	}
	P1OUT |= BIT1;
	P1DIR |= BIT1;
	P1OUT &= ~BIT1;

	TRANSITION_TO(task_exit);
}

void task_exit() {
	//while(1) {
#ifdef CONSOLE
	uint16_t rows = MAT_GET_DIM((&mat_a_dense), 0);
	uint16_t cols = MAT_GET_DIM((&mat_a_dense), 1);
	uint16_t frows = MAT_GET_DIM((&mat_c_dense), 2);
	uint16_t fcols = MAT_GET_DIM((&mat_c_dense), 3);
	MAT_RESHAPE(b2, 1, (rows - frows + 1), (cols - fcols + 1));
	MAT_DUMP(b2, 0);
#endif
	//}
	TRANSITION_TO(task_init);
}
