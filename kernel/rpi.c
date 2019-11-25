#include "types.h"
#include "aarch64/aarch64reg.h"
#include "aarch64/pte.h"
#include "dev/bcm2836reg.h"

extern uint64_t _bss_start[];
extern uint64_t _bss_end[];
extern uint64_t _level1_pagetable[];
extern void page_table_fill_inner_nodes(void);
extern void page_table_fill_leaves(void);

#define UPPERADDR 0xffffFFFF00000000UL
#define PHYSADDR(x) ((x) - (UPPERADDR))

#define __init__ __attribute__((section(".init")))
#define __inline__ __attribute__((always_inline))

void __init__ clear_bss() {
  for (uint64_t *i = (uint64_t *)PHYSADDR((uint64_t)&_bss_start);
      i < (uint64_t *)PHYSADDR((uint64_t)&_bss_end); ++i) {
    *i = 0;
  }
}


void __init__ enable_cache() {
  uint64_t x;
  __asm__ volatile("MRS %0, S3_1_C15_C2_1\n"
                   : "=r" (x)
                   :
      );
  x |= (0x1 << 6); // The SMP bit
  __asm__ volatile("MSR S3_1_C15_c2_1, %0\n"
                   "DSB SY\n"
                   "ISB\n"
                   : 
                   : "r" (x));
}

void __init__ __inline__ invalidate_tlb() {
  __asm__ volatile("TLBI ALLE1\n"
                   "TLBI vmalle1is\n"
                   "DSB ish\n"
                   "ISB\n");
}

void __init__ enable_mmu() {
  __asm__ volatile("MSR sctlr_el1, x0\n"
                   "DSB sy\n"
                   "ISB\n");
}

void __init__ setup_tmp_stack() {
  __asm__ volatile("MOV X1, X29\n"
                   "LDR X2, =_kernel\n"
                   "SUBS X1, X1, X2\n"
                   "MOV SP, X1\n");
}

void __init__ el3_only() {
  uint64_t x;
  __asm__ volatile("MRS %0, SCR_EL3\n"
                   : "=r" (x)
                   :
      );

  x |= (0x1 << 10); // RW EL2 Execution state is AArch64.
  x |= (0x1 << 0);  // NS EL1 is Non-secure world.

  __asm__ volatile("MSR SCR_EL3, %0\n"  // DAIF=0000
                   "MSR SPSR_EL3, %1\n" // M[4:0]=01001 EL2h must match SCR_EL3.RW
                   : 
                   : "r" (x), "r" (0b01001))
    ;
}

void __init__ setup_tlb() {
  invalidate_tlb();

  uint64_t x = PHYSADDR((uint64_t)&_level1_pagetable);
  __asm__ volatile("MSR TTBR1_EL1, %0\n"
                   "MSR TTBR0_EL1, %0\n"
                   : \
                   : "r" (x) \
                  );
}

