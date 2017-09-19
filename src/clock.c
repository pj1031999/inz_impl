/* clock.c - System timer */

#include <klibc.h>
#include <arm.h>
#include <armreg.h>
#include <mmio.h>
#include <bcm2836reg.h>

#define CLK_FREQ (1000 * 1000 * 1000) 
#define CLK_PERIOD (1000 * 1000 * 20)

#define TIMER_IRQ_CTRL_N(x) \
  (BCM2836_ARM_LOCAL_BASE + BCM2836_LOCAL_TIMER_IRQ_CONTROLN(x))
#define INTC_IRQPENDING_N(x) \
  (BCM2836_ARM_LOCAL_BASE + BCM2836_LOCAL_INTC_IRQPENDINGN(x))

void clock_init(void) {
  armreg_cnt_frq_write(CLK_FREQ);
  armreg_cntp_tval_write(CLK_PERIOD);
  armreg_cntp_ctl_write(ARM_CNTCTL_ENABLE);
  arm_isb();

  /* Enable CP0 physical timer interrupt. */
  mmio_write(TIMER_IRQ_CTRL_N(0), __BIT(BCM2836_INT_CNTPSIRQ));
  /* Enable interrupts. */
  arm_set_cpsr_c(I32_bit, 0);
}

static uint32_t ticks = 0;

void clock_irq(void) {
  if (!mmio_read(INTC_IRQPENDING_N(0)))
    return;

  uint32_t val = armreg_cntp_tval_read();
  armreg_cntp_tval_write(val + CLK_PERIOD);
  armreg_cntp_ctl_write(ARM_CNTCTL_ENABLE);
  arm_isb();

  ticks++;

  kprintf("tick %d!\n", ticks);
}
