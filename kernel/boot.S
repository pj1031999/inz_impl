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
    MOV x27, LR
    BL setup_tmp_stack
    BL setup_tlb
    MOV LR, x27

mmu_setup:
    MOV x27, LR
    BL setup_mmu
    MOV LR, x27

	MRS	x4, CurrentEl	
	CMP	x4, 0x8 	//0x8 = el2
	BEQ	_change_el
	
_el3_only:	
    MOV x27, LR
    BL el3_only
    MOV LR, x27
	
	ADR	X0, _change_el     	// el2_entry points to the first instruction of
	MSR	ELR_EL3, X0       	// EL2 code.
   
	ERET	//jump to el2_entry

_change_el:
	// Initialize SCTLR_EL2 and HCR_EL2 to save values before entering EL2
    MOV x27, LR
    BL change_el
    MOV LR, x27
		
_el2_entry:
    MOV x27, LR
    BL el2_entry
    MOV LR, x27
	
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
