#include <userspace_demo.h>
#include <pmap.h>
#include <pmman.h>
#include <aarch64/pte.h>
#include <klibc.h>
#include <aarch64/cpureg.h>
#include <aarch64/ctx.h>
#include <aarch64/cpu.h>
#include <clock.h>
#include <pcpu.h>

static int fib(int a){
  if (a <= 0) return 0;
  if (a <= 2) return 1;
  return fib(a-1) + fib(a-2);
}

static void __attribute__((aligned(4096)))
program1(uint64_t arg){
  static int f = 0;
  arg = fib(f);
  printf("fib(%d) = %d\n", f++, arg);
}

static void __attribute__((unused, aligned(4096)))
program2(){
  int i;
  for(i = 0; i < 100000000; i++);

}
vaddr_t
vm_alloc(size_t size){
  extern vaddr_t _brk_limit;
  static vaddr_t next_addr = (vaddr_t)&_brk_limit;
  next_addr += size;
  return next_addr;
}

vaddr_t
pages_alloc(size_t pages, flags_t flags){
  vaddr_t vaddr = vm_alloc(pages*PAGESIZE);
  paddr_t paddr = pm_alloc(pages*PAGESIZE);
  pmap_kenter(vaddr, paddr, flags);

  return vaddr;
}

static void
ctx_switch(vaddr_t program){
  /*prepare new thread context*/
  flags_t flags = FLAG_MEM_RW | FLAG_MEM_NOT_EX | FLAG_MEM_WRITE_THROUGH;
  vaddr_t thread_stack = (vaddr_t)(pages_alloc(1, flags));
  vaddr_t program_counter = program;
  vaddr_t ret_addr = (vaddr_t)NULL;
  uint64_t arg_x0 = 0xcafebabe;
  ctx_t* ctx_new = ctx_push(arg_x0, thread_stack, program_counter, ret_addr);
  
  struct ctx_t ctx_old;
  ctx_save_switch_restore(&ctx_old, ctx_new);
}

static void __attribute__((unused))
launch_thread(vaddr_t program){
  static int tid = 0;
  printf("Start thread  %d.\n", tid);
  ctx_switch(program);
  printf("Finish thread %d.\n\n", tid++);
}

void demo_clock_switch(){
  printf("Demo clock.\n");
  clock_init();
  for(;;) {
    for(int i = 0; i < 50000; i++);
    printf(".");
    continue;

    launch_thread((vaddr_t)&program1);
    launch_thread((vaddr_t)&program2);
  }
  for(;;);
}


void print_dirty_referenced(vaddr_t va){
  return;
  if(pmap_is_modified(va))
    printf("Page is modified\n");
  else
    printf("Page is not modified.\n");

  
  if(pmap_is_referenced(va))
    printf("Page is referenced.\n");
  else
    printf("Page is not referenced.\n");

}

void demo_pmap(){
  pcpu()->on_fault = true;
  printf("Demo pmap.\n");
  
  //vm_alloc(PAGESIZE*332);
  vaddr_t va = vm_alloc(PAGESIZE);
  printf("new vaddr: %p\n", va);

  uint64_t val = 0;
  const uint64_t set_val = 5;


  if(load_word(va, &val))
    printf("Success! read value: %llu\n", val);
  else
    printf("Fail! address is not mapped.\n");



  if(store_word(va, set_val))
    printf("Success! write value: %llu\n", set_val);
  else
    printf("Fail! Can't write to address.\n");


  print_dirty_referenced(va);

  paddr_t pa = pm_alloc(PAGESIZE);
  printf("new paddr: %p\n", pa);
  const uint64_t PTE_ATTR =
      ATTR_SH(ATTR_SH_IS) | ATTR_NS | L3_PAGE |
      ATTR_AF | ATTR_IDX(ATTR_NORMAL_MEM_WB) | ATTR_AP(ATTR_AP_USER) |
      FLAG_MEM_RW | FLAG_MEM_NOT_EX | FLAG_MEM_WRITE_THROUGH;

  printf("Add mapping: %p --> %p \n", va, pa);
  pmap_kenter(va, pa, PTE_ATTR);

  print_dirty_referenced(va);
  if(load_word(va, &val))
    printf("Success! read value: %llu\n", val);
  else
    printf("Fail! address is not mapped.\n");


  print_dirty_referenced(va);
  if(store_word(va, set_val))
    printf("Success! write value: %llu\n", set_val);
  else
    printf("Fail! Can't write to address.\n");


  print_dirty_referenced(va);
  if(load_word(va, &val))
    printf("Success! read value: %llu\n", val);
  else
    printf("Fail! address is not mapped.\n");


  printf("Remove mapping: %p --> %p \n", va, pa);
  pmap_kremove(va, PAGESIZE);


  if(load_word(va, &val))
    printf("Success! read value: %llu\n", val);
  else
    printf("Fail! address is not mapped.\n");


  if(store_word(va, set_val))
    printf("Success! write value: %llu\n", set_val);
  else
    printf("Fail! Can't write to address.\n");


  kernel_exit();
}

