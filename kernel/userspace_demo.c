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
static void us_setup(paddr_t pt_root){
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

static paddr_t
us_setup_pagetable(){

#define PTE_ATTR  ATTR_SH(ATTR_SH_IS) | ATTR_NS | L3_PAGE | ATTR_AF | ATTR_AP(ATTR_AP_RO) | ATTR_IDX(ATTR_NORMAL_MEM_NC) | ATTR_AP(ATTR_AP_USER)
  extern vaddr_t *_tmp_pagetable_map;
 

  paddr_t alloc1_p = pm_alloc(PAGESIZE);
  paddr_t alloc2_p = pm_alloc(PAGESIZE);
  paddr_t alloc3_p = pm_alloc(PAGESIZE);
  paddr_t alloc4_p = pm_alloc(PAGESIZE);

  paddr_t prog_addr;
  pmap_kextract((vaddr_t)&us_program, &prog_addr);
  
  pte_t entry_l1 = alloc2_p | L1_TABLE;
  pte_t entry_l2 = alloc3_p | L2_TABLE;
  pte_t entry_l3_prog
    = prog_addr
    | PTE_ATTR
    ;
  pte_t entry_l3_stack = alloc4_p | PTE_ATTR | ATTR_AP(ATTR_AP_RW);

  vaddr_t *tmp_map = (vaddr_t*)&_tmp_pagetable_map;
  pmap_kenter((vaddr_t)tmp_map, alloc1_p, FLAG_MEM_RW | FLAG_MEM_NOT_EX | FLAG_MEM_WRITE_THROUGH);
  tmp_map[0] = entry_l1;

  pmap_kenter((vaddr_t)tmp_map, alloc2_p, FLAG_MEM_RW | FLAG_MEM_NOT_EX | FLAG_MEM_WRITE_THROUGH);
  tmp_map[0] = entry_l2;

  pmap_kenter((vaddr_t)tmp_map, alloc3_p, FLAG_MEM_RW | FLAG_MEM_NOT_EX | FLAG_MEM_WRITE_THROUGH);
  tmp_map[0] = entry_l3_prog;
  tmp_map[1] = entry_l3_stack;
  
  return alloc1_p;
}

void
us_launch_program(){
  printf("Launch userspace program.\n");
  paddr_t pt_root = us_setup_pagetable();
  us_setup(pt_root);
  us_switch();
  printf("User's program finished.\n");
}
