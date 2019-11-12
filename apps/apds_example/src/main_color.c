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
#include <libapds/color.h>

// Definitions supporting the alpaca language
#include <libalpaca/alpaca.h>

#include <libpacarana/pacarana.h>

#define REENABLE
void init();

TASK(task_measure);
TASK(task_calc);
TASK(task_end);

__nv uint16_t stored_r;
__nv uint16_t stored_g;
__nv uint16_t stored_b;
__nv uint16_t stored_c;
__nv uint16_t count = 0;
__nv uint16_t count2 = 0;

ENTRY_TASK(task_measure);
INIT_FUNC(init);

void init(){
	capybara_init();
  printf("capy board init'd\r\n");
  fxl_set(BIT_SENSE_SW);
  __delay_cycles(5000);
  fxl_set(BIT_APDS_SW);
  __delay_cycles(5000);
  apds_color_init();
  printf("APDS initialized\r\n");
}


// Reads from the apds color sensor
void task_measure() {
  uint16_t r,g,b,c;
#ifdef REENABLE
#ifdef DISPROF
    P1OUT |= BIT0;
    P1DIR |= BIT0;
    P1OUT &= ~BIT0;
#endif
    apds_color_reenable(0x80);
#ifdef DISPROF
    P1OUT |= BIT0;
    P1DIR |= BIT0;
    P1OUT &= ~BIT0;
#endif
#endif
  for (int i = 0; i < 10; i++) {
    apds_read_color(&r,&g,&b,&c);
    if (i > 2) {
      stored_r = r;
      stored_g = g;
      stored_b = b;
      stored_c = c;
    }
    else {
      __delay_cycles(100);
    }
    //PRINTF("R: %i G: %i B: %i Z: %i\r\n", r, g, b, c);
  }
#ifdef REENABLE
#ifdef DISPROF
    P1OUT |= BIT1;
    P1DIR |= BIT1;
    P1OUT &= ~BIT1;
#endif
    apds_color_disable();
#ifdef DISPROF
    P1OUT |= BIT1;
    P1DIR |= BIT1;
    P1OUT &= ~BIT1;
#endif
#endif
  TRANSITION_TO(task_calc);
}

void task_calc() {
    P1OUT |= BIT0;
    P1DIR |= BIT0;
    P1OUT &= ~BIT0;

  uint16_t x,y,z;
  for (int j = 0; j < 2000; j++) {
    LOOP_ITER(2000)
    x++;
    y++;
    z++;
    if (x > 50000) {
      x = 0;
      y = 0;
      z = 0;
    }
    stored_r = x;
    stored_g = y;
    stored_b = z;
  }
  if (count < 9) {
    count++;
    P1OUT |= BIT1;
    P1DIR |= BIT1;
    P1OUT &= ~BIT1;
    TRANSITION_TO(task_measure);
  }
  else {
    P1OUT |= BIT1;
    P1DIR |= BIT1;
    P1OUT &= ~BIT1;
    TRANSITION_TO(task_end);
  }
}

void task_end() {
  /*P1OUT |= BIT0;
  P1DIR |= BIT0;
  P1OUT &= ~BIT0;*/

  count = 0;
  TRANSITION_TO(task_measure);
}


