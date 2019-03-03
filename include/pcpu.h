#ifndef PCPU_H
#define PCPU_H

#include <arm/mmu.h>

#ifdef AARCH64
#include <aarch64/cpureg.h>
#else
#include <arm/cpureg.h>
#endif



typedef struct cons cons_t;

typedef struct pcpu {
  cons_t *cons;
  pde_t *pdtab;
} pcpu_t;

void pcpu_init(void);

static inline pcpu_t *pcpu(void) {

#ifdef AARCH64
  return (pcpu_t *)reg_tpidr_el1_read();
#else
  return (pcpu_t *)armreg_tpidrprw_read();
#endif
  
}

#endif
