#include <demo/demo.h>
#include <aarch64/pte.h>
#include <aarch64/cpureg.h>
#include <aarch64/ctx.h>
#include <aarch64/cpu.h>
#include <clock.h>
#include <stdatomic.h>

#include <rpi/gpio.h>
#include <rpi/vc_mbox.h>


vaddr_t vm_alloc(size_t size){
  //static atomic_uint_fast64_t  next_addr = (vaddr_t)&_brk_limit;
  //atomic_fetch_add_explicit(&next_addr, size, memory_order_relaxed);
  static vaddr_t next_addr = (vaddr_t)&_brk_limit + 0x1b000;
  next_addr += size;
  return next_addr;
}

vaddr_t pages_alloc(size_t pages, flags_t flags){
  vaddr_t vaddr = vm_alloc(pages*PAGESIZE);
  paddr_t paddr = pm_alloc(pages*PAGESIZE);
  pmap_kenter(vaddr, paddr, flags);

  return vaddr;
}

static int fib(int a){
  if (a <= 0) return 0;
  if (a <= 2) return 1;
  return fib(a-1) + fib(a-2);
}

static void __attribute__((unused, aligned(4096)))
program1(uint64_t arg){
  static int f = 0;
  arg = fib(f);
  printf("fib(%d) = %d\n", f++, arg);
}

static void __attribute__((unused, aligned(4096)))
program2(){
  delay(0xffffff0);
  printf("delay\n");
}

static void
ctx_switch(vaddr_t program){
  /*prepare new thread context*/
  flags_t flags =
    L2_BLOCK | ATTR_SH(ATTR_SH_IS) | ATTR_NS | ATTR_AP(ATTR_AP_RW) |
    ATTR_AF | L3_PAGE | ATTR_IDX(ATTR_NORMAL_MEM_WB);

  vaddr_t program_counter = program;
  vaddr_t ret_addr = (vaddr_t)NULL;
  uint64_t arg_x0 = 0xcafebabe;
  static vaddr_t thread_stack = 0;
  //if(!thread_stack)
  {
    thread_stack = (vaddr_t)pages_alloc(1, flags);
    printf("thread stack: %p\n", thread_stack);
  }


  ctx_t* ctx_new = ctx_push(arg_x0, thread_stack, program_counter, ret_addr);
  
  struct ctx_t ctx_old;
  ctx_save_switch_restore(&ctx_old, ctx_new);
}

static void
launch_thread(vaddr_t program){
  static int tid = 0;
  printf("Start thread  %d.\n", tid);
  ctx_switch(program);
  printf("Finish thread %d.\n\n", tid++);
}

void demo_clock(){
  printf("Demo clock.\n");
  clock_init();
  for(;;) {
    launch_thread((vaddr_t)&program1);
    launch_thread((vaddr_t)&program2);
  }
}


void demo_gpio() {
#define PIN40 21

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

void demo_led(){
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
