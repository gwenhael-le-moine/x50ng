Quick Start Guide (2024-10-24)

Taken over by gwh, originally by Egan Ford <egan@sense.net>

NOTE: READ ALL INSTRUCTIONS

Prereqs:

* MacOS 10.12 64-bit:

  * Install XQuartz, Xcode (from your installation media) in that order.
  * Install Macports (macports.org), then:

```
  sudo port install gtk2 pkgconfig gcc6
  sudo port select --set gcc mp-gcc6
  export PATH=/opt/local/bin:$PATH
```

* Ubuntu:

```
  sudo apt-get install git libgtk2.0-dev
```

* RedHat/CentOS, Fedora:

```
  sudo yum install git gtk2-devel
```

* Arch:

```
  sudo pacman -S git gtk2
```

------------------------------------------------------------------------

Start up X11 and use xterm

------------------------------------------------------------------------

Download x49gpng source:

```
git clone https://github.com/gwenhael-le-moine/x49gpng.git
```

------------------------------------------------------------------------

Build:

```
cd x49gpng
make
```

------------------------------------------------------------------------

Install (optional):

```
make install
```

------------------------------------------------------------------------

Run:

```
./dist/x49gpng
```

When installed, there should be an applications menu entry to run x49gpng.
Installing also enables running it from the terminal in any directory:

```
x49gpng
```

------------------------------------------------------------------------

First launch setup

On the first launch, the calculator will be missing a firmware, forcing
the bootloader to complain and demand a fresh one.
HP's official firmwares can be found at e.g.:
https://www.hpcalc.org/hp49/pc/rom/
Some of the most popular of these are also included in x49gpng's source
directory.
Alternatively, the most up-to-date version of NewRPL can be found at:
https://hpgcc3.org/downloads/newrplfw.bin

Pick a firmware to use and store it in any directory along with its
update.scp file. The update.scp file only contains the filename of the
firmware (when renaming the firmware, make sure the new name fits into
a DOS-style 8.3 naming scheme) followed by a DOS-style linebreak, so a
missing update.scp can be rectified easily.

Right click on the screen, or press the menu key on a physical keyboard,
to open the menu, and click on "Mount SD folder".
Select the directory containing the firmware. Then, select the SD option
from the bootloader's update source menu by clicking on the virtual key
labeled "2" or by pressing the "2" key on a physical keyboard.

Now the bootloader is installing the firmware; wait until it finishes
printing hex numbers to the virtual display, then follow its prompt to
press Reset ( = F12 or the Reset entry in the menu) or Enter.
The calculator should be fully usable after this procedure.

------------------------------------------------------------------------

Do stuff, e.g.:

Restore backup:

```
BACKUP
3
->TAG
RESTORE
```

Install ARMToolbox (HPGCC2):

```
2
SETUP.BIN
3
->TAG
RCL
EVAL
```
(Right Click ON, Left Click C)

------------------------------------------------------------------------

To Exit Emulator

Use any of:

* Press F7 or F10

* Press Alt-F4 or your system's equivalent key combination

* Open the menu using a right click on the screen or the menu key, then
choose "Quit"

* Press Ctrl-C  in the launch terminal

------------------------------------------------------------------------

Start Over:

* clean slate?

```
rm -r ~/.config/x49gpng/
```

* soft reset only?

With x49gpng running, press F12, or right click on the screen and select
"Reset" from the menu.

------------------------------------------------------------------------

Debugging with x49gpng

There is a GDB interface for debugging ARM programs, e.g. HPGCC2/3
applications or replacement firmwares. To use it, start x49gpng from a
terminal with the -d option, and start arm-none-eabi-gdb with an
appropriate ELF file in another terminal. To connect to x49gpng, type in
the GDB console:

```
target remote :1234
```

------------------------------------------------------------------------

Known Limitations:

* HPGCC SD Card I/O
  - `libfsystem` unavailable.
  - `f*` calls unstable (HPGCC2)
  - `f*` calls stable (HPGCC3)

------------------------------------------------------------------------

Post fork todo-list:

* [DOING] first port `gtk+-2` code to `gtk+-3`
  - in progress, starting https://docs.gtk.org/gtk3/migrating-2to3.html#use-cairo-for-drawing
* then port `gtk+-3` code to `gtk4`
