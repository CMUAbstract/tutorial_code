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
	mat_t *src = b1;
	mat_t *dest = b2;
	mat_t *inter = b3;
	mat_t *filter = &mat_c_dense;

	uint16_t rows = MAT_GET_DIM(dest, 0);
	uint16_t cols = MAT_GET_DIM(dest, 1);
	MAT_RESHAPE(inter, rows, cols);

	uint16_t flayers = MAT_GET_DIM(filter, 1);
	uint16_t frows = MAT_GET_DIM(filter, 2);
	uint16_t fcols = MAT_GET_DIM(filter, 3);
	P1OUT |= BIT0;
	P1DIR |= BIT0;
	P1OUT &= ~BIT0;

  for(uint16_t i = 0; i < flayers; i++) {
    fixed *dest_ptr = dest->data;
    for(uint16_t j = 0; j < rows; j++) {
      for(uint16_t k = 0; k < cols; k++) {
        fixed w = 0;
        fixed *filter_ptr = MAT_PTR(filter, i, 0, 0);
        fixed *src_ptr = MAT_PTR(src, i, j, k);
        for(uint16_t l = 0; l < frows; l++) {
          for(uint16_t m = 0; m < fcols; m++) {
            w = F_ADD(w, F_MUL(*src_ptr, *filter_ptr));
            filter_ptr++;
            src_ptr++;
          }
        }
        *dest_ptr++ = w;
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
