#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <sys/times.h>
#include <math.h>
#include <errno.h>
#include <arpa/inet.h> /* For ntohl() */

#include <gtk/gtk.h>
#include <glib.h>
#include <cairo.h>
#include <gdk/gdkkeysyms.h>

#include "options.h"
#include "x49gp.h"
#include "ui.h"
#include "s3c2410.h"

#include "gdbstub.h"

#define NB_KEYS 51

#define FONT_SIZE_SYMBOL 28
#define FONT_SIZE_NUMBER 20
#define FONT_SIZE_KEY 12
#define FONT_SIZE_TINY 8

#define TINY_TEXT_HEIGHT ( FONT_SIZE_TINY + 2 )
#define TINY_TEXT_WIDTH ( TINY_TEXT_HEIGHT / 2 )

#define KB_WIDTH_6_KEYS 36
#define KB_WIDTH_5_KEYS 46

#define KB_WIDTH_6_KEYS 36
#define KB_WIDTH_5_KEYS 46

#define KB_HEIGHT_MENU_KEYS 22
#define KB_HEIGHT_SMALL_KEYS 28
#define KB_HEIGHT_BIG_KEYS 32

#define KB_LINE_HEIGHT ( KB_HEIGHT_BIG_KEYS + ( 1.5 * ( TINY_TEXT_HEIGHT + 2 ) ) )
#define KB_SPACING_KEYS ( 3 * TINY_TEXT_WIDTH )
#define KB_COLUMN_WIDTH_6_KEYS ( KB_WIDTH_6_KEYS + KB_SPACING_KEYS )
#define KB_COLUMN_WIDTH_5_KEYS ( KB_WIDTH_5_KEYS + KB_SPACING_KEYS )

#define ANNUNCIATOR_WIDTH 16
#define ANNUNCIATOR_HEIGHT 16
#define ANNUNCIATORS_HEIGHT ANNUNCIATOR_HEIGHT

#define LCD_PIXEL_SCALE 2
#define LCD_WIDTH ( 131 * LCD_PIXEL_SCALE )
#define LCD_HEIGHT ( 80 * LCD_PIXEL_SCALE )

#define KEYBOARD_PADDING ( TINY_TEXT_HEIGHT + 2 )
#define KEYBOARD_WIDTH ( ui_keys[ NB_KEYS - 1 ].x + ui_keys[ NB_KEYS - 1 ].width )

#define WINDOW_WIDTH ( ( 2 * KEYBOARD_PADDING ) + KEYBOARD_WIDTH )

#define LCD_PADDING ( ( WINDOW_WIDTH - LCD_WIDTH ) / 2 )

