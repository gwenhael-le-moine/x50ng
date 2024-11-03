#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>

#include "x49gp.h"
#include "s3c2410.h"
#include "s3c2410_intc.h"

typedef struct {
    uint32_t rtccon;
    uint32_t ticnt;
    uint32_t rtcalm;
    uint32_t almsec;
    uint32_t almmin;
    uint32_t almhour;
    uint32_t almdate;
    uint32_t almmon;
    uint32_t almyear;
    uint32_t rtcrst;
    uint32_t bcdsec;
    uint32_t bcdmin;
    uint32_t bcdhour;
    uint32_t bcddate;
    uint32_t bcdday;
    uint32_t bcdmon;
    uint32_t bcdyear;

    unsigned int nr_regs;
    s3c2410_offset_t* regs;

    x49gp_t* x49gp;
    x49gp_timer_t* tick_timer;
    x49gp_timer_t* alarm_timer;
    int64_t interval; /* us */
    int64_t expires;  /* us */
} s3c2410_rtc_t;

static int s3c2410_rtc_data_init( s3c2410_rtc_t* rtc )
{
    s3c2410_offset_t regs[] = { S3C2410_OFFSET( RTC, RTCCON, 0x00, rtc->rtccon ),   S3C2410_OFFSET( RTC, TICNT, 0x00, rtc->ticnt ),
                                S3C2410_OFFSET( RTC, RTCALM, 0x00, rtc->rtcalm ),   S3C2410_OFFSET( RTC, ALMSEC, 0x00, rtc->almsec ),
                                S3C2410_OFFSET( RTC, ALMMIN, 0x00, rtc->almmin ),   S3C2410_OFFSET( RTC, ALMHOUR, 0x00, rtc->almhour ),
                                S3C2410_OFFSET( RTC, ALMDATE, 0x01, rtc->almdate ), S3C2410_OFFSET( RTC, ALMMON, 0x01, rtc->almmon ),
                                S3C2410_OFFSET( RTC, ALMYEAR, 0x00, rtc->almyear ), S3C2410_OFFSET( RTC, RTCRST, 0x00, rtc->rtcrst ),
                                S3C2410_OFFSET( RTC, BCDSEC, 0, rtc->bcdsec ),      S3C2410_OFFSET( RTC, BCDMIN, 0, rtc->bcdmin ),
                                S3C2410_OFFSET( RTC, BCDHOUR, 0, rtc->bcdhour ),    S3C2410_OFFSET( RTC, BCDDATE, 0, rtc->bcddate ),
                                S3C2410_OFFSET( RTC, BCDDAY, 0, rtc->bcdday ),      S3C2410_OFFSET( RTC, BCDMON, 0, rtc->bcdmon ),
                                S3C2410_OFFSET( RTC, BCDYEAR, 0, rtc->bcdyear ) };

    memset( rtc, 0, sizeof( s3c2410_rtc_t ) );

    rtc->regs = malloc( sizeof( regs ) );
    if ( NULL == rtc->regs ) {
        fprintf( stderr, "%s:%u: Out of memory\n", __FUNCTION__, __LINE__ );
        return -ENOMEM;
    }

    memcpy( rtc->regs, regs, sizeof( regs ) );
    rtc->nr_regs = sizeof( regs ) / sizeof( regs[ 0 ] );

    return 0;
}

static __inline__ uint32_t bin2bcd( uint32_t bin ) { return ( ( bin / 10 ) << 4 ) | ( bin % 10 ); }

static __inline__ uint32_t bcd2bin( uint32_t bcd ) { return ( ( bcd >> 4 ) * 10 ) + ( bcd & 0x0f ); }

