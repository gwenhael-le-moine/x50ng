#ifndef _UI4x_INNER_H
#  define _UI4x_INNER_H 1

#  include "api.h"
#  include "bitmaps_misc.h"
#  include "fonts.h"

#  define KB_NB_ROWS ( 10 )

#  define NB_KEYS                                                                                                                          \
      ( ui4x_config.model == MODEL_48GX || ui4x_config.model == MODEL_48SX ? NB_HP48_KEYS                                                  \
        : ui4x_config.model == MODEL_50G                                   ? NB_HP50g_KEYS                                                 \
                                                                           : NB_HP49_KEYS )

#  define UI4X_KEY_0 ( ui4x_config.model == MODEL_50G ? HP50g_KEY_0 : ui4x_config.model == MODEL_49G ? HP49_KEY_0 : HP48_KEY_0 )
#  define UI4X_KEY_1 ( ui4x_config.model == MODEL_50G ? HP50g_KEY_1 : ui4x_config.model == MODEL_49G ? HP49_KEY_1 : HP48_KEY_1 )
#  define UI4X_KEY_2 ( ui4x_config.model == MODEL_50G ? HP50g_KEY_2 : ui4x_config.model == MODEL_49G ? HP49_KEY_2 : HP48_KEY_2 )
#  define UI4X_KEY_3 ( ui4x_config.model == MODEL_50G ? HP50g_KEY_3 : ui4x_config.model == MODEL_49G ? HP49_KEY_3 : HP48_KEY_3 )
#  define UI4X_KEY_4 ( ui4x_config.model == MODEL_50G ? HP50g_KEY_4 : ui4x_config.model == MODEL_49G ? HP49_KEY_4 : HP48_KEY_4 )
#  define UI4X_KEY_5 ( ui4x_config.model == MODEL_50G ? HP50g_KEY_5 : ui4x_config.model == MODEL_49G ? HP49_KEY_5 : HP48_KEY_5 )
#  define UI4X_KEY_6 ( ui4x_config.model == MODEL_50G ? HP50g_KEY_6 : ui4x_config.model == MODEL_49G ? HP49_KEY_6 : HP48_KEY_6 )
#  define UI4X_KEY_7 ( ui4x_config.model == MODEL_50G ? HP50g_KEY_7 : ui4x_config.model == MODEL_49G ? HP49_KEY_7 : HP48_KEY_7 )
#  define UI4X_KEY_8 ( ui4x_config.model == MODEL_50G ? HP50g_KEY_8 : ui4x_config.model == MODEL_49G ? HP49_KEY_8 : HP48_KEY_8 )
#  define UI4X_KEY_9 ( ui4x_config.model == MODEL_50G ? HP50g_KEY_9 : ui4x_config.model == MODEL_49G ? HP49_KEY_9 : HP48_KEY_9 )
#  define UI4X_KEY_A ( ui4x_config.model == MODEL_50G ? HP50g_KEY_A : ui4x_config.model == MODEL_49G ? HP49_KEY_A : HP48_KEY_A )
#  define UI4X_KEY_B ( ui4x_config.model == MODEL_50G ? HP50g_KEY_B : ui4x_config.model == MODEL_49G ? HP49_KEY_B : HP48_KEY_B )
#  define UI4X_KEY_C ( ui4x_config.model == MODEL_50G ? HP50g_KEY_C : ui4x_config.model == MODEL_49G ? HP49_KEY_C : HP48_KEY_C )
#  define UI4X_KEY_D ( ui4x_config.model == MODEL_50G ? HP50g_KEY_D : ui4x_config.model == MODEL_49G ? HP49_KEY_D : HP48_KEY_D )
#  define UI4X_KEY_E ( ui4x_config.model == MODEL_50G ? HP50g_KEY_E : ui4x_config.model == MODEL_49G ? HP49_KEY_E : HP48_KEY_E )
#  define UI4X_KEY_F ( ui4x_config.model == MODEL_50G ? HP50g_KEY_F : ui4x_config.model == MODEL_49G ? HP49_KEY_F : HP48_KEY_F )
#  define UI4X_KEY_G ( ui4x_config.model == MODEL_50G ? HP50g_KEY_G : ui4x_config.model == MODEL_49G ? HP49_KEY_APPS : HP48_KEY_MTH )
#  define UI4X_KEY_H ( ui4x_config.model == MODEL_50G ? HP50g_KEY_H : ui4x_config.model == MODEL_49G ? HP49_KEY_MODE : HP48_KEY_PRG )
#  define UI4X_KEY_I ( ui4x_config.model == MODEL_50G ? HP50g_KEY_I : ui4x_config.model == MODEL_49G ? HP49_KEY_TOOL : HP48_KEY_CST )
#  define UI4X_KEY_J ( ui4x_config.model == MODEL_50G ? HP50g_KEY_J : ui4x_config.model == MODEL_49G ? HP49_KEY_VAR : HP48_KEY_VAR )
#  define UI4X_KEY_K ( ui4x_config.model == MODEL_50G ? HP50g_KEY_K : ui4x_config.model == MODEL_49G ? HP49_KEY_STO : HP48_KEY_UP )
#  define UI4X_KEY_L ( ui4x_config.model == MODEL_50G ? HP50g_KEY_L : ui4x_config.model == MODEL_49G ? HP49_KEY_NXT : HP48_KEY_NXT )
#  define UI4X_KEY_M ( ui4x_config.model == MODEL_50G ? HP50g_KEY_M : ui4x_config.model == MODEL_49G ? HP49_KEY_HIST : HP48_KEY_QUOTE )
#  define UI4X_KEY_N ( ui4x_config.model == MODEL_50G ? HP50g_KEY_N : ui4x_config.model == MODEL_49G ? HP49_KEY_CAT : HP48_KEY_STO )
#  define UI4X_KEY_O ( ui4x_config.model == MODEL_50G ? HP50g_KEY_O : ui4x_config.model == MODEL_49G ? HP49_KEY_EQW : HP48_KEY_EVAL )
#  define UI4X_KEY_P ( ui4x_config.model == MODEL_50G ? HP50g_KEY_P : ui4x_config.model == MODEL_49G ? HP49_KEY_SYMB : HP48_KEY_LEFT )
#  define UI4X_KEY_Q ( ui4x_config.model == MODEL_50G ? HP50g_KEY_Q : ui4x_config.model == MODEL_49G ? HP49_KEY_POWER : HP48_KEY_DOWN )
#  define UI4X_KEY_R ( ui4x_config.model == MODEL_50G ? HP50g_KEY_R : ui4x_config.model == MODEL_49G ? HP49_KEY_SQRT : HP48_KEY_RIGHT )
#  define UI4X_KEY_S ( ui4x_config.model == MODEL_50G ? HP50g_KEY_S : ui4x_config.model == MODEL_49G ? HP49_KEY_SIN : HP48_KEY_SIN )
#  define UI4X_KEY_T ( ui4x_config.model == MODEL_50G ? HP50g_KEY_T : ui4x_config.model == MODEL_49G ? HP49_KEY_COS : HP48_KEY_COS )
#  define UI4X_KEY_U ( ui4x_config.model == MODEL_50G ? HP50g_KEY_U : ui4x_config.model == MODEL_49G ? HP49_KEY_TAN : HP48_KEY_TAN )
#  define UI4X_KEY_V ( ui4x_config.model == MODEL_50G ? HP50g_KEY_V : ui4x_config.model == MODEL_49G ? HP49_KEY_EEX : HP48_KEY_SQRT )
#  define UI4X_KEY_W ( ui4x_config.model == MODEL_50G ? HP50g_KEY_W : ui4x_config.model == MODEL_49G ? HP49_KEY_NEG : HP48_KEY_POWER )
#  define UI4X_KEY_X ( ui4x_config.model == MODEL_50G ? HP50g_KEY_X : ui4x_config.model == MODEL_49G ? HP49_KEY_X : HP48_KEY_INV )
#  define UI4X_KEY_Y ( ui4x_config.model == MODEL_50G ? HP50g_KEY_Y : ui4x_config.model == MODEL_49G ? HP49_KEY_INV : HP48_KEY_NEG )
#  define UI4X_KEY_Z ( ui4x_config.model == MODEL_50G ? HP50g_KEY_Z : ui4x_config.model == MODEL_49G ? HP49_KEY_DIV : HP48_KEY_EEX )
#  define UI4X_KEY_UP ( ui4x_config.model == MODEL_50G ? HP50g_KEY_UP : ui4x_config.model == MODEL_49G ? HP49_KEY_UP : HP48_KEY_UP )
#  define UI4X_KEY_DOWN ( ui4x_config.model == MODEL_50G ? HP50g_KEY_DOWN : ui4x_config.model == MODEL_49G ? HP49_KEY_DOWN : HP48_KEY_DOWN )
#  define UI4X_KEY_LEFT ( ui4x_config.model == MODEL_50G ? HP50g_KEY_LEFT : ui4x_config.model == MODEL_49G ? HP49_KEY_LEFT : HP48_KEY_LEFT )
#  define UI4X_KEY_RIGHT                                                                                                                   \
      ( ui4x_config.model == MODEL_50G ? HP50g_KEY_RIGHT : ui4x_config.model == MODEL_49G ? HP49_KEY_RIGHT : HP48_KEY_RIGHT )
#  define UI4X_KEY_SPACE ( ui4x_config.model == MODEL_50G ? HP50g_KEY_SPACE : ui4x_config.model == MODEL_49G ? HP49_KEY_SPC : HP48_KEY_SPC )
#  define UI4X_KEY_ENTER                                                                                                                   \
      ( ui4x_config.model == MODEL_50G ? HP50g_KEY_ENTER : ui4x_config.model == MODEL_49G ? HP49_KEY_ENTER : HP48_KEY_ENTER )
#  define UI4X_KEY_BACKSPACE                                                                                                               \
      ( ui4x_config.model == MODEL_50G ? HP50g_KEY_BACKSPACE : ui4x_config.model == MODEL_49G ? HP49_KEY_BS : HP48_KEY_BS )
#  define UI4X_KEY_DELETE ( ui4x_config.model == MODEL_50G ? -1 : ui4x_config.model == MODEL_49G ? -1 : HP48_KEY_DEL )
#  define UI4X_KEY_PERIOD                                                                                                                  \
      ( ui4x_config.model == MODEL_50G ? HP50g_KEY_PERIOD : ui4x_config.model == MODEL_49G ? HP49_KEY_PERIOD : HP48_KEY_PERIOD )
#  define UI4X_KEY_PLUS ( ui4x_config.model == MODEL_50G ? HP50g_KEY_PLUS : ui4x_config.model == MODEL_49G ? HP49_KEY_PLUS : HP48_KEY_PLUS )
#  define UI4X_KEY_MINUS                                                                                                                   \
      ( ui4x_config.model == MODEL_50G ? HP50g_KEY_MINUS : ui4x_config.model == MODEL_49G ? HP49_KEY_MINUS : HP48_KEY_MINUS )
#  define UI4X_KEY_MULTIPLY                                                                                                                \
      ( ui4x_config.model == MODEL_50G ? HP50g_KEY_MULTIPLY : ui4x_config.model == MODEL_49G ? HP49_KEY_MUL : HP48_KEY_MUL )
#  define UI4X_KEY_DIVIDE ( ui4x_config.model == MODEL_50G ? HP50g_KEY_Z : ui4x_config.model == MODEL_49G ? HP49_KEY_DIV : HP48_KEY_DIV )
#  define UI4X_KEY_LSHIFT                                                                                                                  \
      ( ui4x_config.model == MODEL_50G ? HP50g_KEY_SHIFT_LEFT : ui4x_config.model == MODEL_49G ? HP49_KEY_SHL : HP48_KEY_SHL )
#  define UI4X_KEY_RSHIFT                                                                                                                  \
      ( ui4x_config.model == MODEL_50G ? HP50g_KEY_SHIFT_RIGHT : ui4x_config.model == MODEL_49G ? HP49_KEY_SHR : HP48_KEY_SHR )
#  define UI4X_KEY_ALPHA                                                                                                                   \
      ( ui4x_config.model == MODEL_50G ? HP50g_KEY_ALPHA : ui4x_config.model == MODEL_49G ? HP49_KEY_ALPHA : HP48_KEY_ALPHA )
#  define UI4X_KEY_ON ( ui4x_config.model == MODEL_50G ? HP50g_KEY_ON : ui4x_config.model == MODEL_49G ? HP49_KEY_ON : HP48_KEY_ON )

/* 4.1.1.1: When defined, this symbol represents the threshold of the long
   key pression.  When the mouse button is kept pressed on a calculator's key
   for more than LONG_PRESS_THR milliseconds, the key stays pressed after
   release.
*/
#  define LONG_PRESS_THR 750

// Colors
typedef enum {
    UI4X_COLOR_HP_LOGO = 0,
    UI4X_COLOR_HP_LOGO_BG,
    UI4X_COLOR_48GX_128K_RAM,
    UI4X_COLOR_FRAME,
    UI4X_COLOR_UPPER_FACEPLATE_EDGE_TOP,
    UI4X_COLOR_UPPER_FACEPLATE,
    UI4X_COLOR_UPPER_FACEPLATE_EDGE_BOTTOM,
    UI4X_COLOR_FACEPLATE_EDGE_TOP,
    UI4X_COLOR_FACEPLATE,
    UI4X_COLOR_FACEPLATE_EDGE_BOTTOM,
    UI4X_COLOR_KEYPAD_HIGHLIGHT,
    UI4X_COLOR_BUTTON_EDGE_TOP,
    UI4X_COLOR_BUTTON,
    UI4X_COLOR_BUTTON_EDGE_BOTTOM,
    UI4X_COLOR_PIXEL_OFF,
    UI4X_COLOR_PIXEL_GREY_1,
    UI4X_COLOR_PIXEL_GREY_2,
    UI4X_COLOR_PIXEL_ON,
    UI4X_COLOR_BLACK_PIXEL_OFF,
    UI4X_COLOR_BLACK_PIXEL_GREY_1,
    UI4X_COLOR_BLACK_PIXEL_GREY_2,
    UI4X_COLOR_BLACK_PIXEL_ON,
    UI4X_COLOR_LABEL,
    UI4X_COLOR_ALPHA,
    UI4X_COLOR_SHIFT_LEFT,
    UI4X_COLOR_SHIFT_RIGHT,
    NB_COLORS,
} colors_t;

typedef enum { KEY_PRESS, KEY_RELEASE } key_event_t;

/***********/
/* typedef */
/***********/
typedef struct color_t {
    int r, g, b, a;
    int mono_rgb;
    int gray_rgb;
    int rgb;
} color_t;

typedef struct button_t {
    int x, y;
    int w, h;

    bool highlight;

    /* label on the button (text or bitmap) */
    int label_color;
    unsigned char* label_graphic;
    unsigned int label_graphic_w, label_graphic_h;

    const char* css_class;
    const char* css_id;
    const char* label;
    const char* label_sdl;
    const char* letter;
    const char* left;
    const char* left_sdl;
    const char* right;
    const char* right_sdl;
    const char* below;

    int hpkey;
} button_t;

/*************/
/* variables */
/*************/
extern letter_t small_font[ 128 ];
extern letter_t big_font[ 128 ];

extern color_t colors_48sx[ NB_COLORS ];
extern color_t colors_48gx[ NB_COLORS ];
extern color_t colors_49g[ NB_COLORS ];
extern color_t colors_50g[ NB_COLORS ];

extern button_t buttons_48sx[ NB_HP48_KEYS ];
extern button_t buttons_48gx[ NB_HP48_KEYS ];
extern button_t buttons_49g[ NB_HP49_KEYS ];
extern button_t buttons_50g[ NB_HP50g_KEYS ];

extern char* ui_annunciators[ NB_ANNUNCIATORS ];

/********************************************/
/* API for UI to interact with the emulator */
/********************************************/
extern ui4x_emulator_api_t ui4x_emulator_api;

/*************/
/* functions */
/*************/
extern int SmallTextWidth( const char* string, unsigned int length );
extern int BigTextWidth( const char* string, unsigned int length );

#endif /* !(_UI4x_INNER_H) */
