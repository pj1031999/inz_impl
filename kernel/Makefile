TOPDIR := $(realpath ..)

SOURCES_C := \
	clock.c \
	exc.c \
	gfx.c \
	main.c \
	uart.c

SOURCES_ASM := \
	boot.S \
	evec.S

build: kernel.a

include $(TOPDIR)/build.mk

kernel.a: $(OBJECTS)

extra-clean:
	$(RM) kernel.a

# vim: tabstop=8 shiftwidth=8 noexpandtab:
