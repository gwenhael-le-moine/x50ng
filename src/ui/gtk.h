#ifndef _GTK_UI_H
#  define _GTK_UI_H 1

#  include "../hdw.h"

void gtk_ui_refresh_lcd( hdw_t* hdw_state );
void gtk_ui_handle_pending_inputs( hdw_t* hdw_state );
void gtk_ui_init( hdw_t* hdw_state );
void gtk_ui_exit( void );

#endif /* !(_GTK_UI_H) */
