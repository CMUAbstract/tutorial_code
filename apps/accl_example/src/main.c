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

TASK(task_measure)
TASK(task_calc)

__nv int stored_x;
__nv int stored_y;
__nv int stored_z;

// This function runs first on every reboot
void init() {
  capybara_init();
  __delay_cycles(48000);
  __delay_cycles(48000);
  P1OUT |= BIT0;
  P1DIR |= BIT0;
  P1OUT &= ~BIT0;
#if 0
  while(1) {
  P1OUT |= BIT0;
  P1DIR |= BIT0;
  P1OUT &= ~BIT0;
  fxl_set(BIT_SENSE_SW);
  __delay_cycles(48000);
  __delay_cycles(48000);
  __delay_cycles(48000);
  P1OUT |= BIT1;
  P1DIR |= BIT1;
  P1OUT &= ~BIT1;
  //lsm_reset();
  accel_only_init_odr_hm(0x80, 1);
  P1OUT |= BIT1;
  P1DIR |= BIT1;
  P1OUT &= ~BIT1;
  __delay_cycles(48000);
  __delay_cycles(48000);
  P1OUT |= BIT0;
  P1DIR |= BIT0;
  P1OUT &= ~BIT0;
  fxl_clear(BIT_SENSE_SW);
  __delay_cycles(48000);
  __delay_cycles(48000);
  }
#endif
  fxl_set(BIT_SENSE_SW);
  __delay_cycles(48000);
  accel_only_init_odr_hm(0x80, 1);
}


// Reads from the accelerometer
void task_measure() {
  uint16_t x,y,z;
  for (int i = 0; i < 200; i++) {
    LOOP_ITER(200);
    P1OUT |= BIT0;
    P1DIR |= BIT0;
    P1OUT &= ~BIT0;
    accel_odr_reenable(0x80);
    P1OUT |= BIT0;
    P1DIR |= BIT0;
    P1OUT &= ~BIT0;
    for (int j = 0; j < 500; j++) {
    LOOP_ITER(500);
    //dummy_accel_read( &x, &y, &z);
    x++;
    y++;
    z++;
    if (x > 50000) {
      x = 0;
      y = 0;
      z = 0;
    }
    //if (i > 20) {
      stored_x = x;
      stored_y = y;
      stored_z = z;
    }
      //PRINTF("X: %i Y: %i Z: %i\r\n", (int)x,(int)y,(int)z);
    //}
    /*else {
      __delay_cycles(100);
    }*/
    P1OUT |= BIT1;
    P1DIR |= BIT1;
    P1OUT &= ~BIT1;
    lsm_disable();
    P1OUT |= BIT1;
    P1DIR |= BIT1;
    P1OUT &= ~BIT1;
    __delay_cycles(48000);
  }
  TRANSITION_TO(task_calc);
}

void task_calc() {
  uint16_t x,y,z;

  for (int j = 0; j < 30; j++) {
  LOOP_ITER(30);
  //dummy_accel_read( &x, &y, &z);
  x++;
  y++;
  z++;
  if (x > 50000) {
    x = 0;
    y = 0;
    z = 0;
  }
  //if (i > 20) {
    stored_x = x;
    stored_y = y;
    stored_z = z;
  }
  TRANSITION_TO(task_measure);
}

ENTRY_TASK(task_measure)
INIT_FUNC(init)

