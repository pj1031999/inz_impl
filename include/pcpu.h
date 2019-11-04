#ifndef PCPU_H
#define PCPU_H

#include <aarch64/mmu.h>
#include <aarch64/cpureg.h>
#include <aarch64/pte.h>
#include <aarch64/frame.h>

typedef struct cons cons_t;

typedef struct pcpu {
  cons_t *cons;
  pde_t *pdtab;
  volatile int td_idnest;
  bool on_fault;

  int switch_to; // index in ctx_t table with next context to switch
  struct trapframe *schedule[2];

} pcpu_t;

void pcpu_init(void);

static inline pcpu_t *pcpu(void) {
  return (pcpu_t *)reg_tpidr_el1_read();
}

#endif
