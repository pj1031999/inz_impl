#include <cdefs.h>
#include <bcm2836.h>
#include <mmio.h>

#define TIMER_IRQ_CTRL_N(x) \
  (BCM2836_ARM_LOCAL_BASE + BCM2836_LOCAL_TIMER_IRQ_CONTROLN(x))
#define INTC_IRQPENDING_N(x) \
  (BCM2836_ARM_LOCAL_BASE + BCM2836_LOCAL_INTC_IRQPENDINGN(x))

void bcm2836_local_timer_irq_enable(unsigned cpunum, unsigned irqnum) {
  mmio_write(TIMER_IRQ_CTRL_N(cpunum), __BIT(irqnum));
}

bool bcm2836_local_irq_pending_p(unsigned cpunum, unsigned irqnum) {
  return mmio_read(INTC_IRQPENDING_N(cpunum)) & __BIT(irqnum);
}
