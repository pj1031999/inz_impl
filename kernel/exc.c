#include <arm/cpureg.h>
#include <klibc.h>

__interrupt("UNDEF") void exc_undefined_instruction(void) {
}

__interrupt("SWI") void exc_supervisor_call(void) {
}

__interrupt("ABORT") void exc_prefetch_abort(void) {
}

__interrupt("ABORT") void exc_data_abort(void) {
  uint32_t dfar = armreg_dfar_read();
  uint32_t dfsr = armreg_dfsr_read();

  printf("Illegal %s access at %p (type=%x)!\n",
         (dfsr & FAULT_WRITE) ? "write" : "read", dfar, dfsr & FAULT_TYPE_MASK);
}
