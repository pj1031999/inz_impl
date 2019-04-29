#include "aarch64/aarch64reg.h"
#include "dev/bcm2836reg.h"

#define CPU_STACK_SIZE 4096
#define UPPERADDR 0xffffFFFF00000000
#define PHYSADDR(x) ((x) - (UPPERADDR))
	
.section ".init"
 
.globl _start
_start:
	//start at el3 or el2
	MRS	x4, CurrentEl	
	CMP	x4, 0x8 	//0x8 = el2
	BEQ	set_link_register //if el2 then jump else:   

	// You must ensure this bit is set to 1 before the caches and MMU are
	// enabled, or any cache and TLB maintenance operations are performed.
enable_cache:
	MRS	X0, S3_1_C15_C2_1
	ORR	X0, X0, #(0x1 << 6)	// The  SMP bit.
	MSR	S3_1_C15_C2_1, X0 
	DSB	SY
	ISB

	
set_link_register:	
	// [lr] kernel entry point
	LDR 	lr, =kernel_entry

	// [x3] core number 
	MRS	X3, MPIDR_EL1 		// Multiprocessor Affinity Register
	ANDS	X3, X3, #3
	BEQ	clear_bss
	
cpu_mailbox_wait:	
	// [CPU#1-CPU#3] Wait for entry point to appear in local mailbox #3
	LDR	x4, =BCM2836_ARM_LOCAL_BASE + BCM2836_LOCAL_MAILBOX3_CLRN(0)

	LSL	x3, x3, #4
2:	WFE
	LDR	w30, [x4, x3]	// lr == x30 there id no wlr |  read mailbox #3 for n-th CPU
	CMP	lr, #0
	BEQ	2b
	
	STR	w30, [x4, x3]	// clear the mailbox
	LSR	x3, x3, #4

	
	CMP	x3, #0
	BNE	setup_translation_table


clear_bss:
	// [CPU#0] Clear BSS section.

	LDR	x4, =PHYSADDR(_bss_start)
	LDR	x5, =PHYSADDR(_bss_end)
1:	STP 	xzr, xzr, [x4], #16
	CMP	x4, x5
	BLO	1b

setup_translation_table:
	/* Invalidate all TLB */
	TLBI	ALLE1
	TLBI	vmalle1is
	DSB	ish
	ISB

	ADR	X0, PHYSADDR(_level1_pagetable)	// ttb0_base must be a 4KB-aligned address.
	MSR	TTBR1_EL1, X0
	MSR	TTBR0_EL1, X0

	// The first entry is 1GB block from 0x00000000 to 0x3FFFFFFF. Map 1:1
	LDR	X2, =0x00000721
	STR	X2, [X0], #8

	// The second entry is 1GB block from 0x40000000 to 0x7FFFFFFF. Map 1:1
	LDR	X2, =0x40000721        	
	STR	X2, [X0], #8  
	
	// The third entry is 1GB block from 0x80000000 to 0xBFFFFFFF. Map 1:1
	LDR	X2, =0x80000721
	STR	X2, [X0], #8

	// The fourth entry is 1GB block from 0xC0000000 to 0xFFFFFFFF. Mapped into first GB because Video Core memory mapping
	LDR	X2, =0x00000721
	STR	X2, [X0], #8

mmu_setup:
	DSB	sy
	ISB
	
	/* Invalidate all TLB */
	DSB	ishst
	TLBI	vmalle1is
	DSB	ish
	ISB	

	LDR	x0, =( \
		TCR_TxSZ(32) \
		| TCR_TGx_(4K) \
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
	MSR	sctlr_el1, x0	/* enabling MMU! */
	DSB	sy
	ISB	
	
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
	ORR	X0, X0, #(1 << 1)   	// SWIO hardwired on Pi3
	MSR	HCR_EL2, X0
		
el2_entry:
	/* Enable access to the physical timers at EL1 */
	MRS	x2, cnthctl_el2
	ORR	x2, x2, #(CNTHCTL_EL1PCTEN | CNTHCTL_EL1PCEN)
	MSR	cnthctl_el2, x2
	
	// Determine the  EL1 Execution state
	MOV	X1, #0x3c5//#0b00101 	// DAIF=0000
	MSR	SPSR_EL2, X1      	// M[4:0]=00101 EL1h must match HCR_EL2.RW.
	
	ADR	X1, el1_entry     	// el1_entry points to the first instruction of
	MSR	ELR_EL2, X1       	// EL1 code.
	ERET	
	
el1_entry:
	/* Load the exception vectors */
	LDR	x2, =_exc_vector
	MSR	vbar_el1, x2

	/* Set stack address */
	//ADR	X1, PHYSADDR(_el1_stack)
	LDR	X1, =_el1_stack
	MRS	X2, MPIDR_EL1
	AND	X2, X2, #0xFF               // X2 == CPU number.
	MOV	X3, #CPU_STACK_SIZE
	MUL	X3, X2, X3                 // Create separated stack spaces
	SUB	X1, X1, X3                 // for each processor

	MOV	SP, X1
	msr	daifclr, #3
	
 	// Enter kernel_entry with empty stack.
enter_kernel:
	MOV	x4, lr
	LDR	lr, =kernel_exit
	//MSR	TTBR0_EL1, XZR // turn off user space adress translation
	BR	x4

	
//vim: ft=armv5 ts=8 sw=8 noet	