static x49gp_ui_key_t ui_keys[ NB_KEYS ] = {
    {.css_class = "button-menu",
     .label = "F1",
     .letter = "A",
     .left = "Y=",
     .right = NULL,
     .below = NULL,
     .x = 0,
     .y = 0,
     .width = KB_WIDTH_6_KEYS,
     .height = KB_HEIGHT_MENU_KEYS,
     .column = 5,
     .row = 1,
     .columnbit = ( 1 << 5 ),
     .rowbit = ( 1 << 1 ),
     .eint = 1},
    {.css_class = "button-menu",
     .label = "F2",
     .letter = "B",
     .left = "WIN",
     .right = NULL,
     .below = NULL,
     .x = KB_COLUMN_WIDTH_6_KEYS,
     .y = 0,
     .width = KB_WIDTH_6_KEYS,
     .height = KB_HEIGHT_MENU_KEYS,
     .column = 5,
     .row = 2,
     .columnbit = ( 1 << 5 ),
     .rowbit = ( 1 << 2 ),
     .eint = 2},
    {.css_class = "button-menu",
     .label = "F3",
     .letter = "C",
     .left = "GRAPH",
     .right = NULL,
     .below = NULL,
     .x = 2 * KB_COLUMN_WIDTH_6_KEYS,
     .y = 0,
     .width = KB_WIDTH_6_KEYS,
     .height = KB_HEIGHT_MENU_KEYS,
     .column = 5,
     .row = 3,
     .columnbit = ( 1 << 5 ),
     .rowbit = ( 1 << 3 ),
     .eint = 3},
    {.css_class = "button-menu",
     .label = "F4",
     .letter = "D",
     .left = "2D/3D",
     .right = NULL,
     .below = NULL,
     .x = 3 * KB_COLUMN_WIDTH_6_KEYS,
     .y = 0,
     .width = KB_WIDTH_6_KEYS,
     .height = KB_HEIGHT_MENU_KEYS,
     .column = 5,
     .row = 4,
     .columnbit = ( 1 << 5 ),
     .rowbit = ( 1 << 4 ),
     .eint = 4},
    {.css_class = "button-menu",
     .label = "F5",
     .letter = "E",
     .left = "TBLSET",
     .right = NULL,
     .below = NULL,
     .x = 4 * KB_COLUMN_WIDTH_6_KEYS,
     .y = 0,
     .width = KB_WIDTH_6_KEYS,
     .height = KB_HEIGHT_MENU_KEYS,
     .column = 5,
     .row = 5,
     .columnbit = ( 1 << 5 ),
     .rowbit = ( 1 << 5 ),
     .eint = 5},
    {.css_class = "button-menu",
     .label = "F6",
     .letter = "F",
     .left = "TABLE",
     .right = NULL,
     .below = NULL,
     .x = 5 * KB_COLUMN_WIDTH_6_KEYS,
     .y = 0,
     .width = KB_WIDTH_6_KEYS,
     .height = KB_HEIGHT_MENU_KEYS,
     .column = 5,
     .row = 6,
     .columnbit = ( 1 << 5 ),
     .rowbit = ( 1 << 6 ),
     .eint = 6},

    {.css_class = "button-function",
     .label = "APPS",
     .letter = "G",
     .left = "FILES",
     .right = "BEGIN",
     .below = NULL,
     .x = 0,
     .y = KB_LINE_HEIGHT,
     .width = KB_WIDTH_6_KEYS,
     .height = KB_HEIGHT_SMALL_KEYS,
     .column = 5,
     .row = 7,
     .columnbit = ( 1 << 5 ),
     .rowbit = ( 1 << 7 ),
     .eint = 7},
    {.css_class = "button-function",
     .label = "MODE",
     .letter = "H",
     .left = "CUSTOM",
     .right = "END",
     .below = NULL,
     .x = KB_COLUMN_WIDTH_6_KEYS,
     .y = KB_LINE_HEIGHT,
     .width = KB_WIDTH_6_KEYS,
     .height = KB_HEIGHT_SMALL_KEYS,
     .column = 6,
     .row = 5,
     .columnbit = ( 1 << 6 ),
     .rowbit = ( 1 << 5 ),
     .eint = 5},
    {.css_class = "button-function",
     .label = "TOOL",
     .letter = "I",
     .left = "i",
     .right = "I",
     .below = NULL,
     .x = 2 * KB_COLUMN_WIDTH_6_KEYS,
     .y = KB_LINE_HEIGHT,
     .width = KB_WIDTH_6_KEYS,
     .height = KB_HEIGHT_SMALL_KEYS,
     .column = 6,
     .row = 6,
     .columnbit = ( 1 << 6 ),
     .rowbit = ( 1 << 6 ),
     .eint = 6},
    {.css_class = "button-function",
     .label = "VAR",
     .letter = "J",
     .left = "UPDIR",
     .right = "COPY",
     .below = NULL,
     .x = 0,
     .y = 2 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_6_KEYS,
     .height = KB_HEIGHT_SMALL_KEYS,
     .column = 6,
     .row = 7,
     .columnbit = ( 1 << 6 ),
     .rowbit = ( 1 << 7 ),
     .eint = 7},
    {.css_class = "button-function",
     .label = "STO⏵",
     .letter = "K",
     .left = "RCL",
     .right = "CUT",
     .below = NULL,
     .x = KB_COLUMN_WIDTH_6_KEYS,
     .y = 2 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_6_KEYS,
     .height = KB_HEIGHT_SMALL_KEYS,
     .column = 7,
     .row = 1,
     .columnbit = ( 1 << 7 ),
     .rowbit = ( 1 << 1 ),
     .eint = 1},
    {.css_class = "button-function",
     .label = "NXT",
     .letter = "L",
     .left = "PREV",
     .right = "PASTE",
     .below = NULL,
     .x = 2 * KB_COLUMN_WIDTH_6_KEYS,
     .y = 2 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_6_KEYS,
     .height = KB_HEIGHT_SMALL_KEYS,
     .column = 7,
     .row = 2,
     .columnbit = ( 1 << 7 ),
     .rowbit = ( 1 << 2 ),
     .eint = 2},

    {.css_class = "button-arrow",
     .label = "▲",
     .letter = NULL,
     .left = NULL,
     .right = NULL,
     .below = NULL,
     .x = 4 * KB_COLUMN_WIDTH_6_KEYS,
     .y = 0.75 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_6_KEYS,
     .height = KB_HEIGHT_BIG_KEYS,
     .column = 6,
     .row = 1,
     .columnbit = ( 1 << 6 ),
     .rowbit = ( 1 << 1 ),
     .eint = 1},
    {.css_class = "button-arrow",
     .label = "◀",
     .letter = NULL,
     .left = NULL,
     .right = NULL,
     .below = NULL,
     .x = 3.25 * KB_COLUMN_WIDTH_6_KEYS,
     .y = 1.25 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_6_KEYS,
     .height = KB_HEIGHT_BIG_KEYS,
     .column = 6,
     .row = 2,
     .columnbit = ( 1 << 6 ),
     .rowbit = ( 1 << 2 ),
     .eint = 2},
    {.css_class = "button-arrow",
     .label = "▼",
     .letter = NULL,
     .left = NULL,
     .right = NULL,
     .below = NULL,
     .x = 4 * KB_COLUMN_WIDTH_6_KEYS,
     .y = 1.75 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_6_KEYS,
     .height = KB_HEIGHT_BIG_KEYS,
     .column = 6,
     .row = 3,
     .columnbit = ( 1 << 6 ),
     .rowbit = ( 1 << 3 ),
     .eint = 3},
    {.css_class = "button-arrow",
     .label = "▶",
     .letter = NULL,
     .left = NULL,
     .right = NULL,
     .below = NULL,
     .x = 4.78 * KB_COLUMN_WIDTH_6_KEYS,
     .y = 1.25 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_6_KEYS,
     .height = KB_HEIGHT_BIG_KEYS,
     .column = 6,
     .row = 4,
     .columnbit = ( 1 << 6 ),
     .rowbit = ( 1 << 4 ),
     .eint = 4},

    {.css_class = "button-function",
     .label = "HIST",
     .letter = "M",
     .left = "CMD",
     .right = "UNDO",
     .below = NULL,
     .x = 0,
     .y = 3 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_SMALL_KEYS,
     .column = 4,
     .row = 1,
     .columnbit = ( 1 << 4 ),
     .rowbit = ( 1 << 1 ),
     .eint = 1},
    {.css_class = "button-function",
     .label = "EVAL",
     .letter = "N",
     .left = "PRG",
     .right = "CHARS",
     .below = NULL,
     .x = KB_COLUMN_WIDTH_5_KEYS,
     .y = 3 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_SMALL_KEYS,
     .column = 3,
     .row = 1,
     .columnbit = ( 1 << 3 ),
     .rowbit = ( 1 << 1 ),
     .eint = 1},
    {.css_class = "button-function",
     .label = "'",
     .letter = "O",
     .left = "MTRW",
     .right = "EQW",
     .below = NULL,
     .x = 2 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 3 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_SMALL_KEYS,
     .column = 2,
     .row = 1,
     .columnbit = ( 1 << 2 ),
     .rowbit = ( 1 << 1 ),
     .eint = 1},
    {.css_class = "button-function",
     .label = "SYMB",
     .letter = "P",
     .left = "MTH",
     .right = "CAT",
     .below = NULL,
     .x = 3 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 3 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_SMALL_KEYS,
     .column = 1,
     .row = 1,
     .columnbit = ( 1 << 1 ),
     .rowbit = ( 1 << 1 ),
     .eint = 1},
    {.css_class = "button-function",
     .label = "⬅",
     .letter = NULL,
     .left = "DEL",
     .right = "CLEAR",
     .below = NULL,
     .x = 4 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 3 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_SMALL_KEYS,
     .column = 0,
     .row = 1,
     .columnbit = ( 1 << 0 ),
     .rowbit = ( 1 << 1 ),
     .eint = 1},

    {.css_class = "button-function",
     .label = "Y<sup>x</sup>",
     .letter = "Q",
     .left = "𝑒<sup>x</sup>",
     .right = "LN",
     .below = NULL,
     .x = 0,
     .y = 4 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_SMALL_KEYS,
     .column = 4,
     .row = 2,
     .columnbit = ( 1 << 4 ),
     .rowbit = ( 1 << 2 ),
     .eint = 2},
    {.css_class = "button-function",
     .label = "√𝔁",
     .letter = "R",
     .left = "𝔁<sup>2</sup>",
     .right = "<sup>x</sup>√y",
     .below = NULL,
     .x = KB_COLUMN_WIDTH_5_KEYS,
     .y = 4 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_SMALL_KEYS,
     .column = 3,
     .row = 2,
     .columnbit = ( 1 << 3 ),
     .rowbit = ( 1 << 2 ),
     .eint = 2},
    {.css_class = "button-function",
     .label = "SIN",
     .letter = "S",
     .left = "ASIN",
     .right = "∑",
     .below = NULL,
     .x = 2 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 4 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_SMALL_KEYS,
     .column = 2,
     .row = 2,
     .columnbit = ( 1 << 2 ),
     .rowbit = ( 1 << 2 ),
     .eint = 2},
    {.css_class = "button-function",
     .label = "COS",
     .letter = "T",
     .left = "ACOS",
     .right = "∂",
     .below = NULL,
     .x = 3 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 4 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_SMALL_KEYS,
     .column = 1,
     .row = 2,
     .columnbit = ( 1 << 1 ),
     .rowbit = ( 1 << 2 ),
     .eint = 2},
    {.css_class = "button-function",
     .label = "TAN",
     .letter = "U",
     .left = "ATAN",
     .right = "∫",
     .below = NULL,
     .x = 4 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 4 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_SMALL_KEYS,
     .column = 0,
     .row = 2,
     .columnbit = ( 1 << 0 ),
     .rowbit = ( 1 << 2 ),
     .eint = 2},

    {.css_class = "button-function",
     .label = "EEX",
     .letter = "V",
     .left = "10<sup>𝔁</sup>",
     .right = "LOG",
     .below = NULL,
     .x = 0,
     .y = 5 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_SMALL_KEYS,
     .column = 4,
     .row = 3,
     .columnbit = ( 1 << 4 ),
     .rowbit = ( 1 << 3 ),
     .eint = 3},
    {.css_class = "button-function",
     .label = "+/-",
     .letter = "W",
     .left = "≠",
     .right = "=",
     .below = NULL,
     .x = KB_COLUMN_WIDTH_5_KEYS,
     .y = 5 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_SMALL_KEYS,
     .column = 3,
     .row = 3,
     .columnbit = ( 1 << 3 ),
     .rowbit = ( 1 << 3 ),
     .eint = 3},
    {.css_class = "button-function",
     .label = "𝔁",
     .letter = "X",
     .left = "≤",
     .right = "&gt;",
     .below = NULL,
     .x = 2 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 5 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_SMALL_KEYS,
     .column = 2,
     .row = 3,
     .columnbit = ( 1 << 2 ),
     .rowbit = ( 1 << 3 ),
     .eint = 3},
    {.css_class = "button-function",
     .label = "1/𝔁",
     .letter = "Y",
     .left = "≥",
     .right = ">",
     .below = NULL,
     .x = 3 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 5 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_SMALL_KEYS,
     .column = 1,
     .row = 3,
     .columnbit = ( 1 << 1 ),
     .rowbit = ( 1 << 3 ),
     .eint = 3},
    {.css_class = "button-core-number",
     .label = "÷",
     .letter = "Z",
     .left = "ABS",
     .right = "ARG",
     .below = NULL,
     .x = 4 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 5 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_SMALL_KEYS,
     .column = 0,
     .row = 3,
     .columnbit = ( 1 << 0 ),
     .rowbit = ( 1 << 3 ),
     .eint = 3},

    {.css_class = "button-alpha",
     .label = "ALPHA",
     .letter = NULL,
     .left = "USER",
     .right = "ENTRY",
     .below = NULL,
     .x = 0,
     .y = 6 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_BIG_KEYS,
     .column = 0,
     .row = 0,
     .columnbit = 0,
     .rowbit = 0,
     .eint = 4},
    {.css_class = "button-core-number",
     .label = "7",
     .letter = NULL,
     .left = "S.SLV",
     .right = "NUM.SLV",
     .below = NULL,
     .x = KB_COLUMN_WIDTH_5_KEYS,
     .y = 6 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_BIG_KEYS,
     .column = 3,
     .row = 4,
     .columnbit = ( 1 << 3 ),
     .rowbit = ( 1 << 4 ),
     .eint = 4},
    {.css_class = "button-core-number",
     .label = "8",
     .letter = NULL,
     .left = "EXP&amp;LN",
     .right = "TRIG",
     .below = NULL,
     .x = 2 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 6 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_BIG_KEYS,
     .column = 2,
     .row = 4,
     .columnbit = ( 1 << 2 ),
     .rowbit = ( 1 << 4 ),
     .eint = 4},
    {.css_class = "button-core-number",
     .label = "9",
     .letter = NULL,
     .left = "FINANCE",
     .right = "TIME",
     .below = NULL,
     .x = 3 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 6 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_BIG_KEYS,
     .column = 1,
     .row = 4,
     .columnbit = ( 1 << 1 ),
     .rowbit = ( 1 << 4 ),
     .eint = 4},
    {.css_class = "button-core-number",
     .label = "×",
     .letter = NULL,
     .left = "[ ]",
     .right = "\" \"",
     .below = NULL,
     .x = 4 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 6 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_BIG_KEYS,
     .column = 0,
     .row = 4,
     .columnbit = ( 1 << 0 ),
     .rowbit = ( 1 << 4 ),
     .eint = 4},

    {.css_class = "button-shift-left",
     .label = "⮢",
     .letter = NULL,
     .left = NULL,
     .right = NULL,
     .below = NULL,
     .x = 0,
     .y = 7 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_BIG_KEYS,
     .column = 0,
     .row = 0,
     .columnbit = 0,
     .rowbit = 0,
     .eint = 5},
    {.css_class = "button-core-number",
     .label = "4",
     .letter = NULL,
     .left = "CALC",
     .right = "ALG",
     .below = NULL,
     .x = KB_COLUMN_WIDTH_5_KEYS,
     .y = 7 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_BIG_KEYS,
     .column = 3,
     .row = 5,
     .columnbit = ( 1 << 3 ),
     .rowbit = ( 1 << 5 ),
     .eint = 5},
    {.css_class = "button-core-number",
     .label = "5",
     .letter = NULL,
     .left = "MATRICES",
     .right = "STAT",
     .below = NULL,
     .x = 2 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 7 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_BIG_KEYS,
     .column = 2,
     .row = 5,
     .columnbit = ( 1 << 2 ),
     .rowbit = ( 1 << 5 ),
     .eint = 5},
    {.css_class = "button-core-number",
     .label = "6",
     .letter = NULL,
     .left = "CONVERT",
     .right = "UNITS",
     .below = NULL,
     .x = 3 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 7 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_BIG_KEYS,
     .column = 1,
     .row = 5,
     .columnbit = ( 1 << 1 ),
     .rowbit = ( 1 << 5 ),
     .eint = 5},
    {.css_class = "button-core-number",
     .label = "-",
     .letter = NULL,
     .left = "( )",
     .right = "_",
     .below = NULL,
     .x = 4 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 7 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_BIG_KEYS,
     .column = 0,
     .row = 5,
     .columnbit = ( 1 << 0 ),
     .rowbit = ( 1 << 5 ),
     .eint = 5},

    {.css_class = "button-shift-right",
     .label = "⮣",
     .letter = NULL,
     .left = NULL,
     .right = NULL,
     .below = NULL,
     .x = 0,
     .y = 8 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_BIG_KEYS,
     .column = 0,
     .row = 0,
     .columnbit = 0,
     .rowbit = 0,
     .eint = 6},
    {.css_class = "button-core-number",
     .label = "1",
     .letter = NULL,
     .left = "ARITH",
     .right = "CMPLX",
     .below = NULL,
     .x = KB_COLUMN_WIDTH_5_KEYS,
     .y = 8 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_BIG_KEYS,
     .column = 3,
     .row = 6,
     .columnbit = ( 1 << 3 ),
     .rowbit = ( 1 << 6 ),
     .eint = 6},
    {.css_class = "button-core-number",
     .label = "2",
     .letter = NULL,
     .left = "DEF",
     .right = "LIB",
     .below = NULL,
     .x = 2 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 8 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_BIG_KEYS,
     .column = 2,
     .row = 6,
     .columnbit = ( 1 << 2 ),
     .rowbit = ( 1 << 6 ),
     .eint = 6},
    {.css_class = "button-core-number",
     .label = "3",
     .letter = NULL,
     .left = "#",
     .right = "BASE",
     .below = NULL,
     .x = 3 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 8 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_BIG_KEYS,
     .column = 1,
     .row = 6,
     .columnbit = ( 1 << 1 ),
     .rowbit = ( 1 << 6 ),
     .eint = 6},
    {.css_class = "button-core-number",
     .label = "+",
     .letter = NULL,
     .left = "{ }",
     .right = "« »",
     .below = NULL,
     .x = 4 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 8 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_BIG_KEYS,
     .column = 0,
     .row = 6,
     .columnbit = ( 1 << 0 ),
     .rowbit = ( 1 << 6 ),
     .eint = 6},

    {.css_class = "button-core",
     .label = "ON",
     .letter = NULL,
     .left = "CONT",
     .right = "OFF",
     .below = "CANCEL",
     .x = 0,
     .y = 9 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_BIG_KEYS,
     .column = 0,
     .row = 0,
     .columnbit = 0,
     .rowbit = 0,
     .eint = 0},
    {.css_class = "button-core-number",
     .label = "0",
     .letter = NULL,
     .left = "∞",
     .right = "→",
     .below = NULL,
     .x = KB_COLUMN_WIDTH_5_KEYS,
     .y = 9 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_BIG_KEYS,
     .column = 3,
     .row = 7,
     .columnbit = ( 1 << 3 ),
     .rowbit = ( 1 << 7 ),
     .eint = 7},
    {.css_class = "button-core",
     .label = ".",
     .letter = NULL,
     .left = ": :",
     .right = "↲",
     .below = NULL,
     .x = 2 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 9 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_BIG_KEYS,
     .column = 2,
     .row = 7,
     .columnbit = ( 1 << 2 ),
     .rowbit = ( 1 << 7 ),
     .eint = 7},
    {.css_class = "button-core",
     .label = "SPC",
     .letter = NULL,
     .left = "𝚷",
     .right = ",",
     .below = NULL,
     .x = 3 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 9 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_BIG_KEYS,
     .column = 1,
     .row = 7,
     .columnbit = ( 1 << 1 ),
     .rowbit = ( 1 << 7 ),
     .eint = 7},
    {.css_class = "button-core",
     .label = "ENTER",
     .letter = NULL,
     .left = "ANS",
     .right = "→NUM",
     .below = NULL,
     .x = 4 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 9 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_BIG_KEYS,
     .column = 0,
     .row = 7,
     .columnbit = ( 1 << 0 ),
     .rowbit = ( 1 << 7 ),
     .eint = 7},
};

