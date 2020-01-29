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
extern void kernel_entry(uint32_t, uint32_t, uint32_t);
extern uint8_t _el1_stack[];
extern uint8_t _kernel[];

#define UPPERADDR 0xffffffff00000000UL
#define PHYSADDR(x) ((x) - (UPPERADDR))

#define __init __attribute__((section(".init")))
#define __inline __attribute__((always_inline))
#define __noreturn __attribute__((__noreturn__))
#define __wfe() __asm__ volatile("WFE")
#define __dsb(x) __asm__ volatile("DSB " x)
#define __isb() __asm__ volatile("ISB")
#define __tlbi(x) __asm__ volatile("TLBI " x)
#define __eret() __asm__ volatile("ERET")

__init static long get_cpu(void) {
  /* --- we have CPU 0 - 3 so we only need 2 bits of MPIDR_EL1 */
  return READ_SPECIALREG(MPIDR_EL1) & 0x3UL;
}

__init static void clear_bss(void) {
  for (uint64_t *i = (uint64_t *)PHYSADDR((uint64_t)&_bss_start);
       i < (uint64_t *)PHYSADDR((uint64_t)&_bss_end); ++i) {
    *i = 0;
  }
}

typedef struct {
  intptr_t pc;
  intptr_t sp;
} cpuctx_t;

__init static cpuctx_t cpu_wait(void) {
  long cpu = get_cpu();

  /* --- we have 4 mailboxes for each CPU in continuous sets */
  uint32_t *mailbox = (uint32_t *)(BCM2836_ARM_LOCAL_BASE + 
                                   BCM2836_LOCAL_MAILBOX0_CLRN(cpu));

  intptr_t jump = 0;
  do {
    __wfe();
    /* --- read #3 mailbox for this cpu
     * it contains *relative* jump address
     */
    jump = mailbox[3];
  } while (!jump);

  /* --- clear mailbox */
  mailbox[3] = jump;  

  /* --- add missing offset */
  jump += (intptr_t)&_kernel;

  __dsb("sy");
  __isb();

  intptr_t stack = 0;
  do {
    __wfe();
    /* --- read #1 mailbox for this CPU
     * it contains new stack address
     */
    stack = mailbox[1];
  } while (!stack);

  /* --- clear mailbox */
  mailbox[1] = stack;

  /* --- add missing offset */
  stack += (intptr_t)&_kernel;
  
  return (cpuctx_t){.pc = jump, .sp = stack};
}

__init void invalidate_tlb(void) {
  __tlbi("alle1");
  __tlbi("vmalle1is");
  __dsb("ish");
  __isb();
}

__init __inline static void enable_mmu(void) {
  uint64_t x = MAIR_ATTR(MAIR_DEVICE_nGnRnE, ATTR_DEVICE_MEM)
    | MAIR_ATTR(MAIR_NORMAL_NC, ATTR_NORMAL_MEM_NC)
    | MAIR_ATTR(MAIR_NORMAL_WB, ATTR_NORMAL_MEM_WB)
    | MAIR_ATTR(MAIR_NORMAL_WT, ATTR_NORMAL_MEM_WT);
  __dsb("sy");
  __isb();

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

  /* --- Support for 16KB memory granule size for stage 2. (id_aa64mmfr0_el1)
   * Intermediate Physical Address Size. (tcr_el1)
   * */
  uint64_t v = READ_SPECIALREG(id_aa64mmfr0_el1);
  WRITE_SPECIALREG(tcr_el1, x | ((v & 3) << 32));

  /* --- more magic bits
   * M -- MMU enable for EL1 and EL0 stage 1 address translation. 
   * I -- SP must be aligned to 16.
   * C -- Cacheability control, for data accesses.
   */
  v = SCTLR_M | SCTLR_I | SCTLR_C;
  x |= v;

  WRITE_SPECIALREG(sctlr_el1, x);
  __dsb("sy");
  __isb();
}

__init static void enable_cache(void) {
  if (READ_SPECIALREG(CurrentEl) != 0x8) {
    /* --- we are not in el2 */
    /* 1 << 6 -- SMP bit */
    WRITE_SPECIALREG(S3_1_C15_c2_1, READ_SPECIALREG(S3_1_C15_C2_1) | (1 << 6));
    __dsb("sy");
    __isb();
  }
}

intptr_t platform_stack(void) {
  intptr_t stack = ((intptr_t)&_el1_stack) - ((intptr_t)&_kernel);;
  
  /* --- TODO(pj):
   * - save boot params
   * - bootstrap thread0
   */
 
  /* --- time to return stack */
  return stack;
}

__noreturn void platform_init(void *atags) {
  (void)atags;

  /* shields up, weapons armed - going live */
  kernel_entry(0, 0, 0);

  for (;;) {
    /* --- DO NOTHING */
  }
}

__init cpuctx_t arm64_init(void) {
  cpuctx_t ctx;
  uint64_t x;
  long cpu;
  
  cpu = get_cpu();
  enable_cache();
  invalidate_tlb();

  if (cpu != 0) {
    ctx = cpu_wait();
  } else {
    clear_bss();
    page_table_fill_inner_nodes();
    page_table_fill_leaves();
    ctx = (cpuctx_t){.pc = 0, .sp = 0};
  }

  WRITE_SPECIALREG(TTBR1_EL1, PHYSADDR((uint64_t)&_level1_pagetable));
  WRITE_SPECIALREG(TTBR0_EL1, PHYSADDR((uint64_t)&_level1_pagetable));
  enable_mmu();

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
    __eret();
  }

el2_entry:
  WRITE_SPECIALREG(HCR_EL2, 0);
  /* The Execution state for EL1 is AArch64. The Execution state for EL0 is determined
   * by the current value of PSTATE.nRW when executing at EL0.
   */
  x = READ_SPECIALREG(HCR_EL2) | (1 << 31);
  WRITE_SPECIALREG(HCR_EL2, x);

  x = READ_SPECIALREG(CNTHCTL_EL2) | CNTHCTL_EL1PCTEN | CNTHCTL_EL1PCEN;
  /* --- enable timer for EL1 */
  WRITE_SPECIALREG(SP_EL1, reg_sp_read());
  WRITE_SPECIALREG(CNTHCTL_EL2, x);
  WRITE_SPECIALREG(SPSR_EL2, 0b0101);
  WRITE_SPECIALREG(ELR_EL2, &&el1_entry);
  __eret();

el1_entry:
  WRITE_SPECIALREG(VBAR_EL1, _exc_vector);
  /* --- I don't have any idea why this instruction doesn't work */
#if 0
  WRITE_SPECIALREG(DAIFClr, 3);
#else
  __asm__ volatile ("MSR DAIFClr, #3\n");
#endif

  return ctx;
}
