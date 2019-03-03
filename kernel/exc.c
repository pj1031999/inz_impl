#ifdef AARCH64
#include <aarch64/cpureg.h>

#else
#include <arm/cpureg.h>
#endif

#include <klibc.h>

__interrupt("UNDEF") void exc_undefined_instruction(void) {
}

__interrupt("SWI") void exc_supervisor_call(void) {
}

__interrupt("ABORT") void exc_prefetch_abort(void) {
}

__interrupt("ABORT") void exc_data_abort(void) {

#ifdef AARCH64
  uint32_t dfar = reg_far_el1_read();
  uint32_t dfsr = reg_esr_el1_read();
#else
  uint32_t dfar = armreg_dfar_read();
  uint32_t dfsr = armreg_dfsr_read();
#endif

#define FAULT_WRITE	0x0800	/* fault was due to write (ARMv6+) */
#define FAULT_TYPE_MASK 0x0f
  printf("Illegal %s access at %p (type=%x)!\n",
         (dfsr & FAULT_WRITE) ? "write" : "read", dfar, dfsr & FAULT_TYPE_MASK);
}
