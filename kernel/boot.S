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
#if 0
clear_cache:
	//Disable L1 Caches
	MRS    X0, SCTLR_EL3		// Read SCTLR_EL3.
	MRS    X0, SCTLR_EL1		// Read SCTLR_EL1.
	BIC    X0,   X0, #(0x1 << 2)   	// Disable D Cache.
	MSR    SCTLR_EL3, X0         	// Write SCTLR_EL3.
	MSR    SCTLR_EL1, X0         	// Write SCTLR_EL1.	

	// Invalidate Data cache to make the code general purpose.
	// Calculate the cache size first and loop through each set +
	//   way  .
	MOV    X0, #0x0             	//  X0 = Cache level
	MSR    CSSELR_EL1, x0		// 0x0 for L1 Dcache  0x2    for L2 Dcache.
	MRS    X4, CCSIDR_EL1     	// Read Cache Size ID.
	AND    X1, X4, #0x7
	ADD    X1, X1, #0x4         	// X1 = Cache Line Size.
	LDR    X3, =0x7FFF
	AND    X2, X3, X4, LSR   #13	// X2 = Cache Set Number – 1.
	LDR    X3, =0x3FF
	AND    X3, X3, X4, LSR   #3	//  X3 = Cache Associativity Number – 1.
	CLZ    W4,   W3             	// X4 = way position in the CISW  instruction.
	MOV    X5, #0               	//  X5 = way counter way_loop.
way_loop:
	MOV    X6, #0               	//  X6 = set counter set_loop.
set_loop:
	LSL    X7, X5,  X4 

	ORR    X7, X0, X7           	// Set way.
	LSL    X8, X6, X1
	ORR    X7, X7,  X8 	        // Set set.
	DC     cisw, X7         	// Clean and Invalidate cache line.
	ADD    X6, X6, #1           	// Increment set counter.
	CMP    X6, X2              	// Last set reached yet?
	BLE    set_loop             	// If not, iterate set_loop,
	ADD    X5, X5, #1           	// else, next way.
	CMP    X5, X3               	// Last way reached yet?
	BLE    way_loop             	// I f not, iterate way_loop
#endif
	
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
	TLBI ALLE1
	DSB SY
	ISB
	
#if 1
	// Initialize translation table control registers

	LDR	X1, =0x7910	// 64KB granularity
				// Inner-shareable.
	MSR	TCR_EL2, X1	// Normal Inner and Outer Cacheable.
	MSR	TCR_EL3, X1
	
	LDR	X1, =0xf9107910	// 64KB granularity
				// Inner-shareable.
	MSR	TCR_EL1, X1	// Normal Inner and Outer Cacheable.
	DSB	SY
	ISB
	
	LDR	X1, =0xFF440400	// ATTR0 Device-nGnRnE ATTR1 Device.
	MSR	MAIR_EL3,X1
	MSR	MAIR_EL2,X1	
	MSR	MAIR_EL1,X1	// ATTR2 Normal Non-Cacheable.
				// ATTR3 Normal Cacheable.
	ISB
	
	ADR	X0, PHYSADDR(_level2_pagetable)	// must be a 4KB-aligned address.
	MSR	TTBR1_EL1, X0
	MSR	TTBR0_EL1, X0
	MSR	TTBR0_EL2, X0
	MSR	TTBR0_EL3, X0	
	DSB	SY
	ISB

	LDR	X2, =0x00000721
	LDR    X5, =0x20000000	// Increase 512MB address each time.
	
	MOV    X4, #8192	// Set level2 block entries.
loop:
	STR    X2, [X0], #8 	// Each entry occupies 2 words.
	ADD    X2, X2, X5
	SUBS   X4, X4, #1
	BNE    loop
#endif


	// You must ensure this bit is set to 1 before the caches and MMU are
	// enabled, or any cache and TLB maintenance operations are performed.
