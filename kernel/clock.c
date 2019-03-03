/* clock.c - System timer */

#ifdef AARCH64
#include <aarch64/cpu.h>
#else
#include <arm/cpu.h>
#endif

#include <klibc.h>
#include <rpi/irq.h>

#define CLK_FREQ 19200000

static uint32_t ticks = 0;

static void clock_irq(unsigned irq __unused) {

#ifdef AARCH64
  uint32_t val = reg_cntp_tval_el0_read();
  reg_cntp_tval_el0_write(val + CLK_FREQ);
  reg_cntp_ctl_el0_write(CNTCTL_ENABLE);

#else
  uint32_t val = armreg_cntp_tval_read();
  armreg_cntp_tval_write(val + CLK_FREQ);
  armreg_cntp_ctl_write(ARM_CNTCTL_ENABLE);

#endif

  arm_isb();
  ticks++;
  printf("tick %d!\n", ticks);
}

void clock_init(void) {
#ifdef AARCH64
  reg_cntfrq_el0_write(CLK_FREQ);
  reg_cntp_tval_el0_write(CLK_FREQ);
  reg_cntp_ctl_el0_write(CNTCTL_ENABLE);

#else
  armreg_cnt_frq_write(CLK_FREQ);
  armreg_cntp_tval_write(CLK_FREQ);
  armreg_cntp_ctl_write(ARM_CNTCTL_ENABLE);

#endif

  arm_isb();

  /* Enable CP0 physical timer interrupt. */
  bcm2836_local_irq_register(BCM2836_INT_CNTPSIRQ, clock_irq);
  bcm2836_local_irq_enable(BCM2836_INT_CNTPSIRQ);
}
