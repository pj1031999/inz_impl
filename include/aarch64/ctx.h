#ifndef __ARMV8_CTX_H__
#define __ARMV8_CTX_H__

#define CTX_X19 0x0
#define CTX_X20 0x8
#define CTX_X21 0x10
#define CTX_X22 0x18
#define CTX_X23 0x20
#define CTX_X24 0x28
#define CTX_X25 0x30
#define CTX_X26 0x38
#define CTX_X27 0x40
#define CTX_X28 0x48
#define CTX_FP 0x50
#define CTX_LR 0x58
#define CTX_SP 0x60
#define CTX_PC 0x68
#define CTX_SPSR 0x70
#define CTX_SIZE 0x80


#define SAVE_REG(reg, offset, base) str reg, [base, #CTX_##offset]
#define LOAD_REG(reg, offset, base) ldr reg, [base, #CTX_##offset]

#define SAVE_SPEC_REG(reg, offset, base) \
  mov x8, reg;\
  str x8, [base, #CTX_##offset]

#define LOAD_SPEC_REG(reg, offset, base)\
  ldr x8, [base, #CTX_##offset];\
  mov reg, x8

#define SAVE_SYS_REG(reg, offset, base) \
  mrs x8, reg;\
  str x8, [base, #CTX_##offset]

#define LOAD_SYS_REG(reg, offset, base)\
  ldr x8, [base, #CTX_##offset];\
  msr reg, x8


#define SAVE_CTX(reg) \
  SAVE_REG(x19, X19, reg);\
  SAVE_REG(x20, X20, reg);\
  SAVE_REG(x21, X21, reg);\
  SAVE_REG(x22, X22, reg);\
  SAVE_REG(x23, X23, reg);\
  SAVE_REG(x24, X24, reg);\
  SAVE_REG(x25, X25, reg);\
  SAVE_REG(x26, X26, reg);\
  SAVE_REG(x27, X27, reg);\
  SAVE_REG(x28, X28, reg);\
  SAVE_SPEC_REG(fp, FP,  reg);\
  SAVE_SPEC_REG(sp, SP,  reg);\
  SAVE_SPEC_REG(lr, LR,  reg);\
  SAVE_SYS_REG(SPSR_EL1, SPSR,  reg);\
  //SAVE_SPEC_REG(.,  PC,  reg);


#define LOAD_CTX(reg)\
  LOAD_REG(x19, X19, reg);\
  LOAD_REG(x20, X20, reg);\
  LOAD_REG(x21, X21, reg);\
  LOAD_REG(x22, X22, reg);\
  LOAD_REG(x23, X23, reg);\
  LOAD_REG(x24, X24, reg);\
  LOAD_REG(x25, X25, reg);\
  LOAD_REG(x26, X26, reg);\
  LOAD_REG(x27, X27, reg);\
  LOAD_REG(x28, X28, reg);\
  LOAD_SPEC_REG(sp, SP,  reg);\
  LOAD_SPEC_REG(lr, LR,  reg);\
  LOAD_SYS_REG(SPSR_EL1, SPSR,  reg);\
  //LOAD_SPEC_REG(pc, PC,  reg)

#endif
