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

extern options_t opt;

extern x50ng_t* x50ng;

/*******************/
/* signal handlers */
/*******************/
void signal_handler( int sig )
{
    switch ( sig ) {
        case SIGINT:
        case SIGQUIT:
        case SIGTERM:
            x50ng->arm_exit = 1;
            cpu_exit( x50ng->env );
            break;
        case SIGUSR1:
            //		stop_simulator = 1;
            //		x50ng->arm->CallDebug ^= 1;
            break;
        default:
            fprintf( stderr, "%s: sig %u\n", __func__, sig );
            break;
    }
}

int main( int argc, char** argv )
{
    config_init( argc, argv ); /* initialize global variable `opt` */

    emulator_init(); /* initialize global variable `x50ng` */

    signal( SIGINT, signal_handler );
    signal( SIGTERM, signal_handler );
    signal( SIGQUIT, signal_handler );
    signal( SIGUSR1, signal_handler );

    ui_init( x50ng );

    x50ng_main_loop( x50ng );

    ui_exit();

    emulator_exit();

    return EXIT_SUCCESS;
}
