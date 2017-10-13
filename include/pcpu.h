#ifndef PCPU_H
#define PCPU_H

#include <arm/cpureg.h>
#include <arm/mmu.h>

typedef struct cons cons_t;

typedef struct pcpu {
  cons_t *cons;
  pde_t *pdtab;
} pcpu_t;

void pcpu_init(void);

static inline pcpu_t *pcpu(void) {
  return (pcpu_t *)armreg_tpidrprw_read();
}

#endif