static void s3c2410_rtc_timeout( void* user_data )
{
    s3c2410_rtc_t* rtc = user_data;
    x49gp_t* x49gp = rtc->x49gp;
    int64_t now, us;

    if ( !( rtc->ticnt & 0x80 ) ) {
        return;
    }

#ifdef DEBUG_S3C2410_RTC
    printf( "RTC: assert TICK interrupt\n" );
#endif

    s3c2410_intc_assert( x49gp, INT_TICK, 0 );

    now = x49gp_get_clock();
    while ( rtc->expires <= now ) {
        rtc->expires += rtc->interval;
    }

    us = rtc->expires - now;
    if ( us < 1000 )
        us = 1000;

#ifdef DEBUG_S3C2410_RTC
    printf( "RTC: restart TICK timer (%llu us)\n", ( unsigned long long )us );
#endif

    x49gp_mod_timer( rtc->tick_timer, rtc->expires );
}

static int s3c2410_rtc_set_ticnt( s3c2410_rtc_t* rtc )
{
    int64_t now, us;

    if ( x49gp_timer_pending( rtc->tick_timer ) ) {
        x49gp_del_timer( rtc->tick_timer );
#ifdef DEBUG_S3C2410_RTC
        printf( "RTC: stop TICK timer\n" );
#endif
    }

    if ( !( rtc->ticnt & 0x80 ) ) {
        return 0;
    }

    us = ( ( ( rtc->ticnt & 0x7f ) + 1 ) * 1000000 ) / 128;

    rtc->interval = us;
    if ( rtc->interval < 1000 )
        rtc->interval = 1000;

    now = x49gp_get_clock();
    rtc->expires = now + rtc->interval;

    us = rtc->expires - now;
    if ( us < 1000 )
        us = 1000;

#ifdef DEBUG_S3C2410_RTC
    printf( "RTC: start TICK timer (%lld us)\n", ( unsigned long long )us );
#endif

    x49gp_mod_timer( rtc->tick_timer, rtc->expires );
    return 0;
}

static void s3c2410_rtc_alarm( void* user_data )
{
    s3c2410_rtc_t* rtc = user_data;
    x49gp_t* x49gp = rtc->x49gp;
    struct tm* tm;
    struct timeval tv;
    int64_t now, us;
    int match = 1;

    if ( !( rtc->rtcalm & 0x40 ) ) {
        return;
    }

    gettimeofday( &tv, NULL );
    tm = localtime( &tv.tv_sec );

    now = x49gp_get_clock();
    us = 1000000LL - tv.tv_usec;

    if ( match && ( rtc->rtcalm & 0x01 ) ) {
        if ( tm->tm_sec != bcd2bin( rtc->almsec ) )
            match = 0;
    }
    if ( match && ( rtc->rtcalm & 0x02 ) ) {
        if ( tm->tm_min != bcd2bin( rtc->almmin ) )
            match = 0;
    }
    if ( match && ( rtc->rtcalm & 0x04 ) ) {
        if ( tm->tm_hour != bcd2bin( rtc->almhour ) )
            match = 0;
    }
    if ( match && ( rtc->rtcalm & 0x08 ) ) {
        if ( tm->tm_mday != bcd2bin( rtc->almdate ) )
            match = 0;
    }
    if ( match && ( rtc->rtcalm & 0x10 ) ) {
        if ( ( tm->tm_mon + 1 ) != bcd2bin( rtc->almmon ) )
            match = 0;
    }
    if ( match && ( rtc->rtcalm & 0x20 ) ) {
        if ( ( tm->tm_year % 100 ) != bcd2bin( rtc->almyear ) )
            match = 0;
    }

    if ( match ) {
#ifdef DEBUG_S3C2410_RTC
        printf( "RTC: assert ALARM interrupt\n" );
#endif
        s3c2410_intc_assert( x49gp, INT_RTC, 0 );
    }

#ifdef DEBUG_S3C2410_RTC
    printf( "RTC: reload ALARM timer (%lld us)\n", ( unsigned long long )us );
#endif

    x49gp_mod_timer( rtc->alarm_timer, now + us );
}

static int s3c2410_rtc_set_rtcalm( s3c2410_rtc_t* rtc )
{
    struct timeval tv;
    int64_t now, us;

    if ( !( rtc->rtcalm & 0x40 ) ) {
        x49gp_del_timer( rtc->alarm_timer );
        return 0;
    }

    gettimeofday( &tv, NULL );

    now = x49gp_get_clock();
    us = 1000000LL - tv.tv_usec;

#ifdef DEBUG_S3C2410_RTC
    printf( "RTC: start ALARM timer (%lld us)\n", ( unsigned long long )us );
#endif

    x49gp_mod_timer( rtc->alarm_timer, now + us );
    return 0;
}

