#include "pacarana.h"

/*
  Including libalpaca here is less than ideal, but to hack together a function
  that can correctly transition for the given runtime, we need a way to access
  those struct definitions... and globals for double buffering
*/
#include "libalpaca/alpaca.h"
void __pacarana_to_alpaca_transition(void *next_task) {
	context_t *next_ctx;
	next_ctx = (curctx == &context_0 ? &context_1 : &context_0);
	next_ctx->task = &next_task;
	next_ctx->numRollback = 0;
	curctx = next_ctx;
	return;
}


__nv uint8_t nv_buff[CHKPTLEN];
volatile uint8_t vol_buff[CHKPTLEN];
__nv uint16_t  regs[16];

void checkpoint_test() {
	__asm__ volatile ("MOV 0(R1), %[nt]\n"
                      : /* no outputs */
                      : [nt] "r" (regs[0])
                      );//r0
	__asm__ volatile ("MOV R2, %[nt]\n"
                      : /* no outputs */
                      : [nt] "r" (regs[1])
                      );//r2

  __asm__ volatile ("MOVX.A R4, %[nt]"
                    : /* no outputs */
                    : [nt] "r" (regs[2])
                    );//r0
  __asm__ volatile ("MOVX.A R5, %[nt]"
                    : /* no outputs */
                    : [nt] "r" (regs[3])
                    );//r5
  __asm__ volatile ("MOVX.A R6, %[nt]"
                    : /* no outputs */
                    : [nt] "r" (regs[4])
                    );//r5
  __asm__ volatile ("MOVX.A R7, %[nt]"
                    : /* no outputs */
                    : [nt] "r" (regs[5])
                    );//r5
  __asm__ volatile ("MOVX.A R8, %[nt]"
                    : /* no outputs */
                    : [nt] "r" (regs[6])
                    );//r5
  __asm__ volatile ("MOVX.A R9, %[nt]"
                    : /* no outputs */
                    : [nt] "r" (regs[7])
                    );//r5
  __asm__ volatile ("MOVX.A R10, %[nt]"
                    : /* no outputs */
                    : [nt] "r" (regs[8])
                    );//r5
  __asm__ volatile ("MOVX.A R11, %[nt]"
                    : /* no outputs */
                    : [nt] "r" (regs[9])
                    );//r5
  __asm__ volatile ("MOVX.A R12, %[nt]"
                    : /* no outputs */
                    : [nt] "r" (regs[10])
                    );//r5
  __asm__ volatile ("MOVX.A R13, %[nt]"
                    : /* no outputs */
                    : [nt] "r" (regs[11])
                    );//r5
  __asm__ volatile ("MOVX.A R14, %[nt]"
                    : /* no outputs */
                    : [nt] "r" (regs[12])
                    );//r5
  __asm__ volatile ("MOVX.A R15, %[nt]"
                    : /* no outputs */
                    : [nt] "r" (regs[13])
                    );//r5
  __asm__ volatile ("ADD #4, R1");
  __asm__ volatile ("MOVX.A R1, %[nt]"
                    : /* no outputs */
                    : [nt] "r" (regs[14])
                    );//r5
  __asm__ volatile ("SUB #4, R1");
  for(uint16_t i = 0; i < CHKPTLEN; i++) {
    nv_buff[i] = vol_buff[i];
  }
  return;
}

void set_vol_buff() {
  for(uint16_t i = 0; i < CHKPTLEN; i++) {
    //vol_buff[i] = i & 0xFF;
    vol_buff[i] = 0xFF;
  }
  return;
}

