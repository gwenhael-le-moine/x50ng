#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/time.h>

#include <getopt.h>

#include <gtk/gtk.h>
#include <glib.h>

#include <memory.h>

#include "ui4x/api.h"

#include "emulator_api.h"
#include "hdw.h"
#include "timer.h"
#include "options.h"

static hdw_t* __hdw_state;

/*******************/
/* signal handlers */
/*******************/
void signal_handler( int sig )
{
    switch ( sig ) {
        case SIGINT:
        case SIGQUIT:
        case SIGTERM:
            hdw_stop( __hdw_state );
            break;
        case SIGUSR1:
            //		stop_simulator = 1;
            //		__hdw_state->arm->CallDebug ^= 1;
            break;
        default:
            fprintf( stderr, "%s: sig %u\n", __func__, sig );
            break;
    }
}

int main( int argc, char** argv )
{
    config_t __config = *config_init( argc, argv );

    __hdw_state = emulator_init( &__config );

    signal( SIGINT, signal_handler );
    signal( SIGTERM, signal_handler );
    signal( SIGQUIT, signal_handler );
    signal( SIGUSR1, signal_handler );

    ui4x_config_t config_ui = {
        .model = MODEL_50G,
        .shiftless = false,
        .big_screen = true,
        .black_lcd = true,

        .frontend = __config.frontend,
        .newrpl_keyboard = __config.newrpl_keyboard,

        .mono = false,
        .gray = false,

        .chromeless = false,
        .fullscreen = false,
        .zoom = __config.zoom,

        .tiny = __config.tiny,
        .small = __config.small,

        .netbook = __config.netbook,
        .netbook_pivot_line = __config.netbook_pivot_line,

        .verbose = __config.verbose,

        .name = __config.name,
        .progname = __config.progname,
        .progpath = __config.progpath,
        .wire_name = NULL,
        .ir_name = NULL,

        .datadir = __config.datadir,
        .style_filename = __config.style_filename,

        .sd_dir = __config.sd_dir,
    };
    ui4x_emulator_api_t emulator_api = { .press_key = press_key,
                                         .release_key = release_key,
                                         .is_key_pressed = is_key_pressed,
                                         .is_display_on = is_display_on,
                                         .get_annunciators = get_annunciators,
                                         .get_lcd_buffer = get_lcd_buffer,
                                         .get_contrast = get_contrast,
                                         .do_mount_sd = emulator_mount_sd,
                                         .do_unmount_sd = emulator_unmount_sd,
                                         .is_sd_mounted = emulator_is_sd_mounted,
                                         .get_sd_path = emulator_get_sd_path,
                                         .do_reset = emulator_reset,
                                         .do_stop = emulator_stop,
                                         .do_sleep = emulator_sleep,
                                         .do_wake = emulator_wake,
                                         .do_debug = emulator_debug };
    ui_init( &config_ui, &emulator_api );

    main_loop( __hdw_state );

    ui_exit();

    emulator_exit();

    return EXIT_SUCCESS;
}
