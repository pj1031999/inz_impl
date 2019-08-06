#include <demo/demo.h>
#include <aarch64/ctx.h>
#include <demo/userspace_demo.h>
#include <cons.h>

const int us_prog_page = 129; // 0x81000

static void
us_setup(paddr_t pt_root){
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

  //stack address placed after us_program page - top bytes are reserved (OLD ctx)
  reg_sp_el0_write((us_prog_page+2)*PAGESIZE - 0x200);
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
  paddr_t pa_prog = (paddr_t)&us_program_entry;

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
  va_l3[us_prog_page] = entry_l3_prog;
  va_l3[us_prog_page+1] = entry_l3_stack;
  *va_stack = entry_l3_stack;
  
  return pa_l1;
}

static void
ctx_us_switch(vaddr_t va_stack){
  /*prepare new thread context*/
  //flags_t flags = FLAG_MEM_RW | FLAG_MEM_NOT_EX | FLAG_MEM_WRITE_THROUGH;
  vaddr_t thread_stack = va_stack;
  vaddr_t program_counter = (vaddr_t)&us_program_entry;
  vaddr_t ret_addr = (vaddr_t)NULL;
  ctx_t* ctx_new = ctx_push(0x0, thread_stack, program_counter, ret_addr);
  
  struct ctx_t ctx_old;
  ctx_us_save_switch_restore(&ctx_old, ctx_new);
}

void demo_uart(){
  printf("Demo UART.\n");
  
  extern cons_t uart0_cons;
  uart0_cons.init(NULL);

  printf("Userspace program address: %p.\n", &us_program_entry);

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
