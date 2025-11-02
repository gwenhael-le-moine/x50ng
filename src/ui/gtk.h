#ifndef _GTK_UI_H
#  define _GTK_UI_H 1

#  include "../x50ng.h"

void gtk_ui_refresh_lcd( x50ng_t* x50ng );
void gtk_ui_handle_pending_inputs( x50ng_t* x50ng );
void gtk_ui_init( x50ng_t* x50ng );
void gtk_ui_exit( void );

#endif /* !(_GTK_UI_H) */
