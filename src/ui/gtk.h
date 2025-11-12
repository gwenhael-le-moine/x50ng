#ifndef _GTK_UI_H
#  define _GTK_UI_H 1

#  include "../types.h"
#  include "../options.h"

extern void gtk_ui_refresh_lcd( void );
extern void gtk_ui_handle_pending_inputs( void );
extern void gtk_ui_init( hdw_t* hdw_state );
extern void gtk_ui_exit( void );

#endif /* !(_GTK_UI_H) */
