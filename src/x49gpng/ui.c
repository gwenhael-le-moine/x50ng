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

#define DEBUG_LAYOUT false

#define NB_KEYS 51

#define KB_WIDTH_6_KEYS 36
#define KB_WIDTH_5_KEYS 46

#define KB_HEIGHT_MENU_KEYS 22
#define KB_HEIGHT_SMALL_KEYS 28
#define KB_HEIGHT_BIG_KEYS 32

#define KB_LINE_HEIGHT 48
#define KB_SPACING_KEYS 14
#define KB_COLUMN_WIDTH_6_KEYS ( KB_WIDTH_6_KEYS + KB_SPACING_KEYS )
#define KB_COLUMN_WIDTH_5_KEYS ( KB_WIDTH_5_KEYS + KB_SPACING_KEYS )

static x49gp_ui_key_t ui_keys[ NB_KEYS ] = {
    {.label = "F1",
     .letter = "A",
     .left = "Y=",
     .right = NULL,
     .below = NULL,
     .color = UI_COLOR_BLACK,
     .font_size = 12.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 12.0,
     .x = 0,
     .y = 0,
     .width = KB_WIDTH_6_KEYS,
     .height = KB_HEIGHT_MENU_KEYS,
     .column = 5,
     .row = 1,
     .columnbit = ( 1 << 5 ),
     .rowbit = ( 1 << 1 ),
     .eint = 1,
     .bg_color = UI_COLOR_GRAYSCALE_5},
    {.label = "F2",
     .letter = "B",
     .left = "WIN",
     .right = NULL,
     .below = NULL,
     .color = UI_COLOR_BLACK,
     .font_size = 12.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 12.0,
     .x = KB_COLUMN_WIDTH_6_KEYS,
     .y = 0,
     .width = KB_WIDTH_6_KEYS,
     .height = KB_HEIGHT_MENU_KEYS,
     .column = 5,
     .row = 2,
     .columnbit = ( 1 << 5 ),
     .rowbit = ( 1 << 2 ),
     .eint = 2,
     .bg_color = UI_COLOR_GRAYSCALE_5},
    {.label = "F3",
     .letter = "C",
     .left = "GRAPH",
     .right = NULL,
     .below = NULL,
     .color = UI_COLOR_BLACK,
     .font_size = 12.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 12.0,
     .x = 2 * KB_COLUMN_WIDTH_6_KEYS,
     .y = 0,
     .width = KB_WIDTH_6_KEYS,
     .height = KB_HEIGHT_MENU_KEYS,
     .column = 5,
     .row = 3,
     .columnbit = ( 1 << 5 ),
     .rowbit = ( 1 << 3 ),
     .eint = 3,
     .bg_color = UI_COLOR_GRAYSCALE_5},
    {.label = "F4",
     .letter = "D",
     .left = "2D/3D",
     .right = NULL,
     .below = NULL,
     .color = UI_COLOR_BLACK,
     .font_size = 12.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 12.0,
     .x = 3 * KB_COLUMN_WIDTH_6_KEYS,
     .y = 0,
     .width = KB_WIDTH_6_KEYS,
     .height = KB_HEIGHT_MENU_KEYS,
     .column = 5,
     .row = 4,
     .columnbit = ( 1 << 5 ),
     .rowbit = ( 1 << 4 ),
     .eint = 4,
     .bg_color = UI_COLOR_GRAYSCALE_5},
    {.label = "F5",
     .letter = "E",
     .left = "TBLSET",
     .right = NULL,
     .below = NULL,
     .color = UI_COLOR_BLACK,
     .font_size = 12.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 12.0,
     .x = 4 * KB_COLUMN_WIDTH_6_KEYS,
     .y = 0,
     .width = KB_WIDTH_6_KEYS,
     .height = KB_HEIGHT_MENU_KEYS,
     .column = 5,
     .row = 5,
     .columnbit = ( 1 << 5 ),
     .rowbit = ( 1 << 5 ),
     .eint = 5,
     .bg_color = UI_COLOR_GRAYSCALE_5},
    {.label = "F6",
     .letter = "F",
     .left = "TABLE",
     .right = NULL,
     .below = NULL,
     .color = UI_COLOR_BLACK,
     .font_size = 12.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 12.0,
     .x = 5 * KB_COLUMN_WIDTH_6_KEYS,
     .y = 0,
     .width = KB_WIDTH_6_KEYS,
     .height = KB_HEIGHT_MENU_KEYS,
     .column = 5,
     .row = 6,
     .columnbit = ( 1 << 5 ),
     .rowbit = ( 1 << 6 ),
     .eint = 6,
     .bg_color = UI_COLOR_GRAYSCALE_5},

    {.label = "APPS",
     .letter = "G",
     .left = "FILES",
     .right = "BEGIN",
     .below = NULL,
     .color = UI_COLOR_WHITE,
     .font_size = 10.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 10.0,
     .x = 0,
     .y = KB_LINE_HEIGHT,
     .width = KB_WIDTH_6_KEYS,
     .height = KB_HEIGHT_SMALL_KEYS,
     .column = 5,
     .row = 7,
     .columnbit = ( 1 << 5 ),
     .rowbit = ( 1 << 7 ),
     .eint = 7,
     .bg_color = UI_COLOR_BLACK      },
    {.label = "MODE",
     .letter = "H",
     .left = "CUSTOM",
     .right = "END",
     .below = NULL,
     .color = UI_COLOR_WHITE,
     .font_size = 10.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 10.0,
     .x = KB_COLUMN_WIDTH_6_KEYS,
     .y = KB_LINE_HEIGHT,
     .width = KB_WIDTH_6_KEYS,
     .height = KB_HEIGHT_SMALL_KEYS,
     .column = 6,
     .row = 5,
     .columnbit = ( 1 << 6 ),
     .rowbit = ( 1 << 5 ),
     .eint = 5,
     .bg_color = UI_COLOR_BLACK      },
    {.label = "TOOL",
     .letter = "I",
     .left = "i",
     .right = "I",
     .below = NULL,
     .color = UI_COLOR_WHITE,
     .font_size = 10.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 10.0,
     .x = 2 * KB_COLUMN_WIDTH_6_KEYS,
     .y = KB_LINE_HEIGHT,
     .width = KB_WIDTH_6_KEYS,
     .height = KB_HEIGHT_SMALL_KEYS,
     .column = 6,
     .row = 6,
     .columnbit = ( 1 << 6 ),
     .rowbit = ( 1 << 6 ),
     .eint = 6,
     .bg_color = UI_COLOR_BLACK      },
    {.label = "VAR",
     .letter = "J",
     .left = "UPDIR",
     .right = "COPY",
     .below = NULL,
     .color = UI_COLOR_WHITE,
     .font_size = 10.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 10.0,
     .x = 0,
     .y = 2 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_6_KEYS,
     .height = KB_HEIGHT_SMALL_KEYS,
     .column = 6,
     .row = 7,
     .columnbit = ( 1 << 6 ),
     .rowbit = ( 1 << 7 ),
     .eint = 7,
     .bg_color = UI_COLOR_BLACK      },
    {.label = "STO⏵",
     .letter = "K",
     .left = "RCL",
     .right = "CUT",
     .below = NULL,
     .color = UI_COLOR_WHITE,
     .font_size = 10.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 10.0,
     .x = KB_COLUMN_WIDTH_6_KEYS,
     .y = 2 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_6_KEYS,
     .height = KB_HEIGHT_SMALL_KEYS,
     .column = 7,
     .row = 1,
     .columnbit = ( 1 << 7 ),
     .rowbit = ( 1 << 1 ),
     .eint = 1,
     .bg_color = UI_COLOR_BLACK      },
    {.label = "NXT",
     .letter = "L",
     .left = "PREV",
     .right = "PASTE",
     .below = NULL,
     .color = UI_COLOR_WHITE,
     .font_size = 10.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 10.0,
     .x = 2 * KB_COLUMN_WIDTH_6_KEYS,
     .y = 2 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_6_KEYS,
     .height = KB_HEIGHT_SMALL_KEYS,
     .column = 7,
     .row = 2,
     .columnbit = ( 1 << 7 ),
     .rowbit = ( 1 << 2 ),
     .eint = 2,
     .bg_color = UI_COLOR_BLACK      },

    {.label = "▲",
     .letter = NULL,
     .left = NULL,
     .right = NULL,
     .below = NULL,
     .color = UI_COLOR_BLACK,
     .font_size = 20.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 0.0,
     .x = 4 * KB_COLUMN_WIDTH_6_KEYS,
     .y = 0.75 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_6_KEYS,
     .height = KB_HEIGHT_BIG_KEYS,
     .column = 6,
     .row = 1,
     .columnbit = ( 1 << 6 ),
     .rowbit = ( 1 << 1 ),
     .eint = 1,
     .bg_color = UI_COLOR_GRAYSCALE_5},
    {.label = "◀",
     .letter = NULL,
     .left = NULL,
     .right = NULL,
     .below = NULL,
     .color = UI_COLOR_BLACK,
     .font_size = 20.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 0.0,
     .x = 3.25 * KB_COLUMN_WIDTH_6_KEYS,
     .y = 1.25 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_6_KEYS,
     .height = KB_HEIGHT_BIG_KEYS,
     .column = 6,
     .row = 2,
     .columnbit = ( 1 << 6 ),
     .rowbit = ( 1 << 2 ),
     .eint = 2,
     .bg_color = UI_COLOR_GRAYSCALE_5},
    {.label = "▼",
     .letter = NULL,
     .left = NULL,
     .right = NULL,
     .below = NULL,
     .color = UI_COLOR_BLACK,
     .font_size = 20.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 0.0,
     .x = 4 * KB_COLUMN_WIDTH_6_KEYS,
     .y = 1.75 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_6_KEYS,
     .height = KB_HEIGHT_BIG_KEYS,
     .column = 6,
     .row = 3,
     .columnbit = ( 1 << 6 ),
     .rowbit = ( 1 << 3 ),
     .eint = 3,
     .bg_color = UI_COLOR_GRAYSCALE_5},
    {.label = "▶",
     .letter = NULL,
     .left = NULL,
     .right = NULL,
     .below = NULL,
     .color = UI_COLOR_BLACK,
     .font_size = 20.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 0.0,
     .x = 4.78 * KB_COLUMN_WIDTH_6_KEYS,
     .y = 1.25 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_6_KEYS,
     .height = KB_HEIGHT_BIG_KEYS,
     .column = 6,
     .row = 4,
     .columnbit = ( 1 << 6 ),
     .rowbit = ( 1 << 4 ),
     .eint = 4,
     .bg_color = UI_COLOR_GRAYSCALE_5},

    {.label = "HIST",
     .letter = "M",
     .left = "CMD",
     .right = "UNDO",
     .below = NULL,
     .color = UI_COLOR_WHITE,
     .font_size = 12.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 12.0,
     .x = 0,
     .y = 3 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_SMALL_KEYS,
     .column = 4,
     .row = 1,
     .columnbit = ( 1 << 4 ),
     .rowbit = ( 1 << 1 ),
     .eint = 1,
     .bg_color = UI_COLOR_BLACK      },
    {.label = "EVAL",
     .letter = "N",
     .left = "PRG",
     .right = "CHARS",
     .below = NULL,
     .color = UI_COLOR_WHITE,
     .font_size = 12.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 12.0,
     .x = KB_COLUMN_WIDTH_5_KEYS,
     .y = 3 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_SMALL_KEYS,
     .column = 3,
     .row = 1,
     .columnbit = ( 1 << 3 ),
     .rowbit = ( 1 << 1 ),
     .eint = 1,
     .bg_color = UI_COLOR_BLACK      },
    {.label = "'",
     .letter = "O",
     .left = "MTRW",
     .right = "EQW",
     .below = NULL,
     .color = UI_COLOR_WHITE,
     .font_size = 12.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 12.0,
     .x = 2 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 3 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_SMALL_KEYS,
     .column = 2,
     .row = 1,
     .columnbit = ( 1 << 2 ),
     .rowbit = ( 1 << 1 ),
     .eint = 1,
     .bg_color = UI_COLOR_BLACK      },
    {.label = "SYMB",
     .letter = "P",
     .left = "MTH",
     .right = "CAT",
     .below = NULL,
     .color = UI_COLOR_WHITE,
     .font_size = 12.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 12.0,
     .x = 3 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 3 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_SMALL_KEYS,
     .column = 1,
     .row = 1,
     .columnbit = ( 1 << 1 ),
     .rowbit = ( 1 << 1 ),
     .eint = 1,
     .bg_color = UI_COLOR_BLACK      },
    {.label = "⬅",
     .letter = NULL,
     .left = "DEL",
     .right = "CLEAR",
     .below = NULL,
     .color = UI_COLOR_WHITE,
     .font_size = 12.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 0.0,
     .x = 4 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 3 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_SMALL_KEYS,
     .column = 0,
     .row = 1,
     .columnbit = ( 1 << 0 ),
     .rowbit = ( 1 << 1 ),
     .eint = 1,
     .bg_color = UI_COLOR_BLACK      },

    {.label = "Y<sup>x</sup>",
     .letter = "Q",
     .left = "𝑒<sup>x</sup>",
     .right = "LN",
     .below = NULL,
     .color = UI_COLOR_WHITE,
     .font_size = 12.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 12.0,
     .x = 0,
     .y = 4 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_SMALL_KEYS,
     .column = 4,
     .row = 2,
     .columnbit = ( 1 << 4 ),
     .rowbit = ( 1 << 2 ),
     .eint = 2,
     .bg_color = UI_COLOR_BLACK      },
    {.label = "√𝔁",
     .letter = "R",
     .left = "𝔁<sup>2</sup>",
     .right = "<sup>x</sup>√y",
     .below = NULL,
     .color = UI_COLOR_WHITE,
     .font_size = 12.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 12.0,
     .x = KB_COLUMN_WIDTH_5_KEYS,
     .y = 4 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_SMALL_KEYS,
     .column = 3,
     .row = 2,
     .columnbit = ( 1 << 3 ),
     .rowbit = ( 1 << 2 ),
     .eint = 2,
     .bg_color = UI_COLOR_BLACK      },
    {.label = "SIN",
     .letter = "S",
     .left = "ASIN",
     .right = "∑",
     .below = NULL,
     .color = UI_COLOR_WHITE,
     .font_size = 12.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 12.0,
     .x = 2 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 4 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_SMALL_KEYS,
     .column = 2,
     .row = 2,
     .columnbit = ( 1 << 2 ),
     .rowbit = ( 1 << 2 ),
     .eint = 2,
     .bg_color = UI_COLOR_BLACK      },
    {.label = "COS",
     .letter = "T",
     .left = "ACOS",
     .right = "∂",
     .below = NULL,
     .color = UI_COLOR_WHITE,
     .font_size = 12.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 12.0,
     .x = 3 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 4 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_SMALL_KEYS,
     .column = 1,
     .row = 2,
     .columnbit = ( 1 << 1 ),
     .rowbit = ( 1 << 2 ),
     .eint = 2,
     .bg_color = UI_COLOR_BLACK      },
    {.label = "TAN",
     .letter = "U",
     .left = "ATAN",
     .right = "∫",
     .below = NULL,
     .color = UI_COLOR_WHITE,
     .font_size = 12.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 12.0,
     .x = 4 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 4 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_SMALL_KEYS,
     .column = 0,
     .row = 2,
     .columnbit = ( 1 << 0 ),
     .rowbit = ( 1 << 2 ),
     .eint = 2,
     .bg_color = UI_COLOR_BLACK      },

    {.label = "EEX",
     .letter = "V",
     .left = "10<sup>𝔁</sup>",
     .right = "LOG",
     .below = NULL,
     .color = UI_COLOR_WHITE,
     .font_size = 12.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 12.0,
     .x = 0,
     .y = 5 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_SMALL_KEYS,
     .column = 4,
     .row = 3,
     .columnbit = ( 1 << 4 ),
     .rowbit = ( 1 << 3 ),
     .eint = 3,
     .bg_color = UI_COLOR_BLACK      },
    {.label = "+/-",
     .letter = "W",
     .left = "≠",
     .right = "=",
     .below = NULL,
     .color = UI_COLOR_WHITE,
     .font_size = 12.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 12.0,
     .x = KB_COLUMN_WIDTH_5_KEYS,
     .y = 5 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_SMALL_KEYS,
     .column = 3,
     .row = 3,
     .columnbit = ( 1 << 3 ),
     .rowbit = ( 1 << 3 ),
     .eint = 3,
     .bg_color = UI_COLOR_BLACK      },
    {.label = "𝔁",
     .letter = "X",
     .left = "≤",
     .right = "&gt;",
     .below = NULL,
     .color = UI_COLOR_WHITE,
     .font_size = 12.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 12.0,
     .x = 2 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 5 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_SMALL_KEYS,
     .column = 2,
     .row = 3,
     .columnbit = ( 1 << 2 ),
     .rowbit = ( 1 << 3 ),
     .eint = 3,
     .bg_color = UI_COLOR_BLACK      },
    {.label = "1/𝔁",
     .letter = "Y",
     .left = "≥",
     .right = ">",
     .below = NULL,
     .color = UI_COLOR_WHITE,
     .font_size = 12.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 12.0,
     .x = 3 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 5 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_SMALL_KEYS,
     .column = 1,
     .row = 3,
     .columnbit = ( 1 << 1 ),
     .rowbit = ( 1 << 3 ),
     .eint = 3,
     .bg_color = UI_COLOR_BLACK      },
    {.label = "÷",
     .letter = "Z",
     .left = "ABS",
     .right = "ARG",
     .below = NULL,
     .color = UI_COLOR_WHITE,
     .font_size = 19.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 12.0,
     .x = 4 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 5 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_SMALL_KEYS,
     .column = 0,
     .row = 3,
     .columnbit = ( 1 << 0 ),
     .rowbit = ( 1 << 3 ),
     .eint = 3,
     .bg_color = UI_COLOR_BLACK      },

    {.label = "ALPHA",
     .letter = NULL,
     .left = "USER",
     .right = "ENTRY",
     .below = NULL,
     .color = UI_COLOR_BLACK,
     .font_size = 11.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 0.0,
     .x = 0,
     .y = 6 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_BIG_KEYS,
     .column = 0,
     .row = 0,
     .columnbit = 0,
     .rowbit = 0,
     .eint = 4,
     .bg_color = UI_COLOR_YELLOW     },
    {.label = "7",
     .letter = NULL,
     .left = "S.SLV",
     .right = "NUM.SLV",
     .below = NULL,
     .color = UI_COLOR_WHITE,
     .font_size = 19.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 0.0,
     .x = KB_COLUMN_WIDTH_5_KEYS,
     .y = 6 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_BIG_KEYS,
     .column = 3,
     .row = 4,
     .columnbit = ( 1 << 3 ),
     .rowbit = ( 1 << 4 ),
     .eint = 4,
     .bg_color = UI_COLOR_BLACK      },
    {.label = "8",
     .letter = NULL,
     .left = "EXP&amp;LN",
     .right = "TRIG",
     .below = NULL,
     .color = UI_COLOR_WHITE,
     .font_size = 19.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 0.0,
     .x = 2 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 6 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_BIG_KEYS,
     .column = 2,
     .row = 4,
     .columnbit = ( 1 << 2 ),
     .rowbit = ( 1 << 4 ),
     .eint = 4,
     .bg_color = UI_COLOR_BLACK      },
    {.label = "9",
     .letter = NULL,
     .left = "FINANCE",
     .right = "TIME",
     .below = NULL,
     .color = UI_COLOR_WHITE,
     .font_size = 19.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 0.0,
     .x = 3 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 6 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_BIG_KEYS,
     .column = 1,
     .row = 4,
     .columnbit = ( 1 << 1 ),
     .rowbit = ( 1 << 4 ),
     .eint = 4,
     .bg_color = UI_COLOR_BLACK      },
    {.label = "×",
     .letter = NULL,
     .left = "[ ]",
     .right = "\" \"",
     .below = NULL,
     .color = UI_COLOR_WHITE,
     .font_size = 19.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 0.0,
     .x = 4 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 6 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_BIG_KEYS,
     .column = 0,
     .row = 4,
     .columnbit = ( 1 << 0 ),
     .rowbit = ( 1 << 4 ),
     .eint = 4,
     .bg_color = UI_COLOR_BLACK      },

    {.label = "⮢",
     .letter = NULL,
     .left = NULL,
     .right = NULL,
     .below = NULL,
     .color = UI_COLOR_BLACK,
     .font_size = 19.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 0.0,
     .x = 0,
     .y = 7 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_BIG_KEYS,
     .column = 0,
     .row = 0,
     .columnbit = 0,
     .rowbit = 0,
     .eint = 5,
     .bg_color = UI_COLOR_WHITE      },
    {.label = "4",
     .letter = NULL,
     .left = "CALC",
     .right = "ALG",
     .below = NULL,
     .color = UI_COLOR_WHITE,
     .font_size = 19.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 0.0,
     .x = KB_COLUMN_WIDTH_5_KEYS,
     .y = 7 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_BIG_KEYS,
     .column = 3,
     .row = 5,
     .columnbit = ( 1 << 3 ),
     .rowbit = ( 1 << 5 ),
     .eint = 5,
     .bg_color = UI_COLOR_BLACK      },
    {.label = "5",
     .letter = NULL,
     .left = "MATRICES",
     .right = "STAT",
     .below = NULL,
     .color = UI_COLOR_WHITE,
     .font_size = 19.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 0.0,
     .x = 2 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 7 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_BIG_KEYS,
     .column = 2,
     .row = 5,
     .columnbit = ( 1 << 2 ),
     .rowbit = ( 1 << 5 ),
     .eint = 5,
     .bg_color = UI_COLOR_BLACK      },
    {.label = "6",
     .letter = NULL,
     .left = "CONVERT",
     .right = "UNITS",
     .below = NULL,
     .color = UI_COLOR_WHITE,
     .font_size = 19.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 0.0,
     .x = 3 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 7 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_BIG_KEYS,
     .column = 1,
     .row = 5,
     .columnbit = ( 1 << 1 ),
     .rowbit = ( 1 << 5 ),
     .eint = 5,
     .bg_color = UI_COLOR_BLACK      },
    {.label = "-",
     .letter = NULL,
     .left = "( )",
     .right = "_",
     .below = NULL,
     .color = UI_COLOR_WHITE,
     .font_size = 19.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 0.0,
     .x = 4 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 7 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_BIG_KEYS,
     .column = 0,
     .row = 5,
     .columnbit = ( 1 << 0 ),
     .rowbit = ( 1 << 5 ),
     .eint = 5,
     .bg_color = UI_COLOR_BLACK      },

    {.label = "⮣",
     .letter = NULL,
     .left = NULL,
     .right = NULL,
     .below = NULL,
     .color = UI_COLOR_BLACK,
     .font_size = 19.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 0.0,
     .x = 0,
     .y = 8 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_BIG_KEYS,
     .column = 0,
     .row = 0,
     .columnbit = 0,
     .rowbit = 0,
     .eint = 6,
     .bg_color = UI_COLOR_ORANGE     },
    {.label = "1",
     .letter = NULL,
     .left = "ARITH",
     .right = "CMPLX",
     .below = NULL,
     .color = UI_COLOR_WHITE,
     .font_size = 19.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 0.0,
     .x = KB_COLUMN_WIDTH_5_KEYS,
     .y = 8 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_BIG_KEYS,
     .column = 3,
     .row = 6,
     .columnbit = ( 1 << 3 ),
     .rowbit = ( 1 << 6 ),
     .eint = 6,
     .bg_color = UI_COLOR_BLACK      },
    {.label = "2",
     .letter = NULL,
     .left = "DEF",
     .right = "LIB",
     .below = NULL,
     .color = UI_COLOR_WHITE,
     .font_size = 19.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 0.0,
     .x = 2 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 8 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_BIG_KEYS,
     .column = 2,
     .row = 6,
     .columnbit = ( 1 << 2 ),
     .rowbit = ( 1 << 6 ),
     .eint = 6,
     .bg_color = UI_COLOR_BLACK      },
    {.label = "3",
     .letter = NULL,
     .left = "#",
     .right = "BASE",
     .below = NULL,
     .color = UI_COLOR_WHITE,
     .font_size = 19.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 0.0,
     .x = 3 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 8 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_BIG_KEYS,
     .column = 1,
     .row = 6,
     .columnbit = ( 1 << 1 ),
     .rowbit = ( 1 << 6 ),
     .eint = 6,
     .bg_color = UI_COLOR_BLACK      },
    {.label = "+",
     .letter = NULL,
     .left = "{ }",
     .right = "« »",
     .below = NULL,
     .color = UI_COLOR_WHITE,
     .font_size = 19.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 0.0,
     .x = 4 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 8 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_BIG_KEYS,
     .column = 0,
     .row = 6,
     .columnbit = ( 1 << 0 ),
     .rowbit = ( 1 << 6 ),
     .eint = 6,
     .bg_color = UI_COLOR_BLACK      },

    {.label = "ON",
     .letter = NULL,
     .left = "CONT",
     .right = "OFF",
     .below = "CANCEL",
     .color = UI_COLOR_WHITE,
     .font_size = 12.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 0.0,
     .x = 0,
     .y = 9 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_BIG_KEYS,
     .column = 0,
     .row = 0,
     .columnbit = 0,
     .rowbit = 0,
     .eint = 0,
     .bg_color = UI_COLOR_BLACK      },
    {.label = "0",
     .letter = NULL,
     .left = "∞",
     .right = "→",
     .below = NULL,
     .color = UI_COLOR_WHITE,
     .font_size = 19.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 0.0,
     .x = KB_COLUMN_WIDTH_5_KEYS,
     .y = 9 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_BIG_KEYS,
     .column = 3,
     .row = 7,
     .columnbit = ( 1 << 3 ),
     .rowbit = ( 1 << 7 ),
     .eint = 7,
     .bg_color = UI_COLOR_BLACK      },
    {.label = ".",
     .letter = NULL,
     .left = ": :",
     .right = "↲",
     .below = NULL,
     .color = UI_COLOR_WHITE,
     .font_size = 19.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 0.0,
     .x = 2 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 9 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_BIG_KEYS,
     .column = 2,
     .row = 7,
     .columnbit = ( 1 << 2 ),
     .rowbit = ( 1 << 7 ),
     .eint = 7,
     .bg_color = UI_COLOR_BLACK      },
    {.label = "SPC",
     .letter = NULL,
     .left = "𝚷",
     .right = ",",
     .below = NULL,
     .color = UI_COLOR_WHITE,
     .font_size = 12.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 0.0,
     .x = 3 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 9 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_BIG_KEYS,
     .column = 1,
     .row = 7,
     .columnbit = ( 1 << 1 ),
     .rowbit = ( 1 << 7 ),
     .eint = 7,
     .bg_color = UI_COLOR_BLACK      },
    {.label = "ENTER",
     .letter = NULL,
     .left = "ANS",
     .right = "→NUM",
     .below = NULL,
     .color = UI_COLOR_WHITE,
     .font_size = 11.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 0.0,
     .x = 4 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 9 * KB_LINE_HEIGHT,
     .width = KB_WIDTH_5_KEYS,
     .height = KB_HEIGHT_BIG_KEYS,
     .column = 0,
     .row = 7,
     .columnbit = ( 1 << 0 ),
     .rowbit = ( 1 << 7 ),
     .eint = 7,
     .bg_color = UI_COLOR_BLACK      },
};

/*************/
/* functions */
/*************/

static PangoAttrList* ui_load__pango_attrs_common_new( void )
{
    PangoAttrList* pango_attributes = pango_attr_list_new();

    pango_attr_list_insert( pango_attributes, pango_attr_family_new( opt.font ) );
    pango_attr_list_insert( pango_attributes, pango_attr_weight_new( PANGO_WEIGHT_BOLD ) );

    return pango_attributes;
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
            gtk_button_pressed( GTK_BUTTON( button->button ) );
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

static void ui_release_button( x49gp_ui_button_t* button, x49gp_ui_button_t* cause )
{
    x49gp_t* x49gp = button->x49gp;
    const x49gp_ui_key_t* key = button->key;
    GtkButton* gtkbutton = GTK_BUTTON( button->button );

    /* #ifdef DEBUG_X49GP_UI */
    /*     printf( "%s: button %u: col %u, row %u, eint %u\n", __FUNCTION__, event->button, button->key->column, button->key->row, */
    /*             button->key->eint ); */
    /* #endif */

    button->down = false;
    button->hold = false;

    /* if ( button != cause ) */
    /*     gtkbutton->in_button = false; */
    gtk_button_released( gtkbutton );

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

static gboolean do_show_context_menu( GtkWidget* widget, GdkEventButton* event, gpointer user_data )
{
    x49gp_t* x49gp = user_data;
    x49gp_ui_t* ui = x49gp->ui;

    gtk_widget_set_sensitive( ui->menu_unmount, s3c2410_sdi_is_mounted( x49gp ) );
    if ( ui->menu_debug )
        gtk_widget_set_sensitive( ui->menu_debug, !gdbserver_isactive() );

    if ( event->type == GDK_BUTTON_PRESS && event->button == 3 ) {
        gtk_menu_popup( GTK_MENU( ui->menu ), NULL, NULL, NULL, NULL, event->button, event->time );
        return true;
    }

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
    GtkWidget* dialog;
    x49gp_ui_t* ui = x49gp->ui;

    dialog = gtk_file_chooser_dialog_new( prompt, GTK_WINDOW( ui->window ), action, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN,
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
    if ( !gdk_keymap_translate_keyboard_state( gdk_keymap_get_default(), event->hardware_keycode, event->state & GDK_MOD2_MASK,
                                               event->group, &keyval, NULL, NULL, NULL ) )
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

            gtk_menu_popup( GTK_MENU( ui->menu ), NULL, NULL, NULL, ui->lcd_canvas, 0, event->time );
            // gtk_menu_popup_at_widget( GTK_MENU( ui->menu ), ui->lcd_canvas, GDK_GRAVITY_NORTH_WEST, GDK_GRAVITY_NORTH_WEST, NULL );
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
            gtk_button_pressed( GTK_BUTTON( button->button ) );
            /* GTK_BUTTON( button->button )->in_button = save_in; */
            break;
        case GDK_KEY_RELEASE:
            bev.type = GDK_BUTTON_RELEASE;
            /* GTK_BUTTON( button->button )->in_button = true; */
            gtk_button_released( GTK_BUTTON( button->button ) );
            /* GTK_BUTTON( button->button )->in_button = save_in; */
            react_to_button_release( button->button, &bev, button );
            break;
        default:
            return false;
    }

    return true;
}

static int redraw_lcd( GtkWidget* widget, GdkEventExpose* event, gpointer user_data )
{
    x49gp_t* x49gp = user_data;
    x49gp_ui_t* ui = x49gp->ui;

    cairo_t* cr = gdk_cairo_create( gtk_widget_get_window( widget ) );
    cairo_set_source_surface( cr, ui->lcd_surface, 0, 0 );
    cairo_paint( cr );
    cairo_destroy( cr );

    return false;
}

static int draw_lcd( GtkWidget* widget, GdkEventConfigure* event, gpointer user_data )
{
    x49gp_t* x49gp = user_data;
    x49gp_ui_t* ui = x49gp->ui;

    if ( NULL != ui->lcd_surface )
        return false;

    ui->lcd_surface = cairo_image_surface_create( CAIRO_FORMAT_RGB24, ui->lcd_width, ui->lcd_height );

    cairo_t* cr = cairo_create( ui->lcd_surface );
    GdkColor color = ui->colors[ UI_COLOR_GRAYSCALE_0 ];
    cairo_set_source_rgb( cr, color.red / 65535.0, color.green / 65535.0, color.blue / 65535.0 );
    cairo_rectangle( cr, 0, 0, ui->lcd_width, ui->lcd_height );
    cairo_fill( cr );
    cairo_destroy( cr );

    return false;
}

static inline unsigned color2rgb( x49gp_ui_t* ui, int color )
{
    return 0x000000 | ( ui->colors[ color ].red << 8 ) | ( ui->colors[ color ].green << 16 ) | ( ui->colors[ color ].blue );
}

static gboolean react_to_window_click( GtkWidget* widget, GdkEventButton* event, gpointer user_data )
{
#ifdef DEBUG_X49GP_UI
    fprintf( stderr, "%s:%u: type %u, button %u\n", __FUNCTION__, __LINE__, event->type, event->button );
#endif

    gdk_window_focus( gtk_widget_get_window( widget ), event->time );
    gdk_window_raise( gtk_widget_get_window( widget ) );

    if ( event->type != GDK_BUTTON_PRESS )
        return false;

    if ( event->button != 1 )
        return false;

    gdk_window_begin_move_drag( gtk_widget_get_window( widget ), event->button, event->x_root, event->y_root, event->time );

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

static void _ui_load__init_color( GdkColor* color, u8 red, u8 green, u8 blue )
{
    color->red = ( red << 8 ) | red;
    color->green = ( green << 8 ) | green;
    color->blue = ( blue << 8 ) | blue;
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

static void ui_load__style_button( x49gp_ui_t* ui, x49gp_ui_button_t* button )
{
    GtkStyle* style = gtk_style_new();

    /* style->xthickness = 0; */
    /* style->ythickness = 0; */

    for ( int i = 0; i < 5; i++ ) {
        style->fg[ i ] = ui->colors[ button->key->color ];
        style->bg[ i ] = ui->colors[ button->key->bg_color ];

        style->text[ i ] = style->fg[ i ];
        style->base[ i ] = style->bg[ i ];
    }

    gtk_widget_set_style( button->button, style );
}

static GtkWidget* _ui_load__create_annunciator_widget( x49gp_ui_t* ui, const char* label )
{
    GtkWidget* ui_ann = gtk_label_new( NULL );

    PangoAttrList* pango_attributes = ui_load__pango_attrs_common_new();
    pango_attr_list_insert( pango_attributes, pango_attr_size_new( ( 12.0 / 1.8 ) * PANGO_SCALE ) );
    pango_attr_list_insert( pango_attributes, pango_attr_weight_new( PANGO_WEIGHT_BOLD ) );
    GdkColor* fgcolor = &( ui->colors[ UI_COLOR_GRAYSCALE_0 ] );
    pango_attr_list_insert( pango_attributes, pango_attr_foreground_new( fgcolor->red, fgcolor->green, fgcolor->blue ) );

    gtk_label_set_attributes( GTK_LABEL( ui_ann ), pango_attributes );

    gtk_label_set_use_markup( GTK_LABEL( ui_ann ), true );
    gtk_label_set_markup( GTK_LABEL( ui_ann ), label );

    return ui_ann;
}

static int ui_load( x49gp_module_t* module, GKeyFile* keyfile )
{
    x49gp_t* x49gp = module->x49gp;
    x49gp_ui_t* ui = module->user_data;

    // create all colors
    {
        _ui_load__init_color( &ui->colors[ UI_COLOR_BLACK ], 0x00, 0x00, 0x00 );          /* #000000 */
        _ui_load__init_color( &ui->colors[ UI_COLOR_WHITE ], 0xff, 0xff, 0xff );          /* #ffffff */
        _ui_load__init_color( &ui->colors[ UI_COLOR_YELLOW ], 0xfa, 0xe8, 0x2c );         /* #fae82c */
        _ui_load__init_color( &ui->colors[ UI_COLOR_RED ], 0x8e, 0x25, 0x18 );            /* #8e2518 */
        _ui_load__init_color( &ui->colors[ UI_COLOR_GREEN ], 0x14, 0x4d, 0x49 );          /* #144d49 */
        _ui_load__init_color( &ui->colors[ UI_COLOR_SILVER ], 0xe0, 0xe0, 0xe0 );         /* #e0e0e0 */
        _ui_load__init_color( &ui->colors[ UI_COLOR_ORANGE ], 0xc0, 0x6e, 0x60 );         /* #c06e60 */
        _ui_load__init_color( &ui->colors[ UI_COLOR_BLUE ], 0x40, 0x60, 0xa4 );           /* #4060a4 */
        _ui_load__init_color( &ui->colors[ UI_COLOR_GRAYSCALE_0 ], 0xab, 0xd2, 0xb4 );    /* #abd2b4 */
        _ui_load__init_color( &ui->colors[ UI_COLOR_GRAYSCALE_1 ], 0xa0, 0xc4, 0xa8 );    /* #a0c4a8 */
        _ui_load__init_color( &ui->colors[ UI_COLOR_GRAYSCALE_2 ], 0x94, 0xb6, 0x9c );    /* #94b69c */
        _ui_load__init_color( &ui->colors[ UI_COLOR_GRAYSCALE_3 ], 0x89, 0xa8, 0x90 );    /* #89a890 */
        _ui_load__init_color( &ui->colors[ UI_COLOR_GRAYSCALE_4 ], 0x7d, 0x9a, 0x84 );    /* #7d9a84 */
        _ui_load__init_color( &ui->colors[ UI_COLOR_GRAYSCALE_5 ], 0x72, 0x8c, 0x78 );    /* #728c78 */
        _ui_load__init_color( &ui->colors[ UI_COLOR_GRAYSCALE_6 ], 0x67, 0x7e, 0x6c );    /* #677e6c */
        _ui_load__init_color( &ui->colors[ UI_COLOR_GRAYSCALE_7 ], 0x5b, 0x70, 0x60 );    /* #5b7060 */
        _ui_load__init_color( &ui->colors[ UI_COLOR_GRAYSCALE_8 ], 0x50, 0x62, 0x54 );    /* #506254 */
        _ui_load__init_color( &ui->colors[ UI_COLOR_GRAYSCALE_9 ], 0x44, 0x54, 0x48 );    /* #445448 */
        _ui_load__init_color( &ui->colors[ UI_COLOR_GRAYSCALE_10 ], 0x39, 0x46, 0x3c );   /* #39463c */
        _ui_load__init_color( &ui->colors[ UI_COLOR_GRAYSCALE_11 ], 0x2e, 0x38, 0x30 );   /* #2e3830 */
        _ui_load__init_color( &ui->colors[ UI_COLOR_GRAYSCALE_12 ], 0x22, 0x2a, 0x24 );   /* #222a24 */
        _ui_load__init_color( &ui->colors[ UI_COLOR_GRAYSCALE_13 ], 0x17, 0x1c, 0x18 );   /* #171c18 */
        _ui_load__init_color( &ui->colors[ UI_COLOR_GRAYSCALE_14 ], 0x0b, 0x03, 0x0c );   /* #0b030c */
        _ui_load__init_color( &ui->colors[ UI_COLOR_GRAYSCALE_15 ], 0x00, 0x00, 0x00 );   /* #000000 */
        _ui_load__init_color( &ui->colors[ UI_COLOR_FACEPLATE_49GP ], 0xf5, 0xde, 0xb3 ); /* #f5deb3 */
        _ui_load__init_color( &ui->colors[ UI_COLOR_FACEPLATE_50G ], 0x27, 0x27, 0x27 );  /* #272727 */
    }

    /* set calculator type and name */
    {
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
    }

    /* set coordinates of LCD and keyboard */
    int annunciator_height = 16;
    {
        ui->lcd_width = 131 * LCD_PIXEL_SCALE;
        ui->lcd_height = 80 * LCD_PIXEL_SCALE;
        ui->lcd_x_offset = ui->annunciators_x_offset = ui->annunciators_y_offset = 20;

        ui->kb_x_offset = 10;

        int kb_width = ( ui->kb_x_offset ) + ( 5 * KB_COLUMN_WIDTH_5_KEYS );
        if ( ui->width < kb_width ) {
            ui->width = kb_width;
            ui->lcd_x_offset = ui->annunciators_x_offset = ui->annunciators_y_offset = ( ui->width - ui->lcd_width ) / 2;
        }

        ui->lcd_y_offset = ui->annunciators_y_offset + annunciator_height;

        ui->kb_y_offset = ui->lcd_height + ( 2 * ui->lcd_y_offset );

        ui->width = ui->lcd_width + ( 2 * ui->lcd_x_offset );
        ui->height = ui->kb_y_offset + ui_keys[ NB_KEYS - 1 ].y + KB_LINE_HEIGHT;
    }

    // create window and widgets/stuff
    GtkWidget* lcd_canvas_container = gtk_event_box_new();
    GtkWidget* fixed_widgets_container = gtk_fixed_new();
    {
        int faceplate_color = ( ui->calculator == UI_CALCULATOR_HP49GP || ui->calculator == UI_CALCULATOR_HP49GP_NEWRPL )
                                  ? UI_COLOR_FACEPLATE_49GP
                                  : UI_COLOR_FACEPLATE_50G;

        ui->window = gtk_window_new( GTK_WINDOW_TOPLEVEL );
        gtk_window_set_default_size( GTK_WINDOW( ui->window ), ui->width, ui->height );
        gtk_window_set_accept_focus( GTK_WINDOW( ui->window ), true );
        gtk_window_set_focus_on_map( GTK_WINDOW( ui->window ), true );
        gtk_window_set_decorated( GTK_WINDOW( ui->window ), true );
        gtk_window_set_resizable( GTK_WINDOW( ui->window ), true );
        gtk_window_set_title( GTK_WINDOW( ui->window ), ui->name );
        gtk_widget_set_name( ui->window, ui->name );
        gtk_widget_realize( ui->window );
        gtk_widget_modify_bg( ui->window, GTK_STATE_NORMAL, &( ui->colors[ faceplate_color ] ) );
        gtk_container_add( GTK_CONTAINER( ui->window ), fixed_widgets_container );

        g_signal_connect( G_OBJECT( ui->window ), "focus-out-event", G_CALLBACK( react_to_focus_lost ), x49gp );
        g_signal_connect( G_OBJECT( ui->window ), "key-press-event", G_CALLBACK( react_to_key_event ), x49gp );
        g_signal_connect( G_OBJECT( ui->window ), "key-release-event", G_CALLBACK( react_to_key_event ), x49gp );
        g_signal_connect( G_OBJECT( ui->window ), "button-press-event", G_CALLBACK( react_to_window_click ), x49gp );
        g_signal_connect_swapped( G_OBJECT( ui->window ), "delete-event", G_CALLBACK( do_quit ), x49gp );
        g_signal_connect_swapped( G_OBJECT( ui->window ), "destroy", G_CALLBACK( do_quit ), x49gp );
        gtk_widget_add_events( ui->window, GDK_FOCUS_CHANGE_MASK | GDK_BUTTON_PRESS_MASK | GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK );

        ui->lcd_canvas = gtk_drawing_area_new();
        gtk_widget_set_size_request( ui->lcd_canvas, ui->lcd_width, ui->lcd_height );
#if GTK_MAJOR_VERSION == 2
        g_signal_connect( G_OBJECT( ui->lcd_canvas ), "expose-event", G_CALLBACK( redraw_lcd ), x49gp );
#elif GTK_MAJOR_VERSION == 3
        g_signal_connect( G_OBJECT( ui->lcd_canvas ), "draw", G_CALLBACK( redraw_lcd ), x49gp );
#endif
        g_signal_connect( G_OBJECT( ui->lcd_canvas ), "configure-event", G_CALLBACK( draw_lcd ), x49gp );

        gtk_event_box_set_visible_window( GTK_EVENT_BOX( lcd_canvas_container ), true );
        gtk_event_box_set_above_child( GTK_EVENT_BOX( lcd_canvas_container ), false );
        gtk_container_add( GTK_CONTAINER( lcd_canvas_container ), ui->lcd_canvas );
        gtk_widget_set_size_request( lcd_canvas_container, ui->lcd_width, ui->lcd_height );
        gtk_fixed_put( GTK_FIXED( fixed_widgets_container ), lcd_canvas_container, ui->lcd_x_offset, ui->lcd_y_offset );
        g_signal_connect( G_OBJECT( lcd_canvas_container ), "button-press-event", G_CALLBACK( do_show_context_menu ), x49gp );

        ui->ui_ann_left = _ui_load__create_annunciator_widget( ui, "⮢" );
        ui->ui_ann_right = _ui_load__create_annunciator_widget( ui, "⮣" );
        ui->ui_ann_alpha = _ui_load__create_annunciator_widget( ui, "α" );
        ui->ui_ann_battery = _ui_load__create_annunciator_widget( ui, "🪫" );
        ui->ui_ann_busy = _ui_load__create_annunciator_widget( ui, "⌛" );
        ui->ui_ann_io = _ui_load__create_annunciator_widget( ui, "⇄" );

        GtkWidget* fixed_annunciators_container = gtk_fixed_new();
        gtk_widget_modify_bg( fixed_annunciators_container, GTK_STATE_NORMAL, &( ui->colors[ UI_COLOR_GRAYSCALE_0 ] ) );
        gtk_widget_set_size_request( fixed_annunciators_container, ui->lcd_width, annunciator_height );
        gtk_fixed_put( GTK_FIXED( fixed_widgets_container ), fixed_annunciators_container, ui->annunciators_x_offset,
                       ui->annunciators_y_offset );

        gtk_widget_set_size_request( ui->ui_ann_left, 15, annunciator_height );
        gtk_fixed_put( GTK_FIXED( fixed_annunciators_container ), ui->ui_ann_left, 11, 0 );

        gtk_widget_set_size_request( ui->ui_ann_right, 15, annunciator_height );
        gtk_fixed_put( GTK_FIXED( fixed_annunciators_container ), ui->ui_ann_right, 56, 0 );

        gtk_widget_set_size_request( ui->ui_ann_alpha, 15, annunciator_height );
        gtk_fixed_put( GTK_FIXED( fixed_annunciators_container ), ui->ui_ann_alpha, 101, 0 );

        gtk_widget_set_size_request( ui->ui_ann_battery, 15, annunciator_height );
        gtk_fixed_put( GTK_FIXED( fixed_annunciators_container ), ui->ui_ann_battery, 146, 0 );

        gtk_widget_set_size_request( ui->ui_ann_busy, 15, annunciator_height );
        gtk_fixed_put( GTK_FIXED( fixed_annunciators_container ), ui->ui_ann_busy, 191, 0 );

        gtk_widget_set_size_request( ui->ui_ann_io, 15, annunciator_height );
        gtk_fixed_put( GTK_FIXED( fixed_annunciators_container ), ui->ui_ann_io, 236, 0 );
    }

    // keyboard
    {
        double tiny_font_size = 8.0;
        x49gp_ui_button_t* button;
        PangoAttrList* pango_attributes;
        GdkColor* fgcolor;
        GtkWidget* ui_label;
        GtkWidget* ui_left;
        GtkWidget* ui_right;
        GtkWidget* ui_letter;
        GtkWidget* ui_below;

        int left_color;
        int right_color;
        int below_color;
        GtkRequisition widget_size, widget2_size;
        int x, y, x2, y2;

        switch ( ui->calculator ) {
            case UI_CALCULATOR_HP49GP:
            case UI_CALCULATOR_HP49GP_NEWRPL:
                left_color = UI_COLOR_GREEN;
                right_color = UI_COLOR_RED;
                below_color = UI_COLOR_BLACK;
                break;

            default:
                ui->calculator = UI_CALCULATOR_HP50G;
                /* fall through */

            case UI_CALCULATOR_HP50G:
            case UI_CALCULATOR_HP50G_NEWRPL:
                left_color = UI_COLOR_WHITE;
                right_color = UI_COLOR_ORANGE;
                below_color = UI_COLOR_BLUE;
                break;
        }

        if ( ui->calculator == UI_CALCULATOR_HP49GP_NEWRPL || ui->calculator == UI_CALCULATOR_HP50G_NEWRPL )
            _ui_load__newrplify_ui_keys();

        for ( int i = 0; i < NB_KEYS; i++ ) {
            button = &ui->buttons[ i ];

            button->x49gp = x49gp;
            button->key = &ui_keys[ i ];

            button->button = gtk_button_new();
            gtk_widget_set_size_request( button->button, button->key->width, button->key->height );
            gtk_widget_set_can_focus( button->button, false );

            ui_load__style_button( ui, button );

            if ( button->key->label ) {
                ui_label = gtk_label_new( NULL );

                pango_attributes = ui_load__pango_attrs_common_new();
                pango_attr_list_insert( pango_attributes, pango_attr_size_new( ( button->key->font_size / 1.8 ) * PANGO_SCALE ) );
                pango_attr_list_insert( pango_attributes, pango_attr_weight_new( PANGO_WEIGHT_BOLD ) );
                fgcolor = &( ui->colors[ button->key->color ] );
                pango_attr_list_insert( pango_attributes, pango_attr_foreground_new( fgcolor->red, fgcolor->green, fgcolor->blue ) );

                gtk_label_set_attributes( GTK_LABEL( ui_label ), pango_attributes );

                gtk_label_set_use_markup( GTK_LABEL( ui_label ), true );
                gtk_label_set_markup( GTK_LABEL( ui_label ), button->key->label );

                gtk_container_add( GTK_CONTAINER( button->button ), ui_label );
            }
            if ( button->key->left ) {
                ui_left = gtk_label_new( NULL );

                pango_attributes = ui_load__pango_attrs_common_new();
                pango_attr_list_insert( pango_attributes, pango_attr_size_new( ( tiny_font_size / 1.8 ) * PANGO_SCALE ) );
                pango_attr_list_insert( pango_attributes, pango_attr_weight_new( PANGO_WEIGHT_BOLD ) );
                fgcolor = &( ui->colors[ left_color ] );
                pango_attr_list_insert( pango_attributes, pango_attr_foreground_new( fgcolor->red, fgcolor->green, fgcolor->blue ) );

                gtk_label_set_attributes( GTK_LABEL( ui_left ), pango_attributes );

                gtk_label_set_use_markup( GTK_LABEL( ui_left ), true );
                gtk_label_set_markup( GTK_LABEL( ui_left ), button->key->left );

                if ( button->key->right ) {
                    ui_right = gtk_label_new( NULL );

                    pango_attributes = ui_load__pango_attrs_common_new();
                    pango_attr_list_insert( pango_attributes, pango_attr_size_new( ( tiny_font_size / 1.8 ) * PANGO_SCALE ) );
                    pango_attr_list_insert( pango_attributes, pango_attr_weight_new( PANGO_WEIGHT_BOLD ) );
                    fgcolor = &( ui->colors[ right_color ] );
                    pango_attr_list_insert( pango_attributes, pango_attr_foreground_new( fgcolor->red, fgcolor->green, fgcolor->blue ) );

                    gtk_label_set_attributes( GTK_LABEL( ui_right ), pango_attributes );

                    gtk_label_set_use_markup( GTK_LABEL( ui_right ), true );
                    gtk_label_set_markup( GTK_LABEL( ui_right ), button->key->right );
                }
                if ( button->key->right ) {
                    gtk_widget_size_request( ui_left, &widget_size );
                    gtk_widget_size_request( ui_right, &widget2_size );

                    x = ui->kb_x_offset + button->key->x;
                    y = ui->kb_y_offset + button->key->y - widget_size.height - 2;

                    x2 = ui->kb_x_offset + button->key->x + button->key->width - widget2_size.width;
                    y2 = ui->kb_y_offset + button->key->y - widget2_size.height - 2;

                    if ( widget_size.width + widget2_size.width > button->key->width ) {
                        x -= ( ( widget_size.width + widget2_size.width ) - button->key->width ) / 2;
                        x2 += ( ( widget_size.width + widget2_size.width ) - button->key->width ) / 2;
                    }

                    gtk_fixed_put( GTK_FIXED( fixed_widgets_container ), ui_left, x, y );
                    gtk_fixed_put( GTK_FIXED( fixed_widgets_container ), ui_right, x2, y2 );
                } else {
                    gtk_widget_size_request( ui_left, &widget_size );
                    x = ui->kb_x_offset + button->key->x + ( ( button->key->width - widget_size.width ) / 2 );
                    y = ui->kb_y_offset + button->key->y - widget_size.height - 2;
                    gtk_fixed_put( GTK_FIXED( fixed_widgets_container ), ui_left, x, y );
                }
            }
            if ( button->key->letter ) {
                ui_letter = gtk_label_new( NULL );

                pango_attributes = ui_load__pango_attrs_common_new();
                pango_attr_list_insert( pango_attributes, pango_attr_size_new( ( button->key->letter_size / 1.8 ) * PANGO_SCALE ) );
                pango_attr_list_insert( pango_attributes, pango_attr_weight_new( PANGO_WEIGHT_BOLD ) );
                fgcolor = &( ui->colors[ UI_COLOR_YELLOW ] );
                pango_attr_list_insert( pango_attributes, pango_attr_foreground_new( fgcolor->red, fgcolor->green, fgcolor->blue ) );

                gtk_label_set_attributes( GTK_LABEL( ui_letter ), pango_attributes );

                gtk_label_set_use_markup( GTK_LABEL( ui_letter ), true );
                gtk_label_set_markup( GTK_LABEL( ui_letter ), button->key->letter );

                gtk_widget_size_request( ui_letter, &widget_size );

                x = ui->kb_x_offset + button->key->x + button->key->width;
                y = ui->kb_y_offset + button->key->y + button->key->height - ( widget_size.height / 2 );
                gtk_fixed_put( GTK_FIXED( fixed_widgets_container ), ui_letter, x, y );
            }
            if ( button->key->below ) {
                ui_below = gtk_label_new( NULL );

                pango_attributes = ui_load__pango_attrs_common_new();
                pango_attr_list_insert( pango_attributes, pango_attr_size_new( ( tiny_font_size / 1.8 ) * PANGO_SCALE ) );
                pango_attr_list_insert( pango_attributes, pango_attr_weight_new( PANGO_WEIGHT_BOLD ) );
                fgcolor = &( ui->colors[ below_color ] );
                pango_attr_list_insert( pango_attributes, pango_attr_foreground_new( fgcolor->red, fgcolor->green, fgcolor->blue ) );

                gtk_label_set_attributes( GTK_LABEL( ui_below ), pango_attributes );

                gtk_label_set_use_markup( GTK_LABEL( ui_below ), true );
                gtk_label_set_markup( GTK_LABEL( ui_below ), button->key->below );

                gtk_widget_size_request( ui_below, &widget_size );
                x = ui->kb_x_offset + button->key->x + ( ( button->key->width - widget_size.width ) / 2 );
                y = ui->kb_y_offset + button->key->y + button->key->height + 2;
                gtk_fixed_put( GTK_FIXED( fixed_widgets_container ), ui_below, x, y );
            }

            button->box = gtk_event_box_new();
            gtk_event_box_set_visible_window( GTK_EVENT_BOX( button->box ), true );
            gtk_event_box_set_above_child( GTK_EVENT_BOX( button->box ), false );
            gtk_container_add( GTK_CONTAINER( button->box ), button->button );

            g_signal_connect( G_OBJECT( button->button ), "button-press-event", G_CALLBACK( react_to_button_press ), button );
            g_signal_connect( G_OBJECT( button->button ), "button-release-event", G_CALLBACK( react_to_button_release ), button );
            g_signal_connect( G_OBJECT( button->button ), "leave-notify-event", G_CALLBACK( react_to_button_leave ), button );

            gtk_widget_add_events( button->button, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_LEAVE_NOTIFY_MASK );

            gtk_widget_set_size_request( button->box, button->key->width, button->key->height );
            gtk_fixed_put( GTK_FIXED( fixed_widgets_container ), button->box, ui->kb_x_offset + button->key->x,
                           ui->kb_y_offset + button->key->y );
        }
    }

    // right-click menu
    {
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
    }

    // finally show the window
    gtk_widget_show_all( ui->window );

    return 0;
}

static int ui_save( x49gp_module_t* module, GKeyFile* keyfile ) { return 0; }

static void _draw_pixel( cairo_surface_t* target, int x, int y, int w, int h, GdkColor* color )
{
    cairo_t* cr = cairo_create( target );

    cairo_set_source_rgb( cr, color->red / 65535.0, color->green / 65535.0, color->blue / 65535.0 );
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
        gtk_widget_set_visible( ui->ui_ann_left, x49gp_get_pixel_color( lcd, 131, 1 ) > 0 );
        gtk_widget_set_visible( ui->ui_ann_right, x49gp_get_pixel_color( lcd, 131, 2 ) > 0 );
        gtk_widget_set_visible( ui->ui_ann_alpha, x49gp_get_pixel_color( lcd, 131, 3 ) > 0 );
        gtk_widget_set_visible( ui->ui_ann_battery, x49gp_get_pixel_color( lcd, 131, 4 ) > 0 );
        gtk_widget_set_visible( ui->ui_ann_busy, x49gp_get_pixel_color( lcd, 131, 5 ) > 0 );
        gtk_widget_set_visible( ui->ui_ann_io, x49gp_get_pixel_color( lcd, 131, 0 ) > 0 );

        for ( int y = 0; y < ( ui->lcd_height / LCD_PIXEL_SCALE ); y++ )
            for ( int x = 0; x < ( ui->lcd_width / LCD_PIXEL_SCALE ); x++ )
                _draw_pixel( ui->lcd_surface, LCD_PIXEL_SCALE * x, LCD_PIXEL_SCALE * y, LCD_PIXEL_SCALE, LCD_PIXEL_SCALE,
                             &( ui->colors[ UI_COLOR_GRAYSCALE_0 + x49gp_get_pixel_color( lcd, x, y ) ] ) );
    }

    GdkRectangle rect;
    rect.x = 0;
    rect.y = 0;
    rect.width = ui->lcd_width;
    rect.height = ui->lcd_height;

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
