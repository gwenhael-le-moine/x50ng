#ifndef _X50NG_UI_H
#define _X50NG_UI_H 1

#include "types.h" /* x50ng_t */

#define UI_EVENTS_REFRESH_INTERVAL 30000LL
#define UI_LCD_REFRESH_INTERVAL 50000LL

void ui_update_lcd( x50ng_t* x50ng );
void ui_events_timer( void* data );
void ui_lcd_timer( void* data );

void ui_init( x50ng_t* x50ng );

#endif /* !(_X50NG_UI_H) */
