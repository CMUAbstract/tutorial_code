/* This program tests pacarana's dataflow analysis within multiple tasks
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
TASK(task_2);
TASK(task_3);
TASK(task_4);
TASK(task_5);
TASK(task_6);
TASK(task_7);

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
    TRANSITION_TO(task_2);// Possible vals: camera--6, gyro--3,4,5, radio--1,2
  }
  else {
    TRANSITION_TO(task_3);// Possible vals: camera--6, gyro--3,4,5, radio--1,2
  }
}


void task_2() {
  STATE_CHANGE(radio,0x1);// Possible vals: camera--6, gyro--3,4,5, radio--1
  STATE_CHANGE(gyro,0x7);// Possible vals: camera--6, gyro--7 radio--1
  TRANSITION_TO(task_4);
}

void task_3() {
  STATE_CHANGE(radio,0x2);// Possible vals: camera--6, gyro--3,4,5, radio--2
  TRANSITION_TO(task_4);
}

void task_4() {
  STATE_CHANGE(gyro,0x4);// Possible vals: camera--6, gyro--4 radio--1,2
  PRINTF("Power mode changed!\r\n");
  switch(test) {
    case 0:{
      TRANSITION_TO(task_5);
      break;
    }
    case 1: {
      TRANSITION_TO(task_6);
      break;
    }
    default: {
      TRANSITION_TO(task_7);
      break;
    }
  }
}

void task_5() {
  test++; // Possible vals: camera--6, gyro--4 radio--1,2
  STATE_CHANGE(gyro,0x5);// Possible vals: camera--6, gyro--5 radio--1,2
  TRANSITION_TO(task_1);
}

void task_6() {
  test--;// Possible vals: camera--6, gyro--4 radio--1,2
  STATE_CHANGE(radio,0x1);// Possible vals: camera--6, gyro--4 radio--1
  STATE_CHANGE(camera,0x6);// Possible vals: camera--6, gyro--4 radio--1
  TRANSITION_TO(task_1);
}

void task_7() {
  STATE_CHANGE(gyro,0x3);// Possible vals: camera--6, gyro--3 radio--1,2
  test+=5;// Possible vals: camera--6, gyro--3 radio--1,2
  TRANSITION_TO(task_1);
}
