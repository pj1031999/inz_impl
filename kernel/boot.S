#ifdef AARCH64
// AArch64 mode 
// To keep this in the first portion of the binary.

#include "aarch64/aarch64reg.h"
#include "dev/bcm2836reg.h"
	
#define CPU_STACK_SIZE 4096
#define UPPERADDR 0xFFFFFF0000000000
#define PHYSADDR(x) ((x) - UPPERADDR)

	
.section ".init"
 
.globl _start
.local no_cpu0_init
 
	
_start:
	//start at el3
	MRS	x4, CurrentEl	


	// You must ensure this bit is set to 1 before the caches and MMU are
	// enabled, or any cache and TLB maintenance operations are performed.
enable_cache:
	MRS	X0, S3_1_C15_C2_1
	ORR	X0, X0, #(0x1 << 6)	// The  SMP bit.
	MSR	S3_1_C15_C2_1, X0 
	DSB	SY
	ISB


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
	ldr	lr, [x4, x3]	// read mailbox #3 for n-th CPU
	cmp	lr, #0
	beq	2b
	
	str	lr, [x4, x3]	// clear the mailbox
	lsr	x3, x3, #4

	
	cmp	x3, #0
	bne	no_cpu0_init


clear_bss:
	// [CPU#0] Clear BSS section.

	ldr	x4, =PHYSADDR(_bss_start)
	ldr	x5, =PHYSADDR(_bss_end)
1:	stp 	xzr, xzr, [x4], #16
	cmp	x4, x5
	blo	1b

#if 1
clear_cache:	
	// Disable L1 Caches
	MRS    	X0, SCTLR_EL3		// Read SCTLR_EL3.
	BIC    	X0,   X0, #(0x1 << 2)   // Disable D Cache.
	MSR	SCTLR_EL3, X0         	// Write SCTLR_EL3.

	// Invalidate Data cache to make the code general purpose.
	// Calculate the cache size first and loop through each set +
	//   way  .
	MOV    X0, #0x0            	//  X0 = Cache level
	MSR    CSSELR_EL1, x0          	// 0x0 for L1 Dcache  0x2    for L2 Dcache.

	MRS    X4, CCSIDR_EL1       	// Read Cache Size ID.
	AND    X1, X4, #0x7
	ADD    X1, X1, #0x4         	// X1 = Cache Line Size.
	LDR    X3, =0x7FFF
	AND    X2, X3, X4, LSR   #13  	// X2 = Cache Set Number – 1.
	LDR    X3, =0x3FF
	AND    X3, X3, X4, LSR   #3   	// X3 = Cache Associativity Number – 1.
	CLZ    W4,   W3               	// X4 = way position in the CISW  instruction.
	
	MOV    X5, #0               	//  X5 = way counter way_loop.
way_loop:
	MOV    X6, #0               	//  X6 = set counter set_loop.
set_loop:
	LSL    X7, X5,  X4 
	ORR    X7, X0, X7           // Set way.
	LSL    X8, X6, X1
	ORR    X7, X7,  X8           // Set set.
	DC     cisw, X7             // Clean and Invalidate cache line.
	ADD    X6, X6, #1           // Increment set counter.
	CMP    X6, X2               // Last set reached yet?
	BLE    set_loop             // If not, iterate set_loop,
	ADD    X5, X5, #1           // else, next way.
	CMP    X5, X3               // Last way reached yet?
	BLE    way_loop             // I f not, iterate way_loop.
#endif	


setup_translation_table:
	/* Invalidate all TLB */
	TLBI	ALLE1
	tlbi	vmalle1is
	dsb	ish
	isb

#if 0	
	ADR	X0, PHYSADDR(_level2_pagetable)	// must be a 4KB-aligned address.
	MSR	TTBR1_EL1, X0
	MSR	TTBR0_EL1, X0
	//MSR	TTBR0_EL2, X0
	//MSR	TTBR0_EL3, X0
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
	
