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
void task_sort();
TASK(task_init);
TASK(task_compute);
TASK(task_exit);
TASK(task_sort);

ENTRY_TASK(task_init)
INIT_FUNC(init)

void init() {
  capybara_init();
	printf("Init'd\r\n");
}

////////////////////////////////////////////////////////////////////////////////
///////////////////////////////Weights Matrices/////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#include "common/mat.h"

////////////////////////////////////////////////////////////////////////////////
///////////////////////////////Implementation///////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void task_init() {
	uint16_t count = MAT_GET_DIM((&mat_b_dense), 0);
	MAT_RESHAPE(b1, count);
	MAT_RESHAPE(b2, count);
	MAT_RESHAPE(b3, count);
	mat_t *mat_input_ptr = &mat_a_dense;
	for(uint16_t i = 0; i < count; i++) {
		MAT_SET(b1, MAT_GET(mat_input_ptr, i, 0), i);
	}
	TRANSITION_TO(task_compute);
}

__nv mat_t *src;
__nv mat_t *dest;
__nv mat_t *inter;

int16_t partition(fixed *data, int16_t l, int16_t h) {
	int16_t x = data[h]; 
	int16_t i = (l - 1); 

	for(int16_t j = l; j <= h - 1; j++) {
		if (data[j] <= x) {
			i++;
			fixed tmp = data[i];
			data[i] = data[j];
			data[j] = tmp;
		}
	}
	fixed tmp = data[i + 1];
	data[i + 1] = data[h];
	data[h] = tmp;
	return (i + 1); 
}

void task_sort() {
	P1OUT |= BIT0;
	P1DIR |= BIT0;
	P1OUT &= ~BIT0;


	uint16_t count = MAT_GET_DIM(dest, 0);
	PRINTF("count = %i, mat_size = %i\r\n",count,MAT_SIZE);
	fixed *src_ptr = src->data;
	fixed *dest_ptr = dest->data;
	for(uint16_t i = 0; i < count; i++)
		dest_ptr[i] = src_ptr[i];

	uint16_t l = 0, h = count - 1;
	// TODO: confirm that this substitution is correct
	//uint16_t stack[count];
	uint16_t stack[MAT_SIZE];

	int16_t top = -1;
	stack[++top] = 0;
	stack[++top] = h;

	while(top >= 0) {
		//PRINTF("sorting...\r\n");
		h = stack[top--];
		l = stack[top--];

		int16_t p = partition(dest_ptr, l, h);

		if(p - 1 > l) {
			stack[++top] = l;
			stack[++top] = p - 1;
		}

		if(p + 1 < h) {
			stack[++top] = p + 1;
			stack[++top] = h;
		}
	}
	P1OUT |= BIT1;
	P1DIR |= BIT1;
	P1OUT &= ~BIT1;

	TRANSITION_TO(task_exit);
}

void task_compute() {
	// PLACE CODE HERE
	src = b1;
	dest = b2;
	inter = b3;

	TRANSITION_TO(task_sort);
}

void task_exit() {
	//while(1) {
#ifdef CONSOLE
	uint16_t count = MAT_GET_DIM((&mat_b_dense), 0);
	MAT_RESHAPE(b2, 1, 1, count);
	MAT_DUMP(b2, 0);
#endif
	//}
	TRANSITION_TO(task_init);
}
