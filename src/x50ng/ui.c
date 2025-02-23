#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <sys/times.h>
#include <errno.h>

#include <gtk/gtk.h>
#include <glib.h>
#include <cairo.h>
#include <gdk/gdkkeysyms.h>

#include "options.h"
#include "x49gp.h"
#include "ui.h"
#include "s3c2410.h"

#include "gdbstub.h"

// #define TEST_PASTE true

#define KB_NB_ROWS 10

#define LCD_WIDTH ( 131 * opt.zoom )
#define LCD_HEIGHT ( 80 * opt.zoom )

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
};

/*************/
/* functions */
/*************/

static inline void x50ng_set_key_state( x49gp_t* x49gp, const x49gp_ui_key_t* key, bool state )
{
    if ( opt.verbose )
        fprintf( stderr, "%s -> %s\n", key->label, state ? "true" : "false" );

    if ( key->rowbit )
        s3c2410_io_port_g_update( x49gp, key->column, key->row, key->columnbit, key->rowbit, state );
    else
        s3c2410_io_port_f_set_bit( x49gp, key->eint, state );
}
#define X50NG_PRESS_KEY( x49gp, key ) x50ng_set_key_state( x49gp, key, true );
#define X50NG_RELEASE_KEY( x49gp, key ) x50ng_set_key_state( x49gp, key, false );

static void ui_release_button( x49gp_ui_button_t* button )
{
    if ( !button->down )
        return;

    x49gp_t* x49gp = button->x49gp;
    const x49gp_ui_key_t* key = button->key;

    button->down = false;
    button->hold = false;

#if GTK_MAJOR_VERSION == 4
    gtk_widget_remove_css_class( button->button, "key-down" );
#else
    gtk_style_context_remove_class( gtk_widget_get_style_context( button->button ), "key-down" );
#endif

    X50NG_RELEASE_KEY( x49gp, key );
}

static bool ui_press_button( x49gp_ui_button_t* button, bool hold )
{
    x49gp_t* x49gp = button->x49gp;
    const x49gp_ui_key_t* key = button->key;

    if ( button->down ) {
        if ( button->hold && hold ) {
            ui_release_button( button );
            return GDK_EVENT_STOP;
        } else
            return GDK_EVENT_PROPAGATE;
    }

    button->down = true;
    button->hold = hold;

#if GTK_MAJOR_VERSION == 4
    gtk_widget_add_css_class( button->button, "key-down" );
#else
    gtk_style_context_add_class( gtk_widget_get_style_context( button->button ), "key-down" );
#endif

    X50NG_RELEASE_KEY( x49gp, key );

    return GDK_EVENT_STOP;
}

static bool react_to_button_press( GtkWidget* widget, GdkEventButton* event, gpointer user_data )
{
    x49gp_ui_button_t* button = user_data;
    const x49gp_ui_key_t* key = button->key;
    x49gp_t* x49gp = button->x49gp;

    if ( event->type != GDK_BUTTON_PRESS || event->button > 3 )
        return GDK_EVENT_PROPAGATE;

    if ( !ui_press_button( button, event->button == 3 ) )
        return GDK_EVENT_PROPAGATE;

    X50NG_PRESS_KEY( x49gp, key );

    return GDK_EVENT_STOP;
}

static bool react_to_button_release( GtkWidget* widget, GdkEventButton* event, gpointer user_data )
{
    x49gp_ui_button_t* button = user_data;

    if ( event->type != GDK_BUTTON_RELEASE )
        return GDK_EVENT_PROPAGATE;

    if ( event->button != 1 )
        return GDK_EVENT_PROPAGATE;

    ui_release_button( button );

    return GDK_EVENT_STOP;
}

