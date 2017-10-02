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
	ldr	r4, =BCM2836_ARM_LOCAL_BASE + BCM2836_LOCAL_MAILBOX3_CLRN(0)

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
