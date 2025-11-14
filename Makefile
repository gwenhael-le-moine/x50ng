WITH_SDL = yes
WITH_SDL2 = no

TARGET = x50ng

VERSION_MAJOR = 2
VERSION_MINOR = 4
PATCHLEVEL = 1

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

GTK_CFLAGS = -DHAS_GTK=1 $(shell "$(PKG_CONFIG)" --cflags gtk4)
GTK_LDLIBS = $(shell "$(PKG_CONFIG)" --libs gtk4)
GTK_SRC = src/ui4x/gtk.c
GTK_HEADERS = src/ui4x/gtk.h

### Text UI
NCURSES_CFLAGS = $(shell "$(PKG_CONFIG)" --cflags ncursesw) -DNCURSES_WIDECHAR=1 -DHAS_NCURSES=1
NCURSES_LDLIBS = $(shell "$(PKG_CONFIG)" --libs ncursesw)
NCURSES_SRC = src/ui4x/ncurses.c
NCURSES_HEADERS = src/ui4x/ncurses.h

### SDL UI
ifeq ($(WITH_SDL), yes)
	SDL_CFLAGS = $(shell "$(PKG_CONFIG)" --cflags sdl3) -DHAS_SDL=1
	SDL_LDLIBS = $(shell "$(PKG_CONFIG)" --libs sdl3)
	SDL_SRC = src/ui4x/sdl.c
	SDL_HEADERS = src/ui4x/sdl.h
endif

ifeq ($(WITH_SDL2), yes)
	SDL_CFLAGS = $(shell "$(PKG_CONFIG)" --cflags sdl2) -DHAS_SDL=1 -DHAS_SDL2=1
	SDL_LDLIBS = $(shell "$(PKG_CONFIG)" --libs sdl2)
	SDL_SRC = src/ui4x/sdl.c
	SDL_HEADERS = src/ui4x/sdl.h
endif

# Embedded qemu
QEMU_CFLAGS = \
	-DTARGET_ARM \
	-D_GNU_SOURCE -D_FILE_OFFSET_BITS=64 \
	-D_LARGEFILE_SOURCE \
	-DNEED_CPU_H \
	-fno-strict-aliasing

QEMU_DIR = src/qemu
QEMU_INCLUDES = \
	-I$(QEMU_DIR) \
	-I$(QEMU_DIR)/target-arm \
	-I$(QEMU_DIR)/fpu \
	-I$(QEMU_DIR)/arm-softmmu

QEMU_SRCS = \
	$(QEMU_DIR)/arm-softmmu/exec.c \
	$(QEMU_DIR)/arm-softmmu/translate-all.c \
	$(QEMU_DIR)/arm-softmmu/cpu-exec.c \
	$(QEMU_DIR)/arm-softmmu/translate.c \
	$(QEMU_DIR)/arm-softmmu/fpu/softfloat.c \
	$(QEMU_DIR)/arm-softmmu/op_helper.c \
	$(QEMU_DIR)/arm-softmmu/helper.c \
	$(QEMU_DIR)/arm-softmmu/disas.c \
	$(QEMU_DIR)/arm-softmmu/i386-dis.c \
	$(QEMU_DIR)/arm-softmmu/arm-dis.c \
	$(QEMU_DIR)/arm-softmmu/tcg/tcg.c \
	$(QEMU_DIR)/arm-softmmu/iwmmxt_helper.c \
	$(QEMU_DIR)/arm-softmmu/neon_helper.c
QEMU_OBJS = $(QEMU_SRCS:.c=.o)

# TEMPO hack
VVFAT_SRCS =	\
	$(QEMU_DIR)/cutils.c \
	src/s3c2410/block-vvfat.c \
	src/s3c2410/block-qcow.c \
	src/s3c2410/block-raw.c
VVFAT_OBJS = $(VVFAT_SRCS:.c=.o)

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
	-DDEBUG_HDW_TIMER_IDLE \
	-DDEBUG_HDW_ARM_IDLE \
	-DDEBUG_X50NG_ENABLE_IRQ \
	-DDEBUG_X50NG_BLOCK \
	-DDEBUG_X50NG_MAIN

X50NG_INCLUDES = \
	-Isrc/ \
	-Isrc/s3c2410/ \
	-I${QEMU_DIR}/ \
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
	-std=gnu2x \
	$(X50NG_DISABLED_WARNINGS) \
	$(DEBUG_CFLAGS) \
	$(X50NG_INCLUDES) \
	$(QEMU_CFLAGS) \
	$(GTK_CFLAGS) \
	$(SDL_CFLAGS) \
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
X50NG_LDLIBS = $(QEMU_OBJS) $(GDB_LIBS) $(COCOA_LIBS) $(GTK_LDLIBS) $(SDL_LDLIBS) $(NCURSES_LDLIBS) $(LUA_LDLIBS) -lz -lm

ifeq ($(DEBUG), yes)
	X50NG_CFLAGS += $(X50NG_DEBUG)
	X50NG_LDFLAGS += -g -pg
endif

SRCS = \
	src/s3c2410/block.c \
	src/s3c2410/s3c2410.c \
	src/s3c2410/s3c2410_adc.c \
	src/s3c2410/s3c2410_arm.c \
	src/s3c2410/s3c2410_intc.c \
	src/s3c2410/s3c2410_io_port.c \
	src/s3c2410/s3c2410_lcd.c \
	src/s3c2410/s3c2410_memc.c \
	src/s3c2410/s3c2410_nand.c \
	src/s3c2410/s3c2410_power.c \
	src/s3c2410/s3c2410_rtc.c \
	src/s3c2410/s3c2410_sdi.c \
	src/s3c2410/s3c2410_spi.c \
	src/s3c2410/s3c2410_sram.c \
	src/s3c2410/s3c2410_timer.c \
	src/s3c2410/s3c2410_uart.c \
	src/s3c2410/s3c2410_usbdev.c \
	src/s3c2410/s3c2410_watchdog.c \
	src/ui4x/bitmaps_misc.c \
	src/ui4x/fonts.c \
	src/ui4x/48sx.c \
	src/ui4x/48gx.c \
	src/ui4x/49g.c \
	src/ui4x/50g.c \
	src/ui4x/api.c \
	src/for_qemu.c \
	src/hdw.c \
	src/emulator_api.c \
	src/flash.c \
	src/gdbstub.c \
	src/main.c \
	src/module.c \
	src/options.c \
	src/sram.c \
	src/timer.c \
	$(GTK_SRC) \
	$(NCURSES_SRC) \
	$(SDL_SRC)

OBJS = $(SRCS:.c=.o)

all: do-it-all

ifeq (.depend,$(wildcard .depend))
include .depend
do-it-all: $(QEMU_DIR) dist/$(TARGET)
else
do-it-all: depend-and-build
endif

# Compilation of qemu
$(QEMU_DIR)/config-host.h:
	+( cd $(QEMU_DIR); \
		 ./configure-small; )
	$(MAKE) -C $(QEMU_DIR) -f Makefile-small

qemu-objs: $(QEMU_DIR)/config-host.h
	+$(MAKE) -C $(QEMU_DIR) -f Makefile-small

# Compilation
%.o: %.c
	$(CC) $(X50NG_CFLAGS) -o $@ -c $<


# Depend
MAKEDEPEND = $(CC) -MM

depend-libs: $(QEMU_DIR)/config-host.h

depend: depend-libs
	$(MAKEDEPEND) $(X50NG_CFLAGS) $(SRCS) > .depend

depend-and-build: depend
	$(MAKE) -C . all


# for clangd
compile_commands.json: distclean $(QEMU_DIR)/config-host.h
	bear -- make

# auto-format code
pretty-code:
	clang-format -i src/*.c src/*.h src/ui4x/*.c src/ui4x/*.h src/s3c2410/*.c $(shell ls src/s3c2410/*.h | grep -v s3c2410.h) ## s3c2410.h triggers an error


# Cleaning
clean-qemu:
	$(MAKE) -C $(QEMU_DIR) -f Makefile-small clean

distclean-qemu:
	$(MAKE) -C $(QEMU_DIR) -f Makefile-small distclean

clean: clean-qemu
	rm -f core *~ .depend
	rm -f $(OBJS) $(VVFAT_OBJS) $(QEMU_OBJS)

distclean: clean distclean-qemu
	rm -f compile_commands.json
	rm -f dist/$(TARGET) dist/$(TARGET).desktop dist/$(TARGET).man
	find . -name \*.o -exec  rm {} \;

mrproper: distclean
	make -C dist/firmware/ mrproper


# Populate dist/
dist/$(TARGET): qemu-objs $(OBJS) $(VVFAT_OBJS) $(QEMU_DIR)/config-host.h
	$(CC) $(X50NG_LDFLAGS) -o $@ $(OBJS) $(VVFAT_OBJS) $(LDLIBS) $(X50NG_LDLIBS)

dist/$(TARGET).desktop: src/$(TARGET).desktop.in
	perl -p -e "s!TARGET!$(TARGET)!" < $^ >$@

dist/$(TARGET).man: src/$(TARGET).scd
	scdoc < $^ > $@

# Populate dist/firmware/ from hpcalc.org
pull-firmware:
	make -C dist/firmware/


# Installation
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

uninstall:
	rm -f "$(DESTDIR)$(BINDIR)/$(TARGET)"
	rm -f "$(DESTDIR)$(MENUDIR)/$(TARGET).desktop"
	rm -f "$(DESTDIR)$(MANDIR)/$(TARGET).1"
	rm -fr "$(DESTDIR)$(DATADIR)/"
	rm -fr "$(DESTDIR)$(DOCDIR)/"
