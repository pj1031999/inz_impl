#include <dev/bcm2836reg.h>
#include <pcpu.h>


#ifdef AARCH64
#include <aarch64/cpu.h>
static pcpu_t _pcpu_data[BCM2836_NCPUS];
extern pde_t _level2_pagetable[8192];

void pcpu_init(void) {
  pcpu_t *pcpu = &_pcpu_data[arm_cpu_id()];
  pcpu->cons = NULL;
  
  pcpu->pdtab = _level2_pagetable;
  reg_tpidr_el1_write((uint64_t)pcpu);
}



#else
#include <arm/cpu.h>

static pcpu_t _pcpu_data[BCM2836_NCPUS];
extern pde_t _kernel_pde[4096];

void pcpu_init(void) {
  pcpu_t *pcpu = &_pcpu_data[arm_cpu_id()];
  pcpu->cons = NULL;
  
  pcpu->pdtab = _kernel_pde;
  armreg_tpidrprw_write((uint32_t)pcpu);
}

#endif


