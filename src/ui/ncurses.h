#ifndef _NCURSES_H
#  define _NCURSES_H 1

#  include "../options.h"
#  include "../types.h"

extern void ncurses_refresh_lcd( void );
extern void ncurses_handle_pending_inputs( void );
extern void ncurses_init( hdw_t*, config_t*, void ( *api_emulator_press_key )( int hpkey ), void ( *api_emulator_release_key )( int hpkey ),
                          bool ( *api_emulator_is_key_pressed )( int hpkey ), bool ( *api_emulator_is_display_on )( void ),
                          unsigned char ( *api_emulator_get_annunciators )( void ), void ( *api_emulator_get_lcd_buffer )( int* target ),
                          int ( *api_emulator_get_contrast )( void ) );
extern void ncurses_exit( void );

#endif /* !(_NCURSES_H) */
