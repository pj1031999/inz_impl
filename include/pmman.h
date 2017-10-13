#ifndef PMM_H
#define PMM_H

#include <types.h>

#define PAGESIZE 4096

void pm_init(void);
void pm_add_segment(paddr_t start, paddr_t end);
void pm_reserve(paddr_t start, paddr_t end);
paddr_t pm_alloc(unsigned size);

#endif
