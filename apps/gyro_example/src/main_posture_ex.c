#include <msp430.h>

#include <libmspbuiltins/builtins.h>
#include <libmsp/clock.h>
#include <libmsp/watchdog.h>
#include <libmsp/gpio.h>
#include <libio/console.h>
#include <libcapybara/reconfig.h>
#include <libcapybara/board.h>
#include <libradio/radio.h>
#include <libalpaca/alpaca.h>
#include <liblsm/gyro.h>
#include <libpacarana/pacarana.h>
#include <libapds/proximity.h>
#include <libfxdmath/fxdmath.h>
#include <libpacarana/pacarana.h>

#define BUFF_LEN 20
#define WARNING_LVL 200
#define SERIES_LEN 23
#define CAL_LEN 5
#define SAMPLE_LEN BUFF_LEN
#define RADIO_ON_CYCLES   60 // ~12ms (a bundle of 2-3 pkts 25 payload bytes each on Pin=0)
#define RADIO_BOOT_CYCLES 60
#define RADIO_RST_CYCLES   1


#define UNDEF 0
#define ABOVE 1
#define BELOW 2

REGISTER(gyro);
REGISTER(test);

typedef struct xl_val_ {
  uint16_t x;
  uint16_t y;
  uint16_t z;
} xl_val;

static const uint16_t DATA_SRC[60] = {
#include "extra_data.h"
};

// NV variables that will be handled by coati
__nv xl_val val_buffer[BUFF_LEN];
__nv uint8_t val_rindex;
__nv uint8_t val_windex;
__nv uint32_t buffered_squares[3];

__nv uint16_t cnt_over_sigma;
__nv uint16_t cnt_under_sigma;

__nv uint16_t buffered_zs[CAL_LEN];
__nv uint8_t need_cal;
__nv xl_val cal_avg;
__nv xl_val cal_std_dev;
__nv uint16_t cal_avg_angle;
__nv uint16_t cal_std_dev_angle;
__nv uint16_t cal_angles[CAL_LEN];
__nv uint32_t cal_sums[CAL_LEN];
__nv uint16_t val_cosAngle;
__nv uint16_t radio_packet[11];
__nv uint8_t radio_cmd;

__nv uint16_t partial_m;
__nv uint32_t temp_sum;
__nv uint16_t iterator;

__nv uint16_t count;
// Vars that are really internal to the radio code
typedef enum __attribute__((packed)) {
    RADIO_CMD_SUMMARY = 0,
    RADIO_CMD_ABOVE_WARNING = 1,
    RADIO_CMD_BELOW_WARNING = 2,
    RADIO_CMD_CAL_NEEDED = 3,
} radio_cmd_t;

typedef struct __attribute__((packed)) {
    radio_cmd_t cmd;
    uint8_t series[SERIES_LEN];
} radio_pkt_t;

static radio_pkt_t radio_pkt;

TASK(task_init,3)
TASK(task_val_compare,4)
TASK(task_calc_sqrs,5)
TASK(task_calc_angle,6)
TASK(task_summary,7)
TASK(task_cal_avgs,8)
TASK(task_cal_std_devs,9)
TASK(task_cal_calc_sqrs,10)
TASK(task_cal_calc_angles,11)
TASK(task_cal_angle_avg,12)
TASK(task_cal_std_dev,13)


__nv capybara_task_cfg_t pwr_configs[3] = {
  CFG_ROW(0, CONFIGD, LOWP, LOWP),
  CFG_ROW(1, BURST, LOWP, BACKP),
  CFG_ROW(2, PREBURST, LOWP, BACKP),
};

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

//#define TEST_VDDSENSE
//#define TEST_FIFO_OFF
#define FIFO_BYTES 48

__nv uint8_t array[3][FIFO_BYTES];
__nv uint8_t array1[3][FIFO_BYTES];