#if GTK_MAJOR_VERSION == 3
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

    if ( opt.debug_port != 0 && !gdbserver_isactive() ) {
        gdbserver_start( opt.debug_port );
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
#endif

static bool react_to_key_event( GtkWidget* _widget, GdkEventKey* event, gpointer user_data )
{
    x49gp_t* x49gp = user_data;
    x49gp_ui_t* ui = x49gp->ui;

    /* We want to know the keyval as interpreted without modifiers. */
    /* However, there is one modifier we do care about: NumLock, */
    /* which normally is represented by MOD2. */
    guint keyval;
    if ( !gdk_keymap_translate_keyboard_state( gdk_keymap_get_for_display( gdk_display_get_default() ), event->hardware_keycode,
                                               event->state & GDK_MOD2_MASK, event->group, &keyval, NULL, NULL, NULL ) )
        return GDK_EVENT_PROPAGATE;

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
            x49gp->arm_exit = 1;
            cpu_exit( x49gp->env );
            return GDK_EVENT_STOP;

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
            return GDK_EVENT_STOP;

        case GDK_KEY_Menu:
            gtk_widget_set_sensitive( ui->menu_unmount, s3c2410_sdi_is_mounted( x49gp ) );
            if ( ui->menu_debug )
                gtk_widget_set_sensitive( ui->menu_debug, !gdbserver_isactive() );

            gtk_menu_popup_at_widget( GTK_MENU( ui->menu ), ui->window, GDK_GRAVITY_NORTH_WEST, GDK_GRAVITY_NORTH_WEST, NULL );
            return GDK_EVENT_STOP;

        default:
            return GDK_EVENT_PROPAGATE;
    }

    // Bypassing GUI buttons:
    /* switch ( event->type ) { */
    /*     case GDK_KEY_PRESS: */
    /*         X50NG_PRESS_KEY( x49gp, &ui_keys[ hpkey ] ); */
    /*         break; */
    /*     case GDK_KEY_RELEASE: */
    /*         X50NG_RELEASE_KEY( x49gp, &ui_keys[ hpkey ] ); */
    /*         break; */
    /*     default: */
    /*         return GDK_EVENT_PROPAGATE; */
    /* } */

    // Using GUI buttons:
    x49gp_ui_button_t* button = &ui->buttons[ hpkey ];
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
            return GDK_EVENT_PROPAGATE;
    }

    return GDK_EVENT_STOP;
}

#ifdef TEST_PASTE
static void x50g_string_to_keys_sequence( x49gp_t* x49gp, const char* input )
{
    // struct timespec key_press_delay = { 1, 0 };
    int hpkey = -1;
    for ( int i = 0; i < strlen( input ); i++ ) {
        if ( hpkey > 0 )
            X50NG_RELEASE_KEY( x49gp, &ui_keys[ hpkey ] )

        fprintf( stderr, "%c", input[ i ] );
        if ( input[ i ] >= '0' && input[ i ] <= '9' ) {
            switch ( input[ i ] ) {
                case '0':
                    hpkey = HPKEY_0;
                    break;
                case '1':
                    hpkey = HPKEY_1;
                    break;
                case '2':
                    hpkey = HPKEY_2;
                    break;
                case '3':
                    hpkey = HPKEY_3;
                    break;
                case '4':
                    hpkey = HPKEY_4;
                    break;
                case '5':
                    hpkey = HPKEY_5;
                    break;
                case '6':
                    hpkey = HPKEY_6;
                    break;
                case '7':
                    hpkey = HPKEY_7;
                    break;
                case '8':
                    hpkey = HPKEY_8;
                    break;
                case '9':
                    hpkey = HPKEY_9;
                    break;
                default:
                    hpkey = HPKEY_ENTER;
            }
        }

        if ( hpkey > 0 ) {
            X50NG_RELEASE_KEY( x49gp, &ui_keys[ hpkey ] )
            X50NG_PRESS_KEY( x49gp, &ui_keys[ hpkey ] )
        }
    }
    fprintf( stderr, "\n" );

    if ( hpkey > 0 )
        X50NG_RELEASE_KEY( x49gp, &ui_keys[ hpkey ] )

    /* if ( hpkey > 0 ) { */
    /*     X50NG_RELEASE_KEY( x49gp, &ui_keys[ HPKEY_ENTER ] ) */
    /*     X50NG_PRESS_KEY( x49gp, &ui_keys[ HPKEY_ENTER ] ) */
    /*     X50NG_RELEASE_KEY( x49gp, &ui_keys[ HPKEY_ENTER ] ) */
    /* } */
}
#endif