char* css_global = "window {"
                   "  background-color: %s;"
                   "}"
                   "window * {"
                   "  font-weight: bold;"
                   "}"
                   ".annunciator {"
                   "  padding: 0px;"
                   "  color: #080808;"
                   "}"
                   ".lcd-container, .annunciators-container {"
                   "  background-color: #abd2b4;"
                   "}"
                   "button {"
                   "  background-image: none;"
                   "  padding: 0px;"
                   "}"
                   ".button-menu {"
                   "  background-color: #a9a9a9;"
                   "}"
                   ".button-function {"
                   "  background-color: #696969;"
                   "}"
                   ".button-arrow {"
                   "  background-color: #e0e0e0;"
                   "}"
                   ".button-alpha {"
                   "  background-color: #fae82c;"
                   "}"
                   ".button-core, .button-core-number {"
                   "  background-color: #080808;"
                   "}"
                   ".button-alpha .label-key {"
                   "  color: #080808;"
                   "}"
                   ".button-shift-left {"
                   "  background-color: %s;"
                   "}"
                   ".button-shift-right {"
                   "  background-color: #8e2518;"
                   "}"
                   ".button-shift-left .label-key, .button-shift-right .label-key, .button-arrow .label-key {"
                   "  font-size: %ipx;"
                   "  color: #080808;"
                   "}"
                   ".button-core-number .label-key {"
                   "  font-size: %ipx;"
                   "}"
                   ".label-key {"
                   "  font-size: %ipx;"
                   "  color: #ffffff;"
                   "}"
                   ".label-left, .label-right, .label-letter, .label-below {"
                   "  font-size: %ipx;"
                   "}"
                   ".label-left {"
                   "  color: %s;"
                   "}"
                   ".label-right {"
                   "  color: #c06e60;"
                   "}"
                   ".label-below {"
                   "  color: #4060a4;"
                   "}"
                   ".label-letter {"
                   "  color: #fae82c;"
                   "}";

/*************/
/* functions */
/*************/

static inline int _tiny_text_width( const char* text )
{
    char* stripped_text;
    pango_parse_markup( text, -1, 0, NULL, &stripped_text, NULL, NULL );

    return strlen( stripped_text ) * TINY_TEXT_WIDTH;
}

