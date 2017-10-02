#ifndef SMP_H
#define SMP_H

#include <armreg.h>

static inline unsigned smp_cpu_id(void) {
  return armreg_mpidr_read() & CORTEXA9_MPIDR_CPUID;
}

void smp_bootstrap(void);

#endif
