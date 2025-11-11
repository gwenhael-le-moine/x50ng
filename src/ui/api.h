#ifndef _X50NG_UI_H
#  define _X50NG_UI_H 1

#  include "../options.h"
#  include "../types.h" /* hdw_t */

#  define LCD_WIDTH ( 131 )
#  define LCD_HEIGHT ( 80 )

#  define NB_ANNUNCIATORS ( 6 )

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

extern void ui_handle_pending_inputs( void* data );
extern void ui_refresh_output( void* data );
extern void ui_init( hdw_t* hdw_state, config_t* opt, void ( *api_emulator_press_key )( int hpkey ),
                     void ( *api_emulator_release_key )( int hpkey ), bool ( *api_emulator_is_key_pressed )( int hpkey ),
                     bool ( *api_emulator_is_display_on )( void ), unsigned char ( *api_emulator_get_annunciators )( void ),
                     void ( *api_emulator_get_lcd_buffer )( int* target ), int ( *api_emulator_get_contrast )( void ) );
extern void ui_exit( void );

#endif /* !(_X50NG_UI_H) */
