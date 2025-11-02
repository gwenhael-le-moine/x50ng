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

#include "ui/ui.h"

#include "timer.h"
#include "options.h"

#include "emulator.h"

config_t opt;

static x50ng_t* hdw_state;

/*******************/
/* signal handlers */
/*******************/
void signal_handler( int sig )
{
    switch ( sig ) {
        case SIGINT:
        case SIGQUIT:
        case SIGTERM:
            hdw_state->arm_exit = 1;
            cpu_exit( hdw_state->env );
            break;
        case SIGUSR1:
            //		stop_simulator = 1;
            //		hdw_state->arm->CallDebug ^= 1;
            break;
        default:
            fprintf( stderr, "%s: sig %u\n", __func__, sig );
            break;
    }
}

int main( int argc, char** argv )
{
    opt = *config_init( argc, argv ); /* initialize global variable `opt` */

    hdw_state = emulator_init( opt ); /* initialize global variable `hdw_state` */

    signal( SIGINT, signal_handler );
    signal( SIGTERM, signal_handler );
    signal( SIGQUIT, signal_handler );
    signal( SIGUSR1, signal_handler );

    ui_init( hdw_state );

    x50ng_main_loop( hdw_state );

    ui_exit();

    emulator_exit( opt );

    return EXIT_SUCCESS;
}
