#ifndef PMAP_TEST_H
#define PMAP_TEST_H

#include <types.h>

void pmap_test_kremove();
void pmap_test_kenter();
void pmap_test_clear_referenced(vaddr_t va);
void pmap_test_clear_modified(vaddr_t va);
void pmap_test_kextract_accessed(vaddr_t va);
void pmap_test_kextract(vaddr_t va);

#endif /* !PMAP_TEST_H */
