// Main file for testing the initialization energy of different peripherals. It
// also is a decent representative for the cost of i2c while  you're in a
// certain mode, since we're just doing the init on loop.

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
#define TEST1
void init();

TASK(task_measure)
#ifdef TEST1
__nv uint8_t count = 0;
#endif
#ifdef TEST2
__nv uint8_t count = 9;
#endif
#ifdef TEST3
__nv uint8_t count = 14;
#endif
// This function runs first on every reboot
void init() {
  capybara_init();
  count++;
  #ifdef TEST1
  if (count > 9) {
    count = 1;
  }
  #endif
#ifdef TEST2
  if (count > 14) {
    count = 10;
  }
#endif
#ifdef TEST3
  if (count > 17) {
    count = 15;
  }
#endif
    printf("sTART!, %i\r\n",count);
#ifdef TEST1
  fxl_set(BIT_SENSE_SW);
#endif
#ifdef TEST2
  fxl_set(BIT_SENSE_SW | BIT_APDS_SW);
#endif
#ifdef TEST3
  fxl_set(BIT_SENSE_SW);
#endif
  __delay_cycles(80000);

  __delay_cycles(48000);
  // Initializes gyro and accelerometer in lp at 52 hz
#ifdef TEST1
  switch(count) {
    case(1):
      while(1) {
      gyro_init_data_rate_hm(0x30, 0); //
      }
    case(2):
      while(1) {
      // Initializes gyro and accelerometer in normal mode at 208 hz
      gyro_init_data_rate_hm(0x50, 0);
      }
    case(3):
      while(1) {
      // Initializes gyro and accelerometer in hp at 1.6K hz
      gyro_init_data_rate_hm(0x80, 1);
      }
    case(4):
      while(1) {
      // Initializes gyro in lp at 52 hz
      gyro_only_init_odr_hm(0x30, 0);
      }
    case(5):
      while(1) {
      //// Initializes gyro in normal at 208 hz
      gyro_only_init_odr_hm(0x50, 0);
      }
    case(6):
      while(1) {
      //// Initializes gyro in hp at 1.6khz
      gyro_only_init_odr_hm(0x80, 1);
      }
    case(7):
      while(1) {
      //// Initializes accel in lp at 52 hz
      accel_only_init_odr_hm(0x30, 0);
      }
    case(8):
      while(1) {
      //// Initializes accel in normal at 208 hz
      accel_only_init_odr_hm(0x50, 0);
      }
    case(9):
      while(1) {
      //// Initializes accel in hp at 1.6Khz
      accel_only_init_odr_hm(0x80, 1);
      }
    default:
      PRINTF("Error!\r\n");
      break;
  }
  while(1);
#endif

#ifdef TEST2
  switch(count) {
    case(10):
      while(1) {
      // Initializes color sensor on apds
      apds_color_init();
      }
    case(11):
      while(1) {
      // Initialize proximity sensor on apds with 12.5mA led drive
      enableProximitySensor_ldrive(LED_DRIVE_12_5MA);
      }
    case(12):
      while(1) {
      // Initialize proximity sensor on apds with 25mA led drive
      enableProximitySensor_ldrive(LED_DRIVE_25MA);
      }
    case(13):
      while(1) {
      // Set up the apds and enable gesture detection with 12.5mA drive and 150 boost
      proximity_init_ldrive(LED_DRIVE_12_5MA);
      enableGesture_boost(LED_BOOST_150);
      }
    case(14):
      while(1) {
      // Set up the apds and enable gesture detection with 25mA drive and 150 boost
      proximity_init_ldrive(LED_DRIVE_25MA);
      enableGesture_boost(LED_BOOST_150);
      }
    default:
      PRINTF("Error!\r\n");
      break;
    }
    while(1);
#endif
#ifdef TEST3
  switch(count) {
    case(15):
      while(1) {
      // Initializes lps sensor in bypass mode
      pressure_bypass();
      }
    case(16):
      while(1) {
      // Initializes lps sensor in stream mode
      pressure_stream();
      }
    case(17):
      while(1) {
      // Initializes lps sensor in one shot mode
      pressure_init();
      }
    default:
      PRINTF("eRROR!\r\n");
      break;
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


