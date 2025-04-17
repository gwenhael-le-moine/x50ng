#ifndef _X49GP_TIMER_H
#define _X49GP_TIMER_H 1

#include <stdint.h>

#include "types.h"

#define X49GP_TIMER_VIRTUAL 0
#define X49GP_TIMER_REALTIME 1

int64_t x50ng_get_clock( void );

typedef void ( *x50ng_timer_cb_t )( void* );
typedef struct x50ng_timer_s x50ng_timer_t;

x50ng_timer_t* x50ng_new_timer( long type, x50ng_timer_cb_t, void* user_data );
void x50ng_free_timer( x50ng_timer_t* );

void x50ng_mod_timer( x50ng_timer_t*, int64_t expires );
void x50ng_del_timer( x50ng_timer_t* );
int x50ng_timer_pending( x50ng_timer_t* );
int64_t x50ng_timer_expires( x50ng_timer_t* );

#define X49GP_GTK_REFRESH_INTERVAL 30000LL
#define X49GP_LCD_REFRESH_INTERVAL 50000LL

int x50ng_main_loop( x50ng_t* );
int x50ng_timer_init( x50ng_t* );

#endif /* !(_X49GP_TIMER_H) */
