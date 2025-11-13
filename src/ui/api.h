#ifndef _X50NG_UI_H
#  define _X50NG_UI_H 1

#  include <stdbool.h>

#  define LCD_WIDTH ( 131 )
#  define LCD_HEIGHT ( ui4x_config.model == MODEL_50G ? 80 : 64 )

#  define NB_ANNUNCIATORS ( 6 )

// HP 48{G,S}X Keys
typedef enum {
    HP48_KEY_A = 0,
    HP48_KEY_B,
    HP48_KEY_C,
    HP48_KEY_D,
    HP48_KEY_E,
    HP48_KEY_F,
    HP48_KEY_MTH,
    HP48_KEY_PRG,
    HP48_KEY_CST,
    HP48_KEY_VAR,
    HP48_KEY_UP,
    HP48_KEY_NXT,
    HP48_KEY_QUOTE,
    HP48_KEY_STO,
    HP48_KEY_EVAL,
    HP48_KEY_LEFT,
    HP48_KEY_DOWN,
    HP48_KEY_RIGHT,
    HP48_KEY_SIN,
    HP48_KEY_COS,
    HP48_KEY_TAN,
    HP48_KEY_SQRT,
    HP48_KEY_POWER,
    HP48_KEY_INV,
    HP48_KEY_ENTER,
    HP48_KEY_NEG,
    HP48_KEY_EEX,
    HP48_KEY_DEL,
    HP48_KEY_BS,
    HP48_KEY_ALPHA,
    HP48_KEY_7,
    HP48_KEY_8,
    HP48_KEY_9,
    HP48_KEY_DIV,
    HP48_KEY_SHL,
    HP48_KEY_4,
    HP48_KEY_5,
    HP48_KEY_6,
    HP48_KEY_MUL,
    HP48_KEY_SHR,
    HP48_KEY_1,
    HP48_KEY_2,
    HP48_KEY_3,
    HP48_KEY_MINUS,
    HP48_KEY_ON,
    HP48_KEY_0,
    HP48_KEY_PERIOD,
    HP48_KEY_SPC,
    HP48_KEY_PLUS,
    NB_HP48_KEYS
} hp48sx_gx_keynames_t;

// HP 4{0,9}G Keys
typedef enum {
    HP49_KEY_A = 0,
    HP49_KEY_B,
    HP49_KEY_C,
    HP49_KEY_D,
    HP49_KEY_E,
    HP49_KEY_F,
    HP49_KEY_APPS,
    HP49_KEY_MODE,
    HP49_KEY_TOOL,
    HP49_KEY_VAR,
    HP49_KEY_STO,
    HP49_KEY_NXT,
    HP49_KEY_LEFT,
    HP49_KEY_UP,
    HP49_KEY_RIGHT,
    HP49_KEY_DOWN,
    HP49_KEY_HIST,
    HP49_KEY_CAT,
    HP49_KEY_EQW,
    HP49_KEY_SYMB,
    HP49_KEY_BS,
    HP49_KEY_POWER,
    HP49_KEY_SQRT,
    HP49_KEY_SIN,
    HP49_KEY_COS,
    HP49_KEY_TAN,
    HP49_KEY_EEX,
    HP49_KEY_NEG,
    HP49_KEY_X,
    HP49_KEY_INV,
    HP49_KEY_DIV,
    HP49_KEY_ALPHA,
    HP49_KEY_7,
    HP49_KEY_8,
    HP49_KEY_9,
    HP49_KEY_MUL,
    HP49_KEY_SHL,
    HP49_KEY_4,
    HP49_KEY_5,
    HP49_KEY_6,
    HP49_KEY_MINUS,
    HP49_KEY_SHR,
    HP49_KEY_1,
    HP49_KEY_2,
    HP49_KEY_3,
    HP49_KEY_PLUS,
    HP49_KEY_ON,
    HP49_KEY_0,
    HP49_KEY_PERIOD,
    HP49_KEY_SPC,
    HP49_KEY_ENTER,
    NB_HP49_KEYS
} hp49g_keynames_t;

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

    HP50g_KEY_V,
    HP50g_KEY_W,
    HP50g_KEY_X,
    HP50g_KEY_Y,
    HP50g_KEY_Z,

    HP50g_KEY_ALPHA,
    HP50g_KEY_7,
    HP50g_KEY_8,
    HP50g_KEY_9,
    HP50g_KEY_MULTIPLY,

    HP50g_KEY_SHIFT_LEFT,
    HP50g_KEY_4,
    HP50g_KEY_5,
    HP50g_KEY_6,
    HP50g_KEY_MINUS,

    HP50g_KEY_SHIFT_RIGHT,
    HP50g_KEY_1,
    HP50g_KEY_2,
    HP50g_KEY_3,
    HP50g_KEY_PLUS,

    HP50g_KEY_ON,
    HP50g_KEY_0,
    HP50g_KEY_PERIOD,
    HP50g_KEY_SPACE,
    HP50g_KEY_ENTER,

    NB_HP50g_KEYS
} hp50g_keynames_t;

typedef enum { FRONTEND_SDL, FRONTEND_NCURSES, FRONTEND_GTK } ui4x_frontend_t;

typedef enum { MODEL_48SX = 485, MODEL_48GX = 486, MODEL_40G = 406, MODEL_49G = 496, MODEL_50G = 506 } ui4x_model_t;

typedef struct ui4x_config_t {
    ui4x_model_t model;
    bool shiftless;
    bool big_screen;
    bool black_lcd;
    bool newrpl_keyboard;

    ui4x_frontend_t frontend;
    bool mono;
    bool gray;

    bool chromeless;
    bool fullscreen;

    bool tiny;
    bool small;

    bool verbose;

    double zoom;
    bool netbook;
    int netbook_pivot_line;

    char* name;
    char* progname;
    char* progpath;
    char* wire_name;
    char* ir_name;

    char* datadir;
    char* style_filename;

    char* sd_dir;
} ui4x_config_t;

typedef struct ui4x_emulator_api_t {
    /* keyboard */
    void ( *press_key )( int hpkey );
    void ( *release_key )( int hpkey );
    bool ( *is_key_pressed )( int hpkey );
    /* display */
    bool ( *is_display_on )( void );
    unsigned char ( *get_annunciators )( void );
    void ( *get_lcd_buffer )( int* target );
    int ( *get_contrast )( void );
    /* SD card */
    int ( *do_mount_sd )( char* filename );
    void ( *do_unmount_sd )( void );
    bool ( *is_sd_mounted )( void );
    void ( *get_sd_path )( char** filename );
    /* machine */
    void ( *do_reset )( void );
    void ( *do_stop )( void );
    void ( *do_sleep )( void );
    void ( *do_wake )( void );
    /* debugger */
    void ( *do_debug )( void );
} ui4x_emulator_api_t;

extern ui4x_config_t ui4x_config; /* exposed as it's indirectly used outside of ui/ by LCD_HEIGHT */

extern void ui_handle_pending_inputs( void );
extern void ui_refresh_output( void );
extern void ui_init( ui4x_config_t* opt, ui4x_emulator_api_t* emulator_api );
extern void ui_exit( void );

#endif /* !(_X50NG_UI_H) */
