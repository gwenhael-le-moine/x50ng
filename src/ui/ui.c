#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <sys/times.h>

#include "../options.h"
#include "../types.h"

#include "../s3c2410/s3c2410.h"

#include "inner.h"
#include "gtk.h"
#include "ncurses.h"

x50ng_ui_key_t ui_keys[ NB_HP50g_KEYS ] = {
    {.css_class = "menu",        .css_id = "F1", .label = "F1", .letter = "A", .left = "Y=", .right = NULL, .below = NULL, .hpkey = HP50g_KEY_A },
    {.css_class = "menu",
     .css_id = "F2",
     .label = "F2",
     .letter = "B",
     .left = "WIN",
     .right = NULL,
     .below = NULL,
     .hpkey = HP50g_KEY_B                                                                                                                       },
    {.css_class = "menu",
     .css_id = "F3",
     .label = "F3",
     .letter = "C",
     .left = "GRAPH",
     .right = NULL,
     .below = NULL,
     .hpkey = HP50g_KEY_C                                                                                                                       },
    {.css_class = "menu",
     .css_id = "F4",
     .label = "F4",
     .letter = "D",
     .left = "2D/3D",
     .right = NULL,
     .below = NULL,
     .hpkey = HP50g_KEY_D                                                                                                                       },
    {.css_class = "menu",
     .css_id = "F5",
     .label = "F5",
     .letter = "E",
     .left = "TBLSET",
     .right = NULL,
     .below = NULL,
     .hpkey = HP50g_KEY_E                                                                                                                       },
    {.css_class = "menu",
     .css_id = "F6",
     .label = "F6",
     .letter = "F",
     .left = "TABLE",
     .right = NULL,
     .below = NULL,
     .hpkey = HP50g_KEY_F                                                                                                                       },

    {.css_class = "function",
     .css_id = "APPS",
     .label = "APPS",
     .letter = "G",
     .left = "FILES",
     .right = "BEGIN",
     .below = NULL,
     .hpkey = HP50g_KEY_G                                                                                                                       },
    {.css_class = "function",
     .css_id = "MODE",
     .label = "MODE",
     .letter = "H",
     .left = "CUSTOM",
     .right = "END",
     .below = NULL,
     .hpkey = HP50g_KEY_H                                                                                                                       },
    {.css_class = "function",
     .css_id = "TOOL",
     .label = "TOOL",
     .letter = "I",
     .left = "i",
     .right = "I",
     .below = NULL,
     .hpkey = HP50g_KEY_I                                                                                                                       },
    {.css_class = "arrow",       .css_id = "UP", .label = "⬆",  .letter = "",  .left = "",   .right = NULL, .below = NULL, .hpkey = HP50g_KEY_UP},
    {.css_class = "function",
     .css_id = "VAR",
     .label = "VAR",
     .letter = "J",
     .left = "UPDIR",
     .right = "COPY",
     .below = NULL,
     .hpkey = HP50g_KEY_J                                                                                                                       },
    {.css_class = "function",
     .css_id = "STO",
     .label = "STO⏵",
     .letter = "K",
     .left = "RCL",
     .right = "CUT",
     .below = NULL,
     .hpkey = HP50g_KEY_K                                                                                                                       },
    {.css_class = "function",
     .css_id = "NXT",
     .label = "NXT",
     .letter = "L",
     .left = "PREV",
     .right = "PASTE",
     .below = NULL,
     .hpkey = HP50g_KEY_L                                                                                                                       },

    {.css_class = "arrow",
     .css_id = "LEFT",
     .label = "⬅",
     .letter = "",
     .left = "",
     .right = NULL,
     .below = NULL,
     .hpkey = HP50g_KEY_LEFT                                                                                                                    },
    {.css_class = "arrow",
     .css_id = "DOWN",
     .label = "⬇",
     .letter = "",
     .left = "",
     .right = NULL,
     .below = NULL,
     .hpkey = HP50g_KEY_DOWN                                                                                                                    },
    {.css_class = "arrow",
     .css_id = "RIGHT",
     .label = "➡",
     .letter = "",
     .left = "",
     .right = NULL,
     .below = NULL,
     .hpkey = HP50g_KEY_RIGHT                                                                                                                   },

    {.css_class = "function",
     .css_id = "HIST",
     .label = "HIST",
     .letter = "M",
     .left = "CMD",
     .right = "UNDO",
     .below = NULL,
     .hpkey = HP50g_KEY_M                                                                                                                       },
    {.css_class = "function",
     .css_id = "EVAL",
     .label = "EVAL",
     .letter = "N",
     .left = "PRG",
     .right = "CHARS",
     .below = NULL,
     .hpkey = HP50g_KEY_N                                                                                                                       },
    {.css_class = "function",
     .css_id = "QUOTE",
     .label = "'",
     .letter = "O",
     .left = "MTRW",
     .right = "EQW",
     .below = NULL,
     .hpkey = HP50g_KEY_O                                                                                                                       },
    {.css_class = "function",
     .css_id = "SYMB",
     .label = "SYMB",
     .letter = "P",
     .left = "MTH",
     .right = "CAT",
     .below = NULL,
     .hpkey = HP50g_KEY_P                                                                                                                       },
    {.css_class = "function",
     .css_id = "BACKSPACE",
     .label = "⬅",
     .letter = "",
     .left = "DEL",
     .right = "CLEAR",
     .below = NULL,
     .hpkey = HP50g_KEY_BACKSPACE                                                                                                               },

    {.css_class = "function",
     .css_id = "POW",
     .label = "Y<sup>x</sup>",
     .letter = "Q",
     .left = "𝑒<sup>x</sup>",
     .right = "LN",
     .below = NULL,
     .hpkey = HP50g_KEY_Q                                                                                                                       },
    {.css_class = "function",
     .css_id = "SQRT",
     .label = "√𝓍",
     .letter = "R",
     .left = "𝓍<sup>2</sup>",
     .right = "<sup>x</sup>√𝓎",
     .below = NULL,
     .hpkey = HP50g_KEY_R                                                                                                                       },
    {.css_class = "function",
     .css_id = "SIN",
     .label = "SIN",
     .letter = "S",
     .left = "ASIN",
     .right = "∑",
     .below = NULL,
     .hpkey = HP50g_KEY_S                                                                                                                       },
    {.css_class = "function",
     .css_id = "COS",
     .label = "COS",
     .letter = "T",
     .left = "ACOS",
     .right = "∂",
     .below = NULL,
     .hpkey = HP50g_KEY_T                                                                                                                       },
    {.css_class = "function",
     .css_id = "TAN",
     .label = "TAN",
     .letter = "U",
     .left = "ATAN",
     .right = "∫",
     .below = NULL,
     .hpkey = HP50g_KEY_U                                                                                                                       },

    {.css_class = "enter",
     .css_id = "ENTER",
     .label = "ENTER",
     .letter = NULL,
     .left = "ANS",
     .right = "→NUM",
     .below = NULL,
     .hpkey = HP50g_KEY_ENTER                                                                                                                   },
    {.css_class = "function",
     .css_id = "EEX",
     .label = "EEX",
     .letter = "V",
     .left = "10<sup>𝓍</sup>",
     .right = "LOG",
     .below = NULL,
     .hpkey = HP50g_KEY_V                                                                                                                       },
    {.css_class = "function",
     .css_id = "NEG",
     .label = "+/-",
     .letter = "W",
     .left = "≠",
     .right = "=",
     .below = NULL,
     .hpkey = HP50g_KEY_W                                                                                                                       },
    {.css_class = "function",
     .css_id = "X",
     .label = "𝓍",
     .letter = "X",
     .left = "≤",
     .right = "&gt;",
     .below = NULL,
     .hpkey = HP50g_KEY_X                                                                                                                       },
    {.css_class = "function",
     .css_id = "INV",
     .label = "1/𝓍",
     .letter = "Y",
     .left = "≥",
     .right = ">",
     .below = NULL,
     .hpkey = HP50g_KEY_Y                                                                                                                       },

    {.css_class = "alpha",
     .css_id = "ALPHA",
     .label = "ALPHA",
     .letter = NULL,
     .left = "USER",
     .right = "ENTRY",
     .below = NULL,
     .hpkey = HP50g_KEY_ALPHA                                                                                                                   },
    {.css_class = "core-number",
     .css_id = "SEVEN",
     .label = "7",
     .letter = NULL,
     .left = "S.SLV",
     .right = "NUM.SLV",
     .below = NULL,
     .hpkey = HP50g_KEY_7                                                                                                                       },
    {.css_class = "core-number",
     .css_id = "EIGHT",
     .label = "8",
     .letter = NULL,
     .left = "EXP&amp;LN",
     .right = "TRIG",
     .below = NULL,
     .hpkey = HP50g_KEY_8                                                                                                                       },
    {.css_class = "core-number",
     .css_id = "NINE",
     .label = "9",
     .letter = NULL,
     .left = "FINANCE",
     .right = "TIME",
     .below = NULL,
     .hpkey = HP50g_KEY_9                                                                                                                       },
    {.css_class = "core-number",
     .css_id = "DIVIDE",
     .label = "÷",
     .letter = "Z",
     .left = "ABS",
     .right = "ARG",
     .below = NULL,
     .hpkey = HP50g_KEY_Z                                                                                                                       },

    {.css_class = "shift-left",
     .css_id = "SHIFT-LEFT",
     .label = "⮢",
     .letter = NULL,
     .left = "",
     .right = NULL,
     .below = NULL,
     .hpkey = HP50g_KEY_SHIFT_LEFT                                                                                                              },
    {.css_class = "core-number",
     .css_id = "FOUR",
     .label = "4",
     .letter = NULL,
     .left = "CALC",
     .right = "ALG",
     .below = NULL,
     .hpkey = HP50g_KEY_4                                                                                                                       },
    {.css_class = "core-number",
     .css_id = "FIVE",
     .label = "5",
     .letter = NULL,
     .left = "MATRICES",
     .right = "STAT",
     .below = NULL,
     .hpkey = HP50g_KEY_5                                                                                                                       },
    {.css_class = "core-number",
     .css_id = "SIX",
     .label = "6",
     .letter = NULL,
     .left = "CONVERT",
     .right = "UNITS",
     .below = NULL,
     .hpkey = HP50g_KEY_6                                                                                                                       },
    {.css_class = "core-number",
     .css_id = "MULTIPLY",
     .label = "×",
     .letter = NULL,
     .left = "[ ]",
     .right = "\" \"",
     .below = NULL,
     .hpkey = HP50g_KEY_MULTIPLY                                                                                                                },

    {.css_class = "shift-right",
     .css_id = "SHIFT-RIGHT",
     .label = "⮣",
     .letter = NULL,
     .left = "",
     .right = NULL,
     .below = NULL,
     .hpkey = HP50g_KEY_SHIFT_RIGHT                                                                                                             },
    {.css_class = "core-number",
     .css_id = "ONE",
     .label = "1",
     .letter = NULL,
     .left = "ARITH",
     .right = "CMPLX",
     .below = NULL,
     .hpkey = HP50g_KEY_1                                                                                                                       },
    {.css_class = "core-number",
     .css_id = "TWO",
     .label = "2",
     .letter = NULL,
     .left = "DEF",
     .right = "LIB",
     .below = NULL,
     .hpkey = HP50g_KEY_2                                                                                                                       },
    {.css_class = "core-number",
     .css_id = "THREE",
     .label = "3",
     .letter = NULL,
     .left = "#",
     .right = "BASE",
     .below = NULL,
     .hpkey = HP50g_KEY_3                                                                                                                       },
    {.css_class = "core-number",
     .css_id = "MINUS",
     .label = "-",
     .letter = NULL,
     .left = "( )",
     .right = "_",
     .below = NULL,
     .hpkey = HP50g_KEY_MINUS                                                                                                                   },

    {.css_class = "core",
     .css_id = "ON",
     .label = "ON",
     .letter = NULL,
     .left = "CONT",
     .right = "OFF",
     .below = "CANCEL",
     .hpkey = HP50g_KEY_ON                                                                                                                      },
    {.css_class = "core-number",
     .css_id = "ZERO",
     .label = "0",
     .letter = NULL,
     .left = "∞",
     .right = "→",
     .below = NULL,
     .hpkey = HP50g_KEY_0                                                                                                                       },
    {.css_class = "core",
     .css_id = "PERIOD",
     .label = ".",
     .letter = NULL,
     .left = ": :",
     .right = "↲",
     .below = NULL,
     .hpkey = HP50g_KEY_PERIOD                                                                                                                  },
    {.css_class = "core",
     .css_id = "SPC",
     .label = "SPC",
     .letter = NULL,
     .left = "𝚷",
     .right = ",",
     .below = NULL,
     .hpkey = HP50g_KEY_SPACE                                                                                                                   },
    {.css_class = "core-number",
     .css_id = "PLUS",
     .label = "+",
     .letter = NULL,
     .left = "{ }",
     .right = "« »",
     .below = NULL,
     .hpkey = HP50g_KEY_PLUS                                                                                                                    },
};

int keys_order_normal[ NB_HP50g_KEYS ] = {
    HP50g_KEY_A,        HP50g_KEY_B,          HP50g_KEY_C,  HP50g_KEY_D,     HP50g_KEY_E,  HP50g_KEY_F,     HP50g_KEY_G,
    HP50g_KEY_H,        HP50g_KEY_I,          HP50g_KEY_UP, HP50g_KEY_J,     HP50g_KEY_K,  HP50g_KEY_L,     HP50g_KEY_LEFT,
    HP50g_KEY_DOWN,     HP50g_KEY_RIGHT,      HP50g_KEY_M,  HP50g_KEY_N,     HP50g_KEY_O,  HP50g_KEY_P,     HP50g_KEY_BACKSPACE,
    HP50g_KEY_Q,        HP50g_KEY_R,          HP50g_KEY_S,  HP50g_KEY_T,     HP50g_KEY_U,  HP50g_KEY_V,     HP50g_KEY_W,
    HP50g_KEY_X,        HP50g_KEY_Y,          HP50g_KEY_Z,  HP50g_KEY_ALPHA, HP50g_KEY_7,  HP50g_KEY_8,     HP50g_KEY_9,
    HP50g_KEY_MULTIPLY, HP50g_KEY_SHIFT_LEFT, HP50g_KEY_4,  HP50g_KEY_5,     HP50g_KEY_6,  HP50g_KEY_MINUS, HP50g_KEY_SHIFT_RIGHT,
    HP50g_KEY_1,        HP50g_KEY_2,          HP50g_KEY_3,  HP50g_KEY_PLUS,  HP50g_KEY_ON, HP50g_KEY_0,     HP50g_KEY_PERIOD,
    HP50g_KEY_SPACE,    HP50g_KEY_ENTER,
};

int keys_order_legacy[ NB_HP50g_KEYS ] = {
    HP50g_KEY_A,     HP50g_KEY_B,          HP50g_KEY_C,  HP50g_KEY_D,     HP50g_KEY_E,  HP50g_KEY_F,        HP50g_KEY_G,
    HP50g_KEY_H,     HP50g_KEY_I,          HP50g_KEY_UP, HP50g_KEY_J,     HP50g_KEY_K,  HP50g_KEY_L,        HP50g_KEY_LEFT,
    HP50g_KEY_DOWN,  HP50g_KEY_RIGHT,      HP50g_KEY_M,  HP50g_KEY_N,     HP50g_KEY_O,  HP50g_KEY_P,        HP50g_KEY_BACKSPACE,
    HP50g_KEY_Q,     HP50g_KEY_R,          HP50g_KEY_S,  HP50g_KEY_T,     HP50g_KEY_U,  HP50g_KEY_ENTER,    HP50g_KEY_V,
    HP50g_KEY_W,     HP50g_KEY_X,          HP50g_KEY_Y,  HP50g_KEY_ALPHA, HP50g_KEY_7,  HP50g_KEY_8,        HP50g_KEY_9,
    HP50g_KEY_Z,     HP50g_KEY_SHIFT_LEFT, HP50g_KEY_4,  HP50g_KEY_5,     HP50g_KEY_6,  HP50g_KEY_MULTIPLY, HP50g_KEY_SHIFT_RIGHT,
    HP50g_KEY_1,     HP50g_KEY_2,          HP50g_KEY_3,  HP50g_KEY_MINUS, HP50g_KEY_ON, HP50g_KEY_0,        HP50g_KEY_PERIOD,
    HP50g_KEY_SPACE, HP50g_KEY_PLUS,
};

x50ng_ui_annunciator_t ui_annunciators[ NB_ANNUNCIATORS ] = { { .icon = "⮢" },  { .icon = "⮣" },  { .icon = "α" },
                                                              { .icon = "🪫" }, { .icon = "⌛" }, { .icon = "⇄" } };

/*************/
/* functions */
/*************/
void newrplify_ui_keys()
{
    // modify keys' labeling for newRPL
    for ( int i = HP50g_KEY_A; i <= HP50g_KEY_F; i++ )
        ui_keys[ i ].left = "";

    for ( int i = HP50g_KEY_G; i <= HP50g_KEY_I; i++ ) {
        ui_keys[ i ].label = "";
        ui_keys[ i ].left = "";
        ui_keys[ i ].right = NULL;
    }

    for ( int i = HP50g_KEY_J; i <= HP50g_KEY_K; i++ ) {
        ui_keys[ i ].label = "";
        ui_keys[ i ].left = "";
        ui_keys[ i ].right = NULL;
    }

    ui_keys[ HP50g_KEY_UP ].left = "UPDIR";

    ui_keys[ HP50g_KEY_LEFT ].left = "BEG";
    ui_keys[ HP50g_KEY_LEFT ].right = "COPY";

    ui_keys[ HP50g_KEY_DOWN ].left = "CUT";

    ui_keys[ HP50g_KEY_RIGHT ].left = "END";
    ui_keys[ HP50g_KEY_RIGHT ].right = "PASTE";

    ui_keys[ HP50g_KEY_M ].label = "STO⏵";
    ui_keys[ HP50g_KEY_M ].left = "RCL";
    ui_keys[ HP50g_KEY_M ].right = "PREV.M";

    for ( int i = HP50g_KEY_N; i <= HP50g_KEY_O; i++ ) {
        ui_keys[ i ].left = "";
        ui_keys[ i ].right = NULL;
    }

    ui_keys[ HP50g_KEY_P ].label = "MENU";

    ui_keys[ HP50g_KEY_BACKSPACE ].left = "";

    for ( int i = HP50g_KEY_S; i <= HP50g_KEY_U; i++ )
        ui_keys[ i ].right = NULL;

    for ( int i = HP50g_KEY_ALPHA; i <= HP50g_KEY_9; i++ )
        ui_keys[ i ].left = "";

    ui_keys[ HP50g_KEY_8 ].right = NULL;

    for ( int i = HP50g_KEY_4; i <= HP50g_KEY_6; i++ ) {
        ui_keys[ i ].left = "";
        ui_keys[ i ].right = NULL;
    }

    ui_keys[ HP50g_KEY_2 ].left = "";

    ui_keys[ HP50g_KEY_ON ].left = "";
    ui_keys[ HP50g_KEY_ON ].below = NULL;

    ui_keys[ HP50g_KEY_ENTER ].left = "";
}

