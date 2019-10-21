/* This program tests pacarana's dataflow analysis with multiple tasks,
 * interrupts and disable/enable calls
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

__nv int test = 0;

void DRIVER Port_1_ISR(void) {
  STATE_CHANGE(camera,0x10);
  if (test < 15) {
    STATE_CHANGE(gyro,0x11);
  }
  return;
}

void DRIVER Port_2_ISR(void) {
  STATE_CHANGE(radio,0x12);
  for(int i = 0; i < 10; i++) {
    STATE_CHANGE(gyro,0x13);
  }
  return;
}

void DISABLE(Port_1_ISR) {
  P1IE &= ~BIT4; //disable interrupt bit
  P1IE &= ~BIT5;
  return;
}

void ENABLE(Port_1_ISR) {
  P1IE |= BIT4; //enable interrupt bit
  P1IE |= BIT5;
  return;
}

void DISABLE(Port_2_ISR) {
  P2IE &= ~BIT4; //disable interrupt bit
  P2IE &= ~BIT5;
  return;
}

void ENABLE(Port_2_ISR) {
  P2IE |= BIT4; //enable interrupt bit
  P2IE |= BIT5;
  return;
}

void init() {
    capybara_init();
}



void task_1() {
  test = test + 5; // Possible vals: camera--6 16, gyro--3,4,5,17,19,radio--1,2,18
  STATE_CHANGE(camera,0x6);// Possible vals: camera--6, gyro--3,4,5,17,19,radio--1,2,18
  DISABLE(Port_1_ISR);
  DISABLE(Port_2_ISR);
  if(test > 10) {// Possible vals: camera--6,16, gyro--3,4,5,17,19 radio--1,2,18
    TRANSITION_TO(task_2);// Possible vals: camera--6,16, gyro--3,4,5,17,19 radio--1,2,18
  }
  else {
    TRANSITION_TO(task_3);// Possible vals: camera--6,16, gyro--3,4,5,17,19 radio--1,2,18
  }
}
//TODO finish annotating what the values should be. The output is reliably
//adding the interrupt effects in.

void task_2() {
  STATE_CHANGE(radio,0x1);// Possible vals: camera--6,16, gyro--3,4,5,17,19 radio--1
  DISABLE(Port_2_ISR);
  STATE_CHANGE(gyro,0x7);// Possible vals: camera--6,16, gyro--7 radio--1,18
  TRANSITION_TO(task_4);// Possible vals: camera--6,16, gyro--7,17 radio--1,18
}

void task_3() {
  DISABLE(Port_1_ISR);
  STATE_CHANGE(radio,0x2);// Possible vals: camera--6,16 gyro--3,4,5,17,19 radio--2
  ENABLE(Port_1_ISR);// Possible vals: camera--6,16 gyro--3,4,5,17,19 radio--2,18
  TRANSITION_TO(task_4);
}

void task_4() {
  DISABLE(Port_2_ISR);
  STATE_CHANGE(gyro,0x4);// Possible vals: camera--6,16 gyro--4 radio--1,2,18
  PRINTF("Power mode changed!\r\n");// Possible vals: camera--6,16 gyro--4,17 radio--1,2,18
  switch(test) {
    case 0:{
      ENABLE(Port_1_ISR);
      TRANSITION_TO(task_5);// Possible vals: camera--6,16 gyro--4,17 radio--1,2,18
      break;
    }
    case 1: {
      TRANSITION_TO(task_6);// Possible vals: camera--6,16 gyro--4,17 radio--1,2,18
      break;
    }
    default: {
      ENABLE(Port_2_ISR);
      TRANSITION_TO(task_7);// Possible vals: camera--6,16 gyro--4,17,19 radio--1,2,18
      break;
    }
  }
}

void task_5() {
  test++; // Possible vals: camera--6,16  gyro--4,17,19 radio--1,2,18
  STATE_CHANGE(gyro,0x5);// Possible vals: camera--6,16 gyro--5 radio--1,2,18
  TRANSITION_TO(task_1);
}

void task_6() {
  test--;// Possible vals: camera--6,16, gyro--4,17,19 radio--1,2,18
  DISABLE(Port_1_ISR);
  ENABLE(Port_2_ISR);
  STATE_CHANGE(radio,0x1);// Possible vals: camera--6,16 gyro--4,17,19 radio--1
  STATE_CHANGE(camera,0x6);// Possible vals: camera--6 gyro--4,17,19 radio--1,18
  DISABLE(Port_2_ISR);// Possible vals: camera--6 gyro--4,17,19 radio--1,18
  ENABLE(Port_1_ISR);
  TRANSITION_TO(task_1);// Possible vals: camera--6,16 gyro--4,17,19 radio--1,18
}

void task_7() {
  STATE_CHANGE(gyro,0x3);// Possible vals: camera--6,16, gyro--3 radio--1,2,18
  test+=5;// Possible vals: camera--6,16 gyro--3,17,19 radio--1,2,18
  TRANSITION_TO(task_1);
}