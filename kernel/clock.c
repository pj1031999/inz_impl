/* clock.c - System timer */

#include <aarch64/cpu.h>
#include <klibc.h>
#include <rpi/irq.h>
#include <pmman.h>
#include <pmap_test.h>

#define CLK_FREQ 192000000

static uint32_t ticks = 0;
static uint64_t clk_freq = 0;


static void clock_irq(unsigned irq __unused) {
  uint64_t val = reg_cntp_cval_el0_read();
  reg_cntp_cval_el0_write(val + clk_freq);
  reg_cntp_ctl_el0_write(CNTCTL_ENABLE);

  arm_isb();
  ticks++;

  pmap_test_kextract((ticks-1) * 0x000080000 + 0xffffFFFF00054321);
  printf("tick %d!\n", ticks);
}

void clock_init(void) {

  //reg_cntvct_el0_write(CLK_FREQ);
  clk_freq = reg_cntfrq_el0_read();
  reg_cntp_cval_el0_write(clk_freq );
  reg_cntp_ctl_el0_write(CNTCTL_ENABLE);

  arm_isb();

  /* Enable CP0 physical timer interrupt. */
  bcm2836_local_irq_register(BCM2836_INT_CNTPSIRQ, clock_irq);
  bcm2836_local_irq_enable(BCM2836_INT_CNTPSIRQ);

  bcm2836_local_irq_register(BCM2836_INT_CNTPNSIRQ, clock_irq);
  bcm2836_local_irq_enable(BCM2836_INT_CNTPNSIRQ);

}


