#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <assert.h>
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
    .bootloader = "firmware/boot-50g.bin",
    .firmware = "firmware/hp4950v215/2MB_FIX/2MB_215f.bin",
    .newrpl = false,
    .legacy_keyboard = false,
    .name = NULL,
    .verbose = false,
    .style_filename = NULL,
    .zoom = 2,
    .gray = false,
    .netbook = false,
    .netbook_pivot_line = 3,
};

static lua_State* config_lua_values;

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
              "style = \"%s\" -- CSS file (relative to this file)\n"
              "zoom = %i -- integer only\n"
              "gray = %s\n"
              "netbook = %s\n"
              "netbook-pivot-line = %i -- this marks the transition between higher and lower keyboard\n"
              "newrpl_keyboard = %s -- when true this makes the keyboard labels more suited to newRPL use\n"
              "legacy_keyboard = %s -- when true this put the Enter key where it belongs\n"
              "--- End of x50ng configuration -----------------------------------------------\n",
              opt.style_filename, opt.zoom, opt.gray ? "true" : "false", opt.netbook ? "true" : "false", opt.netbook_pivot_line,
              opt.newrpl ? "true" : "false", opt.legacy_keyboard ? "true" : "false" );

    return config;
}

int save_config( void )
{
    int error;
    const char* config_lua_filename = g_build_filename( opt.datadir, CONFIG_LUA_FILE_NAME, NULL );
    fprintf( stderr, "Loading configuration file: %s\n", config_lua_filename );
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
    bool do_flash = false;
    bool do_flash_full = false;

    char* clopt_style_filename = NULL;
    int clopt_newrpl = -1;
    int clopt_legacy_keyboard = -1;
    int clopt_zoom = -1;
    int clopt_gray = -1;
    int clopt_netbook = -1;
    int clopt_netbook_pivot_line = -1;

    int print_config_and_exit = false;
    int overwrite_config = false;

    asprintf( &( opt.name ), "%s %i.%i.%i", progname, VERSION_MAJOR, VERSION_MINOR, PATCHLEVEL );

    const char* optstring = "d:hrs:vVz:";
    struct option long_options[] = {
        {"help",               no_argument,       NULL,                   'h' },
        {"version",            no_argument,       NULL,                   'v' },
        {"verbose",            no_argument,       NULL,                   'V' },

        {"print-config",       no_argument,       &print_config_and_exit, true},
        {"overwrite-config",   no_argument,       &overwrite_config,      true},
        {"datadir",            required_argument, NULL,                   'd' },

        {"50g",                no_argument,       NULL,                   506 },
        {"49gp",               no_argument,       NULL,                   496 },
        {"newrpl-keyboard",    no_argument,       &clopt_newrpl,          true},
        {"legacy-keyboard",    no_argument,       &clopt_legacy_keyboard, true},
        {"style",              required_argument, NULL,                   's' },
        {"zoom",               required_argument, NULL,                   'z' },
        {"gray",               no_argument,       &clopt_gray,            true},
        {"netbook",            no_argument,       &clopt_netbook,         true},
        {"netbook-pivot-line", required_argument, NULL,                   1001},

        {"enable-debug",       required_argument, NULL,                   10  },
        {"debug",              no_argument,       NULL,                   11  },
        {"flash",              no_argument,       NULL,                   90  },
        {"flash-full",         no_argument,       NULL,                   91  },
        {"bootloader",         required_argument, NULL,                   92  },
        {"firmware",           required_argument, NULL,                   93  },
        {"reset",              no_argument,       NULL,                   'r' },

        {0,                    0,                 0,                      0   }
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
                         "-d --datadir[=absolute path] alternate datadir (default: $XDG_CONFIG_HOME/%s/)\n"
                         "-r --reset                   reboot on startup instead of continuing from the saved state in the state file\n"
                         "--overwrite-config           force writing <datadir>/config.lua even if it exists\n"
                         "\n"
                         "-s --style[=filename]        css filename in <datadir> (default: style-50g.css)\n"
                         "-z --zoom[=X]                scale LCD by X (default: 2)\n"
                         "--gray                       grayish LCD instead of greenish (default: false)\n"
                         "--netbook                    horizontal window (default: false)\n"
                         "--netbook-pivot-line         at which line is the keyboard split in netbook mode (default: 3)\n"
                         "--newrpl-keyboard            label keyboard for newRPL\n"
                         "--legacy-keyboard            place Enter key where it belongs\n"
                         "\n"
                         "--enable-debug[=port]        enable the debugger interface (default port: %i)\n"
                         "--debug                      use along -D to also start the debugger immediately\n"
                         "\n"
                         "--flash         rebuild the flash using the supplied firmware (requires --firmware=) "
                         "(implies -r for safety reasons)\n"
                         "--flash-full    rebuild the flash using the supplied firmware and drop the flash contents in the "
                         "area beyond the firmware (requires --firmware=) (implies -r for safety reasons)\n"
                         "--bootloader[=filename]         bootloader file (default: %s)\n"
                         "--firmware[=filename]         firmware file (default: %s)\n",
                         progname, VERSION_MAJOR, VERSION_MINOR, PATCHLEVEL, progname, progname, DEFAULT_GDBSTUB_PORT, opt.bootloader,
                         opt.firmware );
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
                do_flash = true;
                break;
            case 91:
                do_flash = true;
                do_flash_full = true;
                break;
            case 92:
                opt.bootloader = strdup( optarg );
                break;
            case 93:
                opt.firmware = strdup( optarg );
                break;
            case 1001:
                clopt_netbook_pivot_line = atoi( optarg );
                break;
            case 'd':
                opt.datadir = strdup( optarg );
                break;
            case 'r':
                if ( opt.reinit < X49GP_REINIT_REBOOT_ONLY )
                    opt.reinit = X49GP_REINIT_REBOOT_ONLY;
                break;
            case 's':
                clopt_style_filename = strdup( optarg );
                break;
            case 'S':
            case 'z':
                clopt_zoom = atoi( optarg );
                break;
            case 'v':
                fprintf( stderr, "%i.%i.%i\n", VERSION_MAJOR, VERSION_MINOR, PATCHLEVEL );
                exit( EXIT_SUCCESS );
                break;
            case 'V':
                opt.verbose = true;
                break;

            default:
                break;
        }
    }

    if ( do_flash && ( opt.bootloader == NULL || opt.firmware == NULL ) ) {
        fprintf(
            stderr,
            "Error: --flash(-full) requires you to provide a bootloader and firmware using --bootloader and --firmware respectively!\n" );
        exit( EXIT_FAILURE );
    }

    if ( opt.datadir == NULL )
        opt.datadir = g_build_filename( g_get_user_config_dir(), progname, NULL );

    const char* config_lua_filename = g_build_filename( opt.datadir, CONFIG_LUA_FILE_NAME, NULL );

    /**********************/
    /* 1. read config.lua */
    /**********************/
    opt.haz_config_file = config_read( config_lua_filename );
    if ( opt.haz_config_file ) {
        lua_getglobal( config_lua_values, "style" );
        opt.style_filename = strdup( luaL_optstring( config_lua_values, -1, opt.style_filename ) );

        lua_getglobal( config_lua_values, "zoom" );
        opt.zoom = luaL_optinteger( config_lua_values, -1, opt.zoom );

        lua_getglobal( config_lua_values, "gray" );
        opt.gray = lua_toboolean( config_lua_values, -1 );

        lua_getglobal( config_lua_values, "netbook" );
        opt.netbook = lua_toboolean( config_lua_values, -1 );

        lua_getglobal( config_lua_values, "netbook-pivot-line" );
        opt.netbook_pivot_line = luaL_optinteger( config_lua_values, -1, opt.netbook_pivot_line );

        lua_getglobal( config_lua_values, "newrpl_keyboard" );
        opt.newrpl = lua_toboolean( config_lua_values, -1 );

        lua_getglobal( config_lua_values, "legacy_keyboard" );
        opt.legacy_keyboard = lua_toboolean( config_lua_values, -1 );
    }
    if ( opt.haz_config_file && overwrite_config )
        opt.haz_config_file = false;

    /****************************************************/
    /* 2. treat command-line params which have priority */
    /****************************************************/
    if ( clopt_style_filename != NULL )
        opt.style_filename = strdup( clopt_style_filename );
    else if ( opt.style_filename == NULL )
        opt.style_filename = "style-50g.css";
    if ( clopt_newrpl != -1 )
        opt.newrpl = clopt_newrpl;
    if ( clopt_legacy_keyboard != -1 )
        opt.legacy_keyboard = clopt_legacy_keyboard;
    if ( clopt_zoom > 0 )
        opt.zoom = clopt_zoom;
    if ( clopt_gray != -1 )
        opt.gray = clopt_gray;
    if ( clopt_netbook != -1 )
        opt.netbook = clopt_netbook;
    if ( clopt_netbook_pivot_line != -1 )
        opt.netbook_pivot_line = clopt_netbook_pivot_line;

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
    if ( do_flash ) {
        if ( opt.reinit < X49GP_REINIT_FLASH )
            opt.reinit = X49GP_REINIT_FLASH;

        if ( do_flash_full )
            opt.reinit = X49GP_REINIT_FLASH_FULL;
    }
}
