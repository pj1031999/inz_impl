#ifndef __ARMV8_CTX_H__
#define __ARMV8_CTX_H__

struct regs {
  uint64_t r_reg[31];
  uint64_t r_sp;
  uint64_t r_pc;  /* aka elr */
  uint64_t r_spsr;
  //uint64_t r_tpidr;
};


typedef struct ctx_t{  /* switch frame */
  struct regs regs __aligned(16);
  uint64_t sf_esr;        // 32-bit register
  uint64_t sf_far;        // 64-bit register
#define sf_regs     regs.r_reg
#define sf_lr       regs.r_reg[30]
#define sf_pc       regs.r_pc
#define sf_sp       regs.r_sp
#define sf_spsr     regs.r_spsr
} ctx_t; 

void ctx_save(ctx_t* ctx);
ctx_t* ctx_push(uint64_t x0, vaddr_t sp, vaddr_t pc, vaddr_t ret_addr);
void ctx_switch_to(ctx_t* ctx_old, ctx_t* ctx_new);
void ctx_save_and_switch_to (ctx_t* ctx_old, ctx_t* ctx_new);
void ctx_save_switch_restore(ctx_t* ctx_old, ctx_t* ctx_new);
void ctx_load(ctx_t* ctx);

#endif
