#ifndef _CONFIG_H
#define _CONFIG_H 1

#include <stdbool.h>

#include "x49gp.h"

#ifndef VERSION_MAJOR
#  define VERSION_MAJOR 0
#endif
#ifndef VERSION_MINOR
#  define VERSION_MINOR 0
#endif
#ifndef PATCHLEVEL
#  define PATCHLEVEL 0
#endif

#ifdef X49GP_DATADIR
#  define GLOBAL_DATADIR X49GP_DATADIR
#else
#  define GLOBAL_DATADIR x49gp->progpath
#endif

struct options {
    int debug_port;
    int start_debugger;
    char* bootloader;
    char* firmware;
    x49gp_reinit_t reinit;

    bool newrpl_keyboard;
    bool legacy_keyboard;
    char* name;

    int zoom;
    bool netbook;
    int netbook_pivot_line;
    char* style_filename;
    bool verbose;
    bool haz_config_file;

    char* datadir;
};

extern struct options opt;

/*************/
/* functions */
/*************/
extern void config_init( char* progname, int argc, char* argv[] );
extern int save_config( void );

#endif /* !_CONFIG_H */
