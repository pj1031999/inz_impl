#include "aarch64/aarch64reg.h"
#include "aarch64/pte.h"
#include "dev/bcm2836reg.h"

#define CPU_STACK_SIZE 4096
#define UPPERADDR 0xffffFFFF00000000
#define PHYSADDR(x) ((x) - (UPPERADDR))

.section ".init"

.globl _start
_start:
	//start at el3 or el2
	MRS 	X4, ID_MMFR2_EL1
	MRS	x4, CurrentEl	
	CMP	x4, 0x8 	//0x8 = el2
	BEQ	set_link_register //if el2 then jump else:   

	// You must ensure this bit is set to 1 before the caches and MMU are
	// enabled, or any cache and TLB maintenance operations are performed.
    MOV     x27, LR                         //LR contains target function (e.g. kernel_entry)
    BL  enable_cache
    MOV     LR, x27

	
set_link_register:	
	// [lr] kernel entry point
	LDR 	lr, =kernel_entry

	// [x3] core number 
	MRS	X3, MPIDR_EL1 		// Multiprocessor Affinity Register
	ANDS	X3, X3, #3
	BEQ	_clear_bss
	
cpu_mailbox_wait:	
	// [CPU#1-CPU#3] Wait for entry point to appear in local mailbox #3
	LSL	X3, X3, #4
	LDR	X1, =_kernel

	// jump address
	LDR	X4, =BCM2836_ARM_LOCAL_BASE + BCM2836_LOCAL_MAILBOX3_CLRN(0)
2:	WFE
	LDR	W30, [X4, X3]	// lr == x30 there is no wlr |  read mailbox #3 for n-th CPU
	CMP	LR, #0
	BEQ	2b
	
	STR	W30, [X4, X3]	// clear the mailbox
	ADD	LR, LR, X1

	DSB	SY
	ISB

	// stack address
	LDR	X4, =BCM2836_ARM_LOCAL_BASE + BCM2836_LOCAL_MAILBOX1_CLRN(0)
3:	NOP
	LDR	W29, [X4, X3]
	CMP	W29, #0
	BEQ	3b

	STR	W29, [X4, X3]
	ADD	X29, X29, X1

	LSR	X3, X3, #4
	CMP	X3, #0
	BNE	setup_translation_table

_clear_bss:
	// [CPU#0] Clear BSS section.

	LDR	X4, =_start
	MOV	X5, #240
	STR 	X4,[X5]
	MOV	X5, #232
	STR 	X4,[X5]
	MOV	X5, #224
	STR 	X4,[X5]
	DSB	SY
	ISB
	
    // TODO save LR in x27 at the begin
    MOV     x27, LR                         //LR contains target function (e.g. kernel_entry)
    BL  clear_bss
    MOV     LR, x27

	/* Set stack address core0 */
	LDR	X29, =_el1_stack

setup_translation_table:
	/* Invalidate all TLB */
    MOV x27, LR
    BL invalidate_tlb
    MOV LR, x27

	ADR	X0, PHYSADDR(_level1_pagetable)	// ttb0_base must be a 4KB-aligned address.
	MSR	TTBR1_EL1, X0
	MSR	TTBR0_EL1, X0

    MOV x27, LR
    BL setup_tmp_stack
    MOV LR, x27

	MOV	x27, LR				//LR contains target function (e.g. kernel_entry)
	BL	page_table_fill_inner_nodes
	BL	page_table_fill_leaves
	MOV	LR, x27

mmu_setup:
	DSB	sy
	ISB

	LDR 	X0, =(	\
	MAIR_ATTR(MAIR_DEVICE_nGnRnE, 	ATTR_DEVICE_MEM) \
        | MAIR_ATTR(MAIR_NORMAL_NC, 	ATTR_NORMAL_MEM_NC) 	\
	| MAIR_ATTR(MAIR_NORMAL_WB, 	ATTR_NORMAL_MEM_WB) 	\
	| MAIR_ATTR(MAIR_NORMAL_WT, 	ATTR_NORMAL_MEM_WT)	\
	)
	MSR MAIR_EL1, X0

	
	/* Invalidate all TLB */
    MOV x27, LR
    BL invalidate_tlb
    MOV LR, x27


	LDR	x0, =( \
	TCR_TxSZ(32) \
	| TCR_TGx_(4K) \
	| TCR_IPS_32BIT \
	| (1<<39)/*TCR_HA*/ \
	| (1<<40)/*TCR_HD*/ \
	)
	MRS	x1, id_aa64mmfr0_el1
	BFI	x0, x1, #32, #3
	MSR	tcr_el1, x0

	LDR	x1, =( \
	SCTLR_M	\
	| SCTLR_I \
	| SCTLR_C \
	)
	ORR	x0, x0, x1

mmu_enable:
    MOV x27, LR
    BL enable_mmu
    MOV LR, x27
	
	MRS	x4, CurrentEl	
	CMP	x4, 0x8 	//0x8 = el2
	BEQ	change_el
	
el3_only:	

	MRS	X0, SCR_EL3
	ORR	X0, X0, #(1<<10)	// RW EL2 Execution state is AArch64.
	ORR	X0, X0, #(1<<0)		// NS EL1 is Non-secure world.
	MSR	SCR_EL3, x0

	MOV	X0, #0b01001		// DAIF=0000
	MSR	SPSR_EL3, X0 		// M[4:0]=01001 EL2h must match SCR_EL3.RW

	ADR	X0, change_el     	// el2_entry points to the first instruction of
	MSR	ELR_EL3, X0       	// EL2 code.

	ERET	//jump to el2_entry

change_el:
	// Initialize SCTLR_EL2 and HCR_EL2 to save values before entering EL2
	MSR	HCR_EL2, XZR   
	MRS	X0, HCR_EL2
	ORR	X0, X0, #(1<<31)	// RW=1  EL1  Execution state is AArch64.
	//ORR	X0, X0, #(1 << 1)   	// SWIO hardwired on Pi3
	MSR	HCR_EL2, X0
		
el2_entry:
	/* Enable access to the physical timers at EL1 */
	MRS	x2, cnthctl_el2
	ORR	x2, x2, #(CNTHCTL_EL1PCTEN | CNTHCTL_EL1PCEN)
	MSR	cnthctl_el2, x2
	
	// Determine the  EL1 Execution state
	MOV	X1, #0b0101	 	// DAIF=0000
	MSR	SPSR_EL2, X1      	// M[4:0]=00101 EL1h must match HCR_EL2.RW.
	
	ADR	X1, el1_entry     	// el1_entry points to the first instruction of
	MSR	ELR_EL2, X1       	// EL1 code.
	ERET	
	
el1_entry:
	/* Load the exception vectors */
	LDR	x2, =_exc_vector
	MSR	vbar_el1, x2

	/* Set stack address */
	MOV	SP, X29
	MSR	DAIFClr, #3

// Enter kernel_entry with empty stack.
enter_kernel:

	MOV	x4, lr
	LDR	lr, =kernel_exit  
  
turn_off:
// turn off user space adress translation
	LDR	x3, =turn_off_user_space
	LDR	x1, =_kernel

	ADD	x1, x1, x3
	BR	x1

turn_off_user_space:
	MSR	TTBR0_EL1, XZR

	DSB	SY
	ISB
	BR	x4

//vim: ft=armv5 ts=8 sw=8 noet	
