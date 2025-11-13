#ifndef _UI_INNER_H
#  define _UI_INNER_H 1

#  include "api.h"

#  define KB_NB_ROWS ( 10 )

// #  define NB_KEYS ( ui4x_config.model == MODEL_48GX || ui4x_config.model == MODEL_48SX ? NB_HP48_KEYS : NB_HP49_KEYS )
#  define NB_KEYS ( NB_HP50g_KEYS )

typedef enum { KEY_PRESS, KEY_RELEASE } key_event_t;

typedef struct button_t {
    const char* css_class;
    const char* css_id;
    const char* label;
    const char* letter;
    const char* left;
    const char* right;
    const char* below;

    hp50g_keynames_t hpkey;
} button_t;

extern char* ui_annunciators[ NB_ANNUNCIATORS ];

extern button_t buttons_hp50g[ NB_HP50g_KEYS ];

/********************************************/
/* API for UI to interact with the emulator */
/********************************************/

extern ui4x_emulator_api_t ui4x_emulator_api;

#endif /* !(_UI_INNER_H) */
