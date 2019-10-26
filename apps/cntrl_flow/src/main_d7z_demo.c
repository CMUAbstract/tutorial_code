/* Sample code designed to mimic the behavior of a d7a modem (protocol used for
 * medium range IoT connectivity) from Wizzilab, outfitted with an
 * x-nucleo-iks01a2 sensor expansion board from ST
*/
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

#include <libradio/radio.h>
#include <libhmc/magnetometer.h>
#include <liblps/pressure.h>
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

#define LPS 0
#define LSM 1
#define TEMP 2
#define HMC 3
#define BUTTON 4


REGISTER(modem);
REGISTER(lps);
REGISTER(lsm);
REGISTER(temp);
REGISTER(hmc);

TASK(task_init)
TASK(task_file_modified)
TASK(task_send)
TASK(task_dispatch)
TASK(task_lps)
TASK(task_lsm)
TASK(task_temp)
TASK(task_hmc)

typedef enum report_t {
  REPORT_ON_DIFFERENCE,
  REPORT_ALWAYS,
  REPORT_ON_THRESHOLD
} report_t;

typedef struct sensor_config_t_ {
  report_t report_type;
  int16_t read_period;
  int16_t max_period;
  int16_t max_diff;
  int16_t threshold_high;
  int16_t threshold_low;
} sensor_config_t;

#include "files.h"

uint16_t dummy_single_read() {
  static uint16_t start = 16;
  start = (start + 3) % 255;
  return start;
}

void dummy_multi_read(uint16_t *x, uint16_t *y, uint16_t *z) {
  static uint16_t start = 16;
  start = (start + 3) % 255;
  *x = start;
  start = (start + 3) % 255;
  *y = start;
  start = (start + 3) % 255;
  *z = start;
  return;
}

#define MIN_REPORT_PERIOD 10
static bool report_ok(uint32_t last_report_time)
{
  // Do not send a report if it's been less than MIN_REPORT_PERIOD since the
  // last report
  if ((last_report_time/1000) < MIN_REPORT_PERIOD){
    LOG("Report Skipped. (Locked for %ds)\n", 
        MIN_REPORT_PERIOD -(last_report_time/1000));
    return false;
  }

  return true;
}


// Check parameters to see if data should be send
static bool report_needed(sensor_config_t* config, uint16_t value,
uint16_t last_value, uint16_t last_report_time, uint8_t user_id) {
  switch (config->report_type)
  {
    case REPORT_ALWAYS:
      // Send a report at each measure
      LOG("Report[%d] always\r\n", user_id);
      return report_ok(last_report_time);
    case REPORT_ON_DIFFERENCE:
      // Send a report when the difference between the last reported measure and
      // the current mesure is greater than max_diff
      if (abs(last_value - value) >= config->max_diff && config->max_diff) {
        LOG("Report[%d] on difference (last:%d new:%d max_diff:%d)\r\n",
        user_id, last_value, value, config->max_diff);
        return report_ok(last_report_time);
      }
      break;
    case REPORT_ON_THRESHOLD:
      // Send a report when crossing a threshold
      if (   (value >= config->threshold_high && last_value < config->threshold_high)
          || (value <= config->threshold_low  && last_value > config->threshold_low)
          || (value < config->threshold_high  && last_value >= config->threshold_high)
          || (value > config->threshold_low   && last_value <= config->threshold_low)) {
        LOG("Report[%d] on threshold (last:%d new:%d th:%d tl:%d)\r\n",
        user_id, last_value, value, config->threshold_high,
        config->threshold_low);
        return report_ok(last_report_time);
      }
      break;
    default:
      break;
  }
  // Send a report if it's been more than max_period since the last report
  if (((last_report_time/1000) >= config->max_period) && config->max_period) {
    LOG("Report[%d] on period (max_period:%d time:%d)\r\n", user_id,
    config->max_period, last_report_time);
    return report_ok(last_report_time);
  }

  return false;
}


TASK_SHARED(uint8_t, button);

