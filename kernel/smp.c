#include <types.h>
#include <klibc.h>
#include <pcpu.h>
#include <arm/mbox.h>
#include <rpi/irq.h>
#include <smp.h>
#include <aarch64/cpu.h>
#include <rpi/gpio.h>
#include <rpi/vc_mbox.h>
#include <pmman.h>
#include <userspace_demo.h>

extern void cons_bootstrap(unsigned);
extern const void* _stack_size;
extern const void* _el1_stack;

static inline uint64_t stack_addr_read() {
  uint64_t data;
  __asm__ ("mov %[data], sp"
                   : [data] "=r"(data)
                   : );
  return data;
}

static inline void smp_intro(){

  unsigned cpu = arm_cpu_id();
  pcpu_init();
  cons_bootstrap(cpu);

  printf("CPU#%d started!\n", cpu);
  printf("Stack Pointer: %p\n", stack_addr_read());
  mbox_set(0, 3, __BIT(cpu));
}

static void smp_demo_clock(uint32_t r0 __unused, uint32_t r1 __unused,
                      uint32_t atags __unused) {
  smp_intro();
  kernel_exit();
  arm_irq_enable();
  demo_clock_switch();
  for (;;);
}

static void smp_demo_pmap(uint32_t r0 __unused, uint32_t r1 __unused,
                      uint32_t atags __unused) {
  smp_intro();
  demo_pmap();
  for (;;);
}

static void __unused gpio_blink() {
  for(;;);
#define PIN40 21
  smp_intro();

  for(;;){
    gpio_set_pull(PIN40, GPPUD_PULLUP);
    printf("PIN40 (GPIO21) pullup\n");
    delay(0xfffffff);

    gpio_set_pull(PIN40, GPPUD_PULLDOWN);
    printf("PIN40 (GPIO21) pulldown\n");
    delay(0xfffffff);
  }
}

void set_mail_buffer(volatile unsigned int *mailbuffer, int state){
  unsigned int c  = 1;
  mailbuffer[c++] = 0;         // Request

  mailbuffer[c++] = 0x00038041;// Tag ID (SET_GPIO_STATE)
  mailbuffer[c++] = 8;         // Value buffer size (bytes)
  mailbuffer[c++] = 0;         // Req. + value length (bytes)
  mailbuffer[c++] = 130;       // ACT_LED pin number
  mailbuffer[c++] = state;     // Turn it on/off

  mailbuffer[c++] = 0;         // Terminating tag
  mailbuffer[0] = c*4;         // Buffer size
}

static void smp_demo_led(){
  smp_intro();
  arm_irq_enable();
  for(;;)
    printf(".");
  //kernel_exit();
  static volatile unsigned int mailbuffer[256] __attribute__((aligned (16)));
  unsigned long physical_mb = (unsigned long)mailbuffer;
  unsigned int var;

  printf("Demo LED.\n");
  for(;;){
    set_mail_buffer(mailbuffer, 1);
    vc_mbox_send(physical_mb, 8);
    var = vc_mbox_recv(8);
    printf("led on\n");
    delay(0xffffff0);

    set_mail_buffer(mailbuffer, 0);
    vc_mbox_send(physical_mb, 8);
    var = vc_mbox_recv(8);
    printf("led off\n");
    delay(0xffffff0);
  }
}

void smp_bootstrap() {
#define L2I (uint32_t)(uint64_t)

  int cpu = 0;
  mbox_send(cpu+1, 3, L2I smp_demo_clock);
  mbox_send(cpu+2, 3, L2I smp_demo_led);
  mbox_send(cpu+3, 3, L2I smp_demo_pmap);
  
  paddr_t s1 = pm_alloc(L2I &_stack_size);
  paddr_t s2 = pm_alloc(L2I &_stack_size);
  paddr_t s3 = pm_alloc(L2I &_stack_size);
  
  mbox_send(cpu+1, 1,  s1);
  mbox_send(cpu+2, 1,  s2);
  mbox_send(cpu+3, 1,  s3);

  do {
    __asm__ volatile("wfe");
  } while (mbox_recv(0, 3) != (__BIT(3) | __BIT(2) | __BIT(1)));

  mbox_clr(0, 3, -1);
}
