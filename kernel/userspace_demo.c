#include <userspace_demo.h>
#include <pmap.h>
#include <pmman.h>
#include <aarch64/pte.h>
#include <klibc.h>
#include <aarch64/cpureg.h>


//todo
/* static void us_save_kctx(){} */

//todo
/* static void us_restore_kctx(){} */

void
__attribute__((aligned(4096)))
us_program(){
  __asm__ __volatile__("mov X0, #123" :: );
  //__asm__ __volatile__("MRS X0, CurrentEl" :: );
  __asm__ __volatile__("svc #0x1" :: );
  __asm__ __volatile__("svc #0x0" :: );
}

static void us_switch(){
  reg_elr_el1_write(0x0); // virt addr of us_program is 0x0 
  reg_spsr_el1_write(0x0);
  __asm__ __volatile__("ERET" :: );
}

#define TTBR_ASID_OFFSET 48
static void
us_setup(paddr_t pt_root){
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

  /* printf("\t %.16p -> %.8p\n", vaddr, paddr); */
  /* paddr_t paddr2 = 0; */
  /* pmap_kextract(vaddr, &paddr2); */
  /* printf("\t %.16p -> %.8p\n\n", vaddr, paddr2); */

  return vaddr;
}

static paddr_t
us_setup_pagetable(){
#define PTE_ATTR  ATTR_SH(ATTR_SH_IS) | ATTR_NS | L3_PAGE | ATTR_AF | ATTR_AP(ATTR_AP_RO) | ATTR_IDX(ATTR_NORMAL_MEM_NC) | ATTR_AP(ATTR_AP_USER)

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
ctx_create(){
  
  // setup kernel_space_proc2
  /* void __attribute__((unused))  *program = us_program; */
  /* vaddr_t __attribute__((unused))  stack =; */
  /* vaddr_t __attribute__((unused))  raturn_addr = NULL;   */
  /* void __attribute__((unused))  registers =; */
}

void
us_launch_program(){
  ctx_create();
  
  printf("Launch userspace program.\n");
  paddr_t pt_root = us_setup_pagetable();

  us_setup(pt_root);
  us_switch();
  printf("User's program finished.\n");
}

