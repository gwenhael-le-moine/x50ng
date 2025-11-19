#ifndef _X50NG_UI_API_H
#  define _X50NG_UI_API_H 1

#  include <stdbool.h>

#  include "types.h"
#  include "options.h"

extern hdw_t* emulator_init( config_t* config );
extern void emulator_hdw_reset( void );
extern void emulator_hdw_stop( void );
extern void emulator_hdw_set_asleep( void );
extern void emulator_hdw_set_awake( void );
extern void emulator_debug( void );

extern int emulator_mount_sd( char* filename );
extern void emulator_unmount_sd( void );
extern bool emulator_is_sd_mounted( void );
extern void emulator_get_sd_path( char** filename );

extern void press_key( int hpkey );
extern void release_key( int hpkey );
extern bool is_key_pressed( int hpkey );

extern bool is_display_on( void );
extern unsigned char get_annunciators( void );
extern void get_lcd_buffer( int* target );
extern int get_contrast( void );

#endif /* !(_X50NG_UI_API_H) */
