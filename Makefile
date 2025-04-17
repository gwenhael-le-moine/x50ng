TARGET = x50ng

VERSION_MAJOR = 2
VERSION_MINOR = 3
PATCHLEVEL = 2

INSTALL_PREFIX = /usr/local
INSTALL_BINARY_DIR = "$(INSTALL_PREFIX)"/bin
INSTALL_DATA_DIR = "$(INSTALL_PREFIX)"/share/$(TARGET)
INSTALL_DOC_DIR = "$(INSTALL_PREFIX)"/doc/$(TARGET)
INSTALL_MENU_DIR = "$(INSTALL_PREFIX)"/share/applications
INSTALL_MAN_DIR = "$(INSTALL_PREFIX)/share/man/man1"

#
DEBUG_CFLAGS = -g # -fsanitize=leak -pg
OPTIM ?= 2

LUA_VERSION ?= lua
PKG_CONFIG ?= pkg-config

### lua
LUACFLAGS = $(shell "$(PKG_CONFIG)" --cflags $(LUA_VERSION))
LUALIBS = $(shell "$(PKG_CONFIG)" --libs $(LUA_VERSION))

# GTK
GTK_CFLAGS = $(shell "$(PKG_CONFIG)" --cflags gtk4)
GTK_LDLIBS = $(shell "$(PKG_CONFIG)" --libs gtk4) -lz -lm

# Embedded qemu
QEMU_DIR = src/qemu-git
QEMU_DEFINES = \
	-DTARGET_ARM \
	-D_GNU_SOURCE -D_FILE_OFFSET_BITS=64 \
	-D_LARGEFILE_SOURCE \
	-DNEED_CPU_H \
	-fno-strict-aliasing
QEMU_OBJS = \
	$(QEMU_DIR)/arm-softmmu/exec.o \
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
QEMU_INCLUDES = \
	-I$(QEMU_DIR) \
	-I$(QEMU_DIR)/target-arm \
	-I$(QEMU_DIR)/fpu \
	-I$(QEMU_DIR)/arm-softmmu

# What if this is MacOSX
COCOA_LIBS=$(shell if [ "`uname -s`" = "Darwin" ]; then echo "-F/System/Library/Frameworks -framework Cocoa -framework IOKit"; fi)

X50NG_DEBUG = \
	-DDEBUG_X50NG_MODULES \
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
	-DDEBUG_X50NG_SYSCALL \
	-DDEBUG_X50NG_FLASH_READ \
	-DDEBUG_X50NG_FLASH_WRITE \
	-DDEBUG_X50NG_TIMER_IDLE \
	-DDEBUG_X50NG_ARM_IDLE \
	-DDEBUG_X50NG_ENABLE_IRQ \
	-DDEBUG_X50NG_BLOCK \
	-DDEBUG_X50NG_MAIN

X50NG_INCLUDES = \
	-I./src/x50ng/ \
	-I./src/x50ng/s3c2410/ \
	-I./src/qemu-git/ \
	$(QEMU_INCLUDES)

X50NG_CFLAGS = \
	$(CFLAGS) \
	-O$(OPTIM) \
	-Wall \
	$(DEBUG_CFLAGS) \
	$(X50NG_INCLUDES) \
	$(QEMU_DEFINES) \
	$(GTK_CFLAGS) \
	$(LUACFLAGS) \
	-D_GNU_SOURCE=1 \
	-DVERSION_MAJOR=$(VERSION_MAJOR) \
	-DVERSION_MINOR=$(VERSION_MINOR) \
	-DPATCHLEVEL=$(PATCHLEVEL) \
	-DX50NG_DATADIR=\"$(INSTALL_DATA_DIR)\"

ifeq ($(DEBUG), yes)
	X50NG_CFLAGS += $(X50NG_DEBUG)
endif

X50NG_LDFLAGS = $(DEBUG_CFLAGS) $(LDFLAGS)
X50NG_LDLIBS = $(QEMU_OBJS) $(GDB_LIBS) $(COCOA_LIBS) $(GTK_LDLIBS) $(LUALIBS)

