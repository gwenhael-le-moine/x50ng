#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <getopt.h>
#include <glib.h>

#include <lua.h>
#include <lauxlib.h>

#include "options.h"

#include "gdbstub.h"

#define CONFIG_LUA_FILE_NAME "config.lua"

struct options opt = {
    .datadir = NULL,

    .debug_port = 0,
    .start_debugger = false,
    .reinit = X49GP_REINIT_NONE,
    .firmware = NULL,
    .model = MODEL_50G,
    .newrpl = false,
    .name = NULL,
    .verbose = false,
    .style_filename = NULL,
    .display_scale = 2,
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

static char* config_to_string( void )
{
    char* config;
    asprintf( &config,
              "--------------------------------------------------------------------------------\n"
              "-- Configuration file for x50ng\n"
              "-- This is a comment\n"
              "name = \"%s\" -- this customize the title of the window\n"
              "model = \"%s\" -- possible values: \"49gp\", \"50g\". Changes the bootloader looked for when (re-)flashing\n"
              "newrpl_keyboard = %s -- when true this makes the keyboard labels more suited to newRPL use\n"
              "style = \"%s\" -- CSS file (relative to this file)\n"
              "display_scale = %i -- integer only\n"
              "verbose = %s\n"
              "--- End of x50ng configuration -----------------------------------------------\n",
              opt.name, opt.model == MODEL_50G ? "50g" : "49gp", opt.newrpl ? "true" : "false", opt.style_filename, opt.display_scale,
              opt.verbose ? "true" : "false" );

    return config;
}

int save_config( void )
{
    int error;
    const char* config_lua_filename = g_build_filename( opt.datadir, CONFIG_LUA_FILE_NAME, NULL );
    int fd = open( config_lua_filename, O_WRONLY | O_CREAT | O_TRUNC, 0644 );

    if ( fd < 0 ) {
        error = -errno;
        fprintf( stderr, "%s:%u: open %s: %s\n", __FUNCTION__, __LINE__, config_lua_filename, strerror( errno ) );
        return error;
    }

    char* data = config_to_string();
    if ( write( fd, data, strlen( data ) ) != strlen( data ) ) {
        error = -errno;
        fprintf( stderr, "%s:%u: write %s: %s\n", __FUNCTION__, __LINE__, config_lua_filename, strerror( errno ) );
        close( fd );
        g_free( data );
        return error;
    }

    close( fd );
    g_free( data );

    if ( opt.verbose )
        fprintf( stderr, "Current configuration has been written to %s\n", config_lua_filename );

    return EXIT_SUCCESS;
}

void config_init( char* progname, int argc, char* argv[] )
{
    int option_index;
    int c = '?';

    bool do_enable_debugger = false;
    bool do_start_debugger = false;
    bool do_reflash = false;
    bool do_reflash_full = false;

    int clopt_verbose = -1;

    char* clopt_name = NULL;
    char* clopt_style_filename = NULL;
    int clopt_model = -1;
    int clopt_newrpl = -1;
    int clopt_display_scale = -1;

    int print_config_and_exit = false;
    int overwrite_config = false;

    const char* optstring = "dhrf:n:s:S:vV";
    struct option long_options[] = {
        {"help",             no_argument,       NULL,                   'h' },
        {"version",          no_argument,       NULL,                   'v' },
        {"verbose",          no_argument,       NULL,                   'V' },

        {"print-config",     no_argument,       &print_config_and_exit, true},
        {"overwrite-config", no_argument,       &overwrite_config,      true},
        {"datadir",          required_argument, NULL,                   'd' },

        {"50g",              no_argument,       NULL,                   506 },
        {"49gp",             no_argument,       NULL,                   496 },
        {"newrpl-keyboard",  no_argument,       &clopt_newrpl,          true},
        {"name",             required_argument, NULL,                   'n' },
        {"style",            required_argument, NULL,                   's' },
        {"display-scale",    required_argument, NULL,                   'S' },

        {"enable-debug",     required_argument, NULL,                   10  },
        {"debug",            no_argument,       NULL,                   11  },
        {"reflash",          required_argument, NULL,                   90  },
        {"reflash-full",     required_argument, NULL,                   91  },
        {"reboot",           no_argument,       NULL,                   'r' },

        {0,                  0,                 0,                      0   }
    };

    while ( c != EOF ) {
        c = getopt_long( argc, argv, optstring, long_options, &option_index );

        switch ( c ) {
            case 'h':
                fprintf( stderr,
                         "%s %i.%i.%i\n"
                         "Emulator for HP 49G+ / 50G calculators' hardware\n"
                         "Usage: %s [<options>]\n"
                         "Valid options:\n"
                         "-h --help                    print this message and exit\n"
                         "-v --version                 print out version\n"
                         "-V --verbose                 print out more information\n"
                         "-d --datadir[=absolute path] alternate datadir (default: $XDGCONFIGHOME/%s/)\n"
                         "-n --name[=name]             set alternate UI name\n"
                         "-s --style[=filename]        css filename in <datadir> (default: style-50g.css)\n"
                         "-S --display-scale[=X]       scale LCD by X (default: 2)\n"
                         "-r --reboot                  reboot on startup instead of continuing from the saved state in the state file\n"
                         "--overwrite-config           force writing <datadir>/config.lua even if it exists\n"
                         "\n"
                         "--newrpl-keyboard            label keyboard for newRPL\n"
                         "\n"
                         "--enable-debug[=port]        enable the debugger interface (default port: %i)\n"
                         "--debug                      use along -D to also start the debugger immediately\n"
                         "\n"
                         "--reflash[=firmware]         rebuild the flash using the supplied firmware (default: select one interactively) "
                         "(implies -r for safety reasons)\n"
                         "--reflash-full[=firmware]    rebuild the flash using the supplied firmware and drop the flash contents in the "
                         "area beyond the firmware (default: select one interactively) (implies -r for safety reasons)\n"
                         "--50g                        use an HP 50g bootloader when (re-)flashing (default)\n"
                         "--49gp                       use an HP 49g+ bootloader when (re-)flashing\n",
                         progname, VERSION_MAJOR, VERSION_MINOR, PATCHLEVEL, progname, progname, DEFAULT_GDBSTUB_PORT );
                exit( EXIT_SUCCESS );
                break;
            case 10:
                do_enable_debugger = true;
                opt.debug_port = atoi( optarg );
                break;
            case 11:
                do_start_debugger = true;
                break;
            case 90:
                do_reflash = true;
                opt.firmware = strdup( optarg );
                break;
            case 91:
                do_reflash = true;
                do_reflash_full = true;
                opt.firmware = strdup( optarg );
                break;
            case 496:
                clopt_model = MODEL_49GP;
                if ( clopt_name == NULL )
                    clopt_name = "HP 49g+";
                if ( clopt_style_filename == NULL )
                    clopt_style_filename = "style-49gp.css";
                break;
            case 506:
                clopt_model = MODEL_50G;
                if ( clopt_name == NULL )
                    clopt_name = "HP 50g";
                if ( clopt_style_filename == NULL )
                    clopt_style_filename = "style-50g.css";
                break;
            case 'd':
                opt.datadir = strdup( optarg );
                break;
            case 'n':
                clopt_name = strdup( optarg );
                break;
            case 'r':
                if ( opt.reinit < X49GP_REINIT_REBOOT_ONLY )
                    opt.reinit = X49GP_REINIT_REBOOT_ONLY;
                break;
            case 's':
                clopt_style_filename = strdup( optarg );
                break;
            case 'S':
                clopt_display_scale = atoi( optarg );
                break;
            case 'v':
                fprintf( stderr, "%i.%i.%i\n", VERSION_MAJOR, VERSION_MINOR, PATCHLEVEL );
                exit( EXIT_SUCCESS );
                break;
            case 'V':
                clopt_verbose = true;
                break;

            default:
                break;
        }
    }

    if ( opt.datadir == NULL )
        opt.datadir = g_build_filename( g_get_user_config_dir(), progname, NULL );

    const char* config_lua_filename = g_build_filename( opt.datadir, CONFIG_LUA_FILE_NAME, NULL );

    /**********************/
    /* 1. read config.lua */
    /**********************/
    opt.haz_config_file = config_read( config_lua_filename );
    if ( opt.haz_config_file ) {
        lua_getglobal( config_lua_values, "newrpl_keyboard" );
        opt.newrpl = lua_toboolean( config_lua_values, -1 );

        lua_getglobal( config_lua_values, "model" );
        const char* svalue_model = luaL_optstring( config_lua_values, -1, "50g" );
        if ( svalue_model != NULL ) {
            if ( strcmp( svalue_model, "50g" ) == 0 )
                opt.model = MODEL_50G;
            if ( strcmp( svalue_model, "49gp" ) == 0 )
                opt.model = MODEL_49GP;

            switch ( opt.model ) {
                case MODEL_49GP:
                    opt.name = opt.newrpl ? "HP 49g+ / newRPL" : "HP 49g+";
                    opt.style_filename = "style-49gp.css";
                    break;
                case MODEL_50G:
                default:
                    opt.name = opt.newrpl ? "HP 50g / newRPL" : "HP 50g";
                    opt.style_filename = "style-50g.css";
                    break;
            }
        }

        lua_getglobal( config_lua_values, "name" );
        opt.name = strdup( luaL_optstring( config_lua_values, -1, opt.name ) );

        lua_getglobal( config_lua_values, "style" );
        opt.style_filename = strdup( luaL_optstring( config_lua_values, -1, opt.style_filename ) );

        lua_getglobal( config_lua_values, "display_scale" );
        opt.display_scale = luaL_optinteger( config_lua_values, -1, opt.display_scale );
    }
    if ( opt.haz_config_file && overwrite_config )
        opt.haz_config_file = false;

    /****************************************************/
    /* 2. treat command-line params which have priority */
    /****************************************************/
    if ( clopt_verbose != -1 )
        opt.verbose = clopt_verbose;
    if ( clopt_name != NULL )
        opt.name = strdup( clopt_name );
    if ( clopt_style_filename != NULL )
        opt.style_filename = strdup( clopt_style_filename );
    if ( clopt_model != -1 )
        opt.model = clopt_model;
    if ( clopt_newrpl != -1 )
        opt.newrpl = clopt_newrpl;
    if ( clopt_display_scale > 0 )
        opt.display_scale = clopt_display_scale;

    if ( print_config_and_exit ) {
        fprintf( stdout, config_to_string() );
        exit( EXIT_SUCCESS );
    }
    if ( opt.verbose )
        fprintf( stdout, config_to_string() );

    if ( do_enable_debugger ) {
        if ( opt.debug_port == 0 )
            opt.debug_port = DEFAULT_GDBSTUB_PORT;

        opt.start_debugger = do_start_debugger;
    }
    if ( do_reflash ) {
        if ( opt.reinit < X49GP_REINIT_FLASH )
            opt.reinit = X49GP_REINIT_FLASH;

        if ( do_reflash_full )
            opt.reinit = X49GP_REINIT_FLASH_FULL;
    }
}
