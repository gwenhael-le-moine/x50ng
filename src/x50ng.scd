X50NG(1)

# NAME

x50ng — Emulator for HP 49g+ / 50g calculators

# SYNOPSIS

*x50ng* <options>

# DESCRIPTION

*x50ng* is an emulator capable of running any software written for the HP 49g+ and HP 50g calculators, including ARM programs or even custom firmware. It achieves this by using a modified version of QEMU to emulate the ARM hardware based around the Samsung S3C2410 chip.

## First launch

During the first launch, *x50ng* flashes the bootloader and firmware and generates a new configuration with default values and empty memory.

*x50ng* does ship with a default firmwares and bootloaders for the supported calculators if you run *make pull-firmware*.

Please consult HP's manual (for their official firmware) or the documentation of the custom firmware (if using one) for an explanation of using the calculator.

## Input

For mouse input, the keys shown in the window can be interacted with; left-clicking on one holds it down for as long as the mouse button is down, right-clicking on one holds it down until any key is left-clicked.

Another method of interacting with the calculator is through a physical keyboard. _A-Z_, _0-9_, _F1-F6_, _cursor keys_, _'_, _Backspace_, _^_, _+_, _-_, _\*_, _/_, _._, _Space_, _Enter_ correspond to their sensible calculator counterparts. In addition, _Delete_ acts as extra Backspace key, _,_ acts as extra decimal separator key, _Shift_ presses the Leftshift key (green on the 49g+, white on the 50g), _Control_ presses the Rightshift key (red on the 49g+, orange on the 50g), _Tab+ presses the Alpha key (yellow on the 49g+ and 50g), and _Escape_ presses the On key. Note that in order to allow multiple-key combinations involving e.g. _Shift+ without surprises, such as pressing _Shift-8_ on a US QWERTY keyboard producing the key sequence “Leftshift-hold multiply”, *x50ng* ignores all keyboard modifier states except NumLock. Because that may make certain virtual keys inaccessible to a laptop keyboard without a numpad, there are certain additional key definitions to support at least some common keyboard layouts:

US, UK, International QWERTY: _\\_ acts as \* key, _=_ acts as + key.

German QWERTZ: _#_ acts as \* key, _ß_ acts as / key.

(Further suggestions are welcome.) These additional key definitions do not need their respective layouts to work, so e.g. _\\_ will act as \* on any keyboard that has it as an unshifted key.

Pressing physical keys and clicking on virtual ones can be freely combined. Right-clicked keys are released when no keys are held down by the left mouse button or physical keys anymore. To avoid stuck keys, *x50ng* forcibly releases all keys when losing focus.

There are more remaining miscellaneous key definitions:
- _F7_ or _F10_ closes *x50ng* and saves the configuration, CPU registers, hardware registers, and memory contents to their respective files;
- _F12_ resets the calculator, like inserting a paperclip into the tiny hole at the back of the real device or removing all of its power sources (batteries, USB) would;
- _Menu_ opens a popup menu granting access to several emulator control items. This popup menu can also be opened by right-clicking on the screen area.

## The popup menu

The first group of items in the popup menu deals with the calculator's SD card. The SD card can either be backed by a directory on the host system (item *Mount SD folder...*; *x50ng* will simulate a FAT-formatted file system on the fly). The virtual SD card can be absent entirely, too (item *Unmount SD*).

The second group consists of only one item, and it is only visible when one of the debug options has been used while starting *x50ng*. Named *Start debugger*, it (re-)starts the GDB remote debugging interface. This is hidden by default because accidental clicks will seemingly freeze the emulator while it waits for an incoming connection.

The third and final group contains two items: *Reset*, which resets the calculator just like the _F12_ key, and *Quit*, which closes *x50ng* and saves the configuration, CPU registers, hardware registers, and memory contents to their respective files. This saving step is also performed when *x50ng* is closed in another way, e.g. pressing _Control-C_ in the controlling terminal.

# OPTIONS

*-h* *--help*                    print this message and exit

*-v* *--version*                 print out version

*-V* *--verbose*                 print out more information

*-d* *--datadir*[=_absolute path_] alternate datadir (default: $XDG_CONFIG_HOME/x50ng/)

*-s* *--style*[=_filename_]        css filename in <datadir> (default: style-50g.css)

*-z* *--zoom*[=_X_]                scale LCD by X (default: 2)

*--netbook*                    horizontal window (default: false)

*--netbook-pivot-line*[=_X_]     at which line is the keyboard split in netbook mode (default: 3)

*-r* *--reset*                   reboot on startup instead of continuing from the saved state in the state file

*--overwrite-config*           force writing <datadir>/config.lua even if it exists

*--newrpl-keyboard*            label keyboard for newRPL

