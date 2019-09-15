#ifndef ARMCPU_H
#define ARMCPU_H

#include <aarch64/cpureg.h>
#include <pcpu.h>
#include <klibc.h>

static inline unsigned arm_cpu_id(void) {
  return reg_mpidr_el1_read() & MPIDR_AFF0;
}

#if 0
static inline uint32_t arm_set_cpsr_c(uint32_t clr, uint32_t eor) {
  uint32_t tmp, ret;
  
  __asm__ volatile(//"mrs %0, cpsr\n"   /* Get the CPSR */
                   "bic %1, %0, %2\n" /* Clear bits */
                   "eor %1, %1, %3\n" /* XOR bits */
                   //"msr cpsr_c, %1\n" /* Set the control field of CPSR */
                   : "=&r"(ret), "=&r"(tmp)
                   : "r"(clr), "r"(eor)
                   : "memory");

  return ret;
}
#endif

#define ENABLE_INTERRUPT_ASM    \
  msr daifclr, #((DAIF_I|DAIF_F) >> DAIF_SETCLR_SHIFT)
#define DISABLE_INTERRUPT_ASM   \
  msr daifset, #((DAIF_I|DAIF_F) >> DAIF_SETCLR_SHIFT)


#define ENABLE_INTERRUPT()	daif_enable(DAIF_I|DAIF_F)
#define DISABLE_INTERRUPT()	daif_disable(DAIF_I|DAIF_F)

#define DAIF_MASK		(DAIF_D|DAIF_A|DAIF_I|DAIF_F)

typedef uint64_t register_t;
static void
daif_enable(register_t psw)
{
	if (!__builtin_constant_p(psw)) {
		reg_daif_write(reg_daif_read() & ~psw);
	} else {
		reg_daifclr_write((psw & DAIF_MASK) >> DAIF_SETCLR_SHIFT);
	}
}

static register_t
daif_disable(register_t psw)
{
	register_t oldpsw = reg_daif_read();
	if (!__builtin_constant_p(psw)) {
		reg_daif_write(oldpsw | psw);
	} else {
		reg_daifset_write((psw & DAIF_MASK) >> DAIF_SETCLR_SHIFT);
	}
	return oldpsw;
}

/* Data Memory Barrier */
static inline void arm_dmb(void) { __asm__ volatile("dmb sy" ::: "memory"); }

/* Data Synchronization Barrier */
static inline void arm_dsb(void) { __asm__ volatile("dsb sy" ::: "memory"); }

/* Instruction Synchronization Barrier */
static inline void arm_isb(void) { __asm__ volatile("isb" ::: "memory"); }


static inline void arm_irq_enable(void) {
  assert(pcpu()->td_idnest > 0);

  pcpu()->td_idnest--;
  if(pcpu()->td_idnest == 0){
    ENABLE_INTERRUPT();
  }
  arm_isb();
  arm_dsb();
  arm_dmb();
}

static inline void arm_irq_disable(void) {
  DISABLE_INTERRUPT();

  arm_isb();
  arm_dsb();
  arm_dmb();

  pcpu()->td_idnest++;

  arm_isb();
  arm_dsb();
  arm_dmb();

}

#endif
