/* Sample code designed to mimic the behavior of a d7a modem (protocol used for
 * medium range IoT connectivity) from Wizzilab, outfitted with an
 * x-nucleo-iks01a2 sensor expansion board from ST
 * Behavior is modified to be aware of the state each periph is in and to not
 * set everything on at once.
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
#include <libapds/color.h>
#include <libapds/proximity.h>

#define LPS 0
#define LSM 1
#define LSM2 2
#define TEMP 3
#define HMC 4
#define BUTTON 5
#define LIGHT 6

REGISTER(modem);
REGISTER(lps);
REGISTER(accel);
REGISTER(gyro);
REGISTER(temp);
REGISTER(hmc);
REGISTER(light);

TASK(task_init)
TASK(task_file_modified)
TASK(task_send)
TASK(task_dispatch)
TASK(task_lps)
TASK(task_lsm)
TASK(task_temp)
TASK(task_hmc)
TASK(task_light)
TASK(task_accel)

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

#define MIN_REPORT_PERIOD 10
static bool report_ok(uint32_t last_report_time)
{
  // Do not send a report if it's been less than MIN_REPORT_PERIOD since the
  // last report
  if ((last_report_time/1000) < MIN_REPORT_PERIOD){
    LOG2("Report Skipped. (Locked for %ds)\n", 
        MIN_REPORT_PERIOD -(last_report_time/1000));
    return false;
  }
  // We'll change this so that report_ok doesn't control whether we send a
  // messag or not
  //return true;
  return false;
}


// Check parameters to see if data should be send
static bool report_needed(sensor_config_t* config, uint16_t value,
uint16_t last_value, uint16_t last_report_time, uint8_t user_id)
{ bool flag;
  switch (config->report_type)
  {
    case REPORT_ALWAYS:
      // Send a report at each measure
      LOG2("Report[%d] always\r\n", user_id);
      return report_ok(last_report_time);
    case REPORT_ON_DIFFERENCE:
      // Send a report when the difference between the last reported measure and
      // the current mesure is greater than max_diff
      if (abs(last_value - value) >= config->max_diff && config->max_diff) {
        LOG2("Report[%d] on difference (last:%d new:%d max_diff:%d)\r\n",
        user_id, last_value, value, config->max_diff);
        flag = report_ok(last_report_time);
      }
      break;
    case REPORT_ON_THRESHOLD:
      // Send a report when crossing a threshold
      if (   (value >= config->threshold_high && last_value < config->threshold_high)
          || (value <= config->threshold_low  && last_value > config->threshold_low)
          || (value < config->threshold_high  && last_value >= config->threshold_high)
          || (value > config->threshold_low   && last_value <= config->threshold_low)) {
        LOG2("Report[%d] on threshold (last:%d new:%d th:%d tl:%d)\r\n",
        user_id, last_value, value, config->threshold_high,
        config->threshold_low);
        flag = report_ok(last_report_time);
      }
      break;
    default:
      break;
  }
  // Send a report if it's been more than max_period since the last report
  if (((last_report_time/1000) >= config->max_period) && config->max_period) {
    LOG2("Report[%d] on period (max_period:%d time:%d)\r\n", user_id,
    config->max_period, last_report_time);
    flag  = report_ok(last_report_time);
  }
  // We add this part
  if (last_report_time) {
    LOG("returning true\r\n");
    return true;
  }
  LOG("Returning false\r\n");
  return false;
}


TASK_SHARED(uint8_t, button);

// Stand in interrupt for button press
void DRIVER Port_1_ISR(void) {
//void __attribute__((interrupt(0))) Port_1_ISR(void) {
  // If button fires, transmit alarm and send file contents
  P1IE &= ~BIT4;
  P1IFG &= ~BIT4;
  TS(button) = 1;
  P1IE |= BIT4;
  return;
}

void DISABLE(Port_1_ISR) {
//void disable() {
  P1IE &= ~BIT4; //disable interrupt bit
  return;
}

void ENABLE(Port_1_ISR) {
//void enable() {
  P1IE |= BIT4; //enable interrupt bi
  return;
}

capybara_task_cfg_t pwr_configs[4] = {
  CFG_ROW(0, CONFIGD, LOWP,LOWP),
  CFG_ROW(1, PREBURST, LOWP2,MEDHIGHP),
  CFG_ROW(2, BURST, MEDHIGHP, MEDHIGHP),
  CFG_ROW(3, CONFIGD, MEDLOWP2,MEDLOWP2),
};

int16_t read_temp(void);

#define PACA_SIZE 4

typedef struct {
  uint8_t fxl_byte;
  int num_funcs;
  void (*pointertable[PACA_SIZE])(void);
} pacarana_cfg_t;

#define PACA_CFG_ROW(fxlByte,numFuncs,...) \
  {fxlByte,numFuncs,{__VA_ARGS__}}

void static gyro_init_odr_loc() {
  gyro_only_init_data_rate(0x40);
  return;
}

void static accel_init_odr_loc() {
  accelerometer_init_data_rate(0x40);
  return;
}

pacarana_cfg_t inits[6] = {
  //PACA_CFG_ROW(BIT_SENSE_SW,0,NULL)
  PACA_CFG_ROW(BIT_SENSE_SW, 1, pressure_init),
  PACA_CFG_ROW(BIT_SENSE_SW, 1, pressure_init),
  PACA_CFG_ROW(BIT_SENSE_SW, 1, gyro_init_odr_loc),
  PACA_CFG_ROW(BIT_SENSE_SW, 1, accel_init_odr_loc),
  PACA_CFG_ROW(BIT_SENSE_SW, 1, magnetometer_init),
  PACA_CFG_ROW(BIT_SENSE_SW, 1, enable_photoresistor)
};


void init() {
  capybara_init();
  // Sanity check our number
  P1OUT |= BIT2;
  P1DIR |= BIT2;
  P1OUT &= ~BIT2;
  if(curctx->pacaCfg > sizeof(inits)/sizeof(pacarana_cfg_t)) {
    printf("Error! invalid pacaCfg number\n");
    while(1);
  }
  // interrupt pin setup
  P1OUT |= BIT4;
  P1DIR &= ~BIT4;
  P1REN &= ~BIT4;

  P1IES &= ~BIT4; // Set IFG on rising edge (low --> high)
  P1IFG &= ~BIT4; // Clear flag bit
  P1OUT |= BIT2;
  P1DIR |= BIT2;
  P1OUT &= ~BIT2;
  // Set up the board-side stuff
  fxl_set(inits[curctx->pacaCfg].fxl_byte);
  P1OUT |= BIT2;
  P1DIR |= BIT2;
  P1OUT &= ~BIT2;
  // TODO figure out a solution for delay cycles
  __delay_cycles(80000);
  P1OUT |= BIT2;
  P1DIR |= BIT2;
  P1OUT &= ~BIT2;
  // Walk through the init functions
  for(int i = 0; i < inits[curctx->pacaCfg].num_funcs; i++) {
    inits[curctx->pacaCfg].pointertable[i]();
  }
  P1OUT |= BIT2;
  P1DIR |= BIT2;
  P1OUT &= ~BIT2;
  PRINTF("Done\r\n");
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
TASK_SHARED(uint16_t, last_val_accel, 3);
TASK_SHARED(uint16_t, last_report_time_accel);
TASK_SHARED(uint16_t, last_val_light);
TASK_SHARED(uint16_t, last_report_time_light);

void task_init() {
  // Initialize all the active sensors here
  capybara_transition(3);
  //DISABLE(Port_1_ISR);
  LOG2("task init 1\r\n");
  TS(state) = 0;
  TS(button) = 0;
  TS(last_report_time_lps) = 0x0;
  TS(last_val_lps) = 0;
  TS(last_report_time_lsm) = 0x0;
  TS(last_report_time_temp) = 0x0;
  TS(last_val_temp) = 0;
  TS(last_report_time_hmc) = 0x0;
  for (int i = 0; i < 3; i++) {
    TS(last_val_lsm,i) = 0;
    TS(last_val_hmc,i) = 0;
  }
  // In the mbed version we schedule all the sensor threads here
  PACA_TRANSITION_TO(task_dispatch,0);
}

#if 0
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
  PACA_TRANSITION_TO(task_dispatch,0);
}
#endif

#define ALARM 0xEE

void task_button() {
  LOG2("task button\r\n");
  if (TS(button)) {
    radio_buff[0] = ALARM;
    TS(button) = 0;
    PACA_TRANSITION_TO(task_send,0);
  }
  PACA_TRANSITION_TO(task_dispatch,0);
}


// A proxy for the scheduler in mBed
void task_dispatch() {
  capybara_transition(3);
  LOG2("task dispatch");
  if (!TS(last_report_time_lps)  || 
      !TS(last_report_time_hmc)  || 
      !TS(last_report_time_lsm)  || 
      !TS(last_report_time_temp) ||
      !TS(last_report_time_light)|| 
      !TS(last_report_time_accel)|| 
      TS(state) != BUTTON
    ) {
    LOG2("Mark end\r\n");
    P1OUT |= BIT0;
    P1DIR |= BIT0;
    P1OUT &= ~BIT0;
  }
  switch(TS(state)) {
    case LPS: {
      TS(state) = LSM;
      pressure_init();
      STATE_CHANGE(lps, 0x1);
      PACA_TRANSITION_TO(task_lps,1);
    }
    case LSM: {
      TS(state) = LSM2;
      gyro_init_odr_loc();
      STATE_CHANGE(gyro, 0x1);
      PACA_TRANSITION_TO(task_lsm,2);
    }
    case LSM2: {
      TS(state) = TEMP;
      accel_init_odr_loc();
      STATE_CHANGE(accel, 0x1);
      PACA_TRANSITION_TO(task_accel,3);
    }
    case TEMP: {
      TS(state) = HMC;
      PACA_TRANSITION_TO(task_temp,0);
    }
    case HMC: {
      TS(state) = LIGHT;
      magnetometer_init();
      STATE_CHANGE(hmc,0x1);
      PACA_TRANSITION_TO(task_hmc,4);
    }
    case LIGHT: {
      TS(state) = BUTTON;
      enable_photoresistor();
      STATE_CHANGE(light,0x1);
      PACA_TRANSITION_TO(task_light,5);
    }
    case BUTTON: {
      TS(state) = LPS;
      PACA_TRANSITION_TO(task_button,0);
    }
    default: {
      PRINTF("Error! state is undefined\r\n");
      PACA_TRANSITION_TO(task_dispatch,0);
    }
  }
}

void task_lps() {
  // Read sensor
  pressure_t temp;
  LOG2("task lps\r\n");
  read_pressure(&temp);
  LOG("Pressure: %x %x %x\r\n", temp.MSB, temp.SMSB, temp.LSB);
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
    PACA_TRANSITION_TO(task_send,0);
  }
  TS(last_report_time_lps) = TS(last_report_time_lps) + 1;
  // Thread Sleep?
  pressure_disable();
  STATE_CHANGE(lps, 0x0);
  PACA_TRANSITION_TO(task_dispatch,0);
}

void task_lsm() {
  // Read sensor
  uint16_t vals[3];
  LOG2("task lsm\r\n");
  read_raw_gyro(vals, vals + 1, vals +2);
  LOG("Gyro vals: %x %x %x\r\n",vals[0], vals[1], vals[2]);
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
      PACA_TRANSITION_TO(task_send,0);
    }
  }
  TS(last_report_time_lsm) = TS(last_report_time_lsm) + 1;
  // Thread Sleep?
  lsm_disable();
  STATE_CHANGE(gyro, 0x0);
  STATE_CHANGE(accel, 0x0);
  PACA_TRANSITION_TO(task_dispatch,0);
}

void task_accel() {
  // Read sensor
  uint16_t vals[3];
  LOG2("task accel\r\n");
  accelerometer_read(vals, vals + 1, vals +2);
  LOG("Accel vals: %x %x %x\r\n",vals[0], vals[1], vals[2]);
  // Check if we need another sample
  int flag = 0;
  for (int i = 0; i < 3; i++) {
    if ( report_needed(&f_sensor_config_accel,vals[i],TS(last_val_accel,i),
          TS(last_report_time_accel),LSM)) {
      flag = 1;
      TS(last_val_accel,i) = vals[i];
      TS(last_report_time_accel) = 0;
    }
  }
  // Send if we do
  if (flag) {
    for (int i = 0; i < 3; i++) {
      radio_buff[(i<<2) + 0] = vals[i] & 0xFF;
      radio_buff[(i<<2) + 1] = (vals[i] & 0xFF00) >> 8;
      PACA_TRANSITION_TO(task_send,0);
    }
  }
  TS(last_report_time_accel) = TS(last_report_time_accel) + 1;
  // Thread Sleep?
  lsm_disable();
  STATE_CHANGE(gyro, 0x0);
  STATE_CHANGE(accel, 0x0);
  PACA_TRANSITION_TO(task_dispatch,0);
}

void task_temp() {
  // Read sensor
  LOG2("task_temp\r\n");
  uint16_t val = read_temp();
  LOG("Temp = %u\r\n", val);
  // Check if we need another sample
  if ( report_needed(&f_sensor_config_temp,val,TS(last_val_temp),
        TS(last_report_time_temp),TEMP)) {
    TS(last_val_temp) = val;
    TS(last_report_time_temp) = 0;
  // Send if we do
    radio_buff[0] = val & 0xFF;
    radio_buff[1] = (val & 0xFF00) >> 8;
    PACA_TRANSITION_TO(task_send,0);
  }
  TS(last_report_time_temp) = TS(last_report_time_temp) + 1 ;
  // Thread Sleep?
  PACA_TRANSITION_TO(task_dispatch,0);
}

void task_light() {
  // Read sensor
  LOG2("task_light\r\n");
  uint16_t val = read_photoresistor();
  LOG("Light = %u\r\n", val);
  // Check if we need another sample
  if ( report_needed(&f_sensor_config_light,val,TS(last_val_light),
        TS(last_report_time_light),TEMP)) {
    TS(last_val_light) = val;
    TS(last_report_time_light) = 0;
  // Send if we do
    radio_buff[0] = val & 0xFF;
    radio_buff[1] = (val & 0xFF00) >> 8;
    PACA_TRANSITION_TO(task_send,0);
  }
  TS(last_report_time_light) = TS(last_report_time_light) + 1 ;
  // Thread Sleep?
  disable_photoresistor();
  STATE_CHANGE(light,0x0);
  PACA_TRANSITION_TO(task_dispatch,0);
}

void task_hmc() {
  // Read sensor
  magnet_t temp;
  uint16_t vals[3];
  LOG("task hmc\r\n");
  magnetometer_read(&temp);
  LOG("mag vals: %x %x %x\r\n", temp.x, temp.y, temp.z);
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
      PACA_TRANSITION_TO(task_send,0);
    }
  }
  TS(last_report_time_hmc) = TS(last_report_time_hmc) + 1;
  // Thread Sleep?
  magnetometer_disable();
  STATE_CHANGE(hmc,0x0);
  PACA_TRANSITION_TO(task_dispatch,0);
}

void task_send() {
  // Reconfigure bank
  capybara_transition(3);
  P1OUT |= BIT1;
  P1DIR |= BIT1;
  P1OUT &= ~BIT1;
  PRINTF("task send\r\n");
  // Send header. Our header is 0xAA1234
  /*radio_buff[0] = 0xAA;
  radio_buff[1] = 0x12;
  radio_buff[2] = 0x34;
  */
  radio_buff[0] = 0x12;
  radio_buff[1] = 0x34;
  radio_buff[2] = TS(state);
  // Send data. I'll just send 0x01
  for(int i = 3; i < LIBRADIO_BUFF_LEN; i++) {
      radio_buff[i] = i;
  }
  // Send it!
  STATE_CHANGE(modem, 0x6);
  radio_send();
  for (int i =0; i< 30; i++) {
    __delay_cycles(120000);
  }
  STATE_CHANGE(modem, 0x5);
  PACA_TRANSITION_TO(task_dispatch,0);
}

