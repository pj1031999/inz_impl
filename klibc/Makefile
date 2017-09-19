TOPDIR := $(realpath ..)

SOURCES_C := \
	assert.c \
	bzero.c \
	memchr.c \
	printf.c \
	sbrk.c \
	strlen.c 

build: $(DEPFILES) klibc.a

include $(TOPDIR)/build.mk

klibc.a: $(OBJECTS)

extra-clean:
	$(RM) klibc.a

# vim: tabstop=8 shiftwidth=8 noexpandtab:
