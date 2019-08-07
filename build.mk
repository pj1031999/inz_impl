CPPFLAGS	= -I $(TOPDIR)/include
OPTFLAGS	= -O3 -fomit-frame-pointer -ggdb
TARGET		= aarch64-rpi3-elf
CPUFLAGS	= -mcpu=cortex-a53+nofp -march=armv8-a+nofp -mgeneral-regs-only -DAARCH64 -nostartfiles -ffreestanding  -MMD -ggdb


CC	= $(TARGET)-gcc -g
AR	= $(TARGET)-ar
GDB	= $(TARGET)-gdb
RANLIB	= $(TARGET)-ranlib
READELF = $(TARGET)-readelf
OBJCOPY = $(TARGET)-objcopy
OBJDUMP = $(TARGET)-objdump
NM   	= $(TARGET)-nm

WARNFLAGS   = -Wall -Wextra -Wshadow -Wcast-align -Wwrite-strings
WARNFLAGS   += -Wredundant-decls -Winline
WARNFLAGS   += -Wno-attributes -Wno-deprecated-declarations
WARNFLAGS   += -Wno-div-by-zero -Wno-endif-labels -Wfloat-equal
WARNFLAGS   += -Wformat=2 -Wno-format-extra-args -Winit-self
WARNFLAGS   += -Winvalid-pch -Wmissing-format-attribute
WARNFLAGS   += -Wmissing-include-dirs -Wno-multichar
WARNFLAGS   += -Wredundant-decls -Wshadow
WARNFLAGS   += -Wno-sign-compare -Wswitch -Wsystem-headers -Wundef
WARNFLAGS   += -Wno-pragmas -Wno-unused-but-set-parameter
WARNFLAGS   += -Wno-unused-but-set-variable -Wno-unused-result
WARNFLAGS   += -Wwrite-strings -Wdisabled-optimization -Wpointer-arith
WARNFLAGS   += -Werror
KERNFLAGS   = -ffreestanding -fno-builtin 

ASFLAGS     = $(CPUFLAGS) $(CPPFLAGS)
CFLAGS      = -std=gnu11 $(CPPFLAGS) $(CPUFLAGS) $(OPTFLAGS) $(KERNFLAGS) $(WARNFLAGS)

RM	    = rm -f -v

# Pass "VERBOSE=1" at command line to display command being invoked by GNU Make
ifneq ($(VERBOSE), 1)
.SILENT:
endif

# Disable all built-in recipes
.SUFFIXES:

%.o: %.c
	@echo "[CC] $< -> $@"
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.S
	@echo "[AS] $< -> $@"
	$(CC) $(ASFLAGS) -c $< -o $@

%.a:
	@echo "[AR] $^ -> $@"
	$(AR) rs $@ $^ 2> /dev/null

SOURCES = $(SOURCES_C) $(SOURCES_ASM)
OBJECTS = $(SOURCES_C:.c=.o) $(SOURCES_ASM:.S=.o)
DEPFILES = $(SOURCES_C:%.c=.%.d) $(SOURCES_ASM:%.S=.%.d)

.%.d: %.S
	@echo "[DEP] $<"
	$(CC) $(CPPFLAGS) -MM -MG -o $@ $<

.%.d: %.c
	@echo "[DEP] $<"
	$(CC) $(CFLAGS) $(CPPFLAGS) -MM -MG -o $@ $<

ifeq ($(words $(findstring $(MAKECMDGOALS), clean)), 0)
  -include $(DEPFILES)
endif

# Default rules
build-%:
	$(MAKE) -C $(@:build-%=%) build

clean-%:
	$(MAKE) -C $(@:clean-%=%) clean

clean: extra-clean
	$(RM) $(OBJECTS) $(DEPFILES) *~

.PHONY: build clean extra-clean build-% clean-%

# vim: ts=8 sw=8 noet:
