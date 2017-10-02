TOPDIR := $(realpath ..)

SOURCES_C := \
	cons.c \
	clock.c \
	exc.c \
	irq.c \
	gfx.c \
	gfx_cons.c \
	main.c \
	mbox.c \
	pcpu.c \
	smp.c \
	uart.c \

SOURCES_ASM := \
	boot.S \
	evec.S

build: kernel.a

include $(TOPDIR)/build.mk

kernel.a: $(OBJECTS)

extra-clean:
	$(RM) kernel.a

# vim: tabstop=8 shiftwidth=8 noexpandtab:
