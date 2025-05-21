#ifndef _X50NG_UI_H
#define _X50NG_UI_H 1

#include "types.h" /* x50ng_t */

#define UI_EVENTS_REFRESH_INTERVAL 30000LL
#define UI_LCD_REFRESH_INTERVAL 50000LL

void ui_events_timer( void* data );
void ui_lcd_timer( void* data );

#ifdef USE_GTK_APPLICATION
#  include <gtk/gtk.h>
void ui_init( GtkApplication* app, x50ng_t* x50ng );
#else
void ui_init( x50ng_t* x50ng );
#endif
void ui_update_lcd( x50ng_t* x50ng );

#endif /* !(_X50NG_UI_H) */
