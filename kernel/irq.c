#include <cdefs.h>
#include <klibc.h>
#include <irq.h>
#include <mmio.h>

#define BCM2835_ARMICU(x) \
  BCM2835_PERIPHERALS_BUS_TO_PHYS(BCM2835_ARMICU_BASE + (x))

#define CHOOSE_IRQ_REGISTER(X) \
  uint32_t reg; \
  assert(irq < BCM2835_NIRQ + BCM2836_NIRQ); \
  if (irq >= BCM2835_INT_BASICBASE) { \
    reg = BCM2835_ARMICU(BCM2835_INTC_IRQB ## X); \
    irq -= BCM2835_INT_BASICBASE; \
  } else if (irq >= BCM2835_INT_GPU1BASE) { \
    reg = BCM2835_ARMICU(BCM2835_INTC_IRQ2 ## X); \
    irq -= BCM2835_INT_GPU1BASE; \
  } else { \
    reg = BCM2835_ARMICU(BCM2835_INTC_IRQ1 ## X); \
    irq -= BCM2835_INT_GPU0BASE; \
  }

void bcm2835_irq_enable(unsigned irq) {
  CHOOSE_IRQ_REGISTER(ENABLE);
  mmio_write(reg, mmio_read(reg) | __BIT(irq));
}

void bcm2835_irq_disable(unsigned irq) {
  CHOOSE_IRQ_REGISTER(DISABLE);
  mmio_write(reg, mmio_read(reg) & ~__BIT(irq));
}

bool bcm2835_irq_pending_p(unsigned irq) {
  CHOOSE_IRQ_REGISTER(PENDING);
  return mmio_read(reg) & __BIT(irq);
}

#define TIMER_IRQ_CTRL_N(x) \
  (BCM2836_ARM_LOCAL_BASE + BCM2836_LOCAL_TIMER_IRQ_CONTROLN(x))
#define INTC_IRQPENDING_N(x) \
  (BCM2836_ARM_LOCAL_BASE + BCM2836_LOCAL_INTC_IRQPENDINGN(x))

void bcm2836_local_timer_irq_enable(unsigned cpu, unsigned irq) {
  uint32_t reg = TIMER_IRQ_CTRL_N(cpu);
  mmio_write(reg, mmio_read(reg) | __BIT(irq));
}

void bcm2836_local_timer_irq_disable(unsigned cpu, unsigned irq) {
  uint32_t reg = TIMER_IRQ_CTRL_N(cpu);
  mmio_write(reg, mmio_read(reg) & ~__BIT(irq));
}

bool bcm2836_local_irq_pending_p(unsigned cpu, unsigned irq) {
  return mmio_read(INTC_IRQPENDING_N(cpu)) & __BIT(irq);
}
