#include <cdefs.h>
#include <armreg.h>
#include <bcm2836reg.h>
#include <mmio.h>
#include <klibc.h>
#include <pcpu.h>
#include <smp.h>

#define MAILBOX_IRQ_CTRL_N(x) \
  (BCM2836_ARM_LOCAL_BASE + BCM2836_LOCAL_MAILBOX_IRQ_CONTROLN(x))
#define MAILBOX3_SET_N(x) \
  (BCM2836_ARM_LOCAL_BASE + BCM2836_LOCAL_MAILBOX3_SETN(x))

extern void cons_bootstrap(unsigned);

static void smp_entry(uint32_t r0 __unused, uint32_t r1 __unused,
                      uint32_t atags __unused) {
  pcpu_init();
  cons_bootstrap(smp_cpu_id());
  printf("CPU#%d started!\n", smp_cpu_id());
  for (;;);
}

void smp_bootstrap() {
  for (int cpu = 1; cpu < 4; cpu++) {
    mmio_write(MAILBOX3_SET_N(cpu), (uint32_t)smp_entry);
  }
}
