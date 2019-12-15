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
// Functions for the accelerometer
#include <liblsm/accl.h>
#include <liblsm/gyro.h>
// Definitions supporting the alpaca language
#include <libalpaca/alpaca.h>

#include <libpacarana/pacarana.h>
void init();

#define HIGHPERF_MASK 0b10000000

//#define WRITE_PROFILE
//#define READ_PROFILE
//#define DISPROF
TASK(task_measure)
TASK(task_calc)
TASK(task_end)
TASK(task_profile)

#ifndef WORKLOAD_CYCLES
#define WORKLOAD_CYCLES 100
#endif

#ifndef RATE
// Valid rate definitions are 0x80, 0x40, 0x10 (based on what we have datasheet
// numbers for)
#warning "RATE UNDEFINED!!!!"
#define RATE 0x80
#endif

#ifndef ACCEL
// JUst make sure either ACCEL (for accelerometer) or G_XL (gyro and
// accelerometer) is defined so we run some functions
#define G_XL
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

__nv int stored_x;
__nv int stored_y;
__nv int stored_z;
__nv unsigned count = 0;
__nv unsigned ITER = ITER_START;
// This function runs first on every reboot
void init() {
  capybara_init();
  __delay_cycles(48000);
    P1OUT |= BIT1;
    P1DIR |= BIT1;
    P1OUT &= ~BIT1;
  fxl_set(BIT_SENSE_SW);
    P1OUT |= BIT1;
    P1DIR |= BIT1;
    P1OUT &= ~BIT1;
  __delay_cycles(48000);
    P1OUT |= BIT1;
    P1DIR |= BIT1;
    P1OUT &= ~BIT1;
  __delay_cycles(48000);
    P1OUT |= BIT1;
    P1DIR |= BIT1;
    P1OUT &= ~BIT1;
#if (defined(HPVLP) || defined(PROF)) && !defined(DISABLE_ALL)
#ifdef ACCEL
  int temp = 1;
  while(temp) {
    fxl_clear(BIT_SENSE_SW);
    __delay_cycles(48000);
    fxl_set(BIT_SENSE_SW);
    temp = accel_only_init_odr_hm(RATE, RATE & HIGHPERF_MASK);
  }
#else
  gyro_init_data_rate_hm(RATE,RATE & HIGHPERF_MASK);
#endif
#if defined(PROF) && !defined(HPVLP)
    // We add this in so we wind up in sleep mode. That lets us actually figure
    // out the cost of enabling and disabling
  lsm_accel_disable();
#ifndef ACCEL
  #warning "Dissabling gyro"
  lsm_accel_disable();
  lsm_gyro_sleep();
#endif
#endif
#elif defined(READ_PROFILE)
  while(1) {
    P1OUT |= BIT0;
    P1DIR |= BIT0;
    P1OUT &= ~BIT0;
    accelerometer_read_profile();
    __delay_cycles(16000);
  }
#elif defined(WRITE_PROFILE)
  while(1) {
    P1OUT |= BIT0;
    P1DIR |= BIT0;
    P1OUT &= ~BIT0;
    accelerometer_write_profile();
    __delay_cycles(16000);
  }
#endif
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
#ifdef ACCEL
  accel_odr_reenable(RATE);
  #if RATE == 0x80
  // We need this delay to simulate throwing out the first two results
  __delay_cycles(4800);
  #endif
#else
  accel_odr_reenable(RATE);
  lsm_gyro_reenable();
  // We need this delay to simulate throwing out the first two results
  #if RATE == 0x80
  #warning "Rate is 80!"
  __delay_cycles(19200);
  #elif RATE == 0x40
#warning "Rate is 40!"
  __delay_cycles(153800);
  #endif
  /*for (int i = 0; i < 10 ; i++) {
    __delay_cycles(64000);
  }*/
#endif
#ifdef DISPROF
  P1OUT |= BIT0;
  P1DIR |= BIT0;
  P1OUT &= ~BIT0;
#endif
#endif
  for (int i = 0; i < 10; i++) {
    // We just read from the accelerometer becase G_XL mode turns on both the
    // gyro and the accelerometer anyway
//#ifdef ACCEL
#ifdef ACCEL
    dummy_accel_read( &x, &y, &z);
#else
    read_raw_gyro(&x,&y,&z);
#endif
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
  // We don't have an ifdef here because this disable is ham-handed and shuts
  // off everything.
#ifdef ACCEL
  lsm_accel_disable();
#else
  lsm_accel_disable();
  lsm_gyro_sleep();
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
  for (int i = 0; i <  50; i++) {
    __delay_cycles(1000);
  }
#if 0
  for (int i = 0; i < 10; i++) {
  printf("in loop\r\n");
  uint16_t x,y,z;
#ifdef ACCEL
  dummy_accel_read(&x,&y,&z);
#else
    read_raw_gyro(&x,&y,&z);
#endif
    printf("%i %i %i\r\n",x,y,z);
  }
#endif
#endif
#ifdef REENABLE
  // Second phase
#ifdef ACCEL
  accel_odr_reenable(RATE);
  #if RATE == 0x80
  // We need this delay to simulate throwing out the first two results
  __delay_cycles(4800);
  #endif
#else
  accel_odr_reenable(RATE);
  lsm_gyro_reenable();
  #if RATE == 0x80
  __delay_cycles(19200);
  #elif RATE == 0x40
  __delay_cycles(153800);
  #else
  #error "GYRO RATE IS NOT ALLOWED. CHOOSE 0x80 or 0x40"
  #endif
#endif
  __delay_cycles(100);
#if 0
  for (int i = 0; i < 10; i++) {
  printf("in loop\r\n");
  uint16_t x,y,z;
#ifdef ACCEL
  dummy_accel_read(&x,&y,&z);
#else
    read_raw_gyro(&x,&y,&z);
#endif
    printf("%i %i %i\r\n",x,y,z);
  }
#endif
#ifdef ACCEL
  lsm_accel_disable();
#else
  lsm_accel_disable();
  lsm_gyro_sleep();
#endif
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

