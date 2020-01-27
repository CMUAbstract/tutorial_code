// This file is for testing if our methodology for finding the break even point
// between enabling/disabling peripherals and leaving them running is correct

// Built-in libraries for the msp430
#include <msp430.h>
#include <libmspbuiltins/builtins.h>
#include <libmsp/clock.h>
#include <libmsp/watchdog.h>
#include <libmsp/gpio.h>
#include <libio/console.h>
// Functions for the capybara board
#include <libcapybara/board.h>
// Functions for the apds
#include <libapds/color.h>
#include <libapds/gesture.h>
#include <libapds/proximity.h>
// Definitions supporting the alpaca language
#include <libalpaca/alpaca.h>

#include <libpacarana/pacarana.h>
void init();

REGISTER(apds)
//#define WRITE_PROFILE
//#define READ_PROFILE
TASK(task_measure)
TASK(task_calc)
TASK(task_end)
TASK(task_profile)


#ifndef COLOR
// Make sure either color or proximity is set
#define PROX
// Note, we're not supporting gesture detection-- it's way too much of a pain to
// pin down
#endif

#ifndef WORKLOAD_CYCLES
#define WORKLOAD_CYCLES 100
#endif

#ifndef ITER_START
// This sets the starting point for the number of iterations, if we don't set
// ITER_INC then this is the constant iteration count
#warning "ITER_START UNDEFINED"
#define ITER_START 200
#endif

#ifdef ITER_INC
#warning "ITER_INC defined"
// If we've set iter inc, then the number of iterations of the loop in task_calc
// will increase by ITER_STEP after every completion of 9 runs through the
// workload (task_measure-->task_calc-->task_end)
#ifndef ITER_STEP
#warning "ITER_STEP UNDEFINED"
#define ITER_STEP 200
#endif
// We also need to know when to stop incrementing, so we define ITER_END
#ifndef ITER_END
#warning "ITER_END UNDEFINED"
#define ITER_END 1000
#endif
#endif

#if defined(DISABLE_ALL) && defined(REENABLE)
#error "DISABLE_ALL and REENABLE may not be defined at the same time"
#endif

#if defined(DISABLE_ALL) && !defined(PROF)
#error "DISABLE_ALL must be defined with PROF"
#endif

#if defined(PROF_COMPUTE) && defined(REENABLE)
#error "PROF_COMPUTE not allowed with reenable for now"
#endif

__nv uint16_t stored_r;
__nv uint16_t stored_g;
__nv uint16_t stored_b;
__nv uint16_t stored_c;
__nv unsigned count = 0;

#ifndef ITER_INC
__nv unsigned long ITER = ITER_START;
#endif
// This function runs first on every reboot

void init() {
  capybara_init();
  fxl_set(BIT_SENSE_SW);
  __delay_cycles(48000);
  fxl_set(BIT_APDS_SW);
  __delay_cycles(48000);
#if (defined(HPVLP) || defined(PROF)) && !defined(DISABLE_ALL)
#ifdef COLOR
  apds_color_init();
  uint16_t r,g,b,c;
  apds_read_color(&r,&g,&b,&c);
#else
  proximity_init();
  enableProximitySensor();
  int16_t val = readProximity();
#endif
#if defined(PROF) && !defined(HPVLP)
    // We add this in so we wind up in idle mode. That lets us actually figure
    // out the cost of enabling and disabling
    apds_color_disable();
#endif
#elif defined(READ_PROFILE)
  while(1) {
    P1OUT |= BIT0;
    P1DIR |= BIT0;
    P1OUT &= ~BIT0;
    apds_read_profile();
    __delay_cycles(16000);
  }
#elif defined(WRITE_PROFILE)
  while(1) {
    P1OUT |= BIT0;
    P1DIR |= BIT0;
    P1OUT &= ~BIT0;
    apds_write_profile();
    __delay_cycles(16000);
  }
#endif
  PRINTF("ON\r\n");
}


// Reads from the color sensor
void task_measure() {
  uint16_t r,g,b,c;
#ifdef REENABLE
#ifdef DISPROF
  P1OUT |= BIT0;
  P1DIR |= BIT0;
  P1OUT &= ~BIT0;
#endif
  //STATE_CHANGE(apds,1);
#ifdef COLOR
  apds_color_reenable();
#else
  apds_proximity_reenable();
#endif
#ifdef DISPROF
  P1OUT |= BIT0;
  P1DIR |= BIT0;
  P1OUT &= ~BIT0;
#endif
#endif
  P1OUT |= BIT1;
  P1DIR |= BIT1;
  P1OUT &= ~BIT1;
#ifdef COLOR
  for (int i = 0; i < 3; i++) {
#else
  for (int i = 0; i < 3; i++) {
#endif
    //STATE_CHANGE(apds,1);
#ifdef COLOR
    apds_dummy_read_color(&r,&g,&b,&c);
#else
    uint8_t val = readProximity();
#endif
    __delay_cycles(2400);
    //PRINTF("Val = %u\r\n",val);
    if (i > 2) {
      stored_r = r;
      stored_g = g;
      stored_b = b;
      stored_c = c;
    }
    else {
      __delay_cycles(100);
    }
  }
	// Adding a 2ms delay to force a settle
	for (int i = 0; i < 800; i++) {}
  P1OUT |= BIT2;
  P1DIR |= BIT2;
  P1OUT &= ~BIT2;
#ifdef REENABLE
#ifdef DISPROF
  P1OUT |= BIT1;
  P1DIR |= BIT1;
  P1OUT &= ~BIT1;
#endif
    //STATE_CHANGE(apds,0);
#ifdef COLOR
    apds_color_disable();
#else
    apds_proximity_disable();
#endif
#ifdef DISPROF
  P1OUT |= BIT1;
  P1DIR |= BIT1;
  P1OUT &= ~BIT1;
#endif
#endif
  TRANSITION_TO(task_calc);
}

void task_calc() {
  uint16_t x,y,z;
  for (unsigned long j = 0; j < ITER; j++) {
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
  if (count < WORKLOAD_CYCLES) {
    count++;
    TRANSITION_TO(task_measure);
  }
  else {
    TRANSITION_TO(task_end);
  }
}

void task_end() {
  P1OUT |= BIT0;
  P1DIR |= BIT0;
  P1OUT &= ~BIT0;
  count = 0;
#ifdef ITER_INC
  PRINTF("iter start: %u, iter step %u, iter end %u\r\n",
    ITER_START,ITER_STEP,ITER_END);
  ITER += ITER_STEP;
  if (ITER > ITER_END) {
    ITER=ITER_START;
  }
#endif
  TRANSITION_TO(task_measure);
}

void task_profile() {
  P1OUT |= BIT0;
  P1DIR |= BIT0;
  P1OUT &= ~BIT0;
#ifndef REENABLE
#ifndef PROF_COMPUTE
  for (int i = 0; i <  50; i++) {
    __delay_cycles(1000);
  }
#else
  uint16_t x,y,z;
  for (int j = 0; j < ITER; j++) {
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
#endif
#else
  // Second phase
#ifdef COLOR
  apds_color_reenable();
#else
  apds_proximity_reenable();
#endif
  __delay_cycles(100);
  // It's just the same disable for all of them, don't worry about it.
  apds_proximity_disable();
  for (int i = 0; i <  50; i++) {
    __delay_cycles(1000);
  }
#endif
  TRANSITION_TO(task_profile);
}

#ifndef PROF
ENTRY_TASK(task_measure)
#else
ENTRY_TASK(task_profile)
#endif
INIT_FUNC(init)