#ifndef TEST_VDDSENSE
//void __attribute__((interrupt(0))) Port_1_ISR(void)
void DRIVER Port_1_ISR(void)
{   //Need to clear whatever interrupt flag fired to get us here!
  // Clear LSM_INT1 on processor side
  DISABLE(Port_1_ISR);
  P1OUT |= BIT2;
  P1DIR |= BIT2;
  P1OUT &= ~BIT2;
  //printf("Here now\r\n");
  if(P1IFG & BIT4) {
    P1IFG &= ~BIT4;
    STATE_CHANGE(gyro,0x10);
    ENABLE(Port_1_ISR);
    uint8_t fifolvl = read_fifo_lvl();
    read_tap_src();
    STATE_CHANGE(gyro,0x00);
    //printf("Here1\r\n");
  }
  // Clear LSM_INT2, note, we'll leave INT1 as our higher priority and handle
  // that first, but then we can fall into this statement
#ifndef TEST_FIFO_OFF
  if(P1IFG & BIT5) {
    P1IFG &= ~BIT5;
    uint8_t fifolvl = read_fifo_lvl();
    STATE_CHANGE(gyro,0x01);
    if(fifolvl < FIFO_THR - 1) {
      // reset fifo and return without registering a bottom half
      //printf("Here2\r\n");
      fifo_reset();
      STATE_CHANGE(gyro,0x00);
      return;
    }
    dump_fifo(array, FIFO_THR);
    dump_fifo_high(array1,FIFO_THR);
    fifo_reset();
    STATE_CHANGE(gyro,0x00);
    //printf("Here3\r\n");
  }
  P1IE |= BIT5;
#endif
  //printf("enable\r\n");
  P1IE |= BIT4; //enable interrupt bit
  return;
}
__attribute__((section("__interrupt_vector_port1"),aligned(2)))
void (*__vector_port1)(void) = Port_1_ISR;
#endif

void DRIVER Port_2_ISR(void) {
  STATE_CHANGE(test,0x05);
  if (P2IFG & BIT4) {
    STATE_CHANGE(gyro,0x04);
  }
  return;
}

void init() {
  capybara_init();
  fxl_set(BIT_SENSE_SW);
  //TODO figure out how to better manage this delay
  #ifndef TEST_VDDSENSE
  P1OUT |= BIT4;
  P1DIR &= ~BIT4;
  P1REN &= ~BIT4;

  P1IES &= ~BIT4; // Set IFG on rising edge (low --> high)
  P1IFG &= ~BIT4; // Clear flag bit

  // Setup extra interrupt from LSM connected to AUX5
  P1OUT |= BIT5;	// Set P3.5 as  pull up
  P1DIR &= ~BIT5; // Set P3.5 as input
  P1REN &= ~BIT5; // disable input pull up/down

  P1IES &= ~BIT5; // Set IFG on rising edge (low --> high)
  P1IFG &= ~BIT5; // Clear flag bit
 
  __delay_cycles(160000);
  lsm_reset();
  // Change to different init mode
  gyro_init_fifo_tap();
  read_tap_src();
  ENABLE(Port_1_ISR);
  STATE_CHANGE(gyro,0x00);
  printf("Testing\r\n");
  #endif
}

void task_init() {
  P1OUT |= BIT1;
  P1DIR |= BIT1;
  P1OUT &= ~BIT1;
  // Trigger a preburst power mode
  DISABLE(Port_1_ISR);
  capybara_transition(0);
  uint8_t temp;
  LOG("task init\r\n");
  // Set the buffer index pointers to initial positions
  val_rindex=0;
  val_windex=0;
  need_cal=0;
  temp = val_windex;
  LOG("val_windex = %u\r\n",temp);
  temp = val_rindex;
  LOG("val_rindex = %u\r\n",temp);
  // Zero out these values
  cnt_over_sigma=0;
  cnt_under_sigma=0;
  count=0;
  iterator=0;
  // Write data into buffer
  for(int i = 0; i < BUFF_LEN; i++) {
    // This was deffo a problem when it was for Port_2
    ENABLE(Port_1_ISR);
    val_buffer[i].x = DATA_SRC[i*3 + 0];
    val_buffer[i].y = DATA_SRC[i*3 + 1];
    val_buffer[i].z = DATA_SRC[i*3 + 2];
    //printf("%i %i %i\r\n",DATA_SRC[i*3 + 0],DATA_SRC[i*3 + 1],DATA_SRC[i*3 +2]);
  }
  DISABLE(Port_1_ISR);TRANSITION_TO(task_val_compare);
}


void task_val_compare() {
  capybara_transition(1);ENABLE(Port_1_ISR);
  P1OUT |= BIT1;
  P1DIR |= BIT1;
  P1OUT &= ~BIT1;
  uint8_t temp1, temp2;
  uint16_t temp3;
  temp1 = val_windex;
  temp2 = val_rindex;
  temp3 = count;
  LOG("wait w:%u r:%u\r\n",temp1, temp2);
  count=temp3 + 1;
#ifdef TEST_TAP
  if(need_cal) {
    need_cal=0;
    PRINTF("got TAP!\r\n");
    //PRINTF("test_val = %u\r\n",test_val);
    DISABLE(Port_1_ISR);TRANSITION_TO(task_cal_avgs);
  }
#endif

  uint8_t rindex = val_rindex;
  // We do not wait for more data because we're running the tasks independent of
  // the gyro's machinations --> just update read index
  if(rindex + 1 == BUFF_LEN) {
    val_rindex=0;
    DISABLE(Port_1_ISR);
    TRANSITION_TO(task_init);
  }
  val_rindex=rindex + 1;
  DISABLE(Port_1_ISR);TRANSITION_TO(task_calc_sqrs);
}

