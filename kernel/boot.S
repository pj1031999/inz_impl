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
 
	
// Entry point for the kernel. Registers are not defined as in AArch32.
_start:
	//b old
	// Enable data & instruction caches.
	MRS x4, CurrentEl
	MRS X4, SCTLR_EL1	// Read System Control Register
	ORR X4, X4, #(1 << 2)	// Set [C] bit and enable data caching
	ORR X4, X4, #(1 << 12) 	// Set [I] bit and enable instruction caching
	MSR SCTLR_EL1, X4       // Write System Control Register 


	// You must ensure this bit is set to 1 before the caches and MMU are
	// enabled, or any cache and TLB maintenance operations are performed.
	MRS X4, S3_1_C15_C2_0 	// Read EL1 CPU Auxiliary Control Register
	ORR X4, X4, #(1 << 6)	// Set [L1DEIEN] bit and L1 D-cache data RAM error injection enable
	MSR S3_1_C15_C2_0, X4 	// Write EL1 CPU Auxiliary Control Register	
	// [lr] kernel entry point
	ldr lr, =kernel_entry

	// [x3] core number 
	MRS X3, MPIDR_EL1 	// Multiprocessor Affinity Register
	ANDS X3, X3, #3		// CORTEXA9_MPIDR_CPUID
	BEQ 1f

	// [CPU#1-CPU#3] Wait for entry point to appear in local mailbox #3
	ldr	x4, =BCM2836_ARM_LOCAL_BASE + BCM2836_LOCAL_MAILBOX3_CLRN(0)

	lsl	x3, x3, #4
2:	wfe
	ldr	lr, [x4, x3]// read mailbox #3 for n-th CPU
	cmp	lr, #0
	beq	2b
	
	str	lr, [x4, x3]	// clear the mailbox
	lsr	x3, x3, #4
	
	//Set up initial page table pointer (user space).
1:	ldr X4, =PHYSADDR(_kernel_pde)
	MSR TTBR0_EL1, X4 

	// [CPU#0] Fill in initial page table.
	cmp	X3, #0
	bne	no_cpu0_init

	// 1:1 mapping for first 4GiB
	//mov	x5, #0x0140E	//  TEX=1, APX=0, AP=1, C=1, B=1, section

	//table level0
	mov 	x5, x4//, lsl #2		//set attributes
	//add	x5, x4, #512	// just one descriptor 
	lsl	x5, x5, #12
	orr	x5, x5, #3		// type of descriptor: table
	str	x5, [x4], #8

	//level1
	mov 	x5, #0//, lsl #2		//set attributes
	lsl	x5, x5, #30
	orr	x5, x5, #1		// type of descriptor: block

	str	x5, [x4], #8
	ldr 	x6, =1024*1024*1024
	add	x5, x5, x6, lsl #12
	str	x5, [x4], #8
	add	x5, x5, x6, lsl #12
	str	x5, [x4], #8
	add	x5, x5, x6, lsl #12
	str	x5, [x4], #8

	MSR TTBR1_EL1, X4	

	//table level0
	mov 	x5, x4//, lsl #2		//set attributes	
	//add	x5, x4, #512	// just one descriptor 
	lsl	x5, x5, #12
	orr	x5, x5, #3		// type of descriptor: table
	str	x5, [x4], #8

	//level1
	mov 	x5, #0//, lsl #2	//set attributes
	lsl	x5, x5, #30
	orr	x5, x5, #1		// type of descriptor: block

	str	x5, [x4], #8
	add	x5, x5, x6, lsl #12
	str	x5, [x4], #8
	add	x5, x5, x6, lsl #12
	str	x5, [x4], #8
	add	x5, x5, x6, lsl #12
	str	x5, [x4], #8

	
