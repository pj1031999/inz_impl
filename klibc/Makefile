TOPDIR := $(realpath ..)

SOURCES_C := \
	assert.c \
	sbrk.c \
	stdio/getchar.c \
	stdio/putchar.c \
	stdio/puts.c \
	stdio/printf.c \
	string/bzero.c \
	string/memchr.c \
	string/memmove.c \
	string/strlen.c \
	string/strncmp.c \
	string/strncpy.c \
	string/memcpy.c \
	string/memset.c 

build: klibc.a

include $(TOPDIR)/build.mk

klibc.a: $(OBJECTS)

extra-clean:
	$(RM) klibc.a

# vim: tabstop=8 shiftwidth=8 noexpandtab:
