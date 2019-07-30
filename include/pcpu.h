#ifndef PCPU_H
#define PCPU_H

#include <aarch64/mmu.h>
#include <aarch64/cpureg.h>
#include <aarch64/pte.h>

typedef struct cons cons_t;

typedef struct pcpu {
  cons_t *cons;
  pde_t *pdtab;
  volatile int td_idnest;
  bool on_fault;
} pcpu_t;

void pcpu_init(void);

static inline pcpu_t *pcpu(void) {
  return (pcpu_t *)reg_tpidr_el1_read();
}

#endif
