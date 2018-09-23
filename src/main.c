#include <msp430.h>

#include <libio/console.h>
#include <libmspbuiltins/builtins.h>
#include <libmsp/mem.h>
#include <libmsp/periph.h>
#include <libmsp/clock.h>
#include <libmsp/watchdog.h>
#include <libmsp/gpio.h>

#ifdef ALPACA
#include <libalpaca/alpaca.h>
#endif

#include "data.h"

static void init_hw() {
  msp_watchdog_disable();
  msp_gpio_unlock();
  msp_clock_setup();
#ifdef CONSOLE
  INIT_CONSOLE();
#endif
  __enable_interrupt();
}

#ifdef ALPACA

void init();
void task_init();
void task_compute();
void task_finish();

TASK(task_init);
TASK(task_compute);
TASK(task_finish);

ENTRY_TASK(task_init);
INIT_FUNC(init);

void init() {
  init_hw();
}

void task_init() {

}

void task_compute() {

}

void task_finish() {

}

#else

int main() {
        return 0;
}

#endif
