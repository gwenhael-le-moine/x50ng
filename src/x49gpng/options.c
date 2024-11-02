#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <getopt.h>
#include <glib.h>

#include "options.h"

#include "gdbstub.h"

struct options opt;

void config_init( char* progname, int argc, char* argv[] )
{
    int option_index;
    int c = '?';

    bool do_enable_debugger = false;
    bool do_start_debugger = false;
    bool do_reflash = false;
    bool do_reflash_full = false;

    opt.config = NULL;
    opt.debug_port = 0;
    opt.start_debugger = false;
    opt.reinit = X49GP_REINIT_NONE;
    opt.firmware = NULL;
    opt.model = MODEL_50G;
    opt.name = NULL;
    opt.scale = 1.0;

#if defined( __linux__ )
    opt.font = "urw gothic l";
#else
    opt.font = "Century Gothic";
#endif

    const char* optstring = "hrc:D:df:Fn:t:";
    struct option long_options[] = {
        {"help",         no_argument,       NULL, 'h'},

        {"config",       required_argument, NULL, 'c'},

        {"enable-debug", required_argument, NULL, 'D'},
        {"debug",        no_argument,       NULL, 'd'},
        {"reflash",      required_argument, NULL, 'f'},
        {"reflash-full", no_argument,       NULL, 'F'},
        {"reboot",       no_argument,       NULL, 'r'},

        {"50g",          no_argument,       NULL, 506},
        {"50g-newrpl",   no_argument,       NULL, 507},
        {"49gp",         no_argument,       NULL, 496},
        {"49gp-newrpl",  no_argument,       NULL, 497},
        {"name",         required_argument, NULL, 'n'},
        {"scale",        required_argument, NULL, 's'},

        {"font",         required_argument, NULL, 't'},

        {0,              0,                 0,    0  }
    };

    while ( c != EOF ) {
        c = getopt_long( argc, argv, optstring, long_options, &option_index );

        switch ( c ) {
            case 'h':
                fprintf( stderr,
                         "%s %i.%i.%i Emulator for HP 49G+ / 50G calculators\n"
                         "Usage: %s [<options>]\n"
                         "Valid options:\n"
                         " -h --help                    print this message and exit\n"
                         " -c --config[=<filename>]     alternate config file\n"
                         "    --50g                     show HP 50g faceplate (default)\n"
                         "    --50g-newrpl              show HP 50g faceplate with newRPL labels\n"
                         "    --49gp                    show HP 49g+ faceplate\n"
                         "    --49gp-newrpl             show HP 49g+ faceplate with newRPL labels\n"
                         " -n --name[=<name>]           set alternate UI name\n"
                         " -t --font[=<fontname>]       set alternate UI font\n"
                         " -s --scale[=<X.x>]           scale GUI by X.x\n"
                         " -D --enable-debug[=<port>]   enable the debugger interface\n"
                         "                              (default port: %u)\n"
                         " -d --debug                   use along -D to also start the debugger immediately\n"
                         " -f --reflash[=firmware]      rebuild the flash using the supplied firmware\n"
                         "                              (default: select one interactively)\n"
                         "                              (implies -r for safety reasons)\n"
                         " -F --reflash-full            use along -f to drop the flash contents\n"
                         "                              in the area beyond the firmware\n"
                         " -r --reboot                  reboot on startup instead of continuing from the\n"
                         "                              saved state in the config file\n"
                         "The config file is formatted as INI file and contains the settings for which\n"
                         "  persistence makes sense, like calculator model, CPU registers, etc.\n"
                         "If the config file is omitted, ~/.config/%s/config is used.\n"
                         "Please consult the manual for more details on config file settings.\n",
                         progname, VERSION_MAJOR, VERSION_MINOR, PATCHLEVEL, progname, DEFAULT_GDBSTUB_PORT, progname );
                exit( EXIT_SUCCESS );
                break;
            case 'r':
                if ( opt.reinit < X49GP_REINIT_REBOOT_ONLY )
                    opt.reinit = X49GP_REINIT_REBOOT_ONLY;
                break;
            case 'c':
                opt.config = strdup( optarg );
                break;
            case 'D':
                do_enable_debugger = true;
                break;
            case 'd':
                do_start_debugger = true;
                break;
            case 'f':
                do_reflash = true;
                break;
            case 'F':
                do_reflash_full = true;
                break;
            case 496:
                opt.model = MODEL_49GP;
                break;
            case 497:
                opt.model = MODEL_49GP_NEWRPL;
                break;
            case 506:
                opt.model = MODEL_50G;
                break;
            case 507:
                opt.model = MODEL_50G_NEWRPL;
                break;
            case 'n':
                opt.name = strdup( optarg );
                break;
            case 's':
                opt.scale = atof( optarg );
                break;
            case 't':
                opt.font = strdup( optarg );
                break;
            default:
                break;
        }
    }

    if ( do_enable_debugger ) {
        char* end;
        int port;

        if ( optarg == NULL && opt.debug_port == 0 )
            opt.debug_port = DEFAULT_GDBSTUB_PORT;

        port = strtoul( optarg, &end, 0 );
        if ( ( end == optarg ) || ( *end != '\0' ) ) {
            fprintf( stderr, "Invalid port \"%s\", using default\n", optarg );
            if ( opt.debug_port == 0 )
                opt.debug_port = DEFAULT_GDBSTUB_PORT;
        }

        if ( opt.debug_port != 0 && opt.debug_port != DEFAULT_GDBSTUB_PORT )
            fprintf( stderr, "Additional debug port \"%s\" specified, overriding\n", optarg );
        opt.debug_port = port;

        opt.start_debugger = do_start_debugger;
    }
    if ( do_reflash ) {
        if ( opt.reinit < X49GP_REINIT_FLASH )
            opt.reinit = X49GP_REINIT_FLASH;

        if ( opt.firmware != NULL )
            fprintf( stderr,
                     "Additional firmware file \"%s\" specified,"
                     " overriding\n",
                     optarg );
        opt.firmware = optarg;

        if ( do_reflash_full )
            opt.reinit = X49GP_REINIT_FLASH_FULL;
    }

    if ( opt.config == NULL ) {
        char config_dir[ strlen( progname ) + 9 ];

        const char* home = g_get_home_dir();
        sprintf( config_dir, ".config/%s", progname );
        opt.config = g_build_filename( home, config_dir, "config", NULL );
    }
}
