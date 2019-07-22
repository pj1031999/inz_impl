#include <userspace_demo.h>
#include <pmap.h>
#include <pmman.h>
#include <aarch64/pte.h>
#include <klibc.h>
#include <aarch64/cpureg.h>
#include <aarch64/ctx.h>


static int fib(int a){
  if (a <= 0) return 0;
  if (a <= 2) return 1;
  return fib(a-1) + fib(a-2);
}

static void __attribute__((aligned(4096)))
us_program(uint64_t arg){
  int f = 20;
  arg = fib(f);
  printf("fib(%d) = %d\n", f, arg);
  
  __asm__ __volatile__("svc #0x1" :: );
  //__asm__ __volatile__("svc #0x0" :: );
}

static __attribute__((unused)) void us_switch(){
  reg_elr_el1_write(0x0); // virt addr of us_program is 0x0 
  reg_spsr_el1_write(0x0);
  __asm__ __volatile__("ERET" :: );
}

static __attribute__((unused)) void
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

  reg_sp_el0_write(2*PAGESIZE);
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

static __attribute__((unused)) paddr_t
us_setup_pagetable(){
  const uint64_t PTE_ATTR =  ATTR_SH(ATTR_SH_IS) | ATTR_NS | L3_PAGE | ATTR_AF | ATTR_AP(ATTR_AP_RO) | ATTR_IDX(ATTR_NORMAL_MEM_NC) | ATTR_AP(ATTR_AP_USER);

  flags_t flags = FLAG_MEM_RW | FLAG_MEM_NOT_EX | FLAG_MEM_WRITE_THROUGH;
  vaddr_t* virt_l1 = (vaddr_t*)pages_alloc(1, flags);
  vaddr_t* virt_l2 = (vaddr_t*)pages_alloc(1, flags);
  vaddr_t* virt_l3 = (vaddr_t*)pages_alloc(1, flags);  
  vaddr_t* virt_stack = (vaddr_t*)pages_alloc(1, flags);

  paddr_t phys_l1;
  paddr_t phys_l2;
  paddr_t phys_l3;
  paddr_t phys_stack;
  paddr_t phys_prog;

  pmap_kextract((vaddr_t)virt_l1, &phys_l1);
  pmap_kextract((vaddr_t)virt_l2, &phys_l2);
  pmap_kextract((vaddr_t)virt_l3, &phys_l3);
  pmap_kextract((vaddr_t)virt_stack, &phys_stack);
  pmap_kextract((vaddr_t)&us_program, &phys_prog);

  pte_t entry_l1 = phys_l2 | L1_TABLE;
  pte_t entry_l2 = phys_l3 | L2_TABLE;
  pte_t entry_l3_prog  = phys_prog  | PTE_ATTR;
  pte_t entry_l3_stack = phys_stack | PTE_ATTR | ATTR_AP(ATTR_AP_RW);

  *virt_l1 = entry_l1;
  *virt_l2 = entry_l2;
  *virt_l3 = entry_l3_prog;
  *virt_stack = entry_l3_stack;

  return phys_l1;
}

void
ctx_switch(){
  /*prepare new thread context*/
  flags_t flags = FLAG_MEM_RW | FLAG_MEM_NOT_EX | FLAG_MEM_WRITE_THROUGH;
  vaddr_t thread_stack = (vaddr_t)(pages_alloc(1, flags));
  vaddr_t program_counter = (vaddr_t)&us_program;
  vaddr_t ret_addr = (vaddr_t)NULL;
  uint64_t arg_x0 = 0xcafebabe;
  ctx_t* ctx_new = ctx_push(arg_x0, thread_stack, program_counter, ret_addr);
  
  struct ctx_t ctx_old;
  ctx_save_switch_restore(&ctx_old, ctx_new);
}

void
us_launch_program(){
  printf("Launch new thread.\n");
  ctx_switch();
  printf("Thread finished.\n");
}

