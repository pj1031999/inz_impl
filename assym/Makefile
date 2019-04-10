TOPDIR := $(realpath ..)

SOURCES_C := \
	genassym.c

build: assym.h

include $(TOPDIR)/build.mk

assym.h: genassym.o
assym.h: genassym.sh
	sh genassym.sh genassym.o > assym.h

genassym.o: $(OBJECTS)

extra-clean:
	$(RM) assym.inc genassym.o

# vim: tabstop=8 shiftwidth=8 noexpandtab:
