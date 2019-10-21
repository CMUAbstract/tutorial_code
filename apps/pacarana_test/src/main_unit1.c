/* This program tests pacarana's dataflow analysis within a single task.
*/
#include <msp430.h>

#include <libmspbuiltins/builtins.h>
#include <libmsp/clock.h>
#include <libmsp/watchdog.h>
#include <libmsp/gpio.h>
#include <libio/console.h>
#include <libcapybara/board.h>
#include <libalpaca/alpaca.h>

#include <libpacarana/pacarana.h>


#define zero 0
#define one 1
#define two 2
#define three 3
#define four 4

REGISTER(radio);
REGISTER(gyro);
REGISTER(camera);

void init();

TASK(task_1);

ENTRY_TASK(task_1);
INIT_FUNC(init);

void init() {
    capybara_init();
}

__nv int test = 0;


void task_1() {
  test = test + 5; // Possible vals: camera--6, gyro--3,4,5, radio--1,2
  STATE_CHANGE(camera,0x6);// Possible vals: camera--6, gyro--3,4,5, radio--1,2
  if(test > 10) {// Possible vals: camera--6, gyro--3,4,5, radio--1,2
    STATE_CHANGE(radio,0x1);// Possible vals: camera--6, gyro--3,4,5, radio--1
    STATE_CHANGE(gyro,0x7);// Possible vals: camera--6, gyro--7 radio--1
  }
  else {
    STATE_CHANGE(radio,0x2);// Possible vals: camera--6, gyro--3,4,5, radio--2
  }
  STATE_CHANGE(gyro,0x4);// Possible vals: camera--6, gyro--4 radio--1,2
  PRINTF("Power mode changed!\r\n");
  switch(test) {
    case 0:
      test++; // Possible vals: camera--6, gyro--4 radio--1,2
      STATE_CHANGE(gyro,0x5);// Possible vals: camera--6, gyro--5 radio--1,2
      break;
    case 1:
      test--;// Possible vals: camera--6, gyro--4 radio--1,2
      STATE_CHANGE(radio,0x1);// Possible vals: camera--6, gyro--4 radio--1
      STATE_CHANGE(camera,0x6);// Possible vals: camera--6, gyro--4 radio--1
      break;
    default:
      STATE_CHANGE(gyro,0x3);// Possible vals: camera--6, gyro--4 radio--1,2
      test+=5;// Possible vals: camera--6, gyro--3 radio--1,2
      break;
  }
  TRANSITION_TO(task_1);// Possible vals: camera--6, gyro--3,4,5 radio--1,2
}

