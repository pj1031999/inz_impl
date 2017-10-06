#include <cdefs.h>
#include <klibc.h>
#include <pcpu.h>
#include <arm/mbox.h>
#include <irq.h>
#include <smp.h>

extern void cons_bootstrap(unsigned);

static void smp_entry(uint32_t r0 __unused, uint32_t r1 __unused,
                      uint32_t atags __unused) {
  unsigned cpu = arm_cpu_id();
  pcpu_init();
  cons_bootstrap(cpu);
  bcm2836_local_irq_init();
  arm_irq_enable();

  printf("CPU#%d started!\n", cpu);
  mbox_set(0, 3, __BIT(cpu));
  for (;;);
}

void smp_bootstrap() {
  for (int cpu = 1; cpu < 4; cpu++)
    mbox_send(cpu, 3, (uint32_t)smp_entry);

  do {
    __asm__ volatile("wfe");
  } while (mbox_recv(0, 3) != (__BIT(3) | __BIT(2) | __BIT(1)));

  mbox_clr(0, 3, -1);
}
