CURL = curl -L
RM = rm -f

VER := 4.0.0
QEMU := qemu-$(VER)
QEMUDIR := $(QEMU)-build

CC = gcc -g
CFLAGS = -O2 -Wall

all: qemu-install

### Qemu

$(QEMU).tar.xz:
	$(CURL) -o $@ "https://download.qemu.org/$(QEMU).tar.xz"

qemu-unpack: $(QEMU)/.unpack
$(QEMU)/.unpack: $(QEMU).tar.xz
	tar xvJf $^
	touch $@

qemu-configure: $(QEMU)/.configure
$(QEMU)/.configure: $(QEMU)/.unpack
	rm -rf $(QEMUDIR)
	mkdir $(QEMUDIR)
	cd $(QEMUDIR) && \
		../$(QEMU)/configure \
		--prefix=$(PWD)/bin/ \
		--enable-fdt \
		--target-list=,aarch64-softmmu
	sed -i -e '/qemu_.*dir=/s,/qemu,/qemu-rpi3,' \
		$(QEMUDIR)/config-host.mak
	touch $@

qemu-build: $(QEMU)/.build
$(QEMU)/.build: $(QEMU)/.configure
	cd $(QEMUDIR) && $(MAKE)
	touch $@

qemu-install: $(QEMU)/.install
$(QEMU)/.install: $(QEMU)/.build
	cd $(QEMUDIR) && $(MAKE) install
	mv $(PWD)/bin/bin/qemu-system-aarch64 $(PWD)/bin/bin/qemu-rpi3-aarch64
	touch $@

clean:
qemu-clean:
	$(RM) -r $(QEMUDIR)
	$(RM) -r $(QEMU)
	$(RM) -r $(PWD)/bin/

.PHONY: qemu-unpack qemu-configure qemu-build qemu-install qemu-clean

