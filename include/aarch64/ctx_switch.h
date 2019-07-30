#include "aarch64/assym.h"

#define SAVE_REG(reg, base, offset) str reg, [base, offset]
#define LOAD_REG(reg, base, offset) ldr reg, [base, offset]

#define SAVE_REGS(reg1, reg2, base, offset) stp reg1, reg2, [base, offset]
#define LOAD_REGS(reg1, reg2, base, offset) ldp reg1, reg2, [base, offset]

#define SAVE_SPEC_REG(reg, base, offset) \
  mov x8, reg;\
  str x8, [base, offset]

#define LOAD_SPEC_REG(reg, base, offset)\
  ldr x8, [base, offset];\
  mov reg, x8

#define SAVE_SYS_REG(reg, base, offset) \
  mrs x8, reg;\
  str x8, [base, offset]

#define LOAD_SYS_REG(reg, base, offset)\
  ldr x8, [base, offset];\
  msr reg, x8


#define CTX_SAVE(reg)				\
  DISABLE_INTERRUPT_ASM;			\
  SAVE_SPEC_REG(sp, reg, SF_SP);		\
  SAVE_SPEC_REG(lr, reg, SF_LR);		\
  SAVE_SPEC_REG(., reg, SF_PC);			\
  SAVE_SYS_REG(SPSR_EL1, reg, SF_SPSR);		\
  /* Store the callee-saved registers */	\
  SAVE_REGS( x8,  x9,  reg, SF_REGS + 8 * 8);	\
  SAVE_REGS( x10, x11, reg, SF_REGS + 8 * 10);	\
  SAVE_REGS( x12, x13, reg, SF_REGS + 8 * 12);	\
  SAVE_REGS( x14, x15, reg, SF_REGS + 8 * 14);	\
  SAVE_REGS( x16, x17, reg, SF_REGS + 8 * 16);	\
  SAVE_REGS( x18, x19, reg, SF_REGS + 8 * 18);	\
  SAVE_REGS( x20, x21, reg, SF_REGS + 8 * 20);	\
  SAVE_REGS( x22, x23, reg, SF_REGS + 8 * 22);	\
  SAVE_REGS( x24, x25, reg, SF_REGS + 8 * 24);	\
  SAVE_REGS( x26, x27, reg, SF_REGS + 8 * 26);	\
  SAVE_REGS( x28, x29, reg, SF_REGS + 8 * 28);	\
  SAVE_REG(  x30, reg, SF_REGS + 8 * 30);	\
  ENABLE_INTERRUPT_ASM;


#define LOAD_CTX(reg)				\
  DISABLE_INTERRUPT_ASM;			\
  LOAD_SPEC_REG(sp, reg, SF_SP);		\
  LOAD_SPEC_REG(lr, reg, SF_LR);		\
  LOAD_SYS_REG(SPSR_EL1, reg, SF_SPSR);		\
  /* Restore the callee-saved registers */	\
  LOAD_REGS( x8,  x9,  reg, SF_REGS + 8 * 8);	\
  LOAD_REGS( x10, x11, reg, SF_REGS + 8 * 10);	\
  LOAD_REGS( x12, x13, reg, SF_REGS + 8 * 12);	\
  LOAD_REGS( x14, x15, reg, SF_REGS + 8 * 14);	\
  LOAD_REGS( x16, x17, reg, SF_REGS + 8 * 16);	\
  LOAD_REGS( x18, x19, reg, SF_REGS + 8 * 18);	\
  LOAD_REGS( x20, x21, reg, SF_REGS + 8 * 20);	\
  LOAD_REGS( x22, x23, reg, SF_REGS + 8 * 22);	\
  LOAD_REGS( x24, x25, reg, SF_REGS + 8 * 24);	\
  LOAD_REGS( x26, x27, reg, SF_REGS + 8 * 26);	\
  LOAD_REGS( x28, x29, reg, SF_REGS + 8 * 28);	\
  LOAD_REG(  x30, reg, SF_REGS + 8 * 30);	\
  ENABLE_INTERRUPT_ASM;

#define	DAIF_D			1<<9	// Debug Exception Mask
#define	DAIF_A			1<<8	// SError Abort Mask
#define	DAIF_I			1<<7	// IRQ Mask
#define	DAIF_F		        1<<6	// FIQ Mask
#define	DAIF_SETCLR_SHIFT	6	// for daifset/daifclr #imm shift

#define ENABLE_INTERRUPT_ASM    \
  msr daifclr, #((DAIF_I|DAIF_F) >> DAIF_SETCLR_SHIFT);
#define DISABLE_INTERRUPT_ASM   \
  msr daifset, #((DAIF_I|DAIF_F) >> DAIF_SETCLR_SHIFT);
