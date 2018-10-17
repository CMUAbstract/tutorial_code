#include <msp430.h>
#include <stdlib.h>

#include <libio/console.h>
#include <libmspbuiltins/builtins.h>
#include <libmsp/mem.h>
#include <libmsp/periph.h>
#include <libmsp/clock.h>
#include <libmsp/watchdog.h>
#include <libmsp/gpio.h>
#include <libfixed/fixed.h>
#include <libalpaca/alpaca.h>
#include <libcapybara/reconfig.h>
#include <libcapybara/board.h>
#include <libapds/gesture.h>
#include <libradio/radio.h>



// Capybara power config table Format:
// CFG_ROW(<task number>, <mode type>, <operating mode>, <preburst/burst mode>)
// task number- will be used during capybara_transition calls to invoke a
//    specific row from the power table
// mode type- choose from: CONFIGD, PREBURST, BURST
// operating mode- choose from LOW, MEDLOW, MED, MEDHIGH, HIGH
// preburst/burst mode- choose from LOW, MEDLOW, MED, MEDHIGH, HIGH
capybara_task_cfg_t pwr_configs[4] = {
};

void init();
void task_sample();
void task_gesture();
void task_send();
void task_delay();


TASK(task_init);
TASK(task_sample);
TASK(task_gesture);
TASK(task_send);
TASK(task_delay);

ENTRY_TASK(task_init);
INIT_FUNC(init);

TASK_SHARED(gest_dir, gesture);

void init() {
	capybara_init();
}

// Requires a MEDHIGH energy mode
void task_init() {
	apds_settle();
	TRANSITION_TO(task_sample);
}

// Requires a LOW energy mode
void task_sample() {
  int16_t light = 0;
	enable_photoresistor();
  light = read_photoresistor();
  if((light < CLOSE_OBJECT) && light > LOW_OUTPUT){
    disable_photoresistor();
    PRINTF("Got value %u\r\n",light);
    TRANSITION_TO(task_gesture);
  }
  WAIT_PHOTORESISTOR_DELAY;
  TRANSITION_TO(task_sample);
}

// Requires a HIGH energy mode
void task_gesture() {
  LOG("tsk apds\r\n");
  apds_init();
  WAIT_APDS_DELAY;
  TS(gesture) = apds_get_gesture();
  if(TS(gesture) < DIR_NONE) {
    PRINTF("No gesture %i\r\n",TS(gesture));
		TRANSITION_TO(task_delay);
  }
  else {
    PRINTF("Got gesture code %u\r\n",TS(gesture));
		TRANSITION_TO(task_send);
  }
}

// Requires a MEDHIGH energy mode
void task_send() {
	// Send header. Our header is 0xAA1234
  radio_buff[0] = 0xAA;
  radio_buff[1] = 0x12;
  radio_buff[2] = 0x34;
  // Send data. We'll just repeat the gesture code
	for(int i = 3; i < LIBRADIO_BUFF_LEN; i++) {
      radio_buff[i] = TS(gesture);
  }
  // Send it!
	radio_send();
	TRANSITION_TO(task_delay);
}

// This task is just included so the loop is stable on continuous power
// No need to add a capybara annotation
void task_delay() {
#ifdef LIBCAPYBARA_CONT_POWER
	for (unsigned i = 0; i < 100; ++i) {
		__delay_cycles(4000);
  }
#endif
	TRANSITION_TO(task_init);
}

