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

#define FONT_SIZE_KEY opt.font_size
#define FONT_SIZE_SYMBOL ( 2 * opt.font_size )
#define FONT_SIZE_NUMBER ( ( int )( 1.75 * opt.font_size ) )
#define FONT_SIZE_TINY ( ( int )( 0.75 * opt.font_size ) )

#define TINY_TEXT_HEIGHT ( FONT_SIZE_TINY + 2 )
#define TINY_TEXT_WIDTH ( TINY_TEXT_HEIGHT / 2 )

#define KB_NB_ROWS 10
#define KB_NB_COLS_MENU 6
#define KB_NB_COLS 5

#define KB_WIDTH_6_KEYS ( 3 * opt.font_size )
#define KB_WIDTH_5_KEYS ( 4 * opt.font_size )

#define KB_HEIGHT_MENU_KEYS ( ( int )( 1.75 * opt.font_size ) )
#define KB_HEIGHT_SMALL_KEYS ( ( int )( 2 * opt.font_size ) )
#define KB_HEIGHT_BIG_KEYS ( ( int )( 2.5 * opt.font_size ) )

#define KB_SPACING_KEYS ( ( int )( 0.5 * TINY_TEXT_WIDTH ) )
#define KB_COLUMN_WIDTH_6_KEYS ( KB_WIDTH_6_KEYS + KB_SPACING_KEYS )
#define KB_COLUMN_WIDTH_5_KEYS ( KB_WIDTH_5_KEYS + KB_SPACING_KEYS )

#define ANNUNCIATOR_WIDTH 16
#define ANNUNCIATOR_HEIGHT 16
#define ANNUNCIATORS_HEIGHT ANNUNCIATOR_HEIGHT

#define LCD_PIXEL_SCALE opt.display_scale
#define LCD_WIDTH ( 131 * LCD_PIXEL_SCALE )
#define LCD_HEIGHT ( 80 * LCD_PIXEL_SCALE )

#define WINDOW_WIDTH ( 384 )

