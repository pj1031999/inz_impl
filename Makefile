TOPDIR := $(realpath .)

LDFLAGS   = -nostdlib -nostartfiles -nodefaultlibs
LDSCRIPT  = rpi2-os.lds

QEMU	  = qemu-system-arm
QEMUFLAGS = -M raspi2 -serial stdio 

# Files required to link kernel image
KRT = kernel/kernel.a klibc/klibc.a

build: build-kernel build-klibc kernel.img

include build.mk
 
kernel.elf: $(KRT) $(LDSCRIPT)
	@echo "[LD] $^ -> $@"
	$(CC) $(LDFLAGS) $(KRT) -lgcc -Wl,-Map=kernel.map -T$(LDSCRIPT) -o $@
 
kernel.img: kernel.elf
	@echo "[OBJCOPY] $< -> $@"
	@$(OBJCOPY) kernel.elf -O binary kernel.img

run: build
	$(QEMU) $(QEMUFLAGS) -kernel kernel.elf

debug: build
	$(QEMU) $(QEMUFLAGS) -gdb tcp::9000 -S -kernel kernel.elf
 
extra-clean: clean-kernel clean-klibc
	$(RM) kernel.elf kernel.img kernel.map
 
# vim: ts=8 sw=8 noet:
