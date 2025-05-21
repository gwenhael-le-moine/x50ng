#ifndef _X50NG_UI_H
#define _X50NG_UI_H 1

#include "types.h" /* x50ng_t */
#include "ui_inner.h"

void ui_update_lcd( x50ng_t* x50ng );
void ui_events_timer( void* data );
void ui_lcd_timer( void* data );
void ui_init( x50ng_t* x50ng );

#endif /* !(_X50NG_UI_H) */
