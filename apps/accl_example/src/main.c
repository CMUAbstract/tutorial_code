// Built-in libraries for the msp430
#include <msp430.h>
#include <libmspbuiltins/builtins.h>
#include <libmsp/clock.h>
#include <libmsp/watchdog.h>
#include <libmsp/gpio.h>
#include <libio/console.h>

// Functions for the capybara board
#include <libcapybara/board.h>

// Functions for the accelerometer
#include <liblsm/accl.h>

// Definitions supporting the alpaca language
#include <libalpaca/alpaca.h>

void init();

TASK(task_measure)

// This function runs first on every reboot
void init() {
  capybara_init();
  fxl_set(BIT_SENSE_SW);
  __delay_cycles(16000);
  lsm_reset();
  accelerometer_init();
}


// Reads from the accelerometer
void task_measure() {
  uint16_t x,y,z;
  for (int i = 0; i < 200; i++) {
    accelerometer_read( &x, &y, &z);
    if (i > 20) {
      PRINTF("X: %i Y: %i Z: %i\r\n", (int)x,(int)y,(int)z);
    }
    else {
      __delay_cycles(100);
    }
  }
  while(1);
  TRANSITION_TO(task_measure);
}


ENTRY_TASK(task_measure)
INIT_FUNC(init)

