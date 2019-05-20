#include <types.h>
#include <klibc.h>
#include <pcpu.h>
#include <arm/mbox.h>
#include <rpi/irq.h>
#include <smp.h>
#include <aarch64/cpu.h>
#include <rpi/gpio.h>
#include <rpi/vc_mbox.h>

extern void cons_bootstrap(unsigned);

static void smp_intro(){
  unsigned cpu = arm_cpu_id();
  pcpu_init();
  cons_bootstrap(cpu);
  bcm2836_local_irq_init();
  arm_irq_enable();

  printf("CPU#%d started!\n", cpu);
  mbox_set(0, 3, __BIT(cpu));
}

static void smp_entry(uint32_t r0 __unused, uint32_t r1 __unused,
                      uint32_t atags __unused) {
  smp_intro();
  for (;;);
}

static void gpio_blink(){
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

static void led_blink(){
  smp_intro();
  static volatile unsigned int mailbuffer[256] __attribute__((aligned (16)));
  unsigned long physical_mb = (unsigned long)mailbuffer;
  unsigned int var;

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
  int cpu = 1;
  mbox_send(cpu++, 3, (uint32_t)(long)smp_entry);
  mbox_send(cpu++, 3, (uint32_t)(long)led_blink);
  mbox_send(cpu++, 3, (uint32_t)(long)gpio_blink);

  do {
    __asm__ volatile("wfe");
  } while (mbox_recv(0, 3) != (__BIT(3) | __BIT(2) | __BIT(1)));

  mbox_clr(0, 3, -1);
}
