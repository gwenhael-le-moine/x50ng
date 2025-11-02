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
    uint32_t ulcon;
    uint32_t ucon;
    uint32_t ufcon;
    uint32_t umcon;
    uint32_t utrstat;
    uint32_t uerstat;
    uint32_t ufstat;
    uint32_t umstat;
    uint32_t utxh;
    uint32_t urxh;
    uint32_t ubrdiv;

    int int_err;
    int int_txd;
    int int_rxd;

    unsigned int nr_regs;
    s3c2410_offset_t* regs;

    hdw_t* x50ng;
} s3c2410_uart_reg_t;

typedef struct {
    s3c2410_uart_reg_t uart[ 3 ];
} s3c2410_uart_t;

static int s3c2410_uart_data_init( s3c2410_uart_t* uart )
{
    s3c2410_offset_t regs0[] = { S3C2410_OFFSET( UART0, ULCON, 0x00000000, uart->uart[ 0 ].ulcon ),
                                 S3C2410_OFFSET( UART0, UCON, 0x00000000, uart->uart[ 0 ].ucon ),
                                 S3C2410_OFFSET( UART0, UFCON, 0x00000000, uart->uart[ 0 ].ufcon ),
                                 S3C2410_OFFSET( UART0, UMCON, 0x00000000, uart->uart[ 0 ].umcon ),
                                 S3C2410_OFFSET( UART0, UTRSTAT, 0x00000006, uart->uart[ 0 ].utrstat ),
                                 S3C2410_OFFSET( UART0, UERSTAT, 0x00000000, uart->uart[ 0 ].uerstat ),
                                 S3C2410_OFFSET( UART0, UFSTAT, 0x00000000, uart->uart[ 0 ].ufstat ),
                                 S3C2410_OFFSET( UART0, UMSTAT, 0x00000000, uart->uart[ 0 ].umstat ),
                                 S3C2410_OFFSET( UART0, UTXH, 0, uart->uart[ 0 ].utxh ),
                                 S3C2410_OFFSET( UART0, URXH, 0, uart->uart[ 0 ].urxh ),
                                 S3C2410_OFFSET( UART0, UBRDIV, 0, uart->uart[ 0 ].ubrdiv ) };
    s3c2410_offset_t regs1[] = { S3C2410_OFFSET( UART1, ULCON, 0x00000000, uart->uart[ 1 ].ulcon ),
                                 S3C2410_OFFSET( UART1, UCON, 0x00000000, uart->uart[ 1 ].ucon ),
                                 S3C2410_OFFSET( UART1, UFCON, 0x00000000, uart->uart[ 1 ].ufcon ),
                                 S3C2410_OFFSET( UART1, UMCON, 0x00000000, uart->uart[ 1 ].umcon ),
                                 S3C2410_OFFSET( UART1, UTRSTAT, 0x00000006, uart->uart[ 1 ].utrstat ),
                                 S3C2410_OFFSET( UART1, UERSTAT, 0x00000000, uart->uart[ 1 ].uerstat ),
                                 S3C2410_OFFSET( UART1, UFSTAT, 0x00000000, uart->uart[ 1 ].ufstat ),
                                 S3C2410_OFFSET( UART1, UMSTAT, 0x00000000, uart->uart[ 1 ].umstat ),
                                 S3C2410_OFFSET( UART1, UTXH, 0, uart->uart[ 1 ].utxh ),
                                 S3C2410_OFFSET( UART1, URXH, 0, uart->uart[ 1 ].urxh ),
                                 S3C2410_OFFSET( UART1, UBRDIV, 0, uart->uart[ 1 ].ubrdiv ) };
    s3c2410_offset_t regs2[] = { S3C2410_OFFSET( UART2, ULCON, 0x00000000, uart->uart[ 2 ].ulcon ),
                                 S3C2410_OFFSET( UART2, UCON, 0x00000000, uart->uart[ 2 ].ucon ),
                                 S3C2410_OFFSET( UART2, UFCON, 0x00000000, uart->uart[ 2 ].ufcon ),
                                 S3C2410_OFFSET( UART2, UTRSTAT, 0x00000006, uart->uart[ 2 ].utrstat ),
                                 S3C2410_OFFSET( UART2, UERSTAT, 0x00000000, uart->uart[ 2 ].uerstat ),
                                 S3C2410_OFFSET( UART2, UFSTAT, 0x00000000, uart->uart[ 2 ].ufstat ),
                                 S3C2410_OFFSET( UART2, UTXH, 0, uart->uart[ 2 ].utxh ),
                                 S3C2410_OFFSET( UART2, URXH, 0, uart->uart[ 2 ].urxh ),
                                 S3C2410_OFFSET( UART2, UBRDIV, 0, uart->uart[ 2 ].ubrdiv ) };

    uart->uart[ 0 ].regs = malloc( sizeof( regs0 ) );
    if ( NULL == uart->uart[ 0 ].regs ) {
        fprintf( stderr, "%s:%u: Out of memory\n", __func__, __LINE__ );
        return -ENOMEM;
    }
    uart->uart[ 1 ].regs = malloc( sizeof( regs1 ) );
    if ( NULL == uart->uart[ 1 ].regs ) {
        fprintf( stderr, "%s:%u: Out of memory\n", __func__, __LINE__ );
        free( uart->uart[ 0 ].regs );
        return -ENOMEM;
    }
    uart->uart[ 2 ].regs = malloc( sizeof( regs2 ) );
    if ( NULL == uart->uart[ 2 ].regs ) {
        fprintf( stderr, "%s:%u: Out of memory\n", __func__, __LINE__ );
        free( uart->uart[ 0 ].regs );
        free( uart->uart[ 1 ].regs );
        return -ENOMEM;
    }

    memcpy( uart->uart[ 0 ].regs, regs0, sizeof( regs0 ) );
    uart->uart[ 0 ].nr_regs = sizeof( regs0 ) / sizeof( regs0[ 0 ] );
    uart->uart[ 0 ].int_err = SUB_INT_ERR0;
    uart->uart[ 0 ].int_txd = SUB_INT_TXD0;
    uart->uart[ 0 ].int_rxd = SUB_INT_RXD0;

    memcpy( uart->uart[ 1 ].regs, regs1, sizeof( regs1 ) );
    uart->uart[ 1 ].nr_regs = sizeof( regs1 ) / sizeof( regs1[ 0 ] );
    uart->uart[ 1 ].int_err = SUB_INT_ERR1;
    uart->uart[ 1 ].int_txd = SUB_INT_TXD1;
    uart->uart[ 1 ].int_rxd = SUB_INT_RXD1;

    memcpy( uart->uart[ 2 ].regs, regs2, sizeof( regs2 ) );
    uart->uart[ 2 ].nr_regs = sizeof( regs2 ) / sizeof( regs2[ 0 ] );
    uart->uart[ 2 ].int_err = SUB_INT_ERR2;
    uart->uart[ 2 ].int_txd = SUB_INT_TXD2;
    uart->uart[ 2 ].int_rxd = SUB_INT_RXD2;

    return 0;
}

static uint32_t s3c2410_uart_read( void* opaque, target_phys_addr_t offset )
{
    s3c2410_uart_reg_t* uart_regs = opaque;
    hdw_t* x50ng = uart_regs->x50ng;
    s3c2410_offset_t* reg;
#ifdef DEBUG_S3C2410_UART
    const char* module;
    uint32_t mod_offset;
    uint32_t base;

    base = ( offset & 0x0000c000 ) >> 14;

    switch ( base ) {
        case 0:
            module = "s3c2410-uart0";
            mod_offset = S3C2410_UART0_BASE;
            break;
        case 1:
            module = "s3c2410-uart1";
            mod_offset = S3C2410_UART1_BASE;
            break;
        case 2:
            module = "s3c2410-uart2";
            mod_offset = S3C2410_UART2_BASE;
            break;
        default:
            return ~( 0 );
    }
#endif

    offset &= ~( 0xffffc000 );
    if ( !S3C2410_OFFSET_OK( uart_regs, offset ) ) {
        return ~( 0 );
    }

    reg = S3C2410_OFFSET_ENTRY( uart_regs, offset );

#ifdef DEBUG_S3C2410_UART
    printf( "read  %s [%08x] %s [%08lx] data %08x\n", module, mod_offset, reg->name, ( unsigned long )offset, *( reg->datap ) );
#endif

    switch ( offset ) {
        case S3C2410_UART0_URXH:
            uart_regs->utrstat &= ~( 1 << 0 );

            if ( uart_regs->ucon & ( 1 << 8 ) ) {
                s3c2410_intc_sub_deassert( x50ng, uart_regs->int_rxd );
            }

            break;
    }

    return *( reg->datap );
}

