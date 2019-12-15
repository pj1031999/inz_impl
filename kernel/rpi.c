#include "types.h"
#include "aarch64/aarch64reg.h"
#include "aarch64/cpureg.h"
#include "aarch64/pte.h"
#include "dev/bcm2836reg.h"

extern uint64_t _bss_start[];
extern uint64_t _bss_end[];
extern uint64_t _level1_pagetable[];
extern void page_table_fill_inner_nodes(void);
extern void page_table_fill_leaves(void);
extern void _exc_vector(void);
extern uint8_t _el1_stack[];
extern uint8_t _kernel[];

#define UPPERADDR 0xffffffff00000000UL
#define PHYSADDR(x) ((x) - (UPPERADDR))

#define __init__ __attribute__((section(".init")))
#define __inline__ __attribute__((always_inline))
#define __noreturn__ __attribute__((__noreturn__))

__init__ static long get_cpu(void) {
  /* --- we have CPU 0 - 3 so we only need 2 bits of MPIDR_EL1 */
  return READ_SPECIALREG(MPIDR_EL1) & 0x3UL;
}

__init__ static void clear_bss(void) {
  for (uint64_t *i = (uint64_t *)PHYSADDR((uint64_t)&_bss_start);
       i < (uint64_t *)PHYSADDR((uint64_t)&_bss_end); ++i) {
    *i = 0;
  }
}

#if 0
__init__ static intptr_t cpu_wait(void) {
  for (;;) {
    /* --- DO NOTHING */
  }
  return 0xdeadc0de;
}
#endif

__init__ void invalidate_tlb(void) {
  __asm__ volatile("TLBI ALLE1\n"
                   "TLBI vmalle1is\n"
                   "DSB ish\n"
                   "ISB\n");
}

#if 0
__init__ static void setup_tlb(void) {
  invalidate_tlb();
  WRITE_SPECIALREG(TTBR1_EL1, PHYSADDR((uint64_t)&_level1_pagetable));
  WRITE_SPECIALREG(TTBR0_EL1, PHYSADDR((uint64_t)&_level1_pagetable));
}
#endif

__init__ __inline__ static void enable_mmu(void) {
  uint64_t x = MAIR_ATTR(MAIR_DEVICE_nGnRnE, ATTR_DEVICE_MEM)
    | MAIR_ATTR(MAIR_NORMAL_NC, ATTR_NORMAL_MEM_NC)
    | MAIR_ATTR(MAIR_NORMAL_WB, ATTR_NORMAL_MEM_WB)
    | MAIR_ATTR(MAIR_NORMAL_WT, ATTR_NORMAL_MEM_WT);
  __asm__ volatile("DSB sy\n"
                   "ISB\n");

  WRITE_SPECIALREG(MAIR_EL1, x);

  invalidate_tlb();

  /* --- enable magic bits
   * 40 -- Hardware management of dirty state in stage 1 translations from EL0 and EL1.
   * 39 -- Hardware Access flag update in stage 1 translations from EL0 and EL1.
   * 32 -- Set Physical Address size to 4GB.
   * 21 -- The size offset of the memory region addressed by TTBR1_EL1.
   *  5 -- The size offset of the memory region addressed by TTBR1_EL0.
   * 4K -- Granule size for the TTBR0_EL.
   */
  x = TCR_TxSZ(32ULL) | TCR_TGx_(4K) | (0ULL << 32ULL) | (1ULL << 39ULL) | (1ULL << 40ULL);
  uint64_t v = READ_SPECIALREG(id_aa64mmfr0_el1);

  /* --- Support for 16KB memory granule size for stage 2. (id_aa64mmfr0_el1)
   * Intermediate Physical Address Size. (tcr_el1)
   * */
  __asm__ volatile("BFI %0, %1, #32, #3\n"
                   : "+r" (x)
                   : "r" (v));
  WRITE_SPECIALREG(tcr_el1, x);

  /* --- more magic bits
   * M -- MMU enable for EL1 and EL0 stage 1 address translation. 
   * I -- SP must be aligned to 16.
   * C -- Cacheability control, for data accesses.
   */
  v = SCTLR_M | SCTLR_I | SCTLR_C;
  x |= v;

  WRITE_SPECIALREG(sctlr_el1, x);
  __asm__ volatile("DSB sy\n"
                   "ISB\n");
}

