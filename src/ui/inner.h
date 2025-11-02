#ifndef _UI_INNER_H
#  define _UI_INNER_H 1

#  include "../emulator.h"
#  include "../types.h" /* hdw_t */

#  define UI_EVENTS_REFRESH_INTERVAL 30000LL
#  define UI_LCD_REFRESH_INTERVAL 50000LL

// #  define NB_ANNUNCIATORS 6

#  define KB_NB_ROWS ( 10 )

#  define LCD_WIDTH ( 131 )
#  define LCD_HEIGHT ( 80 )

typedef enum { KEY_PRESS, KEY_RELEASE } key_event_t;

typedef struct {
    const char* css_class;
    const char* css_id;
    const char* label;
    const char* letter;
    const char* left;
    const char* right;
    const char* below;

    hp50g_keynames_t hpkey;
} ui_button_t;

extern ui_button_t ui_buttons_hp50g[ NB_HP50g_KEYS ];
extern int buttons_order_normal[ NB_HP50g_KEYS ];
extern int buttons_order_legacy[ NB_HP50g_KEYS ];
#  define NORMALIZED_BUTTONS_ORDER( hpkey ) ( ( opt.legacy_keyboard ? buttons_order_legacy : buttons_order_normal )[ hpkey ] )

extern char* ui_annunciators[ NB_ANNUNCIATORS ];

extern void newrplify_ui_buttons_hp50g();

#endif /* !(_UI_INNER_H) */
