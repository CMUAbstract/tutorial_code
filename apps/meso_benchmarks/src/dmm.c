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
	PRINTF("init'd");
	}

////////////////////////////////////////////////////////////////////////////////
///////////////////////////////Weights Matrices/////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#include "common/mat.h"

////////////////////////////////////////////////////////////////////////////////
///////////////////////////////Implementation///////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void task_init() {
	// This assumes a square matrix
	uint16_t rows = MAT_GET_DIM((&mat_a_dense), 0);
	uint16_t cols = MAT_GET_DIM((&mat_a_dense), 1);
	MAT_RESHAPE(b1, rows, cols);
	MAT_RESHAPE(b2, rows, cols);
	mat_t *mat_input_ptr = &mat_a_dense;
	for(uint16_t i = 0; i < rows; i++) {
		for(uint16_t j = 0; j < cols; j++) {
			MAT_SET(b1, MAT_GET(mat_input_ptr, i, j), i, j);
		}
	}
	TRANSITION_TO(task_compute);
}

void task_compute() {
	P1OUT |= BIT0;
	P1DIR |= BIT0;
	P1OUT &= ~BIT0;
	// PLACE CODE HERE
	mat_t *src = b1;
	mat_t *dest = b2;
	mat_t *inter = b3;
	mat_t *filter = &mat_a_dense;

	uint16_t rows = MAT_GET_DIM(filter, 0);
	uint16_t cols = MAT_GET_DIM(filter, 1);
	uint16_t dcols = MAT_GET_DIM(dest, 1);
	MAT_RESHAPE(inter, rows, dcols);

#define min(a, b) (((a) < (b)) ? (a) : (b))

  for(uint16_t i = 0; i < rows; i += BLOCK_SIZE) {
    for(uint16_t j = 0; j < dcols; j += BLOCK_SIZE) {
      for(uint16_t k = 0; k < cols; k += BLOCK_SIZE) {
        fixed *filter_ptr = filter->data + i * cols + k;
        for(uint16_t l = i; l < min(i + BLOCK_SIZE, rows); l++) {
          fixed *dest_ptr = dest->data + l * dcols + j;
          for(uint16_t m = j; m < min(j + BLOCK_SIZE, dcols); m++) {
            fixed w = 0;
            fixed *src_ptr = src->data + dcols * k + m;
            fixed *cur_filter_ptr = filter_ptr;
            for(uint16_t n = k; n < min(k + BLOCK_SIZE, cols); n++) {
              w = F_ADD(w, F_MUL(*cur_filter_ptr, *src_ptr));
              src_ptr += dcols;
              cur_filter_ptr++;
            }
            *dest_ptr++ += w;
          }
          filter_ptr += cols;
        }
      }
    }
  }
	P1OUT |= BIT0;
	P1DIR |= BIT0;
	P1OUT &= ~BIT0;

	TRANSITION_TO(task_exit);
}

void task_exit() {
	//while(1) {
	uint16_t rows = MAT_GET_DIM(b2, 0);
	uint16_t cols = MAT_GET_DIM(b2, 1);
	MAT_RESHAPE(b2, 1, rows, cols);
#ifdef CONSOLE
	MAT_DUMP(b2, 0);
#endif
	//}
	TRANSITION_TO(task_init);
}
