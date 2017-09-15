@ boot.S - assembly startup code
 
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
	ldr	r4, =_kernel_pde - 0xc0000000
	mcr	p15, 0, r4, c2, c0, 0	@ Translation Table Base Register 0

	@ 1:1 mapping for first 3GiB
	mov	r5, #0x0140E		@ TEX=1, APX=0, AP=1, C=1, B=1, section
	mov	r6, #3072
1:	str	r5, [r4], #4
	adds	r5, r5, #1024*1024	@ map next 1MiB section
	subs	r6, r6, #1
	bhi	1b

	@ Remap first 1GiB to 0xC000_0000
	movt	r5, 0
	mov	r6, #1024
1:	str	r5, [r4], #4
	adds	r5, r5, #1024*1024	@ map next 1MiB section
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
	mov	r6, #0
	movw	r5, #0x0001		@ Enable MMU
	movt	r5, #0x0080		@ Subpage AP disable
	orr	r4, r4, r5
	mcr	p15, 0, r6, c7, c10, 4	@ Data Synchronization Barrier
	mcr	p15, 0, r4, c1, c0, 0	@ write Control Register
	mcr	p15, 0, r6, c7, c5, 4	@ Instruction Synchronization Barrier

	@ Setup the stack.
	ldr	sp, =_stack_top
 
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
