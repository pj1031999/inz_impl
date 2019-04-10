#ifndef PCPU_H
#define PCPU_H

#include <arm/mmu.h>
#include <aarch64/cpureg.h>

typedef struct cons cons_t;

typedef struct pcpu {
  cons_t *cons;
  pde_t *pdtab;
} pcpu_t;

void pcpu_init(void);

static inline pcpu_t *pcpu(void) {
  //return (pcpu_t *)reg_tpidrro_el0_read();
  return (pcpu_t *)reg_tpidr_el1_read();
  //return (pcpu_t *)armreg_tpidrprw_read();
  
}

#endif
