#ifndef _CONFIG_H
#define _CONFIG_H 1

#include <stdbool.h>

#include "x50ng.h"

#ifndef VERSION_MAJOR
#  define VERSION_MAJOR 0
#endif
#ifndef VERSION_MINOR
#  define VERSION_MINOR 0
#endif
#ifndef PATCHLEVEL
#  define PATCHLEVEL 0
#endif

#ifdef X50NG_DATADIR
#  define GLOBAL_DATADIR X50NG_DATADIR
#else
#  define GLOBAL_DATADIR x50ng->progpath
#endif

struct options {
    bool verbose;
    bool haz_config_file;

    char* datadir;
    char* style_filename;

    char* sd_dir;

    int debug_port;
    int start_debugger;
    char* bootloader;
    char* firmware;
    x50ng_reinit_t reinit;

    bool tui;
    bool tui_small;
    bool tui_tiny;

    bool newrpl_keyboard;
    bool legacy_keyboard;
    char* name;
    double zoom;
    bool netbook;
    int netbook_pivot_line;
};

extern struct options opt;

/*************/
/* functions */
/*************/
extern void config_init( char* progname, int argc, char* argv[] );
extern int save_config( void );

#endif /* !_CONFIG_H */
