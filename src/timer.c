#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <limits.h>
#include <sys/time.h>
#include <signal.h>
#include <poll.h>

#include "x50ng.h"
#include "timer.h"
#include "gdbstub.h"

typedef timer_callback_t QEMUTimerCB;
typedef void* QEMUClock;

struct hdw_timer_s {
    hdw_timer_type_t type;
    int64_t expires;
    timer_callback_t callback;
    void* user_data;
    hdw_timer_t* next;
};

QEMUClock* rt_clock = ( void* )X50NG_TIMER_REALTIME;
QEMUClock* vm_clock = ( void* )X50NG_TIMER_VIRTUAL;
int64_t ticks_per_sec = 1000000;

static hdw_timer_t* timers_list[ 2 ];

static bool is_timer_expired( hdw_timer_t* timer_head, int64_t current_time )
{
    if ( NULL == timer_head )
        return false;

    return ( timer_head->expires <= current_time );
}

static void run_timers( hdw_timer_t** ptimer_head, int64_t current_time )
{
    hdw_timer_t* ts;

    while ( true ) {
        ts = *ptimer_head;
        if ( NULL == ts || ts->expires > current_time )
            break;

        *ptimer_head = ts->next;
        ts->next = NULL;

        ts->callback( ts->user_data );
    }
}

static void alarm_handler( int _sig )
{
    if ( ( is_timer_expired( timers_list[ X50NG_TIMER_VIRTUAL ], timer_get_clock() ) ||
           is_timer_expired( timers_list[ X50NG_TIMER_REALTIME ], timer_get_clock() ) ) &&
         ( cpu_single_env && !cpu_single_env->exit_request ) )
        cpu_exit( cpu_single_env );
}

static void main_loop_wait( hdw_t* hdw_state, int timeout )
{
    if ( gdb_poll( hdw_state->env ) )
        gdb_handlesig( hdw_state->env, 0 );
    else
        poll( NULL, 0, timeout );

    if ( hdw_state->arm_idle != X50NG_ARM_OFF )
        run_timers( &timers_list[ X50NG_TIMER_VIRTUAL ], timer_get_clock() );

    run_timers( &timers_list[ X50NG_TIMER_REALTIME ], timer_get_clock() );
}

int64_t timer_get_clock( void )
{
    struct timeval tv;

    gettimeofday( &tv, NULL );

    return ( tv.tv_sec * 1000000LL + tv.tv_usec );
}

hdw_timer_t* timer_new( hdw_timer_type_t type, timer_callback_t callback, void* user_data )
{
    hdw_timer_t* ts = malloc( sizeof( hdw_timer_t ) );

    if ( NULL == ts )
        return NULL;

    memset( ts, 0, sizeof( hdw_timer_t ) );

    ts->type = type;
    ts->callback = callback;
    ts->user_data = user_data;

    return ts;
}

void timer_free( hdw_timer_t* ts ) { free( ts ); }

void timer_del( hdw_timer_t* ts )
{
    hdw_timer_t **pt, *t;

    // printf("%s: ts %p\n", __func__, ts);
    pt = &timers_list[ ts->type ];
    while ( true ) {
        t = *pt;
        if ( NULL == t )
            break;
        if ( t == ts ) {
            *pt = t->next;
            ts->next = NULL;
            break;
        }
        pt = &t->next;
    }
}

void timer_mod( hdw_timer_t* ts, int64_t expires )
{
    hdw_timer_t **pt, *t;

    timer_del( ts );

    pt = &timers_list[ ts->type ];
    while ( true ) {
        t = *pt;
        if ( NULL == t )
            break;
        if ( t->expires > expires )
            break;
        pt = &t->next;
    }

    ts->expires = expires;
    ts->next = *pt;
    *pt = ts;
}

bool is_timer_pendinig( hdw_timer_t* ts )
{
    for ( hdw_timer_t* t = timers_list[ ts->type ]; t; t = t->next )
        if ( t == ts )
            return true;

    return false;
}

int64_t timer_expires_when( hdw_timer_t* ts ) { return ts->expires; }

/* LD TEMPO HACK */

QEMUTimer* qemu_new_timer( QEMUClock* clock, QEMUTimerCB callback, void* opaque )
{
    return ( void* )timer_new( ( long )clock, callback, opaque );
}

void qemu_free_timer( QEMUTimer* ts ) { /* return */ timer_free( ( void* )ts ); }

void qemu_mod_timer( QEMUTimer* ts, int64_t expire_time ) { /* return */ timer_mod( ( void* )ts, expire_time ); }

void qemu_del_timer( QEMUTimer* ts ) { /* return */ timer_del( ( void* )ts ); }

int qemu_timer_pending( QEMUTimer* ts ) { return is_timer_pendinig( ( void* )ts ); }

int64_t qemu_get_clock( QEMUClock* clock ) { return timer_get_clock(); }

void main_loop( hdw_t* hdw_state )
{
    x50ng_arm_idle_t prev_idle;
    int ret, timeout;

    while ( !hdw_state->arm_exit ) {
        prev_idle = hdw_state->arm_idle;

        if ( hdw_state->arm_idle == X50NG_ARM_RUN ) {
#ifdef DEBUG_X50NG_TIMER_IDLE
            printf( "%lld: %s: call cpu_exec(%p)\n", ( unsigned long long )timer_get_clock(), __func__, hdw_state->env );
#endif
            ret = cpu_exec( hdw_state->env );
#ifdef DEBUG_X50NG_TIMER_IDLE
            printf( "%lld: %s: cpu_exec(): %d, PC %08x\n", ( unsigned long long )timer_get_clock(), __func__, ret,
                    hdw_state->env->regs[ 15 ] );
#endif

            if ( hdw_state->env->regs[ 15 ] == 0x8620 ) {
                printf( "PC %08x: SRAM %08x: %08x %08x %08x <%08x>\n", hdw_state->env->regs[ 15 ], 0x08000a0c,
                        *( ( uint32_t* )&hdw_state->sram[ 0x0a00 ] ), *( ( uint32_t* )&hdw_state->sram[ 0x0a04 ] ),
                        *( ( uint32_t* )&hdw_state->sram[ 0x0a08 ] ), *( ( uint32_t* )&hdw_state->sram[ 0x0a0c ] ) );
                *( ( uint32_t* )&hdw_state->sram[ 0x0a0c ] ) = 0x00000000;
            }

            if ( ret == EXCP_DEBUG ) {
                gdb_handlesig( hdw_state->env, SIGTRAP );
                continue;
            }

            if ( ( hdw_state->arm_idle != prev_idle ) && ( hdw_state->arm_idle == X50NG_ARM_OFF ) )
                cpu_reset( hdw_state->env );

            timeout = ( ret == EXCP_HALTED ) ? 10 : 0;
        } else
            timeout = 1;

        main_loop_wait( hdw_state, timeout );
    }
}

void timer_init( void )
{
    struct sigaction sa;
    struct itimerval it;

    timers_list[ X50NG_TIMER_VIRTUAL ] = NULL;
    timers_list[ X50NG_TIMER_REALTIME ] = NULL;

    sigfillset( &sa.sa_mask );
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = alarm_handler;
    sigaction( SIGALRM, &sa, NULL );

    it.it_interval.tv_sec = 0;
    it.it_interval.tv_usec = 1000;
    it.it_value.tv_sec = 0;
    it.it_value.tv_usec = 1000;

    setitimer( ITIMER_REAL, &it, NULL );
}
