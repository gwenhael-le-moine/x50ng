#ifndef _GUI_H
#  define _GUI_H 1

#  include "../x50ng.h"

void gui_refresh_lcd( x50ng_t* x50ng );
void gui_handle_pending_inputs( x50ng_t* x50ng );
void gui_init( x50ng_t* x50ng );
void gui_exit( void );

#endif /* !(_GUI_H) */
