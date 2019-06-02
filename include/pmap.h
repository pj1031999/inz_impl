#ifndef _PMAP_H_
#define _PMAP_H_

#include <types.h>
#include <aarch64/pte.h>
#include <queue.h>

extern vaddr_t* _kernel;

typedef uint64_t flags_t;
typedef int32_t pt_lvl_t;

#define FLAG_MEM_RO 	ATTR_AP(ATTR_AP_RO)
#define FLAG_MEM_RW 	ATTR_AP(ATTR_AP_RW)
#define FLAG_MEM_NOT_EX ATTR_XN
#define FLAG_MEM_USER_SPACE ATTR_AP(ATTR_AP_USER)

#define FLAG_MEM_NON_CACHEABLE 	ATTR_IDX(ATTR_NORMAL_MEM_NC)
#define FLAG_MEM_WRITE_BACK 	ATTR_IDX(ATTR_NORMAL_MEM_WB)
#define FLAG_MEM_WRITE_THROUGH 	ATTR_IDX(ATTR_NORMAL_MEM_WT)
#define FLAG_MEM_DEVICE 	ATTR_IDX(ATTR_DEVICE_MEM)

typedef uint8_t asid_t;
typedef uint64_t pte_t;

typedef struct pmap { 
  pte_t *pt_root;   	/* root of page table (first level table) */
  vaddr_t start, end;
  asid_t asid;
} pmap_t;

pmap_t kernel_space;
pmap_t user_space;

void pmap_setup_kernel_space();
void pmap_setup_user_space();

void pmap_kenter(vaddr_t, paddr_t, flags_t);
void pmap_kremove(vaddr_t, size_t);
bool pmap_kextract(vaddr_t, paddr_t*);

bool pmap_is_referenced(vaddr_t);
bool pmap_is_modified(vaddr_t);

void pmap_clear_referenced(vaddr_t);
void pmap_clear_modified(vaddr_t);

void pmap_data_abort_access_fault(vaddr_t);

#endif /* !_PMAP_H_ */
