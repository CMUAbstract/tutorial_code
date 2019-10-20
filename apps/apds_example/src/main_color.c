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

void init();

TASK(task_measure);

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

void task_measure() {
  uint16_t r,g,b,c;
  apds_read_color(&r,&g,&b,&c);
  if (r > g && r > b) {
    PRINTF("RED --");
  }
  else if (g > r && g > b) {
    PRINTF("GREEN --");
  }
  else {
    PRINTF("BLUE --");
  }

  PRINTF("R: %i G: %i B: %i Z: %i\r\n", r, g, b, c);

  TRANSITION_TO(task_measure);
}

