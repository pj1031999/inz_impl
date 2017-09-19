TARGET	= arm-none-eabi
CPU	= cortex-a7

CC	= $(TARGET)-gcc -g
AR	= $(TARGET)-ar
GDB	= $(TARGET)-gdb
RANLIB	= $(TARGET)-ranlib
READELF = $(TARGET)-readelf
OBJCOPY = $(TARGET)-objcopy
OBJDUMP = $(TARGET)-objdump
NM   	= $(TARGET)-nm

CPPFLAGS    = -I $(TOPDIR)/include
OPTFLAGS    = -O2 -fomit-frame-pointer
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

ASFLAGS     = -mcpu=$(CPU) $(CPPFLAGS)
CFLAGS      = -std=c11 -mcpu=$(CPU) -ffreestanding -fno-builtin $(CPPFLAGS) 
CFLAGS      += $(OPTFLAGS) $(WARNFLAGS)

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

# Automatically generate dependecy files
define emit_dep_rule
CFILE = $(1)
DFILE = .$(patsubst %.S,%.d,$(patsubst %.c,%.d,$(1)))
$$(DFILE): $$(CFILE)
	@echo "[DEP] $$<"
	$(CC) $(CFLAGS) $(CPPFLAGS) -MM -MG $$^ -o $$@
endef

$(foreach file,$(SOURCES) null,$(eval $(call emit_dep_rule,$(file))))

ifeq ($(words $(findstring $(MAKECMDGOALS), clean)), 0)
  -include $(DEPFILES)
endif

# Default rules
build-%:
	$(MAKE) -C $(@:build-%=%) build

clean-%:
	$(MAKE) -C $(@:clean-%=%) clean

clean: extra-clean
	$(RM) *.o .*.d *~

.PHONY: build clean extra-clean

# vim: ts=8 sw=8 noet:
