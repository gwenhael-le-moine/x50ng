#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "x50ng.h"
#include "s3c2410.h"
#include "s3c2410_intc.h"

typedef struct {
    uint32_t spicon0;
    uint32_t spista0;
    uint32_t sppin0;
    uint32_t sppre0;
    uint32_t sptdat0;
    uint32_t sprdat0;
    uint32_t spicon1;
    uint32_t spista1;
    uint32_t sppin1;
    uint32_t sppre1;
    uint32_t sptdat1;
    uint32_t sprdat1;

    unsigned int nr_regs;
    s3c2410_offset_t* regs;

    x50ng_t* x50ng;
} s3c2410_spi_t;

static int s3c2410_spi_data_init( s3c2410_spi_t* spi )
{
    s3c2410_offset_t regs[] = {
        S3C2410_OFFSET( SPI, SPICON0, 0x00000000, spi->spicon0 ), S3C2410_OFFSET( SPI, SPISTA0, 0x00000000, spi->spista0 ),
        S3C2410_OFFSET( SPI, SPPIN0, 0x00000000, spi->sppin0 ),   S3C2410_OFFSET( SPI, SPPRE0, 0x00000000, spi->sppre0 ),
        S3C2410_OFFSET( SPI, SPTDAT0, 0x00000000, spi->sptdat0 ), S3C2410_OFFSET( SPI, SPRDAT0, 0x00000000, spi->sprdat0 ),
        S3C2410_OFFSET( SPI, SPICON1, 0x00000000, spi->spicon1 ), S3C2410_OFFSET( SPI, SPISTA1, 0x00000000, spi->spista1 ),
        S3C2410_OFFSET( SPI, SPPIN1, 0x00000000, spi->sppin1 ),   S3C2410_OFFSET( SPI, SPPRE1, 0x00000000, spi->sppre1 ),
        S3C2410_OFFSET( SPI, SPTDAT1, 0x00000000, spi->sptdat1 ), S3C2410_OFFSET( SPI, SPRDAT1, 0x00000000, spi->sprdat1 ),
    };

    memset( spi, 0, sizeof( s3c2410_spi_t ) );

    spi->regs = malloc( sizeof( regs ) );
    if ( NULL == spi->regs ) {
        fprintf( stderr, "%s:%u: Out of memory\n", __FUNCTION__, __LINE__ );
        return -ENOMEM;
    }

    memcpy( spi->regs, regs, sizeof( regs ) );
    spi->nr_regs = sizeof( regs ) / sizeof( regs[ 0 ] );

    return 0;
}

uint32_t s3c2410_spi_read( void* opaque, target_phys_addr_t offset )
{
    s3c2410_spi_t* spi = opaque;
    s3c2410_offset_t* reg;

    if ( !S3C2410_OFFSET_OK( spi, offset ) ) {
        return ~( 0 );
    }

    reg = S3C2410_OFFSET_ENTRY( spi, offset );

#ifdef DEBUG_S3C2410_SPI
    printf( "read  %s [%08x] %s [%08lx] data %08x\n", "s3c2410-spi", S3C2410_SPI_BASE, reg->name, ( unsigned long )offset,
            *( reg->datap ) );
#endif

    switch ( offset ) {
        case S3C2410_SPI_SPRDAT0:
            spi->spista0 &= ~( 1 );
            break;

        case S3C2410_SPI_SPRDAT1:
            spi->spista1 &= ~( 1 );
            break;
    }

    return *( reg->datap );
}

void s3c2410_spi_write( void* opaque, target_phys_addr_t offset, uint32_t data )
{
    s3c2410_spi_t* spi = opaque;
    x50ng_t* x50ng = spi->x50ng;
    s3c2410_offset_t* reg;

    if ( !S3C2410_OFFSET_OK( spi, offset ) ) {
        return;
    }

    reg = S3C2410_OFFSET_ENTRY( spi, offset );

#ifdef DEBUG_S3C2410_SPI
    printf( "write %s [%08x] %s [%08lx] data %08x\n", "s3c2410-spi", S3C2410_SPI_BASE, reg->name, ( unsigned long )offset, data );
#endif

    *( reg->datap ) = data;

    switch ( offset ) {
        case S3C2410_SPI_SPTDAT0:
            spi->spista0 |= 1;
            s3c2410_intc_assert( x50ng, INT_SPI0, 0 );
            break;

        case S3C2410_SPI_SPTDAT1:
            spi->spista1 |= 1;
            s3c2410_intc_assert( x50ng, INT_SPI1, 0 );
            break;
    }
}

static int s3c2410_spi_load( x50ng_module_t* module, GKeyFile* key )
{
    s3c2410_spi_t* spi = module->user_data;
    s3c2410_offset_t* reg;
    int error = 0;
    int i;

#ifdef DEBUG_X50NG_MODULES
    printf( "%s: %s:%u\n", module->name, __FUNCTION__, __LINE__ );
#endif

    for ( i = 0; i < spi->nr_regs; i++ ) {
        reg = &spi->regs[ i ];

        if ( NULL == reg->name )
            continue;

        if ( x50ng_module_get_u32( module, key, reg->name, reg->reset, reg->datap ) )
            error = -EAGAIN;
    }

    return error;
}

static int s3c2410_spi_save( x50ng_module_t* module, GKeyFile* key )
{
    s3c2410_spi_t* spi = module->user_data;
    s3c2410_offset_t* reg;
    int i;

#ifdef DEBUG_X50NG_MODULES
    printf( "%s: %s:%u\n", module->name, __FUNCTION__, __LINE__ );
#endif

    for ( i = 0; i < spi->nr_regs; i++ ) {
        reg = &spi->regs[ i ];

        if ( NULL == reg->name )
            continue;

        x50ng_module_set_u32( module, key, reg->name, *( reg->datap ) );
    }

    return 0;
}

static int s3c2410_spi_reset( x50ng_module_t* module, x50ng_reset_t reset )
{
    s3c2410_spi_t* spi = module->user_data;
    s3c2410_offset_t* reg;
    int i;

#ifdef DEBUG_X50NG_MODULES
    printf( "%s: %s:%u\n", module->name, __FUNCTION__, __LINE__ );
#endif

    for ( i = 0; i < spi->nr_regs; i++ ) {
        reg = &spi->regs[ i ];

        if ( NULL == reg->name )
            continue;

        *( reg->datap ) = reg->reset;
    }

    return 0;
}

static CPUReadMemoryFunc* s3c2410_spi_readfn[] = { s3c2410_spi_read, s3c2410_spi_read, s3c2410_spi_read };

static CPUWriteMemoryFunc* s3c2410_spi_writefn[] = { s3c2410_spi_write, s3c2410_spi_write, s3c2410_spi_write };

static int s3c2410_spi_init( x50ng_module_t* module )
{
    s3c2410_spi_t* spi;
    int iotype;

#ifdef DEBUG_X50NG_MODULES
    printf( "%s: %s:%u\n", module->name, __FUNCTION__, __LINE__ );
#endif

    spi = malloc( sizeof( s3c2410_spi_t ) );
    if ( NULL == spi ) {
        fprintf( stderr, "%s:%u: Out of memory\n", __FUNCTION__, __LINE__ );
        return -ENOMEM;
    }
    if ( s3c2410_spi_data_init( spi ) ) {
        free( spi );
        return -ENOMEM;
    }

    module->user_data = spi;
    spi->x50ng = module->x50ng;

    iotype = cpu_register_io_memory( s3c2410_spi_readfn, s3c2410_spi_writefn, spi );
#ifdef DEBUG_S3C2410_SPI
    printf( "%s: iotype %08x\n", __FUNCTION__, iotype );
#endif
    cpu_register_physical_memory( S3C2410_SPI_BASE, S3C2410_MAP_SIZE, iotype );
    return 0;
}

static int s3c2410_spi_exit( x50ng_module_t* module )
{
    s3c2410_spi_t* spi;

#ifdef DEBUG_X50NG_MODULES
    printf( "%s: %s:%u\n", module->name, __FUNCTION__, __LINE__ );
#endif

    if ( module->user_data ) {
        spi = module->user_data;
        if ( spi->regs )
            free( spi->regs );
        free( spi );
    }

    x50ng_module_unregister( module );
    free( module );

    return 0;
}

int x50ng_s3c2410_spi_init( x50ng_t* x50ng )
{
    x50ng_module_t* module;

    if ( x50ng_module_init( x50ng, "s3c2410-spi", s3c2410_spi_init, s3c2410_spi_exit, s3c2410_spi_reset, s3c2410_spi_load, s3c2410_spi_save,
                            NULL, &module ) ) {
        return -1;
    }

    return x50ng_module_register( module );
}
