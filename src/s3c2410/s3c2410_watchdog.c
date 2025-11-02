#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "../module.h"

#include "s3c2410.h"
#include "s3c2410_intc.h"

typedef struct {
    uint32_t wtcon;
    uint32_t wtdat;
    uint32_t wtcnt;

    unsigned int nr_regs;
    s3c2410_offset_t* regs;

    hdw_t* hdw_state;

    unsigned long interval;
    hdw_timer_t* timer;
} s3c2410_watchdog_t;

static int s3c2410_watchdog_data_init( s3c2410_watchdog_t* watchdog )
{
    s3c2410_offset_t regs[] = { S3C2410_OFFSET( WATCHDOG, WTCON, 0x8021, watchdog->wtcon ),
                                S3C2410_OFFSET( WATCHDOG, WTDAT, 0x8000, watchdog->wtdat ),
                                S3C2410_OFFSET( WATCHDOG, WTCNT, 0x8000, watchdog->wtcnt ) };

    memset( watchdog, 0, sizeof( s3c2410_watchdog_t ) );

    watchdog->regs = malloc( sizeof( regs ) );
    if ( NULL == watchdog->regs ) {
        fprintf( stderr, "%s:%u: Out of memory\n", __func__, __LINE__ );
        return -ENOMEM;
    }

    memcpy( watchdog->regs, regs, sizeof( regs ) );
    watchdog->nr_regs = sizeof( regs ) / sizeof( regs[ 0 ] );

    return 0;
}

static void s3c2410_watchdog_tick( void* data )
{
    s3c2410_watchdog_t* watchdog = data;
    hdw_t* hdw_state = watchdog->hdw_state;

    if ( watchdog->wtcnt > 0 )
        watchdog->wtcnt--;
    else
        watchdog->wtcnt = watchdog->wtdat;

    if ( watchdog->wtcnt > 0 ) {
        //		watchdog->timer.expires += watchdog->interval;
        timer_mod( watchdog->timer, timer_get_clock() + watchdog->interval );
        return;
    }

    if ( watchdog->wtcon & 0x0004 ) {
#ifdef DEBUG_S3C2410_WATCHDOG
        printf( "WATCHDOG: assert WDT interrupt\n" );
#endif
        //		g_mutex_lock(hdw_state->memlock);

        s3c2410_intc_assert( hdw_state, INT_WDT, 0 );

        //		g_mutex_unlock(hdw_state->memlock);
    }

    if ( watchdog->wtcon & 0x0001 ) {
#ifdef DEBUG_S3C2410_WATCHDOG
        printf( "WATCHDOG: assert internal RESET\n" );
#endif

        x50ng_modules_reset( hdw_state, HDW_RESET_WATCHDOG );
        cpu_reset( hdw_state->env );

        //		if (hdw_state->arm->NresetSig != LOW) {
        //			hdw_state->arm->NresetSig = LOW;
        //			hdw_state->arm->Exception++;
        //		}
        return;
    }

    //	watchdog->timer.expires += watchdog->interval;
    timer_mod( watchdog->timer, timer_get_clock() + watchdog->interval );
}

unsigned long s3c2410_watchdog_next_interrupt( hdw_t* hdw_state )
{
    s3c2410_watchdog_t* watchdog = hdw_state->s3c2410_watchdog;
    unsigned long irq;
    unsigned long ticks;

    ticks = timer_get_clock();

    if ( !( watchdog->wtcon & 0x0020 ) )
        return ~( 0 );

    if ( is_timer_pendinig( watchdog->timer ) )
        irq = timer_expires_when( watchdog->timer ) - ticks;
    else
        irq = 0;

    if ( watchdog->wtcnt )
        irq += ( watchdog->wtcnt - 1 ) * watchdog->interval;
    else
        irq += watchdog->wtdat * watchdog->interval;

#ifdef DEBUG_S3C2410_WATCHDOG
    printf( "WATCHDOG: wtcnt %u, interval %lu, expires %llu, next irq %lu\n", watchdog->wtcnt, watchdog->interval,
            ( unsigned long long )( is_timer_pendinig( watchdog->timer ) ? timer_expires_when( watchdog->timer ) : 0 ), irq );
#endif

    return irq;
}

static int s3c2410_watchdog_update( s3c2410_watchdog_t* watchdog )
{
    uint32_t pre, mux;

    if ( !( watchdog->wtcon & 0x0020 ) ) {
        timer_del( watchdog->timer );
#ifdef DEBUG_S3C2410_WATCHDOG
        printf( "WATCHDOG: stop timer\n" );
#endif
        return 0;
    }

    pre = ( watchdog->wtcon >> 8 ) & 0xff;
    mux = ( watchdog->wtcon >> 3 ) & 3;

    watchdog->interval = ( pre + 1 ) * ( 16 << mux );

#ifdef DEBUG_S3C2410_WATCHDOG
    printf( "WATCHDOG: start tick (%lu PCLKs)\n", watchdog->interval );
#endif
    timer_mod( watchdog->timer, timer_get_clock() + watchdog->interval );

    return 0;
}

static uint32_t s3c2410_watchdog_read( void* opaque, target_phys_addr_t offset )
{
    s3c2410_watchdog_t* watchdog = opaque;
    s3c2410_offset_t* reg;

    if ( !S3C2410_OFFSET_OK( watchdog, offset ) )
        return ~( 0 );

    reg = S3C2410_OFFSET_ENTRY( watchdog, offset );

#ifdef DEBUG_S3C2410_WATCHDOG
    printf( "read  %s [%08x] %s [%08lx] data %08x\n", "s3c2410-watchdog", S3C2410_WATCHDOG_BASE, reg->name, ( unsigned long )offset,
            *( reg->datap ) );
#endif

    return *( reg->datap );
}

