#ifndef PCPU_H
#define PCPU_H

#include <armreg.h>

typedef struct cons cons_t;

typedef struct pcpu {
  cons_t *cons;
} pcpu_t;

void pcpu_init(void);

static inline pcpu_t *pcpu(void) {
  return (pcpu_t *)armreg_tpidrprw_read();
}

#endif
