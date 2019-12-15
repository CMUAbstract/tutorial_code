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

#define LEFT_ARROW 1
#define UP_ARROW 2
#define DIAG_ARROW 4

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////Tasks///////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void task_init();
void task_compute();
void task_align();
void task_exit();
TASK(task_init);
TASK(task_compute);
TASK(task_exit);
TASK(task_align);

ENTRY_TASK(task_init)
INIT_FUNC(init)

void init() {
  capybara_init();
	PRINTF("Init'd\r\n");
	P1OUT |= BIT1;
	P1DIR |= BIT1;
	P1OUT &= ~BIT1;
  }

////////////////////////////////////////////////////////////////////////////////
///////////////////////////////Weights Matrices/////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#include "common/mat.h"

////////////////////////////////////////////////////////////////////////////////
///////////////////////////////Implementation///////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
__nv uint16_t test1[16] = {'G','A','A','C',
						'G','A','A','C',
						'G','A','A','C',
						'G','A','A','C'};
__nv uint16_t test2[16] = {'G','C','A','C',
						'G','C','A','C',
						'G','C','A','C',
						'G','C','A','C'};
void task_init() {
	uint16_t count = MAT_GET_DIM((&mat_a_dense), 0);
	MAT_RESHAPE(b1, count);
	MAT_RESHAPE(b2, count);
	MAT_RESHAPE(b3, count, count);
	MAT_RESHAPE(b4, count, count);
	mat_t *mat_input_ptr = &mat_a_dense;
  // No longer safe for intermittent execution
	for(uint16_t i = 0; i < count; i++) {
		// MAT_SET(b1, test1[i], i);
		// MAT_SET(b2, test2[i], i);
		MAT_SET(b1, MAT_GET(mat_input_ptr, i, 0), i);
		MAT_SET(b2, MAT_GET(mat_input_ptr, (i + 4) % count, 0), i);
	}
	TRANSITION_TO(task_compute);
}

__nv mat_t *src1;
__nv mat_t *src2;
__nv mat_t *dest;
__nv mat_t *inter;


void task_compute() {
	// PLACE CODE HERE
	src1 = b1;
	src2 = b2;
	dest = b3;
	inter = b4;

	TRANSITION_TO(task_align);
}

void task_align() {
	P1OUT |= BIT0;
	P1DIR |= BIT0;
	P1OUT &= ~BIT0;
	int16_t *src1_ptr = src1->data;
	int16_t *src2_ptr = src2->data;
	int16_t *dest_ptr = dest->data;
	int16_t *inter_ptr = inter->data;

	uint16_t count = MAT_GET_DIM(src1, 0);

	// Zero out the penalty matrix
	uint16_t *row_ptr = inter_ptr;
	uint16_t *col_ptr = inter_ptr;
	for(uint16_t i = 0; i < count; i++) {
		*col_ptr++ = -i;
		*row_ptr = -i;
		row_ptr += count;
	}

	for(uint16_t i = 1; i < count; i++) {
		// k is row index, j is column index
		int16_t *inter_topLeft = inter_ptr + i - 1;
		int16_t *inter_top = inter_ptr + i;
		int16_t *inter_left = inter_ptr + count + i - 1;
		int16_t *dest_diag = dest_ptr + i + count;
		int16_t *inter_diag = inter_ptr + i + count;
		for(uint16_t j = i, k = 1; j > 0; j--, k++) {
			int16_t topleft = *inter_topLeft;
			inter_topLeft += count - 1;
			int16_t top = *inter_top;
			inter_top += count - 1;
			int16_t left = *inter_left;
			inter_left += count - 1;
			int16_t char1 = src1_ptr[k - 1];
			int16_t char2 = src2_ptr[j - 1];
			if(char1 == char2) topleft += 1;
			else topleft -= 1;
			top -= 1;
			left -= 1;
			uint16_t dxn = 0;
			int16_t max = (top >= left) ? top : left;
			if(topleft >= max) max = topleft;

			if(topleft == max) dxn += DIAG_ARROW;
			if(top == max) dxn += UP_ARROW;
			if(left == max) dxn += LEFT_ARROW;

			*dest_diag = dxn;
			dest_diag += count - 1;
			*inter_diag = max;
			inter_diag += count - 1;
		}
	}

	for(uint16_t i = 1; i < count; i++) {
		// k is row index, j is column index
		int16_t *inter_topLeft = MAT_PTR(inter, i - 1, count - 2);
		int16_t *inter_top = MAT_PTR(inter, i - 1, count - 1);
		int16_t *inter_left = MAT_PTR(inter, i, count - 2); 
		int16_t *dest_diag = MAT_PTR(dest, i, count - 1);
		int16_t *inter_diag = MAT_PTR(inter, i, count - 1);
		for(uint16_t j = count, k = i; j > i; j--, k++) {
			int16_t topleft = *inter_topLeft;
			inter_topLeft += count - 1;
			int16_t top = *inter_top;
			inter_top += count - 1;
			int16_t left = *inter_left;
			inter_left += count - 1;
			int16_t char1 = src1_ptr[k - 1];
			int16_t char2 = src2_ptr[j - 2];
			if(char1 == char2) topleft += 1;
			else topleft -= 1;
			top -= 1;
			left -= 1;
			uint16_t dxn = 0;
			int16_t max = (top >= left) ? top : left;
			if(topleft >= max) max = topleft;

			if(topleft == max) dxn += DIAG_ARROW;
			if(top == max) dxn += UP_ARROW;
			if(left == max) dxn += LEFT_ARROW;

			*dest_diag = dxn;
			dest_diag += count - 1;
			*inter_diag = max;
			inter_diag += count - 1;
		}
	}
	P1OUT |= BIT0;
	P1DIR |= BIT0;
	P1OUT &= ~BIT0;

	TRANSITION_TO(task_exit);
}

void task_exit() {
	//while(1) {
#ifdef CONSOLE
	uint16_t count = MAT_GET_DIM((&mat_b_dense), 0);
	printf("\nDistance Matrix:");
	MAT_RESHAPE(b4, 1, count, count);
	MAT_DUMP(b4, 0);
	printf("\nDirection Matrix:");
	MAT_RESHAPE(b3, 1, count, count);
	MAT_DUMP(b3, 0);
#endif
	//}
	TRANSITION_TO(task_init); 
}
