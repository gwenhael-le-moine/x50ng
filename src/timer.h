#ifndef _HDW_TIMER_H
#  define _HDW_TIMER_H 1

#  include <stdbool.h>
#  include <stdint.h>

#  include "types.h"

typedef enum { HDW_TIMER_VIRTUAL, HDW_TIMER_REALTIME } hdw_timer_type_t;

typedef void ( *timer_callback_t )( void* );
typedef struct hdw_timer_s hdw_timer_t;

/* extern QEMUClock* rt_clock; */
/* extern QEMUClock* vm_clock; */
extern int64_t ticks_per_sec;

extern int64_t timer_get_clock( void );

extern hdw_timer_t* timer_new( hdw_timer_type_t type, timer_callback_t, void* user_data );
extern void timer_free( hdw_timer_t* );

extern void timer_mod( hdw_timer_t*, int64_t expires );
extern void timer_del( hdw_timer_t* );
extern bool is_timer_pendinig( hdw_timer_t* );
extern int64_t timer_expires_when( hdw_timer_t* );

extern void main_loop( hdw_t* );
extern void init_timer( void );

#endif /* !(_HDW_TIMER_H) */
