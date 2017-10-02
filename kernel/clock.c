/* clock.c - System timer */

#include <klibc.h>
#include <arm.h>
#include <armreg.h>
#include <irq.h>

#define CLK_FREQ 19200000

void clock_init(void) {
  armreg_cnt_frq_write(CLK_FREQ);
  armreg_cntp_tval_write(CLK_FREQ);
  armreg_cntp_ctl_write(ARM_CNTCTL_ENABLE);
  arm_isb();

  /* Enable CP0 physical timer interrupt. */
  bcm2836_local_timer_irq_enable(0, BCM2836_INT_CNTPSIRQ);
  /* Enable interrupts. */
  arm_set_cpsr_c(I32_bit, 0);
}

static uint32_t ticks = 0;

bool clock_irq(void) {
  if (!bcm2836_local_irq_pending_p(0, BCM2836_INT_CNTPSIRQ))
    return false;

  uint32_t val = armreg_cntp_tval_read();
  armreg_cntp_tval_write(val + CLK_FREQ);
  armreg_cntp_ctl_write(ARM_CNTCTL_ENABLE);
  arm_isb();

  ticks++;

  printf("tick %d!\n", ticks);
  return true;
}
