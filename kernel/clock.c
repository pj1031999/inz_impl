/* clock.c - System timer */

#include <aarch64/cpu.h>
#include <aarch64/cpureg.h>
#include <klibc.h>
#include <rpi/irq.h>

#define CLK_FREQ 19200000

static uint32_t ticks = 0;

static void clock_irq(unsigned irq __unused) {
  uint32_t val = reg_cntp_tval_el0_read();
  reg_cntp_tval_el0_write(val + CLK_FREQ);
  reg_cntp_ctl_el0_write(CNTCTL_ENABLE);

  arm_isb();
  ticks++;
  printf("tick %d!\n", ticks);
}


void clock_init(void) {

#if 1

  //reg_cntvct_el0_write(CLK_FREQ);
  reg_cntp_tval_el0_write(CLK_FREQ);
  reg_cntp_ctl_el0_write(CNTCTL_ENABLE);

  arm_isb();

  /* Enable CP0 physical timer interrupt. */
  bcm2836_local_irq_register(BCM2836_INT_CNTPSIRQ, clock_irq);
  bcm2836_local_irq_enable(BCM2836_INT_CNTPSIRQ);

 
#else
  const unsigned int interval = 9600000;
  unsigned int curVal = 0;

#define DEVICE_BASE 		0x3F000000	
#define TIMER_CLO       (DEVICE_BASE+0x00003004)
#define TIMER_C1        (DEVICE_BASE+0x00003010)
 
  curVal = get32(TIMER_CLO);
  curVal += interval;
  put32(TIMER_C1, curVal);

  gen_timer_init();
  gen_timer_reset();

#define TIMER_INT_CTRL_0    (0x40000040)
#define TIMER_INT_CTRL_0_VALUE  (1 << 1)
#define ENABLE_IRQS_1		(DEVICE_BASE+0x0000B210)
#define SYSTEM_TIMER_IRQ_1	(1 << 1)
  
  put32(ENABLE_IRQS_1, SYSTEM_TIMER_IRQ_1);
  put32(TIMER_INT_CTRL_0, TIMER_INT_CTRL_0_VALUE);
  enable_irq();

#endif
}
