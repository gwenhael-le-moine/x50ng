#ifndef _NCURSES_H
#  define _NCURSES_H 1

#  include "../x50ng.h"

void ncurses_refresh_lcd( x50ng_t* x50ng );
void ncurses_handle_pending_inputs( x50ng_t* x50ng );
void ncurses_init( x50ng_t* x50ng );
void ncurses_exit( void );

#endif /* !(_NCURSES_H) */
