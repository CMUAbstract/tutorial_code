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

TASK(0, task_init);
TASK(1, task_compute);
TASK(2, task_finish);

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

volatile int a = 6;
int main() {
        init_hw();
        PRINTF("\r\n STARTING");
        PRINTF("\r\nHEY %u %u %u", a, a++, ++a);
        volatile int b = a * 6;
        PRINTF("\r\n %u %u", a, b);
        return 0;
}

#endif
