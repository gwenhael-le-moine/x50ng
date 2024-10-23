# $Id: Makefile,v 1.29 2008/12/11 12:18:17 ecd Exp $
#

TARGET = x49gpng
TARGET_ALLCAPS = X49GPNG

VERSION_MAJOR = 1
VERSION_MINOR = 1
PATCHLEVEL = 0

#
DEBUG_CFLAGS = -g # -pg
OPTIM = 2

# GTK
GTK_CFLAGS = $(shell pkg-config --cflags gtk+-2.0)
GTK_LDLIBS = $(shell pkg-config --libs gtk+-2.0) -lz -lm

# Embedded qemu
QEMU_DIR = src/qemu-git
QEMU_DEFINES = -DTARGET_ARM -DX49GP \
	-D_GNU_SOURCE -D_FILE_OFFSET_BITS=64 \
	-D_LARGEFILE_SOURCE \
	-DNEED_CPU_H \
	-fno-strict-aliasing
QEMU_OBJS = $(QEMU_DIR)/arm-softmmu/exec.o \
	$(QEMU_DIR)/arm-softmmu/translate-all.o \
	$(QEMU_DIR)/arm-softmmu/cpu-exec.o \
	$(QEMU_DIR)/arm-softmmu/translate.o \
	$(QEMU_DIR)/arm-softmmu/fpu/softfloat.o \
	$(QEMU_DIR)/arm-softmmu/op_helper.o \
	$(QEMU_DIR)/arm-softmmu/helper.o \
	$(QEMU_DIR)/arm-softmmu/disas.o \
	$(QEMU_DIR)/arm-softmmu/i386-dis.o \
	$(QEMU_DIR)/arm-softmmu/arm-dis.o \
	$(QEMU_DIR)/arm-softmmu/tcg/tcg.o \
	$(QEMU_DIR)/arm-softmmu/iwmmxt_helper.o \
	$(QEMU_DIR)/arm-softmmu/neon_helper.o
QEMU_INCLUDES=-I$(QEMU_DIR)/target-arm \
	-I$(QEMU_DIR) \
	-I$(QEMU_DIR)/fpu \
	-I$(QEMU_DIR)/arm-softmmu

# What if this is MacOSX
COCOA_LIBS=$(shell if [ "`uname -s`" = "Darwin" ]; then echo "-F/System/Library/Frameworks -framework Cocoa -framework IOKit"; fi)

X49GP_DEBUG = \
	-DDEBUG_X49GP_MODULES \
	-DDEBUG_S3C2410_SRAM \
	-DDEBUG_S3C2410_MEMC \
	-DDEBUG_S3C2410_INTC \
	-DDEBUG_S3C2410_POWER \
	-DDEBUG_S3C2410_LCD \
	-DDEBUG_S3C2410_UART \
	-DDEBUG_S3C2410_TIMER \
	-DDEBUG_S3C2410_USBDEV \
	-DDEBUG_S3C2410_WATCHDOG \
	-DDEBUG_S3C2410_IO_PORT \
	-DDEBUG_S3C2410_RTC \
	-DDEBUG_S3C2410_ADC \
	-DDEBUG_S3C2410_SDI \
	-DDEBUG_S3C2410_SPI \
	-DDEBUG_X49GP_SYSCALL \
	-DDEBUG_X49GP_FLASH_READ \
	-DDEBUG_X49GP_FLASH_WRITE \
	-UDEBUG_X49GP_SYSRAM_READ \
	-UDEBUG_X49GP_SYSRAM_WRITE \
	-UDEBUG_X49GP_ERAM_READ \
	-UDEBUG_X49GP_ERAM_WRITE \
	-UDEBUG_X49GP_IRAM_READ \
	-UDEBUG_X49GP_IRAM_WRITE \
	-DDEBUG_X49GP_TIMER_IDLE \
	-DDEBUG_X49GP_ARM_IDLE \
	-DDEBUG_X49GP_ENABLE_IRQ \
	-DDEBUG_X49GP_BLOCK \
	-DDEBUG_X49GP_MAIN \
	-DDEBUG_X49GP_UI

X49GP_INCLUDES = -I./src \
	-I./src/bitmaps \
	$(QEMU_INCLUDES)

X49GP_CFLAGS = $(CFLAGS) \
	-O$(OPTIM) \
	-Wall \
	-Werror \
	$(DEBUG_CFLAGS) \
	$(X49GP_INCLUDES) \
	$(QEMU_DEFINES) \
	$(GTK_CFLAGS) \
	-D_GNU_SOURCE=1 \
	-DVERSION_MAJOR=$(VERSION_MAJOR) \
	-DVERSION_MINOR=$(VERSION_MINOR) \
	-DPATCHLEVEL=$(PATCHLEVEL) \
	-DX49GP_DATADIR=\"$(INSTALL_DATA_DIR)\" \
	-Wno-error=deprecated-declarations

ifeq ($(DEBUG), yes)
	X49GP_CFLAGS += $(X49GP_DEBUG)
endif

X49GP_LDFLAGS = $(DEBUG_CFLAGS) $(LDFLAGS)
X49GP_LDLIBS = $(QEMU_OBJS) $(GDB_LIBS) $(COCOA_LIBS) $(GTK_LDLIBS)

