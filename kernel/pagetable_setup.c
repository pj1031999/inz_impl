#include "aarch64/pte.h"
#include "types.h"

#define UPPERADDR 0xffffFFFF00000000
#define PHYSADDR(x) ((x) - (UPPERADDR))
extern uint64_t _level1_pagetable[];
extern uint64_t _level2_pagetable[];

void __attribute__((section(".init")))
page_table_fill_level_1()
{
  
  uint64_t *entry_l1 = (void*)PHYSADDR((uint64_t)&_level1_pagetable);
  uint64_t *entry_l2 = (void*)PHYSADDR((uint64_t)&_level2_pagetable);
  
  uint64_t entry_l2_firstGB  = (uint64_t)&entry_l2[0]   | L1_TABLE;
  uint64_t entry_l2_secondGB = (uint64_t)&entry_l2[512] | L1_TABLE;
  
  entry_l1[0] = entry_l2_firstGB;
  entry_l1[1] = entry_l2_secondGB;
  //entry_l1[2] = ;
  //entry_l1[3] = ;

}


#define PTE_ATTR L1_BLOCK | ATTR_SH(ATTR_SH_IS) | ATTR_NS | ATTR_AP(ATTR_AP_RW)
#define ENTRY_2MB 0x00200000

void __attribute__((section(".init")))
page_table_fill_level_2()
{
  uint64_t entry = 0;
  uint64_t *_level2_pagetable_phys = (void*)PHYSADDR((uint64_t)&_level2_pagetable);

  //kernel page
  entry = PTE_ATTR | ATTR_AF | ATTR_IDX(ATTR_NORMAL_MEM_NC);
  _level2_pagetable_phys[0] = entry;

 
  //first half (without first 2 MB) of first GB has write back cache 
  entry = ENTRY_2MB | PTE_ATTR | ATTR_IDX(ATTR_NORMAL_MEM_WB);
  for(int i = 1; i < 256; entry += ENTRY_2MB){
    _level2_pagetable_phys[i++] = entry;
  }

  //second half of first GB is non cacheable 
  entry = (256*ENTRY_2MB) | PTE_ATTR | ATTR_IDX(ATTR_NORMAL_MEM_NC);
  for(int i = 256; i < 512; entry += ENTRY_2MB){
    _level2_pagetable_phys[i++] = entry;
  }


  //just one entry for arm mmio pheripherials
  _level2_pagetable_phys = (void*)PHYSADDR((uint64_t)&_level2_pagetable[512]);
  entry = (512*ENTRY_2MB) | ATTR_AF | PTE_ATTR | ATTR_IDX(ATTR_DEVICE_MEM);
  _level2_pagetable_phys[0] = entry;
}