static uint32_t s3c2410_rtc_read( void* opaque, target_phys_addr_t offset )
{
    s3c2410_rtc_t* rtc = opaque;
    s3c2410_offset_t* reg;

    if ( !S3C2410_OFFSET_OK( rtc, offset ) ) {
        return ~( 0 );
    }

    reg = S3C2410_OFFSET_ENTRY( rtc, offset );

    if ( S3C2410_RTC_BCDSEC <= offset && offset <= S3C2410_RTC_BCDYEAR ) {
        struct tm* tm;
        struct timeval tv;

        gettimeofday( &tv, NULL );
        tm = localtime( &tv.tv_sec );

        switch ( offset ) {
            case S3C2410_RTC_BCDSEC:
                *( reg->datap ) = bin2bcd( tm->tm_sec );
                break;
            case S3C2410_RTC_BCDMIN:
                *( reg->datap ) = bin2bcd( tm->tm_min );
                break;
            case S3C2410_RTC_BCDHOUR:
                *( reg->datap ) = bin2bcd( tm->tm_hour );
                break;
            case S3C2410_RTC_BCDDATE:
                *( reg->datap ) = bin2bcd( tm->tm_mday );
                break;
            case S3C2410_RTC_BCDDAY:
                *( reg->datap ) = bin2bcd( tm->tm_wday + 1 );
                break;
            case S3C2410_RTC_BCDMON:
                *( reg->datap ) = bin2bcd( tm->tm_mon + 1 );
                break;
            case S3C2410_RTC_BCDYEAR:
                *( reg->datap ) = bin2bcd( tm->tm_year % 100 );
                break;
        }
    }

#ifdef DEBUG_S3C2410_RTC
    printf( "read  %s [%08x] %s [%08lx] data %08x\n", "s3c2410-rtc", S3C2410_RTC_BASE, reg->name, ( unsigned long )offset,
            *( reg->datap ) );
#endif

    return *( reg->datap );
}

static void s3c2410_rtc_write( void* opaque, target_phys_addr_t offset, uint32_t data )
{
    s3c2410_rtc_t* rtc = opaque;
    s3c2410_offset_t* reg;

    if ( !S3C2410_OFFSET_OK( rtc, offset ) ) {
        return;
    }

    reg = S3C2410_OFFSET_ENTRY( rtc, offset );

#ifdef DEBUG_S3C2410_RTC
    printf( "write %s [%08x] %s [%08lx] data %08x\n", "s3c2410-rtc", S3C2410_RTC_BASE, reg->name, ( unsigned long )offset, data );
#endif

    switch ( offset ) {
        case S3C2410_RTC_TICNT:
            *( reg->datap ) = data;
            s3c2410_rtc_set_ticnt( rtc );
            break;
        case S3C2410_RTC_RTCALM:
            *( reg->datap ) = data;
            s3c2410_rtc_set_rtcalm( rtc );
            break;
        default:
            *( reg->datap ) = data;
            break;
    }
}

static int s3c2410_rtc_load( x49gp_module_t* module, GKeyFile* key )
{
    s3c2410_rtc_t* rtc = module->user_data;
    s3c2410_offset_t* reg;
    int error = 0;
    int i;

#ifdef DEBUG_X49GP_MODULES
    printf( "%s: %s:%u\n", module->name, __FUNCTION__, __LINE__ );
#endif

    for ( i = 0; i < rtc->nr_regs; i++ ) {
        reg = &rtc->regs[ i ];

        if ( NULL == reg->name )
            continue;

        if ( x49gp_module_get_u32( module, key, reg->name, reg->reset, reg->datap ) )
            error = -EAGAIN;
    }

    s3c2410_rtc_set_ticnt( rtc );
    s3c2410_rtc_set_rtcalm( rtc );

    return error;
}

