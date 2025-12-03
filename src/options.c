#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <assert.h>
#include <getopt.h>
#include <glib.h>

#include <lua.h>
#include <lauxlib.h>
#include <string.h>

#include "options.h"

#include "gdbstub.h"
#include "ui4x/src/api.h"

#ifndef LUA_OK
#  define LUA_OK 0
#endif

#ifdef X50NG_DATADIR
#  define GLOBAL_DATADIR X50NG_DATADIR
#else
#  define GLOBAL_DATADIR __config.progpath
#endif

#define CONFIG_LUA_FILE_NAME "config.lua"
#define DEFAULT_STYLE_FILE_NAME "style-50g.css"
#define DEFAULT_BOOTLOADER_FILE_NAME "firmware/boot-50g.bin"
#define DEFAULT_FIRMWARE_FILE_NAME "firmware/hp4950v215/2MB_FIX/2MB_215f.bin"

static config_t __config = {
    .model = MODEL_50G,
    .shiftless = false,
    .black_lcd = true,
    .newrpl_keyboard = false,

    .frontend = FRONTEND_GTK,
    .mono = false,
    .gray = false,

    .chromeless = false,
    .fullscreen = false,

    .tiny = false,
    .small = false,

    .verbose = false,

    .zoom = 2.0,
    .netbook = false,
    .netbook_pivot_line = 3,

    .name = NULL,
    .progname = NULL,
    .progpath = NULL,
    .wire_name = NULL,
    .ir_name = NULL,

    .datadir = NULL,
    .style_filename = NULL,

    .sd_dir = NULL,

    /* x50ng */
    .debug_port = 0,
    .start_debugger = false,
    .reinit = HDW_REINIT_NONE,
};

static lua_State* config_lua_values;

static inline bool config_read( const char* filename )
{
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
    // #ifdef PARANOID
    lua_pushliteral( config_lua_values, "x" );
    lua_pushnil( config_lua_values );
    lua_setmetatable( config_lua_values, -2 );
    lua_pop( config_lua_values, 1 );
    // #endif

    /*-----------------------------------------------------
    ; Lua 5.2+ can restrict scripts to being text only,
    ; to avoid a potential problem with loading pre-compiled
    ; Lua scripts that may have malformed Lua VM code that
    ; could possibly lead to an exploit, but again, if you
    ; have to worry about that, you have bigger security
    ; issues to worry about.  But in any case, here I'm
    ; restricting the file to "text" only.
    ;------------------------------------------------------*/
    int rc = luaL_loadfile( config_lua_values, filename );
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

static void show_help( void )
{
    fprintf( stdout,
             "%s %i.%i.%i\n"
             "Emulator for HP 49G+ / 50G calculators' hardware\n"
             "Usage: %s [<options>]\n"
             "Valid options:\n"
             "-h --help                    print this message and exit\n"
             "-v --version                 print out version\n"
             "-V --verbose                 print out more information\n"
             "-d --datadir[=absolute path] alternate datadir (default: $XDG_CONFIG_HOME/%s/)\n"
             "   --sd-dir[=absolute path]  directory to mount as SD card (default: none)\n"
             "-r --reset                   reboot on startup instead of continuing from the saved state in the state file\n"
             "   --overwrite-config        force writing <datadir>/config.lua even if it exists (allows migrating config file "
             "to its latest format if needed)\n"
             "\n"
             "-n --name[=text]             customize the title of the window (default: \"%s\")\n"
             "   --gtk                     use gtk GUI (Graphical UI) (default: true)\n"
             "   --sdl                     use sdl GUI (Graphical UI) (default: false)\n"
             "   --tui                     use ncurses TUI (Terminal text UI) (default: false)\n"
             "   --tui-small               use ncurses TUI (4 pixels per character) (Terminal text UI) (default: false)\n"
             "   --tui-tiny                use ncurses TUI (8 pixels per character) (Terminal text UI) (default: false)\n"
             "   --chromeless              borderless LCD (ncurses or sdl only) (default: false)\n"
             "   --fullscreen              fullscreen (sdl only) (default: false)\n"
             "   --netbook                 horizontal window (gtk only) (default: false)\n"
             "   --netbook-pivot-line      at which line is the keyboard split in netbook mode (GUI only) (default: 3)\n"
             "   --newrpl-keyboard         label keyboard for newRPL (GUI only)\n"
             "-s --style[=filename]        css filename in <datadir> (gtk only) (default: %s)\n"
             "-z --zoom[=X]                scale LCD by X (gtk or sdl only) (default: 2.0)\n"
             "   --chromeless              only show display (default: false)\n"
             "   --fullscreen              make the UI fullscreen (default: false)\n"
             "   --mono                    make the UI monochrome (default: false)\n"
             "   --gray                    make the UI grayscale (default: false)\n"
             "   --shiftless               don't map the shift keys to let them free for numbers (default: false)\n"
             /* specific x50ng */
             "\n"
             "   --enable-debug[=port]     enable the debugger interface (default port: %i)\n"
             "   --debug                   use along -D to also start the debugger immediately\n"
             "\n"
             "   --flash                   rebuild the flash using the supplied firmware (requires --firmware=) "
             "(implies -r for safety reasons)\n"
             "   --flash-full              rebuild the flash using the supplied firmware and drop the flash contents in the "
             "area beyond the firmware (requires --firmware=) (implies -r for safety reasons)\n"
             "   --bootloader[=filename]   bootloader file (default: %s)\n"
             "   --firmware[=filename]     firmware file (default: %s)\n",
             __config.progname, VERSION_MAJOR, VERSION_MINOR, PATCHLEVEL, __config.progname, __config.progname, __config.progname,
             DEFAULT_STYLE_FILE_NAME, DEFAULT_GDBSTUB_PORT, DEFAULT_BOOTLOADER_FILE_NAME, DEFAULT_FIRMWARE_FILE_NAME );
}

static char* config_to_string( void )
{
    char* config;
    if ( -1 ==
         asprintf( &config,
                   "--------------------------------------------------------------------------------\n"
                   "-- Configuration file for x50ng\n"
                   "-- This is a comment\n"
                   "name = \"%s\"  -- this customize the title of the window\n"
                   "\n"
                   "frontend = \"%s\" -- possible values are: \"gtk\" (default), \"sdl\", \"tui\", \"tui-small\", \"tui-tiny\"\n"
                   "\n"
                   "style = \"%s\" -- CSS file (relative to this file) (gtk only)\n"
                   "zoom = %f -- (gtk or sdl only)\n"
                   "netbook = %s -- (gtk only)\n"
                   "netbook_pivot_line = %i -- marks the transition between higher and lower keyboard (gtk only)\n"
                   "newrpl_keyboard = %s -- makes the keyboard labels more suited to newRPL use (gtk or sdl only)\n"
                   "shiftless = %s\n"
                   "chromeless = %s\n"
                   "fullscreen = %s\n"
                   "mono = %s\n"
                   "gray = %s\n"
                   "--- End of x50ng configuration -----------------------------------------------\n",
                   __config.name,
                   __config.frontend == FRONTEND_GTK
                       ? "gtk"
                       : ( __config.frontend == FRONTEND_SDL ? "sdk"
                                                             : ( __config.tiny ? "tui-tiny" : ( __config.small ? "tui-small" : "tui" ) ) ),
                   __config.style_filename, __config.zoom, __config.netbook ? "true" : "false", __config.netbook_pivot_line,
                   __config.newrpl_keyboard ? "true" : "false", __config.shiftless ? "true" : "false",
                   __config.chromeless ? "true" : "false", __config.fullscreen ? "true" : "false", __config.mono ? "true" : "false",
                   __config.gray ? "true" : "false" ) )
        exit( EXIT_FAILURE );

    return config;
}

static char* make_filename_absolute( char* filename )
{
    /* is filename readable as-is? */
    char* full_path = g_build_filename( filename, NULL );
    if ( !g_file_test( full_path, G_FILE_TEST_EXISTS ) )
        /* does filename exist in configured datadir? */
        full_path = g_build_filename( __config.datadir, filename, NULL );

    if ( !g_file_test( full_path, G_FILE_TEST_EXISTS ) )
        /* does filename exist in global datadir? */
        full_path = g_build_filename( GLOBAL_DATADIR, filename, NULL );

    if ( !g_file_test( full_path, G_FILE_TEST_EXISTS ) )
        /* out of options, hope filename exists relatively to binary */
        full_path = g_build_filename( __config.progpath, filename, NULL );

    return full_path;
}

int save_config( void )
{
    const char* config_lua_filename = g_build_filename( __config.datadir, CONFIG_LUA_FILE_NAME, NULL );
    fprintf( stderr, "Loading configuration file: %s\n", config_lua_filename );
    int fd = open( config_lua_filename, O_WRONLY | O_CREAT | O_TRUNC, 0644 );

    if ( fd < 0 ) {
        fprintf( stderr, "%s:%u: open %s: %s\n", __func__, __LINE__, config_lua_filename, strerror( errno ) );
        return -errno;
    }

    char* data = config_to_string();
    if ( write( fd, data, strlen( data ) ) != ( ssize_t )strlen( data ) ) {
        fprintf( stderr, "%s:%u: write %s: %s\n", __func__, __LINE__, config_lua_filename, strerror( errno ) );
        close( fd );
        g_free( data );
        return -errno;
    }

    close( fd );
    g_free( data );

    if ( __config.verbose )
        fprintf( stderr, "Current configuration has been written to %s\n", config_lua_filename );

    return EXIT_SUCCESS;
}

config_t* config_init( int argc, char* argv[] )
{
    __config.progname = g_path_get_basename( argv[ 0 ] );
    __config.progpath = g_path_get_dirname( argv[ 0 ] );

    /* temporary variables for filenames later treated with make_filename_absolute() */
    char* bootloader_filename = NULL;
    char* firmware_filename = NULL;
    char* style_filename = NULL;

    /***************************/
    /* 1. command-line options */
    /***************************/
    /* The cli options are temporaly stored in their clopt_* variables */
    bool clopt_do_enable_debugger = false;
    bool clopt_do_start_debugger = false;
    bool clopt_do_flash = false;
    bool clopt_do_flash_full = false;

    char* clopt_style_filename = NULL;
    char* clopt_name = NULL;
    char* clopt_sd_dir = NULL;
    int clopt_newrpl_keyboard = -1;
    double clopt_zoom = -1.0;
    int clopt_netbook = -1;
    int clopt_netbook_pivot_line = -1;
    int clopt_frontend = -1;
    int clopt_small = -1;
    int clopt_tiny = -1;
    int clopt_chromeless = -1;
    int clopt_fullscreen = -1;
    int clopt_shiftless = -1;
    int clopt_mono = -1;
    int clopt_gray = -1;

    int clopt_print_config_and_exit = false;
    int clopt_overwrite_config = false;

    const char* optstring = "d:hn:rs:vVz:";
    struct option long_options[] = {
        {"help",               no_argument,       NULL,                         'h' },
        {"version",            no_argument,       NULL,                         'v' },
        {"verbose",            no_argument,       NULL,                         'V' },

        {"print-config",       no_argument,       &clopt_print_config_and_exit, true},
        {"overwrite-config",   no_argument,       &clopt_overwrite_config,      true},
        {"datadir",            required_argument, NULL,                         'd' },

        {"sd-dir",             required_argument, NULL,                         800 },

        {"name",               required_argument, NULL,                         'n' },

        {"gtk",                no_argument,       NULL,                         900 },
        {"gui",                no_argument,       NULL,                         900 }, /* DEPRECATED */
        {"tui",                no_argument,       NULL,                         901 },
        {"tui-small",          no_argument,       NULL,                         902 },
        {"tui-tiny",           no_argument,       NULL,                         903 },
#ifdef HAS_SDL
        {"sdl",                no_argument,       NULL,                         904 },
#endif
        {"fullscreen",         no_argument,       &clopt_fullscreen,            true},
        {"shiftless",          no_argument,       &clopt_shiftless,             true},
        {"mono",               no_argument,       &clopt_mono,                  true},
        {"gray",               no_argument,       &clopt_gray,                  true},
        {"chromeless",         no_argument,       &clopt_chromeless,            true},
        {"newrpl-keyboard",    no_argument,       &clopt_newrpl_keyboard,       true},
        {"style",              required_argument, NULL,                         's' },
        {"zoom",               required_argument, NULL,                         'z' },
        {"netbook",            no_argument,       &clopt_netbook,               true},
        {"netbook-pivot-line", required_argument, NULL,                         1001},

        {"reset",              no_argument,       NULL,                         'r' },

        /* specific x50ng */
        {"enable-debug",       required_argument, NULL,                         10  },
        {"debug",              no_argument,       NULL,                         11  },

        {"flash",              no_argument,       NULL,                         90  },
        {"flash-full",         no_argument,       NULL,                         91  },
        {"bootloader",         required_argument, NULL,                         92  },
        {"firmware",           required_argument, NULL,                         93  },

        {0,                    0,                 0,                            0   }
    };
    int option_index;
    int arg = '?';
    while ( arg != EOF ) {
        arg = getopt_long( argc, argv, optstring, long_options, &option_index );

        switch ( arg ) {
            case 'h':
                show_help();
                exit( EXIT_SUCCESS );
                break;
            case 800:
                clopt_sd_dir = strdup( optarg );
                break;
            case 900:
                clopt_frontend = FRONTEND_GTK;
                break;
            case 901:
                clopt_frontend = FRONTEND_NCURSES;
                clopt_small = false;
                clopt_tiny = false;
                break;
            case 902:
                clopt_frontend = FRONTEND_NCURSES;
                clopt_small = true;
                clopt_tiny = false;
                break;
            case 903:
                clopt_frontend = FRONTEND_NCURSES;
                clopt_small = false;
                clopt_tiny = true;
                break;
            case 904:
                clopt_frontend = FRONTEND_SDL;
                break;
            case 1001:
                clopt_netbook_pivot_line = atoi( optarg );
                break;
            case 'd':
                __config.datadir = strdup( optarg );
                break;
            case 'n':
                clopt_name = strdup( optarg );
                break;
            case 'r':
                if ( __config.reinit < HDW_REINIT_REBOOT_ONLY )
                    __config.reinit = HDW_REINIT_REBOOT_ONLY;
                break;
            case 's':
                clopt_style_filename = strdup( optarg );
                break;
            case 'z':
                clopt_zoom = atof( optarg );
                break;
            case 'v':
                fprintf( stderr, "%i.%i.%i\n", VERSION_MAJOR, VERSION_MINOR, PATCHLEVEL );
                exit( EXIT_SUCCESS );
                break;
            case 'V':
                __config.verbose = true;
                break;

            /* specific x50ng */
            case 10:
                clopt_do_enable_debugger = true;
                __config.debug_port = atoi( optarg );
                break;
            case 11:
                clopt_do_start_debugger = true;
                break;
            case 90:
                clopt_do_flash = true;
                break;
            case 91:
                clopt_do_flash = true;
                clopt_do_flash_full = true;
                break;
            case 92:
                bootloader_filename = strdup( optarg );
                break;
            case 93:
                firmware_filename = strdup( optarg );
                break;

            default:
                break;
        }
    }

    /******************************************************************************/
    /* 2. if datadir hasn't been set through --datadir then set it to its default */
    /******************************************************************************/
    if ( __config.datadir == NULL )
        __config.datadir = g_build_filename( g_get_user_config_dir(), __config.progname, NULL );

    /**********************/
    /* 3. read config.lua */
    /**********************/
    const char* config_lua_filename = g_build_filename( __config.datadir, CONFIG_LUA_FILE_NAME, NULL );
    if ( __config.verbose )
        fprintf( stderr, "Loading configuration file %s\n", config_lua_filename );

    /* populates config_lua_values[]  */
    __config.haz_config_file = config_read( config_lua_filename );
    if ( __config.haz_config_file ) {
        if ( clopt_overwrite_config )
            // pretends config file hasn't been found to force it being overwritten
            __config.haz_config_file = false;

        /* Now pull config options' values from config_lua_values by name
           options are set directly into __config or temp filename variables
         */

        lua_getglobal( config_lua_values, "frontend" );
        const char* frontend = luaL_optstring( config_lua_values, -1, "gtk" );
        if ( frontend != NULL ) {
            if ( strcmp( frontend, "gtk" ) == 0 || strcmp( frontend, "gui" ) == 0 ) {
                __config.frontend = FRONTEND_GTK;
                __config.small = false;
                __config.tiny = false;
            }
            if ( strcmp( frontend, "sdl" ) == 0 ) {
                __config.frontend = FRONTEND_SDL;
                __config.small = false;
                __config.tiny = false;
            }
            if ( strcmp( frontend, "tui" ) == 0 ) {
                __config.frontend = FRONTEND_NCURSES;
                __config.small = false;
                __config.tiny = false;
            }
            if ( strcmp( frontend, "tui-small" ) == 0 ) {
                __config.frontend = FRONTEND_NCURSES;
                __config.small = true;
                __config.tiny = false;
            }
            if ( strcmp( frontend, "tui-tiny" ) == 0 ) {
                __config.frontend = FRONTEND_NCURSES;
                __config.small = false;
                __config.tiny = true;
            }
        }

        lua_getglobal( config_lua_values, "style" );
        const char* lua_style_filename = luaL_optstring( config_lua_values, -1, NULL );
        if ( lua_style_filename != NULL )
            style_filename = strdup( lua_style_filename );

        lua_getglobal( config_lua_values, "name" );
        const char* lua_name = luaL_optstring( config_lua_values, -1, NULL );
        if ( lua_name != NULL )
            __config.name = strdup( lua_name );

        lua_getglobal( config_lua_values, "shiftless" );
        __config.shiftless = lua_toboolean( config_lua_values, -1 );

        lua_getglobal( config_lua_values, "zoom" );
        __config.zoom = luaL_optnumber( config_lua_values, -1, __config.zoom );

        lua_getglobal( config_lua_values, "netbook" );
        __config.netbook = lua_toboolean( config_lua_values, -1 );

        lua_getglobal( config_lua_values, "netbook_pivot_line" );
        __config.netbook_pivot_line = luaL_optinteger( config_lua_values, -1, __config.netbook_pivot_line );

        lua_getglobal( config_lua_values, "newrpl_keyboard" );
        __config.newrpl_keyboard = lua_toboolean( config_lua_values, -1 );

        lua_getglobal( config_lua_values, "chromeless" );
        __config.chromeless = lua_toboolean( config_lua_values, -1 );

        lua_getglobal( config_lua_values, "fullscreen" );
        __config.fullscreen = lua_toboolean( config_lua_values, -1 );

        lua_getglobal( config_lua_values, "mono" );
        __config.mono = lua_toboolean( config_lua_values, -1 );

        lua_getglobal( config_lua_values, "gray" );
        __config.gray = lua_toboolean( config_lua_values, -1 );

        /* lua_getglobal( config_lua_values, "sd_dir" ); */
        /* const char* lua_sd_dir = luaL_optstring( config_lua_values, -1, NULL ); */
        /* if ( lua_sd_dir != NULL && 0 < strlen( lua_sd_dir ) ) */
        /*     __config.sd_dir = strdup( lua_sd_dir ); */
    }

    /********************************************************************************/
    /* 4. Finally chck if any cli options overrides its config file options's value */
    /********************************************************************************/
    if ( clopt_style_filename != NULL )
        style_filename = strdup( clopt_style_filename );
    else if ( style_filename == NULL )
        style_filename = DEFAULT_STYLE_FILE_NAME;

    if ( clopt_name != NULL )
        __config.name = strdup( clopt_name );
    else if ( __config.name == NULL )
        __config.name = strdup( __config.progname );

    if ( clopt_sd_dir != NULL )
        __config.sd_dir = strdup( clopt_sd_dir );

    if ( clopt_newrpl_keyboard != -1 )
        __config.newrpl_keyboard = clopt_newrpl_keyboard;

    if ( clopt_zoom > 0 )
        __config.zoom = clopt_zoom;

    if ( clopt_netbook != -1 )
        __config.netbook = clopt_netbook == true;

    if ( clopt_frontend != -1 )
        __config.frontend = clopt_frontend;

    if ( clopt_small != -1 )
        __config.small = clopt_small == true;

    if ( clopt_tiny != -1 )
        __config.tiny = clopt_tiny == true;

    if ( clopt_chromeless != -1 )
        __config.chromeless = clopt_chromeless == true;

    if ( clopt_fullscreen != -1 )
        __config.fullscreen = clopt_fullscreen == true;

    if ( clopt_shiftless != -1 )
        __config.shiftless = clopt_shiftless == true;

    if ( clopt_mono != -1 )
        __config.mono = clopt_mono == true;

    if ( clopt_gray != -1 )
        __config.gray = clopt_gray == true;

    if ( clopt_netbook_pivot_line > -1 )
        __config.netbook_pivot_line = clopt_netbook_pivot_line;

    /* setup debugger if needed */
    if ( clopt_do_enable_debugger ) {
        if ( __config.debug_port == 0 )
            __config.debug_port = DEFAULT_GDBSTUB_PORT;

        __config.start_debugger = clopt_do_start_debugger;
    }

    /* setup flashing if needed */
    if ( clopt_do_flash ) {
        if ( __config.reinit < HDW_REINIT_FLASH )
            __config.reinit = HDW_REINIT_FLASH;

        if ( clopt_do_flash_full )
            __config.reinit = HDW_REINIT_FLASH_FULL;
    }

    /* set default bootloader and firmware filenames if needed */
    if ( bootloader_filename == NULL )
        bootloader_filename = DEFAULT_BOOTLOADER_FILE_NAME;
    if ( firmware_filename == NULL )
        firmware_filename = DEFAULT_FIRMWARE_FILE_NAME;

    /* make style, bootloader and firmware filenames absolute */
    __config.style_filename = make_filename_absolute( style_filename );
    __config.bootloader = make_filename_absolute( bootloader_filename );
    __config.firmware = make_filename_absolute( firmware_filename );

    free( style_filename );
    free( clopt_style_filename );
    free( clopt_name );
    free( clopt_sd_dir );

    if ( clopt_print_config_and_exit ) {
        fprintf( stdout, "Calculated configuration:\n%s", config_to_string() );
        exit( EXIT_SUCCESS );
    }

    if ( __config.verbose )
        fprintf( stdout, "Calculated configuration:\n%s", config_to_string() );

    return &__config;
}
