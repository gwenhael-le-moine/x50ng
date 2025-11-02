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

#include "ui/common.h"
#include "ui/api.h"

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
    config_t* __config = config_init( argc, argv );

    __hdw_state = emulator_init( __config );

    signal( SIGINT, signal_handler );
    signal( SIGTERM, signal_handler );
    signal( SIGQUIT, signal_handler );
    signal( SIGUSR1, signal_handler );

    ui_init( __hdw_state, __config );

    main_loop( __hdw_state );

    ui_exit();

    emulator_exit();

    return EXIT_SUCCESS;
}
