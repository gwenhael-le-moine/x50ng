#include <unistd.h>
#include <stdarg.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/times.h>

#include "api.h"
#include "inner.h"
#include "gtk.h"
#include "ncurses.h"

char* ui_annunciators[ NB_ANNUNCIATORS ] = { "⮢", "⮣", "α", "🪫", "⌛", "⇄" };

ui4x_config_t ui4x_config;
ui4x_emulator_api_t ui4x_emulator_api;

/*************/
/* functions */
/*************/
static void newrplify_buttons_hp50g()
{
    // modify keys' labeling for newRPL
    for ( int i = HP50g_KEY_A; i <= HP50g_KEY_F; i++ )
        buttons_hp50g[ i ].left = "";

    for ( int i = HP50g_KEY_G; i <= HP50g_KEY_I; i++ ) {
        buttons_hp50g[ i ].label = "";
        buttons_hp50g[ i ].left = "";
        buttons_hp50g[ i ].right = NULL;
    }

    for ( int i = HP50g_KEY_J; i <= HP50g_KEY_K; i++ ) {
        buttons_hp50g[ i ].label = "";
        buttons_hp50g[ i ].left = "";
        buttons_hp50g[ i ].right = NULL;
    }

    buttons_hp50g[ HP50g_KEY_UP ].left = "UPDIR";

    buttons_hp50g[ HP50g_KEY_LEFT ].left = "BEG";
    buttons_hp50g[ HP50g_KEY_LEFT ].right = "COPY";

    buttons_hp50g[ HP50g_KEY_DOWN ].left = "CUT";

    buttons_hp50g[ HP50g_KEY_RIGHT ].left = "END";
    buttons_hp50g[ HP50g_KEY_RIGHT ].right = "PASTE";

    buttons_hp50g[ HP50g_KEY_M ].label = "STO⏵";
    buttons_hp50g[ HP50g_KEY_M ].left = "RCL";
    buttons_hp50g[ HP50g_KEY_M ].right = "PREV.M";

    for ( int i = HP50g_KEY_N; i <= HP50g_KEY_O; i++ ) {
        buttons_hp50g[ i ].left = "";
        buttons_hp50g[ i ].right = NULL;
    }

    buttons_hp50g[ HP50g_KEY_P ].label = "MENU";

    buttons_hp50g[ HP50g_KEY_BACKSPACE ].left = "";

    for ( int i = HP50g_KEY_S; i <= HP50g_KEY_U; i++ )
        buttons_hp50g[ i ].right = NULL;

    for ( int i = HP50g_KEY_ALPHA; i <= HP50g_KEY_9; i++ )
        buttons_hp50g[ i ].left = "";

    buttons_hp50g[ HP50g_KEY_8 ].right = NULL;

    for ( int i = HP50g_KEY_4; i <= HP50g_KEY_6; i++ ) {
        buttons_hp50g[ i ].left = "";
        buttons_hp50g[ i ].right = NULL;
    }

    buttons_hp50g[ HP50g_KEY_2 ].left = "";

    buttons_hp50g[ HP50g_KEY_ON ].left = "";
    buttons_hp50g[ HP50g_KEY_ON ].below = NULL;

    buttons_hp50g[ HP50g_KEY_ENTER ].left = "";
}

/********************/
/* Public functions */
/********************/
void ui_handle_pending_inputs( void )
{
    switch ( ui4x_config.frontend ) {
        case FRONTEND_NCURSES:
            ncurses_handle_pending_inputs();
            break;
        case FRONTEND_GTK:
        default:
            gtk_ui_handle_pending_inputs();
            break;
    }
}

void ui_refresh_output( void )
{
    switch ( ui4x_config.frontend ) {
        case FRONTEND_NCURSES:
            ncurses_refresh_lcd();
            break;
        case FRONTEND_GTK:
        default:
            gtk_ui_refresh_lcd();
            break;
    }
}

void ui_init( ui4x_config_t* config, ui4x_emulator_api_t* emulator_api )
{
    ui4x_config = *config;
    ui4x_emulator_api = *emulator_api;

    if ( ui4x_config.newrpl_keyboard )
        newrplify_buttons_hp50g();

    switch ( ui4x_config.frontend ) {
        case FRONTEND_NCURSES:
            ncurses_init();
            break;
        case FRONTEND_GTK:
        default:
            gtk_ui_init();
            break;
    }
}

void ui_exit( void )
{
    switch ( ui4x_config.frontend ) {
        case FRONTEND_NCURSES:
            ncurses_exit();
            break;
        case FRONTEND_GTK:
        default:
            gtk_ui_exit();
            break;
    }
}
