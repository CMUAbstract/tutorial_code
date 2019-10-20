#include <msp430.h>

// Built-in libraries for the msp430
#include <libcapybara/reconfig.h>
#include <libcapybara/board.h>
#include <libradio/radio.h>
#include <libalpaca/alpaca.h>
#include <msp430.h>
#include <libmspbuiltins/builtins.h>
#include <libmsp/clock.h>
#include <libmsp/watchdog.h>
#include <libmsp/gpio.h>
#include <libio/console.h>
#include <libcapybara/reconfig.h>

// Functions for the capybara board
#include <libcapybara/board.h>

// Functions for the accelerometer
#include <liblsm/accl.h>

// Functions for the color sensor
#include <libapds/color.h>

// Definitions supporting the alpaca language
#include <libalpaca/alpaca.h>

#define zero 0
#define one 1
#define two 2
#define three 3
#define four 4

void init();

int colorCheck(uint16_t c);

TASK(task_measure)
TASK(task_color)
TASK(task_sendMove)
TASK(task_sendColor)

TASK_SHARED(int, color);

capybara_task_cfg_t pwr_configs[4] = {
   CFG_ROW(zero, CONFIGD, LOWP2,LOWP2),
   CFG_ROW(one, PREBURST, LOWP2,MEDHIGHP),
   CFG_ROW(two, BURST, MEDHIGHP, MEDHIGHP),
   CFG_ROW(three, CONFIGD, MEDP,MEDP),
};

// This function runs first on every reboot
void init() {
  capybara_init();
  fxl_set(BIT_SENSE_SW);
  __delay_cycles(16000);
	fxl_set(BIT_APDS_SW);
	__delay_cycles(16000);
	apds_color_init();
  lsm_reset();
  accelerometer_init();
}

int colorCheck(uint16_t c)
{
	if (c < 23) { // black
			return 8;
	}

	else if (c > 23 && c < 45) {
			return 7;
	}

	else if ((c > 45) && (c < 75)) {
			return 6;
	}

  else if ((c > 75) && (c < 105)) {
	      return 5;
	}

	else if ((c > 105) && (c < 135)) {
			  return 4;
	}

	else if ((c > 135) && (c < 165)) {
			 return 3;
	}

	else if ((c > 165) && (c < 195)){
			 return 2;
	}

	else if ((c > 195) && (c < 225)){
			 return 1;
	}

	else if (c > 225) { // white
	  	return 0;
	}

	return -1;
}


void task_color() {
  uint16_t r,g,b,c;
  int color_old=-1;
  int color_new;


  //Throw out first 20 values
  for (int i = 0; i < 20; i++)
  {
    apds_read_color(&r,&g,&b,&c);
    color_old = colorCheck(c);
  }

  for (int i = 0; i < 20; i++)
  {
    apds_read_color(&r,&g,&b,&c);
    color_new = colorCheck(c);

    //if different restart
    if(color_old != color_new)
		{
			TRANSITION_TO(task_color);
		}
	}

  __delay_cycles(0xffff);
  TS(color) = color_old;
  TRANSITION_TO(task_sendColor);

}

ENTRY_TASK(task_measure)
INIT_FUNC(init)


// Reads from the accelerometer
void task_measure() {
	int x,y,z;
  int x_old, y_old, z_old;

	//Throw out first 50 values
	for ( int i = 0; i < 100; i++)
	{
		__delay_cycles(0x100);
		accelerometer_read(&x_old, &y_old, &z_old);
	}

	__delay_cycles(0xffff);
	__delay_cycles(0xffff);
	__delay_cycles(0xffff);

		accelerometer_read( &x, &y, &z);

	//Threshold = 200, may need to update
    if((abs(x-x_old) > 350) && (abs(y-y_old) > 350) && (abs(z-z_old) > 350))
		{
			PRINTF("MOVING!!!!!\r\n");
			//Movement sensed - send -1 to notify user
			TRANSITION_TO(task_sendMove);
		}
		__delay_cycles(0xffff);
		__delay_cycles(0xffff);

	//Movement not sensed - keep sensing
  TRANSITION_TO(task_measure);
}


void task_sendColor() {
      // Reconfigure bank
      capybara_transition(3);
      // Send header. Our header is 0xAA1234
      radio_buff[0] = 0xAA;
      radio_buff[1] = 0x12;
      radio_buff[2] = 0x34;
      PRINTF("Sending\r\n");
      // Send data. I'll just send 0x01
      for(int i = 3; i < LIBRADIO_BUFF_LEN; i++) {
          radio_buff[i] = TS(color);
      }
      // Send it!
      radio_send();
      PRINTF("Sent\r\n");

			__delay_cycles(0xffff);
			//Move finished, info sent - get ready for next move
      TRANSITION_TO(task_measure);
}

 void task_sendMove() {
       // Reconfigure bank
       capybara_transition(3);
       // Send header. Our header is 0xAA1234
       radio_buff[0] = 0xAA;
       radio_buff[1] = 0x12;
       radio_buff[2] = 0x34;
       PRINTF("Sending\r\n");
       // Send data. I'll just send 0x01
       for(int i = 3; i < LIBRADIO_BUFF_LEN; i++) {
           radio_buff[i] = -1;
       }
       // Send it!
       radio_send();
       PRINTF("Sent\r\n");

       __delay_cycles(0xffff);
       //Move finished, info sent - get ready for next move
      TRANSITION_TO(task_color);

}

