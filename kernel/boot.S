#ifdef AARCH64
// AArch64 mode 
// To keep this in the first portion of the binary.

#include "dev/bcm2836reg.h"

#define UPPERADDR 0xFFFF000000000000
#define PHYSADDR(x) ((x) - UPPERADDR)

	
.section ".text.boot"
 
//.org 0x80000

.section ".init"
 
.globl _start
.local no_cpu0_init
 
	
_start:

set_cores:	// [lr] kernel entry point
	LDR 	lr, =kernel_entry

	// [x3] core number 
	MRS	X3, MPIDR_EL1 		// Multiprocessor Affinity Register
	ANDS 	X3, X3, #3		
	BEQ 	clear_bss

	// [CPU#1-CPU#3] Wait for entry point to appear in local mailbox #3
	ldr	x4, =BCM2836_ARM_LOCAL_BASE + BCM2836_LOCAL_MAILBOX3_CLRN(0)

	lsl	x3, x3, #4
2:	wfe
	ldr	lr, [x4, x3]	// read mailbox #3 for n-th CPU
	cmp	lr, #0
	beq	2b
	
	str	lr, [x4, x3]	// clear the mailbox
	lsr	x3, x3, #4

	// [CPU#0] Clear BSS section.
clear_bss:
	ldr	x4, =PHYSADDR(_bss_start)
	ldr	x9, =PHYSADDR(_bss_end)
	mov	x5, #0
	mov	x6, #0
1:	stp 	x5, x6, [x4], #16
	stp 	x5, x6, [x4], #16	
	cmp	x4, x9
	blo	1b


	
setup_translation:
	// Initialize translation table control registers
	LDR	X1, =0x103510	// 4GB space 4KB granularity
				// Inner-shareable.

	MSR	TCR_EL1, X1	// Normal Inner and Outer Cacheable.
	LDR	X1, =0xFF440400	// ATTR0 Device-nGnRnE ATTR1 Device.
	MSR	MAIR_EL1,X1	// ATTR2 Normal Non-Cacheable.
				// ATTR3 Normal Cacheable.

	ADR	X0, PHYSADDR(_kernel_pde)//ttb0_base// ttb0_base must be a 4KB-aligned address.
	MSR	TTBR1_EL1, X0

	// Set up translation table entries in memory with looped store
	// instructions.
	// Set the level 1 translation table.
	// The first entry points to level2_pagetable.
	LDR	X1, =PHYSADDR(_level2_pagetable) // Must  be a 4KB align address.
	LDR	X2, =0xFFFFF000
	AND	X2, X1, X2             // NSTable=0 APTable=0 XNTable=0 PXNTable=0.
	ORR	X2, X2, 0x3
	STR	X2, [X0], #8

	// The second entry is 1GB block from 0x40000000 to 0x7FFFFFFF.
	//LDR	X2, =0x40000741	// Executable Inner and Outer Shareable.
	ADD	X2, X2, #4*1024
	STR	X2, [X0], #8	// R/W at all ELs secure memory
				// AttrIdx=000 Device-nGnRnE.

	// The third entry is 1GB block from 0x80000000 to 0xBFFFFFFF.
	//LDR	X2, =0x80000741
	ADD	X2, X2, #4*1024
	STR	X2, [X0], #8

	// The fourth entry is 1GB block from 0xC0000000 to 0xFFFFFFFF.
	//LDR	X2, =0xC0000741
	ADD	X2, X2, #4*1024
	STR	X2, [X0], #8

	// Set level 2 translation table.
	MOV	x6, #4
	LDR	X2, =0x0000074D	// Executable Inner and Outer Shareable.
				// R/W at all ELs secure memory.
				// AttrIdx=011  Normal Cacheable.
	//MOV	x1, PHYSADDR(_level2_pagetable)  // Base address of level2_pagetable
	MOV	x0, x1
	//ADD	x1, x1, #4*1024
	LDR    X5, =0x00200000	// Increase 2MB address each time.
	
1:	MOV    X4, #512		// Set 512 level2 block entries.
loop:
	STR    X2, [X0], #8 	// Each entry occupies 2 words.
	ADD    X2, X2, X5
	SUBS   X4, X4, #1
	BNE    loop

	SUBS	X6, X6, #1
	BNE	1b
	
	
	// You must ensure this bit is set to 1 before the caches and MMU are
	// enabled, or any cache and TLB maintenance operations are performed.