#if GTK_MAJOR_VERSION == 3
static bool react_to_display_click( GtkWidget* widget, GdkEventButton* event, gpointer user_data )
{
    gdk_window_focus( gtk_widget_get_window( widget ), event->time );
    gdk_window_raise( gtk_widget_get_window( widget ) );

    x49gp_t* x49gp = user_data;
    x49gp_ui_t* ui = x49gp->ui;

#  if GTK_MAJOR_VERSION == 3
    gtk_widget_set_sensitive( ui->menu_unmount, s3c2410_sdi_is_mounted( x49gp ) );
    if ( ui->menu_debug )
        gtk_widget_set_sensitive( ui->menu_debug, !gdbserver_isactive() );
#  endif

    if ( event->type != GDK_BUTTON_PRESS )
        return GDK_EVENT_PROPAGATE;

    switch ( event->button ) {
        case 1: // left click
            gdk_window_begin_move_drag( gtk_widget_get_window( ui->window ), event->button, event->x_root, event->y_root, event->time );
            break;
#  ifdef TEST_PASTE
        case 2: // middle click
            GtkClipboard* clip = gtk_clipboard_get( GDK_SELECTION_CLIPBOARD );
            gchar* text = gtk_clipboard_wait_for_text( clip );
            fprintf( stderr, "clipboard: %s\n", text );

            // x50g_string_to_keys_sequence( x49gp, text );
            x50g_string_to_keys_sequence( x49gp, "0123456789\n" );

            return GDK_EVENT_STOP;
#  endif
        case 3: // right click
#  if GTK_MAJOR_VERSION == 3
            gtk_menu_popup_at_pointer( GTK_MENU( ui->menu ), NULL );
#  else
            fprintf( stderr, "menu not implemented in gtk4 yet.\n" );
#  endif
            return GDK_EVENT_STOP;
        default:
            break;
    }

    return GDK_EVENT_STOP;
}
#endif

static int redraw_lcd( GtkWidget* widget, cairo_t* cr, gpointer user_data )
{
    x49gp_t* x49gp = user_data;
    x49gp_ui_t* ui = x49gp->ui;

    cairo_set_source_surface( cr, ui->lcd_surface, 0, 0 );
    cairo_paint( cr );

    return GDK_EVENT_STOP;
}

static int draw_lcd( GtkWidget* widget, GdkEventConfigure* event, gpointer user_data )
{
    x49gp_t* x49gp = user_data;
    x49gp_ui_t* ui = x49gp->ui;

    if ( NULL == ui->lcd_surface )
        ui->lcd_surface = cairo_image_surface_create( CAIRO_FORMAT_RGB24, LCD_WIDTH, LCD_HEIGHT );

    return GDK_EVENT_STOP;
}

#ifdef TEST_PASTE
static void do_paste( gpointer user_data, GtkWidget* widget, GdkEvent* event )
{
    x49gp_t* x49gp = user_data;

    // x49gp->arm_exit++;

    X50NG_RELEASE_KEY( x49gp, &ui_keys[ HPKEY_1 ] )
    X50NG_PRESS_KEY( x49gp, &ui_keys[ HPKEY_1 ] )
    // nanosleep( &key_press_delay, NULL );
    sleep( 10 );
    X50NG_RELEASE_KEY( x49gp, &ui_keys[ HPKEY_1 ] )
    /* GtkClipboard* clip = gtk_clipboard_get( GDK_SELECTION_CLIPBOARD ); */
    /* gchar* text = gtk_clipboard_wait_for_text( clip ); */
    /* fprintf( stderr, "clipboard: %s\n", text ); */

    /* // x50g_string_to_keys_sequence( x49gp, text ); */
    /* x50g_string_to_keys_sequence( x49gp, "0123456789\n" ); */
}
#endif

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
    int r = 165, g = 210, b = 180; /* start r, g, b from their brightest value */
    if ( opt.gray )
        r = g = b = 210;
    int step_r = r / 15;
    int step_g = g / 15;
    int step_b = b / 15;
    for ( int i = 0; i < 16; i++ ) {
        _ui_load__init_color( &ui->colors[ UI_COLOR_GRAYSCALE_0 + i ], r, g, b );
        if ( opt.verbose )
            fprintf( stderr, "UI_COLOR_GRAYSCALE_%i = %i, %i, %i\n", UI_COLOR_GRAYSCALE_0 + i, r, g, b );
        r -= step_r;
        g -= step_g;
        b -= step_b;
    }

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

