#include <dev/bcm2836reg.h>
#include <pcpu.h>
#include <aarch64/cpu.h>

static pcpu_t _pcpu_data[BCM2836_NCPUS];
//extern int _stack_size;
extern pde_t _level1_pagetable[];

void pcpu_init(void) {
  pcpu_t *pcpu = &_pcpu_data[arm_cpu_id()];
  pcpu->cons = NULL;
  pcpu->pdtab = _level1_pagetable;
  pcpu->td_idnest = 1;
  pcpu->on_fault = false;
  
  reg_tpidr_el1_write((uint64_t)pcpu);
}
