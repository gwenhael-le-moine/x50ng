#ifndef _CONFIG_H
#define _CONFIG_H 1

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

typedef enum { MODEL_49GP = 0, MODEL_49GP_NEWRPL, MODEL_50G, MODEL_50G_NEWRPL } x49gpng_model_t;

struct options {
    char* config;
    int debug_port;
    int start_debugger;
    char* firmware;
    x49gp_reinit_t reinit;

    x49gpng_model_t model;
    char* name;

    char* font;
    int display_scale;
    int text_scale;
};

extern struct options opt;

/*************/
/* functions */
/*************/
extern void config_init( char* progname, int argc, char* argv[] );

#endif /* !_CONFIG_H */
