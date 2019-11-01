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

// Add apds stuff in here too
#include <libapds/proximity.h>
#include <libapds/color.h>

// Functions for pressure sensor
#include <liblps/pressure.h>
#define TEST2
void init();

TASK(task_measure)

// This function runs first on every reboot
void init() {
  capybara_init();
    printf("sTART!\r\n");
  fxl_set(BIT_SENSE_SW);
  __delay_cycles(80000);

  __delay_cycles(48000);
  // Initializes gyro and accelerometer in lp at 52 hz
#ifdef TEST1
  for(int i =0; i < 300;i++) {
  gyro_init_data_rate_hm(0x30, 0); //
  P1OUT |= BIT0;
  P1DIR |= BIT0;
  P1OUT &= ~BIT0;
  }
  // Initializes gyro and accelerometer in normal mode at 208 hz
  for(int i =0; i < 300;i++) {
  gyro_init_data_rate_hm(0x50, 0);
  P1OUT |= BIT1;
  P1DIR |= BIT1;
  P1OUT &= ~BIT1;
  }
  // Initializes gyro and accelerometer in hp at 1.6K hz
  for(int i =0; i < 300;i++) {
  gyro_init_data_rate_hm(0x80, 1);
  P1OUT |= BIT0;
  P1DIR |= BIT0;
  P1OUT &= ~BIT0;
  }
  // Initializes gyro in lp at 52 hz
  for(int i =0; i < 300;i++) {
  gyro_only_init_odr_hm(0x30, 0);
  P1OUT |= BIT1;
  P1DIR |= BIT1;
  P1OUT &= ~BIT1;
  }
  //// Initializes gyro in normal at 208 hz
  for(int i =0; i < 300;i++) {
  gyro_only_init_odr_hm(0x50, 0);
  P1OUT |= BIT0;
  P1DIR |= BIT0;
  P1OUT &= ~BIT0;
  }
  //// Initializes gyro in hp at 1.6khz
  for(int i =0; i < 300;i++) {
  gyro_only_init_odr_hm(0x80, 1);
  P1OUT |= BIT1;
  P1DIR |= BIT1;
  P1OUT &= ~BIT1;
  }
  //// Initializes accel in lp at 52 hz
  for(int i =0; i < 300;i++) {
  accel_only_init_odr_hm(0x30, 0);
  P1OUT |= BIT0;
  P1DIR |= BIT0;
  P1OUT &= ~BIT0;
  }
  //// Initializes accel in normal at 208 hz
  for(int i =0; i < 300;i++) {
  accel_only_init_odr_hm(0x50, 0);
  P1OUT |= BIT1;
  P1DIR |= BIT1;
  P1OUT &= ~BIT1;
  }
  //// Initializes accel in hp at 1.6Khz
  for(int i =0; i < 300;i++) {
  accel_only_init_odr_hm(0x80, 1);
  P1OUT |= BIT0;
  P1DIR |= BIT0;
  P1OUT &= ~BIT0;
  }
#endif
#ifdef TEST2
  // Initializes color sensor on apds
  for(int i =0; i < 300;i++) {
  apds_color_init();
  P1OUT |= BIT0;
  P1DIR |= BIT0;
  P1OUT &= ~BIT0;
  }
  // Initialize proximity sensor on apds with 12.5mA led drive
  for(int i =0; i < 300;i++) {
  enableProximitySensor_ldrive(LED_DRIVE_12_5MA);
  P1OUT |= BIT1;
  P1DIR |= BIT1;
  P1OUT &= ~BIT1;
  }
  // Initialize proximity sensor on apds with 25mA led drive
  for(int i =0; i < 300;i++) {
  enableProximitySensor_ldrive(LED_DRIVE_25MA);
  P1OUT |= BIT0;
  P1DIR |= BIT0;
  P1OUT &= ~BIT0;
  }
  // Set up the apds and enable gesture detection with 12.5mA drive and 150 boost
  for(int i =0; i < 300;i++) {
  proximity_init_ldrive(LED_DRIVE_12_5MA);
  enableGesture_boost(LED_BOOST_150);
  P1OUT |= BIT1;
  P1DIR |= BIT1;
  P1OUT &= ~BIT1;
  }
  // Set up the apds and enable gesture detection with 25mA drive and 150 boost
  for(int i =0; i < 300;i++) {
  proximity_init_ldrive(LED_DRIVE_25MA);
  enableGesture_boost(LED_BOOST_150);
  P1OUT |= BIT0;
  P1DIR |= BIT0;
  P1OUT &= ~BIT0;
  }
#endif
#ifdef TEST1
  for(int i =0; i < 300;i++) {
  // Initializes lps sensor in bypass mode
  pressure_bypass();
  P1OUT |= BIT1;
  P1DIR |= BIT1;
  P1OUT &= ~BIT1;
  }
  // Initializes lps sensor in stream mode
  for(int i =0; i < 300;i++) {
  pressure_stream();
  P1OUT |= BIT0;
  P1DIR |= BIT0;
  P1OUT &= ~BIT0;
  }
  // Initializes lps sensor in one shot mode
  for(int i =0; i < 300;i++) {
  pressure_init();
  P1OUT |= BIT1;
  P1DIR |= BIT1;
  P1OUT &= ~BIT1;
  }
#endif
  printf("Done\r\n");
  while(1);
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

