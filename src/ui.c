#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <sys/times.h>

#include "options.h"
#include "x50ng.h"
#include "s3c2410.h"
#include "ui.h"
#include "gui.h"
#include "tui.h"

#include "gdbstub.h"
#include "types.h"

x50ng_ui_key_t ui_keys[ NB_KEYS ] = {
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

int keys_order_normal[ NB_KEYS ] = {
    HPKEY_A,          HPKEY_B,  HPKEY_C,         HPKEY_D,      HPKEY_E,     HPKEY_F,           HPKEY_G,     HPKEY_H, HPKEY_I,
    HPKEY_UP,         HPKEY_J,  HPKEY_K,         HPKEY_L,      HPKEY_LEFT,  HPKEY_DOWN,        HPKEY_RIGHT, HPKEY_M, HPKEY_N,
    HPKEY_O,          HPKEY_P,  HPKEY_BACKSPACE, HPKEY_Q,      HPKEY_R,     HPKEY_S,           HPKEY_T,     HPKEY_U, HPKEY_V,
    HPKEY_W,          HPKEY_X,  HPKEY_Y,         HPKEY_Z,      HPKEY_ALPHA, HPKEY_7,           HPKEY_8,     HPKEY_9, HPKEY_MULTIPLY,
    HPKEY_SHIFT_LEFT, HPKEY_4,  HPKEY_5,         HPKEY_6,      HPKEY_MINUS, HPKEY_SHIFT_RIGHT, HPKEY_1,     HPKEY_2, HPKEY_3,
    HPKEY_PLUS,       HPKEY_ON, HPKEY_0,         HPKEY_PERIOD, HPKEY_SPACE, HPKEY_ENTER,
};

int keys_order_legacy[ NB_KEYS ] = {
    HPKEY_A,          HPKEY_B,  HPKEY_C,         HPKEY_D,      HPKEY_E,        HPKEY_F,           HPKEY_G,     HPKEY_H, HPKEY_I,
    HPKEY_UP,         HPKEY_J,  HPKEY_K,         HPKEY_L,      HPKEY_LEFT,     HPKEY_DOWN,        HPKEY_RIGHT, HPKEY_M, HPKEY_N,
    HPKEY_O,          HPKEY_P,  HPKEY_BACKSPACE, HPKEY_Q,      HPKEY_R,        HPKEY_S,           HPKEY_T,     HPKEY_U, HPKEY_ENTER,
    HPKEY_V,          HPKEY_W,  HPKEY_X,         HPKEY_Y,      HPKEY_ALPHA,    HPKEY_7,           HPKEY_8,     HPKEY_9, HPKEY_Z,
    HPKEY_SHIFT_LEFT, HPKEY_4,  HPKEY_5,         HPKEY_6,      HPKEY_MULTIPLY, HPKEY_SHIFT_RIGHT, HPKEY_1,     HPKEY_2, HPKEY_3,
    HPKEY_MINUS,      HPKEY_ON, HPKEY_0,         HPKEY_PERIOD, HPKEY_SPACE,    HPKEY_PLUS,
};

/*************/
/* functions */
/*************/
void newrplify_ui_keys()
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

void x50ng_set_key_state( x50ng_t* x50ng, const x50ng_ui_key_t* key, bool state )
{
    if ( key->rowbit )
        s3c2410_io_port_g_update( x50ng, key->column, key->row, key->columnbit, key->rowbit, state );
    else
        s3c2410_io_port_f_set_bit( x50ng, key->eint, state );
}

/********************/
/* Public functions */
/********************/
void ui_handle_pending_inputs( void* data )
{
    x50ng_t* x50ng = data;

    if ( opt.tui )
        tui_handle_pending_inputs( x50ng );
    else
        gui_handle_pending_inputs( x50ng );

    x50ng_mod_timer( x50ng->timer_ui_input, x50ng_get_clock() + UI_EVENTS_REFRESH_INTERVAL );
}

void ui_refresh_output( void* data )
{
    x50ng_t* x50ng = data;

    if ( opt.tui )
        tui_refresh_lcd( x50ng );
    else
        gui_refresh_lcd( x50ng );

    x50ng_mod_timer( x50ng->timer_ui_output, x50ng_get_clock() + UI_LCD_REFRESH_INTERVAL );
}

void ui_init( x50ng_t* x50ng )
{
    if ( opt.newrpl_keyboard )
        newrplify_ui_keys();

    if ( opt.tui )
        tui_init( x50ng );
    else
        gui_init( x50ng );
}

void ui_exit( void )
{
    if ( opt.tui )
        tui_exit();
    else
        gui_exit();
}
