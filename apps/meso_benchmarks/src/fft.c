#include <stdlib.h>
#include <stdint.h>
//#include <string.h>
#include <stdio.h>

#define BIT_FLIP(port,bit) \
	P##port##OUT |= BIT##bit; \
	P##port##DIR |= BIT##bit; \
	P##port##OUT &= ~BIT##bit;


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
// Library for handling gyroscope
#include <liblsm/gyro.h>

#ifndef WORKLOAD_CYCLES
#define WORKLOAD_CYCLES 2
#endif


#ifndef RATE
// Valid rate definitions are 0x80, 0x40, 0x10 (based on what we have datasheet
// numbers for)
#warning "RATE UNDEFINED!!!!"
#define RATE 0x80
#endif

#define HIGHPERF_MASK 0b10000000

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////Tasks///////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void task_init();
void task_compute();
void task_exit();
TASK(task_init);
TASK(task_compute);
TASK(task_exit);
TASK(task_sense);

ENTRY_TASK(task_sense)
INIT_FUNC(init)

void init() {
	capybara_init();
  __delay_cycles(48000);
  fxl_set(BIT_SENSE_SW);
  __delay_cycles(48000);
	PRINTF("Init'd\r\n");
	// Initialize gyro
	BIT_FLIP(1,1);
	gyro_init_data_rate_hm(RATE, RATE & HIGHPERF_MASK);
}

////////////////////////////////////////////////////////////////////////////////
///////////////////////////////Weights Matrices/////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#include "common/mat.h"

// Mat for holding sensor data
// TODO make the size here configurable at compile time
__nv int16_t sensor_input[16][16];

__nv uint32_t  count_ = 0;

////////////////////////////////////////////////////////////////////////////////
///////////////////////////////Implementation///////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


// TODO: make this safe for intermittent execution
void task_sense() {
	printf("Sensing!\r\n");
	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 16; j++) {
			int16_t x,y,z;
			// Read output when available
			gyroscope_read( &x, &y, &z );
			// Only use x for now
			sensor_input[i][j] = x;
#ifndef HPVLP
			printf("%i ",x);
		}
		printf("\r\n");
#else
		}// apologies for putting a paren in an ifdef block...
#endif
	}
	TRANSITION_TO(task_init);
}

void task_init() {
	MAT_RESHAPE(b1, MAT_SIZE, MAT_SIZE);
	MAT_RESHAPE(b2, MAT_SIZE, MAT_SIZE);
	MAT_RESHAPE(b3, MAT_SIZE, MAT_SIZE);
	MAT_RESHAPE(b4, MAT_SIZE, MAT_SIZE);
	MAT_RESHAPE(b5, MAT_SIZE, MAT_SIZE);
	MAT_RESHAPE(b6, MAT_SIZE, MAT_SIZE);
	for(uint16_t i = 0; i < MAT_SIZE; i++) {
		for(uint16_t j = 0; j < MAT_SIZE; j++) {
			// Transform into fixed
			float raw_val = sensor_input[i][j]/250;
			fixed scaled_val = F_LIT(raw_val);
			// Write into matrix
			MAT_SET(b1, scaled_val, i, j);
#ifdef HPVLP
		} // And more apologies for doing it again
	}
#else
			printf("%i ",scaled_val/32);
		}
		printf("\r\n");
	}
	printf("Mat b1 start\r\n");
	MAT_DUMP(b1,0);
#endif
	// Adding a 2ms delay to force a settle
	for (int i = 0; i < 800; i++) {}
	BIT_FLIP(1,2)
#ifdef REENABLE
  lsm_accel_disable();
  lsm_gyro_sleep();
#endif
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
	//printf("rows = %i, cols = %i\r\n",rows,cols);
	uint16_t i = i_glob;
	while(i < rows) {
		num_elements = cols; 
		stride = 1; 
		row_idx = i;
		i_glob++;
		TRANSITION_TO(task_fft1d);
	}
	//printf("num_elements=%i rows=%i cols=%i\r\n",i,rows,cols);
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
	if(count_ > WORKLOAD_CYCLES) {
		BIT_FLIP(1,0);
		count_ = 0;
	}
	else {
		count_++;
	}
	TRANSITION_TO(task_exit);
}

void task_exit() {
#ifndef HPVLP
	uint16_t rows = MAT_GET_DIM(dest_real, 0);
	uint16_t cols = MAT_GET_DIM(dest_real, 1);
	MAT_RESHAPE(b2, 1, rows, cols);
	MAT_RESHAPE(b3, 1, rows, cols);
	printf("Final:");
	printf("Real:");
	MAT_DUMP(b2, 0);
	printf("Imaginary:");
	MAT_DUMP(b3, 0);
#endif
#ifdef REENABLE
  accel_odr_reenable(RATE);
  lsm_gyro_reenable();
  // We need this delay to simulate throwing out the first two results
  #if RATE == 0x80
  __delay_cycles(19200);
  #elif RATE == 0x40
  __delay_cycles(153800);
  #endif
#endif
	BIT_FLIP(1,1);
	i_glob = 0;
	j_glob = 0;
	row_idx = 0;
	col_idx = 0;
	num_elements = 0;
	stride = 0;
	TRANSITION_TO(task_sense);
}
