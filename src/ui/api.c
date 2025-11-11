#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <sys/times.h>

#include "../hdw.h"
#include "../options.h"
#include "../timer.h"
#include "../types.h"

#include "inner.h"
#include "gtk.h"
#include "ncurses.h"

char* ui_annunciators[ NB_ANNUNCIATORS ] = { "⮢", "⮣", "α", "🪫", "⌛", "⇄" };

void ( *emulator_press_key )( int hpkey );
void ( *emulator_release_key )( int hpkey );
bool ( *emulator_is_key_pressed )( int hpkey );

bool ( *emulator_is_display_on )( void );
unsigned char ( *emulator_get_annunciators )( void );
void ( *emulator_get_lcd_buffer )( int* target );
int ( *emulator_get_contrast )( void );

static config_t* __config;

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
void ui_handle_pending_inputs( void* data )
{
    hdw_t* hdw_state = data;

    switch ( __config->frontend ) {
        case FRONTEND_NCURSES:
            ncurses_handle_pending_inputs();
            break;
        case FRONTEND_GTK:
        default:
            gtk_ui_handle_pending_inputs();
            break;
    }

    timer_mod( hdw_state->timer_ui_input, timer_get_clock() + UI_EVENTS_REFRESH_INTERVAL );
}

void ui_refresh_output( void* data )
{
    hdw_t* hdw_state = data;

    switch ( __config->frontend ) {
        case FRONTEND_NCURSES:
            ncurses_refresh_lcd();
            break;
        case FRONTEND_GTK:
        default:
            gtk_ui_refresh_lcd();
            break;
    }

    timer_mod( hdw_state->timer_ui_output, timer_get_clock() + UI_LCD_REFRESH_INTERVAL );
}

void ui_init( hdw_t* hdw_state, config_t* config, void ( *api_emulator_press_key )( int hpkey ),
              void ( *api_emulator_release_key )( int hpkey ), bool ( *api_emulator_is_key_pressed )( int hpkey ),
              bool ( *api_emulator_is_display_on )( void ), unsigned char ( *api_emulator_get_annunciators )( void ),
              void ( *api_emulator_get_lcd_buffer )( int* target ), int ( *api_emulator_get_contrast )( void ) )
{
    __config = config;

    if ( __config->newrpl_keyboard )
        newrplify_buttons_hp50g();

    switch ( __config->frontend ) {
        case FRONTEND_NCURSES:
            ncurses_init( hdw_state, __config, api_emulator_press_key, api_emulator_release_key, api_emulator_is_key_pressed,
                          api_emulator_is_display_on, api_emulator_get_annunciators, api_emulator_get_lcd_buffer,
                          api_emulator_get_contrast );
            break;
        case FRONTEND_GTK:
        default:
            gtk_ui_init( hdw_state, __config, api_emulator_press_key, api_emulator_release_key, api_emulator_is_key_pressed,
                         api_emulator_is_display_on, api_emulator_get_annunciators, api_emulator_get_lcd_buffer,
                         api_emulator_get_contrast );
            break;
    }
}

void ui_exit( void )
{
    switch ( __config->frontend ) {
        case FRONTEND_NCURSES:
            ncurses_exit();
            break;
        case FRONTEND_GTK:
        default:
            gtk_ui_exit();
            break;
    }
}
