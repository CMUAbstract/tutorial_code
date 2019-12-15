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
	MAT_RESHAPE(b4, MAT_SIZE, MAT_SIZE);
	MAT_RESHAPE(b5, MAT_SIZE, MAT_SIZE);
	MAT_RESHAPE(b6, MAT_SIZE, MAT_SIZE);
	mat_t *mat_input_ptr = &mat_a_dense;
	for(uint16_t i = 0; i < MAT_SIZE; i++) {
		for(uint16_t j = 0; j < MAT_SIZE; j++) {
			MAT_SET(b1, MAT_GET(mat_input_ptr, i, j), i, j);
		}
	}
	P1OUT |= BIT0;
	P1DIR |= BIT0;
	P1OUT &= ~BIT0;

	TRANSITION_TO(task_compute);
}

__nv uint16_t row_idx = 0;
__nv uint16_t col_idx = 0;
__nv uint16_t stride = 0;
__nv uint16_t num_elements = 0;
__nv mat_t *dest_real;
__nv mat_t *dest_imag;
__nv mat_t *inter_real;
__nv mat_t *inter_imag;
__nv mat_t *inter2_real;
__nv mat_t *inter2_imag;
__nv uint16_t i_glob = 0, j_glob = 0;
uint16_t flag = 1;

#include "fft_funcs.h"

void task_compute() {
	// PLACE CODE HERE
	dest_real = b2;
	dest_imag = b3;
	inter_real = b1;
	inter_imag = b4;
	inter2_real = b5;
	inter2_imag = b6;
	uint16_t rows = MAT_GET_DIM(inter_real, 0);
	uint16_t cols = MAT_GET_DIM(inter_real, 1);

	uint16_t i = i_glob;
	while(i < rows) {
		num_elements = cols; 
		stride = 1; 
		row_idx = i;
		i_glob++;
		TRANSITION_TO(task_fft1d);
	}
	
	fixed *inter_real_ptr = MAT_PTR(inter_real, 0, 0) + i;
	fixed *inter_imag_ptr = MAT_PTR(inter_imag, 0, 0) + i;
	fixed *dest_real_ptr = MAT_PTR(dest_real, 0, 0) + i;
	fixed *dest_imag_ptr = MAT_PTR(dest_imag, 0, 0) + i;
	for(i = 0; i < rows * cols; i++) {
		*inter_real_ptr++ = *dest_real_ptr++;
		*inter_imag_ptr++ = *dest_imag_ptr++;
	}

	uint16_t j = j_glob;
	while(j < cols) {
		num_elements = rows;
		stride = cols;
		row_idx = 0;
		col_idx = j;
		j_glob++;
		TRANSITION_TO(task_fft1d);
	}
	P1OUT |= BIT0;
	P1DIR |= BIT0;
	P1OUT &= ~BIT0;
	
	TRANSITION_TO(task_exit);
}

void task_exit() {
	//while(1) {
#ifdef CONSOLE
	uint16_t rows = MAT_GET_DIM(dest_real, 0);
	uint16_t cols = MAT_GET_DIM(dest_real, 1);
	MAT_RESHAPE(b2, 1, rows, cols);
	MAT_RESHAPE(b3, 1, rows, cols);
	printf("Real:");
	MAT_DUMP(b2, 0);
	printf("Imaginery:");
	MAT_DUMP(b3, 0);
#endif
	//}
	TRANSITION_TO(task_init);
}
