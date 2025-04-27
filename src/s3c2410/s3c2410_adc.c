#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "x50ng.h"
#include "s3c2410.h"

typedef struct {
    uint32_t adccon;
    uint32_t adctsc;
    uint32_t adcdly;
    uint32_t adcdat0;
    uint32_t adcdat1;

    unsigned int nr_regs;
    s3c2410_offset_t* regs;
} s3c2410_adc_t;

static int s3c2410_adc_data_init( s3c2410_adc_t* adc )
{
    s3c2410_offset_t regs[] = {
        S3C2410_OFFSET( ADC, ADCCON, 0x00003fc4, adc->adccon ), S3C2410_OFFSET( ADC, ADCTSC, 0x00000058, adc->adctsc ),
        S3C2410_OFFSET( ADC, ADCDLY, 0x000000ff, adc->adcdly ), S3C2410_OFFSET( ADC, ADCDAT0, 0x3ff, adc->adcdat0 ),
        S3C2410_OFFSET( ADC, ADCDAT1, 0x3ff, adc->adcdat1 ),
    };

    memset( adc, 0, sizeof( s3c2410_adc_t ) );

    adc->regs = malloc( sizeof( regs ) );
    if ( NULL == adc->regs ) {
        fprintf( stderr, "%s:%u: Out of memory\n", __FUNCTION__, __LINE__ );
        return -ENOMEM;
    }

    memcpy( adc->regs, regs, sizeof( regs ) );
    adc->nr_regs = sizeof( regs ) / sizeof( regs[ 0 ] );

    return 0;
}

static uint32_t s3c2410_adc_read( void* opaque, target_phys_addr_t offset )
{
    s3c2410_adc_t* adc = opaque;
    s3c2410_offset_t* reg;

    if ( !S3C2410_OFFSET_OK( adc, offset ) ) {
        return ~( 0 );
    }

    reg = S3C2410_OFFSET_ENTRY( adc, offset );

#ifdef DEBUG_S3C2410_ADC
    printf( "read  %s [%08x] %s [%08lx] data %08x\n", "s3c2410-adc", S3C2410_ADC_BASE, reg->name, ( unsigned long )offset,
            *( reg->datap ) );
#endif

    switch ( offset ) {
        case S3C2410_ADC_ADCCON:
            *( reg->datap ) &= ~( 0x0001 );
            *( reg->datap ) |= 0x8000;
            break;
        default:
            break;
    }

    return *( reg->datap );
}

static void s3c2410_adc_write( void* opaque, target_phys_addr_t offset, uint32_t data )
{
    s3c2410_adc_t* adc = opaque;
    s3c2410_offset_t* reg;

    if ( !S3C2410_OFFSET_OK( adc, offset ) ) {
        return;
    }

    reg = S3C2410_OFFSET_ENTRY( adc, offset );

#ifdef DEBUG_S3C2410_ADC
    printf( "write %s [%08x] %s [%08lx] data %08x\n", "s3c2410-adc", S3C2410_ADC_BASE, reg->name, ( unsigned long )offset, data );
#endif

    *( reg->datap ) = data;
}

static int s3c2410_adc_load( x50ng_module_t* module, GKeyFile* key )
{
    s3c2410_adc_t* adc = module->user_data;
    s3c2410_offset_t* reg;
    int error = 0;
    unsigned int i;

#ifdef DEBUG_X50NG_MODULES
    printf( "%s: %s:%u\n", module->name, __FUNCTION__, __LINE__ );
#endif

    for ( i = 0; i < adc->nr_regs; i++ ) {
        reg = &adc->regs[ i ];

        if ( NULL == reg->name )
            continue;

        if ( x50ng_module_get_u32( module, key, reg->name, reg->reset, reg->datap ) )
            error = -EAGAIN;
    }

    return error;
}

static int s3c2410_adc_save( x50ng_module_t* module, GKeyFile* key )
{
    s3c2410_adc_t* adc = module->user_data;
    s3c2410_offset_t* reg;
    unsigned int i;

#ifdef DEBUG_X50NG_MODULES
    printf( "%s: %s:%u\n", module->name, __FUNCTION__, __LINE__ );
#endif

    for ( i = 0; i < adc->nr_regs; i++ ) {
        reg = &adc->regs[ i ];

        if ( NULL == reg->name )
            continue;

        x50ng_module_set_u32( module, key, reg->name, *( reg->datap ) );
    }

    return 0;
}

static int s3c2410_adc_reset( x50ng_module_t* module, x50ng_reset_t reset )
{
    s3c2410_adc_t* adc = module->user_data;
    s3c2410_offset_t* reg;
    unsigned int i;

#ifdef DEBUG_X50NG_MODULES
    printf( "%s: %s:%u\n", module->name, __FUNCTION__, __LINE__ );
#endif

    for ( i = 0; i < adc->nr_regs; i++ ) {
        reg = &adc->regs[ i ];

        if ( NULL == reg->name )
            continue;

        *( reg->datap ) = reg->reset;
    }

    return 0;
}

static CPUReadMemoryFunc* s3c2410_adc_readfn[] = { s3c2410_adc_read, s3c2410_adc_read, s3c2410_adc_read };

static CPUWriteMemoryFunc* s3c2410_adc_writefn[] = { s3c2410_adc_write, s3c2410_adc_write, s3c2410_adc_write };

static int s3c2410_adc_init( x50ng_module_t* module )
{
    s3c2410_adc_t* adc;
    int iotype;

#ifdef DEBUG_X50NG_MODULES
    printf( "%s: %s:%u\n", module->name, __FUNCTION__, __LINE__ );
#endif

    adc = malloc( sizeof( s3c2410_adc_t ) );
    if ( NULL == adc ) {
        fprintf( stderr, "%s:%u: Out of memory\n", __FUNCTION__, __LINE__ );
        return -ENOMEM;
    }
    if ( s3c2410_adc_data_init( adc ) ) {
        free( adc );
        return -ENOMEM;
    }

    module->user_data = adc;

    iotype = cpu_register_io_memory( s3c2410_adc_readfn, s3c2410_adc_writefn, adc );
#ifdef DEBUG_S3C2410_ADC
    printf( "%s: iotype %08x\n", __FUNCTION__, iotype );
#endif
    cpu_register_physical_memory( S3C2410_ADC_BASE, S3C2410_MAP_SIZE, iotype );

    return 0;
}

static int s3c2410_adc_exit( x50ng_module_t* module )
{
    s3c2410_adc_t* adc;

#ifdef DEBUG_X50NG_MODULES
    printf( "%s: %s:%u\n", module->name, __FUNCTION__, __LINE__ );
#endif

    if ( module->user_data ) {
        adc = module->user_data;
        if ( adc->regs )
            free( adc->regs );
        free( adc );
    }

    x50ng_module_unregister( module );
    free( module );

    return 0;
}

int x50ng_s3c2410_adc_init( x50ng_t* x50ng )
{
    x50ng_module_t* module;

    if ( x50ng_module_init( x50ng, "s3c2410-adc", s3c2410_adc_init, s3c2410_adc_exit, s3c2410_adc_reset, s3c2410_adc_load, s3c2410_adc_save,
                            NULL, &module ) )
        return -1;

    return x50ng_module_register( module );
}