static x49gp_ui_key_t ui_keys[ NB_KEYS ] = {
    {.css_class = "menu",
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
     .label = "⮕",
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

    {.css_class = "function",
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
    {.css_class = "core-number",
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

    {.css_class = "alpha",
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

    {.css_class = "shift-left",
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

    {.css_class = "shift-right",
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

    {.css_class = "core",
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
    {.css_class = "core",
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
};

char* css_global = "window {"
                   "  background-color: %s;"
                   "}"
                   "window * {"
                   "  font-weight: bold;"
                   "}"
                   /* "box { border: 1px dashed red; }" */
                   /* "label { border: 1px dotted yellow; }" */
                   ".annunciator {"
                   "  padding: 0px;"
                   "  color: #080808;"
                   "}"
                   ".lcd-container, .annunciators-container {"
                   "  background-color: #a9d0b2;"
                   "}"
                   "button {"
                   "  background-image: none;"
                   "  padding: 0px;"
                   "  margin-left: 15px;"
                   "  margin-right: 15px;"
                   "}"
                   "button.key-down {"
                   "  border-color: #080808;"
                   "}"
                   "button.menu {"
                   "  background-color: #a9a9a9;"
                   "}"
                   "button.function {"
                   "  background-color: #696969;"
                   "}"
                   "button.arrow {"
                   "  background-color: #e0e0e0;"
                   "  border-radius: 33%;"
                   "}"
                   "button.alpha {"
                   "  background-color: #fae82c;"
                   "}"
                   "button.core, button.core-number {"
                   "  background-color: #080808;"
                   "}"
                   "button.alpha .label-key, button.arrow .label-key, button.menu .label-key {"
                   "  color: #080808;"
                   "}"
                   "button.shift-left {"
                   "  background-color: %s;"
                   "}"
                   "button.shift-right {"
                   "  background-color: #8e2518;"
                   "}"
                   "button.shift-left .label-key, button.shift-right .label-key {"
                   "  font-size: %ipx;"
                   "  color: #080808;"
                   "}"
                   "button.core-number .label-key, button.arrow .label-key {"
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

static void x49gpng_press_key( x49gp_t* x49gp, const x49gp_ui_key_t* key )
{
    if ( key->rowbit )
        s3c2410_io_port_g_update( x49gp, key->column, key->row, key->columnbit, key->rowbit, 1 );
    else
        s3c2410_io_port_f_set_bit( x49gp, key->eint, 1 );
}

static void x49gpng_release_key( x49gp_t* x49gp, const x49gp_ui_key_t* key )
{
    if ( key->rowbit )
        s3c2410_io_port_g_update( x49gp, key->column, key->row, key->columnbit, key->rowbit, 0 );
    else
        s3c2410_io_port_f_set_bit( x49gp, key->eint, 0 );
}

static inline int _tiny_text_width( const char* text )
{
    char* stripped_text;
    pango_parse_markup( text, -1, 0, NULL, &stripped_text, NULL, NULL );

    return strlen( stripped_text ) * TINY_TEXT_WIDTH;
}

static bool ui_press_button( x49gp_ui_button_t* button, x49gp_ui_button_t* cause, bool hold )
{
    x49gp_t* x49gp = button->x49gp;
    const x49gp_ui_key_t* key = button->key;
    x49gp_ui_t* ui = x49gp->ui;

    if ( button->down )
        return false;

    button->down = true;
    button->hold = hold;

    if ( !button->hold )
        ui->buttons_down++;

#if GTK_MAJOR_VERSION == 4
    gtk_widget_add_css_class( button->button, "key-down" );
#else
    gtk_style_context_add_class( gtk_widget_get_style_context( button->button ), "key-down" );
#endif

    x49gpng_release_key( x49gp, key );

    return true;
}

static void ui_release_button( x49gp_ui_button_t* button, x49gp_ui_button_t* cause )
{
    x49gp_t* x49gp = button->x49gp;
    const x49gp_ui_key_t* key = button->key;

    button->down = false;
    button->hold = false;

#if GTK_MAJOR_VERSION == 4
    gtk_widget_remove_css_class( button->button, "key-down" );
#else
    gtk_style_context_remove_class( gtk_widget_get_style_context( button->button ), "key-down" );
#endif

    x49gpng_release_key( x49gp, key );
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

    if ( event->type != GDK_BUTTON_PRESS || event->button > 3 )
        return false;

    if ( !ui_press_button( button, button, event->button == 3 ) )
        return false;

    x49gpng_press_key( x49gp, key );

    return false;
}

static gboolean react_to_button_release( GtkWidget* widget, GdkEventButton* event, gpointer user_data )
{
    x49gp_ui_button_t* button = user_data;
    x49gp_t* x49gp = button->x49gp;
    x49gp_ui_t* ui = x49gp->ui;

    if ( event->type != GDK_BUTTON_RELEASE )
        return false;

    if ( event->button != 1 )
        return true;

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

    /* We want to know the keyval as interpreted without modifiers. */
    /* However, there is one modifier we do care about: NumLock, */
    /* which normally is represented by MOD2. */
    guint keyval;
    if ( !gdk_keymap_translate_keyboard_state( gdk_keymap_get_for_display( gdk_display_get_default() ), event->hardware_keycode,
                                               event->state & GDK_MOD2_MASK, event->group, &keyval, NULL, NULL, NULL ) )
        return false;

    int index;
    switch ( keyval ) {
        case GDK_KEY_a:
        case GDK_KEY_F1:
            index = HPKEY_A;
            break;
        case GDK_KEY_b:
        case GDK_KEY_F2:
            index = HPKEY_B;
            break;
        case GDK_KEY_c:
        case GDK_KEY_F3:
            index = HPKEY_C;
            break;
        case GDK_KEY_d:
        case GDK_KEY_F4:
            index = HPKEY_D;
            break;
        case GDK_KEY_e:
        case GDK_KEY_F5:
            index = HPKEY_E;
            break;
        case GDK_KEY_f:
        case GDK_KEY_F6:
            index = HPKEY_F;
            break;
        case GDK_KEY_g:
            index = HPKEY_G;
            break;
        case GDK_KEY_h:
            index = HPKEY_H;
            break;
        case GDK_KEY_i:
            index = HPKEY_I;
            break;
        case GDK_KEY_j:
            index = HPKEY_J;
            break;
        case GDK_KEY_k:
            index = HPKEY_K;
            break;
        case GDK_KEY_l:
            index = HPKEY_L;
            break;
        case GDK_KEY_Up:
        case GDK_KEY_KP_Up:
            index = HPKEY_UP;
            break;
        case GDK_KEY_Left:
        case GDK_KEY_KP_Left:
            index = HPKEY_LEFT;
            break;
        case GDK_KEY_Down:
        case GDK_KEY_KP_Down:
            index = HPKEY_DOWN;
            break;
        case GDK_KEY_Right:
        case GDK_KEY_KP_Right:
            index = HPKEY_RIGHT;
            break;
        case GDK_KEY_m:
            index = HPKEY_M;
            break;
        case GDK_KEY_n:
            index = HPKEY_N;
            break;
        case GDK_KEY_o:
        case GDK_KEY_apostrophe:
            index = HPKEY_O;
            break;
        case GDK_KEY_p:
            index = HPKEY_P;
            break;
        case GDK_KEY_BackSpace:
        case GDK_KEY_Delete:
        case GDK_KEY_KP_Delete:
            index = HPKEY_BACKSPACE;
            break;
        case GDK_KEY_dead_circumflex:
        case GDK_KEY_asciicircum:
        case GDK_KEY_q:
        case GDK_KEY_caret:
            index = HPKEY_Q;
            break;
        case GDK_KEY_r:
            index = HPKEY_R;
            break;
        case GDK_KEY_s:
            index = HPKEY_S;
            break;
        case GDK_KEY_t:
            index = HPKEY_T;
            break;
        case GDK_KEY_u:
            index = HPKEY_U;
            break;
        case GDK_KEY_v:
            index = HPKEY_V;
            break;
        case GDK_KEY_w:
            index = HPKEY_W;
            break;
        case GDK_KEY_x:
            index = HPKEY_X;
            break;
        case GDK_KEY_y:
            index = HPKEY_Y;
            break;
        case GDK_KEY_z:
        case GDK_KEY_slash:
        case GDK_KEY_KP_Divide:
            index = HPKEY_Z;
            break;
        case GDK_KEY_Tab:
#ifndef __APPLE__
        case GDK_KEY_Alt_L:
        case GDK_KEY_Alt_R:
        case GDK_KEY_Meta_L:
        case GDK_KEY_Meta_R:
        case GDK_KEY_Mode_switch:
#endif
            index = HPKEY_ALPHA;
            break;
        case GDK_KEY_7:
        case GDK_KEY_KP_7:
            index = HPKEY_7;
            break;
        case GDK_KEY_8:
        case GDK_KEY_KP_8:
            index = HPKEY_8;
            break;
        case GDK_KEY_9:
        case GDK_KEY_KP_9:
            index = HPKEY_9;
            break;
        case GDK_KEY_multiply:
        case GDK_KEY_KP_Multiply:
            index = HPKEY_MULTIPLY;
            break;
        case GDK_KEY_Shift_L:
        case GDK_KEY_Shift_R:
            index = HPKEY_SHIFT_LEFT;
            break;
        case GDK_KEY_4:
        case GDK_KEY_KP_4:
            index = HPKEY_4;
            break;
        case GDK_KEY_5:
        case GDK_KEY_KP_5:
            index = HPKEY_5;
            break;
        case GDK_KEY_6:
        case GDK_KEY_KP_6:
            index = HPKEY_6;
            break;
        case GDK_KEY_minus:
        case GDK_KEY_KP_Subtract:
            index = HPKEY_MINUS;
            break;
        case GDK_KEY_Control_L:
        case GDK_KEY_Control_R:
            index = HPKEY_SHIFT_RIGHT;
            break;
        case GDK_KEY_1:
        case GDK_KEY_KP_1:
            index = HPKEY_1;
            break;
        case GDK_KEY_2:
        case GDK_KEY_KP_2:
            index = HPKEY_2;
            break;
        case GDK_KEY_3:
        case GDK_KEY_KP_3:
            index = HPKEY_3;
            break;
        case GDK_KEY_plus:
        case GDK_KEY_KP_Add:
            index = HPKEY_PLUS;
            break;
        case GDK_KEY_Escape:
            index = HPKEY_ON;
            break;
        case GDK_KEY_0:
        case GDK_KEY_KP_0:
            index = HPKEY_0;
            break;
        case GDK_KEY_period:
        case GDK_KEY_comma:
        case GDK_KEY_KP_Decimal:
        case GDK_KEY_KP_Separator:
            index = HPKEY_PERIOD;
            break;
        case GDK_KEY_space:
        case GDK_KEY_KP_Space:
            index = HPKEY_SPACE;
            break;
        case GDK_KEY_Return:
        case GDK_KEY_KP_Enter:
            index = HPKEY_ENTER;
            break;

        /* QWERTY compat: US English, UK English, International English */
        case GDK_KEY_backslash:
            index = HPKEY_MULTIPLY;
            break;
        case GDK_KEY_equal:
            index = HPKEY_PLUS;
            break;

        /* QWERTZ compat: German */
        case GDK_KEY_ssharp:
            index = HPKEY_Z;
            break;
        case GDK_KEY_numbersign:
            index = HPKEY_MULTIPLY;
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

    x49gp_ui_button_t* button = &ui->buttons[ index ];

    GdkEventButton bev;
    memset( &bev, 0, sizeof( GdkEventButton ) );
    bev.time = event->time;
    bev.button = 1;
    bev.state = event->state;

    switch ( event->type ) {
        case GDK_KEY_PRESS:
            bev.type = GDK_BUTTON_PRESS;
            react_to_button_press( button->button, &bev, button );
            break;
        case GDK_KEY_RELEASE:
            bev.type = GDK_BUTTON_RELEASE;
            react_to_button_release( button->button, &bev, button );
            break;
        default:
            return false;
    }

    return true;
}

static gboolean react_to_display_click( GtkWidget* widget, GdkEventButton* event, gpointer user_data )
{
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
            gdk_window_begin_move_drag( gtk_widget_get_window( ui->window ), event->button, event->x_root, event->y_root, event->time );
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
        ui_keys[ i ].left = "";

    for ( int i = 6; i < 9; i++ ) {
        ui_keys[ i ].label = "";
        ui_keys[ i ].left = "";
        ui_keys[ i ].right = NULL;
    }

    for ( int i = 10; i < 13; i++ ) {
        ui_keys[ i ].label = "";
        ui_keys[ i ].left = "";
        ui_keys[ i ].right = NULL;
    }

    ui_keys[ 9 ].left = "UPDIR";

    ui_keys[ 13 ].left = "BEG";
    ui_keys[ 13 ].right = "COPY";

    ui_keys[ 14 ].left = "CUT";

    ui_keys[ 15 ].left = "END";
    ui_keys[ 15 ].right = "PASTE";

    ui_keys[ 16 ].label = "STO⏵";
    ui_keys[ 16 ].left = "RCL";
    ui_keys[ 16 ].right = "PREV.M";

    for ( int i = 17; i < 20; i++ ) {
        ui_keys[ i ].left = "";
        ui_keys[ i ].right = NULL;
    }

    ui_keys[ 19 ].label = "MENU";

    ui_keys[ 20 ].left = "";

    for ( int i = 23; i < 26; i++ )
        ui_keys[ i ].right = NULL;

    for ( int i = 31; i < 35; i++ )
        ui_keys[ i ].left = "";

    ui_keys[ 33 ].right = NULL;

    for ( int i = 37; i < 39; i++ ) {
        ui_keys[ i ].left = "";
        ui_keys[ i ].right = NULL;
    }

    ui_keys[ 43 ].left = "";

    ui_keys[ 46 ].left = "";
    ui_keys[ 46 ].below = NULL;

    ui_keys[ 50 ].left = "";
}

static GtkWidget* _ui_load__create_annunciator_widget( x49gp_ui_t* ui, const char* label )
{
    GtkWidget* ui_ann = gtk_label_new( NULL );
#if GTK_MAJOR_VERSION == 4
    gtk_widget_add_css_class( ui_ann, "annunciator" );
#else
    gtk_style_context_add_class( gtk_widget_get_style_context( ui_ann ), "annunciator" );
#endif
    gtk_label_set_use_markup( GTK_LABEL( ui_ann ), true );
    gtk_label_set_markup( GTK_LABEL( ui_ann ), label );
    gtk_widget_set_size_request( ui_ann, ANNUNCIATOR_WIDTH, ANNUNCIATOR_HEIGHT );

    return ui_ann;
}

static GtkWidget* _ui_load__create_label( const char* css_class, const char* text )
{
    GtkWidget* ui_label = gtk_label_new( NULL );
#if GTK_MAJOR_VERSION == 4
    gtk_widget_add_css_class( ui_label, css_class );
#else
    gtk_style_context_add_class( gtk_widget_get_style_context( ui_label ), css_class );
#endif
    gtk_label_set_use_markup( GTK_LABEL( ui_label ), true );
    gtk_label_set_markup( GTK_LABEL( ui_label ), text );

    return ui_label;
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
        case MODEL_49GP:
            ui->calculator = UI_CALCULATOR_HP49GP;
            ui->name = opt.name != NULL ? opt.name : "HP 49g+";
            break;
        case MODEL_50G:
        default:
            ui->calculator = UI_CALCULATOR_HP50G;
            ui->name = opt.name != NULL ? opt.name : "HP 50g";
            break;
    }

    // create window and widgets/stuff
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

    GtkWidget* window_container = gtk_box_new( GTK_ORIENTATION_VERTICAL, 0 );
    gtk_container_add( GTK_CONTAINER( ui->window ), window_container );

    g_signal_connect( G_OBJECT( ui->window ), "focus-out-event", G_CALLBACK( react_to_focus_lost ), x49gp );
    g_signal_connect( G_OBJECT( ui->window ), "key-press-event", G_CALLBACK( react_to_key_event ), x49gp );
    g_signal_connect( G_OBJECT( ui->window ), "key-release-event", G_CALLBACK( react_to_key_event ), x49gp );
    /* g_signal_connect( G_OBJECT( ui->window ), "button-press-event", G_CALLBACK( react_to_display_click ), x49gp ); */
    g_signal_connect_swapped( G_OBJECT( ui->window ), "delete-event", G_CALLBACK( do_quit ), x49gp );
    g_signal_connect_swapped( G_OBJECT( ui->window ), "destroy", G_CALLBACK( do_quit ), x49gp );
    gtk_widget_add_events( ui->window, GDK_FOCUS_CHANGE_MASK | GDK_BUTTON_PRESS_MASK | GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK );

    ui->lcd_canvas = gtk_drawing_area_new();
#if GTK_MAJOR_VERSION == 4
    gtk_widget_add_css_class( ui->lcd_canvas, "lcd" );
#else
    gtk_style_context_add_class( gtk_widget_get_style_context( ui->lcd_canvas ), "lcd" );
#endif
    gtk_widget_set_size_request( ui->lcd_canvas, LCD_WIDTH, LCD_HEIGHT );
    g_signal_connect( G_OBJECT( ui->lcd_canvas ), "draw", G_CALLBACK( redraw_lcd ), x49gp );
    g_signal_connect( G_OBJECT( ui->lcd_canvas ), "configure-event", G_CALLBACK( draw_lcd ), x49gp );

    GtkWidget* lcd_container = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 0 );
#if GTK_MAJOR_VERSION == 4
    gtk_widget_add_css_class( lcd_container, "lcd-container" );
#else
    gtk_style_context_add_class( gtk_widget_get_style_context( lcd_container ), "lcd-container" );
#endif
    gtk_widget_set_size_request( lcd_container, LCD_WIDTH, LCD_HEIGHT + 3 );
    gtk_widget_set_margin_bottom( lcd_container, 3 );
    gtk_box_set_center_widget( GTK_BOX( lcd_container ), ui->lcd_canvas );

    GtkWidget* annunciators_container = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, ( LCD_WIDTH - ( 6 * ANNUNCIATOR_WIDTH ) ) / 5 );
#if GTK_MAJOR_VERSION == 4
    gtk_widget_add_css_class( annunciators_container, "annunciators-container" );
#else
    gtk_style_context_add_class( gtk_widget_get_style_context( annunciators_container ), "annunciators-container" );
#endif
    gtk_container_add( GTK_CONTAINER( annunciators_container ), ui->ui_ann_left );
    gtk_container_add( GTK_CONTAINER( annunciators_container ), ui->ui_ann_right );
    gtk_container_add( GTK_CONTAINER( annunciators_container ), ui->ui_ann_alpha );
    gtk_container_add( GTK_CONTAINER( annunciators_container ), ui->ui_ann_battery );
    gtk_container_add( GTK_CONTAINER( annunciators_container ), ui->ui_ann_busy );
    gtk_container_add( GTK_CONTAINER( annunciators_container ), ui->ui_ann_io );

    GtkWidget* display_container = gtk_box_new( GTK_ORIENTATION_VERTICAL, 0 );
#if GTK_MAJOR_VERSION == 4
    gtk_widget_add_css_class( annunciators_container, "display-container" );
#else
    gtk_style_context_add_class( gtk_widget_get_style_context( annunciators_container ), "display-container" );
#endif
    gtk_container_add( GTK_CONTAINER( display_container ), annunciators_container );
    gtk_container_add( GTK_CONTAINER( display_container ), lcd_container );

    GtkWidget* display_container_event_box = gtk_event_box_new();
    g_signal_connect( G_OBJECT( display_container_event_box ), "button-press-event", G_CALLBACK( react_to_display_click ), x49gp );
    gtk_container_add( GTK_CONTAINER( display_container_event_box ), display_container );

    gtk_container_add( GTK_CONTAINER( window_container ), display_container_event_box );

    // keyboard
    GtkWidget* keyboard_container = gtk_box_new( GTK_ORIENTATION_VERTICAL, 0 );
#if GTK_MAJOR_VERSION == 4
    gtk_widget_add_css_class( keyboard_container, "keyboard-container" );
#else
    gtk_style_context_add_class( gtk_widget_get_style_context( keyboard_container ), "keyboard-container" );
#endif
    gtk_box_set_homogeneous( GTK_BOX( keyboard_container ), true );

    gtk_container_add( GTK_CONTAINER( window_container ), keyboard_container );

    x49gp_ui_button_t* button;

    if ( opt.newrpl )
        _ui_load__newrplify_ui_keys();

    GtkWidget* rows_containers[ KB_NB_ROWS ];
    GtkWidget* keys_containers[ NB_KEYS ];
    GtkWidget* keys_top_labels_containers[ NB_KEYS ];

    int key_index = 0;
    for ( int row = 0; row < KB_NB_ROWS; row++ ) {
        rows_containers[ row ] = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, KB_SPACING_KEYS );
        gtk_box_set_homogeneous( GTK_BOX( rows_containers[ row ] ), true );
        gtk_container_add( GTK_CONTAINER( keyboard_container ), rows_containers[ row ] );

        for ( int column = 0; column < ( ( row == 0 ) ? KB_NB_COLS_MENU : KB_NB_COLS ); column++ ) {
            keys_containers[ key_index ] = gtk_box_new( GTK_ORIENTATION_VERTICAL, 3 );
            gtk_box_set_homogeneous( GTK_BOX( keys_containers[ key_index ] ), false );
            gtk_container_add( GTK_CONTAINER( rows_containers[ row ] ), keys_containers[ key_index ] );

            button = &ui->buttons[ key_index ];
            button->x49gp = x49gp;
            button->key = &ui_keys[ key_index ];

            keys_top_labels_containers[ key_index ] = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 0 );
            gtk_box_set_homogeneous( GTK_BOX( keys_top_labels_containers[ key_index ] ), false );

            gtk_container_add( GTK_CONTAINER( keys_containers[ key_index ] ), keys_top_labels_containers[ key_index ] );

            if ( button->key->right ) {
                gtk_box_pack_start( GTK_BOX( keys_top_labels_containers[ key_index ] ),
                                    _ui_load__create_label( "label-left", button->key->left ), true, true, 0 );
                gtk_box_pack_end( GTK_BOX( keys_top_labels_containers[ key_index ] ),
                                  _ui_load__create_label( "label-right", button->key->right ), true, true, 0 );
            } else if ( button->key->left )
                gtk_box_set_center_widget( GTK_BOX( keys_top_labels_containers[ key_index ] ),
                                           _ui_load__create_label( "label-left", button->key->left ) );

            button->button = gtk_button_new();
#if GTK_MAJOR_VERSION == 4
            gtk_widget_add_css_class( button->button, button->key->css_class );
#else
            gtk_style_context_add_class( gtk_widget_get_style_context( button->button ), button->key->css_class );
#endif
            gtk_widget_set_size_request( button->button, ( row == 0 ) ? KB_WIDTH_6_KEYS : KB_WIDTH_5_KEYS,
                                         ( row == 0 )         ? KB_HEIGHT_MENU_KEYS
                                         : ( key_index < 30 ) ? KB_HEIGHT_SMALL_KEYS
                                                              : KB_HEIGHT_BIG_KEYS );
            gtk_widget_set_can_focus( button->button, false );
            gtk_widget_add_events( button->button, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_LEAVE_NOTIFY_MASK );
            g_signal_connect( G_OBJECT( button->button ), "button-press-event", G_CALLBACK( react_to_button_press ), button );
            g_signal_connect( G_OBJECT( button->button ), "button-release-event", G_CALLBACK( react_to_button_release ), button );
            g_signal_connect( G_OBJECT( button->button ), "leave-notify-event", G_CALLBACK( react_to_button_leave ), button );
            gtk_container_add( GTK_CONTAINER( keys_containers[ key_index ] ), button->button );

            if ( button->key->label )
                gtk_container_add( GTK_CONTAINER( button->button ), _ui_load__create_label( "label-key", button->key->label ) );

            if ( button->key->below )
                gtk_container_add( GTK_CONTAINER( keys_containers[ key_index ] ),
                                   _ui_load__create_label( "label-below", button->key->below ) );
            if ( button->key->letter )
                gtk_container_add( GTK_CONTAINER( keys_containers[ key_index ] ),
                                   _ui_load__create_label( "label-letter", button->key->letter ) );

            key_index++;
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
    bool is_50g = ( ui->calculator == UI_CALCULATOR_HP50G );
    char* css;

    asprintf( &css, css_global, is_50g ? color_bg_50g : color_bg_49gp, is_50g ? color_shift_left_50g : color_shift_left_49gp,
              FONT_SIZE_SYMBOL, FONT_SIZE_NUMBER, FONT_SIZE_KEY, FONT_SIZE_TINY, is_50g ? color_shift_left_50g : color_shift_left_49gp );

    gtk_css_provider_load_from_data( style_provider, css, -1, NULL );

    gtk_style_context_add_provider_for_screen( gdk_screen_get_default(), GTK_STYLE_PROVIDER( style_provider ),
                                               GTK_STYLE_PROVIDER_PRIORITY_USER + 1 );

    g_object_unref( style_provider );

    // finally show the window
    gtk_widget_realize( ui->window );
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
        gtk_widget_set_opacity( ui->ui_ann_left, x49gp_get_pixel_color( lcd, 131, 1 ) );
        gtk_widget_set_opacity( ui->ui_ann_right, x49gp_get_pixel_color( lcd, 131, 2 ) );
        gtk_widget_set_opacity( ui->ui_ann_alpha, x49gp_get_pixel_color( lcd, 131, 3 ) );
        gtk_widget_set_opacity( ui->ui_ann_battery, x49gp_get_pixel_color( lcd, 131, 4 ) );
        gtk_widget_set_opacity( ui->ui_ann_busy, x49gp_get_pixel_color( lcd, 131, 5 ) );
        gtk_widget_set_opacity( ui->ui_ann_io, x49gp_get_pixel_color( lcd, 131, 0 ) );

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

    if ( opt.verbose ) {
        fprintf( stderr,
                 "\n FONT_SIZE_KEY = %i\n"
                 " FONT_SIZE_SYMBOL = %i\n"
                 " FONT_SIZE_NUMBER = %i\n"
                 " FONT_SIZE_TINY = %i\n"
                 " TINY_TEXT_HEIGHT = %i\n"
                 " TINY_TEXT_WIDTH = %i\n"
                 " KB_WIDTH_6_KEYS = %i\n"
                 " KB_WIDTH_5_KEYS = %i\n"
                 " KB_HEIGHT_MENU_KEYS = %i\n"
                 " KB_HEIGHT_SMALL_KEYS = %i\n"
                 " KB_HEIGHT_BIG_KEYS = %i\n"
                 " KB_SPACING_KEYS = %i\n"
                 " KB_COLUMN_WIDTH_6_KEYS = %i\n"
                 " KB_COLUMN_WIDTH_5_KEYS = %i\n"
                 " ANNUNCIATOR_WIDTH = %i\n"
                 " ANNUNCIATOR_HEIGHT = %i\n"
                 " ANNUNCIATORS_HEIGHT = %i\n"
                 " LCD_PIXEL_SCALE = %i\n"
                 " LCD_WIDTH = %i\n"
                 " LCD_HEIGHT = %i\n"
                 " WINDOW_WIDTH = %i\n",
                 FONT_SIZE_KEY, FONT_SIZE_SYMBOL, FONT_SIZE_NUMBER, FONT_SIZE_TINY, TINY_TEXT_HEIGHT, TINY_TEXT_WIDTH, KB_WIDTH_6_KEYS,
                 KB_WIDTH_5_KEYS, KB_HEIGHT_MENU_KEYS, KB_HEIGHT_SMALL_KEYS, KB_HEIGHT_BIG_KEYS, KB_SPACING_KEYS, KB_COLUMN_WIDTH_6_KEYS,
                 KB_COLUMN_WIDTH_5_KEYS, ANNUNCIATOR_WIDTH, ANNUNCIATOR_HEIGHT, ANNUNCIATORS_HEIGHT, LCD_PIXEL_SCALE, LCD_WIDTH, LCD_HEIGHT,
                 WINDOW_WIDTH );
    }

    if ( x49gp_module_init( x49gp, "gui", ui_init, ui_exit, ui_reset, ui_load, ui_save, NULL, &module ) )
        return -1;

    return x49gp_module_register( module );
}
