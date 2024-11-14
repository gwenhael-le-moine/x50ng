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

typedef enum { MODEL_49GP = 0, MODEL_50G } x49gpng_model_t;

struct options {
    char* state_filename;
    int debug_port;
    int start_debugger;
    char* firmware;
    x49gp_reinit_t reinit;

    x49gpng_model_t model;
    bool newrpl;
    char* name;

    char* font;
    int display_scale;
    int font_size;
    char* config_lua_filename;
    bool verbose;

    char* datadir;
};

extern struct options opt;

/*************/
/* functions */
/*************/
extern void config_init( char* progname, int argc, char* argv[] );

#endif /* !_CONFIG_H */
