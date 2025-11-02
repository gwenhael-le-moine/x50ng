#ifndef _X50NG_TIMER_H
#  define _X50NG_TIMER_H 1

#  include <stdbool.h>
#  include <stdint.h>

#  include "types.h"

#  define X50NG_TIMER_VIRTUAL 0
#  define X50NG_TIMER_REALTIME 1

typedef void ( *x50ng_timer_cb_t )( void* );
typedef struct x50ng_timer_s hdw_timer_t;

int64_t x50ng_get_clock( void );

hdw_timer_t* x50ng_new_timer( long type, x50ng_timer_cb_t, void* user_data );
void x50ng_free_timer( hdw_timer_t* );

void x50ng_mod_timer( hdw_timer_t*, int64_t expires );
void x50ng_del_timer( hdw_timer_t* );
bool x50ng_timer_pending( hdw_timer_t* );
int64_t x50ng_timer_expires( hdw_timer_t* );

void x50ng_main_loop( x50ng_t* );
void x50ng_timer_init( x50ng_t* );

#endif /* !(_X50NG_TIMER_H) */
