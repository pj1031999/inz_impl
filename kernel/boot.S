#include "aarch64/aarch64reg.h"
#include "dev/bcm2836reg.h"

#define CPU_STACK_SIZE 4096

//#define PAGE64
	
#ifdef PAGE64
#define UPPERADDR 0xffff000000000000
#else
#define UPPERADDR 0xffffFF0000000000
#endif

	
#define PHYSADDR(x) ((x) - (UPPERADDR))

	
.section ".init"
 
.globl _start
.local no_cpu0_init
	
_start:
	//start at el3 or el2
	MRS	x4, CurrentEl	
	CMP	x4, 0x8 	//0x8 = el2
	BEQ	set_link_register

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
	ANDS 	X3, X3, #3		
	BEQ 	clear_bss
	
cpu_mailbox_wait:	
	// [CPU#1-CPU#3] Wait for entry point to appear in local mailbox #3
	ldr	x4, =BCM2836_ARM_LOCAL_BASE + BCM2836_LOCAL_MAILBOX3_CLRN(0)

	lsl	x3, x3, #4
2:	wfe
	ldr	w30, [x4, x3]	// lr == x30 there id no wlr |  read mailbox #3 for n-th CPU
	cmp	lr, #0
	beq	2b
	
	str	w30, [x4, x3]	// clear the mailbox
	lsr	x3, x3, #4

	
	cmp	x3, #0
	bne	setup_translation_table //no_cpu0_init


clear_bss:
	// [CPU#0] Clear BSS section.

	ldr	x4, =PHYSADDR(_bss_start)
	ldr	x5, =PHYSADDR(_bss_end)
1:	stp 	xzr, xzr, [x4], #16
	cmp	x4, x5
	blo	1b

setup_translation_table:
	/* Invalidate all TLB */
	TLBI	ALLE1
	tlbi	vmalle1is
	dsb	ish
	isb

#ifdef PAGE64	
	ADR	X0, PHYSADDR(_level0_pagetable)	// must be a 4KB-aligned address.
	MSR	TTBR1_EL1, X0
	MSR	TTBR0_EL1, X0
	DSB	SY
	ISB
	
	LDR	X2, =0x00000721
	LDR	X5, =0x20000000	// Increase 512MB address each time.
	MOV	X4, #8192	// Set level2 block entries.
loop:
	STR	X2, [X0], #8 	// Each entry occupies 2 words.
	ADD	X2, X2, X5
	SUBS	X4, X4, #1
	BNE	loop
