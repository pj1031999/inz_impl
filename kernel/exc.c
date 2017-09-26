#include <clock.h>
#include <klibc.h>

#define __interrupt(x) __attribute__ ((interrupt(x))) 

__interrupt("UNDEF") void exc_undefined_instruction(void) {
}

__interrupt("SWI") void exc_supervisor_call(void) {
}

__interrupt("ABORT") void exc_prefetch_abort(void) {
}

__interrupt("ABORT") void exc_data_abort(void) {
}

__interrupt("IRQ") void exc_irq(void) {
  if (clock_irq())
    return;
  printf("Spurious interrupt!\n");
}

__interrupt("FIQ") void exc_fast_irq(void) {
}
