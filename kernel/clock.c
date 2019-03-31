/* clock.c - System timer */

#include <clock.h>
#include <aarch64/cpu.h>
#include <aarch64/cpureg.h>
#include <klibc.h>
#include <rpi/irq.h>

#define CLK_FREQ 19200000

static uint32_t ticks = 0;

static void clock_irq_org(unsigned irq __unused) {
  uint32_t val = reg_cntp_tval_el0_read();
  reg_cntp_tval_el0_write(val + CLK_FREQ);
  reg_cntp_ctl_el0_write(CNTCTL_ENABLE);

  arm_isb();
  ticks++;
  printf("tick %d!\n", ticks);
}

void clock_init(void) {

  //reg_cntvct_el0_write(CLK_FREQ);
  reg_cntp_tval_el0_write(CLK_FREQ);
  reg_cntp_ctl_el0_write(CNTCTL_ENABLE);

  arm_isb();

  /* Enable CP0 physical timer interrupt. */
  bcm2836_local_irq_register(BCM2836_INT_CNTPSIRQ, clock_irq_org);
  bcm2836_local_irq_enable(BCM2836_INT_CNTPSIRQ);

  bcm2836_local_irq_register(BCM2836_INT_CNTPNSIRQ, clock_irq_org);
  bcm2836_local_irq_enable(BCM2836_INT_CNTPNSIRQ);

}