no_cpu0_init:
change_el:
	//Change Exception Level
	MRS	x4, CurrentEl
	CMP	x4, #8
	BEQ	el2_entry

	LDR    X1, =0x30C50838
	MSR    SCTLR_EL2, X1 
	//MSR    SCTLR_EL1, X1
	
	// Initialize the    SCTLR_EL1 register before entering EL1.
	MRS	X0, SCTLR_EL1	
	ORR	X0, X0, #(0x1 << 2)	// The   C bit (data cache).
	ORR	X0, X0, #(0x1 << 12)	// The   I bit (instruction cache).
	//ORR	X0, X0, #0x1		// The   M bit (MMU).
	MSR	SCTLR_EL1, X0
	DSB	SY
	ISB
	
	// Initialize SCTLR_EL2 and HCR_EL2 to save values before entering EL2
	MSR	HCR_EL2, XZR   
	MRS	X0, HCR_EL2
	ORR	X0, X0, #(1<<31)	// RW=1  EL1  Execution state is AArch64.
	//ORR 	X0, X0, #(1 << 1)   	// SWIO hardwired on Pi3
	MSR	HCR_EL2, X0

	
	MRS	X0, SCR_EL3
	ORR	X0, X0, #(1<<10)	// RW EL2 Execution state is AArch64.
	ORR	X0, X0, #(1<<0)		// NS EL1 is Non-secure world.
	MSR	SCR_EL3, x0

	MOV	X0, #0b01001		// DAIF=0000
	MSR	SPSR_EL3, X0 		// M[4:0]=01001 EL2h must match SCR_EL3.RW

	// Determine EL2 entry.
	ADR	X0, el2_entry     	// el2_entry points to the first instruction of
	MSR	ELR_EL3, X0       	// EL2 code.


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
	msr	mair_el1, x0


	/* TCR_EL1:IPS[34:32] = AA64MMFR0:PARange[3:0] */
	ldr	x0, =(\
	TCR_TxSZ(24)\
	| TCR_CACHE_ATTRS\
	| TCR_SMP_ATTRS\
	| TCR_TGx_(4K))
	mrs	x1, id_aa64mmfr0_el1
	bfi	x0, x1, #32, #3
	msr	tcr_el1, x0

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
	| SCTLR_C 	\
	| SCTLR_M 	\
	| SCTLR_SA0 	\
	| SCTLR_SA 	\
	| SCTLR_A 	\
	| 0)
	orr x0, x0, x1
	
	ldr	x1, =(SCTLR_EE | SCTLR_EOE)
	//orr	x0, x0, x1 /* set: BigEndian */
	bic x0, x0, x1  /* clear: LittleEndian */
	

	ERET 	//jump to el2_entry
	
el2_entry:
	MRS	x4, CurrentEl

	// Determine the  EL1 Execution stat
	MOV	X1, #0x3c5//#0b00101 		// DAIF=0000
	MSR	SPSR_EL2, X1      	// M[4:0]=00101 EL1h must match HCR_EL2.RW.
	
	ADR	X1, el1_entry     	// el1_entry points to the first instruction of
	MSR	ELR_EL2, X1       	// EL1 code.
	ERET
	
el1_entry:
	MRS	x1, CurrentEl


mmu_enable:
	msr	sctlr_el1, x0	/* enabling MMU! */
	dsb	sy
	isb
	MRS	x4, CurrentEl	

	
// Setup domains - Dom0 is usable, rest is disabled.
// 	mov	x4, #1
// 	mcr	p15, 0, r4, c3, c0, 0	@ Domain Access Control Register
// 	@ Configure exception base vector.
// 	ldr	r4, =_exc_vector
// 	mcr	p15, 0, r4, c12, c0, 0	@ Secure or Non-secure VBAR
// 	mcr	p15, 0, r4, c12, c0, 1	@ Monitor Vector Base Address Register


	ldr	x4, =0xffff0000000810c4
	AT	S1E1R, X4
	ISB
	MRS	X6, PAR_EL1
	ISB

	MRS	X0, SCTLR_EL1

	
	ADR    X1, PHYSADDR(_el1_stack)
	ADD    X1, X1, #4
	MRS    X2, MPIDR_EL1
	AND    X2, X2, #0xFF               // X2 == CPU number.
	MOV    X3, #CPU_STACK_SIZE
	MUL    X3, X2, X3                 // Create separated stack spaces
	SUB    X1, X1, X3                 // for each processor

	ADR    X1, PHYSADDR(_el1_stack)
	MOV    SP, X1
	
 	// Enter kernel_entry with empty stack.
enter_kernel:
	mov	x4, lr
	ldr	lr, =kernel_exit
	br	x4

///////////////////////////////////////
// AArch32
///////////////////////////////////////
#else // AARCH32

@ boot.S - assembly startup code

#include "arm/cpureg.h"
#include "dev/bcm2836reg.h"

#define __BIT(__n) (1 << (__n))
#define PHYSADDR(x) ((x) - 0x80000000)
 
.section ".init"
 
.globl _start
.local no_cpu0_init
 
@ Entry point for the kernel.
@ r15 -> should begin execution at 0x8000.
@ r0 -> 0x00000000
@ r1 -> machid
@ r2 -> atags
@ preserve these registers as argument for the kernel

