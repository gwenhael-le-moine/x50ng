#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <sys/times.h>

#include <gtk/gtk.h>
#include <glib.h>
#include <cairo.h>
#include <gdk/gdkkeysyms.h>

#include "options.h"
#include "x50ng.h"
#include "ui.h"
#include "s3c2410.h"

#include "gdbstub.h"
#include "types.h"

// #define TEST_PASTE true

#define KB_NB_ROWS ( 10 )

#define LCD_WIDTH ( 131 )
#define LCD_HEIGHT ( 80 )

typedef enum {
    HPKEY_A = 0,
    HPKEY_B,
    HPKEY_C,
    HPKEY_D,
    HPKEY_E,
    HPKEY_F,

    HPKEY_G,
    HPKEY_H,
    HPKEY_I,
    HPKEY_UP,
    HPKEY_J, /* 10 */

    HPKEY_K,
    HPKEY_L,
    HPKEY_LEFT,
    HPKEY_DOWN,
    HPKEY_RIGHT,

    HPKEY_M,
    HPKEY_N,
    HPKEY_O,
    HPKEY_P,
    HPKEY_BACKSPACE, /* 20 */

    HPKEY_Q,
    HPKEY_R,
    HPKEY_S,
    HPKEY_T,
    HPKEY_U,

    HPKEY_ENTER,
    HPKEY_V,
    HPKEY_W,
    HPKEY_X,
    HPKEY_Y, /* 30 */

    HPKEY_ALPHA,
    HPKEY_7,
    HPKEY_8,
    HPKEY_9,
    HPKEY_Z,

    HPKEY_SHIFT_LEFT,
    HPKEY_4,
    HPKEY_5,
    HPKEY_6,
    HPKEY_MULTIPLY, /* 40 */

    HPKEY_SHIFT_RIGHT,
    HPKEY_1,
    HPKEY_2,
    HPKEY_3,
    HPKEY_MINUS,

    HPKEY_ON,
    HPKEY_0,
    HPKEY_PERIOD,
    HPKEY_SPACE,
    HPKEY_PLUS, /* 50 */

    NB_KEYS
} x50ng_ui_hpkey_t;

typedef struct {
    const char* css_class;
    const char* css_id;
    const char* label;
    const char* letter;
    const char* left;
    const char* right;
    const char* below;

    int column;
    int row;
    unsigned char columnbit;
    unsigned char rowbit;
    int eint;
} x50ng_ui_key_t;

typedef struct {
    x50ng_t* x50ng;
    const x50ng_ui_key_t* key;
    GtkWidget* button;
    bool down;
    bool hold;
} x50ng_ui_button_t;

static x50ng_ui_key_t ui_keys[ NB_KEYS ] = {
    {.css_class = "menu",
     .css_id = "F1",
     .label = "F1",
     .letter = "A",
     .left = "Y=",
     .right = NULL,
     .below = NULL,
     .column = 5,
     .row = 1,
     .columnbit = ( 1 << 5 ),
     .rowbit = ( 1 << 1 ),
     .eint = 1},
    {.css_class = "menu",
     .css_id = "F2",
     .label = "F2",
     .letter = "B",
     .left = "WIN",
     .right = NULL,
     .below = NULL,
     .column = 5,
     .row = 2,
     .columnbit = ( 1 << 5 ),
     .rowbit = ( 1 << 2 ),
     .eint = 2},
    {.css_class = "menu",
     .css_id = "F3",
     .label = "F3",
     .letter = "C",
     .left = "GRAPH",
     .right = NULL,
     .below = NULL,
     .column = 5,
     .row = 3,
     .columnbit = ( 1 << 5 ),
     .rowbit = ( 1 << 3 ),
     .eint = 3},
    {.css_class = "menu",
     .css_id = "F4",
     .label = "F4",
     .letter = "D",
     .left = "2D/3D",
     .right = NULL,
     .below = NULL,
     .column = 5,
     .row = 4,
     .columnbit = ( 1 << 5 ),
     .rowbit = ( 1 << 4 ),
     .eint = 4},
    {.css_class = "menu",
     .css_id = "F5",
     .label = "F5",
     .letter = "E",
     .left = "TBLSET",
     .right = NULL,
     .below = NULL,
     .column = 5,
     .row = 5,
     .columnbit = ( 1 << 5 ),
     .rowbit = ( 1 << 5 ),
     .eint = 5},
    {.css_class = "menu",
     .css_id = "F6",
     .label = "F6",
     .letter = "F",
     .left = "TABLE",
     .right = NULL,
     .below = NULL,
     .column = 5,
     .row = 6,
     .columnbit = ( 1 << 5 ),
     .rowbit = ( 1 << 6 ),
     .eint = 6},

    {.css_class = "function",
     .css_id = "APPS",
     .label = "APPS",
     .letter = "G",
     .left = "FILES",
     .right = "BEGIN",
     .below = NULL,
     .column = 5,
     .row = 7,
     .columnbit = ( 1 << 5 ),
     .rowbit = ( 1 << 7 ),
     .eint = 7},
    {.css_class = "function",
     .css_id = "MODE",
     .label = "MODE",
     .letter = "H",
     .left = "CUSTOM",
     .right = "END",
     .below = NULL,
     .column = 6,
     .row = 5,
     .columnbit = ( 1 << 6 ),
     .rowbit = ( 1 << 5 ),
     .eint = 5},
    {.css_class = "function",
     .css_id = "TOOL",
     .label = "TOOL",
     .letter = "I",
     .left = "i",
     .right = "I",
     .below = NULL,
     .column = 6,
     .row = 6,
     .columnbit = ( 1 << 6 ),
     .rowbit = ( 1 << 6 ),
     .eint = 6},
    {.css_class = "arrow",
     .css_id = "UP",
     .label = "⬆",
     .letter = "",
     .left = "",
     .right = NULL,
     .below = NULL,
     .column = 6,
     .row = 1,
     .columnbit = ( 1 << 6 ),
     .rowbit = ( 1 << 1 ),
     .eint = 1},
    {.css_class = "function",
     .css_id = "VAR",
     .label = "VAR",
     .letter = "J",
     .left = "UPDIR",
     .right = "COPY",
     .below = NULL,
     .column = 6,
     .row = 7,
     .columnbit = ( 1 << 6 ),
     .rowbit = ( 1 << 7 ),
     .eint = 7},
    {.css_class = "function",
     .css_id = "STO",
     .label = "STO⏵",
     .letter = "K",
     .left = "RCL",
     .right = "CUT",
     .below = NULL,
     .column = 7,
     .row = 1,
     .columnbit = ( 1 << 7 ),
     .rowbit = ( 1 << 1 ),
     .eint = 1},
    {.css_class = "function",
     .css_id = "NXT",
     .label = "NXT",
     .letter = "L",
     .left = "PREV",
     .right = "PASTE",
     .below = NULL,
     .column = 7,
     .row = 2,
     .columnbit = ( 1 << 7 ),
     .rowbit = ( 1 << 2 ),
     .eint = 2},

    {.css_class = "arrow",
     .css_id = "LEFT",
     .label = "⬅",
     .letter = "",
     .left = "",
     .right = NULL,
     .below = NULL,
     .column = 6,
     .row = 2,
     .columnbit = ( 1 << 6 ),
     .rowbit = ( 1 << 2 ),
     .eint = 2},
    {.css_class = "arrow",
     .css_id = "DOWN",
     .label = "⬇",
     .letter = "",
     .left = "",
     .right = NULL,
     .below = NULL,
     .column = 6,
     .row = 3,
     .columnbit = ( 1 << 6 ),
     .rowbit = ( 1 << 3 ),
     .eint = 3},
    {.css_class = "arrow",
     .css_id = "RIGHT",
     .label = "➡",
     .letter = "",
     .left = "",
     .right = NULL,
     .below = NULL,
     .column = 6,
     .row = 4,
     .columnbit = ( 1 << 6 ),
     .rowbit = ( 1 << 4 ),
     .eint = 4},

    {.css_class = "function",
     .css_id = "HIST",
     .label = "HIST",
     .letter = "M",
     .left = "CMD",
     .right = "UNDO",
     .below = NULL,
     .column = 4,
     .row = 1,
     .columnbit = ( 1 << 4 ),
     .rowbit = ( 1 << 1 ),
     .eint = 1},
    {.css_class = "function",
     .css_id = "EVAL",
     .label = "EVAL",
     .letter = "N",
     .left = "PRG",
     .right = "CHARS",
     .below = NULL,
     .column = 3,
     .row = 1,
     .columnbit = ( 1 << 3 ),
     .rowbit = ( 1 << 1 ),
     .eint = 1},
    {.css_class = "function",
     .css_id = "QUOTE",
     .label = "'",
     .letter = "O",
     .left = "MTRW",
     .right = "EQW",
     .below = NULL,
     .column = 2,
     .row = 1,
     .columnbit = ( 1 << 2 ),
     .rowbit = ( 1 << 1 ),
     .eint = 1},
    {.css_class = "function",
     .css_id = "SYMB",
     .label = "SYMB",
     .letter = "P",
     .left = "MTH",
     .right = "CAT",
     .below = NULL,
     .column = 1,
     .row = 1,
     .columnbit = ( 1 << 1 ),
     .rowbit = ( 1 << 1 ),
     .eint = 1},
    {.css_class = "function",
     .css_id = "BACKSPACE",
     .label = "⬅",
     .letter = "",
     .left = "DEL",
     .right = "CLEAR",
     .below = NULL,
     .column = 0,
     .row = 1,
     .columnbit = ( 1 << 0 ),
     .rowbit = ( 1 << 1 ),
     .eint = 1},

    {.css_class = "function",
     .css_id = "POW",
     .label = "Y<sup>x</sup>",
     .letter = "Q",
     .left = "𝑒<sup>x</sup>",
     .right = "LN",
     .below = NULL,
     .column = 4,
     .row = 2,
     .columnbit = ( 1 << 4 ),
     .rowbit = ( 1 << 2 ),
     .eint = 2},
    {.css_class = "function",
     .css_id = "SQRT",
     .label = "√𝓍",
     .letter = "R",
     .left = "𝓍<sup>2</sup>",
     .right = "<sup>x</sup>√𝓎",
     .below = NULL,
     .column = 3,
     .row = 2,
     .columnbit = ( 1 << 3 ),
     .rowbit = ( 1 << 2 ),
     .eint = 2},
    {.css_class = "function",
     .css_id = "SIN",
     .label = "SIN",
     .letter = "S",
     .left = "ASIN",
     .right = "∑",
     .below = NULL,
     .column = 2,
     .row = 2,
     .columnbit = ( 1 << 2 ),
     .rowbit = ( 1 << 2 ),
     .eint = 2},
    {.css_class = "function",
     .css_id = "COS",
     .label = "COS",
     .letter = "T",
     .left = "ACOS",
     .right = "∂",
     .below = NULL,
     .column = 1,
     .row = 2,
     .columnbit = ( 1 << 1 ),
     .rowbit = ( 1 << 2 ),
     .eint = 2},
    {.css_class = "function",
     .css_id = "TAN",
     .label = "TAN",
     .letter = "U",
     .left = "ATAN",
     .right = "∫",
     .below = NULL,
     .column = 0,
     .row = 2,
     .columnbit = ( 1 << 0 ),
     .rowbit = ( 1 << 2 ),
     .eint = 2},

    {.css_class = "enter",
     .css_id = "ENTER",
     .label = "ENTER",
     .letter = NULL,
     .left = "ANS",
     .right = "→NUM",
     .below = NULL,
     .column = 0,
     .row = 7,
     .columnbit = ( 1 << 0 ),
     .rowbit = ( 1 << 7 ),
     .eint = 7},
    {.css_class = "function",
     .css_id = "EEX",
     .label = "EEX",
     .letter = "V",
     .left = "10<sup>𝓍</sup>",
     .right = "LOG",
     .below = NULL,
     .column = 4,
     .row = 3,
     .columnbit = ( 1 << 4 ),
     .rowbit = ( 1 << 3 ),
     .eint = 3},
    {.css_class = "function",
     .css_id = "NEG",
     .label = "+/-",
     .letter = "W",
     .left = "≠",
     .right = "=",
     .below = NULL,
     .column = 3,
     .row = 3,
     .columnbit = ( 1 << 3 ),
     .rowbit = ( 1 << 3 ),
     .eint = 3},
    {.css_class = "function",
     .css_id = "X",
     .label = "𝓍",
     .letter = "X",
     .left = "≤",
     .right = "&gt;",
     .below = NULL,
     .column = 2,
     .row = 3,
     .columnbit = ( 1 << 2 ),
     .rowbit = ( 1 << 3 ),
     .eint = 3},
    {.css_class = "function",
     .css_id = "INV",
     .label = "1/𝓍",
     .letter = "Y",
     .left = "≥",
     .right = ">",
     .below = NULL,
     .column = 1,
     .row = 3,
     .columnbit = ( 1 << 1 ),
     .rowbit = ( 1 << 3 ),
     .eint = 3},

    {.css_class = "alpha",
     .css_id = "ALPHA",
     .label = "ALPHA",
     .letter = NULL,
     .left = "USER",
     .right = "ENTRY",
     .below = NULL,
     .column = 0,
     .row = 0,
     .columnbit = 0,
     .rowbit = 0,
     .eint = 4},
    {.css_class = "core-number",
     .css_id = "SEVEN",
     .label = "7",
     .letter = NULL,
     .left = "S.SLV",
     .right = "NUM.SLV",
     .below = NULL,
     .column = 3,
     .row = 4,
     .columnbit = ( 1 << 3 ),
     .rowbit = ( 1 << 4 ),
     .eint = 4},
    {.css_class = "core-number",
     .css_id = "EIGHT",
     .label = "8",
     .letter = NULL,
     .left = "EXP&amp;LN",
     .right = "TRIG",
     .below = NULL,
     .column = 2,
     .row = 4,
     .columnbit = ( 1 << 2 ),
     .rowbit = ( 1 << 4 ),
     .eint = 4},
    {.css_class = "core-number",
     .css_id = "NINE",
     .label = "9",
     .letter = NULL,
     .left = "FINANCE",
     .right = "TIME",
     .below = NULL,
     .column = 1,
     .row = 4,
     .columnbit = ( 1 << 1 ),
     .rowbit = ( 1 << 4 ),
     .eint = 4},
    {.css_class = "core-number",
     .css_id = "DIVIDE",
     .label = "÷",
     .letter = "Z",
     .left = "ABS",
     .right = "ARG",
     .below = NULL,
     .column = 0,
     .row = 3,
     .columnbit = ( 1 << 0 ),
     .rowbit = ( 1 << 3 ),
     .eint = 3},

    {.css_class = "shift-left",
     .css_id = "SHIFT-LEFT",
     .label = "⮢",
     .letter = NULL,
     .left = "",
     .right = NULL,
     .below = NULL,
     .column = 0,
     .row = 0,
     .columnbit = 0,
     .rowbit = 0,
     .eint = 5},
    {.css_class = "core-number",
     .css_id = "FOUR",
     .label = "4",
     .letter = NULL,
     .left = "CALC",
     .right = "ALG",
     .below = NULL,
     .column = 3,
     .row = 5,
     .columnbit = ( 1 << 3 ),
     .rowbit = ( 1 << 5 ),
     .eint = 5},
    {.css_class = "core-number",
     .css_id = "FIVE",
     .label = "5",
     .letter = NULL,
     .left = "MATRICES",
     .right = "STAT",
     .below = NULL,
     .column = 2,
     .row = 5,
     .columnbit = ( 1 << 2 ),
     .rowbit = ( 1 << 5 ),
     .eint = 5},
    {.css_class = "core-number",
     .css_id = "SIX",
     .label = "6",
     .letter = NULL,
     .left = "CONVERT",
     .right = "UNITS",
     .below = NULL,
     .column = 1,
     .row = 5,
     .columnbit = ( 1 << 1 ),
     .rowbit = ( 1 << 5 ),
     .eint = 5},
    {.css_class = "core-number",
     .css_id = "MULTIPLY",
     .label = "×",
     .letter = NULL,
     .left = "[ ]",
     .right = "\" \"",
     .below = NULL,
     .column = 0,
     .row = 4,
     .columnbit = ( 1 << 0 ),
     .rowbit = ( 1 << 4 ),
     .eint = 4},

    {.css_class = "shift-right",
     .css_id = "SHIFT-RIGHT",
     .label = "⮣",
     .letter = NULL,
     .left = "",
     .right = NULL,
     .below = NULL,
     .column = 0,
     .row = 0,
     .columnbit = 0,
     .rowbit = 0,
     .eint = 6},
    {.css_class = "core-number",
     .css_id = "ONE",
     .label = "1",
     .letter = NULL,
     .left = "ARITH",
     .right = "CMPLX",
     .below = NULL,
     .column = 3,
     .row = 6,
     .columnbit = ( 1 << 3 ),
     .rowbit = ( 1 << 6 ),
     .eint = 6},
    {.css_class = "core-number",
     .css_id = "TWO",
     .label = "2",
     .letter = NULL,
     .left = "DEF",
     .right = "LIB",
     .below = NULL,
     .column = 2,
     .row = 6,
     .columnbit = ( 1 << 2 ),
     .rowbit = ( 1 << 6 ),
     .eint = 6},
    {.css_class = "core-number",
     .css_id = "THREE",
     .label = "3",
     .letter = NULL,
     .left = "#",
     .right = "BASE",
     .below = NULL,
     .column = 1,
     .row = 6,
     .columnbit = ( 1 << 1 ),
     .rowbit = ( 1 << 6 ),
     .eint = 6},
    {.css_class = "core-number",
     .css_id = "MINUS",
     .label = "-",
     .letter = NULL,
     .left = "( )",
     .right = "_",
     .below = NULL,
     .column = 0,
     .row = 5,
     .columnbit = ( 1 << 0 ),
     .rowbit = ( 1 << 5 ),
     .eint = 5},

    {.css_class = "core",
     .css_id = "ON",
     .label = "ON",
     .letter = NULL,
     .left = "CONT",
     .right = "OFF",
     .below = "CANCEL",
     .column = 0,
     .row = 0,
     .columnbit = 0,
     .rowbit = 0,
     .eint = 0},
    {.css_class = "core-number",
     .css_id = "ZERO",
     .label = "0",
     .letter = NULL,
     .left = "∞",
     .right = "→",
     .below = NULL,
     .column = 3,
     .row = 7,
     .columnbit = ( 1 << 3 ),
     .rowbit = ( 1 << 7 ),
     .eint = 7},
    {.css_class = "core",
     .css_id = "PERIOD",
     .label = ".",
     .letter = NULL,
     .left = ": :",
     .right = "↲",
     .below = NULL,
     .column = 2,
     .row = 7,
     .columnbit = ( 1 << 2 ),
     .rowbit = ( 1 << 7 ),
     .eint = 7},
    {.css_class = "core",
     .css_id = "SPC",
     .label = "SPC",
     .letter = NULL,
     .left = "𝚷",
     .right = ",",
     .below = NULL,
     .column = 1,
     .row = 7,
     .columnbit = ( 1 << 1 ),
     .rowbit = ( 1 << 7 ),
     .eint = 7},
    {.css_class = "core-number",
     .css_id = "PLUS",
     .label = "+",
     .letter = NULL,
     .left = "{ }",
     .right = "« »",
     .below = NULL,
     .column = 0,
     .row = 6,
     .columnbit = ( 1 << 0 ),
     .rowbit = ( 1 << 6 ),
     .eint = 6},
};

static int keys_order_normal[ NB_KEYS ] = {
    HPKEY_A,          HPKEY_B,  HPKEY_C,         HPKEY_D,      HPKEY_E,     HPKEY_F,           HPKEY_G,     HPKEY_H, HPKEY_I,
    HPKEY_UP,         HPKEY_J,  HPKEY_K,         HPKEY_L,      HPKEY_LEFT,  HPKEY_DOWN,        HPKEY_RIGHT, HPKEY_M, HPKEY_N,
    HPKEY_O,          HPKEY_P,  HPKEY_BACKSPACE, HPKEY_Q,      HPKEY_R,     HPKEY_S,           HPKEY_T,     HPKEY_U, HPKEY_V,
    HPKEY_W,          HPKEY_X,  HPKEY_Y,         HPKEY_Z,      HPKEY_ALPHA, HPKEY_7,           HPKEY_8,     HPKEY_9, HPKEY_MULTIPLY,
    HPKEY_SHIFT_LEFT, HPKEY_4,  HPKEY_5,         HPKEY_6,      HPKEY_MINUS, HPKEY_SHIFT_RIGHT, HPKEY_1,     HPKEY_2, HPKEY_3,
    HPKEY_PLUS,       HPKEY_ON, HPKEY_0,         HPKEY_PERIOD, HPKEY_SPACE, HPKEY_ENTER,
};

static int keys_order_legacy[ NB_KEYS ] = {
    HPKEY_A,          HPKEY_B,  HPKEY_C,         HPKEY_D,      HPKEY_E,        HPKEY_F,           HPKEY_G,     HPKEY_H, HPKEY_I,
    HPKEY_UP,         HPKEY_J,  HPKEY_K,         HPKEY_L,      HPKEY_LEFT,     HPKEY_DOWN,        HPKEY_RIGHT, HPKEY_M, HPKEY_N,
    HPKEY_O,          HPKEY_P,  HPKEY_BACKSPACE, HPKEY_Q,      HPKEY_R,        HPKEY_S,           HPKEY_T,     HPKEY_U, HPKEY_ENTER,
    HPKEY_V,          HPKEY_W,  HPKEY_X,         HPKEY_Y,      HPKEY_ALPHA,    HPKEY_7,           HPKEY_8,     HPKEY_9, HPKEY_Z,
    HPKEY_SHIFT_LEFT, HPKEY_4,  HPKEY_5,         HPKEY_6,      HPKEY_MULTIPLY, HPKEY_SHIFT_RIGHT, HPKEY_1,     HPKEY_2, HPKEY_3,
    HPKEY_MINUS,      HPKEY_ON, HPKEY_0,         HPKEY_PERIOD, HPKEY_SPACE,    HPKEY_PLUS,
};

#define NORMALIZED_KEYS_ORDER( hpkey ) ( ( opt.legacy_keyboard ? keys_order_legacy : keys_order_normal )[ hpkey ] )

static GtkWidget* window;

static x50ng_ui_button_t* buttons;

static GtkWidget* lcd_canvas;
static cairo_surface_t* lcd_surface;

static GtkWidget* ui_ann_left;
static GtkWidget* ui_ann_right;
static GtkWidget* ui_ann_alpha;
static GtkWidget* ui_ann_battery;
static GtkWidget* ui_ann_busy;
static GtkWidget* ui_ann_io;

/*************************/
/* functions' prototypes */
/*************************/
static void open_menu( int x, int y, x50ng_t* x50ng );

/*************/
/* functions */
/*************/
static inline void x50ng_set_key_state( x50ng_t* x50ng, const x50ng_ui_key_t* key, bool state )
{
    if ( key->rowbit )
        s3c2410_io_port_g_update( x50ng, key->column, key->row, key->columnbit, key->rowbit, state );
    else
        s3c2410_io_port_f_set_bit( x50ng, key->eint, state );
}
#define X50NG_PRESS_KEY( x50ng, key ) x50ng_set_key_state( x50ng, key, true );
#define X50NG_RELEASE_KEY( x50ng, key ) x50ng_set_key_state( x50ng, key, false );

static void ui_release_button( x50ng_ui_button_t* button )
{
    if ( !button->down )
        return;

    x50ng_t* x50ng = button->x50ng;
    const x50ng_ui_key_t* key = button->key;

    button->down = false;
    button->hold = false;

    gtk_widget_remove_css_class( button->button, "key-down" );

    X50NG_RELEASE_KEY( x50ng, key );
}

static bool ui_press_button( x50ng_ui_button_t* button, bool hold )
{
    x50ng_t* x50ng = button->x50ng;
    const x50ng_ui_key_t* key = button->key;

    if ( button->down ) {
        if ( button->hold && hold ) {
            ui_release_button( button );
            return GDK_EVENT_STOP;
        } else
            return GDK_EVENT_PROPAGATE;
    }

    button->down = true;
    button->hold = hold;

    gtk_widget_add_css_class( button->button, "key-down" );

    X50NG_RELEASE_KEY( x50ng, key );

    return GDK_EVENT_STOP;
}

static void react_to_button_press( GtkGesture* _gesture, int _n_press, double _x, double _y, x50ng_ui_button_t* button )
{
    const x50ng_ui_key_t* key = button->key;
    x50ng_t* x50ng = button->x50ng;

    ui_press_button( button, false );

    X50NG_PRESS_KEY( x50ng, key );
}

static void react_to_button_release( GtkGesture* _gesture, int _n_press, double _x, double _y, x50ng_ui_button_t* button )
{
    ui_release_button( button );
}

static void react_to_button_right_click_release( x50ng_ui_button_t* button, GtkGesture* _gesture, int _n_press, double _x, double _y )
{
    const x50ng_ui_key_t* key = button->key;
    x50ng_t* x50ng = button->x50ng;

    button->down = true;
    button->hold = true;

    ui_press_button( button, true );

    X50NG_PRESS_KEY( x50ng, key );
}

#define KEY_PRESS 1
#define KEY_RELEASE 2
static bool react_to_key_event( int keyval, x50ng_t* x50ng, int event_type )
{
    // x50ng_ui_t* ui = x50ng->ui;

    int hpkey;
    switch ( keyval ) {
        case GDK_KEY_a:
        case GDK_KEY_F1:
            hpkey = HPKEY_A;
            break;
        case GDK_KEY_b:
        case GDK_KEY_F2:
            hpkey = HPKEY_B;
            break;
        case GDK_KEY_c:
        case GDK_KEY_F3:
            hpkey = HPKEY_C;
            break;
        case GDK_KEY_d:
        case GDK_KEY_F4:
            hpkey = HPKEY_D;
            break;
        case GDK_KEY_e:
        case GDK_KEY_F5:
            hpkey = HPKEY_E;
            break;
        case GDK_KEY_f:
        case GDK_KEY_F6:
            hpkey = HPKEY_F;
            break;
        case GDK_KEY_g:
            hpkey = HPKEY_G;
            break;
        case GDK_KEY_h:
            hpkey = HPKEY_H;
            break;
        case GDK_KEY_i:
            hpkey = HPKEY_I;
            break;
        case GDK_KEY_j:
            hpkey = HPKEY_J;
            break;
        case GDK_KEY_k:
            hpkey = HPKEY_K;
            break;
        case GDK_KEY_l:
            hpkey = HPKEY_L;
            break;
        case GDK_KEY_Up:
        case GDK_KEY_KP_Up:
            hpkey = HPKEY_UP;
            break;
        case GDK_KEY_Left:
        case GDK_KEY_KP_Left:
            hpkey = HPKEY_LEFT;
            break;
        case GDK_KEY_Down:
        case GDK_KEY_KP_Down:
            hpkey = HPKEY_DOWN;
            break;
        case GDK_KEY_Right:
        case GDK_KEY_KP_Right:
            hpkey = HPKEY_RIGHT;
            break;
        case GDK_KEY_m:
            hpkey = HPKEY_M;
            break;
        case GDK_KEY_n:
            hpkey = HPKEY_N;
            break;
        case GDK_KEY_o:
        case GDK_KEY_apostrophe:
            hpkey = HPKEY_O;
            break;
        case GDK_KEY_p:
            hpkey = HPKEY_P;
            break;
        case GDK_KEY_BackSpace:
        case GDK_KEY_Delete:
        case GDK_KEY_KP_Delete:
            hpkey = HPKEY_BACKSPACE;
            break;
        case GDK_KEY_dead_circumflex:
        case GDK_KEY_asciicircum:
        case GDK_KEY_q:
        case GDK_KEY_caret:
            hpkey = HPKEY_Q;
            break;
        case GDK_KEY_r:
            hpkey = HPKEY_R;
            break;
        case GDK_KEY_s:
            hpkey = HPKEY_S;
            break;
        case GDK_KEY_t:
            hpkey = HPKEY_T;
            break;
        case GDK_KEY_u:
            hpkey = HPKEY_U;
            break;
        case GDK_KEY_v:
            hpkey = HPKEY_V;
            break;
        case GDK_KEY_w:
            hpkey = HPKEY_W;
            break;
        case GDK_KEY_x:
            hpkey = HPKEY_X;
            break;
        case GDK_KEY_y:
            hpkey = HPKEY_Y;
            break;
        case GDK_KEY_z:
        case GDK_KEY_slash:
        case GDK_KEY_KP_Divide:
            hpkey = HPKEY_Z;
            break;
        case GDK_KEY_Tab:
#ifndef __APPLE__
        case GDK_KEY_Alt_L:
        case GDK_KEY_Alt_R:
        case GDK_KEY_Meta_L:
        case GDK_KEY_Meta_R:
        case GDK_KEY_Mode_switch:
#endif
            hpkey = HPKEY_ALPHA;
            break;
        case GDK_KEY_7:
        case GDK_KEY_KP_7:
            hpkey = HPKEY_7;
            break;
        case GDK_KEY_8:
        case GDK_KEY_KP_8:
            hpkey = HPKEY_8;
            break;
        case GDK_KEY_9:
        case GDK_KEY_KP_9:
            hpkey = HPKEY_9;
            break;
        case GDK_KEY_multiply:
        case GDK_KEY_KP_Multiply:
            hpkey = HPKEY_MULTIPLY;
            break;
        case GDK_KEY_Shift_L:
        case GDK_KEY_Shift_R:
            hpkey = HPKEY_SHIFT_LEFT;
            break;
        case GDK_KEY_4:
        case GDK_KEY_KP_4:
            hpkey = HPKEY_4;
            break;
        case GDK_KEY_5:
        case GDK_KEY_KP_5:
            hpkey = HPKEY_5;
            break;
        case GDK_KEY_6:
        case GDK_KEY_KP_6:
            hpkey = HPKEY_6;
            break;
        case GDK_KEY_minus:
        case GDK_KEY_KP_Subtract:
            hpkey = HPKEY_MINUS;
            break;
        case GDK_KEY_Control_L:
        case GDK_KEY_Control_R:
            hpkey = HPKEY_SHIFT_RIGHT;
            break;
        case GDK_KEY_1:
        case GDK_KEY_KP_1:
            hpkey = HPKEY_1;
            break;
        case GDK_KEY_2:
        case GDK_KEY_KP_2:
            hpkey = HPKEY_2;
            break;
        case GDK_KEY_3:
        case GDK_KEY_KP_3:
            hpkey = HPKEY_3;
            break;
        case GDK_KEY_plus:
        case GDK_KEY_KP_Add:
            hpkey = HPKEY_PLUS;
            break;
        case GDK_KEY_Escape:
            hpkey = HPKEY_ON;
            break;
        case GDK_KEY_0:
        case GDK_KEY_KP_0:
            hpkey = HPKEY_0;
            break;
        case GDK_KEY_period:
        case GDK_KEY_comma:
        case GDK_KEY_KP_Decimal:
        case GDK_KEY_KP_Separator:
            hpkey = HPKEY_PERIOD;
            break;
        case GDK_KEY_space:
        case GDK_KEY_KP_Space:
            hpkey = HPKEY_SPACE;
            break;
        case GDK_KEY_Return:
        case GDK_KEY_KP_Enter:
            hpkey = HPKEY_ENTER;
            break;

        /* QWERTY compat: US English, UK English, International English */
        case GDK_KEY_backslash:
            hpkey = HPKEY_MULTIPLY;
            break;
        case GDK_KEY_equal:
            hpkey = HPKEY_PLUS;
            break;

        /* QWERTZ compat: German */
        case GDK_KEY_ssharp:
            hpkey = HPKEY_Z;
            break;
        case GDK_KEY_numbersign:
            hpkey = HPKEY_MULTIPLY;
            break;

        case GDK_KEY_F7:
        case GDK_KEY_F10:
            x50ng->arm_exit = 1;
            cpu_exit( x50ng->env );
            return GDK_EVENT_STOP;

        case GDK_KEY_F12:
            switch ( event_type ) {
                case KEY_PRESS:
                    x50ng_modules_reset( x50ng, X50NG_RESET_POWER_ON );
                    cpu_reset( x50ng->env );
                    x50ng_set_idle( x50ng, 1 );
                    break;
                case KEY_RELEASE:
                    x50ng_set_idle( x50ng, 0 );
                    break;
                default:
                    break;
            }
            return GDK_EVENT_STOP;

        case GDK_KEY_Menu:
            open_menu( ( LCD_WIDTH * opt.zoom ) / 2, ( LCD_HEIGHT * opt.zoom ) / 2, x50ng );
            return GDK_EVENT_STOP;

        default:
            return GDK_EVENT_PROPAGATE;
    }

    // Bypassing GUI buttons:
    /* switch ( event_type ) { */
    /*     case KEY_PRESS: */
    /*         X50NG_PRESS_KEY( x50ng, &ui_keys[ hpkey ] ); */
    /*         break; */
    /*     case KEY_RELEASE: */
    /*         X50NG_RELEASE_KEY( x50ng, &ui_keys[ hpkey ] ); */
    /*         break; */
    /*     default: */
    /*         return GDK_EVENT_PROPAGATE; */
    /* } */

    // Using GUI buttons:
    x50ng_ui_button_t* button = &buttons[ hpkey ];

    switch ( event_type ) {
        case KEY_PRESS:
            react_to_button_press( NULL, 0, 0, 0, button );
            break;
        case KEY_RELEASE:
            react_to_button_release( NULL, 0, 0, 0, button );
            break;
        default:
            return GDK_EVENT_PROPAGATE;
    }

    return GDK_EVENT_STOP;
}

static void mount_sd_folder_file_dialog_callback( GtkFileDialog* dialog, GAsyncResult* result, x50ng_t* x50ng )
{
    g_autoptr( GFile ) file = gtk_file_dialog_select_folder_finish( dialog, result, NULL );

    if ( file != NULL )
        s3c2410_sdi_mount( x50ng, ( char* )g_file_peek_path( file ) );
}

static void do_select_and_mount_sd_folder( x50ng_t* x50ng, GMenuItem* _menuitem )
{
    g_autoptr( GtkFileDialog ) dialog =
        g_object_new( GTK_TYPE_FILE_DIALOG, "title", "Choose SD folder…", "accept-label", "_Open", "modal", TRUE, NULL );

    gtk_file_dialog_select_folder( dialog, GTK_WINDOW( window ), NULL, ( GAsyncReadyCallback )mount_sd_folder_file_dialog_callback, x50ng );
}

static void do_start_gdb_server( GMenuItem* _menuitem, x50ng_t* x50ng )
{
    if ( opt.debug_port != 0 && !gdbserver_isactive() ) {
        gdbserver_start( opt.debug_port );
        gdb_handlesig( x50ng->env, 0 );
    }
}

static void do_reset( x50ng_t* x50ng, GMenuItem* _menuitem )
{
    x50ng_modules_reset( x50ng, X50NG_RESET_POWER_ON );
    cpu_reset( x50ng->env );
    x50ng_set_idle( x50ng, 0 );
}

#ifdef TEST_PASTE
static void x50g_string_to_keys_sequence( x50ng_t* x50ng, const char* input )
{
    for ( int i = 0; i < strlen( input ); i++ ) {
        fprintf( stderr, "%c", input[ i ] );
    }
    fprintf( stderr, "\n" );
}

static void paste_callback( GdkClipboard* source, GAsyncResult* result, x50ng_t* x50ng )
{
    g_autofree char* text = NULL;
    g_autoptr( GError ) error = NULL;

    text = gdk_clipboard_read_text_finish( source, result, &error );

    if ( error ) {
        g_critical( "Couldn't paste text: %s\n", error->message );
        return;
    }

    x50g_string_to_keys_sequence( x50ng, text );
}

static void do_paste( x50ng_t* x50ng, GtkWidget* _menuitem )
{
    gdk_clipboard_read_text_async( gdk_display_get_clipboard( gdk_display_get_default() ), NULL, ( GAsyncReadyCallback )paste_callback,
                                   x50ng );
}
#endif

static void do_quit( x50ng_t* x50ng, GtkWidget* _menuitem ) { x50ng->arm_exit++; }

static void open_menu( int x, int y, x50ng_t* x50ng )
{
    g_autoptr( GMenu ) menu = g_menu_new();
    g_autoptr( GSimpleActionGroup ) action_group = g_simple_action_group_new();

#ifdef TEST_PASTE
    g_autoptr( GSimpleAction ) act_paste = g_simple_action_new( "paste", NULL );
    g_signal_connect_swapped( act_paste, "activate", G_CALLBACK( do_paste ), x50ng );
    g_action_map_add_action( G_ACTION_MAP( action_group ), G_ACTION( act_paste ) );
    g_menu_append( menu, "Paste", "app.paste" );
#endif

    g_autoptr( GSimpleAction ) act_mount_SD = g_simple_action_new( "mount_SD", NULL );
    g_signal_connect_swapped( act_mount_SD, "activate", G_CALLBACK( do_select_and_mount_sd_folder ), x50ng );
    if ( !s3c2410_sdi_is_mounted( x50ng ) )
        g_action_map_add_action( G_ACTION_MAP( action_group ), G_ACTION( act_mount_SD ) );
    g_menu_append( menu, "Mount SD folder…", "app.mount_SD" );

    g_autoptr( GSimpleAction ) act_unmount_SD = g_simple_action_new( "unmount_SD", NULL );
    g_signal_connect_swapped( act_unmount_SD, "activate", G_CALLBACK( s3c2410_sdi_unmount ), x50ng );
    char* unmount_label;
    if ( s3c2410_sdi_is_mounted( x50ng ) ) {
        g_action_map_add_action( G_ACTION_MAP( action_group ), G_ACTION( act_unmount_SD ) );
        char* sd_path;
        s3c2410_sdi_get_path( x50ng, &sd_path );
        if ( -1 == asprintf( &unmount_label, "Unmount SD (%s)", sd_path ) )
            exit( EXIT_FAILURE );
        free( sd_path );
    } else if ( -1 == asprintf( &unmount_label, "Unmount SD" ) )
        exit( EXIT_FAILURE );
    g_menu_append( menu, unmount_label, "app.unmount_SD" );
    free( unmount_label );

    g_autoptr( GSimpleAction ) act_debug = g_simple_action_new( "debug", NULL );
    g_signal_connect_swapped( act_debug, "activate", G_CALLBACK( do_start_gdb_server ), x50ng );
    if ( opt.debug_port != 0 )
        g_action_map_add_action( G_ACTION_MAP( action_group ), G_ACTION( act_debug ) );
    g_menu_append( menu, "Start gdb server", "app.debug" );

    g_autoptr( GSimpleAction ) act_reset = g_simple_action_new( "reset", NULL );
    g_signal_connect_swapped( act_reset, "activate", G_CALLBACK( do_reset ), x50ng );
    g_action_map_add_action( G_ACTION_MAP( action_group ), G_ACTION( act_reset ) );
    g_menu_append( menu, "Reset", "app.reset" );

    g_autoptr( GSimpleAction ) act_quit = g_simple_action_new( "quit", NULL );
    g_signal_connect_swapped( act_quit, "activate", G_CALLBACK( do_quit ), x50ng );
    g_action_map_add_action( G_ACTION_MAP( action_group ), G_ACTION( act_quit ) );
    g_menu_append( menu, "Quit", "app.quit" );

    GtkWidget* popup = gtk_popover_menu_new_from_model( G_MENU_MODEL( menu ) );
    gtk_widget_insert_action_group( popup, "app", G_ACTION_GROUP( action_group ) );

    GdkRectangle rect;
    rect.x = x;
    rect.y = y;
    rect.width = rect.height = 1;
    gtk_popover_set_pointing_to( GTK_POPOVER( popup ), &rect );

    gtk_widget_set_parent( GTK_WIDGET( popup ), window );
    gtk_popover_set_position( GTK_POPOVER( popup ), GTK_POS_BOTTOM );
    gtk_popover_popup( GTK_POPOVER( popup ) );
}

static void redraw_lcd( GtkDrawingArea* _widget, cairo_t* cr, int width, int height, gpointer _user_data )
{
    cairo_pattern_t* lcd_pattern = cairo_pattern_create_for_surface( lcd_surface );
    cairo_pattern_set_filter( lcd_pattern, CAIRO_FILTER_FAST );
    cairo_scale( cr, ( double )width / ( double )LCD_WIDTH, ( double )height / ( double )LCD_HEIGHT );
    cairo_set_source( cr, lcd_pattern );

    cairo_paint( cr );
}

static bool react_to_key_press( GtkEventControllerKey* controller, guint keyval, guint keycode, GdkModifierType state, x50ng_t* x50ng )
{
    return react_to_key_event( keyval, x50ng, KEY_PRESS );
}

static bool react_to_key_release( GtkEventControllerKey* controller, guint keyval, guint keycode, GdkModifierType state, x50ng_t* x50ng )
{
    return react_to_key_event( keyval, x50ng, KEY_RELEASE );
}

static void react_to_display_click( x50ng_t* x50ng, GtkEventController* _gesture, gdouble x, gdouble y )
{
    open_menu( ( int )x, ( int )y, x50ng );
}

static inline void _ui_load__newrplify_ui_keys()
{
    // modify keys' labeling for newRPL
    for ( int i = HPKEY_A; i <= HPKEY_F; i++ )
        ui_keys[ i ].left = "";

    for ( int i = HPKEY_G; i <= HPKEY_I; i++ ) {
        ui_keys[ i ].label = "";
        ui_keys[ i ].left = "";
        ui_keys[ i ].right = NULL;
    }

    for ( int i = HPKEY_J; i <= HPKEY_K; i++ ) {
        ui_keys[ i ].label = "";
        ui_keys[ i ].left = "";
        ui_keys[ i ].right = NULL;
    }

    ui_keys[ HPKEY_UP ].left = "UPDIR";

    ui_keys[ HPKEY_LEFT ].left = "BEG";
    ui_keys[ HPKEY_LEFT ].right = "COPY";

    ui_keys[ HPKEY_DOWN ].left = "CUT";

    ui_keys[ HPKEY_RIGHT ].left = "END";
    ui_keys[ HPKEY_RIGHT ].right = "PASTE";

    ui_keys[ HPKEY_M ].label = "STO⏵";
    ui_keys[ HPKEY_M ].left = "RCL";
    ui_keys[ HPKEY_M ].right = "PREV.M";

    for ( int i = HPKEY_N; i <= HPKEY_O; i++ ) {
        ui_keys[ i ].left = "";
        ui_keys[ i ].right = NULL;
    }

    ui_keys[ HPKEY_P ].label = "MENU";

    ui_keys[ HPKEY_BACKSPACE ].left = "";

    for ( int i = HPKEY_S; i <= HPKEY_U; i++ )
        ui_keys[ i ].right = NULL;

    for ( int i = HPKEY_ALPHA; i <= HPKEY_9; i++ )
        ui_keys[ i ].left = "";

    ui_keys[ HPKEY_8 ].right = NULL;

    for ( int i = HPKEY_4; i <= HPKEY_6; i++ ) {
        ui_keys[ i ].left = "";
        ui_keys[ i ].right = NULL;
    }

    ui_keys[ HPKEY_2 ].left = "";

    ui_keys[ HPKEY_ON ].left = "";
    ui_keys[ HPKEY_ON ].below = NULL;

    ui_keys[ HPKEY_ENTER ].left = "";
}

static GtkWidget* _ui_load__create_annunciator_widget( const char* label )
{
    GtkWidget* ui_ann = gtk_label_new( NULL );
    gtk_widget_add_css_class( ui_ann, "annunciator" );
    gtk_widget_set_name( ui_ann, label );

    gtk_label_set_use_markup( GTK_LABEL( ui_ann ), true );
    gtk_label_set_markup( GTK_LABEL( ui_ann ), label );

    return ui_ann;
}

static GtkWidget* _ui_load__create_label( const char* css_class, const char* text )
{
    GtkWidget* ui_label = gtk_label_new( NULL );
    gtk_widget_add_css_class( ui_label, css_class );

    gtk_label_set_use_markup( GTK_LABEL( ui_label ), true );
    gtk_label_set_markup( GTK_LABEL( ui_label ), text );

    return ui_label;
}

