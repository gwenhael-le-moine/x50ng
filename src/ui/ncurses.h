#ifndef _NCURSES_H
#  define _NCURSES_H 1

#  include "../options.h"
#  include "../types.h"

extern void ncurses_refresh_lcd( void );
extern void ncurses_handle_pending_inputs( void );
extern void ncurses_init( void );
extern void ncurses_exit( void );

#endif /* !(_NCURSES_H) */
