@ boot.S - assembly startup code

#include "armreg.h"
 
.section ".init"
 
.globl _start
 
@ Entry point for the kernel.
@ r15 -> should begin execution at 0x8000.
@ r0 -> 0x00000000
@ r1 -> 0x00000C42
@ r2 -> 0x00000100 - start of ATAGS
@ preserve these registers as argument for kernel_main

_start:
	@ Setup initial page table address.
	ldr	r4, =_kernel_pde - 0x80000000
	mcr	p15, 0, r4, c2, c0, 0	@ Translation Table Base Register 0

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

	@ Setup domains - Dom0 is usable, rest is disabled.
	mov	r4, #1
	mcr	p15, 0, r4, c3, c0, 0	@ Domain Access Control Register

	@ Use only one page table.
	mov	r4, #0
	mcr	p15, 0, r4, c2, c0, 2	@ Translation Table Base Control

	@ Enable MMU in ARMv6 mode.
	mrc	p15, 0, r4, c1, c0, 0	@ read Control Register
	movw	r5, #0x0001		@ Enable MMU
	movt	r5, #0x0080		@ Subpage AP disable
	orr	r4, r4, r5
	dsb
	mcr	p15, 0, r4, c1, c0, 0	@ write Control Register
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
 
	@ Clear out bss.
	ldr	r4, =_bss_start
	ldr	r9, =_bss_end
	mov	r5, #0
	mov	r6, #0
	mov	r7, #0
	mov	r8, #0
1:	stmia	r4!, {r5-r8}		@ Store 16 bytes at once.
	cmp	r4, r9
	blo	1b
 
	@ Enter kernel_main with empty stack.
	ldr	lr, =kernel_exit
	ldr	r3, =kernel_main
	bx	r3

@ vim: ft=armv5 ts=8 sw=8 noet