static void key_to_button( GtkWidget* button, bool is_press )
{
    GdkEventButton event = { .type = GDK_BUTTON_PRESS, .state = 0 };
    static gboolean unused_but_needed_return_value;
    g_signal_emit_by_name( GTK_BUTTON( button ), is_press ? "button-press-event" : "button-release-event", &event,
                           &unused_but_needed_return_value );
}

static void ui_release_button( x49gp_ui_button_t* button, x49gp_ui_button_t* cause )
{
    x49gp_t* x49gp = button->x49gp;
    const x49gp_ui_key_t* key = button->key;
    /* GtkButton* gtkbutton = GTK_BUTTON( button->button ); */

    /* #ifdef DEBUG_X49GP_UI */
    /*     printf( "%s: button %u: col %u, row %u, eint %u\n", __FUNCTION__, event->button, button->key->column, button->key->row, */
    /*             button->key->eint ); */
    /* #endif */

    button->down = false;
    button->hold = false;

    /* if ( button != cause ) */
    /*     gtkbutton->in_button = false; */
    key_to_button( button->button, false );

    if ( key->rowbit )
        s3c2410_io_port_g_update( x49gp, key->column, key->row, key->columnbit, key->rowbit, 0 );
    else
        s3c2410_io_port_f_set_bit( x49gp, key->eint, 0 );
}

static void ui_release_all_buttons( x49gp_t* x49gp, x49gp_ui_button_t* cause )
{
    x49gp_ui_button_t* button;
    x49gp_ui_t* ui = x49gp->ui;

    for ( int i = 0; i < NB_KEYS; i++ ) {
        button = &ui->buttons[ i ];

        if ( !button->down )
            continue;

        ui_release_button( button, cause );
    }
}

static gboolean react_to_button_press( GtkWidget* widget, GdkEventButton* event, gpointer user_data )
{
    x49gp_ui_button_t* button = user_data;
    const x49gp_ui_key_t* key = button->key;
    x49gp_t* x49gp = button->x49gp;
    x49gp_ui_t* ui = x49gp->ui;

#ifdef DEBUG_X49GP_UI
    fprintf( stderr, "%s:%u: type %u, button %u\n", __FUNCTION__, __LINE__, event->type, event->button );
#endif

    if ( event->type != GDK_BUTTON_PRESS )
        return false;

    switch ( event->button ) {
        case 1:
            ui->buttons_down++;
            if ( button->down )
                return false;
            button->down = true;
            break;
        case 3:
            button->hold = true;
            if ( button->down )
                return false;
            key_to_button( button->button, true );
            button->down = true;
            break;
        default:
            return true;
    }

#ifdef DEBUG_X49GP_UI
    printf( "%s: button %u: col %u, row %u, eint %u\n", __FUNCTION__, event->button, button->key->column, button->key->row,
            button->key->eint );
#endif

    if ( key->rowbit )
        s3c2410_io_port_g_update( x49gp, key->column, key->row, key->columnbit, key->rowbit, 1 );
    else
        s3c2410_io_port_f_set_bit( x49gp, key->eint, 1 );

    return false;
}

static gboolean react_to_button_release( GtkWidget* widget, GdkEventButton* event, gpointer user_data )
{
    x49gp_ui_button_t* button = user_data;
    x49gp_t* x49gp = button->x49gp;
    x49gp_ui_t* ui = x49gp->ui;

    if ( event->type != GDK_BUTTON_RELEASE )
        return false;

    switch ( event->button ) {
        case 1:
            break;
        default:
            return true;
    }

    if ( ui->buttons_down > 0 )
        ui->buttons_down--;

    if ( ui->buttons_down == 0 )
        ui_release_all_buttons( x49gp, button );
    else
        ui_release_button( button, button );

    return false;
}

static gboolean react_to_button_leave( GtkWidget* widget, GdkEventCrossing* event, gpointer user_data )
{
    x49gp_ui_button_t* button = user_data;

    if ( event->type != GDK_LEAVE_NOTIFY )
        return false;

    if ( !button->hold )
        return false;

    return true;
}

static gboolean react_to_focus_lost( GtkWidget* widget, GdkEventFocus* event, gpointer user_data )
{
    x49gp_t* x49gp = user_data;
    x49gp_ui_t* ui = x49gp->ui;

    if ( event->type != GDK_FOCUS_CHANGE )
        return false;

    ui->buttons_down = 0;
    ui_release_all_buttons( x49gp, NULL );

    return false;
}

static void ui_open_file_dialog( x49gp_t* x49gp, const char* prompt, GtkFileChooserAction action, char** filename )
{
    x49gp_ui_t* ui = x49gp->ui;
    GtkWidget* dialog = gtk_file_chooser_dialog_new( prompt, GTK_WINDOW( ui->window ), action, "_Cancel", GTK_RESPONSE_CANCEL, "_Open",
                                                     GTK_RESPONSE_ACCEPT, NULL );

    gtk_file_chooser_set_local_only( GTK_FILE_CHOOSER( dialog ), true );
    gtk_file_chooser_set_select_multiple( GTK_FILE_CHOOSER( dialog ), false );

    if ( gtk_dialog_run( GTK_DIALOG( dialog ) ) == GTK_RESPONSE_ACCEPT )
        *filename = gtk_file_chooser_get_filename( GTK_FILE_CHOOSER( dialog ) );
    else
        *filename = NULL;

    gtk_widget_destroy( dialog );
}

static void do_select_and_mount_sd_folder( GtkMenuItem* menuitem, gpointer user_data )
{
    x49gp_t* x49gp = user_data;
    char* filename;

    ui_open_file_dialog( x49gp, "Choose SD folder ...", GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER, &filename );
    if ( filename != NULL )
        s3c2410_sdi_mount( x49gp, filename );
}

static void do_select_and_mount_sd_image( GtkMenuItem* menuitem, gpointer user_data )
{
    x49gp_t* x49gp = user_data;
    char* filename;

    ui_open_file_dialog( x49gp, "Choose SD image ...", GTK_FILE_CHOOSER_ACTION_OPEN, &filename );
    if ( filename != NULL )
        s3c2410_sdi_mount( x49gp, filename );
}

static void do_start_gdb_server( GtkMenuItem* menuitem, gpointer user_data )
{
    x49gp_t* x49gp = user_data;

    if ( x49gp->debug_port != 0 && !gdbserver_isactive() ) {
        gdbserver_start( x49gp->debug_port );
        gdb_handlesig( x49gp->env, 0 );
    }
}

static void do_emulator_reset( GtkMenuItem* menuitem, gpointer user_data )
{
    x49gp_t* x49gp = user_data;

    x49gp_modules_reset( x49gp, X49GP_RESET_POWER_ON );
    cpu_reset( x49gp->env );
    x49gp_set_idle( x49gp, 0 );
}

static gboolean react_to_key_event( GtkWidget* widget, GdkEventKey* event, gpointer user_data )
{
    x49gp_t* x49gp = user_data;
    x49gp_ui_t* ui = x49gp->ui;
    x49gp_ui_button_t* button;
    GdkEventButton bev;
    /* gboolean save_in; */
    int index;
    guint keyval;

#ifdef DEBUG_X49GP_UI
    fprintf( stderr, "%s:%u: type %u, keyval %04x\n", __FUNCTION__, __LINE__, event->type, event->keyval );
#endif
    /* We want to know the keyval as interpreted without modifiers. */
    /* However, there is one modifier we do care about: NumLock, */
    /* which normally is represented by MOD2. */
    if ( !gdk_keymap_translate_keyboard_state( gdk_keymap_get_for_display( gdk_display_get_default() ), event->hardware_keycode,
                                               event->state & GDK_MOD2_MASK, event->group, &keyval, NULL, NULL, NULL ) )
        return false;
#ifdef DEBUG_X49GP_UI
    fprintf( stderr, "%s:%u: state %u, base keyval %04x\n", __FUNCTION__, __LINE__, event->state, keyval );
#endif

    switch ( keyval ) {
        case GDK_KEY_a:
        case GDK_KEY_F1:
            index = 0;
            break;
        case GDK_KEY_b:
        case GDK_KEY_F2:
            index = 1;
            break;
        case GDK_KEY_c:
        case GDK_KEY_F3:
            index = 2;
            break;
        case GDK_KEY_d:
        case GDK_KEY_F4:
            index = 3;
            break;
        case GDK_KEY_e:
        case GDK_KEY_F5:
            index = 4;
            break;
        case GDK_KEY_f:
        case GDK_KEY_F6:
            index = 5;
            break;
        case GDK_KEY_g:
            index = 6;
            break;
        case GDK_KEY_h:
            index = 7;
            break;
        case GDK_KEY_i:
            index = 8;
            break;
        case GDK_KEY_j:
            index = 9;
            break;
        case GDK_KEY_k:
            index = 10;
            break;
        case GDK_KEY_l:
            index = 11;
            break;
        case GDK_KEY_Up:
        case GDK_KEY_KP_Up:
            index = 12;
            break;
        case GDK_KEY_Left:
        case GDK_KEY_KP_Left:
            index = 13;
            break;
        case GDK_KEY_Down:
        case GDK_KEY_KP_Down:
            index = 14;
            break;
        case GDK_KEY_Right:
        case GDK_KEY_KP_Right:
            index = 15;
            break;
        case GDK_KEY_m:
            index = 16;
            break;
        case GDK_KEY_n:
            index = 17;
            break;
        case GDK_KEY_o:
        case GDK_KEY_apostrophe:
            index = 18;
            break;
        case GDK_KEY_p:
            index = 19;
            break;
        case GDK_KEY_BackSpace:
        case GDK_KEY_Delete:
        case GDK_KEY_KP_Delete:
            index = 20;
            break;
        case GDK_KEY_dead_circumflex:
        case GDK_KEY_asciicircum:
        case GDK_KEY_q:
        case GDK_KEY_caret:
            index = 21;
            break;
        case GDK_KEY_r:
            index = 22;
            break;
        case GDK_KEY_s:
            index = 23;
            break;
        case GDK_KEY_t:
            index = 24;
            break;
        case GDK_KEY_u:
            index = 25;
            break;
        case GDK_KEY_v:
            index = 26;
            break;
        case GDK_KEY_w:
            index = 27;
            break;
        case GDK_KEY_x:
            index = 28;
            break;
        case GDK_KEY_y:
            index = 29;
            break;
        case GDK_KEY_z:
        case GDK_KEY_slash:
        case GDK_KEY_KP_Divide:
            index = 30;
            break;
        case GDK_KEY_Tab:
            index = 31;
            break;
#ifndef __APPLE__
        case GDK_KEY_Alt_L:
        case GDK_KEY_Alt_R:
        case GDK_KEY_Meta_L:
        case GDK_KEY_Meta_R:
        case GDK_KEY_Mode_switch:
            index = 31;
            break;
#endif
        case GDK_KEY_7:
        case GDK_KEY_KP_7:
            index = 32;
            break;
        case GDK_KEY_8:
        case GDK_KEY_KP_8:
            index = 33;
            break;
        case GDK_KEY_9:
        case GDK_KEY_KP_9:
            index = 34;
            break;
        case GDK_KEY_multiply:
        case GDK_KEY_KP_Multiply:
            index = 35;
            break;
        case GDK_KEY_Shift_L:
        case GDK_KEY_Shift_R:
            index = 36;
            break;
        case GDK_KEY_4:
        case GDK_KEY_KP_4:
            index = 37;
            break;
        case GDK_KEY_5:
        case GDK_KEY_KP_5:
            index = 38;
            break;
        case GDK_KEY_6:
        case GDK_KEY_KP_6:
            index = 39;
            break;
        case GDK_KEY_minus:
        case GDK_KEY_KP_Subtract:
            index = 40;
            break;
        case GDK_KEY_Control_L:
        case GDK_KEY_Control_R:
            index = 41;
            break;
        case GDK_KEY_1:
        case GDK_KEY_KP_1:
            index = 42;
            break;
        case GDK_KEY_2:
        case GDK_KEY_KP_2:
            index = 43;
            break;
        case GDK_KEY_3:
        case GDK_KEY_KP_3:
            index = 44;
            break;
        case GDK_KEY_plus:
        case GDK_KEY_KP_Add:
            index = 45;
            break;
        case GDK_KEY_Escape:
            index = 46;
            break;
        case GDK_KEY_0:
        case GDK_KEY_KP_0:
            index = 47;
            break;
        case GDK_KEY_period:
        case GDK_KEY_comma:
        case GDK_KEY_KP_Decimal:
        case GDK_KEY_KP_Separator:
            index = 48;
            break;
        case GDK_KEY_space:
        case GDK_KEY_KP_Space:
            index = 49;
            break;
        case GDK_KEY_Return:
        case GDK_KEY_KP_Enter:
            index = 50;
            break;

        /* QWERTY compat: US English, UK English, International English */
        case GDK_KEY_backslash:
            index = 35;
            break;
        case GDK_KEY_equal:
            index = 45;
            break;

        /* QWERTZ compat: German */
        case GDK_KEY_ssharp:
            index = 30;
            break;
        case GDK_KEY_numbersign:
            index = 35;
            break;

        case GDK_KEY_F7:
        case GDK_KEY_F10:
            x49gp->arm_exit = 1;
            cpu_exit( x49gp->env );
            return false;

        case GDK_KEY_F12:
            switch ( event->type ) {
                case GDK_KEY_PRESS:
                    x49gp_modules_reset( x49gp, X49GP_RESET_POWER_ON );
                    cpu_reset( x49gp->env );
                    x49gp_set_idle( x49gp, 1 );
                    break;
                case GDK_KEY_RELEASE:
                    x49gp_set_idle( x49gp, 0 );
                    break;
                default:
                    break;
            }
            return false;

        case GDK_KEY_Menu:
            gtk_widget_set_sensitive( ui->menu_unmount, s3c2410_sdi_is_mounted( x49gp ) );
            if ( ui->menu_debug )
                gtk_widget_set_sensitive( ui->menu_debug, !gdbserver_isactive() );

            gtk_menu_popup_at_widget( GTK_MENU( ui->menu ), ui->window, GDK_GRAVITY_NORTH_WEST, GDK_GRAVITY_NORTH_WEST, NULL );
            return false;

        default:
            return false;
    }

    button = &ui->buttons[ index ];

    memset( &bev, 0, sizeof( GdkEventButton ) );

    bev.time = event->time;
    bev.button = 1;
    bev.state = event->state;

    /* save_in = GTK_BUTTON( button->button )->in_button; */

    switch ( event->type ) {
        case GDK_KEY_PRESS:
            bev.type = GDK_BUTTON_PRESS;
            react_to_button_press( button->button, &bev, button );
            /* GTK_BUTTON( button->button )->in_button = true; */
            key_to_button( button->button, true );
            /* GTK_BUTTON( button->button )->in_button = save_in; */
            break;
        case GDK_KEY_RELEASE:
            bev.type = GDK_BUTTON_RELEASE;
            /* GTK_BUTTON( button->button )->in_button = true; */
            key_to_button( button->button, false );
            /* GTK_BUTTON( button->button )->in_button = save_in; */
            react_to_button_release( button->button, &bev, button );
            break;
        default:
            return false;
    }

    return true;
}

static int redraw_lcd( GtkWidget* widget, cairo_t* cr, gpointer user_data )
{
    x49gp_t* x49gp = user_data;
    x49gp_ui_t* ui = x49gp->ui;

    cairo_set_source_surface( cr, ui->lcd_surface, 0, 0 );
    cairo_paint( cr );

    return false;
}

static int draw_lcd( GtkWidget* widget, GdkEventConfigure* event, gpointer user_data )
{
    x49gp_t* x49gp = user_data;
    x49gp_ui_t* ui = x49gp->ui;

    if ( NULL != ui->lcd_surface )
        return false;

    ui->lcd_surface = cairo_image_surface_create( CAIRO_FORMAT_RGB24, LCD_WIDTH, LCD_HEIGHT );

    return false;
}

static gboolean react_to_window_click( GtkWidget* widget, GdkEventButton* event, gpointer user_data )
{
#ifdef DEBUG_X49GP_UI
    fprintf( stderr, "%s:%u: type %u, button %u\n", __FUNCTION__, __LINE__, event->type, event->button );
#endif

    gdk_window_focus( gtk_widget_get_window( widget ), event->time );
    gdk_window_raise( gtk_widget_get_window( widget ) );

    x49gp_t* x49gp = user_data;
    x49gp_ui_t* ui = x49gp->ui;

    gtk_widget_set_sensitive( ui->menu_unmount, s3c2410_sdi_is_mounted( x49gp ) );
    if ( ui->menu_debug )
        gtk_widget_set_sensitive( ui->menu_debug, !gdbserver_isactive() );

    if ( event->type != GDK_BUTTON_PRESS )
        return false;

    switch ( event->button ) {
        case 1: // left click
            gdk_window_begin_move_drag( gtk_widget_get_window( widget ), event->button, event->x_root, event->y_root, event->time );
            break;
        case 2: // middle click
            GtkClipboard* clip = gtk_clipboard_get( GDK_SELECTION_CLIPBOARD );
            gchar* text = gtk_clipboard_wait_for_text( clip );
            fprintf( stderr, "clipboard: %s\n", text );
            break;
        case 3: // right click
            gtk_menu_popup_at_pointer( GTK_MENU( ui->menu ), NULL );
            return true;
        default:
            break;
    }

    return false;
}

static void do_quit( gpointer user_data, GtkWidget* widget, GdkEvent* event )
{
    x49gp_t* x49gp = user_data;

    x49gp->arm_exit++;
}

static int ui_init( x49gp_module_t* module )
{
    x49gp_t* x49gp = module->x49gp;
    x49gp_ui_t* ui;

    ui = malloc( sizeof( x49gp_ui_t ) );
    if ( NULL == ui ) {
        fprintf( stderr, "%s: %s:%u: Out of memory\n", x49gp->progname, __FUNCTION__, __LINE__ );
        return -ENOMEM;
    }
    memset( ui, 0, sizeof( x49gp_ui_t ) );

    ui->buttons = malloc( NB_KEYS * sizeof( x49gp_ui_button_t ) );
    if ( NULL == ui->buttons ) {
        fprintf( stderr, "%s: %s:%u: Out of memory\n", x49gp->progname, __FUNCTION__, __LINE__ );
        free( ui );
        return -ENOMEM;
    }
    memset( ui->buttons, 0, NB_KEYS * sizeof( x49gp_ui_button_t ) );

    module->user_data = ui;
    x49gp->ui = ui;

    return 0;
}

static int ui_exit( x49gp_module_t* module ) { return 0; }

static int ui_reset( x49gp_module_t* module, x49gp_reset_t reset ) { return 0; }

static void _ui_load__init_color( GdkRGBA* color, u8 red, u8 green, u8 blue )
{
    color->red = red / 255.0;
    color->green = green / 255.0;
    color->blue = blue / 255.0;
    color->alpha = 1.0;
}

static inline void _ui_load__newrplify_ui_keys()
{
    // modify keys' labeling for newRPL
    for ( int i = 0; i < 6; i++ )
        ui_keys[ i ].left = NULL;

    for ( int i = 6; i < 12; i++ ) {
        ui_keys[ i ].label = "";
        ui_keys[ i ].left = NULL;
        ui_keys[ i ].right = NULL;
    }

    ui_keys[ 12 ].left = "UPDIR";

    ui_keys[ 13 ].left = "BEG";
    ui_keys[ 13 ].right = "COPY";

    ui_keys[ 14 ].left = "CUT";

    ui_keys[ 15 ].left = "END";
    ui_keys[ 15 ].right = "PASTE";

    ui_keys[ 16 ].label = "STO⏵";
    ui_keys[ 16 ].left = "RCL";
    ui_keys[ 16 ].right = "PREV.M";

    for ( int i = 17; i < 20; i++ ) {
        ui_keys[ i ].left = NULL;
        ui_keys[ i ].right = NULL;
    }

    ui_keys[ 19 ].label = "MENU";

    ui_keys[ 20 ].left = NULL;

    for ( int i = 23; i < 26; i++ )
        ui_keys[ i ].right = NULL;

    for ( int i = 31; i < 35; i++ )
        ui_keys[ i ].left = NULL;

    ui_keys[ 33 ].right = NULL;

    for ( int i = 37; i < 39; i++ ) {
        ui_keys[ i ].left = NULL;
        ui_keys[ i ].right = NULL;
    }

    ui_keys[ 43 ].left = NULL;

    ui_keys[ 46 ].left = NULL;
    ui_keys[ 46 ].below = NULL;

    ui_keys[ 50 ].left = NULL;
}

static GtkWidget* _ui_load__create_annunciator_widget( x49gp_ui_t* ui, const char* label )
{
    GtkWidget* ui_ann = gtk_label_new( NULL );
    gtk_style_context_add_class( gtk_widget_get_style_context( ui_ann ), "annunciator" );
    gtk_label_set_use_markup( GTK_LABEL( ui_ann ), true );
    gtk_label_set_markup( GTK_LABEL( ui_ann ), label );
    gtk_widget_set_size_request( ui_ann, ANNUNCIATOR_WIDTH, ANNUNCIATOR_HEIGHT );

    return ui_ann;
}

static int ui_load( x49gp_module_t* module, GKeyFile* keyfile )
{
    x49gp_t* x49gp = module->x49gp;
    x49gp_ui_t* ui = module->user_data;

    // create all colors
    _ui_load__init_color( &ui->colors[ UI_COLOR_GRAYSCALE_0 ], 0xab, 0xd2, 0xb4 );  /* #abd2b4 */
    _ui_load__init_color( &ui->colors[ UI_COLOR_GRAYSCALE_1 ], 0xa0, 0xc4, 0xa8 );  /* #a0c4a8 */
    _ui_load__init_color( &ui->colors[ UI_COLOR_GRAYSCALE_2 ], 0x94, 0xb6, 0x9c );  /* #94b69c */
    _ui_load__init_color( &ui->colors[ UI_COLOR_GRAYSCALE_3 ], 0x89, 0xa8, 0x90 );  /* #89a890 */
    _ui_load__init_color( &ui->colors[ UI_COLOR_GRAYSCALE_4 ], 0x7d, 0x9a, 0x84 );  /* #7d9a84 */
    _ui_load__init_color( &ui->colors[ UI_COLOR_GRAYSCALE_5 ], 0x72, 0x8c, 0x78 );  /* #728c78 */
    _ui_load__init_color( &ui->colors[ UI_COLOR_GRAYSCALE_6 ], 0x67, 0x7e, 0x6c );  /* #677e6c */
    _ui_load__init_color( &ui->colors[ UI_COLOR_GRAYSCALE_7 ], 0x5b, 0x70, 0x60 );  /* #5b7060 */
    _ui_load__init_color( &ui->colors[ UI_COLOR_GRAYSCALE_8 ], 0x50, 0x62, 0x54 );  /* #506254 */
    _ui_load__init_color( &ui->colors[ UI_COLOR_GRAYSCALE_9 ], 0x44, 0x54, 0x48 );  /* #445448 */
    _ui_load__init_color( &ui->colors[ UI_COLOR_GRAYSCALE_10 ], 0x39, 0x46, 0x3c ); /* #39463c */
    _ui_load__init_color( &ui->colors[ UI_COLOR_GRAYSCALE_11 ], 0x2e, 0x38, 0x30 ); /* #2e3830 */
    _ui_load__init_color( &ui->colors[ UI_COLOR_GRAYSCALE_12 ], 0x22, 0x2a, 0x24 ); /* #222a24 */
    _ui_load__init_color( &ui->colors[ UI_COLOR_GRAYSCALE_13 ], 0x17, 0x1c, 0x18 ); /* #171c18 */
    _ui_load__init_color( &ui->colors[ UI_COLOR_GRAYSCALE_14 ], 0x0b, 0x03, 0x0c ); /* #0b030c */
    _ui_load__init_color( &ui->colors[ UI_COLOR_GRAYSCALE_15 ], 0x00, 0x00, 0x00 ); /* #000000 */

    /* set calculator type and name */
    switch ( opt.model ) {
        case MODEL_50G_NEWRPL:
            ui->calculator = UI_CALCULATOR_HP50G_NEWRPL;
            ui->name = opt.name != NULL ? opt.name : "HP 50g / newRPL";
            break;
        case MODEL_49GP:
            ui->calculator = UI_CALCULATOR_HP49GP;
            ui->name = opt.name != NULL ? opt.name : "HP 49g+";
            break;
        case MODEL_49GP_NEWRPL:
            ui->calculator = UI_CALCULATOR_HP49GP_NEWRPL;
            ui->name = opt.name != NULL ? opt.name : "HP 49g+ / newRPL";
            break;
        case MODEL_50G:
        default:
            ui->calculator = UI_CALCULATOR_HP50G;
            ui->name = opt.name != NULL ? opt.name : "HP 50g";
            break;
    }

    // create window and widgets/stuff
    GtkWidget* window_container = gtk_box_new( GTK_ORIENTATION_VERTICAL, 0 );
    ui->ui_ann_left = _ui_load__create_annunciator_widget( ui, "⮢" );
    ui->ui_ann_right = _ui_load__create_annunciator_widget( ui, "⮣" );
    ui->ui_ann_alpha = _ui_load__create_annunciator_widget( ui, "α" );
    ui->ui_ann_battery = _ui_load__create_annunciator_widget( ui, "🪫" );
    ui->ui_ann_busy = _ui_load__create_annunciator_widget( ui, "⌛" );
    ui->ui_ann_io = _ui_load__create_annunciator_widget( ui, "⇄" );

    ui->window = gtk_window_new( GTK_WINDOW_TOPLEVEL );
    gtk_window_set_accept_focus( GTK_WINDOW( ui->window ), true );
    gtk_window_set_focus_on_map( GTK_WINDOW( ui->window ), true );
    gtk_window_set_decorated( GTK_WINDOW( ui->window ), true );
    gtk_window_set_resizable( GTK_WINDOW( ui->window ), true );
    gtk_window_set_title( GTK_WINDOW( ui->window ), ui->name );
    gtk_widget_realize( ui->window );
    gtk_container_add( GTK_CONTAINER( ui->window ), window_container );
    gtk_style_context_add_class( gtk_widget_get_style_context( ui->window ), "main-window" );

    g_signal_connect( G_OBJECT( ui->window ), "focus-out-event", G_CALLBACK( react_to_focus_lost ), x49gp );
    g_signal_connect( G_OBJECT( ui->window ), "key-press-event", G_CALLBACK( react_to_key_event ), x49gp );
    g_signal_connect( G_OBJECT( ui->window ), "key-release-event", G_CALLBACK( react_to_key_event ), x49gp );
    g_signal_connect( G_OBJECT( ui->window ), "button-press-event", G_CALLBACK( react_to_window_click ), x49gp );
    g_signal_connect_swapped( G_OBJECT( ui->window ), "delete-event", G_CALLBACK( do_quit ), x49gp );
    g_signal_connect_swapped( G_OBJECT( ui->window ), "destroy", G_CALLBACK( do_quit ), x49gp );
    gtk_widget_add_events( ui->window, GDK_FOCUS_CHANGE_MASK | GDK_BUTTON_PRESS_MASK | GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK );

    ui->lcd_canvas = gtk_drawing_area_new();
    gtk_style_context_add_class( gtk_widget_get_style_context( ui->lcd_canvas ), "lcd" );
    gtk_widget_set_size_request( ui->lcd_canvas, LCD_WIDTH, LCD_HEIGHT );
    g_signal_connect( G_OBJECT( ui->lcd_canvas ), "draw", G_CALLBACK( redraw_lcd ), x49gp );
    g_signal_connect( G_OBJECT( ui->lcd_canvas ), "configure-event", G_CALLBACK( draw_lcd ), x49gp );

    GtkWidget* lcd_container = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 0 );
    gtk_style_context_add_class( gtk_widget_get_style_context( lcd_container ), "lcd-container" );
    gtk_widget_set_size_request( lcd_container, LCD_WIDTH, LCD_HEIGHT + 3 );
    gtk_widget_set_margin_bottom( lcd_container, 3 );
    gtk_box_set_center_widget( GTK_BOX( lcd_container ), ui->lcd_canvas );

    GtkWidget* annunciators_container = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, ( LCD_WIDTH - ( 6 * ANNUNCIATOR_WIDTH ) ) / 5 );
    gtk_style_context_add_class( gtk_widget_get_style_context( annunciators_container ), "annunciators-container" );
    gtk_container_add( GTK_CONTAINER( annunciators_container ), ui->ui_ann_left );
    gtk_container_add( GTK_CONTAINER( annunciators_container ), ui->ui_ann_right );
    gtk_container_add( GTK_CONTAINER( annunciators_container ), ui->ui_ann_alpha );
    gtk_container_add( GTK_CONTAINER( annunciators_container ), ui->ui_ann_battery );
    gtk_container_add( GTK_CONTAINER( annunciators_container ), ui->ui_ann_busy );
    gtk_container_add( GTK_CONTAINER( annunciators_container ), ui->ui_ann_io );

    GtkWidget* display_container = gtk_box_new( GTK_ORIENTATION_VERTICAL, 0 );
    gtk_style_context_add_class( gtk_widget_get_style_context( annunciators_container ), "display-container" );
    gtk_widget_set_margin_top( display_container, LCD_PADDING );
    gtk_widget_set_margin_bottom( display_container, LCD_PADDING );
    gtk_widget_set_margin_start( display_container, LCD_PADDING );
    gtk_widget_set_margin_end( display_container, WINDOW_WIDTH - ( LCD_PADDING + LCD_WIDTH ) );
    gtk_container_add( GTK_CONTAINER( display_container ), annunciators_container );
    gtk_container_add( GTK_CONTAINER( display_container ), lcd_container );

    gtk_container_add( GTK_CONTAINER( window_container ), display_container );

    // keyboard
    GtkWidget* keyboard_container = gtk_fixed_new();
    gtk_style_context_add_class( gtk_widget_get_style_context( keyboard_container ), "keyboard-container" );
    gtk_widget_set_margin_top( keyboard_container, KEYBOARD_PADDING );
    gtk_widget_set_margin_bottom( keyboard_container, KEYBOARD_PADDING );
    gtk_widget_set_margin_start( keyboard_container, KEYBOARD_PADDING );
    gtk_widget_set_margin_end( keyboard_container, KEYBOARD_PADDING );

    gtk_container_add( GTK_CONTAINER( window_container ), keyboard_container );

    x49gp_ui_button_t* button;
    GtkWidget* ui_label;
    GtkWidget* ui_left;
    GtkWidget* ui_right;
    GtkWidget* ui_letter;
    GtkWidget* ui_below;
    int x, y, x2, y2;

    if ( ui->calculator == UI_CALCULATOR_HP49GP_NEWRPL || ui->calculator == UI_CALCULATOR_HP50G_NEWRPL )
        _ui_load__newrplify_ui_keys();

    for ( int i = 0; i < NB_KEYS; i++ ) {
        button = &ui->buttons[ i ];

        button->x49gp = x49gp;
        button->key = &ui_keys[ i ];

        button->button = gtk_button_new();
        gtk_widget_set_size_request( button->button, button->key->width, button->key->height );
        gtk_widget_set_can_focus( button->button, false );
        gtk_style_context_add_class( gtk_widget_get_style_context( button->button ), button->key->css_class );

        button->box = gtk_event_box_new();
        gtk_event_box_set_visible_window( GTK_EVENT_BOX( button->box ), true );
        gtk_event_box_set_above_child( GTK_EVENT_BOX( button->box ), false );
        gtk_container_add( GTK_CONTAINER( button->box ), button->button );

        g_signal_connect( G_OBJECT( button->button ), "button-press-event", G_CALLBACK( react_to_button_press ), button );
        g_signal_connect( G_OBJECT( button->button ), "button-release-event", G_CALLBACK( react_to_button_release ), button );
        g_signal_connect( G_OBJECT( button->button ), "leave-notify-event", G_CALLBACK( react_to_button_leave ), button );

        gtk_widget_add_events( button->button, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_LEAVE_NOTIFY_MASK );

        gtk_widget_set_size_request( button->box, button->key->width, button->key->height );
        gtk_fixed_put( GTK_FIXED( keyboard_container ), button->box, button->key->x, KEYBOARD_PADDING + button->key->y );

        if ( button->key->label ) {
            ui_label = gtk_label_new( NULL );
            gtk_style_context_add_class( gtk_widget_get_style_context( ui_label ), "label-key" );

            gtk_label_set_use_markup( GTK_LABEL( ui_label ), true );
            gtk_label_set_markup( GTK_LABEL( ui_label ), button->key->label );

            gtk_container_add( GTK_CONTAINER( button->button ), ui_label );
        }
        if ( button->key->left ) {
            ui_left = gtk_label_new( NULL );
            gtk_style_context_add_class( gtk_widget_get_style_context( ui_left ), "label-left" );
            gtk_label_set_use_markup( GTK_LABEL( ui_left ), true );
            gtk_label_set_markup( GTK_LABEL( ui_left ), button->key->left );

            if ( button->key->right ) {
                ui_right = gtk_label_new( NULL );
                gtk_style_context_add_class( gtk_widget_get_style_context( ui_right ), "label-right" );
                gtk_label_set_use_markup( GTK_LABEL( ui_right ), true );
                gtk_label_set_markup( GTK_LABEL( ui_right ), button->key->right );
            }
            y = y2 = button->key->y - ( TINY_TEXT_HEIGHT + 2 );
            if ( button->key->right ) {
                x = button->key->x;

                x2 = button->key->x + button->key->width - _tiny_text_width( button->key->right );

                if ( _tiny_text_width( button->key->right ) + _tiny_text_width( button->key->left ) > button->key->width ) {
                    x -= ( ( _tiny_text_width( button->key->right ) + _tiny_text_width( button->key->left ) ) - button->key->width ) / 2;
                    x2 += ( ( _tiny_text_width( button->key->right ) + _tiny_text_width( button->key->left ) ) - button->key->width ) / 2;
                }

                gtk_fixed_put( GTK_FIXED( keyboard_container ), ui_left, x, KEYBOARD_PADDING + y );
                gtk_fixed_put( GTK_FIXED( keyboard_container ), ui_right, x2, KEYBOARD_PADDING + y2 );
            } else {
                x = button->key->x + ( ( button->key->width - _tiny_text_width( button->key->left ) ) / 2 );
                gtk_fixed_put( GTK_FIXED( keyboard_container ), ui_left, x, KEYBOARD_PADDING + y );
            }
        }
        if ( button->key->letter ) {
            ui_letter = gtk_label_new( NULL );
            gtk_style_context_add_class( gtk_widget_get_style_context( ui_letter ), "label-letter" );
            gtk_label_set_use_markup( GTK_LABEL( ui_letter ), true );
            gtk_label_set_markup( GTK_LABEL( ui_letter ), button->key->letter );

            x = button->key->x + button->key->width + ( TINY_TEXT_WIDTH / 2 );
            y = button->key->y + button->key->height - ( TINY_TEXT_HEIGHT / 2 );
            gtk_fixed_put( GTK_FIXED( keyboard_container ), ui_letter, x, KEYBOARD_PADDING + y );
        }
        if ( button->key->below ) {
            ui_below = gtk_label_new( NULL );
            gtk_style_context_add_class( gtk_widget_get_style_context( ui_below ), "label-below" );
            gtk_label_set_use_markup( GTK_LABEL( ui_below ), true );
            gtk_label_set_markup( GTK_LABEL( ui_below ), button->key->below );

            x = button->key->x + ( ( button->key->width - _tiny_text_width( button->key->below ) ) / 2 );
            y = button->key->y + button->key->height + 2;
            gtk_fixed_put( GTK_FIXED( keyboard_container ), ui_below, x, KEYBOARD_PADDING + y );
        }
    }

    // Right-click menu
    ui->menu = gtk_menu_new();

    GtkWidget* menu_mount_folder = gtk_menu_item_new_with_label( "Mount SD folder ..." );
    gtk_menu_shell_append( GTK_MENU_SHELL( ui->menu ), menu_mount_folder );
    g_signal_connect( G_OBJECT( menu_mount_folder ), "activate", G_CALLBACK( do_select_and_mount_sd_folder ), x49gp );

    GtkWidget* menu_mount_image = gtk_menu_item_new_with_label( "Mount SD image ..." );
    gtk_menu_shell_append( GTK_MENU_SHELL( ui->menu ), menu_mount_image );
    g_signal_connect( G_OBJECT( menu_mount_image ), "activate", G_CALLBACK( do_select_and_mount_sd_image ), x49gp );

    GtkWidget* menu_unmount = gtk_menu_item_new_with_label( "Unmount SD" );
    gtk_menu_shell_append( GTK_MENU_SHELL( ui->menu ), menu_unmount );
    g_signal_connect_swapped( G_OBJECT( menu_unmount ), "activate", G_CALLBACK( s3c2410_sdi_unmount ), x49gp );
    ui->menu_unmount = menu_unmount;

    if ( x49gp->debug_port != 0 ) {
        gtk_menu_shell_append( GTK_MENU_SHELL( ui->menu ), gtk_separator_menu_item_new() );

        GtkWidget* menu_debug = gtk_menu_item_new_with_label( "Start debugger" );
        gtk_menu_shell_append( GTK_MENU_SHELL( ui->menu ), menu_debug );
        g_signal_connect( G_OBJECT( menu_debug ), "activate", G_CALLBACK( do_start_gdb_server ), x49gp );
        ui->menu_debug = menu_debug;
    } else
        ui->menu_debug = NULL;

    gtk_menu_shell_append( GTK_MENU_SHELL( ui->menu ), gtk_separator_menu_item_new() );

    GtkWidget* menu_reset = gtk_menu_item_new_with_label( "Reset" );
    gtk_menu_shell_append( GTK_MENU_SHELL( ui->menu ), menu_reset );
    g_signal_connect( G_OBJECT( menu_reset ), "activate", G_CALLBACK( do_emulator_reset ), x49gp );

    GtkWidget* menu_quit = gtk_menu_item_new_with_label( "Quit" );
    gtk_menu_shell_append( GTK_MENU_SHELL( ui->menu ), menu_quit );
    g_signal_connect_swapped( G_OBJECT( menu_quit ), "activate", G_CALLBACK( do_quit ), x49gp );

    gtk_widget_show_all( ui->menu );

    // Apply CSS
    GtkCssProvider* style_provider = gtk_css_provider_new();
    char* color_bg_49gp = "#f5deb3";
    char* color_bg_50g = "#272727";
    char* color_shift_left_49gp = "#4060a4";
    char* color_shift_left_50g = "#f5f5f5";
    bool is_50g = ( ui->calculator == UI_CALCULATOR_HP50G || ui->calculator == UI_CALCULATOR_HP50G_NEWRPL );
    char* css;

    asprintf( &css, css_global, is_50g ? color_bg_50g : color_bg_49gp, is_50g ? color_shift_left_50g : color_shift_left_49gp,
              FONT_SIZE_SYMBOL, FONT_SIZE_NUMBER, FONT_SIZE_KEY, FONT_SIZE_TINY, is_50g ? color_shift_left_50g : color_shift_left_49gp );

    gtk_css_provider_load_from_data( style_provider, css, -1, NULL );

    gtk_style_context_add_provider_for_screen( gdk_screen_get_default(), GTK_STYLE_PROVIDER( style_provider ),
                                               GTK_STYLE_PROVIDER_PRIORITY_USER + 1 );

    g_object_unref( style_provider );

    // finally show the window
    gtk_widget_show_all( ui->window );

    return 0;
}

