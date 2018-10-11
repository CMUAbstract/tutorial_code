#include <msp430.h>

#include <libio/console.h>
#include <libmspbuiltins/builtins.h>
#include <libmsp/mem.h>
#include <libmsp/periph.h>
#include <libmsp/clock.h>
#include <libmsp/watchdog.h>
#include <libmsp/gpio.h>
#include <libfixed/fixed.h>

#include "sample.h"
#include "weights.h"

#define ROWS 4
#define COLS 513
#define DCOLS 1

__nv fixed result[ROWS * DCOLS];

void matmul() {
  for(uint16_t i = 0; i < ROWS; i++) {
    for(uint16_t k = 0; k < DCOLS; k++) {
      fixed w = 0;
      for(uint16_t j = 0; j < COLS; j++) {
        fixed tmp = F_MUL(sample[j * DCOLS + k], weights[i * COLS + j]);
        w = F_ADD(w, tmp);
      }
      MAT_SET(result, w, i, k);
			result[i * DCOLS + k] = w;
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
				PRINTF("\r\n %i ", result[i]);
		}
#endif
    return 0;
}