void task_calc_sqrs() {
  capybara_transition(0);ENABLE(Port_1_ISR);
  LOG("tx_begin\r\n");
  uint8_t index = val_rindex;
  uint16_t x,y,z;

  x = val_buffer[index].x;
  y = val_buffer[index].y;
  z = val_buffer[index].z;

  x = absval((int)x);
  y = absval((int)y);
  z = absval((int)z);
  LOG2("|x,y,z|: %x %x %x\r\n",x,y,z);
  uint32_t gx_squared, gy_squared, gz_squared;
  gx_squared = mult16_F16_dec2(x,x);
  gy_squared = mult16_F16_dec2(y,y);
  gz_squared = mult16_F16_dec2(z,z);
  LOG2("|x^2,y^2,z^2|: %l %l %l\r\n",gx_squared,gy_squared,gz_squared);

  // Write the squared values back into the buffer
  buffered_squares[0]=gx_squared;
  buffered_squares[1]=gy_squared;
  buffered_squares[2]=gz_squared;

  DISABLE(Port_1_ISR);TRANSITION_TO(task_calc_angle);
}

void task_calc_angle() {
  capybara_transition(0);ENABLE(Port_1_ISR);
  LOG("calc_angle\r\n");
  uint32_t sum, sx, sy, sz;
  uint16_t cosAngle, sqrt, z;
  uint8_t index;

  // Pull out the squared values we just stored
  index = val_rindex;
  sx = buffered_squares[0];
  sy = buffered_squares[1];
  sz = buffered_squares[2];
  sum = sx + sy + sz;
  ENABLE(Port_1_ISR);

  // Write temp-sum to a shared location
  temp_sum=sum;
  sqrt = sqrt32_bin_search(sum,5);
  z = val_buffer[index].z;
  cosAngle = div16_F16_dec2(z,sqrt);
  LOG("Sqrt= %x, z= %x, angle=%u \r\n",sqrt,z,cosAngle);
  val_cosAngle =cosAngle;

  DISABLE(Port_1_ISR);TRANSITION_TO(task_summary);
}

void task_summary() {
  capybara_transition(0);ENABLE(Port_1_ISR);
  //TODO pull this out! It's only for debugging right now
  //
  LOG2("summary\r\n");
  uint8_t index;
  index = val_rindex;
  uint16_t loc_cosAngle, glob_avg_cosAngle;
  loc_cosAngle = val_cosAngle;
  glob_avg_cosAngle = cal_avg_angle;
  //printf("loc angle = %u, global = %u, index=%u \r\n",loc_cosAngle,
  //glob_avg_cosAngle, index);
  // First check cosAngle > avg
  if(loc_cosAngle > glob_avg_cosAngle) {
    ENABLE(Port_1_ISR);
    uint16_t sigma;
    sigma = cal_std_dev_angle;
    LOG2("Std dev = %u\r\n",sigma);
    if(loc_cosAngle > glob_avg_cosAngle + sigma) {
        cnt_over_sigma = cnt_over_sigma + 1;
    }
  }
  else {
    DISABLE(Port_1_ISR);
    uint16_t sigma;
    sigma = cal_std_dev_angle;
    if(loc_cosAngle < glob_avg_cosAngle - sigma) {
        cnt_under_sigma = cnt_under_sigma + 1;
    }
  }

  // Increment counter of angles
  count =count + 1;
  // Check if we need to send a packet
  LOG("read index: %u\r\n",val_rindex);
  LOG("tx end\r\n");
  // Just go to init
  // Update circular buffer read pointer
  //val_rindex++;
  // Transition to cal_avgs if we have enough data
  DISABLE(Port_1_ISR);TRANSITION_TO(task_val_compare);
  //disable();TRANSITION_TO(task_cal_avgs);
}


