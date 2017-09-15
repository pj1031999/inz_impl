# Makefile - build script */
 
# build environment
PREFIX ?= /usr
ARMGNU ?= $(PREFIX)/bin/arm-none-eabi

CC	= $(ARMGNU)-gcc 
OBJCOPY = $(ARMGNU)-objcopy

# source files
SOURCES_ASM := $(wildcard src/*.S)
SOURCES_C   := $(wildcard src/*.c)
 
# object files
OBJS        := $(patsubst %.S,%.o,$(SOURCES_ASM))
OBJS        += $(patsubst %.c,%.o,$(SOURCES_C))
 
# Build flags
DEPENDFLAGS := -MD -MP
INCLUDES    := -I src/include
OPTFLAGS    := -O2 -mcpu=arm1176jzf-s
BASEFLAGS   := -ffreestanding -fno-builtin -fomit-frame-pointer
WARNFLAGS   := -Wall -Wextra -Wshadow -Wcast-align -Wwrite-strings
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
ASFLAGS     := $(INCLUDES) $(DEPENDFLAGS) -D__ASSEMBLY__
CFLAGS      := $(INCLUDES) $(DEPENDFLAGS) $(OPTFLAGS) $(BASEFLAGS) $(WARNFLAGS)
CFLAGS      += -std=c11
LDFLAGS     := -nostdlib -nostartfiles -nodefaultlibs

QEMU        := qemu-system-arm
QEMUFLAGS   := -M raspi2 -serial stdio 
 
# build rules
all: kernel.img
 
include $(wildcard src/*.d)
 
kernel.elf: $(OBJS) link-arm-eabi.ld
	@echo "[LD] $^ -> $@"
	@$(CC) $(LDFLAGS) $(OBJS) -lgcc -Wl,-Map=kernel.map -Tlink-arm-eabi.ld -o $@
 
kernel.img: kernel.elf
	@echo "[OBJCOPY] $< -> $@"
	@$(OBJCOPY) kernel.elf -O binary kernel.img

run: kernel.elf
	$(QEMU) $(QEMUFLAGS) -kernel kernel.elf

debug: kernel.elf
	$(QEMU) $(QEMUFLAGS) -gdb tcp::9000 -S -kernel kernel.elf
 
clean:
	$(RM) $(OBJS) kernel.elf kernel.img
 
dist-clean: clean
	$(RM) *.d
 
# C.
%.o: %.c Makefile
	@echo "[CC] $< -> $@"
	@$(CC) -g $(CFLAGS) -c $< -o $@
 
# AS.
%.o: %.S Makefile
	@echo "[AS] $< -> $@"
	@$(CC) -g $(ASFLAGS) -c $< -o $@

# vim: ts=8 sw=8 noet