_start:
	@ Enable data & instruction caches.
	mrc	p15, 0, r4, c1, c0, 0	@ System Control Register
	orr	r4, r4, #CPU_CONTROL_DC_ENABLE
	orr	r4, r4, #CPU_CONTROL_IC_ENABLE
	mcr	p15, 0, r4, c1, c0, 0

	@ You must ensure this bit is set to 1 before the caches and MMU are
	@ enabled, or any cache and TLB maintenance operations are performed.
	mrc	p15, 0, r4, c1, c0, 1	@ Read Auxiliary Control Register
	orr	r4, r4, #CORTEXA9_AUXCTL_SMP
	mcr	p15, 0, r4, c1, c0, 1

	@ [lr] kernel entry point
	ldr	lr, =kernel_entry

	@ [r3] core number 
	mrc	p15, 0, r3, c0, c0, 5	@ Multiprocessor Affinity Register
	ands	r3, r3, #3		@ CORTEXA9_MPIDR_CPUID
	beq	1f

	@ [CPU#1-CPU#3] Wait for entry point to appear in local mailbox #3
	ldr	r4, = BCM2836_ARM_LOCAL_BASE + BCM2836_LOCAL_MAILBOX3_CLRN(0)

2:	wfe
	ldr	lr, [r4, r3, lsl #4]	@ read mailbox #3 for n-th CPU
	cmp	lr, #0
	beq	2b

	str	lr, [r4, r3, lsl #4]	@ clear the mailbox
	
	@ Set up initial page table pointer.
1:	ldr	r4, =PHYSADDR(_kernel_pde)
	mcr	p15, 0, r4, c2, c0, 0	@ Translation Table Base Register 0

	@ [CPU#0] Fill in initial page table.
	cmp	r3, #0
	bne	no_cpu0_init

	@ 1:1 mapping for first 2GiB
	mov	r5, #0x0140E		@ TEX=1, APX=0, AP=1, C=1, B=1, section
	mov	r6, #2048
1:	str	r5, [r4], #4
	add	r5, r5, #1024*1024	@ map next 1MiB section
	subs	r6, r6, #1
	bhi	1b

	@ Remap first 2GiB to 0x8000_0000
	movt	r5, 0
	mov	r6, #2048
1:	str	r5, [r4], #4
	add	r5, r5, #1024*1024	@ map next 1MiB section
	subs	r6, r6, #1
	bhi	1b

	@ [CPU#0] Clear BSS section.
	ldr	r4, =PHYSADDR(_bss_start)
	ldr	r9, =PHYSADDR(_bss_end)
	mov	r5, #0
	mov	r6, #0
	mov	r7, #0
	mov	r8, #0
1:	stmia	r4!, {r5-r8}		@ Store 16 bytes at once.
	cmp	r4, r9
	blo	1b

no_cpu0_init:
	@ Setup domains - Dom0 is usable, rest is disabled.
	mov	r4, #1
	mcr	p15, 0, r4, c3, c0, 0	@ Domain Access Control Register

	@ Enable MMU in ARMv6 mode.
	mov	r4, #TTBCR_S_PD1	@ Don't use TTBR1, PD aligned to 16KiB
	mcr	p15, 0, r4, c2, c0, 2	@ Translation Table Base Control

	mrc	p15, 0, r4, c1, c0, 0	@ Control Register
	movw	r5, #CPU_CONTROL_MMU_ENABLE
	movt	r5, #(CPU_CONTROL_XP_ENABLE >> 16)
	orr	r4, r4, r5
	dsb
	mcr	p15, 0, r4, c1, c0, 0	@ Control Register
	isb

	@ Configure exception base vector.
	ldr	r4, =_exc_vector
	mcr	p15, 0, r4, c12, c0, 0	@ Secure or Non-secure VBAR
	mcr	p15, 0, r4, c12, c0, 1	@ Monitor Vector Base Address Register

	@ Initialially we're running in supervisor mode.
	mrs	r4, spsr
	bic	r4, r4, #PSR_MODE
	@ Setup the stack for fast interrupt mode.
	orr	r5, r4, #PSR_FIQ32_MODE
	msr	cpsr_c, r5
	ldr	sp, =_irq_stack+4096
	@ Setup the stack for interrupt mode.
	orr	r5, r4, #PSR_IRQ32_MODE
	msr	cpsr_c, r5
	ldr	sp, =_irq_stack+4096
	@ Setup the stack for abort mode.
	orr	r5, r4, #PSR_ABT32_MODE
	msr	cpsr_c, r5
	ldr	sp, =_abt_stack+4096
	@ Setup the stack for supervisor mode.
	orr	r5, r4, #PSR_SVC32_MODE
	msr	cpsr_c, r5
	ldr	sp, =_svc_stack+4096
	@ Continue in supervisor mode.

	@ Cheat for CPU#1-CPU#3 for now.
	cmp	r3, #0
	beq	1f
	ldr	sp, =0x8000
	sub	sp, r3, lsl #12
1:
 
	@ Enter kernel_main with empty stack.
	mov	r4, lr
	ldr	lr, =kernel_exit
	bx	r4

@ vim: ft=armv5 ts=8 sw=8 noet
	
#endif	
