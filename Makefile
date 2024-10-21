# $Id: Makefile,v 1.29 2008/12/11 12:18:17 ecd Exp $
#

SRC_DIR = .

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

DEBUG = -g # -pg

IMAGE49GP = hp49g+.png
IMAGE50G = hp50g.png

QEMU_DEFINES = -DTARGET_ARM -DX49GP \
	-D_GNU_SOURCE -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE \
	-fno-strict-aliasing

# Use this to debug
# DEFINES = $(X49GP_DEBUG) $(QEMU_DEFINES)
# Use this for speed
DEFINES = $(QEMU_DEFINES)

QEMU=qemu/qemu-git

QEMUMAKE = $(shell if [ "`uname -s`" = "Linux" -a "`uname -m`" = "sun4u" ]; then echo "sparc32 $(MAKE)"; else echo "$(MAKE)"; fi)

QEMU_DIR=$(QEMU)
QEMU_DIR_BUILD=$(QEMU_DIR)/arm-softmmu
QEMU_DEFINES+=-DNEED_CPU_H
QEMU_OBJS = $(QEMU_DIR_BUILD)/exec.o $(QEMU_DIR_BUILD)/translate-all.o $(QEMU_DIR_BUILD)/cpu-exec.o $(QEMU_DIR_BUILD)/translate.o $(QEMU_DIR_BUILD)/fpu/softfloat.o $(QEMU_DIR_BUILD)/op_helper.o $(QEMU_DIR_BUILD)/helper.o $(QEMU_DIR_BUILD)/disas.o $(QEMU_DIR_BUILD)/i386-dis.o $(QEMU_DIR_BUILD)/arm-dis.o $(QEMU_DIR_BUILD)/tcg/tcg.o $(QEMU_DIR_BUILD)/iwmmxt_helper.o $(QEMU_DIR_BUILD)/neon_helper.o
X49GP_LDFLAGS =
X49GP_LIBS = $(QEMU_OBJS)
QEMU_INCDIR=$(QEMU_DIR)
QEMU_INC=-I$(QEMU_INCDIR)/target-arm -I$(QEMU_INCDIR) -I$(QEMU_INCDIR)/fpu -I$(QEMU_INCDIR)/arm-softmmu

X49GP_INCLUDES = -I$(SRC_DIR)/include -I$(SRC_DIR)/bitmaps $(QEMU_INC)

INCLUDES = $(GDB_INCLUDES) $(X49GP_INCLUDES)

INSTALL_PREFIX = /usr/local
INSTALL_BINARY_DIR = "$(INSTALL_PREFIX)"/bin
INSTALL_DATA_DIR = "$(INSTALL_PREFIX)"/share/$(TARGET)
INSTALL_DOC_DIR = "$(INSTALL_PREFIX)"/doc/$(TARGET)
INSTALL_MENU_DIR = "$(INSTALL_PREFIX)"/share/applications
INSTALL_MAN_DIR = "$(INSTALL_PREFIX)/share/man/man1"
DEFINES += -DX49GP_DATADIR=\"$(INSTALL_DATA_DIR)\"

CC = gcc
LD = $(CC)
AR = ar
RANLIB = ranlib

CC += $(shell if [ "`uname -m`" = "sparc64" -o "`uname -m`" = "sun4u" ]; then echo "-mcpu=ultrasparc -m32"; fi)

COCOA_LIBS=$(shell if [ "`uname -s`" = "Darwin" ]; then echo "-F/System/Library/Frameworks -framework Cocoa -framework IOKit"; fi)

X49GP_CFLAGS = -O2 -Wall -Werror $(DEBUG) $(INCLUDES) $(DEFINES) -Wno-error=deprecated-declarations -Wno-error=stringop-overflow= -Wno-error=array-bounds
X49GP_LDFLAGS += $(DEBUG) $(GDB_LDFLAGS)
X49GP_LDLIBS = $(X49GP_LIBS) $(GDB_LIBS) $(COCOA_LIBS)

MAKEDEPEND = $(CC) -MM

X49GP_CFLAGS += $(shell pkg-config --cflags gtk+-2.0)
X49GP_LDLIBS += $(shell pkg-config --libs gtk+-2.0) -lz -lm

LIBS = $(QEMU)

