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


void __attribute__((section(".init"))) enable_cache() {
  __asm__ volatile("MRS X0, S3_1_C15_C2_1\n"
                   "ORR X0, X0, #(0x1 << 6)\n" // The SMP bit
                   "MSR S3_1_C15_c2_1, X0\n"
                   "DSB SY\n"
                   "ISB\n");
}

void __attribute__((section(".init"))) invalidate_tlb() {
  __asm__ volatile("TLBI ALLE1\n"
                   "TLBI vmalle1is\n"
                   "DSB ish\n"
                   "ISB\n");
}
