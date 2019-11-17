#include "types.h"

extern uint64_t _bss_start[];
extern uint64_t _bss_end[];

#define UPPERADDR 0xffffFFFF00000000UL
#define PHYSADDR(x) ((x) - (UPPERADDR))

void __attribute__((section(".init"))) clear_bss() {
  for (uint64_t *i = (uint64_t *)PHYSADDR((uint64_t)&_bss_start);
      i < (uint64_t *)PHYSADDR((uint64_t)&_bss_end); ++i) {
    *i = 0;
  }
}