static int s3c2410_rtc_save( x49gp_module_t* module, GKeyFile* key )
{
    s3c2410_rtc_t* rtc = module->user_data;
    s3c2410_offset_t* reg;
    int i;

#ifdef DEBUG_X49GP_MODULES
    printf( "%s: %s:%u\n", module->name, __FUNCTION__, __LINE__ );
#endif

    for ( i = 0; i < rtc->nr_regs; i++ ) {
        reg = &rtc->regs[ i ];

        if ( NULL == reg->name )
            continue;

        x49gp_module_set_u32( module, key, reg->name, *( reg->datap ) );
    }

    return 0;
}

static int s3c2410_rtc_reset( x49gp_module_t* module, x49gp_reset_t reset )
{
    s3c2410_rtc_t* rtc = module->user_data;
    s3c2410_offset_t* reg;
    int i;

#ifdef DEBUG_X49GP_MODULES
    printf( "%s: %s:%u\n", module->name, __FUNCTION__, __LINE__ );
#endif

    for ( i = 0; i < ( S3C2410_RTC_RTCALM >> 2 ); i++ ) {
        reg = &rtc->regs[ i ];

        if ( NULL == reg->name )
            continue;

        *( reg->datap ) = reg->reset;
    }

    s3c2410_rtc_set_ticnt( rtc );

    return 0;
}

static CPUReadMemoryFunc* s3c2410_rtc_readfn[] = { s3c2410_rtc_read, s3c2410_rtc_read, s3c2410_rtc_read };

static CPUWriteMemoryFunc* s3c2410_rtc_writefn[] = { s3c2410_rtc_write, s3c2410_rtc_write, s3c2410_rtc_write };

static int s3c2410_rtc_init( x49gp_module_t* module )
{
    s3c2410_rtc_t* rtc;
    int iotype;

#ifdef DEBUG_X49GP_MODULES
    printf( "%s: %s:%u\n", module->name, __FUNCTION__, __LINE__ );
#endif

    rtc = malloc( sizeof( s3c2410_rtc_t ) );
    if ( NULL == rtc ) {
        fprintf( stderr, "%s: %s:%u: Out of memory\n", module->x49gp->progname, __FUNCTION__, __LINE__ );
        return -ENOMEM;
    }
    if ( s3c2410_rtc_data_init( rtc ) ) {
        free( rtc );
        return -ENOMEM;
    }

    module->user_data = rtc;
    rtc->x49gp = module->x49gp;

    rtc->tick_timer = x49gp_new_timer( X49GP_TIMER_REALTIME, s3c2410_rtc_timeout, rtc );
    rtc->alarm_timer = x49gp_new_timer( X49GP_TIMER_REALTIME, s3c2410_rtc_alarm, rtc );

    iotype = cpu_register_io_memory( s3c2410_rtc_readfn, s3c2410_rtc_writefn, rtc );
#ifdef DEBUG_S3C2410_RTC
    printf( "%s: iotype %08x\n", __FUNCTION__, iotype );
#endif
    cpu_register_physical_memory( S3C2410_RTC_BASE, S3C2410_MAP_SIZE, iotype );

    return 0;
}

static int s3c2410_rtc_exit( x49gp_module_t* module )
{
    s3c2410_rtc_t* rtc;

#ifdef DEBUG_X49GP_MODULES
    printf( "%s: %s:%u\n", module->name, __FUNCTION__, __LINE__ );
#endif

    if ( module->user_data ) {
        rtc = module->user_data;
        if ( rtc->regs )
            free( rtc->regs );
        free( rtc );
    }

    x49gp_module_unregister( module );
    free( module );

    return 0;
}

int x49gp_s3c2410_rtc_init( x49gp_t* x49gp )
{
    x49gp_module_t* module;

    if ( x49gp_module_init( x49gp, "s3c2410-rtc", s3c2410_rtc_init, s3c2410_rtc_exit, s3c2410_rtc_reset, s3c2410_rtc_load, s3c2410_rtc_save,
                            NULL, &module ) ) {
        return -1;
    }

    return x49gp_module_register( module );
}