enable_cache:
	MRS 	X4, S3_1_C15_C2_0 	// Read EL1 CPU Auxiliary Control Register
	ORR 	X4, X4, #(1 << 6)	// Set [L1DEIEN] bit and L1 D-cache 
	MSR 	S3_1_C15_C2_0, X4 	// Write EL1 CPU Auxiliary Control Register
	
	// Enable data & instruction caches.
	MRS 	X4, SCTLR_EL1		// Read System Control Register
	ORR 	X4, X4, #(1 << 2)	// Set [C] bit and enable data caching
	ORR 	X4, X4, #(1 << 12) 	// Set [I] bit and enable instruction caching
	ORR	X4, X4, #0x1		// The M bit (MMU).
	MSR 	SCTLR_EL1, X4		// Write System Control Register 
	DSB	SY
	ISB

	//b no_cpu0_init
change_el:
	//Change Exception Level
	MRS	x4, CurrentEl
	//b 	set_cores
	CMP	x4, #8
	BEQ	el2_entry
	
	// Initialize SCTLR_EL2 and HCR_EL2 to save values before entering EL2.
	MSR    SCTLR_EL2, XZR
	MSR    HCR_EL2, XZR		// Determine the  EL2    Execution state.
	MRS    X0, SCR_EL3
	ORR    X0, X0, #(1<<10)		// RW  EL2 Execution state is AArch64.
	ORR    X0, X0, #(1<<0)		// NS   EL1 is Non-secure world.
	MSR    SCR_EL3, x0
	MOV    X0, #0b01001		// DAIF=0000
	MSR    SPSR_EL3, X0 		// M[4:0]=01001 EL2h must match SCR_EL3.RW

	// Determine EL2 entry.
	ADR    X0, el2_entry     // el2_entry points to the first instruction of
	MSR    ELR_EL3, X0       // EL2 code.

	ERET
	MRS 	x4, CurrentEl	
	
el2_entry:
	// Initialize the    SCTLR_EL1 register before entering EL1.
	MSR    SCTLR_EL1, XZR

	// Determine the  EL1    Execution state.
	MRS    X0, HCR_EL2
	ORR    X0,   X0, #(1<<31) 	// RW=1  EL1  Execution state is AArch64.
	MSR    HCR_EL2, X0
	MOV    X0, #0b00101      // DAIF=0000
	MSR    SPSR_EL2, X0      // M[4:0]=00101 EL1h must match HCR_EL2.RW.
	ADR    X0, el1_entry     // el1_entry points to the first instruction of
	MSR    ELR_EL2, X0       // EL1 code.

	ERET
el1_entry:
	// Determine the  EL0    Execution state.
	MOV    X0, #0b00000      // DAIF=0000  M[4:0]=00000  EL0t.
	MSR    SPSR_EL1, X0
	//ADR    x0, el0_entry     // el1_entry points to the first instruction of
	//MSR    ELR_EL1, X0       // EL0 code.ERET el0_entry:// EL0 code here.	
	//ERET
	MRS	x4, CurrentEl

	


no_cpu0_init:
	// Setup domains - Dom0 is usable, rest is disabled.
// 	mov	x4, #1
// 	mcr	p15, 0, r4, c3, c0, 0	@ Domain Access Control Register


// 	@ Configure exception base vector.
// 	ldr	r4, =_exc_vector
// 	mcr	p15, 0, r4, c12, c0, 0	@ Secure or Non-secure VBAR
// 	mcr	p15, 0, r4, c12, c0, 1	@ Monitor Vector Base Address Register

// 	@ Initialially we're running in supervisor mode.
// 	mrs	r4, spsr
// 	bic	r4, r4, #PSR_MODE
	// Setup the stack for fast interrupt mode.
// 	orr	r5, r4, #PSR_FIQ32_MODE
// 	msr	cpsr_c, r5
// 	ldr	sp, =_irq_stack+4096
// 	@ Setup the stack for interrupt mode.
// 	orr	r5, r4, #PSR_IRQ32_MODE
// 	msr	cpsr_c, r5
// 	ldr	sp, =_irq_stack+4096
// 	@ Setup the stack for abort mode.
// 	orr	r5, r4, #PSR_ABT32_MODE
// 	msr	cpsr_c, r5
// 	ldr	sp, =_abt_stack+4096
 	// Setup the stack for supervisor mode.
// 	orr	x5, x4, #PSR_SVC32_MODE
// 	msr	cpsr_c, r5
 	ldr	x4, =_svc_stack+4096
	mov 	sp, x4	
	// Continue in supervisor mode.

	// Cheat for CPU#1-CPU#3 for now.
 	// cmp	x3, #0
 	// beq	1f
 	// ldr	x4, =0x80000
	// mov 	sp, x4
	// lsl	x3, x3, #12
 	// sub	sp, sp, x3
	// lsr	x3, x3, #12	
1:

 	// Enter kernel_entry with empty stack.
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
