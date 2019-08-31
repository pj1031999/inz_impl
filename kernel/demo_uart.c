#include <demo/demo.h>
#include <aarch64/ctx.h>
#include <demo/userspace_demo.h>
#include <cons.h>

static void
us_setup(paddr_t pt_root, vaddr_t stack){
  const uint64_t TTBR_ASID_OFFSET = 48;

  user_space.asid = 1;
  /* user_space.start = 0; */
  /* user_space.end = 2*PAGESIZE; */
  user_space.pt_root = (pte_t*)pt_root;

  reg_ttbr0_el1_write
    (
     ( (uint64_t)user_space.asid << TTBR_ASID_OFFSET)
     | (uint64_t)user_space.pt_root
     );

  reg_sp_el0_write(stack + PAGESIZE);
}

static paddr_t
us_setup_pagetable(vaddr_t* stack, vaddr_t* usstack){

  flags_t flags =
    ATTR_SH(ATTR_SH_IS) | ATTR_NS | L3_PAGE |
    FLAG_MEM_RW | FLAG_MEM_NOT_EX |
    ATTR_AF | ATTR_IDX(ATTR_NORMAL_MEM_NC);
  
  vaddr_t* va_l1 = (vaddr_t*)pages_alloc(1, flags);
  vaddr_t* va_l2 = (vaddr_t*)pages_alloc(1, flags);
  vaddr_t* va_l3 = (vaddr_t*)pages_alloc(1, flags);
  vaddr_t* va_stack = (vaddr_t*)pages_alloc(1, flags);

  paddr_t pa_l1;
  paddr_t pa_l2;
  paddr_t pa_l3;
  paddr_t pa_stack;
  paddr_t pa_prog = (paddr_t)&us_program_entry;

  pmap_kextract((vaddr_t)va_l1, &pa_l1);
  pmap_kextract((vaddr_t)va_l2, &pa_l2);
  pmap_kextract((vaddr_t)va_l3, &pa_l3);
  pmap_kextract((vaddr_t)va_stack, &pa_stack);
  //pmap_kextract((vaddr_t)&us_program, &phys_prog);

  const uint64_t PTE_ATTR =
    ATTR_SH(ATTR_SH_IS) | ATTR_NS | L3_PAGE | ATTR_AF |
    ATTR_IDX(ATTR_NORMAL_MEM_NC) | ATTR_AP(ATTR_AP_USER);

  pte_t entry_l1 = pa_l2 | L1_TABLE;
  pte_t entry_l2 = pa_l3 | L2_TABLE;
  pte_t entry_l3_prog  = pa_prog  | PTE_ATTR;// | FLAG_MEM_RO;
  pte_t entry_l3_stack = pa_stack | PTE_ATTR;// | FLAG_MEM_NOT_EX;

  
  const int us_prog_page = 129; // 0x81000
  *va_l1 = entry_l1;
  *va_l2 = entry_l2;
  va_l3[us_prog_page] = entry_l3_prog;
  va_l3[us_prog_page-1] = entry_l3_stack;
  
  *stack = (vaddr_t)va_stack;
  *usstack = (pa_prog & ~0xfff) - PAGESIZE;

   
  /* extern __unused uint64_t _level1_pagetable[]; */
  /* va_l1[0] = _level1_pagetable[0]; */

  printf("l1: %p <--> %p\n", va_l1, pa_l1);
  printf("l2: %p <--> %p\n", va_l2, pa_l2);
  printf("l3: %p <--> %p\n", va_l3, pa_l3);
  
  printf("PT vaddr: %p\n", va_l1);
  printf("PT paddr: %p\n", pa_l1);
  //return (paddr_t)(void*)PHYSADDR((uint64_t)va_l1);
  return pa_l1;
}

static void
ctx_us_switch(vaddr_t va_stack){
  /*prepare new thread context*/

  vaddr_t thread_stack = va_stack;
  vaddr_t program_counter = (vaddr_t)&us_program_entry;
  vaddr_t ret_addr = (vaddr_t)NULL;
  ctx_t* ctx_new = ctx_us_push(0x0, thread_stack, program_counter, ret_addr);
  
  struct ctx_t ctx_old;
  ctx_us_save_switch_restore(&ctx_old, ctx_new);
}

void demo_uart(){
  printf("Demo UART.\n");
  
  extern cons_t uart0_cons;
  uart0_cons.init(NULL);
    
  printf("Userspace program address: %p.\n", &us_program_entry);

  vaddr_t va_stack;
  vaddr_t va_usstack;
  paddr_t pt_root = us_setup_pagetable(&va_stack, &va_usstack);
  us_setup(pt_root, va_usstack);

  while(true)
  {
    printf("Userspace in.\n");
    printf("Enter a number.\n");
    ctx_us_switch(va_stack);
    printf("Userspace out.\n\n");
  }
}