SRCS = \
	./src/x50ng/s3c2410/s3c2410.c \
	./src/x50ng/s3c2410/s3c2410_sram.c \
	./src/x50ng/s3c2410/s3c2410_memc.c \
	./src/x50ng/s3c2410/s3c2410_intc.c \
	./src/x50ng/s3c2410/s3c2410_power.c \
	./src/x50ng/s3c2410/s3c2410_lcd.c \
	./src/x50ng/s3c2410/s3c2410_nand.c \
	./src/x50ng/s3c2410/s3c2410_uart.c \
	./src/x50ng/s3c2410/s3c2410_timer.c \
	./src/x50ng/s3c2410/s3c2410_usbdev.c \
	./src/x50ng/s3c2410/s3c2410_watchdog.c \
	./src/x50ng/s3c2410/s3c2410_io_port.c \
	./src/x50ng/s3c2410/s3c2410_rtc.c \
	./src/x50ng/s3c2410/s3c2410_adc.c \
	./src/x50ng/s3c2410/s3c2410_spi.c \
	./src/x50ng/s3c2410/s3c2410_sdi.c \
	./src/x50ng/s3c2410/s3c2410_arm.c \
	./src/x50ng/s3c2410/block.c \
	./src/x50ng/module.c \
	./src/x50ng/flash.c \
	./src/x50ng/sram.c \
	./src/x50ng/ui.c \
	./src/x50ng/timer.c \
	./src/x50ng/gdbstub.c \
	./src/x50ng/options.c \
	./src/x50ng/main.c

OBJS = $(SRCS:.c=.o)

# TEMPO hack
VVFATOBJS =	\
	$(QEMU_DIR)/cutils.o \
	./src/x50ng/s3c2410/block-vvfat.o \
	./src/x50ng/s3c2410/block-qcow.o \
	./src/x50ng/s3c2410/block-raw.o

all: do-it-all

ifeq (.depend,$(wildcard .depend))
include .depend
do-it-all: $(QEMU_DIR) dist/$(TARGET)
else
do-it-all: depend-and-build
endif

# Compilation
dist/$(TARGET): $(OBJS) $(VVFATOBJS) $(QEMU_OBJS)
	$(CC) $(X50NG_LDFLAGS) -o $@ $(OBJS) $(VVFATOBJS) $(LDLIBS) $(X50NG_LDLIBS)

%.o: %.c
	$(CC) $(X50NG_CFLAGS) -o $@ -c $<

# Compilation of qemu-git
$(QEMU_DIR)/config-host.h:
	+( cd $(QEMU_DIR); \
	./configure-small; \
	$(MAKE) -f Makefile-small )

$(QEMU_OBJS): qemu-objs

qemu-objs:
	+$(MAKE) -C $(QEMU_DIR) -f Makefile-small

clean-qemu:
	$(MAKE) -C $(QEMU_DIR) -f Makefile-small clean

# Depend
MAKEDEPEND = $(CC) -MM

depend-libs: $(QEMU_DIR)/config-host.h

depend-and-build: depend
	$(MAKE) -C . all

depend: depend-libs
	$(MAKEDEPEND) $(X50NG_CFLAGS) $(SRCS) >.depend

# for clangd
compile_commands.json: distclean
	bear -- make dist/$(TARGET)

# Cleaning
clean:
	rm -f ./src/x50ng/*.o ./src/x50ng/s3c2410/*.o core *~ .depend

distclean: clean clean-qemu
	rm -f compile_commands.json
	rm -f dist/$(TARGET) dist/$(TARGET).desktop dist/$(TARGET).man

mrproper: distclean
	make -C dist/firmware/ mrproper

# auto-format code
pretty-code:
	clang-format -i ./src/x50ng/*.c ./src/x50ng/*.h ./src/x50ng/s3c2410/*.c $(shell ls ./src/x50ng/s3c2410/*.h | grep -v s3c2410.h) ## s3c2410.h triggers an error

# Populate dist/firmware/ from hpcalc.org
pull-firmware:
	make -C dist/firmware/

# Installation
dist/$(TARGET).desktop: src/$(TARGET).desktop.in
	perl -p -e "s!TARGET!$(TARGET)!" < $^ >$@

dist/$(TARGET).man: src/$(TARGET).scd
	scdoc < $^ > $@

install: all dist/$(TARGET).desktop dist/$(TARGET).man
	mkdir -p "$(DESTDIR)$(INSTALL_BINARY_DIR)/"
	install -D -m 755 dist/$(TARGET) "$(DESTDIR)$(INSTALL_BINARY_DIR)/$(TARGET)"
	mkdir -p "$(DESTDIR)$(INSTALL_MENU_DIR)/"
	install -D -m 644 dist/$(TARGET).desktop "$(DESTDIR)$(INSTALL_MENU_DIR)/$(TARGET).desktop"
	mkdir -p "$(DESTDIR)$(INSTALL_MAN_DIR)/"
	install -D -m 644 dist/$(TARGET).man "$(DESTDIR)$(INSTALL_MAN_DIR)/$(TARGET).1"
	mkdir -p "$(DESTDIR)$(INSTALL_DATA_DIR)/"
	cp -R dist/firmware/ "$(DESTDIR)$(INSTALL_DATA_DIR)/firmware"
	cp dist/*.css "$(DESTDIR)$(INSTALL_DATA_DIR)/"