#ifdef USE_GTK_APPLICATION
static int ui_load( GtkApplication* app, x50ng_t* x50ng )
#else
static int ui_load( x50ng_t* x50ng )
#endif
{
    // create window and widgets/stuff
#ifdef USE_GTK_APPLICATION
    window = gtk_application_window_new( app );
#else
    window = gtk_window_new();
#endif
    gtk_window_set_decorated( GTK_WINDOW( window ), true );
    gtk_window_set_resizable( GTK_WINDOW( window ), true );
    gtk_window_set_title( GTK_WINDOW( window ), opt.name );
    gtk_window_set_decorated( GTK_WINDOW( window ), true );
    // Sets the title of this instance
    g_set_application_name( opt.name );
    // Sets the app_id of all instances
    g_set_prgname( x50ng->progname );

    GtkWidget* window_container = gtk_box_new( opt.netbook ? GTK_ORIENTATION_HORIZONTAL : GTK_ORIENTATION_VERTICAL, 0 );
    gtk_widget_add_css_class( window_container, "window-container" );
    gtk_widget_set_name( window_container, "window-container" );

    gtk_window_set_child( ( GtkWindow* )window, window_container );

    g_signal_connect_swapped( G_OBJECT( window ), "destroy", G_CALLBACK( do_quit ), x50ng );

    GtkEventController* keys_controller = gtk_event_controller_key_new();
    g_signal_connect( keys_controller, "key-pressed", G_CALLBACK( react_to_key_press ), x50ng );
    g_signal_connect( keys_controller, "key-released", G_CALLBACK( react_to_key_release ), x50ng );
    gtk_widget_add_controller( window, keys_controller );

    /* for --netbook */
    GtkWidget* upper_left_container = gtk_box_new( GTK_ORIENTATION_VERTICAL, 0 );
    gtk_widget_add_css_class( upper_left_container, "upper-left-container" );
    gtk_widget_set_name( upper_left_container, "upper-left-container" );
    gtk_box_append( ( GTK_BOX( window_container ) ), upper_left_container );

    GtkWidget* downer_right_container = gtk_box_new( GTK_ORIENTATION_VERTICAL, 0 );
    gtk_widget_set_vexpand( GTK_WIDGET( downer_right_container ), true );
    gtk_widget_add_css_class( downer_right_container, "downer-right-container" );
    gtk_widget_set_name( downer_right_container, "downer-right-container" );
    gtk_box_append( GTK_BOX( window_container ), downer_right_container );

    GtkWidget* header_container = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 0 );
    gtk_widget_add_css_class( header_container, "header-container" );
    gtk_widget_set_name( header_container, "header-container" );
    gtk_box_append( ( GTK_BOX( upper_left_container ) ), header_container );

    lcd_canvas = gtk_drawing_area_new();
    gtk_widget_add_css_class( lcd_canvas, "lcd" );
    gtk_widget_set_name( lcd_canvas, "lcd" );

    gtk_drawing_area_set_content_width( GTK_DRAWING_AREA( lcd_canvas ), ( LCD_WIDTH * opt.zoom ) );
    gtk_drawing_area_set_content_height( GTK_DRAWING_AREA( lcd_canvas ), ( LCD_HEIGHT * opt.zoom ) );
    gtk_drawing_area_set_draw_func( GTK_DRAWING_AREA( lcd_canvas ), redraw_lcd, x50ng, NULL );

    GtkWidget* lcd_container = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 0 );
    gtk_widget_set_halign( lcd_container, GTK_ALIGN_CENTER );
    gtk_widget_add_css_class( lcd_container, "lcd-container" );
    gtk_widget_set_name( lcd_container, "lcd-container" );

    gtk_widget_set_size_request( lcd_container, ( LCD_WIDTH * opt.zoom ), ( LCD_HEIGHT * opt.zoom ) );
    gtk_box_append( GTK_BOX( lcd_container ), lcd_canvas );
    gtk_widget_set_halign( GTK_WIDGET( lcd_canvas ), GTK_ALIGN_CENTER );
    gtk_widget_set_hexpand( GTK_WIDGET( lcd_canvas ), false );

    GtkWidget* annunciators_container = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 0 );
    gtk_box_set_homogeneous( GTK_BOX( annunciators_container ), true );
    gtk_widget_add_css_class( annunciators_container, "annunciators-container" );
    gtk_widget_set_name( annunciators_container, "annunciators-container" );

    ui_ann_left = _ui_load__create_annunciator_widget( "⮢" );
    ui_ann_right = _ui_load__create_annunciator_widget( "⮣" );
    ui_ann_alpha = _ui_load__create_annunciator_widget( "α" );
    ui_ann_battery = _ui_load__create_annunciator_widget( "🪫" );
    ui_ann_busy = _ui_load__create_annunciator_widget( "⌛" );
    ui_ann_io = _ui_load__create_annunciator_widget( "⇄" );

    gtk_box_append( GTK_BOX( annunciators_container ), ui_ann_left );
    gtk_box_append( GTK_BOX( annunciators_container ), ui_ann_right );
    gtk_box_append( GTK_BOX( annunciators_container ), ui_ann_alpha );
    gtk_box_append( GTK_BOX( annunciators_container ), ui_ann_battery );
    gtk_box_append( GTK_BOX( annunciators_container ), ui_ann_busy );
    gtk_box_append( GTK_BOX( annunciators_container ), ui_ann_io );

    GtkWidget* display_container = gtk_box_new( GTK_ORIENTATION_VERTICAL, 0 );
    gtk_widget_add_css_class( annunciators_container, "display-container" );
    gtk_widget_set_name( display_container, "display-container" );

    gtk_box_append( GTK_BOX( display_container ), annunciators_container );
    gtk_box_append( GTK_BOX( display_container ), lcd_container );

    gtk_box_append( GTK_BOX( upper_left_container ), display_container );

    GtkGesture* right_click_controller = gtk_gesture_click_new();
    gtk_gesture_single_set_button( GTK_GESTURE_SINGLE( right_click_controller ), 3 );
    g_signal_connect_swapped( right_click_controller, "pressed", G_CALLBACK( react_to_display_click ), x50ng );
    gtk_widget_add_controller( display_container, GTK_EVENT_CONTROLLER( right_click_controller ) );

    // keyboard
    GtkWidget* high_keyboard_container = gtk_box_new( GTK_ORIENTATION_VERTICAL, 0 );
    gtk_widget_add_css_class( high_keyboard_container, "keyboard-container" );
    gtk_widget_set_name( high_keyboard_container, "high-keyboard-container" );

    gtk_box_set_homogeneous( GTK_BOX( high_keyboard_container ), true );

    gtk_box_append( GTK_BOX( upper_left_container ), high_keyboard_container );

    GtkWidget* low_keyboard_container = gtk_box_new( GTK_ORIENTATION_VERTICAL, 0 );
    gtk_widget_set_valign( GTK_WIDGET( low_keyboard_container ), GTK_ALIGN_END );
    gtk_widget_set_vexpand( GTK_WIDGET( low_keyboard_container ), true );
    gtk_widget_add_css_class( low_keyboard_container, "keyboard-container" );
    gtk_widget_set_name( low_keyboard_container, "low-keyboard-container" );

    gtk_box_set_homogeneous( GTK_BOX( low_keyboard_container ), true );

    gtk_box_append( GTK_BOX( downer_right_container ), low_keyboard_container );

    x50ng_ui_button_t* button;

    if ( opt.newrpl_keyboard )
        _ui_load__newrplify_ui_keys();

    GtkWidget* rows_containers[ KB_NB_ROWS ];
    GtkWidget* keys_containers[ NB_KEYS ];
    GtkWidget* keys_top_labels_containers[ NB_KEYS ];

    int key_index = 0;
    int nb_keys_in_row = 0;
    for ( int row = 0; row < KB_NB_ROWS; row++ ) {
        rows_containers[ row ] = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 0 );
        gtk_widget_add_css_class( rows_containers[ row ], "row-container" );
        gtk_box_set_homogeneous( GTK_BOX( rows_containers[ row ] ), true );
        gtk_box_append( ( GTK_BOX( row < opt.netbook_pivot_line ? high_keyboard_container : low_keyboard_container ) ),
                        rows_containers[ row ] );

        switch ( row ) {
            case 1:
                nb_keys_in_row = 4;
                break;
            case 0:
            case 2:
                nb_keys_in_row = 6;
                break;
            default:
                nb_keys_in_row = 5;
        }

        for ( int column = 0; column < nb_keys_in_row; column++ ) {
            keys_containers[ key_index ] = gtk_box_new( GTK_ORIENTATION_VERTICAL, 0 );
            gtk_widget_add_css_class( keys_containers[ key_index ], "key-container" );
            gtk_box_set_homogeneous( GTK_BOX( keys_containers[ key_index ] ), false );
            if ( row == 1 && column == 3 )
                gtk_box_append( GTK_BOX( rows_containers[ row ] ), gtk_box_new( GTK_ORIENTATION_VERTICAL, 2 ) );
            gtk_box_append( GTK_BOX( rows_containers[ row ] ), keys_containers[ key_index ] );
            if ( row == 1 && column == 3 )
                gtk_box_append( GTK_BOX( rows_containers[ row ] ), gtk_box_new( GTK_ORIENTATION_VERTICAL, 2 ) );

            button = &buttons[ NORMALIZED_KEYS_ORDER( key_index ) ];
            button->x50ng = x50ng;
            button->key = &ui_keys[ NORMALIZED_KEYS_ORDER( key_index ) ];

            keys_top_labels_containers[ key_index ] = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 0 );
            gtk_widget_add_css_class( keys_top_labels_containers[ key_index ], "top-labels-container" );

            gtk_box_append( GTK_BOX( keys_containers[ key_index ] ), keys_top_labels_containers[ key_index ] );

            GtkWidget* label_left = NULL;
            if ( button->key->left )
                label_left = _ui_load__create_label( "label-left", button->key->left );

            GtkWidget* label_right = NULL;
            if ( button->key->right )
                label_right = _ui_load__create_label( "label-right", button->key->right );

            if ( button->key->left && button->key->right ) {
                gtk_box_append( GTK_BOX( keys_top_labels_containers[ key_index ] ), label_left );
                gtk_widget_set_halign( GTK_WIDGET( label_left ), GTK_ALIGN_START );

                gtk_box_append( GTK_BOX( keys_top_labels_containers[ key_index ] ), label_right );
                gtk_widget_set_halign( GTK_WIDGET( label_right ), GTK_ALIGN_END );
                gtk_widget_set_hexpand( label_right, true );
            } else if ( button->key->left ) {
                gtk_widget_set_halign( GTK_WIDGET( keys_top_labels_containers[ key_index ] ), GTK_ALIGN_CENTER );

                gtk_box_append( GTK_BOX( keys_top_labels_containers[ key_index ] ), label_left );
                gtk_widget_set_halign( GTK_WIDGET( label_left ), GTK_ALIGN_CENTER );
                gtk_widget_set_hexpand( GTK_WIDGET( label_left ), false );
            }

            button->button = gtk_button_new();
            gtk_widget_add_css_class( button->button, "key" );
            gtk_widget_add_css_class( button->button, button->key->css_class );
            gtk_widget_set_name( button->button, button->key->css_id );

            // There's always a label, even if it's empty.
            GtkWidget* label = _ui_load__create_label( "label-key", button->key->label );
            gtk_button_set_child( GTK_BUTTON( button->button ), label );

            gtk_widget_set_can_focus( button->button, false );
            GtkGesture* left_click_controller = gtk_gesture_click_new();
            gtk_gesture_single_set_button( GTK_GESTURE_SINGLE( left_click_controller ), 1 );
            g_signal_connect( left_click_controller, "pressed", G_CALLBACK( react_to_button_press ), button );
            g_signal_connect( left_click_controller, /* "released" */ "end", G_CALLBACK( react_to_button_release ), button );
            /* Here we attach the controller to the label because… gtk4 reasons? gtk4 button only handles 'clicked' event now but we
             * actually need pressed and released (AKA end?) */
            gtk_widget_add_controller( label, GTK_EVENT_CONTROLLER( left_click_controller ) );

            GtkGesture* right_click_controller = gtk_gesture_click_new();
            gtk_gesture_single_set_button( GTK_GESTURE_SINGLE( right_click_controller ), 3 );
            g_signal_connect_swapped( right_click_controller, /* "released" */ "pressed", G_CALLBACK( react_to_button_right_click_release ),
                                      button );
            gtk_widget_add_controller( label, GTK_EVENT_CONTROLLER( right_click_controller ) );

            gtk_box_append( GTK_BOX( keys_containers[ key_index ] ), button->button );

            if ( button->key->below )
                gtk_box_append( GTK_BOX( keys_containers[ key_index ] ), _ui_load__create_label( "label-below", button->key->below ) );
            if ( button->key->letter )
                gtk_box_append( GTK_BOX( keys_containers[ key_index ] ), _ui_load__create_label( "label-letter", button->key->letter ) );

            key_index++;
        }
    }

    // Apply CSS
    char* style_full_path = g_build_filename( opt.style_filename, NULL );
    if ( !g_file_test( style_full_path, G_FILE_TEST_EXISTS ) )
        style_full_path = g_build_filename( opt.datadir, opt.style_filename, NULL );
    if ( !g_file_test( style_full_path, G_FILE_TEST_EXISTS ) )
        style_full_path = g_build_filename( GLOBAL_DATADIR, opt.style_filename, NULL );
    if ( !g_file_test( style_full_path, G_FILE_TEST_EXISTS ) )
        style_full_path = g_build_filename( x50ng->progpath, opt.style_filename, NULL );

    if ( !g_file_test( style_full_path, G_FILE_TEST_EXISTS ) )
        fprintf( stderr, "Can't load style %s neither from %s/%s nor from %s/%s nor from %s/%s\n", opt.style_filename, opt.datadir,
                 opt.style_filename, GLOBAL_DATADIR, opt.style_filename, x50ng->progpath, opt.style_filename );
    else {
        g_autoptr( GtkCssProvider ) style_provider = gtk_css_provider_new();
        gtk_css_provider_load_from_path( style_provider, style_full_path );

        gtk_style_context_add_provider_for_display( gdk_display_get_default(), GTK_STYLE_PROVIDER( style_provider ),
                                                    GTK_STYLE_PROVIDER_PRIORITY_USER + 1 );

        if ( opt.verbose )
            fprintf( stderr, "Loaded style from %s\n", style_full_path );
    }
    free( style_full_path );

    // finally show the window
    gtk_widget_realize( window );
    gtk_window_present( GTK_WINDOW( window ) );

    return 0;
}

