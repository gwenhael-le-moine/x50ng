#ifndef _X50NG_UI_H
#  define _X50NG_UI_H 1

#  include "../options.h"
#  include "../types.h" /* hdw_t */

extern void ui_handle_pending_inputs( void* data );
extern void ui_refresh_output( void* data );
extern void ui_init( hdw_t* hdw_state, config_t* opt );
extern void ui_exit( void );

#endif /* !(_X50NG_UI_H) */
