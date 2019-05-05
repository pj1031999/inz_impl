#include <aarch64/cpu.h>
#include <types.h>
#include <klibc.h>
#include <rpi/irq.h>
#include <mmio.h>

static irq_handler_t _handler_vec[BCM2835_NIRQ + BCM2836_NIRQ];

/* BCM2835 interrupt handling (peripherals) */

#define BCM2835_ARMICU(x) \
  BCM2835_PERIPHERALS_BUS_TO_PHYS(BCM2835_ARMICU_BASE + (x))

#define CHECK_IRQ() \
  assert(irq >= BCM2836_NIRQ); \
  assert(irq < BCM2835_NIRQ + BCM2836_NIRQ)

#define CHOOSE_IRQ_REGISTER(X) \
  uint32_t reg; \
  CHECK_IRQ(); \
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

void bcm2835_irq_init(void) {
/* #define PERIPHERAL_BASE 0x3F000000 */
/* #define INTERRUPTS_PENDING (INTERRUPTS_BASE + 0x200) */
  
/*   interrupt_regs->irq_basic_disable = -1; */
/*   interrupt_regs->irq_gpu_disable1 = -1; */
/*   interrupt_regs->irq_gpu_disable2 = -1; */
  mmio_write(BCM2835_ARMICU(BCM2835_INTC_IRQBDISABLE), -1);
  mmio_write(BCM2835_ARMICU(BCM2835_INTC_IRQ1DISABLE), -1);
  mmio_write(BCM2835_ARMICU(BCM2835_INTC_IRQ2DISABLE), -1);
}

void bcm2835_irq_register(unsigned irq, irq_handler_t handler) {
  CHECK_IRQ();
  _handler_vec[irq] = handler;
}

void bcm2835_irq_enable(unsigned irq) {
  CHOOSE_IRQ_REGISTER(ENABLE);
  mmio_write(reg, mmio_read(reg) | __BIT(irq));
}

void bcm2835_irq_disable(unsigned irq) {
  CHOOSE_IRQ_REGISTER(DISABLE);
  mmio_write(reg, mmio_read(reg) & ~__BIT(irq));
}

/* BCM2836 interrupt handling (core private interrupts) */

#define TIMER_IRQ_CTRL_N(x) \
  (BCM2836_ARM_LOCAL_BASE + BCM2836_LOCAL_TIMER_IRQ_CONTROLN(x))
#define MAILBOX_IRQ_CTRL_N(x) \
  (BCM2836_ARM_LOCAL_BASE + BCM2836_LOCAL_MAILBOX_IRQ_CONTROLN(x))
#define INTC_IRQPENDING_N(x) \
  (BCM2836_ARM_LOCAL_BASE + BCM2836_LOCAL_INTC_IRQPENDINGN(x))

#define CHECK_LOCAL_IRQ() \
  assert(irq <= BCM2836_INT_MAILBOX3)

#define CHOOSE_LOCAL_IRQ_REGISTER() \
  uint32_t reg; \
  CHECK_LOCAL_IRQ(); \
  if (irq >= BCM2836_INT_MAILBOX0) { \
    reg = MAILBOX_IRQ_CTRL_N(cpu); \
    irq -= BCM2836_INT_MAILBOX0; \
  } else { \
    reg = TIMER_IRQ_CTRL_N(cpu); \
  }

void bcm2836_local_irq_init() {
  unsigned cpu = arm_cpu_id();
  mmio_write(TIMER_IRQ_CTRL_N(cpu), 0);
  mmio_write(MAILBOX_IRQ_CTRL_N(cpu), 0);
}

void bcm2836_local_irq_register(unsigned irq, irq_handler_t handler) {
  unsigned cpu = arm_cpu_id();
  CHECK_LOCAL_IRQ();
  _handler_vec[BCM2836_INT_BASECPUN(cpu) + irq] = handler;
}

void bcm2836_local_irq_enable(unsigned irq) {
  unsigned cpu = arm_cpu_id();
  CHOOSE_LOCAL_IRQ_REGISTER();
  mmio_write(reg, mmio_read(reg) | __BIT(irq));
}

void bcm2836_local_irq_disable(unsigned irq) {
  unsigned cpu = arm_cpu_id();
  CHOOSE_LOCAL_IRQ_REGISTER();
  mmio_write(reg, mmio_read(reg) & ~__BIT(irq));
}

/* interrupt handlers entry points */

static void irq_dispatch(uint32_t reg, unsigned irq_base) {
  uint32_t pending = mmio_read(reg);
  unsigned irq;

  /* __builtin_ffs returns one plus the index of the least significant
   * 1-bit of x, or if x is zero, returns zero. */
  while ((irq = __builtin_ffs(pending))) {
    irq -= 1;

    unsigned _irq = irq_base + irq;

    irq_handler_t handler = _handler_vec[_irq];
    if (handler) {
      handler(_irq);
    } else {
      printf("Spurious interrupt %d!\n", _irq);
    }

    pending &= ~__BIT(irq);
  }
}

void exc_irq(void) {
  unsigned cpu = arm_cpu_id();
  /* Firstly, dispatch local interrupts. */
  irq_dispatch(INTC_IRQPENDING_N(cpu), BCM2836_INT_BASECPUN(cpu));

  if (cpu == 0) {
    irq_dispatch(BCM2835_ARMICU(BCM2835_INTC_IRQBPENDING),
                 BCM2835_INT_BASICBASE);
    irq_dispatch(BCM2835_ARMICU(BCM2835_INTC_IRQ1PENDING),
                 BCM2835_INT_GPU0BASE);
    irq_dispatch(BCM2835_ARMICU(BCM2835_INTC_IRQ2PENDING),
                 BCM2835_INT_GPU1BASE);
  }
}

void exc_fast_irq(void) {
  panic("System must not generate Fast Interrupts!");
}
