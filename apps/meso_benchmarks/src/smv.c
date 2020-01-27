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
	MAT_RESHAPE(b1, MAT_GET_DIM((&mat_b_dense), 0), 1);
	MAT_RESHAPE(b2, MAT_GET_DIM((&mat_a_dense), 0), 1);
	MAT_RESHAPE(b3, MAT_GET_DIM((&mat_a_dense), 0), 1);
	mat_t *mat_input_ptr = &mat_b_dense;
	for(uint16_t i = 0; i < MAT_GET_DIM((&mat_b_dense), 0); i++) {
		MAT_SET(b1, MAT_GET(mat_input_ptr, i, 0), i, 0);
	}
	TRANSITION_TO(task_compute);
}

static __nvram fixed val_bak = 0;
static __nvram struct {
	uint16_t i;
	uint16_t j;
} pos_bak;

void task_compute() {
	// PLACE CODE HERE
	mat_t *src = b1;
	mat_t *dest = b2;
	mat_t *filter = &mat_a_sparse;
	mat_t *bfilter = &mat_a_bsparse;

	uint16_t rows = MAT_GET_DIM(dest, 0);
	uint16_t dcols = MAT_GET_DIM(dest, 1);

	uint16_t block_cols = bfilter->sparse.dims[1];
	uint16_t cols = block_cols * BLOCK_SIZE;
	uint16_t block_offset = 0;
	uint16_t offset = 0;
	for(uint16_t i = 0; i < rows; i += BLOCK_SIZE) {
		PRINTF("Alive\r\n");
		for(uint16_t j = 0; j < cols; j += BLOCK_SIZE) {
			fixed *dest_ptr = dest->data + i;
			for(uint16_t k = block_offset; k < block_offset + BLOCK_SIZE; k++) {
				fixed w = 0;
				uint16_t l = bfilter->sparse.sizes[k] + offset;
				for(; l < bfilter->sparse.sizes[k + 1] + offset; l++) {
					fixed *src_ptr = src->data + bfilter->sparse.offsets[l];
					w = F_ADD(w, F_MUL(bfilter->data[l], *src_ptr));
				}
				*dest_ptr++ += w;
			}
			offset += bfilter->sparse.sizes[block_offset + BLOCK_SIZE];
			block_offset += BLOCK_SIZE + 1;
		}
	}

	TRANSITION_TO(task_exit);
}

void task_exit() {
	while(1) {
	MAT_RESHAPE(b2, 1, MAT_GET_DIM((&mat_a_dense), 0), 1);
#ifdef CONSOLE
	MAT_DUMP(b2, 0);
#endif
	}
}
