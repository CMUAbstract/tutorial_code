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


typedef struct xl_val_ {
  uint16_t x;
  uint16_t y;
  uint16_t z;
} xl_val;

static const uint16_t DATA_SRC[60] = {
#include "extra_data.h"
};

// NV variables that will be handled by coati
static const xl_val val_buffer[BUFF_LEN] =  { 
  #include "extra_data2.h"
  };
__nv uint8_t val_rindex = 0;
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

__nv capybara_task_cfg_t pwr_configs[3] = {
  CFG_ROW(0, CONFIGD, LOWP, LOWP),
  CFG_ROW(1, BURST, LOWP, BACKP),
  CFG_ROW(2, PREBURST, LOWP, BACKP),
};

void disable() {
  P1IE &= ~BIT4; //disable interrupt bit
  P1IE &= ~BIT5;
  return;
}

//#define TEST_VDDSENSE
//#define TEST_FIFO_OFF
#define FIFO_BYTES 48

void enable() {
#ifndef VDD_SENSE
  P1IE |= BIT4; //enable interrupt bit
  P1IE |= BIT5;
#endif
  return;
}


__nv uint8_t array[3][FIFO_BYTES];
__nv uint8_t array1[3][FIFO_BYTES];

void __attribute__((interrupt(0))) Port_1_ISR(void)
{   //Need to clear whatever interrupt flag fired to get us here!
  // Clear LSM_INT1 on processor side
  disable();
#ifndef TEST_VDDSENSE
  P1OUT |= BIT2;
  P1DIR |= BIT2;
  P1OUT &= ~BIT2;
  //printf("Here now\r\n");
  if(P1IFG & BIT4) {
    P1IFG &= ~BIT4;
    uint8_t fifolvl = read_fifo_lvl();
    read_tap_src();
    //printf("Here1\r\n");
  }
  // Clear LSM_INT2, note, we'll leave INT1 as our higher priority and handle
  // that first, but then we can fall into this statement
#ifndef TEST_FIFO_OFF
  if(P1IFG & BIT5) {
    P1IFG &= ~BIT5;
    uint8_t fifolvl = read_fifo_lvl();
    if(fifolvl < FIFO_THR - 1) {
      // reset fifo and return without registering a bottom half
      //printf("Here2\r\n");
      fifo_reset();
      return;
    }
    dump_fifo(array, FIFO_THR);
    dump_fifo_high(array1,FIFO_THR);
    fifo_reset();
    //printf("Here3\r\n");
  }
  P1IE |= BIT5;
#endif
  //printf("enable\r\n");
  P1IE |= BIT4; //enable interrupt bit
#endif
  return;
}
__attribute__((section("__interrupt_vector_port1"),aligned(2)))
void (*__vector_port1)(void) = Port_1_ISR;

void init() {
  capybara_init();
  fxl_set(BIT_SENSE_SW);
  //TODO figure out how to better manage this delay
  P1OUT |= BIT0;
  P1DIR |= BIT0;
  P1OUT &= ~BIT0; 
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
  enable();
  //#else
  // __delay_cycles(179040);
  #endif
  P1OUT |= BIT0;
  P1DIR |= BIT0;
  P1OUT &= ~BIT0; 
  val_rindex = 0;
  __delay_cycles(160000);
  __delay_cycles(160000);
  __delay_cycles(160000);
  __delay_cycles(160000);
  TRANSITION_TO(task_val_compare);
  //printf("init\r\n");
}

void task_init() {
  disable();capybara_transition(0);
  // Set the buffer index pointers to initial positions
  val_rindex=0;
  //printf("done task init\r\n");
  disable();TRANSITION_TO(task_val_compare);
}

void task_val_compare() {
  capybara_transition(0);enable();
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
    disable();TRANSITION_TO(task_cal_avgs);
  }
#endif

  uint8_t rindex = val_rindex;
  // We do not wait for more data because we're running the tasks independent of
  // the gyro's machinations --> just update read index
  if(rindex + 1 == BUFF_LEN) {
    val_rindex=0;
  }
  else {
    val_rindex=rindex + 1;
  }
 // printf("done task val compare\r\n");
  disable();TRANSITION_TO(task_calc_sqrs);
}

void task_calc_sqrs() {
  capybara_transition(0);enable();
  LOG("tx_begin\r\n");
  uint8_t index = val_rindex;
  uint16_t x,y,z;
  /*val_buffer[index].x = DATA_SRC[index*3 + 0];
  val_buffer[index].y = DATA_SRC[index*3 + 1];
  val_buffer[index].z = DATA_SRC[index*3 + 2];*/
  //printf("Sucked in data\r\n");

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

  disable();TRANSITION_TO(task_calc_angle);
}

void task_calc_angle() {
  capybara_transition(0);enable();
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

  // Write temp-sum to a shared location
  temp_sum=sum;
  sqrt = sqrt32_bin_search(sum,5);
  z = val_buffer[index].z;
  cosAngle = div16_F16_dec2(z,sqrt);
  LOG("Sqrt= %x, z= %x, angle=%u \r\n",sqrt,z,cosAngle);
  val_cosAngle =cosAngle;

  disable();TRANSITION_TO(task_summary);
}

void task_summary() {
  capybara_transition(0);enable();
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
    uint16_t sigma;
    sigma = cal_std_dev_angle;
    LOG2("Std dev = %u\r\n",sigma);
    if(loc_cosAngle > glob_avg_cosAngle + sigma) {
        cnt_over_sigma = cnt_over_sigma + 1;
    }
  }
  else {
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
  disable();TRANSITION_TO(task_val_compare);
  //disable();TRANSITION_TO(task_cal_avgs);
}



ENTRY_TASK(task_init)
INIT_FUNC(init)