#if GTK_MAJOR_VERSION == 4
    ui->window = gtk_window_new();
#else
    ui->window = gtk_window_new( GTK_WINDOW_TOPLEVEL );
    gtk_window_set_accept_focus( GTK_WINDOW( ui->window ), true );
    gtk_window_set_focus_on_map( GTK_WINDOW( ui->window ), true );
#endif
    gtk_window_set_decorated( GTK_WINDOW( ui->window ), true );
    gtk_window_set_resizable( GTK_WINDOW( ui->window ), true );
    gtk_window_set_title( GTK_WINDOW( ui->window ), ui->name );

    GtkWidget* window_container = gtk_box_new( GTK_ORIENTATION_VERTICAL, 0 );
    gtk_container_add( GTK_CONTAINER( ui->window ), window_container );

    g_signal_connect( G_OBJECT( ui->window ), "key-press-event", G_CALLBACK( react_to_key_event ), x49gp );
    g_signal_connect( G_OBJECT( ui->window ), "key-release-event", G_CALLBACK( react_to_key_event ), x49gp );
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

    GtkWidget* annunciators_container = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 0 );
    gtk_box_set_homogeneous( GTK_BOX( annunciators_container ), true );
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

#if GTK_MAJOR_VERSION == 4
    gtk_container_add( GTK_CONTAINER( window_container ), display_container );
#else
    GtkWidget* display_container_event_box = gtk_event_box_new();
    g_signal_connect( G_OBJECT( display_container_event_box ), "button-press-event", G_CALLBACK( react_to_display_click ), x49gp );
    gtk_container_add( GTK_CONTAINER( display_container_event_box ), display_container );

    gtk_container_add( GTK_CONTAINER( window_container ), display_container_event_box );
#endif

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
    int nb_keys_in_row = 0;
    for ( int row = 0; row < KB_NB_ROWS; row++ ) {
        rows_containers[ row ] = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 0 );
        gtk_box_set_homogeneous( GTK_BOX( rows_containers[ row ] ), true );
        gtk_container_add( GTK_CONTAINER( keyboard_container ), rows_containers[ row ] );

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
            keys_containers[ key_index ] = gtk_box_new( GTK_ORIENTATION_VERTICAL, 2 );
            gtk_box_set_homogeneous( GTK_BOX( keys_containers[ key_index ] ), false );
            if ( row == 1 && column == 3 )
                gtk_container_add( GTK_CONTAINER( rows_containers[ row ] ), gtk_box_new( GTK_ORIENTATION_VERTICAL, 2 ) );
            gtk_container_add( GTK_CONTAINER( rows_containers[ row ] ), keys_containers[ key_index ] );
            if ( row == 1 && column == 3 )
                gtk_container_add( GTK_CONTAINER( rows_containers[ row ] ), gtk_box_new( GTK_ORIENTATION_VERTICAL, 2 ) );

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
            gtk_widget_set_can_focus( button->button, false );
            gtk_widget_add_events( button->button, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_LEAVE_NOTIFY_MASK );
            g_signal_connect( G_OBJECT( button->button ), "button-press-event", G_CALLBACK( react_to_button_press ), button );
            g_signal_connect( G_OBJECT( button->button ), "button-release-event", G_CALLBACK( react_to_button_release ), button );
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
#if GTK_MAJOR_VERSION == 3
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

    if ( opt.debug_port != 0 ) {
        gtk_menu_shell_append( GTK_MENU_SHELL( ui->menu ), gtk_separator_menu_item_new() );

        GtkWidget* menu_debug = gtk_menu_item_new_with_label( "Start debugger" );
        gtk_menu_shell_append( GTK_MENU_SHELL( ui->menu ), menu_debug );
        g_signal_connect( G_OBJECT( menu_debug ), "activate", G_CALLBACK( do_start_gdb_server ), x49gp );
        ui->menu_debug = menu_debug;
    } else
        ui->menu_debug = NULL;

    gtk_menu_shell_append( GTK_MENU_SHELL( ui->menu ), gtk_separator_menu_item_new() );