#else
// 	ADR    X0, PHYSADDR(_level1_pagetable)	// ttb0_base must be a 4KB-aligned address.
// 	MSR    TTBR1_EL1, X0
// 	MSR    TTBR0_EL1, X0
// 	
// 	LDR    X2, = PHYSADDR(_level2_pagetable) // Must  be a 4KB align address.
// 	ORR    X2, X2, 0x3
// 	LDR    X2, =0x00000721
// 	STR    X2, [X0], #8
// 	
// 	
// 	//LDR    X2, = PHYSADDR(_level3_pagetable) // Must  be a 4KB align address.
// 	//ORR    X2, X2, 0x3
// 	LDR    X2, =0x40000721        	// Executable Inner and    Outer Shareable.
// 	//ADD    X2, X2, #4096
// 	STR    X2, [X0], #8       	//   R/W at all ELs secure memory
// 					// AttrIdx=000 Device-nGnRnE.
// 	
// 	// The third entry is 1GB block from 0x80000000 to 0xBFFFFFFF.
// 	LDR    X2, =0x80000721
// 	//ADD    X2, X2, #4096
// 	STR    X2, [X0], #8
// 
// 	// The fourth entry is 1GB block from 0xC0000000 to 0xFFFFFFFF.
// 	LDR    X2, =0xC0000721
// 	//ADD    X2, X2, #4096
// 	STR    X2, [X0], #8
// 
// 	// Set level 2 translation table.
// 	LDR    X0, =PHYSADDR(_level2_pagetable) 
// 	LDR    X2, =0x00000721
// 	MOV    X4, #4096      
// 	LDR    X5, =0x00200000
// loop:
// 	STR    X2, [X0], #8   
// 	ADD    X2, X2, X5
// 	SUBS   X4, X4, #1
// 	BNE	loop
	//
	// Initialize translation table control registers
	LDR    X1, =0x3520	// 4GB space 4KB granularity
	// Inner-shareable.
	MSR    TCR_EL3, X1	// Normal Inner and    Outer Cacheable.
	LDR    X1, =0xFF440400	// ATTR0 Device-nGnRnE ATTR1 Device.
	MSR    MAIR_EL3, X1     // ATTR2 Normal Non-Cacheable.
				//   ATTR3 Normal Cacheable.
	ADR    X0, PHYSADDR(_level0_pagetable)	// ttb0_base must be a 4KB-aligned address.
	//MSR    TTBR0_EL3, X0
	MSR    TTBR1_EL1, X0
	MSR    TTBR0_EL1, X0
	
	// Set up translation table entries in memory with looped store
	// instructions.
	// Set the level 1 translation table.
	// The first entry points to level2_pagetable.

	LDR    X1, = PHYSADDR(_level2_pagetable) // Must  be a 4KB align address.
	LDR    X2, =0xFFFFF000
	AND    X2, X1, X2             // NSTable=0 APTable=0 XNTable=0 PXNTable=0.
	ORR    X2, X2, 0x3
	STR    X2, [X0], #8

	// The second entry is 1GB block from 0x40000000 to 0x7FFFFFFF.
	LDR    X2, =0x40000741        	// Executable Inner and    Outer Shareable.
	STR    X2, [X0], #8       	//   R/W at all ELs secure memory
	// AttrIdx=000 Device-nGnRnE.

	// The third entry is 1GB block from 0x80000000 to 0xBFFFFFFF.
	LDR    X2, =0x80000741
	STR    X2, [X0], #8

	// The fourth entry is 1GB block from 0xC0000000 to 0xFFFFFFFF.
	LDR    X2, =0xC0000741
	STR    X2, [X0], #8

	// Set level 2 translation table.
	LDR    X0, =PHYSADDR(_level2_pagetable)  	// Base address of level2_pagetable.
	LDR    X2, =0x00000721        	// Executable Inner and Outer Shareable.
					// R/W at all ELs secure memory.
					// AttrIdx=011  Normal Cacheable.

	MOV    X4, #512			// Set 512 level2 block entries.
	LDR    X5, =0x00200000        	// Increase 2MB address each time.
loop:
	STR    X2, [X0], #8 		// Each entry occupies 2 words.
	ADD    X2, X2, X5
	SUBS   X4, X4, #1
	BNE	loop	

	
#endif	

mmu_setup:
	dsb	sy
	isb
	
	/* Invalidate all TLB */
	dsb	ishst
	tlbi	vmalle1is
	dsb	ish
	isb

	ldr	x0, =( \
	MAIR_ATTR(MAIR_DEVICE_nGnRnE, 3) \
	| MAIR_ATTR(MAIR_NORMAL_WT, 2) \
	| MAIR_ATTR(MAIR_NORMAL_NC, 1) \
	| MAIR_ATTR(MAIR_NORMAL_WB, 0))
	//ldr	x0, =( \
	//MAIR_ATTR(MAIR_DEVICE_nGnRnE, 3) \
	//| MAIR_ATTR(MAIR_DEVICE_nGnRnE, 2) \
	//| MAIR_ATTR(MAIR_DEVICE_nGnRnE, 1) \
	//| MAIR_ATTR(MAIR_DEVICE_nGnRnE, 0))

	msr	mair_el1, x0


	/* TCR_EL1:IPS[34:32] = AA64MMFR0:PARange[3:0] */
#ifdef PAGE64
	ldr	x0, =(\
	TCR_TxSZ(16)\
	| TCR_CACHE_ATTRS\
	| TCR_SMP_ATTRS\
	| TCR_TGx_(64K))
	mrs	x1, id_aa64mmfr0_el1
	bfi	x0, x1, #32, #3
	msr	tcr_el1, x0

#else
	ldr	x0, =(\
	TCR_TxSZ(24)\
	| TCR_CACHE_ATTRS\
	| TCR_SMP_ATTRS\
	| TCR_TGx_(4K))
	mrs	x1, id_aa64mmfr0_el1
	bfi	x0, x1, #32, #3
	msr	tcr_el1, x0
