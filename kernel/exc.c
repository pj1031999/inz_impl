#ifdef AARCH64
#include <aarch64/cpureg.h>

#else
#include <arm/cpureg.h>
#endif

#include <klibc.h>

__interrupt("UNDEF") void exc_undefined_instruction(void) {
}

__interrupt("SWI") void exc_supervisor_call(void) {
}

__interrupt("ABORT") void exc_prefetch_abort(void) {
}

__interrupt("ABORT") void exc_data_abort(void) {

#ifdef AARCH64
  uint32_t dfar = reg_far_el1_read();
  uint32_t dfsr = reg_esr_el1_read();
#else
  uint32_t dfar = armreg_dfar_read();
  uint32_t dfsr = armreg_dfsr_read();
#endif

#define FAULT_WRITE	0x0800	/* fault was due to write (ARMv6+) */
#define FAULT_TYPE_MASK 0x0f
  printf("Illegal %s access at %p (type=%x)!\n",
         (dfsr & FAULT_WRITE) ? "write" : "read", dfar, dfsr & FAULT_TYPE_MASK);
}

static uint32_t ticks = 0;
__interrupt("ABORT") void  clock_irq(void) {
#define CLK_FREQ 192000000

  uint32_t val = reg_cntp_tval_el0_read();
  reg_cntp_tval_el0_write(val + CLK_FREQ);
  reg_cntp_ctl_el0_write(CNTCTL_ENABLE);

  //arm_isb();
  ticks++;
  printf("tick %d!\n", ticks);
}
