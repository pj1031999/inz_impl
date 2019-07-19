#include "aarch64/ctx.h"

#define ENTRY(sym) .globl sym; .align 2; .type sym, #function; sym:
#define END(sym)   .size sym, . - sym

	.section ".text"

ENTRY(ctx_save)
	SAVE_CTX(x0)
	RET 
END(ctx_save)

	
ENTRY(ctx_push)
	SAVE_SPEC_REG(x1, SP,  x1);
	SAVE_SPEC_REG(x2, PC,  x1);
	SAVE_SPEC_REG(x3, LR,  x1);
	SAVE_SYS_REG(SPSR_EL1, SPSR, x1);

	MOV X0, X1
	RET
END(ctx_push)


ENTRY(ctx_switch_to)
	LOAD_CTX(x1)
	LOAD_SPEC_REG(lr, LR, x1);
	LOAD_SPEC_REG(x8, PC, x1);

	MSR elr_el1, x8
	ERET
END(ctx_switch_to)


ENTRY(ctx_load)
	LOAD_CTX(x0)
	RET
END(ctx_load)
