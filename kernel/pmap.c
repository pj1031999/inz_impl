#include <pmap.h>
#include <klibc.h>
#include <aarch64/cpureg.h>
#include <aarch64/aarch64reg.h>
#include <aarch64/pte.h>


extern uint64_t _level1_pagetable[];
extern uint64_t* _kernel;
bool get_block_entry(vaddr_t va, uint64_t **res_entry){
  uint64_t kernel = (uint64_t)&_kernel;
  
  uint32_t l1_idx = pmap_l1_index(va & ~kernel);
  uint32_t l2_idx = pmap_l2_index(va & ~kernel);

  uint64_t entry = _level1_pagetable[l1_idx];
  uint64_t *l2pt = (uint64_t*)(kernel | (entry & Ln_TABLE_PA));

  if(!(entry & l2pt[l2_idx] & Ln_VALID))
    return false;
  
  *res_entry = &l2pt[l2_idx];
  return true;
}


void pmap_kenter(vaddr_t __attribute__((unused)) va,
		 paddr_t __attribute__((unused)) pa,
		 flags_t __attribute__((unused)) flags){
  
}

void pmap_kremove(vaddr_t __attribute__((unused)) va,
		  size_t __attribute__((unused)) size){

  assert(size % 0x1000 == 0);
  size_t pages = size/0x1000;
  
  uint64_t *entry = NULL;
  while(pages--)
    if(get_block_entry(va, &entry))
      *entry = 0x0;
}

bool pmap_kextract(vaddr_t va, paddr_t *pa_p){

  __asm__ __volatile__
    (
     "AT S1E1R, %[vaddr] \n"
     "ISB \n"
     :: [vaddr] "r"(va)
     );
  
  /* D12.2.90 PAR_EL1, Physical Address Register */
  uint64_t par = reg_par_el1_read();
  
  if(!(par & PAR_F))
    { //if not fail
      
      *pa_p = (paddr_t)((par & PAR_PA) | (va & L2_OFFSET));
      //printf("\t %.16p -> %.8p  (%p) \t", va, *pa_p, par);
      return true;

    }

  // simulate table walk
  uint64_t *entry = NULL;

  if( !get_block_entry(va, &entry) )
    return false;
    
  *pa_p = (paddr_t)((*entry & L2_BLOCK_OA) | (va & L2_OFFSET));
  return true;
}

extern uint64_t _level2_pagetable[];
bool pmap_is_referenced(vaddr_t __attribute__((unused)) va){
  uint64_t *entry = NULL;
  get_block_entry(va, &entry);
  return *entry & ATTR_AF;
}

bool pmap_is_modified(vaddr_t __attribute__((unused)) va){
  uint64_t *entry = NULL;
  get_block_entry(va, &entry);
  return *entry & ATTR_DBM;
}


void pmap_clear_referenced(vaddr_t va){
  uint64_t *entry = NULL;
  if(get_block_entry(va, &entry))
    *entry &= ~ATTR_AF;
}

void pmap_clear_modified(vaddr_t __attribute__((unused)) va){
  uint64_t *entry = NULL;
  if(get_block_entry(va, &entry))
    *entry &= ~ATTR_DBM;
}
