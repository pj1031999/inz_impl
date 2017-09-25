#ifndef ARMMMU_H
#define ARMMMU_H

#include <stdint.h>

/* ARMv6 page directory and page table entries */

#define PDE_TYPE_FAULT 0
#define PDE_TYPE_COARSE 1
#define PDE_TYPE_SECTION 2
#define PDE_TYPE_RESERVED 3
#define PDE_TYPE_MASK 3

typedef struct {
  union {
    struct {
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
    } section;
    struct {
      uint32_t type : 2;
      uint32_t pxn : 1;
      uint32_t ns : 1;
      uint32_t sbz : 1;
      uint32_t domain : 4;
      uint32_t impl : 1;
      uint32_t base_address : 20;
    } coarse;
    uint32_t raw;
  };
} pde_t;

#define PTE_TYPE_FAULT 0
#define PTE_TYPE_LARGE_PAGE 1
#define PTE_TYPE_PAGE 2
#define PTE_TYPE_PAGE_XN 3
#define PTE_TYPE_MASK 3

typedef struct {
  union {
    struct {
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
    } large_page;
    struct {
      uint32_t type : 2;
      uint32_t b : 1;
      uint32_t c : 1;
      uint32_t ap : 2;
      uint32_t tex : 3;
      uint32_t apx : 1;
      uint32_t s : 1;
      uint32_t ng : 1;
      uint32_t base_address : 20;
    } page;
    uint32_t raw;
  };
} pte_t;

#endif // #ifndef ARMMMU_H