__init__ static void enable_cache(void) {
  if (READ_SPECIALREG(CurrentEl) != 0x8) {
    /* --- we are not in el2 */
    /* 1 << 6 -- SMP bit */
    WRITE_SPECIALREG(S3_1_C15_c2_1, READ_SPECIALREG(S3_1_C15_C2_1) | (0x1 << 6));
    __asm__ volatile("DSB SY\n"
                     "ISB\n");
  }
}

intptr_t platform_stack(void) {
  // long cpu = get_cpu();
  intptr_t stack = ((intptr_t)&_el1_stack) - ((intptr_t)&_kernel);;
  
  /* --- TODO(pj):
   * - save boot params
   * - bootstrap thread0
   */
 
  /* --- time to return stack */
  return stack;
}

__noreturn__ void platform_init(void *atags) {
  (void)atags;
  for (;;) {
    /* --- DO NOTHING */
  }
}

__init__ long arm64_init(void) {
  uint64_t x;
  long cpu = get_cpu();
  enable_cache();
  invalidate_tlb();


#if 1
  if (cpu == 0) {
    /* --- now we are CPU0 */
    clear_bss();
    page_table_fill_inner_nodes();
    page_table_fill_leaves();
    WRITE_SPECIALREG(TTBR1_EL1, PHYSADDR((uint64_t)&_level1_pagetable));
    WRITE_SPECIALREG(TTBR0_EL1, PHYSADDR((uint64_t)&_level1_pagetable));
    enable_mmu();
  }
#endif

  /* --- el3_only */
  /* --- 0x8 = EL2 */
  if (READ_SPECIALREG(CurrentEl) != 0x8) {
    /* --- we are not in el2 */
    x = READ_SPECIALREG(SCR_EL3);
    /* --- Execution state control for lower Exception levels. 
     * The next lower level is AArch64 */
    x |= (1 << 10);
    /* --- Non-secure bit. 
     * Indicates that Exception levels lower than EL3 are in Non-secure state, 
     * and so memory accesses from those Exception levels cannot access Secure 
     * memory.
     */
    x |= (1 << 0);
    WRITE_SPECIALREG(SP_EL2, reg_sp_read());
    WRITE_SPECIALREG(SCR_EL3, x);
    WRITE_SPECIALREG(SPSR_EL3, 0b01001);
    WRITE_SPECIALREG(ELR_EL3, &&el2_entry);
    __asm__ volatile ("ERET\n");
  }

el2_entry:
  WRITE_SPECIALREG(HCR_EL2, 0);
  /* The Execution state for EL1 is AArch64. The Execution state for EL0 is determined
   * by the current value of PSTATE.nRW when executing at EL0.
   */
  x = READ_SPECIALREG(HCR_EL2) | (1 << 31);
  WRITE_SPECIALREG(HCR_EL2, x);

  x = READ_SPECIALREG(CNTHCTL_EL2);
  x |= CNTHCTL_EL1PCTEN | CNTHCTL_EL1PCEN;
  /* --- enable timer for EL1 */
  WRITE_SPECIALREG(SP_EL1, reg_sp_read());
  WRITE_SPECIALREG(CNTHCTL_EL2, x);
  WRITE_SPECIALREG(SPSR_EL2, 0b0101);
  WRITE_SPECIALREG(ELR_EL2, &&el1_entry);
  __asm__ volatile ("ERET\n");

el1_entry:
  WRITE_SPECIALREG(VBAR_EL1, _exc_vector);
  /* --- I don't have any idea why this instruction doesn't work */
#if 0
  WRITE_SPECIALREG(DAIFClr, 3);
#else
  __asm__ volatile ("MSR DAIFClr, #3\n");
#endif

  if (cpu == 0) {
    /* --- now we are CPU0 */
#if 0
    clear_bss();
    page_table_fill_inner_nodes();
    page_table_fill_leaves();
    WRITE_SPECIALREG(TTBR1_EL1, PHYSADDR((uint64_t)&_level1_pagetable));
    WRITE_SPECIALREG(TTBR0_EL1, PHYSADDR((uint64_t)&_level1_pagetable));
    enable_mmu();
    // WRITE_SPECIALREG(VBAR_EL1, _exc_vector);
#endif
  } else {
    for (;;) {
      /* --- DO NOTHING */
    }
  }

  return cpu;
}
