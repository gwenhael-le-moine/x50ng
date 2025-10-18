#ifndef _TUI_H
#  define _TUI_H 1

#  include "x50ng.h"

void tui_refresh_lcd( x50ng_t* x50ng );
void tui_handle_pending_inputs( x50ng_t* x50ng );
void tui_init( x50ng_t* x50ng );
void tui_exit( void );

#endif /* !(_TUI_H) */
