#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "s3c2410/s3c2410_arm.h"
#include "s3c2410/s3c2410_io_port.h"
#include "s3c2410/s3c2410_lcd.h"
#include "s3c2410/s3c2410_sdi.h"

#include "ui/api.h"

#include "emulator_api.h"
#include "list.h"
#include "hdw.h"
#include "gdbstub.h"
#include "options.h"
#include "sram.h"
#include "flash.h"
#include "module.h"

static hdw_t* __hdw_state;
static config_t* __config;

typedef struct hp50g_key_t {
    int column;
    int row;
    int eint;
    bool pressed;
} hp50g_key_t;

static hp50g_key_t x50ng_keyboard[ NB_HP50g_KEYS ] = {
    {.column = 5, .row = 1, .eint = -1 /* 1 */, .pressed = false},
    {.column = 5, .row = 2, .eint = -1 /* 2 */, .pressed = false},
    {.column = 5, .row = 3, .eint = -1 /* 3 */, .pressed = false},
    {.column = 5, .row = 4, .eint = -1 /* 4 */, .pressed = false},
    {.column = 5, .row = 5, .eint = -1 /* 5 */, .pressed = false},
    {.column = 5, .row = 6, .eint = -1 /* 6 */, .pressed = false},

    {.column = 5, .row = 7, .eint = -1 /* 7 */, .pressed = false},
    {.column = 6, .row = 5, .eint = -1 /* 5 */, .pressed = false},
    {.column = 6, .row = 6, .eint = -1 /* 6 */, .pressed = false},
    {.column = 6, .row = 1, .eint = -1 /* 1 */, .pressed = false},
    {.column = 6, .row = 7, .eint = -1 /* 7 */, .pressed = false},
    {.column = 7, .row = 1, .eint = -1 /* 1 */, .pressed = false},
    {.column = 7, .row = 2, .eint = -1 /* 2 */, .pressed = false},

    {.column = 6, .row = 2, .eint = -1 /* 2 */, .pressed = false},
    {.column = 6, .row = 3, .eint = -1 /* 3 */, .pressed = false},
    {.column = 6, .row = 4, .eint = -1 /* 4 */, .pressed = false},

    {.column = 4, .row = 1, .eint = -1 /* 1 */, .pressed = false},
    {.column = 3, .row = 1, .eint = -1 /* 1 */, .pressed = false},
    {.column = 2, .row = 1, .eint = -1 /* 1 */, .pressed = false},
    {.column = 1, .row = 1, .eint = -1 /* 1 */, .pressed = false},
    {.column = 0, .row = 1, .eint = -1 /* 1 */, .pressed = false},

    {.column = 4, .row = 2, .eint = -1 /* 2 */, .pressed = false},
    {.column = 3, .row = 2, .eint = -1 /* 2 */, .pressed = false},
    {.column = 2, .row = 2, .eint = -1 /* 2 */, .pressed = false},
    {.column = 1, .row = 2, .eint = -1 /* 2 */, .pressed = false},
    {.column = 0, .row = 2, .eint = -1 /* 2 */, .pressed = false},

    {.column = 0, .row = 7, .eint = -1 /* 7 */, .pressed = false},
    {.column = 4, .row = 3, .eint = -1 /* 3 */, .pressed = false},
    {.column = 3, .row = 3, .eint = -1 /* 3 */, .pressed = false},
    {.column = 2, .row = 3, .eint = -1 /* 3 */, .pressed = false},
    {.column = 1, .row = 3, .eint = -1 /* 3 */, .pressed = false},

    {.column = 0, .row = 0, .eint = 4,          .pressed = false}, /* HP50g_KEY_ALPHA */
    {.column = 3, .row = 4, .eint = -1 /* 4 */, .pressed = false},
    {.column = 2, .row = 4, .eint = -1 /* 4 */, .pressed = false},
    {.column = 1, .row = 4, .eint = -1 /* 4 */, .pressed = false},
    {.column = 0, .row = 3, .eint = -1 /* 3 */, .pressed = false},

    {.column = 0, .row = 0, .eint = 5,          .pressed = false}, /* HP50g_KEY_SHIFT_LEFT */
    {.column = 3, .row = 5, .eint = -1 /* 5 */, .pressed = false},
    {.column = 2, .row = 5, .eint = -1 /* 5 */, .pressed = false},
    {.column = 1, .row = 5, .eint = -1 /* 5 */, .pressed = false},
    {.column = 0, .row = 4, .eint = -1 /* 4 */, .pressed = false},

    {.column = 0, .row = 0, .eint = 6,          .pressed = false}, /* HP50g_KEY_SHIFT_RIGHT */
    {.column = 3, .row = 6, .eint = -1 /* 6 */, .pressed = false},
    {.column = 2, .row = 6, .eint = -1 /* 6 */, .pressed = false},
    {.column = 1, .row = 6, .eint = -1 /* 6 */, .pressed = false},
    {.column = 0, .row = 5, .eint = -1 /* 5 */, .pressed = false},

    {.column = 0, .row = 0, .eint = 0,          .pressed = false}, /* HP50g_KEY_ON */
    {.column = 3, .row = 7, .eint = -1 /* 7 */, .pressed = false},
    {.column = 2, .row = 7, .eint = -1 /* 7 */, .pressed = false},
    {.column = 1, .row = 7, .eint = -1 /* 7 */, .pressed = false},
    {.column = 0, .row = 6, .eint = -1 /* 6 */, .pressed = false},
};

