#ifndef _UI_INNER_H
#  define _UI_INNER_H 1

#  include "../emulator.h"
#  include "../types.h" /* x50ng_t */

#  define UI_EVENTS_REFRESH_INTERVAL 30000LL
#  define UI_LCD_REFRESH_INTERVAL 50000LL

#  define NB_ANNUNCIATORS 6

#  define KB_NB_ROWS ( 10 )

#  define LCD_WIDTH ( 131 )
#  define LCD_HEIGHT ( 80 )

#  define KEY_PRESS 1
#  define KEY_RELEASE 2

typedef struct {
    const char* css_class;
    const char* css_id;
    const char* label;
    const char* letter;
    const char* left;
    const char* right;
    const char* below;

    hp50g_keynames_t hpkey;
} x50ng_ui_key_t;

typedef struct {
    char* icon;
} x50ng_ui_annunciator_t;

extern x50ng_ui_key_t ui_keys[ NB_KEYS ];
extern int keys_order_normal[ NB_KEYS ];
extern int keys_order_legacy[ NB_KEYS ];
#  define NORMALIZED_KEYS_ORDER( hpkey ) ( ( opt.legacy_keyboard ? keys_order_legacy : keys_order_normal )[ hpkey ] )

extern x50ng_ui_annunciator_t ui_annunciators[ NB_ANNUNCIATORS ];

bool ui_handle_key_event( int keyval, x50ng_t* x50ng, int event_type );
void newrplify_ui_keys();

#endif /* !(_UI_INNER_H) */
