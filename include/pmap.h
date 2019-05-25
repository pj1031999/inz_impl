#ifndef _PMAP_H_
#define _PMAP_H_

#include <types.h>

//typedef uint64_t vaddr_t;
//typedef uint64_t paddr_t;
typedef uint64_t flags_t;

void pmap_kenter(vaddr_t, paddr_t, flags_t);
void pmap_kremove(vaddr_t, size_t);
bool pmap_kextract(vaddr_t, paddr_t*);

bool pmap_is_referenced(vaddr_t);
bool pmap_is_modified(vaddr_t);

void pmap_clear_referenced(vaddr_t);
void pmap_clear_modified(vaddr_t);

bool get_block_entry(vaddr_t, uint64_t**);

#endif /* !_PMAP_H_ */
