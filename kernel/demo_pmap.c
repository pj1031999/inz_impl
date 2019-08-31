#include <demo/demo.h>
#include <pmap.h>

void print_dirty_referenced(vaddr_t va){
  //return;
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
  
  //vm_alloc(PAGESIZE*4);
  vaddr_t va = vm_alloc(PAGESIZE);
  printf("new vaddr: %p\n", va);
  pmap_kremove(va, PAGESIZE);
    
  uint64_t val = 0;
  const uint64_t set_val = 5;

  print_dirty_referenced(va);
  
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
    /* ATTR_AF | */ ATTR_IDX(ATTR_NORMAL_MEM_WB) |// ATTR_AP(ATTR_AP_USER) |
      FLAG_MEM_RO | ATTR_SW_RW | FLAG_MEM_NOT_EX | FLAG_MEM_WRITE_BACK;

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

  print_dirty_referenced(va);
}

