// This file is for testing if our methodology for finding the break even point
// between enabling/disabling peripherals and leaving them running is correct
// This one in particularl looks for the break even point for the LSM6ds3 when
// both the accelerometer and gyro are enabled

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
#include <liblsm/gyro.h>
// Definitions supporting the alpaca language
#include <libalpaca/alpaca.h>

#include <libpacarana/pacarana.h>
void init();

//#define REENABLE


TASK(task_measure)
TASK(task_calc)
TASK(task_end)

__nv int stored_x;
__nv int stored_y;
__nv int stored_z;
__nv unsigned count = 0;
__nv unsigned total_runs = 0;
__nv unsigned ITER = 1000;
// This function runs first on every reboot
void init() {
  capybara_init();
  __delay_cycles(48000);
  fxl_set(BIT_SENSE_SW);
  __delay_cycles(48000);
  __delay_cycles(48000);
  gyro_init_data_rate_hm(0x80, 1);
  PRINTF("ON\r\n");
}


// Reads from the accelerometer
void task_measure() {
  uint16_t x,y,z;
#ifdef REENABLE
#ifdef DISPROF
  P1OUT |= BIT0;
  P1DIR |= BIT0;
  P1OUT &= ~BIT0;
#endif
  lsm_odr_reenable(0x80);
#ifdef DISPROF
  P1OUT |= BIT0;
  P1DIR |= BIT0;
  P1OUT &= ~BIT0;
#endif
#endif
  for (int i = 0; i < 10; i++) {
    accelerometer_read( &x, &y, &z);
    if (i > 2) {
      stored_x = x;
      stored_y = y;
      stored_z = z;
    }
    else {
      __delay_cycles(100);
    }
  }
#ifdef REENABLE
#ifdef DISPROF
  P1OUT |= BIT1;
  P1DIR |= BIT1;
  P1OUT &= ~BIT1;
#endif
    lsm_disable();
#ifdef DISPROF
  P1OUT |= BIT1;
  P1DIR |= BIT1;
  P1OUT &= ~BIT1;
#endif
#endif
  TRANSITION_TO(task_calc);
}

void task_calc() {
  /*P1OUT |= BIT1;
  P1DIR |= BIT1;
  P1OUT &= ~BIT1;*/
  uint16_t x,y,z; 
  //for (int j = 0; j < 1000; j++) {
  for (int j = 0; j < ITER; j++) {
    x++;
    y++;
    z++;
    if (x > 50000) {
      x = 0;
      y = 0;
      z = 0;
    }
    stored_x = x;
    stored_y = y;
    stored_z = z;
  }
  if (count < 9) {
    count++;
    /*P1OUT |= BIT1;
    P1DIR |= BIT1;
    P1OUT &= ~BIT1;*/
    TRANSITION_TO(task_measure);
  }
  else {
    /*P1OUT |= BIT1;
    P1DIR |= BIT1;
    P1OUT &= ~BIT1;*/
    TRANSITION_TO(task_end);
  }
}

void task_end() {
  P1OUT |= BIT0;
  P1DIR |= BIT0;
  P1OUT &= ~BIT0;
  count = 0;
  ITER += 1000;
  //ITER += 200;
  //if (ITER > 3000) {
  if (ITER > 5000) {
    ITER=1000;
  P1OUT |= BIT0;
  P1DIR |= BIT0;
  P1OUT &= ~BIT0;
  P1OUT |= BIT0;
  P1DIR |= BIT0;
  P1OUT &= ~BIT0;
  }
  TRANSITION_TO(task_measure);
}

ENTRY_TASK(task_measure)
INIT_FUNC(init)

