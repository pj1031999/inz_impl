#include <arm/cpu.h>
#include <dev/bcm2836reg.h>
#include <pcpu.h>

static pcpu_t _pcpu_data[BCM2836_NCPUS];
extern pde_t _kernel_pde[4096];

void pcpu_init(void) {
  pcpu_t *pcpu = &_pcpu_data[arm_cpu_id()];
  pcpu->cons = NULL;
  pcpu->pdtab = _kernel_pde;
  armreg_tpidrprw_write((uint32_t)pcpu);
}
