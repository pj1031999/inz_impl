#include <mmu.h>

paddr_t mmu_translate(vaddr_t va) {
  return va - (paddr_t)_kernel;
}

/* vaddr_t mmu_translate(paddr_t pa) { */
/*   return pa + (paddr_t)_kernel; */
/* } */


