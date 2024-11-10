#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <getopt.h>
#include <glib.h>

#include <lua.h>
#include <lauxlib.h>

#include "options.h"

#include "gdbstub.h"

struct options opt = {
    .config_lua_filename = NULL,
    .state_filename = NULL,
    .debug_port = 0,
    .start_debugger = false,
    .reinit = X49GP_REINIT_NONE,
    .firmware = NULL,
    .model = MODEL_50G,
    .name = NULL,
    .text_scale = 1,
    .display_scale = 2,
#if defined( __linux__ )
    .font = "urw gothic l",
#else
    .font = "Century Gothic",
#endif
};

lua_State* config_lua_values;

static inline bool config_read( const char* filename )
{
    int rc;

    assert( filename != NULL );

    /*---------------------------------------------------
    ; Create the Lua state, which includes NO predefined
    ; functions or values.  This is literally an empty
    ; slate.
    ;----------------------------------------------------*/
    config_lua_values = luaL_newstate();
    if ( config_lua_values == NULL ) {
        fprintf( stderr, "cannot create Lua state\n" );
        return false;
    }

    /*-----------------------------------------------------
    ; For the truly paranoid about sandboxing, enable the
    ; following code, which removes the string library,
    ; which some people find problematic to leave un-sand-
    ; boxed. But in my opinion, if you are worried about
    ; such attacks in a configuration file, you have bigger
    ; security issues to worry about than this.
    ;------------------------------------------------------*/
#ifdef PARANOID
    lua_pushliteral( config_lua_values, "x" );
    lua_pushnil( config_lua_values );
    lua_setmetatable( config_lua_values, -2 );
    lua_pop( config_lua_values, 1 );
#endif

    /*-----------------------------------------------------
    ; Lua 5.2+ can restrict scripts to being text only,
    ; to avoid a potential problem with loading pre-compiled
    ; Lua scripts that may have malformed Lua VM code that
    ; could possibly lead to an exploit, but again, if you
    ; have to worry about that, you have bigger security
    ; issues to worry about.  But in any case, here I'm
    ; restricting the file to "text" only.
    ;------------------------------------------------------*/
    rc = luaL_loadfile( config_lua_values, filename );
    if ( rc != LUA_OK ) {
        /* fprintf( stderr, "Lua error: (%d) %s\n", rc, lua_tostring( config_lua_values, -1 ) ); */
        return false;
    }

    rc = lua_pcall( config_lua_values, 0, 0, 0 );
    if ( rc != LUA_OK ) {
        /* fprintf( stderr, "Lua error: (%d) %s\n", rc, lua_tostring( config_lua_values, -1 ) ); */
        return false;
    }

    return true;
}

static void print_config( void )
{
    fprintf( stdout, "--------------------------------------------------------------------------------\n" );
    fprintf( stdout, "-- Configuration file for x49gpng\n" );
    fprintf( stdout, "-- This is a comment\n" );

    fprintf( stdout, "name = \"%s\"\n", opt.name );

    fprintf( stdout, "model = \"" );
    switch ( opt.model ) {
        case MODEL_49GP:
            fprintf( stdout, "49gp" );
            break;
        case MODEL_49GP_NEWRPL:
            fprintf( stdout, "49gp-newrpl" );
            break;
        case MODEL_50G:
            fprintf( stdout, "50g" );
            break;
        case MODEL_50G_NEWRPL:
            fprintf( stdout, "50g-newrpl" );
            break;
    }
    fprintf( stdout, "\" -- possible values: \"49gp\", \"50g\", \"49gp-newrpl\", \"50g-newrpl\"\n" );
    fprintf( stdout, "font = \"%s\"\n", opt.font );
    fprintf( stdout, "text_scale = %i\n", opt.text_scale );
    fprintf( stdout, "display_scale = %i\n", opt.display_scale );

    fprintf( stdout, "--- End of saturnng configuration ----------------------------------------------\n" );
}

void config_init( char* progname, int argc, char* argv[] )
{
    int option_index;
    int c = '?';

    char* config_lua_filename = ( char* )"config.lua";

    bool do_enable_debugger = false;
    bool do_start_debugger = false;
    bool do_reflash = false;
    bool do_reflash_full = false;

    char* clopt_name = NULL;
    char* clopt_font = NULL;
    int clopt_model = -1;
    int clopt_text_scale = -1;
    int clopt_display_scale = -1;

    int print_config_and_exit = false;

    const char* optstring = "hrc:D:df:Fn:t:";
    struct option long_options[] = {
        {"help",          no_argument,       NULL,                   'h' },
        {"print-config",  no_argument,       &print_config_and_exit, true},

        {"config",        required_argument, NULL,                   'c' },

        {"state",         required_argument, NULL,                   1   },

        {"enable-debug",  required_argument, NULL,                   'D' },
        {"debug",         no_argument,       NULL,                   'd' },
        {"reflash",       required_argument, NULL,                   'f' },
        {"reflash-full",  no_argument,       NULL,                   'F' },
        {"reboot",        no_argument,       NULL,                   'r' },

        {"50g",           no_argument,       NULL,                   506 },
        {"50g-newrpl",    no_argument,       NULL,                   507 },
        {"49gp",          no_argument,       NULL,                   496 },
        {"49gp-newrpl",   no_argument,       NULL,                   497 },
        {"name",          required_argument, NULL,                   'n' },
        {"text-scale",    required_argument, NULL,                   's' },
        {"display-scale", required_argument, NULL,                   'S' },

        {"font",          required_argument, NULL,                   't' },

        {0,               0,                 0,                      0   }
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
                         "    --state[=<filename>]     alternate config file\n"
                         "    --50g                     show HP 50g faceplate (default)\n"
                         "    --50g-newrpl              show HP 50g faceplate with newRPL labels\n"
                         "    --49gp                    show HP 49g+ faceplate\n"
                         "    --49gp-newrpl             show HP 49g+ faceplate with newRPL labels\n"
                         " -n --name[=<name>]           set alternate UI name\n"
                         " -t --font[=<fontname>]       set alternate UI font\n"
                         " -s --text-scale[=<X>]        scale text by X (default: 1)\n"
                         " -S --display-scale[=<X>]     scale LCD by X (default: 2)\n"
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
            case 1:
                opt.state_filename = strdup( optarg );
                break;
            case 'D':
                do_enable_debugger = true;
                opt.debug_port = atoi( optarg );
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
                clopt_model = MODEL_49GP;
                if ( clopt_name == NULL )
                    clopt_name = "HP 49g+";
                break;
            case 497:
                clopt_model = MODEL_49GP_NEWRPL;
                if ( clopt_name == NULL )
                    clopt_name = "HP 49g+ / newRPL";
                break;
            case 506:
                clopt_model = MODEL_50G;
                if ( clopt_name == NULL )
                    clopt_name = "HP 50g";
                break;
            case 507:
                clopt_model = MODEL_50G_NEWRPL;
                if ( clopt_name == NULL )
                    clopt_name = "HP 50g / newRPL";
                break;
            case 'n':
                clopt_name = strdup( optarg );
                break;
            case 's':
                clopt_text_scale = atoi( optarg );
                break;
            case 'S':
                clopt_display_scale = atoi( optarg );
                break;
            case 't':
                clopt_font = strdup( optarg );
                break;
            default:
                break;
        }
    }

    char config_dir[ strlen( progname ) + 9 ];
    const char* home = g_get_home_dir();
    sprintf( config_dir, ".config/%s", progname );

    opt.config_lua_filename = g_build_filename( home, config_dir, config_lua_filename, NULL );

    /**********************/
    /* 1. read config.lua */
    /**********************/
    bool haz_config_file = config_read( opt.config_lua_filename );
    if ( haz_config_file ) {
        lua_getglobal( config_lua_values, "model" );
        const char* svalue_model = luaL_optstring( config_lua_values, -1, "50g" );
        if ( svalue_model != NULL ) {
            if ( strcmp( svalue_model, "50g" ) == 0 )
                opt.model = MODEL_50G;
            if ( strcmp( svalue_model, "50g-newrpl" ) == 0 )
                opt.model = MODEL_50G_NEWRPL;
            if ( strcmp( svalue_model, "49gp" ) == 0 )
                opt.model = MODEL_49GP;
            if ( strcmp( svalue_model, "49gp-newrpl" ) == 0 )
                opt.model = MODEL_49GP_NEWRPL;

            switch ( opt.model ) {
                case MODEL_50G_NEWRPL:
                    opt.name = "HP 50g / newRPL";
                    break;
                case MODEL_49GP:
                    opt.name = "HP 49g+";
                    break;
                case MODEL_49GP_NEWRPL:
                    opt.name = "HP 49g+ / newRPL";
                    break;
                case MODEL_50G:
                default:
                    opt.name = "HP 50g";
                    break;
            }
        }

        lua_getglobal( config_lua_values, "name" );
        opt.name = strdup( luaL_optstring( config_lua_values, -1, opt.name ) );

        lua_getglobal( config_lua_values, "font" );
        opt.font = strdup( luaL_optstring( config_lua_values, -1, opt.font ) );

        lua_getglobal( config_lua_values, "text_scale" );
        opt.text_scale = luaL_optinteger( config_lua_values, -1, 1 );

        lua_getglobal( config_lua_values, "display_scale" );
        opt.display_scale = luaL_optinteger( config_lua_values, -1, 2 );
    }

    /****************************************************/
    /* 2. treat command-line params which have priority */
    /****************************************************/
    if ( clopt_name != NULL )
        opt.name = strdup( clopt_name );
    if ( clopt_font != NULL )
        opt.font = strdup( clopt_font );
    if ( clopt_model != -1 )
        opt.model = clopt_model;
    if ( clopt_text_scale != -1 )
        opt.text_scale = clopt_text_scale;
    if ( clopt_display_scale != -1 )
        opt.display_scale = clopt_display_scale;

    if ( print_config_and_exit ) {
        print_config();
        exit( EXIT_SUCCESS );
    }

    if ( !haz_config_file ) {
        fprintf( stderr, "\nConfiguration file %s doesn't seem to exist or is invalid!\n", opt.config_lua_filename );
        fprintf( stderr, "You can solve this by running `mkdir -p %s/%s && %s --print-config >> %s`\n\n", home, config_dir, progname,
                 opt.config_lua_filename );
    }

    if ( do_enable_debugger ) {
        if ( opt.debug_port == 0 )
            opt.debug_port = DEFAULT_GDBSTUB_PORT;

        opt.start_debugger = do_start_debugger;
    }
    if ( do_reflash ) {
        if ( opt.reinit < X49GP_REINIT_FLASH )
            opt.reinit = X49GP_REINIT_FLASH;

        if ( opt.firmware != NULL )
            fprintf( stderr, "Additional firmware file \"%s\" specified, overriding\n", optarg );
        opt.firmware = optarg;

        if ( do_reflash_full )
            opt.reinit = X49GP_REINIT_FLASH_FULL;
    }

    if ( opt.state_filename == NULL )
        opt.state_filename = g_build_filename( home, config_dir, "state", NULL );
}
