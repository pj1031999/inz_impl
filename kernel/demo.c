#include <demo/demo.h>
#include <aarch64/pte.h>
#include <aarch64/cpureg.h>
#include <aarch64/ctx.h>
#include <aarch64/cpu.h>
#include <aarch64/frame.h>
#include <clock.h>

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
  f %= 40;
  arg = fib(f);
  printf("fib(%d) = %d\n", f++, arg);
}

static void __attribute__((unused, aligned(4096)))
program2(){
  delay(0x1ffffff0);
  printf("delay\n");
}

static void __attribute__((unused, aligned(4096)))
program3(){
  unsigned int i = 0;
  for(;;){
    delay(0xcffff0);
    printf("%c", 'a'+((i++)%('z'-'a'+1)));
  }
}


static struct trapframe*
ctx_create(vaddr_t program){
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

  ctx_t* new = ctx_push(arg_x0, thread_stack, program_counter, ret_addr);
  struct trapframe* frame = (struct trapframe*)pages_alloc(1, flags);
  frame->tf_sp = new->sf_sp;
  frame->tf_lr = new->sf_lr;
  frame->tf_elr = new->sf_pc;
  frame->tf_esr = new->sf_esr;
  frame->tf_spsr = new->sf_spsr;
  for(int i = 0; i < 30; i++)
    frame->tf_x[i] = new->sf_regs[i];

  return frame;
}

void demo_clock(){
  printf("Demo clock.\n");

  //schedule[0] or schedule[1] will be overwritten
  //witch current context in next_ctx_stack
  pcpu()->schedule[0] =  pcpu()->schedule[1] = ctx_create((vaddr_t)program3);
  clock_init();

  int i = 0;
  for(;;){
    delay(0x1fffff0);
    printf("%d", (i++)%10);
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

  extern unsigned int  _mail_buffer[256];
  unsigned long physical_mb = (unsigned long)_mail_buffer; //will cut to 32 bit value at vc_mbox_send
  unsigned int var;

  printf("Demo LED.\n");
  for(;;){
    set_mail_buffer(_mail_buffer, 1);
    vc_mbox_send(physical_mb, 8);
    var = vc_mbox_recv(8);
    printf("led on\n");
    delay(0xffffff0);

    set_mail_buffer(_mail_buffer, 0);
    vc_mbox_send(physical_mb, 8);
    var = vc_mbox_recv(8);
    printf("led off\n");
    delay(0xffffff0);
  }
}


#include <klibc.h>
#include <fat/diskio.h>

int fat_init()
{
  disk_initialize();
  printf("Not frozen\n");
  return 0;
}

int fileio_test()
{
  FATFS fs;
  {
    FRESULT res = pf_mount(&fs);
    printf("mount result: %d\n", res);
  }

  const char * file_path = "/issue.txt";
  {
    FRESULT res = pf_open(file_path);
    printf("open file: \"%s\", result: %d\n", file_path, res);
    if(res != FR_OK){
      printf("Failed to open file!\n");
      kernel_exit();
    }
  }

  {
    char buf[256] = "ala ma kota";
    char *str = buf;
    uint32_t n;
    
    FRESULT res = pf_write(str, 255, &n);
    printf("write file result: %d\n", res);
    if(res != FR_OK){
      printf("Failed to write file!\n");
      kernel_exit();
    }
    printf("\n[INFO] Attempting write from file:\n%s", str);
  }

  {
    char buf[256];
    char *str = buf;
    uint32_t n;
    
    FRESULT res = pf_read(str, 255, &n);
    printf("read file result: %d\n", res);
    if(res != FR_OK){
      printf("Failed to read file!\n");
      kernel_exit();
    }
    printf("\n[INFO] Attempting read from file:\n%s", str);
  }

  return 0;
}

void demo_sd(void){
  fat_init();
  fileio_test();  

  kernel_exit();
}
