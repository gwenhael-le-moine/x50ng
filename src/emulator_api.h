#ifndef _X50NG_UI_API_H
#  define _X50NG_UI_API_H 1

#  include <stdbool.h>

#  include "types.h"
#  include "options.h"

extern hdw_t* emulator_init( config_t* config );
extern void emulator_exit( void );

extern void press_key( int hpkey );
extern void release_key( int hpkey );
extern bool is_key_pressed( int hpkey );

extern bool is_display_on( void );
extern unsigned char get_annunciators( void );
extern void get_lcd_buffer( int* target );
extern int get_contrast( void );

#endif /* !(_X50NG_UI_API_H) */
