#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include <libmspware/driverlib.h>

#include <libalpaca/alpaca.h>

#include <libcapybara/capybara.h>
#include <libcapybara/power.h>
#include <libcapybara/reconfig.h>
#include <libcapybara/board.h>

#include <libmspbuiltins/builtins.h>
#include <libio/console.h>
#include <libmsp/mem.h>
#include <libmsp/periph.h>
#include <libmsp/clock.h>
#include <libmsp/sleep.h>
#include <libmsp/watchdog.h>
#include <libmsp/gpio.h>
//#include <libmspmath/msp-math.h>
#include <libpacarana/pacarana.h>


REGISTER(modem);
REGISTER(lps);
REGISTER(lsm);
REGISTER(temp);
REGISTER(hmc);

TASK(task_init)
TASK(task_file_modified)
TASK(task_button)
TASK(task_dispatch)
TASK(task_lps)
TASK(task_lsm)
TASK(task_temp)
TASK(task_hmc)

// Stand in interrupt for button press
void DRIVER Port_1_ISR(void) {
  // If button fires, transmit alarm and send file contents
}

DISABLE(Port_1_ISR) {
  P1IE &= ~BIT4; //disable interrupt bit
  P1IE &= ~BIT5;
  return;
}

ENABLE(Port_1_ISR) {
  P1IE |= BIT4; //enable interrupt bit
  P1IE |= BIT5;
  return;
}


void task_init() {
  // Initialize all the active sensors here

  // Init modem
  // Init lps
  // Init lsm
  // Init temp
  // Init hmc

  // In the mbed version we schedule all the sensor threads here
}

void task_file_modified() {
}

void task_button() {
}

void task_dispatch() {
}

void task_lps() {
TRANSITION_TO(task_dispatch);
}

void task_lsm() {
TRANSITION_TO(task_dispatch);
}

void task_temp() {
TRANSITION_TO(task_dispatch);
}

void task_hmc() {
TRANSITION_TO(task_dispatch);
}
