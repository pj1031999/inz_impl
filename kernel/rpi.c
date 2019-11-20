#include "types.h"
#include "aarch64/aarch64reg.h"
#include "aarch64/pte.h"
#include "dev/bcm2836reg.h"


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

void __attribute__((section(".init"))) enable_mmu() {
  __asm__ volatile("MSR sctlr_el1, x0\n"
                   "DSB sy\n"
                   "ISB\n");
}

void __attribute__((section(".init"))) setup_tmp_stack() {
  __asm__ volatile("MOV X1, X29\n"
                   "LDR X2, =_kernel\n"
                   "SUBS X1, X1, X2\n"
                   "MOV SP, X1\n");
}

void __attribute__((section(".init"))) el3_only() {
  __asm__ volatile("MRS X0, SCR_EL3\n"
                   "ORR X0, X0, #(1<<10)\n" // RW EL2 Execution state is AArch64.
                   "ORR X0, X0, #(1<<0)\n" // NS EL1 is Non-secure world.
                   "MSR SCR_EL3, X0\n" 
                   "MOV X0, #0b01001\n" // DAIF=0000
                   "MSR SPSR_EL3, X0\n"); // M[4:0]=01001 EL2h must match SCR_EL3.RW
}
