#ifndef _X50NG_EMULATOR_H
#  define _X50NG_EMULATOR_H 1

#  include "types.h"
#  include "x50ng.h"

typedef enum {
    HP50g_KEY_A = 0,
    HP50g_KEY_B,
    HP50g_KEY_C,
    HP50g_KEY_D,
    HP50g_KEY_E,
    HP50g_KEY_F,

    HP50g_KEY_G,
    HP50g_KEY_H,
    HP50g_KEY_I,
    HP50g_KEY_UP,
    HP50g_KEY_J, /* 10 */

    HP50g_KEY_K,
    HP50g_KEY_L,
    HP50g_KEY_LEFT,
    HP50g_KEY_DOWN,
    HP50g_KEY_RIGHT,

    HP50g_KEY_M,
    HP50g_KEY_N,
    HP50g_KEY_O,
    HP50g_KEY_P,
    HP50g_KEY_BACKSPACE, /* 20 */

    HP50g_KEY_Q,
    HP50g_KEY_R,
    HP50g_KEY_S,
    HP50g_KEY_T,
    HP50g_KEY_U,

    HP50g_KEY_ENTER,
    HP50g_KEY_V,
    HP50g_KEY_W,
    HP50g_KEY_X,
    HP50g_KEY_Y, /* 30 */

    HP50g_KEY_ALPHA,
    HP50g_KEY_7,
    HP50g_KEY_8,
    HP50g_KEY_9,
    HP50g_KEY_Z,

    HP50g_KEY_SHIFT_LEFT,
    HP50g_KEY_4,
    HP50g_KEY_5,
    HP50g_KEY_6,
    HP50g_KEY_MULTIPLY, /* 40 */

    HP50g_KEY_SHIFT_RIGHT,
    HP50g_KEY_1,
    HP50g_KEY_2,
    HP50g_KEY_3,
    HP50g_KEY_MINUS,

    HP50g_KEY_ON,
    HP50g_KEY_0,
    HP50g_KEY_PERIOD,
    HP50g_KEY_SPACE,
    HP50g_KEY_PLUS, /* 50 */

    NB_HP50g_KEYS
} hp50g_keynames_t;

typedef struct {
    int column;
    int row;
    int eint;
    bool pressed;
} x50ng_key_t;

typedef struct {
    int state_pixel_index;
} x50ng_annunciator_t;

extern x50ng_key_t x50ng_keys[ NB_HP50g_KEYS ];
extern x50ng_annunciator_t x50ng_annunciators[ 6 ];

extern void x50ng_set_idle( x50ng_t*, x50ng_arm_idle_t idle );

extern void emulator_init( void );
extern void emulator_exit( void );

extern void press_key( int hpkey );
extern void release_key( int hpkey );
extern bool is_key_pressed( int hpkey );

extern unsigned char get_annunciators( void );
extern bool get_display_state( void );
extern void get_lcd_buffer( int* target );
extern int get_contrast( void );

#endif /* !(_X50NG_EMULATOR_H) */
