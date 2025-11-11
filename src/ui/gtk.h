#ifndef _GTK_UI_H
#  define _GTK_UI_H 1

#  include "../types.h"
#  include "../options.h"

extern void gtk_ui_refresh_lcd( void );
extern void gtk_ui_handle_pending_inputs( void );
extern void gtk_ui_init( hdw_t*, config_t*, void ( *api_emulator_press_key )( int hpkey ), void ( *api_emulator_release_key )( int hpkey ),
                         bool ( *api_emulator_is_key_pressed )( int hpkey ), bool ( *api_emulator_is_display_on )( void ),
                         unsigned char ( *api_emulator_get_annunciators )( void ), void ( *api_emulator_get_lcd_buffer )( int* target ),
                         int ( *api_emulator_get_contrast )( void ) );
extern void gtk_ui_exit( void );

#endif /* !(_GTK_UI_H) */
