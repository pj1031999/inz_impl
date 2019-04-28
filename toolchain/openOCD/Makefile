CURL = curl -L
RM = rm -f

VER := 0.10.0+dev
OOCD := openocd-$(VER)
OOCDDIR := $(OOCD)-build

CC = gcc -g
CFLAGS = -O2 -Wall

all: openocd-install

### OpenOCD

$(OOCD)-download: $(OOCD)
$(OOCD) :
	git clone https://git.code.sf.net/p/openocd/code $(OOCD)

openocd-configure: $(OOCD)/.configure
$(OOCD)/.configure: $(OOCD)-download
	rm -rf $(OOCDDIR) &&	mkdir $(OOCDDIR)
	cd $(OOCD) && ./bootstrap && \
	sed -i 's/'"'"'openocd'"'"'/'"'"'openocd-rpi3'"'"'/g' configure
	cd $(OOCDDIR) && \
	    ../$(OOCD)/configure \
	    --prefix=$(PWD)/bin/ \
	    --program-suffix='-rpi3' \
	    --enable-jlink
	touch $@

openocd-build: $(OOCD)/.build
$(OOCD)/.build: $(OOCD)/.configure
	cd $(OOCDDIR) && $(MAKE) -j$(shell nproc --all) LDFLAGS="-Wl,--as-needed"
	touch $@

openocd-install: $(OOCD)/.install
$(OOCD)/.install: $(OOCD)/.build
	cd $(OOCDDIR) && $(MAKE) install
	touch $@

clean: openocd-clean
openocd-clean:
	$(RM) -r $(OOCDDIR)
	$(PWD)/bin/
	
distclean: clean
	$(RM) -r $(OOCD)


.PHONY: openocd-unpack openocd-configure openocd-build openocd-install openocd-clean