static int ui_save( x49gp_module_t* module, GKeyFile* keyfile ) { return 0; }

static void _draw_pixel( cairo_surface_t* target, int x, int y, int w, int h, GdkRGBA* color )
{
    cairo_t* cr = cairo_create( target );

    cairo_set_source_rgb( cr, color->red, color->green, color->blue );
    cairo_rectangle( cr, x, y, w, h );
    cairo_fill( cr );

    cairo_destroy( cr );
}

/********************/
/* Public functions */
/********************/

void gui_update_lcd( x49gp_t* x49gp )
{
    x49gp_ui_t* ui = x49gp->ui;
    s3c2410_lcd_t* lcd = x49gp->s3c2410_lcd;

    if ( lcd->lcdcon1 & 1 ) {
        gtk_widget_set_opacity( ui->ui_ann_left, x49gp_get_pixel_color( lcd, 131, 1 ) > 0 ? 1 : 0 );
        gtk_widget_set_opacity( ui->ui_ann_right, x49gp_get_pixel_color( lcd, 131, 2 ) > 0 ? 1 : 0 );
        gtk_widget_set_opacity( ui->ui_ann_alpha, x49gp_get_pixel_color( lcd, 131, 3 ) > 0 ? 1 : 0 );
        gtk_widget_set_opacity( ui->ui_ann_battery, x49gp_get_pixel_color( lcd, 131, 4 ) > 0 ? 1 : 0 );
        gtk_widget_set_opacity( ui->ui_ann_busy, x49gp_get_pixel_color( lcd, 131, 5 ) > 0 ? 1 : 0 );
        gtk_widget_set_opacity( ui->ui_ann_io, x49gp_get_pixel_color( lcd, 131, 0 ) > 0 ? 1 : 0 );

        for ( int y = 0; y < ( LCD_HEIGHT / LCD_PIXEL_SCALE ); y++ )
            for ( int x = 0; x < ( LCD_WIDTH / LCD_PIXEL_SCALE ); x++ )
                _draw_pixel( ui->lcd_surface, LCD_PIXEL_SCALE * x, LCD_PIXEL_SCALE * y, LCD_PIXEL_SCALE, LCD_PIXEL_SCALE,
                             &( ui->colors[ UI_COLOR_GRAYSCALE_0 + x49gp_get_pixel_color( lcd, x, y ) ] ) );
    }

    GdkRectangle rect;
    rect.x = 0;
    rect.y = 0;
    rect.width = LCD_WIDTH;
    rect.height = LCD_HEIGHT;

    gdk_window_invalidate_rect( gtk_widget_get_window( ui->lcd_canvas ), &rect, false );
}

void gui_show_error( x49gp_t* x49gp, const char* text )
{
    GtkWidget* dialog;
    x49gp_ui_t* ui = x49gp->ui;

    dialog =
        gtk_message_dialog_new( GTK_WINDOW( ui->window ), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "%s", text );

    gtk_dialog_run( GTK_DIALOG( dialog ) );
    gtk_widget_destroy( dialog );
}

void gui_open_firmware( x49gp_t* x49gp, char** filename )
{
    ui_open_file_dialog( x49gp, "Choose firmware ...", GTK_FILE_CHOOSER_ACTION_OPEN, filename );
}

int gui_init( x49gp_t* x49gp )
{
    x49gp_module_t* module;

    if ( x49gp_module_init( x49gp, "gui", ui_init, ui_exit, ui_reset, ui_load, ui_save, NULL, &module ) )
        return -1;

    return x49gp_module_register( module );
}
