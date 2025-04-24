#ifndef _X50NG_UI_H
#define _X50NG_UI_H 1

#include "types.h" /* x50ng_t */

#ifdef USE_GTK_APPLICATION
#  include <gtk/gtk.h>
void ui_init( GtkApplication* app, x50ng_t* x50ng );
#else
void ui_init( x50ng_t* x50ng );
#endif
void ui_update_lcd( x50ng_t* x50ng );

#endif /* !(_X50NG_UI_H) */
