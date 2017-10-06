/* clock.c - System timer */

#include <klibc.h>
#include <arm/cpu.h>
#include <irq.h>

#define CLK_FREQ 19200000

static uint32_t ticks = 0;

static void clock_irq(unsigned irq __unused) {
  uint32_t val = armreg_cntp_tval_read();
  armreg_cntp_tval_write(val + CLK_FREQ);
  armreg_cntp_ctl_write(ARM_CNTCTL_ENABLE);
  arm_isb();

  ticks++;

  printf("tick %d!\n", ticks);
}

void clock_init(void) {
  armreg_cnt_frq_write(CLK_FREQ);
  armreg_cntp_tval_write(CLK_FREQ);
  armreg_cntp_ctl_write(ARM_CNTCTL_ENABLE);
  arm_isb();

  /* Enable CP0 physical timer interrupt. */
  bcm2836_local_irq_register(BCM2836_INT_CNTPSIRQ, clock_irq);
  bcm2836_local_irq_enable(BCM2836_INT_CNTPSIRQ);
}