static void s3c2410_uart_write( void* opaque, target_phys_addr_t offset, uint32_t data )
{
    s3c2410_uart_reg_t* uart_regs = opaque;
    hdw_t* x50ng = uart_regs->x50ng;
    s3c2410_offset_t* reg;
    uint32_t base;
#ifdef DEBUG_S3C2410_UART
    const char* module;
    uint32_t mod_offset, ubrdivn, baud;
#endif

    base = ( offset & 0x0000c000 ) >> 14;

#ifdef DEBUG_S3C2410_UART
    switch ( base ) {
        case 0:
            module = "s3c2410-uart0";
            mod_offset = S3C2410_UART0_BASE;
            break;
        case 1:
            module = "s3c2410-uart1";
            mod_offset = S3C2410_UART1_BASE;
            break;
        case 2:
            module = "s3c2410-uart2";
            mod_offset = S3C2410_UART2_BASE;
            break;
        default:
            return;
    }
#endif

    offset &= ~( 0xffffc000 );
    if ( !S3C2410_OFFSET_OK( uart_regs, offset ) ) {
        return;
    }

    reg = S3C2410_OFFSET_ENTRY( uart_regs, offset );

#ifdef DEBUG_S3C2410_UART
    printf( "write %s [%08x] %s [%08lx] data %08x\n", module, mod_offset, reg->name, ( unsigned long )offset, data );
#endif

    *( reg->datap ) = data;

    switch ( offset ) {
        case S3C2410_UART0_UCON:
            if ( *( reg->datap ) & ( 1 << 9 ) )
                s3c2410_intc_sub_assert( x50ng, uart_regs->int_txd, 1 );
            if ( *( reg->datap ) & ( 1 << 8 ) )
                s3c2410_intc_sub_deassert( x50ng, uart_regs->int_rxd );
            break;

        case S3C2410_UART0_UBRDIV:
#ifdef DEBUG_S3C2410_UART
            ubrdivn = ( data >> 0 ) & 0xffff;
            if ( uart_regs->ucon & ( 1 << 10 ) ) {
                baud = x50ng->UCLK / 16 / ( ubrdivn + 1 );
                printf( "%s: UEXTCLK %u, ubrdivn %u, baud %u\n", module, x50ng->UCLK, ubrdivn, baud );
            } else {
                baud = x50ng->PCLK / 16 / ( ubrdivn + 1 );
                printf( "%s: PCLK %u, ubrdivn %u, baud %u\n", module, x50ng->PCLK, ubrdivn, baud );
            }
#endif
            break;

        case S3C2410_UART0_UTXH:
            if ( uart_regs->ucon & ( 1 << 9 ) )
                s3c2410_intc_sub_deassert( x50ng, uart_regs->int_txd );

            uart_regs->utrstat |= ( 1 << 2 ) | ( 1 << 1 );

            if ( uart_regs->ucon & ( 1 << 9 ) )
                s3c2410_intc_sub_assert( x50ng, uart_regs->int_txd, 1 );
            else
                s3c2410_intc_sub_assert( x50ng, uart_regs->int_txd, 0 );

            if ( uart_regs->ucon & ( 1 << 5 ) ) {
                uart_regs->urxh = data;
                uart_regs->utrstat |= ( 1 << 0 );

                if ( uart_regs->ucon & ( 1 << 8 ) )
                    s3c2410_intc_sub_assert( x50ng, uart_regs->int_rxd, 1 );
                else
                    s3c2410_intc_sub_assert( x50ng, uart_regs->int_rxd, 0 );
            } else if ( base == 2 ) {
                uart_regs->urxh = data;
                uart_regs->utrstat |= ( 1 << 0 );

                if ( uart_regs->ucon & ( 1 << 8 ) )
                    s3c2410_intc_sub_assert( x50ng, uart_regs->int_rxd, 1 );
                else
                    s3c2410_intc_sub_assert( x50ng, uart_regs->int_rxd, 0 );
            }

            break;
    }
}

static int s3c2410_uart_load( hdw_module_t* module, GKeyFile* key )
{
    s3c2410_uart_reg_t* uart_regs = module->user_data;
    s3c2410_offset_t* reg;
    int error = 0;
    unsigned int i;

#ifdef DEBUG_X50NG_MODULES
    printf( "%s: %s:%u\n", module->name, __func__, __LINE__ );
#endif

    for ( i = 0; i < uart_regs->nr_regs; i++ ) {
        reg = &uart_regs->regs[ i ];

        if ( NULL == reg->name )
            continue;

        if ( x50ng_module_get_u32( module, key, reg->name, reg->reset, reg->datap ) )
            error = -EAGAIN;
    }

    return error;
}

static int s3c2410_uart_save( hdw_module_t* module, GKeyFile* key )
{
    s3c2410_uart_reg_t* uart_regs = module->user_data;
    s3c2410_offset_t* reg;
    unsigned int i;

#ifdef DEBUG_X50NG_MODULES
    printf( "%s: %s:%u\n", module->name, __func__, __LINE__ );
#endif

    for ( i = 0; i < uart_regs->nr_regs; i++ ) {
        reg = &uart_regs->regs[ i ];

        if ( NULL == reg->name )
            continue;

        x50ng_module_set_u32( module, key, reg->name, *( reg->datap ) );
    }

    return 0;
}

static int s3c2410_uart_reset( hdw_module_t* module, x50ng_reset_t reset )
{
    s3c2410_uart_reg_t* uart_regs = module->user_data;
    s3c2410_offset_t* reg;
    unsigned int i;

#ifdef DEBUG_X50NG_MODULES
    printf( "%s: %s:%u\n", module->name, __func__, __LINE__ );
#endif

    for ( i = 0; i < uart_regs->nr_regs; i++ ) {
        reg = &uart_regs->regs[ i ];

        if ( NULL == reg->name )
            continue;

        *( reg->datap ) = reg->reset;
    }

    return 0;
}

static CPUReadMemoryFunc* s3c2410_uart_readfn[] = { s3c2410_uart_read, s3c2410_uart_read, s3c2410_uart_read };

static CPUWriteMemoryFunc* s3c2410_uart_writefn[] = { s3c2410_uart_write, s3c2410_uart_write, s3c2410_uart_write };

static int s3c2410_uart_init( hdw_module_t* module )
{
    s3c2410_uart_reg_t* uart_regs = module->user_data;
    int iotype;

#ifdef DEBUG_X50NG_MODULES
    printf( "%s: %s:%u\n", module->name, __func__, __LINE__ );
#endif

    iotype = cpu_register_io_memory( s3c2410_uart_readfn, s3c2410_uart_writefn, uart_regs );
#ifdef DEBUG_S3C2410_UART
    printf( "%s: iotype %08x\n", __func__, iotype );
#endif
    cpu_register_physical_memory( S3C2410_UART0_BASE, S3C2410_MAP_SIZE, iotype );

    return 0;
}

static int s3c2410_uart_exit( hdw_module_t* module )
{
    s3c2410_uart_reg_t* uart_regs;

#ifdef DEBUG_X50NG_MODULES
    printf( "%s: %s:%u\n", module->name, __func__, __LINE__ );
#endif

    if ( module->user_data ) {
        uart_regs = module->user_data;
        if ( uart_regs->regs )
            free( uart_regs->regs );
    }

    x50ng_module_unregister( module );
    free( module );

    return 0;
}

int x50ng_s3c2410_uart_init( hdw_t* x50ng )
{
    s3c2410_uart_t* uart;
    hdw_module_t* module;

    uart = malloc( sizeof( s3c2410_uart_t ) );
    if ( NULL == uart ) {
        fprintf( stderr, "%s:%u: Out of memory\n", __func__, __LINE__ );
        return -ENOMEM;
    }
    memset( uart, 0, sizeof( s3c2410_uart_t ) );

    if ( s3c2410_uart_data_init( uart ) ) {
        free( uart );
        return -ENOMEM;
    }

    uart->uart[ 0 ].x50ng = x50ng;
    uart->uart[ 1 ].x50ng = x50ng;
    uart->uart[ 2 ].x50ng = x50ng;

    if ( x50ng_module_init( x50ng, "s3c2410-uart0", s3c2410_uart_init, s3c2410_uart_exit, s3c2410_uart_reset, s3c2410_uart_load,
                            s3c2410_uart_save, &uart->uart[ 0 ], &module ) )
        return -1;

    if ( x50ng_module_register( module ) )
        return -1;

    if ( x50ng_module_init( x50ng, "s3c2410-uart1", s3c2410_uart_init, s3c2410_uart_exit, s3c2410_uart_reset, s3c2410_uart_load,
                            s3c2410_uart_save, &uart->uart[ 1 ], &module ) )
        return -1;

    if ( x50ng_module_register( module ) )
        return -1;

    if ( x50ng_module_init( x50ng, "s3c2410-uart2", s3c2410_uart_init, s3c2410_uart_exit, s3c2410_uart_reset, s3c2410_uart_load,
                            s3c2410_uart_save, &uart->uart[ 2 ], &module ) )
        return -1;

    if ( x50ng_module_register( module ) )
        return -1;

    return 0;
}
