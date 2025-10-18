#ifndef _X50NG_UI_H
#  define _X50NG_UI_H 1

#  include "types.h" /* x50ng_t */
#  include "ui_inner.h"

void ui_handle_pending_inputs( void* data );
void ui_refresh_output( void* data );
void ui_init( x50ng_t* x50ng );
void ui_exit( void );

#endif /* !(_X50NG_UI_H) */
