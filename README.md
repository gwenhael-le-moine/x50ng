# x50ng -- HP 50g hardware level emulator

This is my fork of x49gp, emulator of the HP 49G+/50G hardware platform. Instead of emulating the Saturn processor like most (all?) HP emulators this one goes one level deeper and emulates the actual ARM system.

**x50ng** is part of my little collection of HP calculators' emulators' that I took over maintenance :

- (x50ng)[https://codeberg.org/gwh/x50ng] ← You are here
- (x48ng)[https://codeberg.org/gwh/x48ng]
- (saturnng)[https://codeberg.org/gwh/saturnng]
- (hpemung)[https://codeberg.org/gwh/hpemung]

[ The reference public source repository is https://codeberg.org/gwh/x50ng ]

## Screenshots
![screenshot of x50ng](./screenshot.png?raw=true "screenshot of x50ng") ![screenshot of x50ng running newRPL](./screenshot-newRPL.png?raw=true "screenshot of x50ng running newRPL")

## Usage
`x50ng --help`


## Tips & tricks

- __arigit__: For anyone having the "slow keystroke issue" for repeated keystrokes, this is easily fixable by configuring the calculator with "300" as the KEYTIME value (the default value on a stock HP 50g is `1138`.)

To issue the KEYTIME command on the HP50g calculator, follow these steps:

   1. Enter the desired KEYTIME value (e.g., 500) on the stack
   2. Press [RIGHT-SHIFT] followed by the key to produce the right arrow (→) symbol
   3. Type "KEYTIME" using the [ALPHA] key for each letter
   4. Press [ENTER] to execute the command

Problem solved (reference: [1](https://www.site2241.net/october2008.htm) [2](https://www.hpmuseum.org/cgi-sys/cgiwrap/hpmuseum/archv016.cgi?read=107543) )


## Compilation
`make INSTALL_PREFIX=/usr`

### Dependencies
- `lua` (`luajit` also works, see `LUA_VERSION` in `Makefile`)
- `gtk+3`


## Installation

0. I advise you to download the necessary firmware and ROM from https://hpcalc.org/ by running `make pull-firmware`
1. Run `sudo make install INSTALL_PREFIX=/usr` (see the Makefile to see what variables your can override.)


## Run

*A configuration and data folder is automatically created and populated in `$XDG_CONFIG_HOME/x50ng/` (typically `~/.config/x50ng/`)*

### If you have installed x50ng
#### First run
1. launch the emulator by running `./dist/x50ng`
2. in the file chooser dialog navigate to the folder **/usr/share/x50ng/firmware/hp4950v215/2MB_FIX/** then click __Open__ (if no dialog shows up: right-click on the screen and choose _Mount SD Folder..._)
   (if prompted: in the emulator press the key _2_)
3. follow instructions if prompted, enjoy your virtual HP 50g
4. you can right-click on the screen and choose _Unmount SD_

### Run Locally without installation
#### First run
1. You will need to download the necessary firmware and ROM from (hpcalc.org)[https://hpcalc.org/] by running `make pull-firmware`
2. launch the emulator by running `./dist/x50ng`
3. in the file chooser dialog navigate to the folder **./dist/firmware/hp4950v215/2MB_FIX/** then click __Open__ (if no dialog shows up: right-click on the screen and choose _Mount SD Folder..._)
   (if prompted: in the emulator press the key _2_)
4. follow instructions if prompted, enjoy your virtual HP 50g
5. you can right-click on the screen and choose _Unmount SD_

## Styling

You can apply your own styling using a CSS file ( (gtk+3 css documentation)[https://docs.gtk.org/gtk3/css-overview.html] ).
You can access gtk's UI by running `GTK_DEBUG=interactive x50ng`. In it you can see the classes and names of each components.

For example here's my HiDPI customisation:
- in `~/.config/x50ng/config.lua` I set
```lua
zoom = 4
style = "style-gwh.css"
```

and here's `~/.config/x50ng/style-gwh.css` :
```css
@import "/usr/share/x50ng/style-50g.css";

window {
    font-size: 20px;
}
button.shift-left .label-key,
button.shift-right .label-key {
    font-size: 2em;            /* typically 2 * base font size */
}
button.core-number .label-key,
button.arrow .label-key,
.annunciator {
    font-size: 1.75em;            /* typically 1.75 * base font size */
}
.label-key {
    font-size: 1em;            /* base font size */
}
.label-left,
.label-right,
.label-letter,
.label-below {
    font-size: 0.75em;            /* typically 0.75 * base font size */
}
```

## Development

- `make` to compile with all warnings
- `make clean` and `make mrproper` to clean between compilation runs
- `make pretty-code` to format the code using `clang-format` and the provided `.clang-format`


## Post fork todo-list:

* [TODO] then port `gtk+-3` code to `gtk4`
  - https://docs.gtk.org/gtk4/migrating-3to4.html