// Using tx for first two tasks in these calcs so the read/write indices don't
// get stolen out from under us by the fifo.
void task_cal_avgs() {
  capybara_transition(0);ENABLE(Port_1_ISR);
  LOG("tx cal begin\r\n");
  uint16_t sumx = 0, sumy = 0, sumz= 0;
  uint16_t quox = 0, quoy = 0, quoz= 0;

  // Just hand back control if there isn't enough data.
  uint8_t windex = val_windex;
  uint8_t rindex = val_rindex;
  if((windex > rindex) && (windex - rindex < CAL_LEN)) {
    LOG("not enough data for cal\r\n");
    DISABLE(Port_1_ISR);TRANSITION_TO(task_val_compare);
  }
  if((rindex > windex) && (((BUFF_LEN - rindex) +  windex) < CAL_LEN)) {
    LOG("not enough data for cal\r\n");
    DISABLE(Port_1_ISR);TRANSITION_TO(task_val_compare);
  }
  // Update circular buffer read pointer
  rindex = (rindex + 2 > BUFF_LEN) ? 0 : rindex + 1;
  LOG2("Writing r: %u\r\n", rindex);
  val_rindex=rindex;
  // Get the values
  for(uint8_t i = 0; i < CAL_LEN; i++) {
    sumx += val_buffer[rindex].x;
    sumy += val_buffer[rindex].y;
    sumz += val_buffer[rindex].z;
    rindex = (rindex > BUFF_LEN - 2) ? 0 : rindex + 1;
  }
  LOG("here\r\n");
  // We need to scale the denominator here too
  quox = div16_F16_dec2(sumx, 500);
  quoy = div16_F16_dec2(sumy, 500);
  quoz = div16_F16_dec2(sumz, 500);

  // Record averages
  cal_avg.x= quox;
  cal_avg.y= quoy;
  cal_avg.z= quoz;

  DISABLE(Port_1_ISR);TRANSITION_TO(task_cal_std_devs);
}

void task_cal_std_devs() {
  capybara_transition(0);ENABLE(Port_1_ISR);
  LOG2("cal std devs\r\n");
  uint16_t diffx = 0, diffy = 0, diffz= 0;
  uint16_t quox = 0, quoy = 0, quoz= 0;
  uint16_t stdevx = 0, stdevy = 0, stdevz = 0;
  uint16_t temp;
  uint8_t rindex = val_rindex;

  // calculate diff from average
  for(uint8_t i = 0; i < CAL_LEN; i++) {
    temp = cal_avg.x - val_buffer[rindex].x;
    diffx += absval((int)temp);
    temp = cal_avg.y- val_buffer[rindex].y;
    diffy += absval((int)temp);
    temp = cal_avg.z - val_buffer[rindex].z;
    diffz += absval((int)temp);
    rindex = (rindex > BUFF_LEN - 2) ? 0 : rindex + 1;
  }

  // Calc average diff (variance)
  quox = div16_F16_dec2(diffx, 500);
  quoy = div16_F16_dec2(diffy, 500);
  quoz = div16_F16_dec2(diffz, 500);
  LOG2("incoming = %u \r\n", quox);

  // Calc sqrt of variance
  stdevx = sqrt32_bin_search((uint32_t) quox, 1);
  stdevy = sqrt32_bin_search((uint32_t) quoy, 1);
  stdevz = sqrt32_bin_search((uint32_t) quoz, 1);

  // Write back
  cal_std_dev.x= stdevx;
  cal_std_dev.y= stdevy;
  cal_std_dev.z= stdevz;

  DISABLE(Port_1_ISR);TRANSITION_TO(task_cal_calc_sqrs);
}

void task_cal_calc_sqrs() {
  capybara_transition(0);ENABLE(Port_1_ISR);
  LOG2("cal calc sqrs\r\n");
  uint16_t x,y,z;
  uint8_t rindex = val_rindex;
  // Pull out values and record z's for later
  for(uint8_t i = 0; i < CAL_LEN; i++) {
    x = val_buffer[rindex].x;
    y = val_buffer[rindex].y;
    z = val_buffer[rindex].z;
    buffered_zs[i]=z;

    x = absval((int)x);
    y = absval((int)y);
    z = absval((int)z);
    LOG("cal vals: %x %x %x \r\n",x,y,z);
    uint32_t sum, gx_squared, gy_squared, gz_squared;
    gx_squared = mult16_F16_dec2(x,x);
    gy_squared = mult16_F16_dec2(y,y);
    gz_squared = mult16_F16_dec2(z,z);
    LOG2("cal sqrs: %l %l %l \r\n",gx_squared,gy_squared,gz_squared);
    sum = gx_squared + gy_squared + gz_squared;

    // Record sum of squares for each value
    cal_sums[i]=sum;
    // Don't want to write this on the last iteration
    if(i != CAL_LEN - 1) {
      rindex = (rindex > BUFF_LEN - 2) ? 0 : rindex + 1;
    }
  }
  // Write back new read index
  LOG2("Writing r: %u\r\n", rindex);
  val_rindex=rindex;
  count = count + 5;
  LOG("tx cal end %u\r\n", val_rindex);
  // TODO remove the following lines, it's just for testing!!!!
  if ( count > 10) {
    STATE_CHANGE(test,0x1);
    TRANSITION_TO(task_cal_calc_sqrs);
  }
  if (count > 15) {
    STATE_CHANGE(test,0x2);
    TRANSITION_TO(task_cal_calc_sqrs);
  }
  DISABLE(Port_1_ISR);TRANSITION_TO(task_cal_calc_angles);
}

