#include "aarch64/pte.h"
#include "types.h"

#define UPPERADDR 0xffffFFFF00000000
#define PHYSADDR(x) ((x) - (UPPERADDR))
#define PAGESIZE 4096
extern uint64_t _level1_pagetable[];
extern uint64_t _level2_pagetable[];
extern uint64_t _level3_pagetable[];

void __attribute__((section(".init")))
page_table_fill_inner_nodes(void)
{
  
  uint64_t *entry_l1 = (void*)PHYSADDR((uint64_t)&_level1_pagetable);
  uint64_t *entry_l2 = (void*)PHYSADDR((uint64_t)&_level2_pagetable);
  uint64_t *entry_l3 = (void*)PHYSADDR((uint64_t)&_level3_pagetable);
  
  uint64_t entry_l2_firstGB  = (uint64_t)&entry_l2[0]   | L1_TABLE;
  uint64_t entry_l2_secondGB = (uint64_t)&entry_l2[512] | L1_TABLE;
  uint64_t entry_l3_first2MB = (uint64_t)&entry_l3[0]   | L2_TABLE;
  
  entry_l1[0] = entry_l2_firstGB;
  entry_l1[1] = entry_l2_secondGB;
  //entry_l1[2] = ;
  //entry_l1[3] = ;

  entry_l2[0] = entry_l3_first2MB;
}

#define PTE_ATTR  L2_BLOCK | ATTR_SH(ATTR_SH_IS) | ATTR_NS | ATTR_AP(ATTR_AP_RW)
#define ENTRY_2MB 0x00200000
#define ENTRY_4KB 0x00001000

extern int  _mail_buffer[];
extern int  _bss_end[];
void __attribute__((section(".init")))
page_table_fill_leaves(void)
{
  uint64_t entry = 0;
  uint64_t *_level2_pagetable_phys = (void*)PHYSADDR((uint64_t)&_level2_pagetable);
  uint64_t *_level3_pagetable_phys = (void*)PHYSADDR((uint64_t)&_level3_pagetable);

  // kernel page - first 2MB
  entry = PTE_ATTR | ATTR_AF | L3_PAGE | ATTR_IDX(ATTR_NORMAL_MEM_WB);
  for(int i = 0; i < 512; entry += ENTRY_4KB){
    _level3_pagetable_phys[i++] = entry;
  }

#define A PHYSADDR((vaddr_t)&_level2_pagetable) / PAGESIZE
#define C PHYSADDR((vaddr_t)&_mail_buffer) / PAGESIZE

  entry = A*ENTRY_4KB | PTE_ATTR | ATTR_AF | L3_PAGE | ATTR_IDX(ATTR_NORMAL_MEM_NC);
  _level3_pagetable_phys[A] = entry;
        
  entry = C*ENTRY_4KB | PTE_ATTR | ATTR_AF | L3_PAGE | ATTR_IDX(ATTR_NORMAL_MEM_NC);
  _level3_pagetable_phys[C] = entry;
    
  // page_table L2
  
  /* //first half (without first 2 MB) of first GB has write back cache  */
  int z = 480;
  //entry = ENTRY_2MB | PTE_ATTR | ATTR_IDX(ATTR_NORMAL_MEM_WB);
  for(int i = 1; i < z; entry += ENTRY_2MB){
    _level2_pagetable_phys[i++] = 0x0; //entry;
  }

  //second half of first GB is non cacheable - last few needed MB
  entry = (z*ENTRY_2MB) | PTE_ATTR | ATTR_IDX(ATTR_NORMAL_MEM_NC);
  for(int i = z; i < 512; entry += ENTRY_2MB){
    _level2_pagetable_phys[i++] = entry;
  }


  //just one entry for arm mmio pheripherials
  _level2_pagetable_phys = (void*)PHYSADDR((uint64_t)&_level2_pagetable[512]);
  entry = (512*ENTRY_2MB) | ATTR_AF | PTE_ATTR | ATTR_IDX(ATTR_DEVICE_MEM);
  _level2_pagetable_phys[0] = entry;
}