*--legacy-keyboard*            place Enter key where it belongs

*--enable-debug*[=_port_]        enable the debugger interface (default port: 1234)

*--debug*                      use along --enable-debug to also start the debugger immediately

*--flash*                      rebuild the flash using the supplied firmware (implies -r for safety reasons)

*--flash-full*                 rebuild the flash using the supplied firmware and drop the flash contents in the area beyond the firmware (default: select one interactively) (implies -r for safety reasons)

*--bootloader*[=_bootloader_]    bootloader file to be used for flashing (default: firmware/boot-50g.bin)
(_bootloader_ is the absolute or relative path to a bootloader file. It is recommended to run `make pull-firmware

*--firmware*[=_firmware_]        firmware file to be used for flashing (default: firmware/hp4950v215/2MB_FIX/2MB_215f.bin)
(_firmware_ is the absolute or relative path to a firmware file. It is recommended to run `make pull-firmware`

# FILES

## System-wide

- *style-common.css*, *style-50g.css* and *style-49gp.css*

Default styles provided defining colors and fonts (mostly) for the GUI. The format is CSS as defined by gtk4 (documentated at _https://docs.gtk.org/gtk4/css-overview.html_ and _https://docs.gtk.org/gtk4/css-properties.html_).

- *firmware/*

This directory contains a _Makefile_ that will download and extract the following files:
- the arm bootloaders [ from _https://www.hpcalc.org/hp49/pc/rom/hp-arm-models-bootloader.zip_ ]
- the latest official HP ROM 2.15 [ from _https://www.hpcalc.org/hp49/pc/rom/hp4950v215.zip_ ]
- the latest newRPL build [ from _https://hpgcc3.org/downloads/newrplfw.bin_ ]

## All further files are located under ~/.config/x50ng/

- *config.lua*

The general configuration file of *x50ng*. Content reflecting default values (as printed by running *x50ng --print-config*):

```
--------------------------------------------------------------------------------
-- Configuration file for x50ng
-- This is a comment
style = "style-50g.css" -- CSS file (relative to this file)
zoom = 2 -- integer only
gray = false
netbook = false
netbook-pivot-line = 3
newrpl_keyboard = false -- when true this makes the keyboard labels more suited to newRPL use
legacy_keyboard = false -- when true this put the Enter key where it belongs
--- End of saturnng configuration ----------------------------------------------
```

- *state*

The default location for the configuration file. The file format is the INI-like “key-file” format defined by GLib. The configuration file stores, for the most part, the state of the calculator's hardware, excluding the memory. The peripherals' offi‐ cial names according the the S3C2410 datasheet are used as section names with the prefix “s3c2410-”, the respective register names are the keys used in a section, and the values are the numbers stored in these registers. For the ARM processor itself, the section name is s3c2410-arm, and the keys correspond to the names of attributes in the QEMU CPU state structures.

In  addition, there are keys called _filename_ in the sections _flash_, _sram_, _s3c2410-sram_, _s3c2410-sdi_ which allow changing the filename of the files where the state of the respective memory is held. These can be absolute paths, or paths rela‐ tive to the location of the configuration file (in the simplest case, only a file‐ name). Any files that do not exist will be created with appropriate size and place‐ holder content. For _s3c2410-sdi_, the empty string is accepted as a special value for no inserted SD card.

- *flash*

The default name of the file backing the flash memory; required size: 2 MiB. If this file does not exist or is too small, the calculator type's appropriate bootloader will be copied into the first 16 KiB. Do NOT simply put a firmware file here, it will not work. Just use the builtin firmware installation mechanism instead.

- *sram*

The default name of the file backing the SRAM memory; required size: 512 KiB.

- *s3c2410-sram*

The default name of the file backing the S3C2410's internal SRAM memory; required size: 4 KiB.

# CAVEATS

The emulation of the calculators' hardware is not fully accurate. Most notably, emulation speed is not authentic (it depends on the host's capabilites), and communication ports (serial, infrared, USB) are not available. For most purposes this is good enough, though.

# Tips & tricks

- For anyone having the "slow keystroke issue" for repeated keystrokes, this is easily fixable by configuring the calculator with "500" as the KEYTIME value (the default value on a stock HP 50g is `1138`.)

To issue the KEYTIME command on the HP50g calculator, follow these steps:

1. Enter the desired KEYTIME value (e.g., 500) on the stack

2. Press [RIGHT-SHIFT] followed by the key to produce the right arrow (→) symbol

3. Type "KEYTIME" using the [ALPHA] key for each letter

4. Press [ENTER] to execute the command

(reference: [1](https://www.site2241.net/october2008.htm) [2](https://www.hpmuseum.org/cgi-sys/cgiwrap/hpmuseum/archv016.cgi?read=107543) )
