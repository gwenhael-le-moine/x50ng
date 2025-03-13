# x50ng -- HP 50g hardware level emulator

This is my fork of x49gp, emulator of the HP 49G+/50G hardware platform. Instead of emulating the Saturn processor like most (all?) HP emulators this one goes one level deeper and emulates the actual ARM system.

**x50ng** is part of my little collection of HP calculators' emulators' that I took over maintenance :

- [x50ng](https://codeberg.org/gwh/x50ng) ← You are here
- [x48ng](https://codeberg.org/gwh/x48ng)
- [saturnng](https://codeberg.org/gwh/saturnng)
- [hpemung](https://codeberg.org/gwh/hpemung)

[ The reference public source repository is https://codeberg.org/gwh/x50ng ]

## Screenshots
![screenshot of x50ng](./screenshot.png?raw=true "screenshot of x50ng") ![screenshot of x50ng running newRPL](./screenshot-newRPL.png?raw=true "screenshot of x50ng running newRPL")

## Usage
`x50ng --help`

A full man-page is available: `man x50ng`

When running you can right-click on the display (or use your keayboard's Menu key) to open a small menu allowing you to:
- mount and unmount a directory as the SD card
- reset the calculators'
- quit

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

0. I advise you to download the necessary firmware and ROM from [hpcalc.org](https://hpcalc.org/) by running `make pull-firmware`
1. Run `sudo make install INSTALL_PREFIX=/usr` (see the Makefile to see what variables your can override.)


## Run

*A configuration and data folder is automatically created and populated in `$XDG_CONFIG_HOME/x50ng/` (typically `~/.config/x50ng/`)*

### If you have installed x50ng
#### First run
1. launch the emulator by running `./dist/x50ng --reflash-full=/usr/share/x50ng/firmware/hp4950v215/2MB_FIX/2MB_215f.bin`

### Run Locally without installation
#### First run
1. You will need to download the necessary firmware and ROM from [hpcalc.org](https://hpcalc.org/) by running `make pull-firmware`
2. launch the emulator by running `./dist/x50ng --reflash-full=./dist/firmware/hp4950v215/2MB_FIX/2MB_215f.bin`

## Styling

You can apply your own styling using a CSS file ( [gtk+3 css documentation](https://docs.gtk.org/gtk3/css-overview.html) ).
You can access gtk's UI by running `GTK_DEBUG=interactive x50ng`. In it you can see the classes and names of each components.

For reference here is a tree of the css selectors defined by the application:

- window.background
  - box#window-container
    - box#upper-left-container
      - box#display-container
        - box#annunciators-container
          - label.annunciator
        - box#lcd-container
          - GtkDrawingArea#lcd
      - box.keyboard-container#high-keyboard-container
    - box#downer-right-container
      - box.keyboard-container#low-keyboard-container
        - box.row-container
          - box.key-container
            - box.top-labels-container
              - label.label-left
              - label.label-right
            - button.key.(menu,function,arrow,enter,core,core-number,alpha,shift-left,shift-right)
              - label.label-key
            - label.label-letter

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
```

## Development

- `make` to compile with all warnings
- `make clean` and `make mrproper` to clean between compilation runs
- `make pretty-code` to format the code using `clang-format` and the provided `.clang-format`


## Post fork todo-list:

* [DONE] then port `gtk+-3` code to `gtk4`
  - https://docs.gtk.org/gtk4/migrating-3to4.html
