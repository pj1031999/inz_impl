#ifndef MMU_H
#define MMU_H

#include <types.h>

paddr_t mmu_translate(vaddr_t va);

#endif /* !MMU_H */
