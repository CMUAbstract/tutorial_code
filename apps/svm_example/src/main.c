#include <msp430.h>

#include <libio/console.h>
#include <libmspbuiltins/builtins.h>
#include <libmsp/mem.h>
#include <libmsp/periph.h>
#include <libmsp/clock.h>
#include <libmsp/watchdog.h>
#include <libmsp/gpio.h>
#include <libfixed/fixed.h>
#include <libmat/mat.h>

#include "sample.h"
#include "weights.h"

#define ROWS 4
#define COLS 513
#define DCOLS 1

__nv fixed result[ROWS * DCOLS];

__nv MAT_DECLARE(sample) = {
  .dims = {COLS, DCOLS},
	.strides = {DCOLS, 1},
  .len_dims = 2
};

__nv MAT_DECLARE(weights) = {
  .dims = {ROWS, COLS},
	.strides = {COLS, 1},
  .len_dims = 2,
};

__nv MAT_DECLARE(result) = {
  .dims = {ROWS, DCOLS},
	.strides = {DCOLS, 1},
  .len_dims = 2,
};

void matmul() {
  uint16_t rows = MAT_GET_DIM(weights, 0);
  uint16_t cols = MAT_GET_DIM(weights, 1);
  uint16_t dcols = MAT_GET_DIM(sample, 1);
  for(uint16_t i = 0; i < rows; i++) {
    for(uint16_t k = 0; k < dcols; k++) {
      fixed w = 0;
      for(uint16_t j = 0; j < cols; j++) {
        fixed tmp = F_MUL(MAT_GET(sample, j, k), MAT_GET(weights, i, j));
        w = F_ADD(w, tmp);
      }
      MAT_SET(result, w, i, k);
    }
  }
}

static void init_hw() {
    msp_watchdog_disable();
    msp_gpio_unlock();
    msp_clock_setup();
#ifdef CONSOLE
#warning console
    INIT_CONSOLE();
#endif
    __enable_interrupt();
}

int main() {
    init_hw();
		PRINTF("Starting...\r\n");
    matmul();
#ifdef CONSOLE
		for(uint16_t i = 0; i < ROWS; i++) {
			for(uint16_t j = 0; j < DCOLS; j++) {
				PRINTF("\r\n %i ", MAT_GET(result, i, j));
			}
		}
#endif
    return 0;
}
