#ifndef _X50NG_UI_H
#  define _X50NG_UI_H 1

#  include "../options.h"
#  include "../types.h" /* hdw_t */

void ui_handle_pending_inputs( void* data );
void ui_refresh_output( void* data );
void ui_init( hdw_t* hdw_state, config_t* opt );
void ui_exit( void );

#endif /* !(_X50NG_UI_H) */
