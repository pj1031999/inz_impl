TOPDIR := $(realpath .)

LDFLAGS   = -nostdlib -nostartfiles -nodefaultlibs

ifdef MIMIKER_ARMV8
QEMU	  = qemu-mimiker-aarch64
QEMUFLAGS = -smp 4 -M raspi3 -gdb tcp::9000 -serial stdio
GDB	  = aarch64-mimiker-elf-gdb
LDSCRIPT  = rpi3-os.lds

KERIMG = kernel8.img
KERELF = kernel8.elf
KERMAP = kernel8.map
export MIMIKER_ARMV8

else
QEMU	  = qemu-mimiker-aarch64
QEMUFLAGS = -smp 4 -M raspi2 -gdb tcp::9000 -serial stdio
GDB       = arm-none-eabi-gdb
LDSCRIPT  = rpi2-os.lds

KERIMG = kernel.img
KERELF = kernel.elf
KERMAP = kernel.map
endif

# Files required to link kernel image
KRT = kernel/kernel.a klibc/klibc.a font/font.a

build: build-kernel build-klibc build-font $(KERIMG)

include build.mk

$(KERELF): $(KRT) $(LDSCRIPT)
	@echo "[LD] $^ -> $@"
	$(CC) $(LDFLAGS) $(KRT) -lgcc -Wl,-Map=kernel.map -T$(LDSCRIPT) -o $@
 
$(KERIMG): $(KERELF)
	@echo "[OBJCOPY] $< -> $@"
	$(OBJCOPY) $(KERELF) --gap-fill=0xff -O binary $(KERIMG)

run: build
	$(QEMU) $(QEMUFLAGS) -kernel $(KERELF)

debug: build
	$(QEMU) $(QEMUFLAGS) -S -kernel $(KERELF)
	
debuggdb: build
	$(QEMU) $(QEMUFLAGS) -S -kernel $(KERELF) &
	$(GDB) \
	-ex "file $(KERELF)" \
	-ex 'target extended-remote localhost:9000' \
	-ex 'set scheduler-locking on' \
	-ex 'set remotetimeout unlimited' \
	-ex 'b kernel_entry'
 
extra-clean: clean-kernel clean-klibc
	$(RM) $(KERELF) $(KERIMG) $(KERMAP)
 
# vim: ts=8 sw=8 noet:
