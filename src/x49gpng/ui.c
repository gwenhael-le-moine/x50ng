/* $Id: ui.c,v 1.34 2008/12/11 12:18:17 ecd Exp $
 */

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
#include "x49gp_ui.h"
#include "s3c2410.h"
#include "bitmaps.h"
#include "bitmap_font.h"
#include "symbol.h"
#include "glyphname.h"

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
     .layout = UI_LAYOUT_LEFT,
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
     .layout = UI_LAYOUT_LEFT,
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
     .layout = UI_LAYOUT_LEFT,
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
     .layout = UI_LAYOUT_LEFT,
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
     .layout = UI_LAYOUT_LEFT,
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
     .layout = UI_LAYOUT_LEFT,
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
     .layout = UI_LAYOUT_BELOW,
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
     .layout = UI_LAYOUT_BELOW,
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
     .layout = UI_LAYOUT_BELOW,
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
    {.label = "V\\kern-1 AR",
     .letter = "J",
     .left = "UPDIR",
     .right = "COPY",
     .below = NULL,
     .color = UI_COLOR_WHITE,
     .font_size = 10.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 10.0,
     .layout = UI_LAYOUT_BELOW,
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
    {.label = "STO \\triangleright",
     .letter = "K",
     .left = "RCL",
     .right = "CUT",
     .below = NULL,
     .color = UI_COLOR_WHITE,
     .font_size = 10.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 10.0,
     .layout = UI_LAYOUT_BELOW,
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
     .layout = UI_LAYOUT_BELOW,
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

    {.label = "\\arrowup",
     .letter = NULL,
     .left = NULL,
     .right = NULL,
     .below = NULL,
     .color = UI_COLOR_BLACK,
     .font_size = 10.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 0.0,
     .layout = 0,
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
    {.label = "\\arrowleft",
     .letter = NULL,
     .left = NULL,
     .right = NULL,
     .below = NULL,
     .color = UI_COLOR_BLACK,
     .font_size = 10.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 0.0,
     .layout = 0,
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
    {.label = "\\arrowdown",
     .letter = NULL,
     .left = NULL,
     .right = NULL,
     .below = NULL,
     .color = UI_COLOR_BLACK,
     .font_size = 10.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 0.0,
     .layout = 0,
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
    {.label = "\\arrowright",
     .letter = NULL,
     .left = NULL,
     .right = NULL,
     .below = NULL,
     .color = UI_COLOR_BLACK,
     .font_size = 10.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 0.0,
     .layout = 0,
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
     .layout = UI_LAYOUT_LEFT,
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
    {.label = "EV\\kern-1 AL",
     .letter = "N",
     .left = "PRG",
     .right = "CHARS",
     .below = NULL,
     .color = UI_COLOR_WHITE,
     .font_size = 12.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 12.0,
     .layout = UI_LAYOUT_LEFT,
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
    {.label = "\\tick",
     .letter = "O",
     .left = "MTRW",
     .right = "EQW",
     .below = NULL,
     .color = UI_COLOR_WHITE,
     .font_size = 12.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 12.0,
     .layout = UI_LAYOUT_LEFT,
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
    {.label = "S\\kern-1 Y\\kern-1 M\\kern-1 B",
     .letter = "P",
     .left = "MTH",
     .right = "CAT",
     .below = NULL,
     .color = UI_COLOR_WHITE,
     .font_size = 12.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 12.0,
     .layout = UI_LAYOUT_LEFT,
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
    {.label = "\\arrowleftdblfull",
     .letter = NULL,
     .left = "DEL",
     .right = "CLEAR",
     .below = NULL,
     .color = UI_COLOR_WHITE,
     .font_size = 12.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 0.0,
     .layout = 0,
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

    {.label = "Y\\super x\\/super",
     .letter = "Q",
     .left = "\\math_e\\xsuperior",
     .right = "LN",
     .below = NULL,
     .color = UI_COLOR_WHITE,
     .font_size = 12.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 12.0,
     .layout = UI_LAYOUT_LEFT,
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
    {.label = "\\radical\\overscore\\kern-7 X",
     .letter = "R",
     .left = "\\math_x\\twosuperior",
     .right = "\\xsuperior\\kern-4\\math_radical\\overscore\\kern-5\\math_y",
     .below = NULL,
     .color = UI_COLOR_WHITE,
     .font_size = 12.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 12.0,
     .layout = UI_LAYOUT_LEFT,
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
     .right = "\\math_summation",
     .below = NULL,
     .color = UI_COLOR_WHITE,
     .font_size = 12.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 12.0,
     .layout = UI_LAYOUT_LEFT,
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
     .right = "\\math_partialdiff",
     .below = NULL,
     .color = UI_COLOR_WHITE,
     .font_size = 12.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 12.0,
     .layout = UI_LAYOUT_LEFT,
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
     .right = "\\math_integral",
     .below = NULL,
     .color = UI_COLOR_WHITE,
     .font_size = 12.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 12.0,
     .layout = UI_LAYOUT_LEFT,
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
     .left = "10\\xsuperior",
     .right = "LOG",
     .below = NULL,
     .color = UI_COLOR_WHITE,
     .font_size = 12.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 12.0,
     .layout = UI_LAYOUT_LEFT,
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
    {.label = "+\\divisionslash\\minus",
     .letter = "W",
     .left = "\\math_notequal",
     .right = "\\math_equal",
     .below = NULL,
     .color = UI_COLOR_WHITE,
     .font_size = 12.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 12.0,
     .layout = UI_LAYOUT_LEFT,
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
    {.label = "X",
     .letter = "X",
     .left = "\\math_lessequal",
     .right = "\\math_less",
     .below = NULL,
     .color = UI_COLOR_WHITE,
     .font_size = 12.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 12.0,
     .layout = UI_LAYOUT_LEFT,
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
    {.label = "1/X",
     .letter = "Y",
     .left = "\\math_greaterequal",
     .right = "\\math_greater",
     .below = NULL,
     .color = UI_COLOR_WHITE,
     .font_size = 12.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 12.0,
     .layout = UI_LAYOUT_LEFT,
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
    {.label = "\\divide",
     .letter = "Z",
     .left = "ABS",
     .right = "ARG",
     .below = NULL,
     .color = UI_COLOR_WHITE,
     .font_size = 19.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 12.0,
     .layout = UI_LAYOUT_LEFT_NO_SPACE,
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
     .font_size = 12.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 0.0,
     .layout = 0,
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
     .layout = 0,
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
     .left = "EXP&LN",
     .right = "TRIG",
     .below = NULL,
     .color = UI_COLOR_WHITE,
     .font_size = 19.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 0.0,
     .layout = 0,
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
     .layout = 0,
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
    {.label = "\\multiply",
     .letter = NULL,
     .left = "[ ]",
     .right = "\" \"",
     .below = NULL,
     .color = UI_COLOR_WHITE,
     .font_size = 19.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 0.0,
     .layout = 0,
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

    {.label = "\\uparrowleft",
     .letter = NULL,
     .left = NULL,
     .right = NULL,
     .below = NULL,
     .color = UI_COLOR_BLACK,
     .font_size = 19.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 0.0,
     .layout = 0,
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
     .layout = 0,
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
     .layout = 0,
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
     .layout = 0,
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
    {.label = "\\minus",
     .letter = NULL,
     .left = "( )",
     .right = "_",
     .below = NULL,
     .color = UI_COLOR_WHITE,
     .font_size = 19.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 0.0,
     .layout = 0,
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

    {.label = "\\uparrowright",
     .letter = NULL,
     .left = NULL,
     .right = NULL,
     .below = NULL,
     .color = UI_COLOR_BLACK,
     .font_size = 19.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 0.0,
     .layout = 0,
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
     .layout = 0,
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
     .layout = 0,
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
     .left = "\\math_numbersign",
     .right = "BASE",
     .below = NULL,
     .color = UI_COLOR_WHITE,
     .font_size = 19.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 0.0,
     .layout = 0,
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
     .right = "\\guillemotleft\\ \\guillemotright",
     .below = NULL,
     .color = UI_COLOR_WHITE,
     .font_size = 19.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 0.0,
     .layout = 0,
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
     .layout = 0,
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
     .left = "\\math_infinity",
     .right = "\\math_arrowright",
     .below = NULL,
     .color = UI_COLOR_WHITE,
     .font_size = 19.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 0.0,
     .layout = 0,
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
    {.label = "\\bullet",
     .letter = NULL,
     .left = ": :",
     .right = "\\math_downarrowleft",
     .below = NULL,
     .color = UI_COLOR_WHITE,
     .font_size = 19.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 0.0,
     .layout = 0,
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
     .left = "\\math_pi",
     .right = "\\large_comma",
     .below = NULL,
     .color = UI_COLOR_WHITE,
     .font_size = 12.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 0.0,
     .layout = 0,
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
     .right = "\\arrowright NUM",
     .below = NULL,
     .color = UI_COLOR_WHITE,
     .font_size = 12.0,
     .font_weight = CAIRO_FONT_WEIGHT_BOLD,
     .letter_size = 0.0,
     .layout = 0,
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

typedef struct {
    uint32_t magic;
    uint32_t length;
    uint32_t pixdata_type;
    uint32_t rowstride;
    uint32_t width;
    uint32_t height;
} pixbuf_inline_data_t;

/*************/
/* functions */
/*************/
static inline unsigned color2rgb( x49gp_ui_t* ui, int color )
{
    return 0x000000 | ( ui->colors[ color ].red << 8 ) | ( ui->colors[ color ].green << 16 ) | ( ui->colors[ color ].blue );
}

static void x49gp_ui_color_init( GdkColor* color, u8 red, u8 green, u8 blue )
{
    color->red = ( red << 8 ) | red;
    color->green = ( green << 8 ) | green;
    color->blue = ( blue << 8 ) | blue;
}

static void x49gp_ui_style_init( GtkStyle* style, GtkWidget* widget, GdkColor* fg, GdkColor* bg )
{
    for ( int i = 0; i < 5; i++ ) {
        style->fg[ i ] = *fg;
        style->bg[ i ] = *bg;

        style->text[ i ] = style->fg[ i ];
        style->base[ i ] = style->bg[ i ];
    }

    style->xthickness = 0;
    style->ythickness = 0;
}

static int x49gp_ui_button_pixmaps_init( x49gp_t* x49gp, x49gp_ui_button_t* button, x49gp_ui_color_t color )
{
    x49gp_ui_t* ui = x49gp->ui;
    GdkPixbuf* src;
    cairo_t* cr;
    GtkStyle* style = gtk_style_new();
    int y;

    x49gp_ui_style_init( style, button->button, &ui->colors[ button->key->color ], &ui->colors[ UI_COLOR_BLACK ] );

    for ( int i = 0; i < 5; i++ ) {
        style->bg_pixmap[ i ] = gdk_pixmap_new( gtk_widget_get_window( ui->window ), button->key->width, button->key->height, -1 );

        y = ui->kb_y_offset + button->key->y;

        if ( i == GTK_STATE_ACTIVE )
            y += 1;

        src = gdk_pixbuf_new_subpixbuf( ui->bg_pixbuf, ui->kb_x_offset + button->key->x, y, button->key->width, button->key->height );

        cr = gdk_cairo_create( style->bg_pixmap[ i ] );
        gdk_cairo_set_source_pixbuf( cr, src, 0, 0 );
        cairo_paint( cr );
        cairo_destroy( cr );

        g_object_unref( src );
    }

    gtk_widget_set_style( button->button, style );

    return 0;
}

static void x49gp_ui_symbol_path( cairo_t* cr, double size, double xoffset, double yoffset, const x49gp_symbol_t* symbol )
{
    const symbol_path_t* path;
    const cairo_path_data_t* data;

    path = symbol->path;
    if ( NULL == path )
        return;

    cairo_move_to( cr, xoffset, yoffset );

    for ( int i = 0; i < path->num_data; i += path->data[ i ].header.length ) {
        data = &path->data[ i ];

        switch ( data->header.type ) {
            case CAIRO_PATH_MOVE_TO:
                cairo_rel_move_to( cr, size * data[ 1 ].point.x, -size * data[ 1 ].point.y );
                break;
            case CAIRO_PATH_LINE_TO:
                cairo_rel_line_to( cr, size * data[ 1 ].point.x, -size * data[ 1 ].point.y );
                break;
            case CAIRO_PATH_CURVE_TO:
                cairo_rel_curve_to( cr, size * data[ 1 ].point.x, -size * data[ 1 ].point.y, size * data[ 2 ].point.x,
                                    -size * data[ 2 ].point.y, size * data[ 3 ].point.x, -size * data[ 3 ].point.y );
                break;
            case CAIRO_PATH_CLOSE_PATH:
                cairo_close_path( cr );
                break;
        }
    }
}

/* static void x49gp_ui_draw_symbol( cairo_t* cr, GdkColor* color, double size, double line_width, gboolean fill, double xoffset, */
/*                                   double yoffset, const x49gp_symbol_t* symbol ) */
/* { */
/*     cairo_set_line_cap( cr, CAIRO_LINE_CAP_BUTT ); */
/*     cairo_set_line_join( cr, CAIRO_LINE_JOIN_MITER ); */
/*     cairo_set_line_width( cr, line_width ); */
/*     cairo_set_source_rgb( cr, ( ( double )color->red ) / 65535.0, ( ( double )color->green ) / 65535.0, */
/*                           ( ( double )color->blue ) / 65535.0 ); */

/*     x49gp_ui_symbol_path( cr, size, xoffset, yoffset, symbol ); */

/*     if ( fill ) */
/*         cairo_fill( cr ); */
/*     else */
/*         cairo_stroke( cr ); */
/* } */

static bool x49gp_ui_lookup_glyph( const char* name, int namelen, gunichar* glyph )
{
    for ( int i = 0; i < NR_GLYPHNAMES; i++ ) {
        if ( ( strlen( x49gp_glyphs[ i ].name ) == namelen ) && !strncmp( x49gp_glyphs[ i ].name, name, namelen ) ) {
            if ( glyph )
                *glyph = x49gp_glyphs[ i ].unichar;

            return true;
        }
    }

    return false;
}

static int __text_strlen( const char* text )
{
    const char *p, *q;
    char c;
    int namelen;
    int n = 0;

    p = text;
    while ( ( c = *p++ ) ) {
        if ( c != '\\' ) {
            n++;
            continue;
        }

        q = p;
        while ( *q ) {
            if ( ( *q == '\\' ) || ( *q == ' ' ) )
                break;
            q++;
        }
        if ( q == p ) {
            n++;
            p++;
            continue;
        }
        namelen = q - p;
        if ( *q == ' ' )
            q++;

        if ( symbol_lookup_glyph_by_name( p, namelen, NULL ) ) {
            p = q;
            n++;
            continue;
        }

        if ( x49gp_ui_lookup_glyph( p, namelen, NULL ) ) {
            p = q;
            n++;
            continue;
        }

        /*
         * Insert symbol .notdef here...
         */
        p = q;
        n++;
    }

    return n;
}

/* used in x49gp_ui_vtext_path() */
static int _text_to_ucs4( const char* text, gunichar** ucs4p )
{
    const char *p, *q;
    gunichar glyph;
    gunichar* ucs4;
    char c;
    int namelen;
    int i = 0;
    int n = __text_strlen( text );

    if ( n <= 0 )
        return n;

    ucs4 = malloc( n * sizeof( gunichar ) );

    p = text;
    while ( ( c = *p++ ) ) {
        if ( i == n ) {
            free( ucs4 );
            return -1;
        }

        if ( c != '\\' ) {
            ucs4[ i++ ] = c;
            continue;
        }

        q = p;
        while ( *q ) {
            if ( ( *q == '\\' ) || ( *q == ' ' ) )
                break;
            q++;
        }
        if ( q == p ) {
            ucs4[ i++ ] = *p++;
            continue;
        }
        namelen = q - p;
        if ( *q == ' ' )
            q++;

        if ( symbol_lookup_glyph_by_name( p, namelen, &glyph ) ) {
            ucs4[ i++ ] = glyph;
            p = q;
            continue;
        }

        if ( x49gp_ui_lookup_glyph( p, namelen, &glyph ) ) {
            ucs4[ i++ ] = glyph;
            p = q;
            continue;
        }

        /*
         * Insert symbol .notdef here...
         */
        ucs4[ i++ ] = 0xe000;
        p = q;
    }

    *ucs4p = ucs4;

    return n;
}

static void x49gp_ui_vtext_path( cairo_t* cr, const char* family, double size, double x, double y, int n, va_list ap )
{
    cairo_text_extents_t extents;
    cairo_font_weight_t weight;
    cairo_font_slant_t slant;
    const x49gp_symbol_t* symbol;
    const char* text;
    gunichar* ucs4;
    char out[ 8 ];
    int bytes;
    int len;

    for ( int i = 0; i < n; i++ ) {
        slant = va_arg( ap, cairo_font_slant_t );
        weight = va_arg( ap, cairo_font_weight_t );
        text = va_arg( ap, const char* );

        cairo_select_font_face( cr, family, slant, weight );
        cairo_set_font_size( cr, size );

        ucs4 = NULL;
        len = _text_to_ucs4( text, &ucs4 );
        if ( len <= 0 )
            continue;

        for ( int j = 0; j < len; j++ ) {
            if ( g_unichar_type( ucs4[ j ] ) == G_UNICODE_PRIVATE_USE ) {
                /*
                 * Draw Symbol, Increment x...
                 */
                symbol = symbol_get_by_glyph( ucs4[ j ] );
                if ( NULL == symbol )
                    symbol = symbol_get_by_glyph( 0xe000 );

                size *= symbol->prescale;

                x49gp_ui_symbol_path( cr, size, x, y, symbol );

                x += size * symbol->x_advance;
                y -= size * symbol->y_advance;

                size *= symbol->postscale;

                if ( symbol->prescale * symbol->postscale != 1. )
                    cairo_set_font_size( cr, size );

                continue;
            }

            bytes = g_unichar_to_utf8( ucs4[ j ], out );
            out[ bytes ] = '\0';

            cairo_text_extents( cr, out, &extents );

            cairo_move_to( cr, x, y );

            cairo_text_path( cr, out );

            x += extents.x_advance;
            y += extents.y_advance;
        }

        free( ucs4 );
    }
}

static void x49gp_ui_text_size( cairo_t* cr, const char* family, double size, double* x_bearing, double* y_bearing, double* width,
                                double* height, double* ascent, double* descent, int n, ... )
{
    va_list ap0, ap1;
    cairo_font_extents_t font_extents;
    cairo_font_weight_t weight;
    cairo_font_slant_t slant;
    double x1, y1, x2, y2, a, d;
    const char* text;

    if ( n < 1 )
        return;

    va_start( ap0, n );
    va_copy( ap1, ap0 );

    x49gp_ui_vtext_path( cr, family, size, 0.0, 0.0, n, ap0 );

    va_end( ap0 );

    cairo_fill_extents( cr, &x1, &y1, &x2, &y2 );

    if ( y2 < 0.0 )
        y2 = 0.0;

    a = 0.0;
    d = 0.0;

    for ( int i = 0; i < n; i++ ) {
        slant = va_arg( ap1, cairo_font_slant_t );
        weight = va_arg( ap1, cairo_font_weight_t );
        text = va_arg( ap1, const char* );
        ( void )text;

        cairo_select_font_face( cr, family, slant, weight );
        cairo_set_font_size( cr, size );

        cairo_font_extents( cr, &font_extents );

        /*
         * Cairo seems to return overall height in ascent,
         * so fix this by calculating ascent = height - descent.
         */
        if ( font_extents.ascent - font_extents.descent > a )
            a = font_extents.ascent - font_extents.descent;
        if ( font_extents.descent > -d )
            d = -font_extents.descent;
    }

    *x_bearing = x1;
    *y_bearing = y2;
    *width = x2 - x1;
    *height = y2 - y1;
    *ascent = a;
    *descent = d;

    va_end( ap1 );
}

static void x49gp_ui_draw_text( cairo_t* cr, GdkColor* color, const char* family, double size, double line_width, int xoffset, int yoffset,
                                int n, ... )
{
    va_list ap;

    if ( n < 1 )
        return;

    va_start( ap, n );

    cairo_set_line_width( cr, line_width );
    cairo_set_source_rgb( cr, ( ( double )color->red ) / 65535.0, ( ( double )color->green ) / 65535.0,
                          ( ( double )color->blue ) / 65535.0 );

    x49gp_ui_vtext_path( cr, family, size, xoffset, yoffset, n, ap );

    if ( line_width == 0.0 )
        cairo_fill( cr );
    else
        cairo_stroke( cr );

    va_end( ap );
}

static unsigned char bitmap_font_lookup_glyph( const bitmap_font_t* font, const char* name, int namelen )
{
    for ( int i = 0; font->glyphs[ i ].name; i++ )
        if ( ( strlen( font->glyphs[ i ].name ) == namelen ) && !strncmp( font->glyphs[ i ].name, name, namelen ) )
            return i;

    return 0;
}

static unsigned char bitmap_font_lookup_ascii( const bitmap_font_t* font, char c )
{
    int namelen = 0;
    char* name;

    switch ( c ) {
        case ' ':
            name = "space";
            break;
        case '!':
            name = "exclam";
            break;
        case '"':
            name = "quotedbl";
            break;
        case '#':
            name = "numbersign";
            break;
        case '$':
            name = "dollar";
            break;
        case '%':
            name = "percent";
            break;
        case '&':
            name = "ampersand";
            break;
        case '(':
            name = "parenleft";
            break;
        case ')':
            name = "parenright";
            break;
        case '*':
            name = "asterisk";
            break;
        case '+':
            name = "plus";
            break;
        case ',':
            name = "comma";
            break;
        case '-':
            name = "hyphen";
            break;
        case '.':
            name = "period";
            break;
        case '/':
            name = "slash";
            break;
        case '0':
            name = "zero";
            break;
        case '1':
            name = "one";
            break;
        case '2':
            name = "two";
            break;
        case '3':
            name = "three";
            break;
        case '4':
            name = "four";
            break;
        case '5':
            name = "five";
            break;
        case '6':
            name = "six";
            break;
        case '7':
            name = "seven";
            break;
        case '8':
            name = "eight";
            break;
        case '9':
            name = "nine";
            break;
        case ':':
            name = "colon";
            break;
        case ';':
            name = "semicolon";
            break;
        case '<':
            name = "less";
            break;
        case '=':
            name = "equal";
            break;
        case '>':
            name = "greater";
            break;
        case '?':
            name = "question";
            break;
        case '@':
            name = "at";
            break;
        case '[':
            name = "bracketleft";
            break;
        case '\\':
            name = "backslash";
            break;
        case ']':
            name = "bracketright";
            break;
        case '^':
            name = "asciicircum";
            break;
        case '_':
            name = "underscore";
            break;
        case '`':
            name = "quoteleft";
            break;
        case '{':
            name = "braceleft";
            break;
        case '|':
            name = "bar";
            break;
        case '}':
            name = "braceright";
            break;
        case '~':
            name = "asciitilde";
            break;
        default:
            name = &c;
            namelen = 1;
            break;
    }

    if ( 0 == namelen )
        namelen = strlen( name );

    return bitmap_font_lookup_glyph( font, name, namelen );
}

static int bitmap_font_strlen( const char* text )
{
    const char *p, *q;
    char c;
    int n = 0;

    p = text;
    while ( ( c = *p++ ) ) {
        if ( c != '\\' ) {
            n++;
            continue;
        }

        q = p;
        while ( *q ) {
            if ( ( *q == '\\' ) || ( *q == ' ' ) )
                break;
            q++;
        }
        if ( q == p ) {
            n++;
            p++;
            continue;
        }
        if ( *q == ' ' )
            q++;

        n++;
        p = q;
    }

    return n;
}

static int bitmap_font_text_to_glyphs( const bitmap_font_t* font, const char* text, unsigned char** glyphp )
{
    unsigned char* glyphs;
    const char *p, *q;
    unsigned char c;
    int namelen;
    int i, n;

    n = bitmap_font_strlen( text );
    if ( n <= 0 )
        return n;

    glyphs = malloc( n );

    i = 0;

    p = text;
    while ( ( c = *p++ ) ) {
        if ( i == n ) {
            free( glyphs );
            return -1;
        }

        if ( c != '\\' ) {
            glyphs[ i++ ] = bitmap_font_lookup_ascii( font, c );
            continue;
        }

        q = p;
        while ( *q ) {
            if ( ( *q == '\\' ) || ( *q == ' ' ) )
                break;
            q++;
        }
        if ( q == p ) {
            glyphs[ i++ ] = bitmap_font_lookup_ascii( font, *p++ );
            continue;
        }
        namelen = q - p;
        if ( *q == ' ' )
            q++;

        glyphs[ i++ ] = bitmap_font_lookup_glyph( font, p, namelen );
        p = q;
    }

    *glyphp = glyphs;
    return n;
}

static void bitmap_font_text_size( const bitmap_font_t* font, const char* text, int* width, int* height, int* ascent, int* descent )
{
    const bitmap_glyph_t* glyph;
    unsigned char* glyphs;
    int n, w, a, d;

    w = 0;
    a = 0;
    d = 0;

    n = bitmap_font_text_to_glyphs( font, text, &glyphs );

    for ( int i = 0; i < n; i++ ) {
        glyph = &font->glyphs[ glyphs[ i ] ];

        w += glyph->width;

        if ( glyph->ascent > a )
            a = glyph->ascent;
        if ( glyph->descent < d )
            d = glyph->descent;
    }

    *width = w - 1;
    *height = font->ascent - font->descent;
    *ascent = a;
    *descent = d;

    if ( n > 0 )
        free( glyphs );
}

static void bitmap_font_draw_text( GdkDrawable* drawable, GdkColor* color, const bitmap_font_t* font, int x, int y, const char* text )
{
    const bitmap_glyph_t* glyph;
    unsigned char* glyphs;
    GdkBitmap* bitmap;
    GdkGC* gc;
    int n, w, h;

    gc = gdk_gc_new( drawable );
    gdk_gc_set_rgb_fg_color( gc, color );

    n = bitmap_font_text_to_glyphs( font, text, &glyphs );

    for ( int i = 0; i < n; i++ ) {
        glyph = &font->glyphs[ glyphs[ i ] ];

        w = glyph->width - glyph->kern;
        h = glyph->ascent - glyph->descent;

        if ( w <= 0 || h <= 0 ) {
            x += glyph->width;
            continue;
        }

        bitmap = gdk_bitmap_create_from_data( NULL, ( char* )glyph->bits, w, h );

        gdk_gc_set_ts_origin( gc, x + glyph->kern, y + font->ascent - glyph->ascent );
        gdk_gc_set_stipple( gc, bitmap );
        gdk_gc_set_fill( gc, GDK_STIPPLED );

        gdk_draw_rectangle( drawable, gc, true, x + glyph->kern, y + font->ascent - glyph->ascent, w, h );

        g_object_unref( bitmap );

        x += glyph->width;
    }

    g_object_unref( gc );

    if ( n > 0 )
        free( glyphs );
}

void x49gp_ui_show_error( x49gp_t* x49gp, const char* text )
{
    GtkWidget* dialog;
    x49gp_ui_t* ui = x49gp->ui;

    dialog =
        gtk_message_dialog_new( GTK_WINDOW( ui->window ), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "%s", text );

    gtk_dialog_run( GTK_DIALOG( dialog ) );
    gtk_widget_destroy( dialog );
}

static gboolean x49gp_ui_button_press( GtkWidget* widget, GdkEventButton* event, gpointer user_data )
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

static void x49gp_release_single_button( x49gp_ui_button_t* button, x49gp_ui_button_t* cause )
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

static void x49gp_release_all_buttons( x49gp_t* x49gp, x49gp_ui_button_t* cause )
{
    x49gp_ui_button_t* button;
    x49gp_ui_t* ui = x49gp->ui;

    for ( int i = 0; i < ui->nr_buttons; i++ ) {
        button = &ui->buttons[ i ];

        if ( !button->down )
            continue;

        x49gp_release_single_button( button, cause );
    }
}

static gboolean x49gp_ui_button_release( GtkWidget* widget, GdkEventButton* event, gpointer user_data )
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
        x49gp_release_all_buttons( x49gp, button );
    else
        x49gp_release_single_button( button, button );

    return false;
}

static gboolean x49gp_ui_show_menu( GtkWidget* widget, GdkEventButton* event, gpointer user_data )
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

static gboolean x49gp_ui_button_leave( GtkWidget* widget, GdkEventCrossing* event, gpointer user_data )
{
    x49gp_ui_button_t* button = user_data;

    if ( event->type != GDK_LEAVE_NOTIFY )
        return false;

    if ( !button->hold )
        return false;

    return true;
}

static gboolean x49gp_ui_focus_lost( GtkWidget* widget, GdkEventFocus* event, gpointer user_data )
{
    x49gp_t* x49gp = user_data;
    x49gp_ui_t* ui = x49gp->ui;

    if ( event->type != GDK_FOCUS_CHANGE )
        return false;

    ui->buttons_down = 0;
    x49gp_release_all_buttons( x49gp, NULL );

    return false;
}

static void x49gp_ui_popup_at_widget( GtkMenu* menu, gint* x, gint* y, gboolean* push_in, gpointer user_data )
{
    GtkWidget* widget = GTK_WIDGET( user_data );
    GtkAllocation widget_allocation;

    gtk_widget_get_allocation( widget, &widget_allocation );

    gdk_window_get_origin( gtk_widget_get_window( widget ), x, y );
    *x += widget_allocation.x;
    *y += widget_allocation.y;
}

static void x49gp_ui_choose_file( x49gp_t* x49gp, const char* prompt, GtkFileChooserAction action, char** filename )
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

void x49gp_ui_open_firmware( x49gp_t* x49gp, char** filename )
{
    x49gp_ui_choose_file( x49gp, "Choose firmware ...", GTK_FILE_CHOOSER_ACTION_OPEN, filename );
}

static void x49gp_ui_mount_sd_folder( GtkMenuItem* menuitem, gpointer user_data )
{
    x49gp_t* x49gp = user_data;
    char* filename;

    x49gp_ui_choose_file( x49gp, "Choose SD folder ...", GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER, &filename );
    if ( filename != NULL )
        s3c2410_sdi_mount( x49gp, filename );
}

static void x49gp_ui_mount_sd_image( GtkMenuItem* menuitem, gpointer user_data )
{
    x49gp_t* x49gp = user_data;
    char* filename;

    x49gp_ui_choose_file( x49gp, "Choose SD image ...", GTK_FILE_CHOOSER_ACTION_OPEN, &filename );
    if ( filename != NULL )
        s3c2410_sdi_mount( x49gp, filename );
}

static void x49gp_ui_debug( GtkMenuItem* menuitem, gpointer user_data )
{
    x49gp_t* x49gp = user_data;

    if ( x49gp->debug_port != 0 && !gdbserver_isactive() ) {
        gdbserver_start( x49gp->debug_port );
        gdb_handlesig( x49gp->env, 0 );
    }
}

static void x49gp_ui_calculator_reset( GtkMenuItem* menuitem, gpointer user_data )
{
    x49gp_t* x49gp = user_data;

    x49gp_modules_reset( x49gp, X49GP_RESET_POWER_ON );
    cpu_reset( x49gp->env );
    x49gp_set_idle( x49gp, 0 );
}

static gboolean x49gp_ui_key_event( GtkWidget* widget, GdkEventKey* event, gpointer user_data )
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
        case GDK_KP_Separator:
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

            gtk_menu_popup( GTK_MENU( ui->menu ), NULL, NULL, x49gp_ui_popup_at_widget, ui->lcd_canvas, 0, event->time );
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
            x49gp_ui_button_press( button->button, &bev, button );
            /* GTK_BUTTON( button->button )->in_button = true; */
            gtk_button_pressed( GTK_BUTTON( button->button ) );
            /* GTK_BUTTON( button->button )->in_button = save_in; */
            break;
        case GDK_KEY_RELEASE:
            bev.type = GDK_BUTTON_RELEASE;
            /* GTK_BUTTON( button->button )->in_button = true; */
            gtk_button_released( GTK_BUTTON( button->button ) );
            /* GTK_BUTTON( button->button )->in_button = save_in; */
            x49gp_ui_button_release( button->button, &bev, button );
            break;
        default:
            return false;
    }

    return true;
}

/* Draw button's pixmap onto window */
static int x49gp_button_expose_event( GtkWidget* widget, GdkEventExpose* event, gpointer user_data )
{
    x49gp_ui_button_t* button = user_data;
    GtkAllocation widget_allocation;

    gtk_widget_get_allocation( widget, &widget_allocation );

    int x = widget_allocation.x;
    int y = widget_allocation.y;

    if ( gtk_widget_get_state( widget ) == GTK_STATE_ACTIVE )
        y -= 1;

    gdk_draw_drawable( gtk_widget_get_window( widget ), gtk_widget_get_style( widget )->black_gc, button->pixmap, 0, 0, x, y,
                       widget_allocation.width, widget_allocation.height );

    return false;
}

/* Prepare button's pixmap */
static void x49gp_button_realize( GtkWidget* widget, gpointer user_data )
{
    x49gp_ui_button_t* button = user_data;
    x49gp_ui_t* ui = button->x49gp->ui;
    const x49gp_ui_key_t* key = button->key;
    cairo_t* cr;
    double xoff, yoff, width, height, ascent, descent;
    unsigned int w, h;
    int xoffset, yoffset, x, y;
    GtkAllocation widget_allocation;

    gtk_widget_get_allocation( widget, &widget_allocation );

    xoffset = widget_allocation.x;
    yoffset = widget_allocation.y;
    w = widget_allocation.width;
    h = widget_allocation.height;

    button->pixmap = gdk_pixmap_new( gtk_widget_get_style( widget )->bg_pixmap[ 0 ], w, h, -1 );

    xoffset += 2;
    yoffset += 2;
    w -= 4;
    h -= 4;

    cr = gdk_cairo_create( button->pixmap );
    cairo_set_line_cap( cr, CAIRO_LINE_CAP_BUTT );
    cairo_set_line_join( cr, CAIRO_LINE_JOIN_MITER );

    // #if DEBUG_LAYOUT /* Layout Debug */
    cairo_set_source_rgb( cr, 1.0, 1.0, 1.0 );
    cairo_set_line_width( cr, 1.0 );
    cairo_move_to( cr, xoffset, yoffset );
    cairo_line_to( cr, xoffset + w - 1, yoffset );
    cairo_line_to( cr, xoffset + w - 1, yoffset + h - 1 );
    cairo_line_to( cr, xoffset, yoffset + h - 1 );
    cairo_close_path( cr );
    cairo_stroke( cr );
    // #endif

    cairo_set_source_rgb( cr, ui->colors[ key->bg_color ].red / 65535.0, ui->colors[ key->bg_color ].green / 65535.0,
                          ui->colors[ key->bg_color ].blue / 65535.0 );
    cairo_set_line_width( cr, 1.0 );
    cairo_move_to( cr, xoffset, yoffset );
    cairo_line_to( cr, xoffset + w - 1, yoffset );
    cairo_line_to( cr, xoffset + w - 1, yoffset + h - 1 );
    cairo_line_to( cr, xoffset, yoffset + h - 1 );
    cairo_close_path( cr );
    cairo_fill( cr );

    if ( key->letter ) {
        x49gp_ui_text_size( cr, opt.font, key->letter_size, &xoff, &yoff, &width, &height, &ascent, &descent, 1, CAIRO_FONT_SLANT_NORMAL,
                            key->font_weight, key->letter );

        switch ( key->layout ) {
            case UI_LAYOUT_LEFT:
            default:
                x = ( int )floor( w - 1.0 - width - xoff + 0.5 );
                y = ( int )floor( ( h - 1.0 + ascent ) / 2.0 + 0.5 );
                w -= width;
                break;
            case UI_LAYOUT_LEFT_NO_SPACE:
                x = ( int )floor( w - 1.0 - width - xoff + 0.5 );
                y = ( int )floor( ( h - 1.0 + ascent ) / 2.0 + 0.5 );
                break;
            case UI_LAYOUT_BELOW:
                x = ( int )floor( ( w - 1.0 - width ) / 2.0 - xoff + 0.5 );
                y = ( int )h - 1.0;
                h -= ascent;
                break;
        }

        x49gp_ui_draw_text( cr, &ui->colors[ UI_COLOR_YELLOW ], opt.font, key->letter_size, 0.0, x + xoffset, y + yoffset, 1,
                            CAIRO_FONT_SLANT_NORMAL, key->font_weight, key->letter );
    }

    x49gp_ui_text_size( cr, opt.font, key->font_size, &xoff, &yoff, &width, &height, &ascent, &descent, 1, CAIRO_FONT_SLANT_NORMAL,
                        key->font_weight, key->label );

    x = ( int )floor( ( w - 1.0 - width ) / 2.0 - xoff + 0.5 );
    y = ( int )floor( ( h - 1.0 + ascent ) / 2.0 + 0.5 );

    x49gp_ui_draw_text( cr, &gtk_widget_get_style( widget )->text[ 0 ], opt.font, key->font_size, 0.0, x + xoffset, y + yoffset, 1,
                        CAIRO_FONT_SLANT_NORMAL, key->font_weight, key->label );

    cairo_destroy( cr );
}

static int x49gp_lcd_expose_event( GtkWidget* widget, GdkEventExpose* event, gpointer user_data )
{
    x49gp_t* x49gp = user_data;
    x49gp_ui_t* ui = x49gp->ui;
    GdkRectangle* rects;
    int n;

    gdk_region_get_rectangles( event->region, &rects, &n );
    for ( int i = 0; i < n; i++ ) {
        gdk_draw_drawable( gtk_widget_get_window( widget ), gtk_widget_get_style( widget )->black_gc, ui->lcd_pixmap, rects[ i ].x,
                           rects[ i ].y, rects[ i ].x, rects[ i ].y, rects[ i ].width, rects[ i ].height );
    }

    g_free( rects );

    return false;
}

/* static void draw_xbm(cairo_t *cr, GdkRectangle *rect, int width, int height, unsigned char *bits) */
/* { */
/*         double			  sz	= rect->width < rect->height ? rect->width : rect->height; */
/*         cairo_surface_t	* icon	= cairo_image_surface_create_for_data( */
/*                                                                         bits, */
/*                                                                         CAIRO_FORMAT_A1, */
/*                                                                         width,height, */
/*                                                                         cairo_format_stride_for_width(CAIRO_FORMAT_A1,width)); */

/*         cairo_save(cr); */

/*         cairo_scale(cr,	sz / ((double) width), */
/*                                         sz / ((double) height)); */

/*         cairo_mask_surface(cr,icon,(rect->width-sz)/2,(rect->height-sz)/2); */

/*         cairo_surface_destroy(icon); */

/*         cairo_restore(cr); */
/* } */

static int x49gp_lcd_configure_event( GtkWidget* widget, GdkEventConfigure* event, gpointer user_data )
{
    x49gp_t* x49gp = user_data;
    x49gp_ui_t* ui = x49gp->ui;

    if ( NULL != ui->lcd_pixmap )
        return false;

    const cairo_format_t cairo_fmt = CAIRO_FORMAT_A1;
    int stride = cairo_format_stride_for_width( cairo_fmt, 12 );
    ui->ann_left_surface = cairo_image_surface_create_for_data( ann_left_bits, cairo_fmt, ann_left_width, ann_left_height,
                                                                stride );
    ui->ann_right_surface = cairo_image_surface_create_for_data( ann_right_bits, cairo_fmt, ann_right_width, ann_right_height,
                                                                 stride );
    ui->ann_alpha_surface = cairo_image_surface_create_for_data( ann_alpha_bits, cairo_fmt, ann_alpha_width, ann_alpha_height,
                                                                 stride );
    ui->ann_battery_surface = cairo_image_surface_create_for_data( ann_battery_bits, cairo_fmt, ann_battery_width, ann_battery_height,
                                                                   stride );
    ui->ann_busy_surface = cairo_image_surface_create_for_data( ann_busy_bits, cairo_fmt, ann_busy_width, ann_busy_height,
                                                                stride );
    ui->ann_io_surface = cairo_image_surface_create_for_data( ann_io_bits, cairo_fmt, ann_io_width, ann_io_height,
                                                              stride );

    ui->lcd_pixmap = gdk_pixmap_new( gtk_widget_get_window( ui->lcd_canvas ), ui->lcd_width, ui->lcd_height, -1 );

    cairo_t* cr = gdk_cairo_create( ui->lcd_pixmap );
    GdkColor color = ui->colors[ UI_COLOR_GRAYSCALE_0 ];
    cairo_set_source_rgb( cr, color.red / 65535.0, color.green / 65535.0, color.blue / 65535.0 );
    cairo_rectangle( cr, 0, 0, ui->lcd_width, ui->lcd_height );
    cairo_fill( cr );
    cairo_destroy( cr );

    return false;
}

static int x49gp_window_configure_event( GtkWidget* widget, GdkEventConfigure* event, gpointer user_data )
{
    x49gp_t* x49gp = user_data;
    x49gp_ui_t* ui = x49gp->ui;
    const x49gp_ui_key_t* key;
    cairo_t* cr;
    int left_color;
    int right_color;
    int below_color;
    int xl, xr, a;
    int wl = 0, wr = 0;
    int hl = 0, hr = 0;
    int dl = 0, dr = 0;
    int faceplate_color;

    if ( NULL != ui->bg_pixmap )
        return false;

    if ( ui->calculator == UI_CALCULATOR_HP49GP || ui->calculator == UI_CALCULATOR_HP49GP_NEWRPL )
        faceplate_color = UI_COLOR_FACEPLATE_49GP;
    else
        faceplate_color = UI_COLOR_FACEPLATE_50G;

    gdk_pixbuf_fill( ui->bg_pixbuf, color2rgb( ui, faceplate_color ) );

    /* cairo_surface_t* surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, ui->width, ui->height); */
    /* cr = cairo_create( surface ); */
    /* /\* Draw the pixbuf *\/ */
    /* gdk_cairo_set_source_pixbuf (cr, ui->bg_pixbuf, 0, 0); */
    /* cairo_paint (cr); */
    /* /\* Draw a red rectangle *\/ */

    /* GdkColor color = ui->colors[ UI_COLOR_GRAYSCALE_0 ]; */
    /* cairo_set_source_rgb( cr, color.red / 65535.0, color.green / 65535.0, color.blue / 65535.0 ); */
    /* cairo_rectangle (cr, ui->lcd_x_offset - 2, ui->lcd_y_offset - 2, ui->lcd_width + 4, ui->lcd_height + 4 ); */
    /* cairo_fill (cr); */

    /* cairo_surface_destroy( surface ); */
    /* cairo_destroy( cr ); */

    ui->bg_pixmap = gdk_pixmap_new( gtk_widget_get_window( widget ), ui->width, ui->height, -1 );

    cr = gdk_cairo_create( ui->bg_pixmap );
    gdk_cairo_set_source_pixbuf( cr, ui->bg_pixbuf, 0, 0 );
    cairo_paint( cr );

    switch ( ui->calculator ) {
        case UI_CALCULATOR_HP49GP:
        case UI_CALCULATOR_HP49GP_NEWRPL:
            /* x49gp_ui_draw_text( cr, &gtk_widget_get_style( widget )->black, opt.font, 15.0, 0.0, 14 /\* 38 *\/, 20 /\* 42 *\/, 2, */
            /*                     CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD, "hp", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL, */
            /*                     " 49g+" ); */

            /* x49gp_ui_draw_text( cr, &gtk_widget_get_style( widget )->black, opt.font, 13.0, 0.0, 14 /\* 38 *\/, 34 /\* 56 *\/, 1, */
            /*                     CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL, "graphing calculator" ); */

            /* x49gp_ui_draw_symbol( cr, &gtk_widget_get_style( widget )->black, 10.0, 0.0, true, 114 /\* 138 *\/, 8 /\* 25 *\/, */
            /*                       symbol_get_by_name( "triangleup" ) ); */

            left_color = UI_COLOR_GREEN;
            right_color = UI_COLOR_RED;
            below_color = UI_COLOR_BLACK;
            break;

        default:
            ui->calculator = UI_CALCULATOR_HP50G;
            /* fall through */

        case UI_CALCULATOR_HP50G:
        case UI_CALCULATOR_HP50G_NEWRPL:
            /* x49gp_ui_draw_text( cr, &gtk_widget_get_style( widget )->white, opt.font, 15.0, 0.0, 14 /\* 38 *\/, 20 /\* 42 *\/, 2, */
            /*                     CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL, "HP", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL, */
            /*                     " 50g" ); */

            /* x49gp_ui_draw_text( cr, &gtk_widget_get_style( widget )->white, opt.font, 13.0, 0.0, 14 /\* 38 *\/, 34 /\* 56 *\/, 1, */
            /*                     CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL, "Graphing Calculator" ); */

            /* x49gp_ui_draw_symbol( cr, &gtk_widget_get_style( widget )->white, 10.0, 0.0, true, 134 /\* 168 *\/, 8 /\* 25 *\/, */
            /*                       symbol_get_by_name( "triangleup" ) ); */

            left_color = UI_COLOR_WHITE;
            right_color = UI_COLOR_ORANGE;
            below_color = UI_COLOR_BLUE;
            break;
    }

    cairo_destroy( cr );

    for ( int i = 0; i < ui->nr_buttons; i++ ) {
        key = ui->buttons[ i ].key;

        if ( key->left ) {
            bitmap_font_text_size( &tiny_font, key->left, &wl, &hl, &a, &dl );
            if ( !key->right ) {
                xl = key->x + ( key->width - wl ) / 2;
                bitmap_font_draw_text( ui->bg_pixmap, &ui->colors[ left_color ], &tiny_font, ui->kb_x_offset + xl,
                                       ui->kb_y_offset + key->y - hl + dl + 1, key->left );
            }
        }

        if ( key->right ) {
            bitmap_font_text_size( &tiny_font, key->right, &wr, &hr, &a, &dr );
            if ( !key->left ) {
                xr = key->x + ( key->width - wr ) / 2;
                bitmap_font_draw_text( ui->bg_pixmap, &ui->colors[ right_color ], &tiny_font, ui->kb_x_offset + xr,
                                       ui->kb_y_offset + key->y - hr + dr + 1, key->right );
            }
        }

        if ( key->left && key->right ) {
            xl = key->x;
            xr = key->x + key->width - 1 - wr;

            if ( wl + wr > key->width - 4 ) {
                xl += ( key->width - 4 - ( wl + wr ) ) / 2;
                xr -= ( key->width - 4 - ( wl + wr ) ) / 2;
            }

            bitmap_font_draw_text( ui->bg_pixmap, &ui->colors[ left_color ], &tiny_font, ui->kb_x_offset + xl,
                                   ui->kb_y_offset + key->y - hl + dl + 1, key->left );

            bitmap_font_draw_text( ui->bg_pixmap, &ui->colors[ right_color ], &tiny_font, ui->kb_x_offset + xr,
                                   ui->kb_y_offset + key->y - hr + dr + 1, key->right );
        }

        if ( key->below ) {
            bitmap_font_text_size( &tiny_font, key->below, &wl, &hl, &a, &dl );
            xl = key->x + ( key->width - wl ) / 2;

            bitmap_font_draw_text( ui->bg_pixmap, &ui->colors[ below_color ], &tiny_font, ui->kb_x_offset + xl,
                                   ui->kb_y_offset + key->y + key->height + 2, key->below );
        }

#if DEBUG_LAYOUT /* Debug Button Layout */
        gdk_draw_rectangle( ui->bg_pixmap, gtk_widget_get_style( ui->window )->white_gc, false, ui->kb_x_offset + key->x,
                            ui->kb_y_offset + key->y, key->width, key->height );
#endif
    }

    gdk_window_set_back_pixmap( gtk_widget_get_window( widget ), ui->bg_pixmap, false );

    return false;
}

static gboolean x49gp_window_button_press( GtkWidget* widget, GdkEventButton* event, gpointer user_data )
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

static void x49gp_ui_quit( gpointer user_data, GtkWidget* widget, GdkEvent* event )
{
    x49gp_t* x49gp = user_data;

    x49gp->arm_exit++;
}

static void x49gp_ui_place_at( x49gp_t* x49gp, GtkFixed* fixed, GtkWidget* widget, gint x, gint y, gint width, gint height )
{
    gtk_widget_set_size_request( widget, width, height );
    gtk_fixed_put( fixed, widget, x, y );
}

static int gui_init( x49gp_module_t* module )
{
    x49gp_t* x49gp = module->x49gp;
    x49gp_ui_t* ui;

    ui = malloc( sizeof( x49gp_ui_t ) );
    if ( NULL == ui ) {
        fprintf( stderr, "%s: %s:%u: Out of memory\n", x49gp->progname, __FUNCTION__, __LINE__ );
        return -ENOMEM;
    }
    memset( ui, 0, sizeof( x49gp_ui_t ) );

    ui->nr_buttons = NB_KEYS;
    ui->buttons = malloc( ui->nr_buttons * sizeof( x49gp_ui_button_t ) );
    if ( NULL == ui->buttons ) {
        fprintf( stderr, "%s: %s:%u: Out of memory\n", x49gp->progname, __FUNCTION__, __LINE__ );
        free( ui );
        return -ENOMEM;
    }
    memset( ui->buttons, 0, ui->nr_buttons * sizeof( x49gp_ui_button_t ) );

    module->user_data = ui;
    x49gp->ui = ui;

    return 0;
}

static int gui_exit( x49gp_module_t* module ) { return 0; }

static int gui_reset( x49gp_module_t* module, x49gp_reset_t reset ) { return 0; }

static void init_colors( x49gp_ui_t* ui )
{
    x49gp_ui_color_init( &ui->colors[ UI_COLOR_BLACK ], 0x00, 0x00, 0x00 );        /* #000000 */
    x49gp_ui_color_init( &ui->colors[ UI_COLOR_WHITE ], 0xff, 0xff, 0xff );        /* #ffffff */
    x49gp_ui_color_init( &ui->colors[ UI_COLOR_YELLOW ], 0xfa, 0xe8, 0x2c );       /* #fae82c */
    x49gp_ui_color_init( &ui->colors[ UI_COLOR_RED ], 0x8e, 0x25, 0x18 );          /* #8e2518 */
    x49gp_ui_color_init( &ui->colors[ UI_COLOR_GREEN ], 0x14, 0x4d, 0x49 );        /* #144d49 */
    x49gp_ui_color_init( &ui->colors[ UI_COLOR_SILVER ], 0xe0, 0xe0, 0xe0 );       /* #e0e0e0 */
    x49gp_ui_color_init( &ui->colors[ UI_COLOR_ORANGE ], 0xc0, 0x6e, 0x60 );       /* #c06e60 */
    x49gp_ui_color_init( &ui->colors[ UI_COLOR_BLUE ], 0x40, 0x60, 0xa4 );         /* #4060a4 */
    x49gp_ui_color_init( &ui->colors[ UI_COLOR_GRAYSCALE_0 ], 0xab, 0xd2, 0xb4 );  /* #abd2b4 */
    x49gp_ui_color_init( &ui->colors[ UI_COLOR_GRAYSCALE_1 ], 0xa0, 0xc4, 0xa8 );  /* #a0c4a8 */
    x49gp_ui_color_init( &ui->colors[ UI_COLOR_GRAYSCALE_2 ], 0x94, 0xb6, 0x9c );  /* #94b69c */
    x49gp_ui_color_init( &ui->colors[ UI_COLOR_GRAYSCALE_3 ], 0x89, 0xa8, 0x90 );  /* #89a890 */
    x49gp_ui_color_init( &ui->colors[ UI_COLOR_GRAYSCALE_4 ], 0x7d, 0x9a, 0x84 );  /* #7d9a84 */
    x49gp_ui_color_init( &ui->colors[ UI_COLOR_GRAYSCALE_5 ], 0x72, 0x8c, 0x78 );  /* #728c78 */
    x49gp_ui_color_init( &ui->colors[ UI_COLOR_GRAYSCALE_6 ], 0x67, 0x7e, 0x6c );  /* #677e6c */
    x49gp_ui_color_init( &ui->colors[ UI_COLOR_GRAYSCALE_7 ], 0x5b, 0x70, 0x60 );  /* #5b7060 */
    x49gp_ui_color_init( &ui->colors[ UI_COLOR_GRAYSCALE_8 ], 0x50, 0x62, 0x54 );  /* #506254 */
    x49gp_ui_color_init( &ui->colors[ UI_COLOR_GRAYSCALE_9 ], 0x44, 0x54, 0x48 );  /* #445448 */
    x49gp_ui_color_init( &ui->colors[ UI_COLOR_GRAYSCALE_10 ], 0x39, 0x46, 0x3c ); /* #39463c */
    x49gp_ui_color_init( &ui->colors[ UI_COLOR_GRAYSCALE_11 ], 0x2e, 0x38, 0x30 ); /* #2e3830 */
    x49gp_ui_color_init( &ui->colors[ UI_COLOR_GRAYSCALE_12 ], 0x22, 0x2a, 0x24 ); /* #222a24 */
    x49gp_ui_color_init( &ui->colors[ UI_COLOR_GRAYSCALE_13 ], 0x17, 0x1c, 0x18 ); /* #171c18 */
    x49gp_ui_color_init( &ui->colors[ UI_COLOR_GRAYSCALE_14 ], 0x0b, 0x03, 0x0c ); /* #0b030c */
    x49gp_ui_color_init( &ui->colors[ UI_COLOR_GRAYSCALE_15 ], 0x00, 0x00, 0x00 ); /* #000000 */

    x49gp_ui_color_init( &ui->colors[ UI_COLOR_FACEPLATE_49GP ], 0xf5, 0xde, 0xb3 ); /* #f5deb3 */
    x49gp_ui_color_init( &ui->colors[ UI_COLOR_FACEPLATE_50G ], 0x27, 0x27, 0x27 ); /* #272727 */
}

static int gui_load( x49gp_module_t* module, GKeyFile* keyfile )
{
    x49gp_t* x49gp = module->x49gp;
    x49gp_ui_t* ui = module->user_data;

    /* create all colors */
    init_colors( ui );

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

    /* set coordinates of LCD and keyboard */
    ui->lcd_annunciators_height = 16;
    ui->lcd_width = 131 * LCD_PIXEL_SCALE;
    ui->lcd_height = ( 80 * LCD_PIXEL_SCALE ) + ui->lcd_annunciators_height;
    ui->lcd_x_offset = 20;
    ui->lcd_y_offset = ui->lcd_x_offset;

    ui->kb_x_offset = 10;
    ui->kb_y_offset = ui->lcd_height + ( 2 * ui->lcd_y_offset);

    ui->width = ui->lcd_width + ( 2 * ui->lcd_x_offset );
    ui->height = ui->kb_y_offset + ui_keys[ NB_KEYS - 1 ].y + KB_LINE_HEIGHT;
    int kb_width = ( ui->kb_x_offset ) + ( 5 * KB_COLUMN_WIDTH_5_KEYS );
    if ( ui->width < kb_width ) {
        ui->width = kb_width;
        ui->lcd_x_offset = ( ui->width - ui->lcd_width ) / 2;
    }

    ui->bg_pixbuf = gdk_pixbuf_new( GDK_COLORSPACE_RGB, FALSE, 8, ui->width, ui->height );

    ui->window = gtk_window_new( GTK_WINDOW_TOPLEVEL );
    gtk_widget_set( ui->window, "can-focus", true, NULL );
    gtk_widget_set( ui->window, "accept-focus", true, NULL );
    gtk_widget_set( ui->window, "focus-on-map", true, NULL );
    gtk_widget_set( ui->window, "resizable", false, NULL );
    gtk_window_set_decorated( GTK_WINDOW( ui->window ), true );

    gtk_widget_set_name( ui->window, ui->name );
    gtk_window_set_title( GTK_WINDOW( ui->window ), ui->name );

    //	gtk_window_set_icon(GTK_WINDOW(ui->window), ui->bg_pixbuf);

    gtk_widget_realize( ui->window );

    ui->fixed = gtk_fixed_new();
    gtk_container_add( GTK_CONTAINER( ui->window ), ui->fixed );

    ui->background = gtk_drawing_area_new();
    gtk_drawing_area_size( GTK_DRAWING_AREA( ui->background ), ui->width, ui->height );
    x49gp_ui_place_at( x49gp, GTK_FIXED( ui->fixed ), ui->background, 0, 0, ui->width, ui->height );

    ui->lcd_canvas = gtk_drawing_area_new();
    gtk_drawing_area_size( GTK_DRAWING_AREA( ui->lcd_canvas ), ui->lcd_width, ui->lcd_height );

    GtkWidget* screen_box = gtk_event_box_new();
    gtk_event_box_set_visible_window( GTK_EVENT_BOX( screen_box ), true );
    gtk_event_box_set_above_child( GTK_EVENT_BOX( screen_box ), false );
    gtk_container_add( GTK_CONTAINER( screen_box ), ui->lcd_canvas );
    x49gp_ui_place_at( x49gp, GTK_FIXED( ui->fixed ), screen_box, ui->lcd_x_offset, ui->lcd_y_offset, ui->lcd_width, ui->lcd_height );

    {
        x49gp_ui_button_t* button;

        if ( ui->calculator == UI_CALCULATOR_HP49GP_NEWRPL || ui->calculator == UI_CALCULATOR_HP50G_NEWRPL ) {
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

            ui_keys[ 16 ].label = "S\\kern-1 T\\kern-1 O\\kern-1\\triangleright";
            ui_keys[ 16 ].left = "RCL";
            ui_keys[ 16 ].right = "PREV.M";

            for ( int i = 17; i < 20; i++ ) {
                ui_keys[ i ].left = NULL;
                ui_keys[ i ].right = NULL;
            }

            ui_keys[ 19 ].label = "M\\kern-1 E\\kern-1 N\\kern-1 U";

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

        for ( int i = 0; i < ui->nr_buttons; i++ ) {
            button = &ui->buttons[ i ];

            button->x49gp = x49gp;
            button->key = &ui_keys[ i ];

            button->button = gtk_button_new();
            gtk_widget_set_size_request( button->button, ui_keys[ i ].width, ui_keys[ i ].height );
            gtk_widget_set( button->button, "can-focus", false, NULL );

            x49gp_ui_button_pixmaps_init( x49gp, button, ui_keys[ i ].color );

            if ( ui_keys[ i ].label ) {
                button->label = gtk_label_new( "" );
                gtk_widget_set_style( button->label, gtk_widget_get_style( button->button ) );
                gtk_container_add( GTK_CONTAINER( button->button ), button->label );

                g_signal_connect( G_OBJECT( button->label ), "expose-event", G_CALLBACK( x49gp_button_expose_event ), button );

                g_signal_connect_after( G_OBJECT( button->label ), "realize", G_CALLBACK( x49gp_button_realize ), button );
            }

            button->box = gtk_event_box_new();
            gtk_event_box_set_visible_window( GTK_EVENT_BOX( button->box ), true );
            gtk_event_box_set_above_child( GTK_EVENT_BOX( button->box ), false );
            gtk_container_add( GTK_CONTAINER( button->box ), button->button );

            x49gp_ui_place_at( x49gp, GTK_FIXED( ui->fixed ), button->box, ui->kb_x_offset + ui_keys[ i ].x, ui->kb_y_offset + ui_keys[ i ].y,
                               ui_keys[ i ].width, ui_keys[ i ].height );

            g_signal_connect( G_OBJECT( button->button ), "button-press-event", G_CALLBACK( x49gp_ui_button_press ), button );
            g_signal_connect( G_OBJECT( button->button ), "button-release-event", G_CALLBACK( x49gp_ui_button_release ), button );
            g_signal_connect( G_OBJECT( button->button ), "leave-notify-event", G_CALLBACK( x49gp_ui_button_leave ), button );

            gtk_widget_add_events( button->button, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_LEAVE_NOTIFY_MASK );
        }
    }
    // right-click menu
    // {
    ui->menu = gtk_menu_new();

    GtkWidget* menu_mount_folder = gtk_menu_item_new_with_label( "Mount SD folder ..." );
    gtk_menu_shell_append( GTK_MENU_SHELL( ui->menu ), menu_mount_folder );
    g_signal_connect( G_OBJECT( menu_mount_folder ), "activate", G_CALLBACK( x49gp_ui_mount_sd_folder ), x49gp );

    GtkWidget* menu_mount_image = gtk_menu_item_new_with_label( "Mount SD image ..." );
    gtk_menu_shell_append( GTK_MENU_SHELL( ui->menu ), menu_mount_image );
    g_signal_connect( G_OBJECT( menu_mount_image ), "activate", G_CALLBACK( x49gp_ui_mount_sd_image ), x49gp );

    GtkWidget* menu_unmount = gtk_menu_item_new_with_label( "Unmount SD" );
    gtk_menu_shell_append( GTK_MENU_SHELL( ui->menu ), menu_unmount );
    g_signal_connect_swapped( G_OBJECT( menu_unmount ), "activate", G_CALLBACK( s3c2410_sdi_unmount ), x49gp );
    ui->menu_unmount = menu_unmount;

    if ( x49gp->debug_port != 0 ) {
        gtk_menu_shell_append( GTK_MENU_SHELL( ui->menu ), gtk_separator_menu_item_new() );

        GtkWidget* menu_debug = gtk_menu_item_new_with_label( "Start debugger" );
        gtk_menu_shell_append( GTK_MENU_SHELL( ui->menu ), menu_debug );
        g_signal_connect( G_OBJECT( menu_debug ), "activate", G_CALLBACK( x49gp_ui_debug ), x49gp );
        ui->menu_debug = menu_debug;
    } else
        ui->menu_debug = NULL;

    gtk_menu_shell_append( GTK_MENU_SHELL( ui->menu ), gtk_separator_menu_item_new() );

    GtkWidget* menu_reset = gtk_menu_item_new_with_label( "Reset" );
    gtk_menu_shell_append( GTK_MENU_SHELL( ui->menu ), menu_reset );
    g_signal_connect( G_OBJECT( menu_reset ), "activate", G_CALLBACK( x49gp_ui_calculator_reset ), x49gp );

    GtkWidget* menu_quit = gtk_menu_item_new_with_label( "Quit" );
    gtk_menu_shell_append( GTK_MENU_SHELL( ui->menu ), menu_quit );
    g_signal_connect_swapped( G_OBJECT( menu_quit ), "activate", G_CALLBACK( x49gp_ui_quit ), x49gp );

    gtk_widget_show_all( ui->menu );
    // }

    g_signal_connect( G_OBJECT( screen_box ), "button-press-event", G_CALLBACK( x49gp_ui_show_menu ), x49gp );
    g_signal_connect( G_OBJECT( ui->background ), "configure-event", G_CALLBACK( x49gp_window_configure_event ), x49gp );

    g_signal_connect( G_OBJECT( ui->lcd_canvas ), "expose-event", G_CALLBACK( x49gp_lcd_expose_event ), x49gp );
    g_signal_connect( G_OBJECT( ui->lcd_canvas ), "configure-event", G_CALLBACK( x49gp_lcd_configure_event ), x49gp );

    g_signal_connect( G_OBJECT( ui->window ), "focus-out-event", G_CALLBACK( x49gp_ui_focus_lost ), x49gp );
    g_signal_connect( G_OBJECT( ui->window ), "key-press-event", G_CALLBACK( x49gp_ui_key_event ), x49gp );
    g_signal_connect( G_OBJECT( ui->window ), "key-release-event", G_CALLBACK( x49gp_ui_key_event ), x49gp );
    g_signal_connect( G_OBJECT( ui->window ), "button-press-event", G_CALLBACK( x49gp_window_button_press ), x49gp );
    g_signal_connect_swapped( G_OBJECT( ui->window ), "delete-event", G_CALLBACK( x49gp_ui_quit ), x49gp );
    g_signal_connect_swapped( G_OBJECT( ui->window ), "destroy", G_CALLBACK( x49gp_ui_quit ), x49gp );

    gtk_widget_add_events( ui->window, GDK_FOCUS_CHANGE_MASK | GDK_BUTTON_PRESS_MASK | GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK );

    gtk_widget_show_all( ui->window );

    return 0;
}

static int gui_save( x49gp_module_t* module, GKeyFile* keyfile ) { return 0; }

int x49gp_ui_init( x49gp_t* x49gp )
{
    x49gp_module_t* module;

    if ( x49gp_module_init( x49gp, "gui", gui_init, gui_exit, gui_reset, gui_load, gui_save, NULL, &module ) )
        return -1;

    return x49gp_module_register( module );
}