static void s3c2410_watchdog_write( void* opaque, target_phys_addr_t offset, uint32_t data )
{
    s3c2410_watchdog_t* watchdog = opaque;
    s3c2410_offset_t* reg;

    if ( !S3C2410_OFFSET_OK( watchdog, offset ) )
        return;

    reg = S3C2410_OFFSET_ENTRY( watchdog, offset );

#ifdef DEBUG_S3C2410_WATCHDOG
    printf( "write %s [%08x] %s [%08lx] data %08x\n", "s3c2410-watchdog", S3C2410_WATCHDOG_BASE, reg->name, ( unsigned long )offset, data );
#endif

    *( reg->datap ) = data;

    switch ( offset ) {
        case S3C2410_WATCHDOG_WTCON:
        case S3C2410_WATCHDOG_WTCNT:
            s3c2410_watchdog_update( watchdog );
            break;
        default:
            break;
    }
}

static int s3c2410_watchdog_load( hdw_module_t* module, GKeyFile* key )
{
    s3c2410_watchdog_t* watchdog = module->user_data;
    s3c2410_offset_t* reg;
    int error = 0;
    unsigned int i;

#ifdef DEBUG_X50NG_MODULES
    printf( "%s: %s:%u\n", module->name, __func__, __LINE__ );
#endif

    for ( i = 0; i < watchdog->nr_regs; i++ ) {
        reg = &watchdog->regs[ i ];

        if ( NULL == reg->name )
            continue;

        if ( x50ng_module_get_u32( module, key, reg->name, reg->reset, reg->datap ) )
            error = -EAGAIN;
    }

    s3c2410_watchdog_update( watchdog );

    return error;
}

static int s3c2410_watchdog_save( hdw_module_t* module, GKeyFile* key )
{
    s3c2410_watchdog_t* watchdog = module->user_data;
    s3c2410_offset_t* reg;
    unsigned int i;

#ifdef DEBUG_X50NG_MODULES
    printf( "%s: %s:%u\n", module->name, __func__, __LINE__ );
#endif

    for ( i = 0; i < watchdog->nr_regs; i++ ) {
        reg = &watchdog->regs[ i ];

        if ( NULL == reg->name )
            continue;

        x50ng_module_set_u32( module, key, reg->name, *( reg->datap ) );
    }

    return 0;
}

static int s3c2410_watchdog_reset( hdw_module_t* module, hdw_reset_t reset )
{
    s3c2410_watchdog_t* watchdog = module->user_data;
    s3c2410_offset_t* reg;
    unsigned int i;

#ifdef DEBUG_X50NG_MODULES
    printf( "%s: %s:%u\n", module->name, __func__, __LINE__ );
#endif

    for ( i = 0; i < watchdog->nr_regs; i++ ) {
        reg = &watchdog->regs[ i ];

        if ( NULL == reg->name )
            continue;

        *( reg->datap ) = reg->reset;
    }

    s3c2410_watchdog_update( watchdog );

    return 0;
}

static CPUReadMemoryFunc* s3c2410_watchdog_readfn[] = { s3c2410_watchdog_read, s3c2410_watchdog_read, s3c2410_watchdog_read };

static CPUWriteMemoryFunc* s3c2410_watchdog_writefn[] = { s3c2410_watchdog_write, s3c2410_watchdog_write, s3c2410_watchdog_write };

static int s3c2410_watchdog_init( hdw_module_t* module )
{
    s3c2410_watchdog_t* watchdog;
    int iotype;

#ifdef DEBUG_X50NG_MODULES
    printf( "%s: %s:%u\n", module->name, __func__, __LINE__ );
#endif

    watchdog = malloc( sizeof( s3c2410_watchdog_t ) );
    if ( NULL == watchdog ) {
        fprintf( stderr, "%s:%u: Out of memory\n", __func__, __LINE__ );
        return -ENOMEM;
    }
    if ( s3c2410_watchdog_data_init( watchdog ) ) {
        free( watchdog );
        return -ENOMEM;
    }

    module->user_data = watchdog;

    watchdog->hdw_state = module->hdw_state;
    module->hdw_state->s3c2410_watchdog = watchdog;

    watchdog->timer = timer_new( X50NG_TIMER_VIRTUAL, s3c2410_watchdog_tick, watchdog );

    iotype = cpu_register_io_memory( s3c2410_watchdog_readfn, s3c2410_watchdog_writefn, watchdog );
#ifdef DEBUG_S3C2410_WATCHDOG
    printf( "%s: iotype %08x\n", __func__, iotype );
#endif
    cpu_register_physical_memory( S3C2410_WATCHDOG_BASE, S3C2410_MAP_SIZE, iotype );
    return 0;
}

static int s3c2410_watchdog_exit( hdw_module_t* module )
{
    s3c2410_watchdog_t* watchdog;

#ifdef DEBUG_X50NG_MODULES
    printf( "%s: %s:%u\n", module->name, __func__, __LINE__ );
#endif

    if ( module->user_data ) {
        watchdog = module->user_data;
        if ( watchdog->regs )
            free( watchdog->regs );
        free( watchdog );
    }

    x50ng_module_unregister( module );
    free( module );

    return 0;
}

int x50ng_s3c2410_watchdog_init( hdw_t* hdw_state )
{
    hdw_module_t* module;

    if ( x50ng_module_init( hdw_state, "s3c2410-watchdog", s3c2410_watchdog_init, s3c2410_watchdog_exit, s3c2410_watchdog_reset,
                            s3c2410_watchdog_load, s3c2410_watchdog_save, NULL, &module ) )
        return -1;

    return x50ng_module_register( module );
}
