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

__nv fixed buf[ROWS * DCOLS];

__nv mat_t mat_inputs = {
  .dims = {COLS, DCOLS},
	.strides = {DCOLS, 1},
  .len_dims = 2,
  .data = sample
};

__nv mat_t mat_weights = {
  .dims = {ROWS, COLS},
	.strides = {COLS, 1},
  .len_dims = 2,
  .data = weight
};

__nv mat_t mat_result = {
  .dims = {ROWS, DCOLS},
	.strides = {DCOLS, 1},
  .len_dims = 2,
  .data = buf
};

__nv mat_t *input_ptr = &mat_inputs;
__nv mat_t *weight_ptr = &mat_weights;
__nv mat_t *result_ptr = &mat_result;

void matmul(mat_t *result, mat_t *weights, mat_t *inputs) {
  uint16_t rows = MAT_GET_DIM(weights, 0);
  uint16_t cols = MAT_GET_DIM(weights, 1);
  uint16_t dcols = MAT_GET_DIM(inputs, 1);
  for(uint16_t i = 0; i < rows; i++) {
    for(uint16_t k = 0; k < dcols; k++) {
      fixed w = 0;
      for(uint16_t j = 0; j < cols; j++) {
        fixed tmp = F_MUL(MAT_GET(inputs, j, k), MAT_GET(weights, i, j));
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
    matmul(result_ptr, weight_ptr, input_ptr);
#ifdef CONSOLE
    MAT_RESHAPE(result_ptr, 0, ROWS, DCOLS);
    MAT_DUMP(result_ptr, 0);
#endif
    return 0;
}