// 	//level1
// 	mov	x6, #4		// 4 descriptors (1GiB each)
// 	add	x5, x4, #4 * 512	
// 	mov	x5, x5, lsl #11	
// 1:	or	x5, #3		// type of descriptor: table
// 	str	x5, [x4], #8
// 	add	x5, x5, #512, lsl #11
// 	sub	x6, x6, #1
// 	bhi	1b
// 	add	x4, x4, #508 * 8
	
// 	//level2
// 	mov	x6, #2048	// 2048 descriptors (2MiB each)
// 	add	x5, x4, #2048 * 8	
// 	mov	x5, x5, lsl #11	
// 1:	or	x5, #3		// type of descriptor: table
// 	str	x5, [x4], #8
// 	add	x5, x5, #512, lsl #11
// 	sub	x6, x6, #1
// 	bhi	1b

	/////////////////////////
	
// 	mov	x6, #1024
// 1:	str	x5, [x4], #8
// 	add	x5, x5, #4*1024*1024	// map next 4MiB section
// 	subs	x6, x6, #1
// 	bhi	1b

// 	//Set up initial page table pointer (kernal space).	
// 	MSR TTBR1_EL0, X4
	
// 	// Remap first 4GiB to UPPERADDR	
// 	and	x5, x5, #0xFFFF
// 	mov	x6, #1024
// 1:	str	x5, [x4], #8
// 	add	x5, x5, #4*1024*1024	// map next 4MiB section
// 	subs	x6, x6, #1
// 	bhi	1b

	// [CPU#0] Clear BSS section.
	ldr	x4, =PHYSADDR(_bss_start)
	ldr	x9, =PHYSADDR(_bss_end)
	mov	x5, #0
	mov	x6, #0
1:	//stmia	x4!, {x5-x8}		// Store 16 bytes at once.
	stp 	x5, x6, [x4], #16
	stp 	x5, x6, [x4], #16	
	cmp	x4, x9
	blo	1b

no_cpu0_init:
	// Setup domains - Dom0 is usable, rest is disabled.
// 	mov	x4, #1
// 	mcr	p15, 0, r4, c3, c0, 0	@ Domain Access Control Register

// 	@ Enable MMU in ARMv6 mode.
// 	mov	r4, #TTBCR_S_PD1	@ Don't use TTBR1, PD aligned to 16KiB
// 	mcr	p15, 0, r4, c2, c0, 2	@ Translation Table Base Control

// 	mrc	p15, 0, r4, c1, c0, 0	@ Control Register
// 	movw	r5, #CPU_CONTROL_MMU_ENABLE
// 	movt	r5, #(CPU_CONTROL_XP_ENABLE >> 16)
// 	orr	r4, r4, r5
// 	dsb
// 	mcr	p15, 0, r4, c1, c0, 0	@ Control Register
// 	isb

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
 	cmp	x3, #0
 	beq	1f
 	ldr	x4, =0x80000
	mov 	sp, x4
	lsl	x3, x3, #12
 	sub	sp, sp, x3
	lsr	x3, x3, #12	
1:

 	// Enter kernel_entry with empty stack.
 	mov	x4, lr
 	ldr	lr, =kernel_exit
 	br	x4

	
///////////////////////////////////////
// old
///////////////////////////////////////
		
old:
//read cpu id, stop slave cores
    mrs     x1, mpidr_el1
    and     x1, x1, #3
    cbz     x1, 2f
    // cpu id > 0, stop
1:  wfe
    b       1b
2:  // cpu id == 0
 
    // set stack before our code
    ldr     x1, =_svc_stack+4096//_start
    mov     sp, x1
 
    // clear bss
    ldr     x1, =PHYSADDR(_bss_start)
    ldr     x2, =PHYSADDR(_bss_end)
    sub     x2, x2, x1
3:  cbz     x2, 4f
    str     xzr, [x1], #8
    sub     x2, x2, #1
    cbnz    x2, 3b
 
    // jump to C code, should not return
4:
    bl      kernel_entry
    // for failsafe, halt this core too
    b 1b

	
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