void task_cal_calc_angles() {
  uint16_t i = iterator;
  for(; i < (iterator + (CAL_LEN >> 2)); i++) {
    uint32_t sum = cal_sums[i];
    uint16_t sqrt = sqrt32_bin_search(sum, 5);
    uint16_t cosAngle = div16_F16_dec2(buffered_zs[i],sqrt);
    cal_angles[i] = cosAngle;
    LOG("Got angle %u, in %u\r\n",cosAngle,i);
  }
  if(i < CAL_LEN) {
    iterator=i;
    DISABLE(Port_1_ISR);TRANSITION_TO(task_cal_calc_angles);
  }
  iterator=0;
  DISABLE(Port_1_ISR);TRANSITION_TO(task_cal_angle_avg);
}

void task_cal_angle_avg() {
  capybara_transition(0);ENABLE(Port_1_ISR);
  LOG2("cal angle avg\r\n");
  LOG2("sums: ");
  uint16_t sum = 0, quo = 0;
  for(uint16_t i = 0; i < CAL_LEN; i++) {
    sum += cal_angles[i];
    LOG2("%u\t",sum);
  }
  LOG2("\r\n");
  quo = div16_F16_dec2(sum, 500);
  LOG("Avg = %u\r\n",quo);
  cal_avg_angle=quo;

  DISABLE(Port_1_ISR);TRANSITION_TO(task_cal_std_dev);
}

void task_cal_std_dev() {
  capybara_transition(0);ENABLE(Port_1_ISR);
  LOG2("cal std dev\r\n");
  uint16_t diffs = 0, quo = 0, temp;
  for(unsigned i = 0; i < CAL_LEN; i++) {
    temp = cal_avg_angle - cal_angles[i];
    diffs += absval((int) temp);
  }
  quo = div16_F16_dec2(diffs, 500);
  quo = sqrt32_bin_search((uint32_t) quo, 1);
  cal_std_dev_angle=quo;
  STATE_CHANGE(test,0x1);
  DISABLE(Port_1_ISR);TRANSITION_TO(task_val_compare);
}


void task_test_pass_reach_front() {
  int x = cal_avg_angle;
  for (int i = 0; i < x; i++) {
    PRINTF("i = %x\r\n",i);
    if ( i % 2 ) {
      PRINTF("odd\r\n");
    }
    else {
      PRINTF("EVEN\r\n");
    }
  }
  if (x > 20) {
    DISABLE(Port_1_ISR);
  }
  uint16_t temp;
  temp = sqrt32_bin_search((uint32_t) x, 1);
  TRANSITION_TO(task_init);
}

void task_test_pass_multiple_blocks() {
  int x = cal_avg_angle;
  for (int i = 0; i < x; i++) {
    PRINTF("i = %x\r\n",i);
    if ( i % 2 ) {
      PRINTF("odd\r\n");
    }
    else {
      PRINTF("EVEN\r\n");
    }
  }
  if (x > 20) {
    DISABLE(Port_1_ISR);
  }
  uint16_t temp;
  temp = sqrt32_bin_search((uint32_t) x, 1);
  TRANSITION_TO(task_init);
}

void task_test_pass_multiple_ISRS() {
  int x = cal_avg_angle;
  ENABLE(Port_2_ISR);
  for (int i = 0; i < x; i++) {
    PRINTF("i = %x\r\n",i);
    if ( i % 2 ) {
      ENABLE(Port_1_ISR);
      PRINTF("odd\r\n");
    }
    else {
      PRINTF("EVEN\r\n");
    }
  }
  if (x > 20) {
    DISABLE(Port_1_ISR);
    DISABLE(Port_2_ISR);
  }
  uint16_t temp;
  temp = sqrt32_bin_search((uint32_t) x, 1);
  TRANSITION_TO(task_init);
}

ENTRY_TASK(task_init)
INIT_FUNC(init)