static int x50ng_annunciators_index[ 6 ] = { 1, 2, 3, 4, 5, 0 };

/***********/
/* SD card */
/***********/
int emulator_mount_sd( char* filename ) { return s3c2410_sdi_mount( __hdw_state, strdup( filename ) ); }

void emulator_unmount_sd( void ) { s3c2410_sdi_unmount( __hdw_state ); }

bool emulator_is_sd_mounted( void ) { return s3c2410_sdi_is_mounted( __hdw_state ); }

void emulator_get_sd_path( char** filename ) { s3c2410_sdi_get_path( __hdw_state, filename ); }

/***********/
/* machine */
/***********/
void emulator_stop( void ) { hdw_stop( __hdw_state ); }

void emulator_sleep( void ) { hdw_set_idle( __hdw_state, HDW_ARM_SLEEP ); }

void emulator_wake( void ) { hdw_set_idle( __hdw_state, HDW_ARM_RUN ); }

void emulator_reset( void )
{
    reset_modules( __hdw_state, HDW_RESET_POWER_ON );
    cpu_reset( __hdw_state->env );
    emulator_wake();
}

/************/
/* keyboard */
/************/
static void set_key_state( const hp50g_key_t key, bool state )
{
    if ( key.eint >= 0 /* && key.row == 0 && key.column == 0 */ )
        s3c2410_io_port_f_set_bit( __hdw_state, key.eint, state );
    else
        s3c2410_io_port_g_update( __hdw_state, key.column, key.row, state );
}

void press_key( int hpkey )
{
    set_key_state( x50ng_keyboard[ hpkey ], true );
    x50ng_keyboard[ hpkey ].pressed = true;
}

void release_key( int hpkey )
{
    set_key_state( x50ng_keyboard[ hpkey ], false );
    x50ng_keyboard[ hpkey ].pressed = false;
}

bool is_key_pressed( int hpkey )
{
    if ( hpkey < 0 || hpkey > NB_HP50g_KEYS )
        return false;

    return x50ng_keyboard[ hpkey ].pressed;
}

/***********/
/* display */
/***********/
bool is_display_on( void )
{
    s3c2410_lcd_t* lcd = __hdw_state->s3c2410_lcd;

    return ( lcd->lcdcon1 & 1 );
}

unsigned char get_annunciators( void )
{
    s3c2410_lcd_t* lcd = __hdw_state->s3c2410_lcd;

    char annunciators = 0;

    for ( int i = 0; i < NB_ANNUNCIATORS; ++i )
        if ( s3c2410_get_pixel_color( lcd, LCD_WIDTH, x50ng_annunciators_index[ i ] ) > 0 )
            annunciators |= 0x01 << i;

    return annunciators;
}

int get_contrast( void ) { return 19; }

void get_lcd_buffer( int* target )
{
    s3c2410_lcd_t* lcd = __hdw_state->s3c2410_lcd;

    for ( int y = 0; y < LCD_HEIGHT; ++y )
        for ( int x = 0; x < LCD_WIDTH; ++x )
            target[ ( y * LCD_WIDTH ) + x ] = s3c2410_get_pixel_color( lcd, x, y );
}

/************/
/* debugger */
/************/
void emulator_debug( void )
{
    if ( __config->debug_port != 0 && !gdbserver_isactive() ) {
        gdbserver_start( __config->debug_port );
        gdb_handlesig( __hdw_state->env, 0 );
    }
}

/****************/
/* used in main */
/****************/
#define UI_EVENTS_REFRESH_INTERVAL 30000LL
#define UI_LCD_REFRESH_INTERVAL 50000LL

static void callback_handle_pending_inputs( void* data )
{
    hdw_t* hdw_state = data;

    ui_handle_pending_inputs();

    timer_mod( hdw_state->timer_ui_input, timer_get_clock() + UI_EVENTS_REFRESH_INTERVAL );
}

static void callback_refresh_output( void* data )
{
    hdw_t* hdw_state = data;

    ui_refresh_output();

    timer_mod( hdw_state->timer_ui_output, timer_get_clock() + UI_LCD_REFRESH_INTERVAL );
}

hdw_t* emulator_init( config_t* config )
{
    __config = config;
    __hdw_state = malloc( sizeof( hdw_t ) );
    if ( NULL == __hdw_state ) {
        fprintf( stderr, "%s: %s:%u: Out of memory\n", __config->progname, __func__, __LINE__ );
        exit( EXIT_FAILURE );
    }
    memset( __hdw_state, 0, sizeof( hdw_t ) );

#ifdef DEBUG_X50NG_MAIN
    fprintf( stderr, "_SC_PAGE_SIZE: %08lx\n", sysconf( _SC_PAGE_SIZE ) );

    printf( "%s:%u: x50ng: %p\n", __func__, __LINE__, __hdw_state );
#endif

    INIT_LIST_HEAD( &__hdw_state->modules );

    __hdw_state->clk_tck = sysconf( _SC_CLK_TCK );
    __hdw_state->emulator_fclk = 75000000;
    __hdw_state->PCLK_ratio = 4;
    __hdw_state->PCLK = 75000000 / 4;

    // cpu_set_log(0xffffffff);
    cpu_exec_init_all( 0 );
    __hdw_state->env = cpu_init( "arm926" );

    //	cpu_set_log(cpu_str_to_log_mask("all"));

    init_timer();

    __hdw_state->timer_ui_input = timer_new( HDW_TIMER_REALTIME, callback_handle_pending_inputs, __hdw_state );
    __hdw_state->timer_ui_output = timer_new( HDW_TIMER_VIRTUAL, callback_refresh_output, __hdw_state );

    init_s3c2410_arm( __hdw_state );
    init_flash( __hdw_state, config );
    init_sram( __hdw_state );
    init_s3c2410( __hdw_state );

    if ( init_modules( __hdw_state, config ) )
        exit( EXIT_FAILURE );

    int error = load_modules( __hdw_state );
    if ( error || __config->reinit >= HDW_REINIT_REBOOT_ONLY ) {
        if ( error && error != -EAGAIN )
            exit( EXIT_FAILURE );

        reset_modules( __hdw_state, HDW_RESET_POWER_ON );
    }

    emulator_wake();

    // stl_phys(0x08000a1c, 0x55555555);

    timer_mod( __hdw_state->timer_ui_input, timer_get_clock() );
    timer_mod( __hdw_state->timer_ui_output, timer_get_clock() );

    if ( __config->debug_port != 0 && __config->start_debugger ) {
        gdbserver_start( __config->debug_port );
        gdb_handlesig( __hdw_state->env, 0 );
    }

    if ( __config->sd_dir != NULL ) {
        if ( __config->verbose )
            fprintf( stderr, "> mounting --sd-dir %s\n", __config->sd_dir );
        // s3c2410_sdi_mount( __hdw_state, strdup( __config->sd_dir ) );
        emulator_mount_sd( __config->sd_dir );
    }

    return __hdw_state;
}

void emulator_exit( void )
{
    save_modules( __hdw_state );

    if ( !__config->haz_config_file )
        save_config();

    exit_modules( __hdw_state );
}
