TOPDIR := $(realpath ..)

SOURCES_C := \
	fl_access.c \
	fl_debug.c \
	fl_file_string.c \
	fl_misc.c \
	fl_write.c \
	fl_cache.c \
	fl_filelib.c \
	fl_format.c \
	fl_table.c

build: fat_driver.a

include $(TOPDIR)/build.mk

CFLAGS += -I/home/michal/studia/praca_inz/rpi3-os-skeleton/fat_driver/include/usr/include

fat_driver.a: $(OBJECTS)

extra-clean:
	$(RM) fat_driver.a

# vim: tabstop=8 shiftwidth=8 noexpandtab:
