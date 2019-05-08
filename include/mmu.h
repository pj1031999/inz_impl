#ifndef MMU_H
#define MMU_H

#include <types.h>

extern uint8_t _kernel[];

paddr_t mmu_translate(vaddr_t va);
//vaddr_t mmu_translate(paddr_t pa);

#define mem_p2v(X) (X+(paddr_t)_kernel)
#define mem_v2p(X) (X-(paddr_t)_kernel)

#endif /* !MMU_H */
