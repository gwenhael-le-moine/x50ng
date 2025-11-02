#ifndef _CONFIG_H
#  define _CONFIG_H 1

#  include <stdbool.h>

#  ifndef VERSION_MAJOR
#    define VERSION_MAJOR 0
#  endif
#  ifndef VERSION_MINOR
#    define VERSION_MINOR 0
#  endif
#  ifndef PATCHLEVEL
#    define PATCHLEVEL 0
#  endif

#  ifdef X50NG_DATADIR
#    define GLOBAL_DATADIR X50NG_DATADIR
#  else
#    define GLOBAL_DATADIR opt.progpath
#  endif

typedef enum { FRONTEND_SDL, FRONTEND_NCURSES, FRONTEND_GTK } frontend_t;
typedef enum { HDW_REINIT_NONE = 0, HDW_REINIT_REBOOT_ONLY, HDW_REINIT_FLASH, HDW_REINIT_FLASH_FULL } hdw_reinit_t;

typedef struct config_t {
    char* progname;
    char* progpath;

    bool verbose;
    bool haz_config_file;

    char* datadir;
    char* style_filename;

    char* sd_dir;

    int debug_port;
    int start_debugger;
    char* bootloader;
    char* firmware;
    hdw_reinit_t reinit;

    frontend_t frontend;
    bool small;
    bool tiny;

    bool newrpl_keyboard;
    bool legacy_keyboard;
    char* name;
    double zoom;
    bool netbook;
    int netbook_pivot_line;
} config_t;

// extern config_t opt;

/*************/
/* functions */
/*************/
extern config_t* config_init( int argc, char* argv[] );
extern int save_config( void );

#endif /* !_CONFIG_H */
