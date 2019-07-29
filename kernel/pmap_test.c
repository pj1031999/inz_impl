#include <pmap_test.h>
#include <pmap.h>
#include <pmman.h>
#include <klibc.h>
#include <aarch64/mmu.h>

void pmap_test_kremove(){
  paddr_t alloc_p = pm_alloc(PAGESIZE);
  vaddr_t *alloc_v = (vaddr_t*)((uint64_t)&_kernel | (uint64_t)alloc_p);
  *alloc_v = 0xcafebabe; 
  printf("\t alloc_v = %p \t *alloc_v = %x \n", alloc_v, *alloc_v);
  pmap_kremove((vaddr_t)alloc_v, PAGESIZE); // kernel panic
  printf("\t alloc_v = %p \t *alloc_v = %x \t", alloc_v, *alloc_v);
}

extern uint64_t *_tmp_pagetable_map;
void pmap_test_kenter(){ // swap pages
  paddr_t alloc1_p = pm_alloc(PAGESIZE);
  vaddr_t *alloc1_v = (vaddr_t*)((uint64_t)&_kernel | (uint64_t)alloc1_p);
  *alloc1_v = 0x1; 

  paddr_t alloc2_p = pm_alloc(PAGESIZE);
  vaddr_t *alloc2_v = (vaddr_t*)((uint64_t)&_kernel | (uint64_t)alloc2_p);
  *alloc2_v = 0x2; 

  paddr_t pa1 = 0, pa2 = 0;
  pmap_kextract((vaddr_t)alloc1_v, &pa1);
  pmap_kextract((vaddr_t)alloc2_v, &pa2);
  printf("\t alloc1_v = %p \t *alloc1_v = %x \t phys = %p \n", alloc1_v, *alloc1_v, pa1);
  printf("\t alloc2_v = %p \t *alloc2_v = %x \t phys = %p \n", alloc2_v, *alloc2_v, pa2);

  pmap_kenter((vaddr_t)alloc1_v, alloc2_p, FLAG_MEM_RW | FLAG_MEM_NOT_EX | FLAG_MEM_WRITE_THROUGH);
  pmap_kenter((vaddr_t)alloc2_v, alloc1_p, FLAG_MEM_RW | FLAG_MEM_NOT_EX | FLAG_MEM_WRITE_THROUGH);
  *alloc1_v *= 0x1; 
  *alloc2_v *= 0x1; 
  
  pmap_kextract((vaddr_t)alloc1_v, &pa1);
  pmap_kextract((vaddr_t)alloc2_v, &pa2);
  printf("\t alloc1_v = %p \t *alloc1_v = %x \t phys = %p \n", alloc1_v, *alloc1_v, pa1);
  printf("\t alloc2_v = %p \t *alloc2_v = %x \t phys = %p \n", alloc2_v, *alloc2_v, pa2);
}

void pmap_test_clear_referenced(vaddr_t va){
  pmap_clear_referenced(va);

  paddr_t pa = 0;
  pmap_kextract(va, &pa);
  printf("\t %.16p -> %.8p af=%d dirty=%d \t", va, pa, pmap_is_referenced(va), pmap_is_modified(va));
}

void pmap_test_clear_modified(vaddr_t va){
  pmap_clear_modified(va);

  paddr_t pa = 0;
  pmap_kextract(va, &pa);
  printf("\t %.16p -> %.8p af=%d dirty=%d \t", va, pa, pmap_is_referenced(va), pmap_is_modified(va));
}

void pmap_test_kextract_accessed(vaddr_t va){
  paddr_t pa = 0;
  pa = *((uint64_t*)va);
  pmap_kextract(va, &pa);
  printf("\t %.16p -> %.8p af=%d dirty=%d \t", va, pa, pmap_is_referenced(va), pmap_is_modified(va));
}

void pmap_test_kextract(vaddr_t va){
  paddr_t pa = 0;
  pmap_kextract(va, &pa);
  printf("\t %.16p -> %.8p af=%d dirty=%d \t", va, pa, pmap_is_referenced(va), pmap_is_modified(va));
}