SRCS = $(SRC_DIR)/main.c \
	$(SRC_DIR)/module.c \
	$(SRC_DIR)/flash.c \
	$(SRC_DIR)/sram.c \
	$(SRC_DIR)/s3c2410.c \
	$(SRC_DIR)/s3c2410_sram.c \
	$(SRC_DIR)/s3c2410_memc.c \
	$(SRC_DIR)/s3c2410_intc.c \
	$(SRC_DIR)/s3c2410_power.c \
	$(SRC_DIR)/s3c2410_lcd.c \
	$(SRC_DIR)/s3c2410_nand.c \
	$(SRC_DIR)/s3c2410_uart.c \
	$(SRC_DIR)/s3c2410_timer.c \
	$(SRC_DIR)/s3c2410_usbdev.c \
	$(SRC_DIR)/s3c2410_watchdog.c \
	$(SRC_DIR)/s3c2410_io_port.c \
	$(SRC_DIR)/s3c2410_rtc.c \
	$(SRC_DIR)/s3c2410_adc.c \
	$(SRC_DIR)/s3c2410_spi.c \
	$(SRC_DIR)/s3c2410_sdi.c \
	$(SRC_DIR)/s3c2410_arm.c \
	$(SRC_DIR)/ui.c \
	$(SRC_DIR)/timer.c \
	$(SRC_DIR)/tiny_font.c \
	$(SRC_DIR)/symbol.c \
	$(SRC_DIR)/gdbstub.c \
	$(SRC_DIR)/block.c

OBJS = $(SRCS:.c=.o)

# TEMPO hack
VVFATOBJS = $(SRC_DIR)/block-vvfat.o \
	$(SRC_DIR)/block-qcow.o \
	$(SRC_DIR)/block-raw.o

VVFATOBJS += $(QEMU_DIR)/cutils.o

TARGET = x49gp
TARGET_ALLCAPS = X49GP

all: do-it-all

ifeq (.depend,$(wildcard .depend))
include .depend
do-it-all: $(QEMU) $(TARGET)
else
do-it-all: depend-and-build
endif

$(TARGET): $(OBJS) $(VVFATOBJS) $(QEMU_OBJS)
	$(CC) $(LDFLAGS) $(X49GP_LDFLAGS) -o $@ $(OBJS) $(VVFATOBJS) $(LDLIBS) $(X49GP_LDLIBS)

install: all $(TARGET).desktop $(TARGET).man
	install -D -m 755 $(TARGET) "$(DESTDIR)$(INSTALL_BINARY_DIR)/$(TARGET)"
	install -D -m 644 $(IMAGE49GP) "$(DESTDIR)$(INSTALL_DATA_DIR)/$(IMAGE49GP)"
	install -D -m 644 $(IMAGE50G) "$(DESTDIR)$(INSTALL_DATA_DIR)/$(IMAGE50G)"
	install -D -m 644 pull-roms.sh "$(DESTDIR)$(INSTALL_DOC_DIR)/pull-roms.sh"
	install -D -m 644 $(TARGET).desktop "$(DESTDIR)$(INSTALL_MENU_DIR)/$(TARGET).desktop"
	install -D -m 644 $(TARGET).man "$(DESTDIR)$(INSTALL_MAN_DIR)/$(TARGET).1"

$(TARGET).desktop: x49gp.desktop.in
	perl -p -e "s!TARGET!$(TARGET)!" <x49gp.desktop.in >$@

$(TARGET).man: x49gp.man.in
	perl -p -e "s!TARGET_ALLCAPS!$(TARGET_ALLCAPS)!;" -e "s!TARGET!$(TARGET)!" <x49gp.man.in >$@

sdcard:
ifeq ($(shell uname),Darwin)
	rm -f sdcard.dmg
	hdiutil create sdcard -megabytes 64 -fs MS-DOS -volname x49gp
else
	/sbin/mkdosfs -v -C -S 512 -f 2 -F 16 -r 512 -R 2 -n "x49gp" $@ 65536
endif

sim: dummy
	$(MAKE) -C $@

$(QEMU)/config-host.h:
	+( cd $(QEMU); \
	./configure-small --extra-cflags=-DX49GP; \
	$(QEMUMAKE) -f Makefile-small )

$(QEMU_OBJS): _dir_qemu

_dir_qemu: dummy
	+$(QEMUMAKE) -C $(QEMU) -f Makefile-small

%.o: %.c
	$(CC) $(CFLAGS) $(X49GP_CFLAGS) -o $@ -c $<

block-vvfat.o: block-vvfat.c
	$(CC) $(CFLAGS) $(X49GP_CFLAGS) -fno-aggressive-loop-optimizations -o $@ -c $<

clean-qemu:
	$(MAKE) -C $(QEMU) -f Makefile-small clean

clean: clean-qemu
	rm -f *.o core *~ .depend

distclean: clean
	$(MAKE) -C $(QEMU) -f Makefile-small distclean
	rm -f $(TARGET) $(TARGET).desktop $(TARGET).man

depend-libs: $(QEMU)/config-host.h

depend-and-build: depend
	$(MAKE) -C . all

depend: depend-libs
	$(MAKEDEPEND) $(CFLAGS) $(X49GP_CFLAGS) $(SRCS) >.depend

mrproper: clean-qemu distclean

pretty-code:
	clang-format -i *.c *.h

dummy:
