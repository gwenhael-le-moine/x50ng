#ifndef _OPTIONS_H
#  define _OPTIONS_H 1

#  include <stdbool.h>

#  include "ui4x/src/api.h"

#  ifndef VERSION_MAJOR
#    define VERSION_MAJOR 0
#  endif
#  ifndef VERSION_MINOR
#    define VERSION_MINOR 0
#  endif
#  ifndef PATCHLEVEL
#    define PATCHLEVEL 0
#  endif

typedef enum { HDW_REINIT_NONE = 0, HDW_REINIT_REBOOT_ONLY, HDW_REINIT_FLASH, HDW_REINIT_FLASH_FULL } hdw_reinit_t;

typedef struct config_t {
    /* duplicating ui4x_config_t here so that config_init can return one big struct */
    ui4x_model_t model;
    bool shiftless;
    bool black_lcd;
    bool newrpl_keyboard;

    ui4x_frontend_t frontend;
    bool mono;
    bool gray;

    bool chromeless;
    bool fullscreen;

    bool tiny;
    bool small;

    bool verbose;

    double zoom;
    bool netbook;
    int netbook_pivot_line;

    char* name;
    char* progname;
    char* progpath;
    char* wire_name;
    char* ir_name;

    char* datadir;
    char* style_filename;

    char* sd_dir;

    /* options below not copied to UI */
    bool haz_config_file;
    int debug_port;
    int start_debugger;
    char* bootloader;
    char* firmware;
    hdw_reinit_t reinit;
} config_t;

/*************/
/* functions */
/*************/
extern config_t* config_init( int argc, char* argv[] );
extern int save_config( void );

#endif /* !_OPTIONS_H */