SRCS = ./src/main.c \
	./src/module.c \
	./src/flash.c \
	./src/sram.c \
	./src/s3c2410.c \
	./src/s3c2410_sram.c \
	./src/s3c2410_memc.c \
	./src/s3c2410_intc.c \
	./src/s3c2410_power.c \
	./src/s3c2410_lcd.c \
	./src/s3c2410_nand.c \
	./src/s3c2410_uart.c \
	./src/s3c2410_timer.c \
	./src/s3c2410_usbdev.c \
	./src/s3c2410_watchdog.c \
	./src/s3c2410_io_port.c \
	./src/s3c2410_rtc.c \
	./src/s3c2410_adc.c \
	./src/s3c2410_spi.c \
	./src/s3c2410_sdi.c \
	./src/s3c2410_arm.c \
	./src/ui.c \
	./src/timer.c \
	./src/tiny_font.c \
	./src/symbol.c \
	./src/gdbstub.c \
	./src/block.c \
	./src/options.c

OBJS = $(SRCS:.c=.o)

# TEMPO hack
VVFATOBJS = ./src/block-vvfat.o \
	./src/block-qcow.o \
	./src/block-raw.o \
	$(QEMU_DIR)/cutils.o

all: do-it-all

ifeq (.depend,$(wildcard .depend))
include .depend
do-it-all: $(QEMU_DIR) dist/$(TARGET)
else
do-it-all: depend-and-build
endif

# Compilation
dist/$(TARGET): $(OBJS) $(VVFATOBJS) $(QEMU_OBJS)
	$(CC) $(X49GP_LDFLAGS) -o $@ $(OBJS) $(VVFATOBJS) $(LDLIBS) $(X49GP_LDLIBS)

%.o: %.c
	$(CC) $(X49GP_CFLAGS) -o $@ -c $<

./src/block-vvfat.o: ./src/block-vvfat.c
	$(CC) $(X49GP_CFLAGS) -fno-aggressive-loop-optimizations -o $@ -c $<

# Compilation of qemu-git
$(QEMU_DIR)/config-host.h:
	+( cd $(QEMU_DIR); \
	./configure-small --extra-cflags=-DX49GP; \
	$(MAKE) -f Makefile-small )

$(QEMU_OBJS): dummy
	+$(MAKE) -C $(QEMU_DIR) -f Makefile-small

# Depend
MAKEDEPEND = $(CC) -MM

depend-libs: $(QEMU_DIR)/config-host.h

depend-and-build: depend
	$(MAKE) -C . all

depend: depend-libs
	$(MAKEDEPEND) $(X49GP_CFLAGS) $(SRCS) >.depend

# Cleaning
clean-qemu:
	$(MAKE) -C $(QEMU_DIR) -f Makefile-small clean

clean: clean-qemu
	rm -f ./src/*.o core *~ .depend

distclean: clean
	$(MAKE) -C $(QEMU_DIR) -f Makefile-small distclean
	rm -f dist/$(TARGET) dist/$(TARGET).desktop dist/$(TARGET).man

mrproper: clean-qemu distclean
	make -C dist/firmware/ mrproper

# auto-format code
pretty-code:
	clang-format -i ./src/*.c ./src/*.h

# Populate dist/firmware/ from hpcalc.org
pull-firmware:
	make -C dist/firmware/

# Create an sdcard file
sdcard:
ifeq ($(shell uname),Darwin)
	rm -f sdcard.dmg
	hdiutil create $@ -megabytes 64 -fs MS-DOS -volname $(TARGET)
else
	/sbin/mkdosfs -v -C -S 512 -f 2 -F 16 -r 512 -R 2 -n "$(TARGET)" $@ 65536
endif

dist/$(TARGET).desktop: dist/$(TARGET).desktop.in
	perl -p -e "s!TARGET!$(TARGET)!" < dist/$(TARGET).desktop.in >$@

dist/$(TARGET).man: dist/$(TARGET).man.in
	perl -p -e "s!TARGET_ALLCAPS!$(TARGET_ALLCAPS)!;" -e "s!TARGET!$(TARGET)!" < dist/$(TARGET).man.in >$@

# Installation
INSTALL_PREFIX = /usr/local
INSTALL_BINARY_DIR = "$(INSTALL_PREFIX)"/bin
INSTALL_DATA_DIR = "$(INSTALL_PREFIX)"/share/$(TARGET)
INSTALL_DOC_DIR = "$(INSTALL_PREFIX)"/doc/$(TARGET)
INSTALL_MENU_DIR = "$(INSTALL_PREFIX)"/share/applications
INSTALL_MAN_DIR = "$(INSTALL_PREFIX)/share/man/man1"

install: all dist/$(TARGET).desktop dist/$(TARGET).man
	install -D -m 755 dist/$(TARGET) "$(DESTDIR)$(INSTALL_BINARY_DIR)/$(TARGET)"
	mkdir -p "$(DESTDIR)$(INSTALL_DATA_DIR)/"
	cp dist/*.png "$(DESTDIR)$(INSTALL_DATA_DIR)/"
	install -D -m 644 dist/$(TARGET).desktop "$(DESTDIR)$(INSTALL_MENU_DIR)/$(TARGET).desktop"
	install -D -m 644 dist/$(TARGET).man "$(DESTDIR)$(INSTALL_MAN_DIR)/$(TARGET).1"
	cp -R dist/firmware/ "$(DESTDIR)$(INSTALL_DATA_DIR)/firmware"

dummy:
