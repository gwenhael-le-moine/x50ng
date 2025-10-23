#ifndef _UI_INNER_H
#  define _UI_INNER_H 1

#  include "../types.h" /* x50ng_t */

#  define UI_EVENTS_REFRESH_INTERVAL 30000LL
#  define UI_LCD_REFRESH_INTERVAL 50000LL

#  define NB_ANNUNCIATORS 6

#  define KB_NB_ROWS ( 10 )

#  define LCD_WIDTH ( 131 )
#  define LCD_HEIGHT ( 80 )

#  define KEY_PRESS 1
#  define KEY_RELEASE 2

typedef enum {
    HPKEY_A = 0,
    HPKEY_B,
    HPKEY_C,
    HPKEY_D,
    HPKEY_E,
    HPKEY_F,

    HPKEY_G,
    HPKEY_H,
    HPKEY_I,
    HPKEY_UP,
    HPKEY_J, /* 10 */

    HPKEY_K,
    HPKEY_L,
    HPKEY_LEFT,
    HPKEY_DOWN,
    HPKEY_RIGHT,

    HPKEY_M,
    HPKEY_N,
    HPKEY_O,
    HPKEY_P,
    HPKEY_BACKSPACE, /* 20 */

    HPKEY_Q,
    HPKEY_R,
    HPKEY_S,
    HPKEY_T,
    HPKEY_U,

    HPKEY_ENTER,
    HPKEY_V,
    HPKEY_W,
    HPKEY_X,
    HPKEY_Y, /* 30 */

    HPKEY_ALPHA,
    HPKEY_7,
    HPKEY_8,
    HPKEY_9,
    HPKEY_Z,

    HPKEY_SHIFT_LEFT,
    HPKEY_4,
    HPKEY_5,
    HPKEY_6,
    HPKEY_MULTIPLY, /* 40 */

    HPKEY_SHIFT_RIGHT,
    HPKEY_1,
    HPKEY_2,
    HPKEY_3,
    HPKEY_MINUS,

    HPKEY_ON,
    HPKEY_0,
    HPKEY_PERIOD,
    HPKEY_SPACE,
    HPKEY_PLUS, /* 50 */

    NB_KEYS
} x50ng_ui_hpkey_t;

typedef struct {
    const char* css_class;
    const char* css_id;
    const char* label;
    const char* letter;
    const char* left;
    const char* right;
    const char* below;

    int column;
    int row;
    unsigned char columnbit;
    unsigned char rowbit;
    int eint;
} x50ng_ui_key_t;

typedef struct {
    char* icon;
    int state_pixel_index;
} x50ng_ui_annunciator_t;

extern x50ng_ui_key_t ui_keys[ NB_KEYS ];
extern int keys_order_normal[ NB_KEYS ];
extern int keys_order_legacy[ NB_KEYS ];
#  define NORMALIZED_KEYS_ORDER( hpkey ) ( ( opt.legacy_keyboard ? keys_order_legacy : keys_order_normal )[ hpkey ] )

extern x50ng_ui_annunciator_t ui_annunciators[ NB_ANNUNCIATORS ];

bool ui_handle_key_event( int keyval, x50ng_t* x50ng, int event_type );
void newrplify_ui_keys();
void x50ng_set_key_state( x50ng_t* x50ng, const x50ng_ui_key_t* key, bool state );
#  define X50NG_PRESS_KEY( x50ng, key ) x50ng_set_key_state( x50ng, key, true );
#  define X50NG_RELEASE_KEY( x50ng, key ) x50ng_set_key_state( x50ng, key, false );

#endif /* !(_UI_INNER_H) */
