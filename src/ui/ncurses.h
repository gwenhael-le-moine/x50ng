#ifndef _NCURSES_H
#  define _NCURSES_H 1

#  include "../types.h"

void ncurses_refresh_lcd( void );
void ncurses_handle_pending_inputs( void );
void ncurses_init( hdw_t*, config_t* );
void ncurses_exit( void );

#endif /* !(_NCURSES_H) */
