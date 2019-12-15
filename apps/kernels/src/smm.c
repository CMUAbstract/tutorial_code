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
void task_smm();
void task_exit();
TASK(task_init);
TASK(task_compute);
TASK(task_smm);
TASK(task_exit);

void init() {
	capybara_init();
	for(int i = 0; i < 100; i++) {
		__delay_cycles(100000);
		PRINTF("init'd\r\n");
	}
	PRINTF("Init'd\r\n");
}

ENTRY_TASK(task_init)
INIT_FUNC(init)


////////////////////////////////////////////////////////////////////////////////
///////////////////////////////Weights Matrices/////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#include "common/mat.h"

////////////////////////////////////////////////////////////////////////////////
///////////////////////////////Implementation///////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void task_init() {
	PRINTF("Task init\r\n");
	TRANSITION_TO(task_compute);
}

__nv mat_t *src;
__nv mat_t *bsrc;
__nv mat_t *dest;
__nv mat_t *inter;
__nv mat_t *filter;
__nv mat_t *bfilter;
__nv uint16_t rows;
__nv uint16_t cols;
__nv uint16_t dcols;

void task_compute() {
	PRINTF("Task compute\r\n");
	// PLACE CODE HERE
	src = &mat_a_sparse;
	dest = b2;
	inter = b3;
	filter = &mat_a_sparse;
	bsrc = &mat_a_bsparse;
	bfilter = &mat_a_bsparse;

	rows = filter->sparse.dims[0];
	cols = filter->sparse.dims[1];
	dcols = src->sparse.dims[1];
	MAT_RESHAPE(inter, rows, dcols);
	MAT_RESHAPE(dest, rows, dcols);

	TRANSITION_TO(task_smm);
}

void task_smm() {
	P1OUT |= BIT0;
	P1DIR |= BIT0;
	P1OUT &= ~BIT0;
	PRINTF("Alive\r\n");
	fixed *dest_ptr = dest->data;
	for(uint16_t i = 0; i < rows; i++) {
		for(uint16_t j = 0; j < cols; j++) {
			*dest_ptr++ = 0;
		}
	}
#if 1
	for(uint16_t i = 0; i < rows; i++) {
		fixed *dest_ptr = MAT_PTR(dest, i, 0);
		uint16_t j = filter->sparse.sizes[i];
		for(; j < filter->sparse.sizes[i + 1]; j++) {
			uint16_t row = filter->sparse.offsets[j];	
			uint16_t k = src->sparse.sizes[row];
			fixed f = filter->data[j];
			for(; k < src->sparse.sizes[row + 1]; k++) {
				fixed w = F_MUL(f, src->data[k]);
				uint16_t col = src->sparse.offsets[k];
				fixed *d = dest_ptr + col;
				*d = F_ADD(w, *d);
			}
		}
	}
#else

	uint16_t block_cols = bsrc->sparse.dims[1];
	uint16_t filter_offset = 0;
	uint16_t filter_block_offset = 0;
	for(uint16_t i = 0; i < rows; i += BLOCK_SIZE) {
		PRINTF("i = %i\r\n",i);
		uint16_t src_offset = 0;
		uint16_t src_block_offset = 0;
		for(uint16_t j = 0, crow = 0; 
			j < cols; j += BLOCK_SIZE, crow += (BLOCK_SIZE + 1) * block_cols) {
			PRINTF("j = %i\r\n",j);

			for(uint16_t k = 0, ccol = 0; k < dcols; 
				k += BLOCK_SIZE, ccol += (BLOCK_SIZE + 1)) {
				PRINTF("k = %i\r\n",k);

				uint16_t m = filter_block_offset;
				for(; m < filter_block_offset + BLOCK_SIZE; m++) {
					PRINTF("m = %i\r\n",m);
					uint16_t row = i + m - filter_block_offset;
					fixed *dest_ptr = MAT_PTR(dest, row, 0);
					uint16_t n = bfilter->sparse.sizes[m] + filter_offset;
					for(; n < bfilter->sparse.sizes[m + 1] + filter_offset; n++) {
						PRINTF("n = %i\r\n",n);
						fixed f = bfilter->data[n];
						uint16_t col = bfilter->sparse.offsets[n];

						uint16_t col_idx = crow + ccol + col % BLOCK_SIZE;
						uint16_t p = bsrc->sparse.sizes[col_idx] + src_offset;
						for(; p < bsrc->sparse.sizes[col_idx + 1] + src_offset; p++) {
							PRINTF("p = %x / %x\r\n",p,bsrc->sparse.sizes[col_idx + 1] + src_offset);
							fixed w = F_MUL(f, bsrc->data[p]);
							uint16_t dcol = bsrc->sparse.offsets[p];
							fixed *d = dest_ptr + dcol;
							*d = F_ADD(w, *d);
						}
					}
				}
				PRINTF("Another loop\r\n");
				src_offset += bsrc->sparse.sizes[src_block_offset + BLOCK_SIZE];
				src_block_offset += BLOCK_SIZE + 1;
			}

			filter_offset += bfilter->sparse.sizes[filter_block_offset + BLOCK_SIZE];
			filter_block_offset += BLOCK_SIZE + 1;
		}
	}
#endif
	P1OUT |= BIT1;
	P1DIR |= BIT1;
	P1OUT &= ~BIT1;
	TRANSITION_TO(task_exit);
}

void task_exit() {
	PRINTF("Task exit\r\n");
	//while(1) {
	uint16_t rows = MAT_GET_DIM(b2, 0);
	uint16_t cols = MAT_GET_DIM(b2, 1);
	MAT_RESHAPE(b2, 1, rows, cols);
#ifdef CONSOLE
	MAT_DUMP(b2, 0);
#endif
	PRINTF("Actual output..2");
	//}
	TRANSITION_TO(task_init);
}
