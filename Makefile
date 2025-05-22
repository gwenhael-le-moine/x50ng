TARGET = x50ng

VERSION_MAJOR = 2
VERSION_MINOR = 4
PATCHLEVEL = 0

PREFIX = /usr
BINDIR = $(PREFIX)/bin
DATADIR = $(PREFIX)/share/$(TARGET)
DOCDIR = $(PREFIX)/doc/$(TARGET)
MENUDIR = $(PREFIX)/share/applications
MANDIR = $(PREFIX)/share/man/man1

# optimization level for cc
OPTIM ?= 2

LUA_VERSION ?= lua
PKG_CONFIG ?= pkg-config

### lua
LUA_CFLAGS = $(shell "$(PKG_CONFIG)" --cflags $(LUA_VERSION))
LUA_LDLIBS = $(shell "$(PKG_CONFIG)" --libs $(LUA_VERSION))

# GTK
GTK_CFLAGS = $(shell "$(PKG_CONFIG)" --cflags gtk4)
GTK_LDLIBS = $(shell "$(PKG_CONFIG)" --libs gtk4) -lz -lm

# Ncurses
NCURSES_CFLAGS = $(shell "$(PKG_CONFIG)" --cflags ncursesw) -DNCURSES_WIDECHAR=1
NCURSES_LDLIBS = $(shell "$(PKG_CONFIG)" --libs ncursesw)

# Embedded qemu
QEMU_DIR = src/qemu
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

X50NG_DEBUG = \
	-g \
	-pg \
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
	-I./src/ \
	-I./src/s3c2410/ \
	-I./${QEMU_DIR}/ \
	$(QEMU_INCLUDES)

X50NG_DISABLED_WARNINGS = \
	-Wno-unused-parameter \
	-Wno-pointer-arith \
	-Wno-overflow # see src/s3c2410/block-vvfat.c:437

X50NG_CFLAGS = \
	$(CFLAGS) \
	-O$(OPTIM) \
	-W \
	-Wall \
	-Wextra \
	-pedantic \
	$(X50NG_DISABLED_WARNINGS) \
	$(DEBUG_CFLAGS) \
	$(X50NG_INCLUDES) \
	$(QEMU_DEFINES) \
	$(GTK_CFLAGS) \
	$(NCURSES_CFLAGS) \
	$(LUA_CFLAGS) \
	-D_GNU_SOURCE=1 \
	-DVERSION_MAJOR=$(VERSION_MAJOR) \
	-DVERSION_MINOR=$(VERSION_MINOR) \
	-DPATCHLEVEL=$(PATCHLEVEL) \
	-DX50NG_DATADIR=\"$(DATADIR)\"

# What if this is MacOSX
COCOA_LIBS=$(shell if [ "`uname -s`" = "Darwin" ]; then echo "-F/System/Library/Frameworks -framework Cocoa -framework IOKit"; fi)

X50NG_LDFLAGS = $(LDFLAGS)
X50NG_LDLIBS = $(QEMU_OBJS) $(GDB_LIBS) $(COCOA_LIBS) $(GTK_LDLIBS) $(NCURSES_LDLIBS) $(LUA_LDLIBS)

ifeq ($(DEBUG), yes)
	X50NG_CFLAGS += $(X50NG_DEBUG)
	X50NG_LDFLAGS += -g -pg
endif

SRCS = \
	./src/s3c2410/s3c2410.c \
	./src/s3c2410/s3c2410_sram.c \
	./src/s3c2410/s3c2410_memc.c \
	./src/s3c2410/s3c2410_intc.c \
	./src/s3c2410/s3c2410_power.c \
	./src/s3c2410/s3c2410_lcd.c \
	./src/s3c2410/s3c2410_nand.c \
	./src/s3c2410/s3c2410_uart.c \
	./src/s3c2410/s3c2410_timer.c \
	./src/s3c2410/s3c2410_usbdev.c \
	./src/s3c2410/s3c2410_watchdog.c \
	./src/s3c2410/s3c2410_io_port.c \
	./src/s3c2410/s3c2410_rtc.c \
	./src/s3c2410/s3c2410_adc.c \
	./src/s3c2410/s3c2410_spi.c \
	./src/s3c2410/s3c2410_sdi.c \
	./src/s3c2410/s3c2410_arm.c \
	./src/s3c2410/block.c \
	./src/module.c \
	./src/flash.c \
	./src/sram.c \
	./src/gui.c \
	./src/tui.c \
	./src/ui.c \
	./src/timer.c \
	./src/gdbstub.c \
	./src/options.c \
	./src/main.c

OBJS = $(SRCS:.c=.o)

# TEMPO hack
VVFATOBJS =	\
	$(QEMU_DIR)/cutils.o \
	./src/s3c2410/block-vvfat.o \
	./src/s3c2410/block-qcow.o \
	./src/s3c2410/block-raw.o

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

# Compilation of qemu
$(QEMU_DIR)/config-host.h:
	+( cd $(QEMU_DIR); \
	./configure-small; \
	$(MAKE) -f Makefile-small )

$(QEMU_OBJS): qemu-objs

qemu-objs: $(QEMU_DIR)/config-host.h
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
	rm -f ./src/*.o ./src/s3c2410/*.o core *~ .depend

distclean: clean clean-qemu
	rm -f compile_commands.json
	rm -f dist/$(TARGET) dist/$(TARGET).desktop dist/$(TARGET).man

mrproper: distclean
	make -C dist/firmware/ mrproper

# auto-format code
pretty-code:
	clang-format -i ./src/*.c ./src/*.h ./src/s3c2410/*.c $(shell ls ./src/s3c2410/*.h | grep -v s3c2410.h) ## s3c2410.h triggers an error

# Populate dist/firmware/ from hpcalc.org
pull-firmware:
	make -C dist/firmware/

# Installation
dist/$(TARGET).desktop: src/$(TARGET).desktop.in
	perl -p -e "s!TARGET!$(TARGET)!" < $^ >$@

dist/$(TARGET).man: src/$(TARGET).scd
	scdoc < $^ > $@

install: all dist/$(TARGET).desktop dist/$(TARGET).man
	mkdir -p "$(DESTDIR)$(BINDIR)/"
	install -D -m 755 dist/$(TARGET) "$(DESTDIR)$(BINDIR)/$(TARGET)"
	mkdir -p "$(DESTDIR)$(MENUDIR)/"
	install -D -m 644 dist/$(TARGET).desktop "$(DESTDIR)$(MENUDIR)/$(TARGET).desktop"
	mkdir -p "$(DESTDIR)$(MANDIR)/"
	install -D -m 644 dist/$(TARGET).man "$(DESTDIR)$(MANDIR)/$(TARGET).1"
	mkdir -p "$(DESTDIR)$(DATADIR)/"
	cp -R dist/firmware/ "$(DESTDIR)$(DATADIR)/firmware"
	cp dist/*.css "$(DESTDIR)$(DATADIR)/"
	mkdir -p "$(DESTDIR)$(DOCDIR)/"
	cp -R docs/ README.* screenshot*.png "$(DESTDIR)$(DOCDIR)/"