// Stand in interrupt for button press
void DRIVER Port_1_ISR(void) {
  // If button fires, transmit alarm and send file contents
  TS(button) = 1;

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

capybara_task_cfg_t pwr_configs[4] = {
  CFG_ROW(0, CONFIGD, LOWP2,LOWP2),
  CFG_ROW(1, PREBURST, LOWP2,MEDHIGHP),
  CFG_ROW(2, BURST, MEDHIGHP, MEDHIGHP),
  CFG_ROW(3, CONFIGD, MEDP,MEDP),
};

int16_t read_temp(void);

void init() {
  capybara_init();
  fxl_set(BIT_SENSE_SW);
  __delay_cycles(80000);
  __delay_cycles(80000);
  /*pressure_init();
  pressure_t temp;
  while(1) {
    read_pressure(&temp);
    PRINTF("Vals: %x %x %x\r\n", temp.MSB, temp.SMSB, temp.LSB);
  }
  magnetometer_init();
  PRINTF("Initialized!\r\n");
  magnet_t temp;
  while(1) {
    magnetometer_read(&temp);
    PRINTF("Vals: %x %x %x\r\n", temp.x, temp.y, temp.z);
  }*/
}

TASK_SHARED(uint8_t,state);
TASK_SHARED(uint16_t, last_val_lps);
TASK_SHARED(uint16_t, last_report_time_lps);
TASK_SHARED(uint16_t, last_val_lsm, 3);
TASK_SHARED(uint16_t, last_report_time_lsm);
TASK_SHARED(uint16_t, last_val_temp);
TASK_SHARED(uint16_t, last_report_time_temp);
TASK_SHARED(uint16_t, last_val_hmc, 3);
TASK_SHARED(uint16_t, last_report_time_hmc);

void task_init() {
  // Initialize all the active sensors here
  TS(state) = 0;
  TS(button) = 0;
  TS(last_report_time_lps) = 0xFF;
  TS(last_val_lps) = 0;
  TS(last_report_time_lsm) = 0xFF;
  TS(last_report_time_temp) = 0xFF;
  TS(last_val_temp) = 0;
  TS(last_report_time_hmc) = 0xFF;
  for (int i = 0; i < 3; i++) {
    TS(last_val_lsm,i) = 0;
    TS(last_val_hmc,i) = 0;
  }
  // Init modem
  STATE_CHANGE(modem, 0x1);
  STATE_CHANGE(modem, 0x2);
  STATE_CHANGE(modem, 0x3);
  STATE_CHANGE(modem, 0x4);
  STATE_CHANGE(modem, 0x5);
  // Init lps
  STATE_CHANGE(lps,0x1);
  STATE_CHANGE(lps,0x2);
  pressure_init(); //Set up for one shots
  // Init lsm
  STATE_CHANGE(lsm,0x1);
  STATE_CHANGE(lsm,0x2);
  gyro_init_data_rate(0x40);
  // Init temp
  STATE_CHANGE(temp,0x1);
  STATE_CHANGE(temp,0x2);
  // Init hmc
  STATE_CHANGE(hmc,0x1);
  STATE_CHANGE(hmc,0x2);
  magnetometer_init();
  PRINTF("Done!\r\n");

  // In the mbed version we schedule all the sensor threads here
  TRANSITION_TO(task_file_modified);
}

void task_file_modified() {
  // Check if any incoming messages on modem
  // Write updates to sensor files if there are any
  // This means the potential for arbitrary state changes to all of the
  // peripherals!!!!
  // We'll model this by doing:
  switch (TS(state)) {
    case LPS:
      STATE_CHANGE(lps,0x3);
      break;
    case LSM:
      STATE_CHANGE(lsm,0x3);
      break;
    case TEMP:
      STATE_CHANGE(temp,0x3);
      break;
    case HMC:
      STATE_CHANGE(hmc,0x3);
      break;
    default:
      break;
  }
  TRANSITION_TO(task_dispatch);
}

#define ALARM 0xEE

void task_button() {
  if (TS(button)) {
    radio_buff[0] = ALARM;
    TS(button) = 0;
    TRANSITION_TO(task_send);
  }
  TRANSITION_TO(task_dispatch);
}


// A proxy for the scheduler in mBed
void task_dispatch() {
  PRINTF("State is: %i\r\n",TS(state));
  switch(TS(state)) {
    case LPS: {
      TS(state) = LSM;
      TRANSITION_TO(task_lps);
    }
    case LSM: {
      TS(state) = TEMP;
      TRANSITION_TO(task_lsm);
    }
    case TEMP: {
      TS(state) = HMC;
      TRANSITION_TO(task_temp);
    }
    case HMC: {
      TS(state) = BUTTON;
      TRANSITION_TO(task_hmc);
    }
    case BUTTON: {
      TS(state) = LPS;
      TRANSITION_TO(task_button);
    }
    default: {
      PRINTF("Error! state is undefined\r\n");
      TRANSITION_TO(task_dispatch);
    }
  }
}

void task_lps() {
  // Read sensor
  pressure_t temp;
  read_pressure(&temp);
  PRINTF("Pressure: %x %x %x\r\n", temp.MSB, temp.SMSB, temp.LSB);
  uint16_t val;
  val = ((uint16_t)temp.MSB << 8) + temp.SMSB;
  // Check if we need another sample
  if ( report_needed(&f_sensor_config_lps,val,TS(last_val_lps),
        TS(last_report_time_lps),LPS)) {
    TS(last_val_lps) = val;
    TS(last_report_time_lps) = 0;
  // Send if we do
    radio_buff[0] = val & 0xFF;
    radio_buff[1] = (val & 0xFF00) >> 8;
    TRANSITION_TO(task_send);
  }
  TS(last_report_time_lps) = TS(last_report_time_lps) + 4;
  // Thread Sleep?
  TRANSITION_TO(task_dispatch);
}

void task_lsm() {
  // Read sensor
  uint16_t vals[3];
  read_raw_gyro(vals, vals + 1, vals +2);
  PRINTF("Gyro vals: %x %x %x\r\n",vals[0], vals[1], vals[2]);
  //TODO add accelerometer
  // Check if we need another sample
  int flag = 0;
  for (int i = 0; i < 3; i++) {
    if ( report_needed(&f_sensor_config_lsm,vals[i],TS(last_val_lsm,i),
          TS(last_report_time_lsm),LSM)) {
      flag = 1;
      TS(last_val_lsm,i) = vals[i];
      TS(last_report_time_lsm) = 0;
    }
  }
  // Send if we do
  if (flag) {
    for (int i = 0; i < 3; i++) {
      radio_buff[(i<<2) + 0] = vals[i] & 0xFF;
      radio_buff[(i<<2) + 1] = (vals[i] & 0xFF00) >> 8;
      TRANSITION_TO(task_send);
    }
  }
  TS(last_report_time_lsm) = TS(last_report_time_lsm) + 4;
  // Thread Sleep?
  TRANSITION_TO(task_dispatch);
}

void task_temp() {
  // Read sensor
  uint16_t val = read_temp();
  PRINTF("Temp = %u\r\n", val);
  // Check if we need another sample
  if ( report_needed(&f_sensor_config_temp,val,TS(last_val_temp),
        TS(last_report_time_temp),TEMP)) {
    TS(last_val_temp) = val;
    TS(last_report_time_temp) = 0;
  // Send if we do
    radio_buff[0] = val & 0xFF;
    radio_buff[1] = (val & 0xFF00) >> 8;
    TRANSITION_TO(task_send);
  }
  TS(last_report_time_temp) = TS(last_report_time_temp) + 4;
  // Thread Sleep?
  TRANSITION_TO(task_dispatch);
}

void task_hmc() {
  // Read sensor
  magnet_t temp;
  uint16_t vals[3];
  magnetometer_read(&temp);
  PRINTF("mag vals: %x %x %x\r\n", temp.x, temp.y, temp.z);
  vals[0] = temp.x;
  vals[1] = temp.y;
  vals[2] = temp.z;
  // Check if we need another sample
  int flag = 0;
  for (int i = 0; i < 3; i++) {
    if ( report_needed(&f_sensor_config_hmc,vals[i],TS(last_val_hmc,i),
          TS(last_report_time_hmc),LSM)) {
      flag = 1;
      TS(last_val_hmc,i) = vals[i];
      TS(last_report_time_hmc) = 0;
    }
  }
  // Send if we do
  if (flag) {
    for (int i = 0; i < 3; i++) {
      radio_buff[(i<<2) + 0] = vals[i] & 0xFF;
      radio_buff[(i<<2) + 1] = (vals[i] & 0xFF00) >> 8;
      TRANSITION_TO(task_send);
    }
  }
  TS(last_report_time_hmc) = TS(last_report_time_hmc) + 4;
  // Thread Sleep?
  TRANSITION_TO(task_dispatch);
}

void task_send() {
  // Reconfigure bank
  capybara_transition(3);
  // Send header. Our header is 0xAA1234
  radio_buff[0] = 0xAA;
  radio_buff[1] = 0x12;
  radio_buff[2] = 0x34;
  PRINTF("Sending\r\n");
  // Send data. I'll just send 0x01
  for(int i = 3; i < LIBRADIO_BUFF_LEN; i++) {
      radio_buff[i] = 0x01;
  }
  // Send it!
  STATE_CHANGE(modem, 0x6);
  radio_send();
  STATE_CHANGE(modem, 0x5);
  PRINTF("Sent\r\n");
  TRANSITION_TO(task_dispatch);
}

ENTRY_TASK(task_init)
INIT_FUNC(init)

// Table 6-62: ADC12 calibration for 1.2v reference
#define TLV_CAL30 ((int *)(0x01A1A))
#define TLV_CAL85 ((int *)(0x01A1C))

// Returns temperature in degrees C (approx range -40deg - 85deg)
int16_t read_temp() {
  ADC12CTL0 &= ~ADC12ENC;           // Disable conversions

  ADC12CTL3 |= ADC12TCMAP;
  ADC12CTL1 = ADC12SHP;
  ADC12CTL2 = ADC12RES_2;
  ADC12MCTL0 = ADC12VRSEL_1 | BIT4 | BIT3 | BIT2 | BIT1;
  ADC12CTL0 |= ADC12SHT03 | ADC12ON;

  while( REFCTL0 & REFGENBUSY );

  REFCTL0 = REFVSEL_0 | REFON;

  // Wait for REF to settle
  __delay_cycles(40000);

  ADC12CTL0 |= ADC12ENC;                         // Enable conversions
  ADC12CTL0 |= ADC12SC;                   // Start conversion
  while (ADC12CTL1 & ADC12BUSY) ;

  int sample = ADC12MEM0;

  ADC12CTL0 &= ~ADC12ENC;           // Disable conversions
  ADC12CTL0 &= ~(ADC12ON);  // Shutdown ADC12
  REFCTL0 &= ~REFON;

  int cal30 = *TLV_CAL30;
  int cal85 = *TLV_CAL85;
  int tempC = (sample - cal30) * 55 / (cal85 - cal30) + 30;

  LOG("[temp] sample=%i => T=%i\r\n", sample, tempC);

  return tempC;
}
