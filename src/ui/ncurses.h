#ifndef _NCURSES_H
#  define _NCURSES_H 1

#  include "../types.h"

void ncurses_refresh_lcd( hdw_t* hdw_state );
void ncurses_handle_pending_inputs( hdw_t* hdw_state );
void ncurses_init( hdw_t* hdw_state );
void ncurses_exit( void );

#endif /* !(_NCURSES_H) */
