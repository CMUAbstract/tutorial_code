#include <msp430.h>

#include <libmspbuiltins/builtins.h>
#include <libmsp/clock.h>
#include <libmsp/watchdog.h>
#include <libmsp/gpio.h>
#include <libcapybara/reconfig.h>
#include <libcapybara/board.h>
#include <libapds/gesture.h>
#include <libio/console.h>

#ifdef ALPACA
#include <libalpaca/alpaca.h>
#endif


#ifdef ALPACA

#define zero 0
#define one 1
#define two 2
#define three 3

capybara_task_cfg_t pwr_configs[4] = {
  CFG_ROW(zero, CONFIGD, LOWP2,LOWP2),
  CFG_ROW(one, PREBURST, LOWP2,MEDHIGHP),
  CFG_ROW(two, BURST, MEDHIGHP, MEDHIGHP),
  CFG_ROW(three, CONFIGD, MEDP,MEDP),
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

void init()
{ capybara_init();
}

void task_init()
{ capybara_transition(3);
  apds_settle();
  TRANSITION_TO(task_sample);
}

void task_sample() {
  capybara_transition(1);
  int16_t proxVal = 0;
  enable_photoresistor();
  proxVal = read_photoresistor();
  if((proxVal < ALERT_THRESH) && proxVal > LOW_OUTPUT){
    disable_photoresistor();
    PRINTF("Got value %u\r\n",proxVal);
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

#else
int main() {
    capybara_init();
    while(1) {
        PRINTF("Starting no alpaca\r\n");
    }
    return 0;
}

#endif
