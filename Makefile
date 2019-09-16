TOPDIR := $(realpath .)

LDFLAGS   = -nostdlib -nostartfiles -nodefaultlibs
QEMU	  = qemu-rpi3-aarch64
QEMUFLAGS = -smp 4 -M raspi3 -gdb tcp::9000 -serial stdio -d int,mmu,page
GDB	  = aarch64-rpi3-elf-gdb
LDSCRIPT  = rpi3-os.lds

KERIMG = kernel8.img
KERELF = kernel8.elf
KERMAP = kernel8.map

# Files required to link kernel image
KRT = kernel/kernel.a fat_driver/fat_driver.a klibc/klibc.a font/font.a 

build: build-kernel build-klibc build-font $(KERIMG)

toolchain:
	make -C external/qemu PREFIX=$(PWD)/toolchain
	make -C external/openOCD PREFIX=$(PWD)/toolchain
	make -C external/gnu PREFIX=$(PWD)/toolchain

include build.mk

$(KERELF): $(KRT) $(LDSCRIPT)
	@echo "[LD] $^ -> $@"
	$(CC) $(LDFLAGS) $(KRT) -lgcc -Wl,-Map=kernel.map -T$(LDSCRIPT) -o $@

$(KERIMG): $(KERELF)
	@echo "[OBJCOPY] $< -> $@"
	$(OBJCOPY) $(KERELF) --gap-fill=0xff -O binary $(KERIMG)

run: build
	$(QEMU) $(QEMUFLAGS) -kernel $(KERELF)

# debug: build
# 	$(QEMU) $(QEMUFLAGS) -S -kernel $(KERELF)

# debuggdb: build
# 	$(GDB) \
# 	-ex "file $(KERELF)" \
# 	-ex 'target extended-remote localhost:9000' \
# 	-ex 'set scheduler-locking on' \
# 	-ex 'set remotetimeout unlimited' \
# 	-x gdbinit

extra-clean: clean-kernel clean-klibc
	$(RM) $(KERELF) $(KERIMG) $(KERMAP)

.PHONY: run build toolchain

# vim: ts=8 sw=8 noet:
