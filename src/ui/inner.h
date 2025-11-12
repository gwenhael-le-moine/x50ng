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
extern int buttons_order_normal[ NB_HP50g_KEYS ];
extern int buttons_order_legacy[ NB_HP50g_KEYS ];
#  define NORMALIZED_BUTTONS_ORDER( hpkey ) ( ( ui4x_config.legacy_keyboard ? buttons_order_legacy : buttons_order_normal )[ hpkey ] )

/********************************************/
/* API for UI to interact with the emulator */
/********************************************/
/* keyboard */
extern void ( *emulator_press_key )( int hpkey );
extern void ( *emulator_release_key )( int hpkey );
extern bool ( *emulator_is_key_pressed )( int hpkey );
/* display */
extern bool ( *emulator_is_display_on )( void );
extern unsigned char ( *emulator_get_annunciators )( void );
extern int ( *emulator_get_contrast )( void );
extern void ( *emulator_get_lcd_buffer )( int* target );
/* machine */
extern void ( *emulator_do_reset )( void );
extern void ( *emulator_do_stop )( void );
extern void ( *emulator_do_sleep )( void );
extern void ( *emulator_do_wake )( void );
/* SD card */
extern int ( *emulator_do_mount_sd )( char* filename );
extern void ( *emulator_do_unmount_sd )( void );
extern bool ( *emulator_do_is_sd_mounted )( void );
extern void ( *emulator_do_get_sd_path )( char** filename );
/* debugger */
extern void ( *emulator_do_debug )( void );

#endif /* !(_UI_INNER_H) */
