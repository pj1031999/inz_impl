#include <mmu.h>

extern uint8_t _kernel[];

paddr_t mmu_translate(vaddr_t va) {
  return va - (paddr_t)_kernel;
}
