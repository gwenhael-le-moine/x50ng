X50NG(1)

# NAME

x50ng — Emulator for HP 49g+ / 50g calculators

# SYNOPSIS

*x50ng* <options>

# DESCRIPTION

*x50ng* is an emulator capable of running any software written for the HP 49g+  and  HP  50g calculators,  including  ARM  programs  or even custom firmware. It achieves this by using a modified version of QEMU to emulate the ARM hardware based around the Samsung S3C2410 chip.

## Input

For  mouse  input, the keys shown in the window can be interacted with; left-clicking on one holds it down for as long as the mouse button is down, right-clicking on one holds  it  down until any key is left-clicked.

Another method of interacting with the calculator is through a physical keyboard.  _A-Z_, _0-9_, _F1-F6_,  _cursor keys_, _'_, _Backspace_, _^_, _+_, _-_, _\*_, _/_, _._, _Space_, _Enter_ correspond to their sensible calculator counterparts. In addition, _Delete_ acts as extra Backspace key, _,_ acts as  extra  decimal separator key, _Shift_ presses the Leftshift key (green on the 49g+, white on the 50g), _Control_ presses the Rightshift key (red on the 49g+, orange on the 50g),  _Tab+  presses the  Alpha key (yellow on the 49g+ and 50g), and _Escape_ presses the On key. Note that in order to allow multiple-key combinations involving e.g.   _Shift+  without  surprises,  such  as pressing  _Shift-8_  on  a  US  QWERTY  keyboard  producing the key sequence “Leftshift-hold multiply”, *x50ng* ignores all keyboard modifier states except  NumLock.  Because  that  may make certain virtual keys inaccessible to a laptop keyboard without a numpad, there are certain additional key definitions to support at least some common keyboard layouts:

US, UK, International QWERTY: _\\_ acts as \* key, _=_ acts as + key.

German QWERTZ: _#_ acts as \* key, _ß_ acts as / key.

(Further  suggestions  are  welcome.) These additional key definitions do not need their respective layouts to work, so e.g.  _\\_ will act as \* on any keyboard that has  it  as  an  unshifted key.

Pressing  physical  keys  and clicking on virtual ones can be freely combined. Right-clicked keys are released when no keys are held down by the left mouse button or physical keys  anymore. To avoid stuck keys, *x50ng* forcibly releases all keys when losing focus.

There  are  more remaining miscellaneous key definitions:
- _F7_ or _F10_ closes *x50ng* and saves the configuration, CPU registers, hardware registers, and memory contents to their respective files;
- _F12_ resets the calculator, like inserting a paperclip into the tiny hole at the back of the real device or removing all of its power sources (batteries, USB) would;
- _Menu_ opens a popup menu granting access to several emulator control items. This popup menu can also be opened by  right-clicking  on  the  screen area.

## The popup menu

The  first group of items in the popup menu deals with the calculator's SD card. The SD card can either be backed by a directory on the host system (item *Mount SD  folder...*;  *x50ng* will  simulate  a  FAT-formatted file system on the fly), or by an image file (item *Mount SD image...*).  The former is more convenient for general use, but when debugging a  hypothetical  replacement  firmware's  SD driver, the latter variant's accuracy (it allows corrupting the filesystem, for instance) might be more desirable. The virtual SD card can be absent entirely, too (item *Unmount SD*).

The second group consists of only one item, and it is only visible when one of the debug options has been used while starting *x50ng*.  Named *Start debugger*, it  (re-)starts  the  GDB remote  debugging  interface. This is hidden by default because accidental clicks will seemingly freeze the emulator while it waits for an incoming connection.

The third and final group contains two items: *Reset*, which resets the calculator  just  like the  _F12_  key,  and  *Quit*,  which closes *x50ng* and saves the configuration, CPU registers, hardware registers, and memory contents to their respective files. This saving step is  also performed  when *x50ng* is closed in another way, e.g. pressing _Control-C_ in the controlling terminal.

## First launch

During the first launch, *x50ng* generates a new configuration with default values and empty memory.  *x50ng* does not ship with a default firmware, only with the  bootloaders  for  the supported  calculators.  Hence,  the  user is required to select a firmware to load into the flash.  *x50ng* will show a file selection window for that task. For the stock  firmware  HP has  always  used the file extension “.bin”, any other files (including the update.scp files that typically come with them!) are generally not actual firmware files.