#include <cons.h>
void __attribute__((section(".us_text")))
us_program(){
  
  int64_t res = 9;
  __asm__ __volatile__( // read number
  		       "svc #0x1 \n"
  		       "mov %[res], x0"
  		       :[res] "=r" (res) );
  res *= 2;

  __asm__ __volatile__(// write number
		       "mov x0, %[res] \n"
		       "svc #0x2"
		       :: [res] "r" (res) );

  __asm__ __volatile__("svc #0x0"); //exit
  return;
}

static void
us_setup(paddr_t pt_root){
  const uint64_t TTBR_ASID_OFFSET = 48;

  user_space.asid = 1;
  user_space.start = 0;
  user_space.end = 2*PAGESIZE;
  user_space.pt_root = (pte_t*)pt_root;

  reg_ttbr0_el1_write
    (
     ( (uint64_t)user_space.asid << TTBR_ASID_OFFSET)
     | (uint64_t)user_space.pt_root
     );

  reg_sp_el0_write(1*PAGESIZE + 0x800);
}

static paddr_t
us_setup_pagetable(vaddr_t stack){

  const uint64_t PTE_ATTR =
    ATTR_SH(ATTR_SH_IS) | ATTR_NS | L3_PAGE |
    ATTR_AF | ATTR_AP(ATTR_AP_RO) |
    ATTR_IDX(ATTR_NORMAL_MEM_NC) | ATTR_AP(ATTR_AP_USER);

  flags_t flags =
    FLAG_MEM_RW | FLAG_MEM_NOT_EX | FLAG_MEM_WRITE_THROUGH |
    ATTR_SH(ATTR_SH_IS) | ATTR_NS | L3_PAGE |
    ATTR_AF | ATTR_IDX(ATTR_NORMAL_MEM_NC) | ATTR_AP(ATTR_AP_USER);
  
  vaddr_t* va_l1 = (vaddr_t*)pages_alloc(1, flags);
  vaddr_t* va_l2 = (vaddr_t*)pages_alloc(1, flags);
  vaddr_t* va_l3 = (vaddr_t*)pages_alloc(1, flags);  
  vaddr_t* va_stack = (vaddr_t*)stack;//(vaddr_t*)pages_alloc(1, flags);

  paddr_t pa_l1;
  paddr_t pa_l2;
  paddr_t pa_l3;
  paddr_t pa_stack;
  paddr_t pa_prog = (paddr_t)&us_program;

  pmap_kextract((vaddr_t)va_l1, &pa_l1);
  pmap_kextract((vaddr_t)va_l2, &pa_l2);
  pmap_kextract((vaddr_t)va_l3, &pa_l3);
  pmap_kextract((vaddr_t)va_stack, &pa_stack);
  //pmap_kextract((vaddr_t)&us_program, &phys_prog);

  pte_t entry_l1 = pa_l2 | L1_TABLE;
  pte_t entry_l2 = pa_l3 | L2_TABLE;
  pte_t entry_l3_prog  = pa_prog  | PTE_ATTR;
  pte_t entry_l3_stack = pa_stack | flags;
  
  *va_l1 = entry_l1;
  *va_l2 = entry_l2;
  va_l3[0] = entry_l3_prog;
  va_l3[1] = entry_l3_stack;
  *va_stack = entry_l3_stack;

  return pa_l1;
}

/* static void us_switch(){ */
/*   reg_elr_el1_write(0x0); */
/*   reg_spsr_el1_write(0x0); */
/*   __asm__ __volatile__("ERET" :: ); */
/* } */


static void
ctx_us_switch(vaddr_t va_stack){
  /*prepare new thread context*/
  //flags_t flags = FLAG_MEM_RW | FLAG_MEM_NOT_EX | FLAG_MEM_WRITE_THROUGH;
  vaddr_t thread_stack = va_stack;
  vaddr_t program_counter = 0x0; // mapping 0x0 -> us_program
  vaddr_t ret_addr = (vaddr_t)NULL;
  ctx_t* ctx_new = ctx_push(0x0, thread_stack, program_counter, ret_addr);
  
  struct ctx_t ctx_old;
  ctx_us_save_switch_restore(&ctx_old, ctx_new);
}


void demo_uart(){
  printf("Demo UART.\n");
  
  extern cons_t uart0_cons;
  uart0_cons.init(NULL);

  printf("Userspace program address: %p.\n", &us_program);

  flags_t flags =
    FLAG_MEM_RW | FLAG_MEM_NOT_EX | FLAG_MEM_WRITE_THROUGH |
    ATTR_SH(ATTR_SH_IS) | ATTR_NS | L3_PAGE |
    ATTR_AF | ATTR_IDX(ATTR_NORMAL_MEM_WB) | ATTR_AP(ATTR_AP_USER);
  vaddr_t va_stack = (vaddr_t)pages_alloc(1, flags);
  paddr_t pt_root = us_setup_pagetable(va_stack);
  us_setup(pt_root);
  
  
  while(true)
  {
    printf("Userspace in.\n");
    printf("Enter a number.\n");
    ctx_us_switch(va_stack);
    printf("Userspace out.\n\n");
  }
}
