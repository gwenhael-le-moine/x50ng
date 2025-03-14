#ifndef _X49GP_UI_H
#define _X49GP_UI_H 1

#include <stdbool.h>

#include <gtk/gtk.h>
#include <glib.h>
#include <cairo.h>

#include "x49gp_types.h"

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
} x49gp_ui_hpkey_t;

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
} x49gp_ui_key_t;

typedef struct {
    x49gp_t* x49gp;
    const x49gp_ui_key_t* key;
    GtkWidget* button;
    bool down;
    bool hold;
} x49gp_ui_button_t;

struct __x49gp_ui_s__ {
    GtkWidget* window;

    x49gp_ui_button_t* buttons;

    GtkWidget* lcd_canvas;
    cairo_surface_t* lcd_surface;

    GtkWidget* ui_ann_left;
    GtkWidget* ui_ann_right;
    GtkWidget* ui_ann_alpha;
    GtkWidget* ui_ann_battery;
    GtkWidget* ui_ann_busy;
    GtkWidget* ui_ann_io;
};

void gui_update_lcd( x49gp_t* x49gp );

int gui_init( x49gp_t* x49gp );

#endif /* !(_X49GP_UI_H) */