#
endif
	/*
	 * configure SCTLR
	 */
	mrs	x0, sctlr_el1
	ldr	x1, =(	\
	SCTLR_IESB 	\
	| SCTLR_WXN 	\
	| SCTLR_UMA 	\
	| SCTLR_ITD 	\
	| SCTLR_THEE 	\
	| SCTLR_CP15BEN \
	| SCTLR_SA0 	\
	| SCTLR_SA 	\
	| SCTLR_A 	\
	| 0)
	bic	x0, x0, x1

	ldr	x1, =(	\
	SCTLR_LSMAOE 	\
	| SCTLR_nTLSMD 	\
	| SCTLR_UCI 	\
	| SCTLR_SPAN 	\
	| SCTLR_UCT 	\
	| SCTLR_nTWE 	\
	| SCTLR_nTWI 	\
	| SCTLR_DZE 	\
	| SCTLR_I 	\
	| SCTLR_SED 	\
	| 0 /*SCTLR_C*/ 	\
	| SCTLR_M 	\
	| SCTLR_SA0 	\
	| SCTLR_SA 	\
	| SCTLR_A 	\
	| 0)
	orr x0, x0, x1
	
	ldr	x1, =(SCTLR_EE | SCTLR_EOE)
	//orr	x0, x0, x1 /* set: BigEndian */
	bic x0, x0, x1  /* clear: LittleEndian */

mmu_enable:
	msr	sctlr_el1, x0	/* enabling MMU! */
	dsb	sy
	isb
	MRS	x4, CurrentEl	

	
no_cpu0_init:
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

	ERET 	//jump to el2_entry

change_el:
	//Change Exception Level
	// Initialize SCTLR_EL2 and HCR_EL2 to save values before entering EL2
	MSR	HCR_EL2, XZR   
	MRS	X0, HCR_EL2
	ORR	X0, X0, #(1<<31)	// RW=1  EL1  Execution state is AArch64.
	ORR 	X0, X0, #(1 << 1)   	// SWIO hardwired on Pi3
	MSR	HCR_EL2, X0
		
el2_entry:
	MRS	x4, CurrentEl

	// Disable trapping of   accessing in EL3 and EL2.
	//MSR       CPTR_EL3, XZR
	//MSR       CPTR_EL3, XZR

	// Disable access tra pping in EL1 and EL0.
	//MOV    X1, #(0x3 << 20)       // FPEN disables trapping to EL1.
	//MSR    CPACR_EL1, X1
	//ISB 

	/* Enable access to the physical timers at EL1 */
	MRS	x2, cnthctl_el2
	ORR	x2, x2, #(CNTHCTL_EL1PCTEN | CNTHCTL_EL1PCEN)
	MSR	cnthctl_el2, x2
	
	// Determine the  EL1 Execution stat
	MOV	X1, #0x3c5//#0b00101 		// DAIF=0000
	MSR	SPSR_EL2, X1      	// M[4:0]=00101 EL1h must match HCR_EL2.RW.
	
	ADR	X1, el1_entry     	// el1_entry points to the first instruction of
	MSR	ELR_EL2, X1       	// EL1 code.
	ERET
	
el1_entry:
	MRS	x1, CurrentEl
	
	ldr	x4, =0xffffff007e00b224
	AT	S1E1R, X4
	ISB
	MRS	X6, PAR_EL1
	ISB
	ldr	x4, =0x400000dc
	AT	S1E1R, X4
	ISB
	MRS	X6, PAR_EL1
	ISB
	MRS	X0, SCTLR_EL1

	/* Load the exception vectors */
	//LDR 	x2, =exception_vectors
	LDR 	x2, =_exc_vector
	MSR 	vbar_el1, x2
	
	ADR    X1, PHYSADDR(_el1_stack)
	//ADD    X1, X1, #4
	MRS    X2, MPIDR_EL1
	AND    X2, X2, #0xFF               // X2 == CPU number.
	MOV    X3, #CPU_STACK_SIZE
	MUL    X3, X2, X3                 // Create separated stack spaces
	SUB    X1, X1, X3                 // for each processor

	//ADR    X1, PHYSADDR(_el1_stack)
	MOV    SP, X1

	msr    daifclr, #3
	
 	// Enter kernel_entry with empty stack.
enter_kernel:
	mov	x4, lr
	ldr	lr, =kernel_exit
	br	x4

	
//vim: ft=armv5 ts=8 sw=8 noet	
