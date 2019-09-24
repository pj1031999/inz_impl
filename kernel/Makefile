TOPDIR := $(realpath ..)

SOURCES_C := \
	cons.c \
	clock.c \
	exc.c \
	irq.c \
	gfx.c \
	gfx_cons.c \
	gpio.c \
	main.c \
	mbox.c \
	mmu.c \
	pcpu.c \
	pmman.c \
	smp.c \
	uart.c \
	vc_mbox.c \
	pmap.c \
	pmap_test.c \
	pagetable_setup.c \
	demo.c \
	demo_pmap.c \
	demo_uart.c \
	userspace_demo.c \
	sd.c \
	delays.c \
	diskio.c \
	pff.c \

SOURCES_ASM := \
	boot.S \
	ctx_switch.S \
	evec.S 

build: kernel.a

include $(TOPDIR)/build.mk

kernel.a: $(OBJECTS)

extra-clean:
	$(RM) kernel.a

# vim: tabstop=8 shiftwidth=8 noexpandtab:
