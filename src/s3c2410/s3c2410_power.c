#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "../emulator.h"
#include "../module.h"

#include "s3c2410.h"
#include "s3c2410_power.h"

static const uint32_t EXTCLK = 12000000;

typedef struct {
    uint32_t locktime;
    uint32_t mpllcon;
    uint32_t upllcon;
    uint32_t clkcon;
    uint32_t clkslow;
    uint32_t clkdivn;

    unsigned int nr_regs;
    s3c2410_offset_t* regs;

    hdw_t* hdw_state;
} s3c2410_power_t;

static int s3c2410_power_data_init( s3c2410_power_t* power )
{
    s3c2410_offset_t regs[] = {
        S3C2410_OFFSET( POWER, LOCKTIME, 0x00ffffff, power->locktime ), S3C2410_OFFSET( POWER, MPLLCON, 0x0005c080, power->mpllcon ),
        S3C2410_OFFSET( POWER, UPLLCON, 0x00028080, power->upllcon ),   S3C2410_OFFSET( POWER, CLKCON, 0x0007fff0, power->clkcon ),
        S3C2410_OFFSET( POWER, CLKSLOW, 0x00000004, power->clkslow ),   S3C2410_OFFSET( POWER, CLKDIVN, 0x00000000, power->clkdivn ) };

    memset( power, 0, sizeof( s3c2410_power_t ) );

    power->regs = malloc( sizeof( regs ) );
    if ( NULL == power->regs ) {
        fprintf( stderr, "%s:%u: Out of memory\n", __func__, __LINE__ );
        return -ENOMEM;
    }

    memcpy( power->regs, regs, sizeof( regs ) );
    power->nr_regs = sizeof( regs ) / sizeof( regs[ 0 ] );

    return 0;
}

static uint32_t s3c2410_power_read( void* opaque, target_phys_addr_t offset )
{
    s3c2410_power_t* power = opaque;
    s3c2410_offset_t* reg;

    if ( !S3C2410_OFFSET_OK( power, offset ) ) {
        return ~( 0 );
    }

    reg = S3C2410_OFFSET_ENTRY( power, offset );

#ifdef DEBUG_S3C2410_POWER
    printf( "read  %s [%08x] %s [%08lx] data %08x\n", "s3c2410-power", S3C2410_POWER_BASE, reg->name, ( unsigned long )offset,
            *( reg->datap ) );
#endif

    return *( reg->datap );
}

static void s3c2410_power_write( void* opaque, target_phys_addr_t offset, uint32_t data )
{
    s3c2410_power_t* power = opaque;
    hdw_t* hdw_state = power->hdw_state;
    s3c2410_offset_t* reg;
    uint32_t mMdiv, mPdiv, mSdiv;
    uint32_t uMdiv, uPdiv, uSdiv;
    uint32_t slow_bit, slow_val;

    if ( !S3C2410_OFFSET_OK( power, offset ) ) {
        return;
    }

    reg = S3C2410_OFFSET_ENTRY( power, offset );

#ifdef DEBUG_S3C2410_POWER
    printf( "write %s [%08x] %s [%08lx] data %08x\n", "s3c2410-power", S3C2410_POWER_BASE, reg->name, ( unsigned long )offset, data );
#endif

    switch ( offset ) {
        case S3C2410_POWER_CLKCON:
            if ( data & CLKCON_POWER_OFF ) {
                *( reg->datap ) = 0x7fff0;
#ifdef DEBUG_S3C2410_POWER
                printf( "POWER: enter POWER_OFF\n" );
#endif

                reset_modules( hdw_state, HDW_RESET_POWER_OFF );

                //			if (hdw_state->arm->NresetSig != LOW) {
                //				hdw_state->arm->NresetSig = LOW;
                //				hdw_state->arm->Exception++;
                //			}
                hdw_set_idle( hdw_state, HDW_ARM_OFF );
                return;
            }

            if ( !( power->clkcon & CLKCON_IDLE ) && ( data & CLKCON_IDLE ) ) {
                *( reg->datap ) = data;
#ifdef DEBUG_S3C2410_POWER
                printf( "POWER: enter IDLE\n" );
#endif
                hdw_set_idle( hdw_state, HDW_ARM_SLEEP );
                return;
            }

            *( reg->datap ) = data;
            return;

        case S3C2410_POWER_LOCKTIME:
            *( reg->datap ) = data;
            return;

        default:
            *( reg->datap ) = data;
            break;
    }

    mMdiv = ( power->mpllcon >> 12 ) & 0xff;
    mPdiv = ( power->mpllcon >> 4 ) & 0x3f;
    mSdiv = ( power->mpllcon >> 0 ) & 0x03;
    hdw_state->MCLK = ( ( ( u64 )EXTCLK ) * ( ( u64 )( mMdiv + 8 ) ) ) / ( ( u64 )( ( mPdiv + 2 ) * ( 1 << mSdiv ) ) );

    uMdiv = ( power->upllcon >> 12 ) & 0xff;
    uPdiv = ( power->upllcon >> 4 ) & 0x3f;
    uSdiv = ( power->upllcon >> 0 ) & 0x03;
    hdw_state->UCLK = ( ( ( u64 )EXTCLK ) * ( ( u64 )( uMdiv + 8 ) ) ) / ( ( u64 )( ( uPdiv + 2 ) * ( 1 << uSdiv ) ) );

    slow_bit = ( power->clkslow & 0x10 );
    if ( slow_bit ) {
        slow_val = ( power->clkslow >> 0 ) & 0x07;
        if ( 0 == slow_val )
            hdw_state->FCLK = EXTCLK;
        else
            hdw_state->FCLK = EXTCLK / ( 2 * slow_val );
    } else {
        hdw_state->FCLK = hdw_state->MCLK;
    }

    if ( power->clkdivn & 4 ) {
        hdw_state->HCLK = hdw_state->FCLK / 4;
        hdw_state->PCLK = hdw_state->FCLK / 4;
        hdw_state->PCLK_ratio = 4;
    } else {
        switch ( power->clkdivn & 3 ) {
            case 0:
                hdw_state->HCLK = hdw_state->FCLK;
                hdw_state->PCLK = hdw_state->HCLK;
                hdw_state->PCLK_ratio = 1;
                break;
            case 1:
                hdw_state->HCLK = hdw_state->FCLK;
                hdw_state->PCLK = hdw_state->HCLK / 2;
                hdw_state->PCLK_ratio = 2;
                break;
            case 2:
                hdw_state->HCLK = hdw_state->FCLK / 2;
                hdw_state->PCLK = hdw_state->HCLK;
                hdw_state->PCLK_ratio = 2;
                break;
            case 3:
                hdw_state->HCLK = hdw_state->FCLK / 2;
                hdw_state->PCLK = hdw_state->HCLK / 2;
                hdw_state->PCLK_ratio = 4;
                break;
        }
    }

#ifdef DEBUG_S3C2410_POWER
    printf( "%s: EXTCLK %u, mdiv %u, pdiv %u, sdiv %u: MCLK %u\n", __func__, EXTCLK, mMdiv, mPdiv, mSdiv, hdw_state->MCLK );
    printf( "%s: EXTCLK %u, mdiv %u, pdiv %u, sdiv %u: UCLK %u\n", __func__, EXTCLK, uMdiv, uPdiv, uSdiv, hdw_state->UCLK );
    printf( "%s: FCLK %s: %u\n", __func__, slow_bit ? "(slow)" : "", hdw_state->FCLK );
    printf( "%s: HCLK %u, PCLK %u\n", __func__, hdw_state->HCLK, hdw_state->PCLK );
#endif
}

static int s3c2410_power_load( hdw_module_t* module, GKeyFile* key )
{
    s3c2410_power_t* power = module->user_data;
    s3c2410_offset_t* reg;
    int error = 0;
    unsigned int i;

#ifdef DEBUG_X50NG_MODULES
    printf( "%s: %s:%u\n", module->name, __func__, __LINE__ );
#endif

    for ( i = 0; i < power->nr_regs; i++ ) {
        reg = &power->regs[ i ];

        if ( NULL == reg->name )
            continue;

        if ( module_get_u32( module, key, reg->name, reg->reset, reg->datap ) )
            error = -EAGAIN;
    }

    if ( error ) {
        return error;
    }

    s3c2410_power_write( power, S3C2410_POWER_BASE | S3C2410_POWER_CLKDIVN, power->clkdivn );
    return 0;
}

static int s3c2410_power_save( hdw_module_t* module, GKeyFile* key )
{
    s3c2410_power_t* power = module->user_data;
    s3c2410_offset_t* reg;
    unsigned int i;

#ifdef DEBUG_X50NG_MODULES
    printf( "%s: %s:%u\n", module->name, __func__, __LINE__ );
#endif

    for ( i = 0; i < power->nr_regs; i++ ) {
        reg = &power->regs[ i ];

        if ( NULL == reg->name )
            continue;

        module_set_u32( module, key, reg->name, *( reg->datap ) );
    }

    return 0;
}

static int s3c2410_power_reset( hdw_module_t* module, hdw_reset_t reset )
{
    s3c2410_power_t* power = module->user_data;
    s3c2410_offset_t* reg;
    unsigned int i;

#ifdef DEBUG_X50NG_MODULES
    printf( "%s: %s:%u\n", module->name, __func__, __LINE__ );
#endif

    for ( i = 0; i < power->nr_regs; i++ ) {
        reg = &power->regs[ i ];

        if ( NULL == reg->name )
            continue;

        *( reg->datap ) = reg->reset;
    }

    return 0;
}

static CPUReadMemoryFunc* s3c2410_power_readfn[] = { s3c2410_power_read, s3c2410_power_read, s3c2410_power_read };

static CPUWriteMemoryFunc* s3c2410_power_writefn[] = { s3c2410_power_write, s3c2410_power_write, s3c2410_power_write };

static int s3c2410_power_init( hdw_module_t* module )
{
    s3c2410_power_t* power;
    int iotype;

#ifdef DEBUG_X50NG_MODULES
    printf( "%s: %s:%u\n", module->name, __func__, __LINE__ );
#endif

    power = malloc( sizeof( s3c2410_power_t ) );
    if ( NULL == power ) {
        fprintf( stderr, "%s:%u: Out of memory\n", __func__, __LINE__ );
        return -ENOMEM;
    }
    if ( s3c2410_power_data_init( power ) ) {
        free( power );
        return -ENOMEM;
    }

    module->user_data = power;
    power->hdw_state = module->hdw_state;

    iotype = cpu_register_io_memory( s3c2410_power_readfn, s3c2410_power_writefn, power );
#ifdef DEBUG_S3C2410_POWER
    printf( "%s: iotype %08x\n", __func__, iotype );
#endif
    cpu_register_physical_memory( S3C2410_POWER_BASE, S3C2410_MAP_SIZE, iotype );
    return 0;
}

static int s3c2410_power_exit( hdw_module_t* module )
{
    s3c2410_power_t* power;

#ifdef DEBUG_X50NG_MODULES
    printf( "%s: %s:%u\n", module->name, __func__, __LINE__ );
#endif

    if ( module->user_data ) {
        power = module->user_data;
        if ( power->regs )
            free( power->regs );
        free( power );
    }

    module_unregister( module );
    free( module );

    return 0;
}

int s3c2410_init_module_power( hdw_t* hdw_state )
{
    hdw_module_t* module;

    if ( x50ng_module_init( hdw_state, "s3c2410-power", s3c2410_power_init, s3c2410_power_exit, s3c2410_power_reset, s3c2410_power_load,
                            s3c2410_power_save, NULL, &module ) )
        return -1;

    return module_register( module );
}
