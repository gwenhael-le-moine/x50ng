#ifndef _X49GP_UI_H
#define _X49GP_UI_H 1

#include "x49gp_types.h" /* x49gp_t */

int gui_init( /* GtkApplication* app, */ x49gp_t* x49gp );
void gui_update_lcd( x49gp_t* x49gp );

#endif /* !(_X49GP_UI_H) */
