#ifndef _X49GP_UI_H
#define _X49GP_UI_H 1

#include <gtk/gtk.h>
#include <glib.h>
#include <cairo.h>

#include "x49gp_types.h"

#define LCD_PIXEL_SCALE 2

typedef enum {
    UI_COLOR_BLACK = 0,
    UI_COLOR_WHITE,
    UI_COLOR_YELLOW,
    UI_COLOR_RED,
    UI_COLOR_GREEN,
    UI_COLOR_SILVER,
    UI_COLOR_ORANGE,
    UI_COLOR_BLUE,
    UI_COLOR_GRAYSCALE_0,
    UI_COLOR_GRAYSCALE_1,
    UI_COLOR_GRAYSCALE_2,
    UI_COLOR_GRAYSCALE_3,
    UI_COLOR_GRAYSCALE_4,
    UI_COLOR_GRAYSCALE_5,
    UI_COLOR_GRAYSCALE_6,
    UI_COLOR_GRAYSCALE_7,
    UI_COLOR_GRAYSCALE_8,
    UI_COLOR_GRAYSCALE_9,
    UI_COLOR_GRAYSCALE_10,
    UI_COLOR_GRAYSCALE_11,
    UI_COLOR_GRAYSCALE_12,
    UI_COLOR_GRAYSCALE_13,
    UI_COLOR_GRAYSCALE_14,
    UI_COLOR_GRAYSCALE_15,
    UI_COLOR_FACEPLATE_49GP,
    UI_COLOR_FACEPLATE_50G,
    UI_COLOR_MAX,
} x49gp_ui_color_t;

typedef enum {
    UI_CALCULATOR_HP49GP = 0,
    UI_CALCULATOR_HP49GP_NEWRPL,
    UI_CALCULATOR_HP50G,
    UI_CALCULATOR_HP50G_NEWRPL
} x49gp_ui_calculator_t;

typedef struct {
    const char* label;
    const char* letter;
    const char* left;
    const char* right;
    const char* below;
    x49gp_ui_color_t color;
    double font_size;
    cairo_font_weight_t font_weight;
    double letter_size;
    int x;
    int y;
    int width;
    int height;
    int column;
    int row;
    unsigned char columnbit;
    unsigned char rowbit;
    int eint;
    x49gp_ui_color_t bg_color;
} x49gp_ui_key_t;

typedef struct {
    x49gp_t* x49gp;
    const x49gp_ui_key_t* key;
    GtkWidget* button;
    GtkWidget* box;
    gboolean down;
    gboolean hold;
} x49gp_ui_button_t;

struct __x49gp_ui_s__ {
    GtkWidget* window;
    GtkWidget* menu;
    GtkWidget* menu_unmount;
    GtkWidget* menu_debug;

#if GTK_MAJOR_VERSION == 2
    GdkColor colors[ UI_COLOR_MAX ];
#elif GTK_MAJOR_VERSION == 3
    GdkRGBA colors[ UI_COLOR_MAX ];
#endif

    x49gp_ui_calculator_t calculator;

    x49gp_ui_button_t* buttons;
    unsigned int buttons_down;

    char* name;

    GtkWidget* lcd_canvas;
    cairo_surface_t* lcd_surface;

    GtkWidget* ui_ann_left;
    GtkWidget* ui_ann_right;
    GtkWidget* ui_ann_alpha;
    GtkWidget* ui_ann_battery;
    GtkWidget* ui_ann_busy;
    GtkWidget* ui_ann_io;

    gint width;
    gint height;

    gint kb_x_offset;
    gint kb_y_offset;

    gint annunciators_x_offset;
    gint annunciators_y_offset;

    gint lcd_x_offset;
    gint lcd_y_offset;
    gint lcd_width;
    gint lcd_height;
};

void gui_update_lcd( x49gp_t* x49gp );

int gui_init( x49gp_t* x49gp );
void gui_show_error( x49gp_t* x49gp, const char* text );
void gui_open_firmware( x49gp_t* x49gp, char** filename );

#endif /* !(_X49GP_UI_H) */