#  ifdef TEST_PASTE
    GtkWidget* menu_paste = gtk_menu_item_new_with_label( "Paste" );
    gtk_menu_shell_append( GTK_MENU_SHELL( ui->menu ), menu_paste );
    g_signal_connect_swapped( G_OBJECT( menu_paste ), "activate", G_CALLBACK( do_paste ), x49gp );
#  endif

    GtkWidget* menu_reset = gtk_menu_item_new_with_label( "Reset" );
    gtk_menu_shell_append( GTK_MENU_SHELL( ui->menu ), menu_reset );
    g_signal_connect( G_OBJECT( menu_reset ), "activate", G_CALLBACK( do_emulator_reset ), x49gp );

    GtkWidget* menu_quit = gtk_menu_item_new_with_label( "Quit" );
    gtk_menu_shell_append( GTK_MENU_SHELL( ui->menu ), menu_quit );
    g_signal_connect_swapped( G_OBJECT( menu_quit ), "activate", G_CALLBACK( do_quit ), x49gp );

    gtk_widget_show_all( ui->menu );
#endif

    // Apply CSS
    char* style_full_path = g_build_filename( opt.datadir, opt.style_filename, NULL );
    if ( !g_file_test( style_full_path, G_FILE_TEST_EXISTS ) )
        style_full_path = g_build_filename( GLOBAL_DATADIR, opt.style_filename, NULL );

    if ( !g_file_test( style_full_path, G_FILE_TEST_EXISTS ) )
        fprintf( stderr, "Can't load style %s neither from %s/%s nor form %s/%s\n", opt.style_filename, opt.datadir, opt.style_filename,
                 GLOBAL_DATADIR, opt.style_filename );
    else {
        GtkCssProvider* style_provider = gtk_css_provider_new();
        gtk_css_provider_load_from_path( style_provider, style_full_path, NULL );

        gtk_style_context_add_provider_for_screen( gdk_screen_get_default(), GTK_STYLE_PROVIDER( style_provider ),
                                                   GTK_STYLE_PROVIDER_PRIORITY_USER + 1 );

        g_object_unref( style_provider );

        if ( opt.verbose )
            fprintf( stderr, "Loaded style from %s\n", style_full_path );
    }
    free( style_full_path );

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

        for ( int y = 0; y < ( LCD_HEIGHT / opt.zoom ); y++ )
            for ( int x = 0; x < ( LCD_WIDTH / opt.zoom ); x++ )
                _draw_pixel( ui->lcd_surface, opt.zoom * x, opt.zoom * y, opt.zoom, opt.zoom,
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
#if GTK_MAJOR_VERSION == 4
    fprintf( stderr, "Error: %s\n", text );
#else
    x49gp_ui_t* ui = x49gp->ui;

    GtkWidget* dialog =
        gtk_message_dialog_new( GTK_WINDOW( ui->window ), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "%s", text );

    gtk_dialog_run( GTK_DIALOG( dialog ) );
    gtk_widget_destroy( dialog );
#endif
}

void gui_open_firmware( x49gp_t* x49gp, char** filename )
{
#if GTK_MAJOR_VERSION == 3
    ui_open_file_dialog( x49gp, "Choose firmware ...", GTK_FILE_CHOOSER_ACTION_OPEN, filename );
#endif
}

int gui_init( x49gp_t* x49gp )
{
    x49gp_module_t* module;

    if ( x49gp_module_init( x49gp, "gui", ui_init, ui_exit, ui_reset, ui_load, ui_save, NULL, &module ) )
        return -1;

    return x49gp_module_register( module );
}