enable_cache:
	//It is implemented in the CPUECTLR register.
	MRS    X0, S3_1_C15_C2_1
	ORR    X0, X0, #(0x1 << 6)	// The  SMP bit.
	DSB    SY
	ISB
	MSR    S3_1_C15_C2_1, X0 
	DSB    SY
	ISB

	//MSR   SCTLR_EL3, XZR
	MRS    X0, SCTLR_EL3

	ORR    X0, X0, #(0x1 << 2)     	// The   C bit (data cache).
	ORR    X0, X0, #(0x1 << 12)     // The   I bit (instruction cache).
	ORR    X0, X0, #0x1             // The   M bit (MMU).

	MSR    SCTLR_EL3, X0
	MSR    SCTLR_EL2, X0	
	//MSR    SCTLR_EL1, X0
	DSB    SY
	ISB

change_el:

#if 1
	//Change Exception Level
	MRS	x4, CurrentEl
	CMP	x4, #8
	BEQ	el2_entry
	
	// Initialize SCTLR_EL2 and HCR_EL2 to save values before entering EL2.
	mov	x4, 0b1
	MSR    	SCTLR_EL2, x4
	
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
	
el2_entry:
	MRS	x4, CurrentEl

	// Determine the  EL1    Execution state.
	MRS    X0, HCR_EL2
	ORR    X0,   X0, #(1<<31) 	// RW=1  EL1  Execution state is AArch64.
	MSR    HCR_EL2, X0

	MOV    X0, #0b00101      // DAIF=0000
	MSR    SPSR_EL2, X0      // M[4:0]=00101 EL1h must match HCR_EL2.RW.

	ADR    X0, el1_entry     // el1_entry points to the first instruction of
	MSR    ELR_EL2, X0       // EL1 code.
	ERET
	
#else

	/* Try drop from el3 to el1  */
	/*=============================================================*/
	/*      Initialize sctlr_el1                                   */
	/*=============================================================*/
	// mov x0, xzr
	// orr x0, x0, #(1 << 29)        
	// orr x0, x0, #(1 << 28)        
	// orr x0, x0, #(1 << 23)
	// orr x0, x0, #(1 << 22)
	// orr x0, x0, #(1 << 20)
	// orr x0, x0, #(1 << 11)
	 
	ORR    X0, X0, #(0x1 << 2)     	// The   C bit (data cache).
	ORR    X0, X0, #(0x1 << 12)     // The   I bit (instruction cache).
	ORR    X0, X0, #0x1             // The   M bit (MMU).	
	MSR    SCTLR_EL1, X0
	DSB    SY
	ISB

	/*=============================================================*/
	/*      Initialize scr_el3                                     */
	/*=============================================================*/
	mrs x0, scr_el3
	orr x0, x0, #(1<<10)    // Lower EL is 64bits
	ORR X0, X0, #(1<<0)	// NS EL1 is Non-secure world.
	msr scr_el3, x0

	/*=============================================================*/
	/*      Initialize spsr_el3                                    */
	/*=============================================================*/
	mov x0, xzr
	mov x0, #0b0101            /* EL2 */
	orr x0, x0, #(1 << 8)       /* Enable SError and External Abort. */
	orr x0, x0, #(1 << 7)       /* IRQ interrupt Process state mask. */
	orr x0, x0, #(1 << 6)       /* FIQ interrupt Process state mask. */
	msr spsr_el3, x0

	/*=============================================================*/
	/*      Initialize elr_el3                                     */
	/*=============================================================*/
	adr x0, el1_entry
	msr elr_el3, x0
	eret
		
#endif

el1_entry:
	MRS	x0, CurrentEl

	// Initialize the    SCTLR_EL1 register before entering EL1.
	MSR    SCTLR_EL1, XZR
	MRS    X0, SCTLR_EL1
	ORR    X0, X0, #(0x1 << 2)     	// The   C bit (data cache).
	ORR    X0, X0, #(0x1 << 12)     // The   I bit (instruction cache).
	ORR    X0, X0, #0x1             // The   M bit (MMU).
	MSR    SCTLR_EL1, X0
	DSB    SY
	ISB

	
	ldr x4, =0xffff0000000810c4
	AT S1E1R, X4
	ISB
	MRS X6, PAR_EL1
	ISB

	MRS	X0, SCTLR_EL1


	
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
