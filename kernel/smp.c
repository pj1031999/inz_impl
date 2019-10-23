#include <types.h>
#include <klibc.h>
#include <pcpu.h>
#include <arm/mbox.h>
#include <rpi/irq.h>
#include <smp.h>
#include <aarch64/cpu.h>
#include <pmman.h>
#include <demo/demo.h>

extern void cons_bootstrap(unsigned);
extern const void *_stack_size;

static inline uint64_t stack_addr_read() {
  uint64_t data;
  __asm__("mov %[data], sp" : [ data ] "=r"(data));
  return data;
}

void smp_intro() {

  unsigned cpu = arm_cpu_id();
  pcpu_init();
  cons_bootstrap(cpu);

  printf("CPU#%d started!\n", cpu);
  printf("Stack Pointer: %p\n", stack_addr_read());
  mbox_set(0, 3, __BIT(cpu));
}

void demo_none() {
}

#define smp_demo(foo, irq)                                                     \
  void smp_demo_##foo() {                                                      \
    smp_intro();                                                               \
    if (irq) {                                                                 \
      bcm2835_irq_init();                                                      \
      bcm2836_local_irq_init();                                                \
      arm_irq_enable();                                                        \
    }                                                                          \
    demo_##foo();                                                              \
    kernel_exit();                                                             \
  }

smp_demo(led, false);
smp_demo(gpio, false);
smp_demo(pmap, false);
smp_demo(clock, true);
smp_demo(uart, false);
smp_demo(sd, false);
smp_demo(none, false);

void smp_bootstrap() {
#define L2I (uint32_t)(uint64_t)

  int cpu = 0;
  mbox_send(cpu + 1, 3, L2I smp_demo_clock);
  mbox_send(cpu + 2, 3, L2I smp_demo_sd);
  mbox_send(cpu + 3, 3, L2I smp_demo_pmap);

  pm_alloc(1 * L2I & _stack_size);
  vm_alloc(1 * L2I & _stack_size);
  paddr_t s1 = pm_alloc(L2I & _stack_size);
  paddr_t s2 = pm_alloc(L2I & _stack_size);
  paddr_t s3 = pm_alloc(L2I & _stack_size);

  mbox_send(cpu + 1, 1, s1);
  mbox_send(cpu + 2, 1, s2);
  mbox_send(cpu + 3, 1, s3);

  do {
    __asm__ volatile("wfe");
    __asm__ volatile("sev");
  } while (mbox_recv(0, 3) != (__BIT(3) | __BIT(2) | __BIT(1)));

  mbox_clr(0, 3, -1);
}
