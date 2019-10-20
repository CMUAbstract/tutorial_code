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
//TASK(task_send);

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

	//if ((r > 205) && (g < 50) && (b < 50) && (c < 175)) {
	//	PRINTF("0");
	//}
	//else if ((r < 50) && (g < 50) && (b < 50) && (c < 50)) {
	//	PRINTF("1");
	//}
	//else if ((r > 205) && (g < 50) && (b > 205) && (c < 175)) {
	//	PRINTF("2");
	//}
	//else if ((r > 100) && (g > 100) && (b > 100) && (c < 175)){
	//	PRINTF("3");
	//}
	//else if ((r < 50) && (g < 50) && (b > 205) && (c < 175)) {
	//	PRINTF("4");
	//}
	//else if ((r > 205) && (g > 205) && (b > 205) && (c > 205)) {
	//	PRINTF("5");
	//}
	//else if ((r > 205) && (g > 205) && (b < 50) && (c < 175)) {
	//	PRINTF("6");
	//}
	//else if ((r < 50) && (g > 205) && (b > 205) && (c < 175)) {
	//	PRINTF("7");
	//}
	//else if ((r < 50) && (g > 205) && (b < 50) && (c < 175)){
	//	PRINTF("8");
	//}

	if (c < 23) { // black
			PRINTF("8");
			TRANSITON_TO(task_send(int 8));
	}

	else if (c > 23 && c < 45) {
			PRINTF("7");
			TRANSITION_TO(task_send(int 7));
	}

	else if ((c > 45) && (c < 75)) {
			PRINTF("6");
			TRANSITION_TO(task_send(int 6));
	}

  else if ((c > 75) && (c < 105)) {
	      PRINTF("5");
				TRANSITION_TO(task_send(int 5));
	}

	else if ((c > 105) && (c < 135)) {
			PRINTF("4");
			TRANSITION_TO(task_send(int 4));
	}

	else if ((c > 135) && (c < 165)) {
			PRINTF("3");
			TRANSITION_TO(task_send(int 3));
	}


	else if ((c > 165) && (c < 195)){
			PRINTF("2");
			TRANSITION_TO(task_send(int 2));
	}

	else if ((c > 195) && (c < 225)){
			PRINTF("1");
			TRANSITION_TO(task_send(int 1));
	}

	else if (c > 225) { // white
			PRINTF("0");
			TRANSITION_TO(task_send(int 0));
	}

  PRINTF("R: %i G: %i B: %i Z: %i\r\n", r, g, b, c);

  TRANSITION_TO(task_measure); // keep measuring colors!
}

//void task_send(int color) {
			// Reconfigure bank
//      capybara_transition(3);
      // Send header. Our header is 0xAA1234
//	    radio_buff[0] = 0xAA;
//	    radio_buff[1] = 0x12;
//		  radio_buff[2] = 0x34;
//		  PRINTF("Sending\r\n");
			// Send data. I'll just send 0x01
//			for(int i = 3; i < LIBRADIO_BUFF_LEN; i++) {
//		        radio_buff[i] = color;
//			}
			// Send it
//			radio_send();
//			PRINTF("Sent\r\n");
//			TRANSITION_TO(task_measure);