/**********/
/* timers */
/**********/
void ui_events_timer( void* data )
{
    x50ng_t* x50ng = data;

    while ( g_main_context_pending( NULL ) )
        g_main_context_iteration( NULL, false );

    x50ng_mod_timer( x50ng->ui_timer, x50ng_get_clock() + UI_EVENTS_REFRESH_INTERVAL );
}

void ui_lcd_timer( void* data )
{
    x50ng_t* x50ng = data;
    int64_t now, expires;

    ui_update_lcd( x50ng );
    gdk_display_flush( gdk_display_get_default() );

    now = x50ng_get_clock();
    expires = now + UI_LCD_REFRESH_INTERVAL;

    x50ng_mod_timer( x50ng->lcd_timer, expires );
}

/********************/
/* Public functions */
/********************/

void ui_update_lcd( x50ng_t* x50ng )
{
    s3c2410_lcd_t* lcd = x50ng->s3c2410_lcd;

    if ( !( lcd->lcdcon1 & 1 ) )
        return;

    gtk_widget_set_opacity( ui_ann_left, x50ng_get_pixel_color( lcd, LCD_WIDTH, 1 ) );
    gtk_widget_set_opacity( ui_ann_right, x50ng_get_pixel_color( lcd, LCD_WIDTH, 2 ) );
    gtk_widget_set_opacity( ui_ann_alpha, x50ng_get_pixel_color( lcd, LCD_WIDTH, 3 ) );
    gtk_widget_set_opacity( ui_ann_battery, x50ng_get_pixel_color( lcd, LCD_WIDTH, 4 ) );
    gtk_widget_set_opacity( ui_ann_busy, x50ng_get_pixel_color( lcd, LCD_WIDTH, 5 ) );
    gtk_widget_set_opacity( ui_ann_io, x50ng_get_pixel_color( lcd, LCD_WIDTH, 0 ) );

    if ( NULL != lcd_surface )
        g_free( lcd_surface );
    lcd_surface = cairo_image_surface_create( CAIRO_FORMAT_ARGB32, LCD_WIDTH, LCD_HEIGHT );
    cairo_t* cr = cairo_create( lcd_surface );

    for ( int y = 0; y < LCD_HEIGHT; y++ ) {
        for ( int x = 0; x < LCD_WIDTH; x++ ) {
            cairo_set_source_rgba( cr, 0, 0, 0, x50ng_get_pixel_color( lcd, x, y ) / 15.0 );
            cairo_rectangle( cr, x, y, 1.0, 1.0 );
            cairo_fill( cr );
        }
    }

    cairo_destroy( cr );

    gtk_widget_queue_draw( lcd_canvas );
}

#ifdef USE_GTK_APPLICATION
void ui_init( GtkApplication* app, x50ng_t* x50ng )
#else
void ui_init( x50ng_t* x50ng )
#endif
{
    buttons = malloc( NB_KEYS * sizeof( x50ng_ui_button_t ) );
    if ( NULL == buttons ) {
        fprintf( stderr, "%s:%u: Out of memory\n", __func__, __LINE__ );
        return;
    }
    memset( buttons, 0, NB_KEYS * sizeof( x50ng_ui_button_t ) );

    gtk_init();

#ifdef USE_GTK_APPLICATION
    ui_load( app, x50ng );
#else
    ui_load( x50ng );
#endif
}
