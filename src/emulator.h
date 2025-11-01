#ifndef _X50NG_EMULATOR_H
#  define _X50NG_EMULATOR_H 1

#  include "types.h"
#  include "x50ng.h"

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
    int column;
    int row;
    unsigned char columnbit;
    unsigned char rowbit;
    int eint;
    bool pressed;
} x50ng_key_t;

typedef struct {
    int state_pixel_index;
} x50ng_annunciator_t;

extern x50ng_key_t x50ng_keys[ NB_KEYS ];
extern x50ng_annunciator_t x50ng_annunciators[ 6 ];

void x50ng_set_idle( x50ng_t*, x50ng_arm_idle_t idle );

void emulator_init( void );
void emulator_exit( void );

extern void press_key( int hpkey );
extern void release_key( int hpkey );
extern bool is_key_pressed( int hpkey );

extern unsigned char get_annunciators( void );
extern bool get_display_state( void );
extern void get_lcd_buffer( int* target );
/* extern int get_contrast( void ); */

#endif /* !(_X50NG_EMULATOR_H) */
