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

void __init__ clear_bss(void) {
  for (uint64_t *i = (uint64_t *)PHYSADDR((uint64_t)&_bss_start);
      i < (uint64_t *)PHYSADDR((uint64_t)&_bss_end); ++i) {
    *i = 0;
  }
}

void __init__ enable_cache(void) {
  WRITE_SPECIALREG(S3_1_C15_c2_1, READ_SPECIALREG(S3_1_C15_C2_1) | (0x1 << 6));
  __asm__ volatile("DSB SY\n"
                   "ISB\n");
}

void __init__ __inline__ invalidate_tlb(void) {
  __asm__ volatile("TLBI ALLE1\n"
                   "TLBI vmalle1is\n"
                   "DSB ish\n"
                   "ISB\n");
}

void __init__ enable_mmu(uint64_t x) {
  WRITE_SPECIALREG(sctlr_el1, x);
  __asm__ volatile("DSB sy\n"
                   "ISB\n");
}

void __init__ __inline__ setup_tmp_stack(void) {
  __asm__ volatile("MOV X1, X29\n"
                   "LDR X2, =_kernel\n"
                   "SUBS X1, X1, X2\n"
                   "MOV SP, X1\n");
}

void __init__ el3_only(void) {
  WRITE_SPECIALREG(SCR_EL3, READ_SPECIALREG(SCR_EL3) | (0x1 << 10) | (0x1 << 0));
  WRITE_SPECIALREG(SPSR_EL3, 0b01001);
}

void __init__ setup_tlb(void) {
  invalidate_tlb();
  WRITE_SPECIALREG(TTBR1_EL1, PHYSADDR((uint64_t)&_level1_pagetable));
  WRITE_SPECIALREG(TTBR0_EL1, PHYSADDR((uint64_t)&_level1_pagetable));
  page_table_fill_inner_nodes();
  page_table_fill_leaves();
}


void __init__ change_el(void) {
  __asm__ volatile("MSR HCR_EL2, XZR\n");
  WRITE_SPECIALREG(HCR_EL2, READ_SPECIALREG(HCR_EL2) | (1<<31));
}

void __init__ el2_entry(void) {
  WRITE_SPECIALREG(cnthctl_el2, READ_SPECIALREG(cnthctl_el2) | CNTHCTL_EL1PCTEN | CNTHCTL_EL1PCEN);
  WRITE_SPECIALREG(SPSR_EL2, 0b0101);
}

void __init__ setup_mmu(void) {
  uint64_t x = MAIR_ATTR(MAIR_DEVICE_nGnRnE, ATTR_DEVICE_MEM)
      | MAIR_ATTR(MAIR_NORMAL_NC, ATTR_NORMAL_MEM_NC)
      | MAIR_ATTR(MAIR_NORMAL_WB, ATTR_NORMAL_MEM_WB)
      | MAIR_ATTR(MAIR_NORMAL_WT, ATTR_NORMAL_MEM_WT);
  __asm__ volatile("DSB sy\n"
                   "ISB\n");
  
  WRITE_SPECIALREG(MAIR_EL1, x);

  invalidate_tlb();

  x = TCR_TxSZ(32ULL) | TCR_TGx_(4K) | (1ULL<<39ULL) | (1ULL<<40ULL);
  uint64_t v = READ_SPECIALREG(id_aa64mmfr0_el1);
  __asm__ volatile("BFI %0, %1, #32, #3\n"
                   : "+r" (x)
                   : "r" (v));
  WRITE_SPECIALREG(tcr_el1, x);
  v = SCTLR_M | SCTLR_I | SCTLR_C;
  x |= v;
  enable_mmu(x);
}
