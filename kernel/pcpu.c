#include <arm/cpureg.h>
#include <dev/bcm2836reg.h>
#include <smp.h>
#include <pcpu.h>

static pcpu_t _pcpu_data[BCM2836_NCPUS];

void pcpu_init(void) {
  armreg_tpidrprw_write((uint32_t)&_pcpu_data[arm_cpu_id()]);
}
