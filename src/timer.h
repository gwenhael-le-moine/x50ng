#ifndef _X50NG_TIMER_H
#  define _X50NG_TIMER_H 1

#  include <stdbool.h>
#  include <stdint.h>

#  include "types.h"

typedef enum { X50NG_TIMER_VIRTUAL, X50NG_TIMER_REALTIME } hdw_timer_type_t;

typedef void ( *timer_callback_t )( void* );
typedef struct hdw_timer_s hdw_timer_t;

/* extern QEMUClock* rt_clock; */
/* extern QEMUClock* vm_clock; */
extern int64_t ticks_per_sec;

int64_t timer_get_clock( void );

hdw_timer_t* timer_new( hdw_timer_type_t type, timer_callback_t, void* user_data );
void timer_free( hdw_timer_t* );

void timer_mod( hdw_timer_t*, int64_t expires );
void timer_del( hdw_timer_t* );
bool is_timer_pendinig( hdw_timer_t* );
int64_t timer_expires_when( hdw_timer_t* );

void main_loop( hdw_t* );
void init_timer( void );

#endif /* !(_X50NG_TIMER_H) */