When the file selection dialog is cancelled or the file could not be read, *x50ng* creates a flash without firmware; the bootloader will complain and offer ways to correct the situation just like it would on a real device, which include receiving a firmware via  USB  (currently not  available  in  *x50ng*,  see _Caveats_ below) or loading one from the SD card. Apart from these options, it's also possible to recover from this by exiting *x50ng* and either  deleting  the  flash  file  or using one of the options *-f* and *-F*, causing *x50ng* to rebuild the flash and ask for a firmware again.

Please consult HP's manual (for their official firmware) or the documentation of the  custom firmware (if using one) for an explanation of using the calculator.

# OPTIONS

*-h* *--help*                    print this message and exit
*-v* *--version*                 print out version
*-V* *--verbose*                 print out more information
*-d* *--datadir*[=_absolute path_] alternate datadir (default: $XDG_CONFIG_HOME/x50ng/)
*-n* *--name*[=_name_]             set alternate UI name
*-s* *--style*[=_filename_]        css filename in <datadir> (default: style-50g.css)
*-z* *--zoom*[=_X_]                scale LCD by X (default: 2)
*-r* *--reset*                   reboot on startup instead of continuing from the saved state in the state file
   *--overwrite-config*        force writing <datadir>/config.lua even if it exists

   *--newrpl-keyboard*         label keyboard for newRPL

   *--enable-debug*[=_port_]  enable the debugger interface (default port: 1234)
   *--debug*                use along -D to also start the debugger immediately

   *--reflash*[=_firmware_]   rebuild the flash using the supplied firmware (default: select one interactively) (implies -r for safety reasons)
   *--reflash-full*[=_firmware_] rebuild the flash using the supplied firmware and drop the flash contents in the area beyond the firmware (default: select one interactively) (implies -r for safety reasons)
   *--50g*                     use an HP 50g bootloader when (re-)flashing (default)
   *--49gp*                    use an HP 49g+ bootloader when (re-)flashing

# FILES

## System-wide

- *style-50g.css* and *style-49gp.css*

Default styles provided defining colors and fonts (mostly) for the GUI. The format is CSS as defined by gtk+-3.

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
name = "HP 50g" -- this customize the title of the window
model = "50g" -- possible values: "49gp", "50g". Changes the colors and the bootloader looked for when (re-)flashing
newrpl_keyboard = false -- when true this makes the keyboard labels more suited to newRPL use
style_filename = "style-50g.css" -- CSS file (relative to this file)
display_scale = 2 -- integer only
verbose = false
--- End of saturnng configuration ----------------------------------------------
```

- *state*

The  default  location  for  the configuration file. The file format is the INI-like “key-file” format defined by GLib. The configuration file stores, for the most part, the state of the calculator's hardware, excluding the memory. The peripherals' offi‐ cial names according the the S3C2410 datasheet are used as section  names  with  the prefix “s3c2410-”, the respective register names are the keys used in a section, and the  values are the numbers stored in these registers. For the ARM processor itself, the section name is s3c2410-arm, and the keys correspond to the names of  attributes in the QEMU CPU state structures.

In   addition,  there  are  keys  called  _filename_  in  the  sections  _flash_,  _sram_, _s3c2410-sram_, _s3c2410-sdi_ which allow changing the filename of the files  where  the state  of the respective memory is held. These can be absolute paths, or paths rela‐ tive to the location of the configuration file (in the simplest case, only  a  file‐ name).  Any files that do not exist will be created with appropriate size and place‐ holder content. For _s3c2410-sdi_, the empty string is accepted as a special value for no inserted SD card.

- *flash*

The default name of the file backing the flash memory; required size: 2 MiB. If this file does not exist or is too small, the calculator  type's  appropriate  bootloader will  be  copied  into  the first 16 KiB. Do NOT simply put a firmware file here, it will not work. Just use the builtin firmware installation mechanism instead.

- *sram*

The default name of the file backing the SRAM memory; required size: 512 KiB.

- *s3c2410-sram*

The default name of the file backing the S3C2410's internal  SRAM  memory;  required size: 4 KiB.

# CAVEATS

The emulation of the calculators' hardware is not fully accurate. Most notably, emulation speed is not authentic (it depends on the host's capabilites), and communication ports (serial, infrared, USB) are not available. For most purposes this is good enough, though.
