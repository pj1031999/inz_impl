#ifndef ARMMMU_H
#define ARMMMU_H

#include <types.h>

/* ARMv6 page directory and page table entries */

#define PDE_TYPE_FAULT 0
#define PDE_TYPE_TABLE 1
#define PDE_TYPE_SECTION 2
#define PDE_TYPE_RESERVED 3
#define PDE_TYPE_MASK 3

typedef struct {
  uint32_t type : 2;
  uint32_t b : 1;
  uint32_t c : 1;
  uint32_t xn : 1;
  uint32_t domain : 4;
  uint32_t impl : 1;
  uint32_t ap : 2;
  uint32_t tex : 3;
  uint32_t apx : 1;
  uint32_t s : 1;
  uint32_t ng : 1;
  uint32_t zero : 1;
  uint32_t ns : 1;
  uint32_t base_address : 12;
} pde_section_t;

typedef struct {
    uint32_t type : 2;
    uint32_t pxn : 1;
    uint32_t ns : 1;
    uint32_t sbz : 1;
    uint32_t domain : 4;
    uint32_t impl : 1;
    uint32_t base_address : 20;
} pde_table_t;

typedef union {
  pde_table_t table;
  pde_section_t section;
  uint32_t raw;
} pde_t;

#define PTE_TYPE_FAULT 0
#define PTE_TYPE_LARGE_PAGE 1
#define PTE_TYPE_PAGE 2
#define PTE_TYPE_PAGE_XN 3
#define PTE_TYPE_MASK 3

typedef struct {
  uint32_t type : 2;
  uint32_t b : 1;
  uint32_t c : 1;
  uint32_t ap : 2;
  uint32_t tex : 3;
  uint32_t apx : 1;
  uint32_t s : 1;
  uint32_t ng : 1;
  uint32_t base_address : 20;
} pte_page_t;

typedef struct {
  uint32_t type : 2;
  uint32_t b : 1;
  uint32_t c : 1;
  uint32_t ap : 2;
  uint32_t sbz : 3;
  uint32_t apx : 1;
  uint32_t s : 1;
  uint32_t ng : 1;
  uint32_t tex : 3;
  uint32_t xn : 1;
  uint32_t base_address : 16;
} pte_large_page_t;

typedef union {
  pte_page_t page;
  pte_large_page_t large_page;
  uint32_t raw;
} pte_t;

#endif // #ifndef ARMMMU_H