ENTRY_TASK(task_init)
INIT_FUNC(init)

// Table 6-62: ADC12 calibration for 1.2v reference
#define TLV_CAL30 ((int *)(0x01A1A))
#define TLV_CAL85 ((int *)(0x01A1C))

// Returns temperature in degrees C (approx range -40deg - 85deg)
int16_t DRIVER read_temp() {
  ADC12CTL0 &= ~ADC12ENC;           // Disable conversions
  
  ADC12CTL3 |= ADC12TCMAP;
  ADC12CTL1 = ADC12SHP;
  ADC12CTL2 = ADC12RES_2;
  ADC12MCTL0 = ADC12VRSEL_1 | BIT4 | BIT3 | BIT2 | BIT1;
  ADC12CTL0 |= ADC12SHT03 | ADC12ON;
  STATE_CHANGE(temp,0x1);
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
  STATE_CHANGE(temp,0x0);

  int cal30 = *TLV_CAL30;
  int cal85 = *TLV_CAL85;
  int tempC = (sample - cal30) * 55 / (cal85 - cal30) + 30;

  LOG2("[temp] sample=%i => T=%i\r\n", sample, tempC);

  return tempC;
}


__attribute__((section("__interrupt_vector_port1"),aligned(2)))
void (*__vector_port1)(void) = Port_1_ISR;

