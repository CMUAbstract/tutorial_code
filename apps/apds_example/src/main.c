#include <msp430.h>

#include <libmspbuiltins/builtins.h>
#include <libmsp/clock.h>
#include <libmsp/watchdog.h>
#include <libmsp/gpio.h>
#include <libcapybara/reconfig.h>
#include <libcapybara/board.h>
#include <libapds/gesture.h>
#include <libio/console.h>
#include <libalpaca/alpaca.h>

#define zero 0
#define one 1
#define two 2
#define three 3

// Capybara power config table Format:
// CFG_ROW(<task number>, <mode type>, <operating mode>, <preburst/burst mode>)
// task number- will be used during capybara_transition calls to invoke a
//    specific row from the power table
// mode type- choose from: CONFIGD, PREBURST, BURST
// operating mode- choose from LOW, MEDLOW, MED, MEDHIGH, HIGH
// preburst/burst mode- choose from LOW, MEDLOW, MED, MEDHIGH, HIGH


capybara_task_cfg_t pwr_configs[3] = {
  CFG_ROW(0, CONFIGD, MED, NA),
  CFG_ROW(1, PREBURST, LOW, MEDHIGH),
  CFG_ROW(2, BURST, NA, MEDHIGHP),
};

void init();
void task_init();
void task_sample();
void task_gesture();

TASK(task_init);
TASK(task_sample);
TASK(task_gesture);

ENTRY_TASK(task_init);
INIT_FUNC(init);

void init(){
	capybara_init();
}

void task_init() {
	// Call capybara_transtion(<task number>) at the beginning of each task
  capybara_transition(0);
  apds_settle();
  TRANSITION_TO(task_sample);
}

void task_sample() {
  capybara_transition(1);
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

void task_gesture()
{
  capybara_transition(2);
  LOG("tsk apds\r\n");
  apds_init();
  WAIT_APDS_DELAY;
  gest_dir gesture = apds_get_gesture();
  if(gesture > DIR_NONE) {
    PRINTF("Got gesture %i\r\n",gesture);
  }
  else {
    PRINTF("No gesture found %i\r\n",gesture);
  }
  TRANSITION_TO(task_init);
}
