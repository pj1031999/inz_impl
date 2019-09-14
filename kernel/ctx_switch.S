#include "aarch64/ctx_switch.h"

#define ENTRY(sym) .globl sym; .align 2; .type sym, #function; sym:
#define END(sym)   .size sym, . - sym

//void ctx_save(ctx_t* ctx);
ENTRY(ctx_save)
// x0 - memory to save current context
	CTX_SAVE(x0);
	RET 
END(ctx_save)

//ctx_t* ctx_push(uint64_t x0, vaddr_t sp, vaddr_t pc, vaddr_t ret_addr);
ENTRY(ctx_push)
// x0 - value to pass to new thread - arg
// x1 - stack pointer - will contain pushed context
// x2 - addrdess of instruction where new thread will start processing
// x3 - value for link register

	BASE .req X1 // Alias
	ADD X4, X1, #0x1000
	SAVE_REG(X4, BASE, SF_SP);
	SAVE_REG(X2, BASE, SF_PC);
	SAVE_REG(X3, BASE, SF_LR);
	SAVE_SYS_REG(SPSR_EL1, BASE, SF_SPSR);
	SAVE_REG(X0, BASE, SF_REGS + 8 * 0);
	MOV X0, X1
	RET
END(ctx_push)

//void ctx_switch_to (ctx_t* ctx_old, ctx_t* ctx_new);
ENTRY(ctx_switch_to)
	NEW .req X1// Alias

	LOAD_SPEC_REG(x8, NEW, SF_PC);
	MSR elr_el1, x8
	LOAD_CTX(NEW)
	ERET
END(ctx_switch_to)


//void ctx_save_and_switch_to(ctx_t* ctx_old, ctx_t* ctx_new);
ENTRY(ctx_save_and_switch_to)
	OLD .req X0 // Alias
	NEW .req X1 // Alias

	CTX_SAVE(OLD);

	LOAD_SPEC_REG(x8, NEW, SF_PC);
	MSR elr_el1, x8
	LOAD_CTX(NEW)
	ERET
END(ctx_save_and_switch_to)

//void ctx_save_switch_restore(ctx_t* ctx_old, ctx_t* ctx_new);
ENTRY(ctx_save_switch_restore)
	OLD .req X0 // Alias
	NEW .req X1 // Alias

	CTX_SAVE(OLD);
	LOAD_SPEC_REG(x8, NEW, SF_PC);
	MSR elr_el1, x8
	LOAD_CTX(NEW)
	LDR LR, =1f //restore
	ADD SP, SP, #4096
	STR OLD, [sp, #-(8 * 2)]! //push - stack align 16B
	LOAD_REG(x0, NEW, SF_REGS + 8*0);
	ERET

1: //restore
	LDR OLD, [sp], #(8 * 2) //pop
	LOAD_CTX(OLD)
	RET

END(ctx_save_switch_restore)


//ctx_t* ctx_push(uint64_t x0, vaddr_t sp, vaddr_t pc, vaddr_t ret_addr);
ENTRY(ctx_us_push)
// x0 - value to pass to new thread - arg
// x1 - stack pointer - will contain pushed context
// x2 - addrdess of instruction where new thread will start processing
// x3 - value for link register

	BASE .req X1 // Alias
	SAVE_SPEC_REG(sp, BASE, SF_SP);
	SAVE_REG(X2, BASE, SF_PC);
	SAVE_REG(X3, BASE, SF_LR);
	SAVE_SYS_REG(SPSR_EL1, BASE, SF_SPSR);
	SAVE_REG(X0, BASE, SF_REGS + 8 * 0);
	MOV X0, X1
	RET

END(ctx_us_push)



//void ctx_save_switch_restore(ctx_t* ctx_old, ctx_t* ctx_new);
ENTRY(ctx_us_save_switch_restore)
	OLD .req X0 // Alias
	NEW .req X1 // Alias

	CTX_SAVE(OLD);
	LOAD_SPEC_REG(x8, NEW, SF_PC);
	MSR elr_el1, x8		//store us program address in elr_el1
	LOAD_CTX(NEW)

	MRS X8, spsr_el1
	AND X8, X8, ~0x1f // el0 - userspace
	MSR spsr_el1, x8	

	LDR LR, =1f // restore
	STR OLD, [sp, #-(8 * 2)]! //push - stack align 16B
	LOAD_REG(x0, NEW, SF_REGS + 8*0);

	Ldr X8, =0x81000
	AT s1e1r, x8
	MRs x8, PAR_EL1

	Ldr X7, =0x81000
	AT s1e0r, x7
	MRs x7, PAR_EL1

	
	ERET

1: //restore
	LDR OLD, [sp], #(8 * 2) //pop
	LOAD_CTX(OLD)
	RET

END(ctx_us_save_switch_restore)



//void ctx_load(ctx_t* ctx);
ENTRY(ctx_load)
// x0 - context to restore
	LOAD_CTX(x0)
	RET
END(ctx_load)
