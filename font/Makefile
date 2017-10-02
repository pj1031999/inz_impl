TOPDIR := $(realpath ..)

SOURCES_C := \
	terminus16.c

build: font.a

include $(TOPDIR)/build.mk

font.a: $(OBJECTS)

extra-clean:
	$(RM) font.a

# vim: tabstop=8 shiftwidth=8 noexpandtab:
