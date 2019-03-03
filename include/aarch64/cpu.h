#ifndef ARMCPU_H
#define ARMCPU_H

//#include <arm/cpureg.h>
#include <aarch64/cpureg.h>

static inline unsigned arm_cpu_id(void) {
  //return armreg_mpidr_read() & CORTEXA9_MPIDR_CPUID;
  return reg_fpsr_read() & MPIDR_AFF0;
}

static inline uint32_t arm_set_cpsr_c(uint32_t clr, uint32_t eor) {
  uint32_t tmp, ret;
  __asm__ volatile(//"mrs %0, cpsr\n"   /* Get the CPSR */
                   //"bic %1, %0, %2\n" /* Clear bits */
                   //"eor %1, %1, %3\n" /* XOR bits */
                   //"msr cpsr_c, %1\n" /* Set the control field of CPSR */
                   ""
                   : "=&r"(ret), "=&r"(tmp)
                   : "r"(clr), "r"(eor)
                   : "memory");

  return ret;
}

static inline void arm_irq_enable(void) {
  //arm_set_cpsr_c(I32_bit, 0);
  arm_set_cpsr_c(DAIF_I, 0);
}

static inline void arm_irq_disable(void) {
  //arm_set_cpsr_c(I32_bit, I32_bit);
  arm_set_cpsr_c(DAIF_I, DAIF_I);
}

/* Data Memory Barrier */
static inline void arm_dmb(void) { __asm__ volatile("dmb" ::: "memory"); }

/* Data Synchronization Barrier */
static inline void arm_dsb(void) { __asm__ volatile("dsb" ::: "memory"); }

/* Instruction Synchronization Barrier */
static inline void arm_isb(void) { __asm__ volatile("isb" ::: "memory"); }

#endif
