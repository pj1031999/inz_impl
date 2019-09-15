/* clock.c - System timer */

#include <aarch64/cpu.h>
#include <klibc.h>
#include <rpi/irq.h>
#include <pmman.h>
#include <pmap_test.h>

static uint32_t ticks = 0;
static uint64_t clk_freq = 0;

static void clock_irq(unsigned irq __unused) {
  uint64_t val = reg_cntp_cval_el0_read();
  reg_cntp_cval_el0_write(val + clk_freq);

  arm_isb();
  ticks++;
  
  printf("\t tick %d!\n", ticks);
  pcpu()->switch_to = ticks%2;
}

void clock_init(void) {

  clk_freq = reg_cntfrq_el0_read();
  reg_cntp_cval_el0_write(clk_freq);	// compare value
  reg_cntp_ctl_el0_write(CNTCTL_ENABLE);// enable counter and interrupt generation
  arm_isb();

  bcm2836_local_irq_register(BCM2836_INT_CNTPNSIRQ, clock_irq);
  bcm2836_local_irq_enable(BCM2836_INT_CNTPNSIRQ);

}


