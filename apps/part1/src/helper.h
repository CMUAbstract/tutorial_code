#ifndef __HELPER__
#define __HELPER__
#include <libfixed/fixed.h>

enum {CORRECT_ANSWER, INCORRECT_ANSWER};

#include <libradio/radio.h>

#define zero 0
#define one 1
#define two 2
#define three 3
#define four 4

capybara_task_cfg_t pwr_configs[4] = {
  CFG_ROW(zero, CONFIGD, LOWP2,LOWP2),
  CFG_ROW(one, PREBURST, LOWP2,MEDHIGHP),
  CFG_ROW(two, BURST, MEDHIGHP, MEDHIGHP),
  CFG_ROW(three, CONFIGD, MEDP,MEDP),
};

void print_result(int ROWS, int DCOLS, fixed* result) {
	for(uint16_t i = 0; i < ROWS; i++) {
		for(uint16_t k = 0; k < DCOLS; k++) {
			PRINTF("%u ", result[i * DCOLS + k]);
		}
	}
	PRINTF("\r\n");
}

void send_radio(int ROWS, int DCOLS, fixed* result) {
	// Reconfigure bank
	capybara_transition(3);
	// Send header. Our header is 0xAA1234
	radio_buff[0] = 0xAA;
	radio_buff[1] = 0x12;
	radio_buff[2] = 0x34;
	// Send data. I'll just send 0x01
	if (result[0] == 65216 && result[1] == 105
			&& result[2] == 65414 && result[3] == 65476) {
		// Correct
		radio_buff[4] = CORRECT_ANSWER;
	} else {
		// Incorrect
		radio_buff[4] = INCORRECT_ANSWER;
	}
	// Send it!
	radio_send();
}

#endif
