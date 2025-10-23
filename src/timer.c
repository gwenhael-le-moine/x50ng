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

typedef struct {
    long type;
} x50ng_clock_t;

struct x50ng_timer_s {
    long type;
    int64_t expires;
    x50ng_timer_cb_t cb;
    void* user_data;
    x50ng_timer_t* next;
};

typedef x50ng_timer_cb_t QEMUTimerCB;
typedef void* QEMUClock;
QEMUClock* rt_clock = ( void* )X50NG_TIMER_REALTIME;
QEMUClock* vm_clock = ( void* )X50NG_TIMER_VIRTUAL;
int64_t ticks_per_sec = 1000000;

static x50ng_timer_t* x50ng_timer_lists[ 2 ];

int64_t x50ng_get_clock( void )
{
    struct timeval tv;

    gettimeofday( &tv, NULL );

    return ( tv.tv_sec * 1000000LL + tv.tv_usec );
}

x50ng_timer_t* x50ng_new_timer( long type, x50ng_timer_cb_t cb, void* user_data )
{
    x50ng_timer_t* ts = malloc( sizeof( x50ng_timer_t ) );

    if ( NULL == ts )
        return NULL;

    memset( ts, 0, sizeof( x50ng_timer_t ) );

    ts->type = type;
    ts->cb = cb;
    ts->user_data = user_data;

    return ts;
}

void x50ng_free_timer( x50ng_timer_t* ts ) { free( ts ); }

void x50ng_del_timer( x50ng_timer_t* ts )
{
    x50ng_timer_t **pt, *t;

    // printf("%s: ts %p\n", __func__, ts);
    pt = &x50ng_timer_lists[ ts->type ];
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

void x50ng_mod_timer( x50ng_timer_t* ts, int64_t expires )
{
    x50ng_timer_t **pt, *t;

    x50ng_del_timer( ts );

    pt = &x50ng_timer_lists[ ts->type ];
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

bool x50ng_timer_pending( x50ng_timer_t* ts )
{
    for ( x50ng_timer_t* t = x50ng_timer_lists[ ts->type ]; t; t = t->next )
        if ( t == ts )
            return true;

    return false;
}

int64_t x50ng_timer_expires( x50ng_timer_t* ts ) { return ts->expires; }

static int x50ng_timer_expired( x50ng_timer_t* timer_head, int64_t current_time )
{
    if ( NULL == timer_head )
        return 0;

    return ( timer_head->expires <= current_time );
}

/* LD TEMPO HACK */

QEMUTimer* qemu_new_timer( QEMUClock* clock, QEMUTimerCB cb, void* opaque )
{
    return ( void* )x50ng_new_timer( ( long )clock, cb, opaque );
}

void qemu_free_timer( QEMUTimer* ts ) { /* return */ x50ng_free_timer( ( void* )ts ); }

void qemu_mod_timer( QEMUTimer* ts, int64_t expire_time ) { /* return */ x50ng_mod_timer( ( void* )ts, expire_time ); }

void qemu_del_timer( QEMUTimer* ts ) { /* return */ x50ng_del_timer( ( void* )ts ); }

int qemu_timer_pending( QEMUTimer* ts ) { return x50ng_timer_pending( ( void* )ts ); }

int64_t qemu_get_clock( QEMUClock* clock ) { return x50ng_get_clock(); }

static void x50ng_run_timers( x50ng_timer_t** ptimer_head, int64_t current_time )
{
    x50ng_timer_t* ts;

    while ( true ) {
        ts = *ptimer_head;
        if ( NULL == ts || ts->expires > current_time )
            break;

        *ptimer_head = ts->next;
        ts->next = NULL;

        ts->cb( ts->user_data );
    }
}

static void x50ng_alarm_handler( int sig )
{
    if ( ( x50ng_timer_expired( x50ng_timer_lists[ X50NG_TIMER_VIRTUAL ], x50ng_get_clock() ) ||
           x50ng_timer_expired( x50ng_timer_lists[ X50NG_TIMER_REALTIME ], x50ng_get_clock() ) ) &&
         ( cpu_single_env && !cpu_single_env->exit_request ) )
        cpu_exit( cpu_single_env );
}

static void x50ng_main_loop_wait( x50ng_t* x50ng, int timeout )
{
    if ( gdb_poll( x50ng->env ) )
        gdb_handlesig( x50ng->env, 0 );
    else
        poll( NULL, 0, timeout );

    if ( x50ng->arm_idle != X50NG_ARM_OFF )
        x50ng_run_timers( &x50ng_timer_lists[ X50NG_TIMER_VIRTUAL ], x50ng_get_clock() );

    x50ng_run_timers( &x50ng_timer_lists[ X50NG_TIMER_REALTIME ], x50ng_get_clock() );
}

void x50ng_main_loop( x50ng_t* x50ng )
{
    x50ng_arm_idle_t prev_idle;
    int ret, timeout;

    while ( !x50ng->arm_exit ) {
        prev_idle = x50ng->arm_idle;

        if ( x50ng->arm_idle == X50NG_ARM_RUN ) {
#ifdef DEBUG_X50NG_TIMER_IDLE
            printf( "%lld: %s: call cpu_exec(%p)\n", ( unsigned long long )x50ng_get_clock(), __func__, x50ng->env );
#endif
            ret = cpu_exec( x50ng->env );
#ifdef DEBUG_X50NG_TIMER_IDLE
            printf( "%lld: %s: cpu_exec(): %d, PC %08x\n", ( unsigned long long )x50ng_get_clock(), __func__, ret, x50ng->env->regs[ 15 ] );
#endif

            if ( x50ng->env->regs[ 15 ] == 0x8620 ) {
                printf( "PC %08x: SRAM %08x: %08x %08x %08x <%08x>\n", x50ng->env->regs[ 15 ], 0x08000a0c,
                        *( ( uint32_t* )&x50ng->sram[ 0x0a00 ] ), *( ( uint32_t* )&x50ng->sram[ 0x0a04 ] ),
                        *( ( uint32_t* )&x50ng->sram[ 0x0a08 ] ), *( ( uint32_t* )&x50ng->sram[ 0x0a0c ] ) );
                *( ( uint32_t* )&x50ng->sram[ 0x0a0c ] ) = 0x00000000;
            }

            if ( ret == EXCP_DEBUG ) {
                gdb_handlesig( x50ng->env, SIGTRAP );
                continue;
            }

            if ( ( x50ng->arm_idle != prev_idle ) && ( x50ng->arm_idle == X50NG_ARM_OFF ) )
                cpu_reset( x50ng->env );

            timeout = ( ret == EXCP_HALTED ) ? 10 : 0;
        } else
            timeout = 1;

        x50ng_main_loop_wait( x50ng, timeout );
    }
}

void x50ng_timer_init( x50ng_t* x50ng )
{
    struct sigaction sa;
    struct itimerval it;

    x50ng_timer_lists[ X50NG_TIMER_VIRTUAL ] = NULL;
    x50ng_timer_lists[ X50NG_TIMER_REALTIME ] = NULL;

    sigfillset( &sa.sa_mask );
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = x50ng_alarm_handler;
    sigaction( SIGALRM, &sa, NULL );

    it.it_interval.tv_sec = 0;
    it.it_interval.tv_usec = 1000;
    it.it_value.tv_sec = 0;
    it.it_value.tv_usec = 1000;

    setitimer( ITIMER_REAL, &it, NULL );
}