/********************/
/* Public functions */
/********************/
void ui_handle_pending_inputs( void* data )
{
    x50ng_t* x50ng = data;

    switch ( opt.frontend ) {
        case FRONTEND_NCURSES:
            ncurses_handle_pending_inputs( x50ng );
            break;
        case FRONTEND_GTK:
        default:
            gui_handle_pending_inputs( x50ng );
            break;
    }

    x50ng_mod_timer( x50ng->timer_ui_input, x50ng_get_clock() + UI_EVENTS_REFRESH_INTERVAL );
}

void ui_refresh_output( void* data )
{
    x50ng_t* x50ng = data;

    switch ( opt.frontend ) {
        case FRONTEND_NCURSES:
            ncurses_refresh_lcd( x50ng );
            break;
        case FRONTEND_GTK:
        default:
            gui_refresh_lcd( x50ng );
            break;
    }

    x50ng_mod_timer( x50ng->timer_ui_output, x50ng_get_clock() + UI_LCD_REFRESH_INTERVAL );
}

void ui_init( x50ng_t* x50ng )
{
    if ( opt.newrpl_keyboard )
        newrplify_ui_keys();

    switch ( opt.frontend ) {
        case FRONTEND_NCURSES:
            ncurses_init( x50ng );
            break;
        case FRONTEND_GTK:
        default:
            gui_init( x50ng );
            break;
    }
}

void ui_exit( void )
{
    switch ( opt.frontend ) {
        case FRONTEND_NCURSES:
            ncurses_exit();
            break;
        case FRONTEND_GTK:
        default:
            gui_exit();
            break;
    }
}
