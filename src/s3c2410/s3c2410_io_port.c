#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "x50ng.h"
#include "s3c2410.h"
#include "s3c2410_intc.h"

typedef struct {
    uint32_t gpacon;
    uint32_t gpadat;

    uint32_t gpbcon;
    uint32_t gpbdat;
    uint32_t gpbup;

    uint32_t gpccon;
    uint32_t gpcdat;
    uint32_t gpcup;

    uint32_t gpdcon;
    uint32_t gpddat;
    uint32_t gpdup;

    uint32_t gpecon;
    uint32_t gpedat;
    uint32_t gpeup;

    uint32_t gpfcon;
    uint32_t gpfdat;
    uint32_t gpfup;

    uint32_t gpgcon;
    uint32_t gpgdat;
    uint32_t gpgup;

    uint32_t gphcon;
    uint32_t gphdat;
    uint32_t gphup;

    uint32_t misccr;
    uint32_t dclkcon;

    uint32_t extint0;
    uint32_t extint1;
    uint32_t extint2;
    uint32_t eintflt0;
    uint32_t eintflt1;
    uint32_t eintflt2;
    uint32_t eintflt3;
    uint32_t eintmask;
    uint32_t eintpend;

    uint32_t gstatus0;
    uint32_t gstatus1;
    uint32_t gstatus2;
    uint32_t gstatus3;
    uint32_t gstatus4;

    unsigned int nr_regs;
    s3c2410_offset_t* regs;

    x50ng_t* x50ng;
} s3c2410_io_port_t;

static int s3c2410_io_port_data_init( s3c2410_io_port_t* io )
{
    s3c2410_offset_t regs[] = {
        S3C2410_OFFSET( IO_PORT, GPACON, 0x007fffff, io->gpacon ),     S3C2410_OFFSET( IO_PORT, GPADAT, 0x00000000, io->gpadat ),

        S3C2410_OFFSET( IO_PORT, GPBCON, 0x00000000, io->gpbcon ),     S3C2410_OFFSET( IO_PORT, GPBDAT, 0x00000000, io->gpbdat ),
        S3C2410_OFFSET( IO_PORT, GPBUP, 0x00000000, io->gpbup ),

        S3C2410_OFFSET( IO_PORT, GPCCON, 0x00000000, io->gpccon ),     S3C2410_OFFSET( IO_PORT, GPCDAT, 0x00000000, io->gpcdat ),
        S3C2410_OFFSET( IO_PORT, GPCUP, 0x00000000, io->gpcup ),

        S3C2410_OFFSET( IO_PORT, GPDCON, 0x00000000, io->gpdcon ),     S3C2410_OFFSET( IO_PORT, GPDDAT, 0x0000038c, io->gpddat ),
        S3C2410_OFFSET( IO_PORT, GPDUP, 0x0000f000, io->gpdup ),

        S3C2410_OFFSET( IO_PORT, GPECON, 0x00000000, io->gpecon ),     S3C2410_OFFSET( IO_PORT, GPEDAT, 0x0000c7c0, io->gpedat ),
        S3C2410_OFFSET( IO_PORT, GPEUP, 0x00000000, io->gpeup ),

        S3C2410_OFFSET( IO_PORT, GPFCON, 0x00000000, io->gpfcon ),     S3C2410_OFFSET( IO_PORT, GPFDAT, 0x00000008, io->gpfdat ),
        S3C2410_OFFSET( IO_PORT, GPFUP, 0x00000000, io->gpfup ),

        S3C2410_OFFSET( IO_PORT, GPGCON, 0x00000000, io->gpgcon ),     S3C2410_OFFSET( IO_PORT, GPGDAT, 0x0000fffe, io->gpgdat ),
        S3C2410_OFFSET( IO_PORT, GPGUP, 0x0000f800, io->gpgup ),

        S3C2410_OFFSET( IO_PORT, GPHCON, 0x00000000, io->gphcon ),     S3C2410_OFFSET( IO_PORT, GPHDAT, 0x00000000, io->gphdat ),
        S3C2410_OFFSET( IO_PORT, GPHUP, 0x00000000, io->gphup ),

        S3C2410_OFFSET( IO_PORT, MISCCR, 0x00010330, io->misccr ),     S3C2410_OFFSET( IO_PORT, DCLKCON, 0x00000000, io->dclkcon ),

        S3C2410_OFFSET( IO_PORT, EXTINT0, 0x00000000, io->extint0 ),   S3C2410_OFFSET( IO_PORT, EXTINT1, 0x00000000, io->extint1 ),
        S3C2410_OFFSET( IO_PORT, EXTINT2, 0x00000000, io->extint2 ),   S3C2410_OFFSET( IO_PORT, EINTFLT0, 0x00000000, io->eintflt0 ),
        S3C2410_OFFSET( IO_PORT, EINTFLT1, 0x00000000, io->eintflt1 ), S3C2410_OFFSET( IO_PORT, EINTFLT2, 0x00000000, io->eintflt2 ),
        S3C2410_OFFSET( IO_PORT, EINTFLT3, 0x00000000, io->eintflt3 ), S3C2410_OFFSET( IO_PORT, EINTMASK, 0x00fffff0, io->eintmask ),
        S3C2410_OFFSET( IO_PORT, EINTPEND, 0x00000000, io->eintpend ),

        S3C2410_OFFSET( IO_PORT, GSTATUS0, 0x00000001, io->gstatus0 ), S3C2410_OFFSET( IO_PORT, GSTATUS1, 0x32410002, io->gstatus1 ),
        S3C2410_OFFSET( IO_PORT, GSTATUS2, 0x00000001, io->gstatus2 ), S3C2410_OFFSET( IO_PORT, GSTATUS3, 0x00000000, io->gstatus3 ),
        S3C2410_OFFSET( IO_PORT, GSTATUS4, 0x00000000, io->gstatus4 ) };

    memset( io, 0, sizeof( s3c2410_io_port_t ) );

    io->regs = malloc( sizeof( regs ) );
    if ( NULL == io->regs ) {
        fprintf( stderr, "%s:%u: Out of memory\n", __func__, __LINE__ );
        return -ENOMEM;
    }

    memcpy( io->regs, regs, sizeof( regs ) );
    io->nr_regs = sizeof( regs ) / sizeof( regs[ 0 ] );

    return 0;
}

static uint32_t s3c2410_scan_keys( x50ng_t* x50ng, uint32_t gpgcon, uint32_t gpgdat )
{
    uint32_t result;
    int col, row;

    result = 0xfffe | ( gpgdat & 1 );

    for ( col = 0; col < 8; col++ ) {
        switch ( ( gpgcon >> ( 2 * ( col + 8 ) ) ) & 3 ) {
            case 0: /* Input */
            case 2: /* Interrupt */
            case 3: /* Reserved */
                break;
            case 1: /* Output */
                result &= ~( 1 << ( col + 8 ) );
                result |= gpgdat & ( 1 << ( col + 8 ) );

                if ( 0 == ( gpgdat & ( 1 << ( col + 8 ) ) ) ) {
                    result &= ~( x50ng->keybycol[ col ] );
                }
                break;
        }
    }

    for ( row = 1; row < 8; row++ ) {
        switch ( ( gpgcon >> ( 2 * row ) ) & 3 ) {
            case 0: /* Input */
            case 2: /* Interrupt */
            case 3: /* Reserved */
                break;
            case 1: /* Output */
                result &= ~( 1 << row );
                result |= gpgdat & ( 1 << row );

                if ( 0 == ( gpgdat & ( 1 << row ) ) ) {
                    result &= ~( x50ng->keybyrow[ row ] << 8 );
                }
                break;
        }
    }

    return result;
}

static uint32_t s3c2410_io_port_read( void* opaque, target_phys_addr_t offset )
{
    s3c2410_io_port_t* io = opaque;
    s3c2410_offset_t* reg;

    if ( !S3C2410_OFFSET_OK( io, offset ) ) {
        fprintf( stderr, "%s:%u: offset %08lx not OK\n", __func__, __LINE__, ( unsigned long )offset );
        abort();
        return ~( 0 );
    }

    reg = S3C2410_OFFSET_ENTRY( io, offset );

    switch ( offset ) {
        case S3C2410_IO_PORT_MISCCR:
            //		if (io->x50ng->arm->NresetSig != LOW) {
            *( reg->datap ) |= 0x00010000;
            //		}
            break;

        case S3C2410_IO_PORT_GPCDAT:
            if ( 0 == ( ( io->gpccon >> 30 ) & 3 ) ) {
                *( reg->datap ) |= 0x8000;
            }
            if ( 0 == ( ( io->gpccon >> 28 ) & 3 ) ) {
                *( reg->datap ) |= 0x4000;
            }
            if ( 0 == ( ( io->gpccon >> 26 ) & 3 ) ) {
                *( reg->datap ) |= 0x2000;
            }
            if ( 0 == ( ( io->gpccon >> 24 ) & 3 ) ) {
                *( reg->datap ) |= 0x1000;
            }
            break;

        case S3C2410_IO_PORT_GPDDAT:
            if ( 0 == ( ( io->gpdcon >> 6 ) & 3 ) ) {
                *( reg->datap ) |= 0x0008;
            }
            break;

        case S3C2410_IO_PORT_GPEDAT:
            if ( 0 == ( ( io->gpecon >> 30 ) & 3 ) ) {
                *( reg->datap ) |= 0x8000;
            }
            if ( 0 == ( ( io->gpecon >> 28 ) & 3 ) ) {
                *( reg->datap ) |= 0x4000;
            }
            break;

        case S3C2410_IO_PORT_GPFDAT:
            if ( 1 != ( ( io->gpfcon >> 6 ) & 3 ) ) {
                *( reg->datap ) |= 0x0008;
            }
            break;

        case S3C2410_IO_PORT_GPGDAT:
            return s3c2410_scan_keys( io->x50ng, io->gpgcon, io->gpgdat );

        case S3C2410_IO_PORT_GPHDAT:
            if ( 0 == ( ( io->gphcon >> 14 ) & 3 ) ) {
                *( reg->datap ) |= 0x80;
            }
            if ( 0 == ( ( io->gphcon >> 12 ) & 3 ) ) {
                *( reg->datap ) &= ~( 0x40 );
            }
            break;
    }

#ifdef DEBUG_S3C2410_IO_PORT
    printf( "read  %s [%08x] %s [%08lx] data %08x\n", "s3c2410-io-port", S3C2410_IO_PORT_BASE, reg->name, ( unsigned long )offset,
            *( reg->datap ) );
#endif

    return *( reg->datap );
}

static void s3c2410_io_port_write( void* opaque, target_phys_addr_t offset, uint32_t data )
{
    s3c2410_io_port_t* io = opaque;
    s3c2410_offset_t* reg;
    uint32_t change;
    static uint32_t lcd_data = 0;

    if ( !S3C2410_OFFSET_OK( io, offset ) ) {
        return;
    }

    reg = S3C2410_OFFSET_ENTRY( io, offset );

#ifdef DEBUG_S3C2410_IO_PORT
    printf( "write %s [%08x] %s [%08lx] data %08x\n", "s3c2410-io-port", S3C2410_IO_PORT_BASE, reg->name, ( unsigned long )offset, data );
#endif

    switch ( offset ) {
        case S3C2410_IO_PORT_GPDDAT:
            change = *( reg->datap ) ^ data;
            *( reg->datap ) = data;

            if ( !( data & 0x200 ) && ( change & 0x200 ) ) {
                lcd_data = 0;
            }

            if ( !( data & 0x200 ) && ( data & 0x2000 ) && ( change & 0x2000 ) ) {
#ifdef DEBUG_S3C2410_IO_PORT
                printf( "IO_PORT GPDDAT: clk0 rise: data %u\n", ( data >> 12 ) & 1 );
#endif
                lcd_data <<= 1;
                lcd_data |= ( data >> 12 ) & 1;
            }

            if ( ( data & 0x200 ) && ( change & 0x200 ) ) {
#ifdef DEBUG_S3C2410_IO_PORT
                printf( "IO_PORT GPDDAT: cs0 rise: data %04x\n", lcd_data );
#endif
            }

            break;

        case S3C2410_IO_PORT_MISCCR:
            *( reg->datap ) = data;
            if ( !( *( reg->datap ) & 0x00010000 ) ) {
                *( reg->datap ) = 0x10330;
                //			if (io->x50ng->arm->NresetSig != LOW) {
                //				io->x50ng->arm->NresetSig = LOW;
                //				io->x50ng->arm->Exception++;
                //			}
            }
            break;

        case S3C2410_IO_PORT_GSTATUS0:
        case S3C2410_IO_PORT_GSTATUS1:
            /* read only */
            break;

        case S3C2410_IO_PORT_GSTATUS2:
            *( reg->datap ) &= ~( data & 7 );
            break;

        case S3C2410_IO_PORT_EINTPEND:
            *( reg->datap ) &= ~( data );

            if ( 0 == ( *( reg->datap ) & 0x000000f0 ) )
                s3c2410_intc_deassert( io->x50ng, EINT4_7 );
            if ( 0 == ( *( reg->datap ) & 0x00ffff00 ) )
                s3c2410_intc_deassert( io->x50ng, EINT8_23 );
            break;

        default:
            *( reg->datap ) = data;
            break;
    }
}

void s3c2410_io_port_g_update( x50ng_t* x50ng, int column, int row, unsigned char columnbit, unsigned char rowbit, uint32_t new_state )
{
    s3c2410_io_port_t* io = x50ng->s3c2410_io_port;
    uint32_t oldvalue, newvalue, change;
    int n;

    oldvalue = s3c2410_scan_keys( x50ng, io->gpgcon, io->gpgdat );

    if ( new_state ) {
        x50ng->keybycol[ column ] |= rowbit;
        x50ng->keybyrow[ row ] |= columnbit;

    } else {
        x50ng->keybycol[ column ] &= ~rowbit;
        x50ng->keybyrow[ row ] &= ~columnbit;
    }

    newvalue = s3c2410_scan_keys( x50ng, io->gpgcon, io->gpgdat );
    change = newvalue ^ oldvalue;

    for ( n = 0; n < 15; ++n ) {

        switch ( ( io->gpgcon >> ( 2 * n ) ) & 3 ) {

            case 2: /* Interrupt */
                {
                    switch ( n + 8 <= 15 ? ( io->extint1 >> ( 4 * n ) ) & 7 : // EINT 8-15
                                 ( io->extint2 >> ( 4 * ( n - 8 ) ) ) & 7     // EINT 16-23
                    ) {
                        case 0: /* Low Level */
                            if ( !( newvalue & ( 1 << n ) ) ) {
                                io->eintpend |= 1 << ( n + 8 );
                                if ( io->eintpend & ~( io->eintmask ) )
                                    s3c2410_intc_assert( x50ng, EINT8_23, 1 );
                            }
                            break;
                        case 1: /* High Level */
                            if ( newvalue & ( 1 << n ) ) {
                                io->eintpend |= 1 << ( n + 8 );
                                if ( io->eintpend & ~( io->eintmask ) )
                                    s3c2410_intc_assert( x50ng, EINT8_23, 1 );
                            }
                            break;
                        case 2: /* Falling Edge */
                        case 3:
                            if ( ( change & ( 1 << n ) ) && !( newvalue & ( 1 << n ) ) ) {
                                io->eintpend |= 1 << ( n + 8 );
                                if ( io->eintpend & ~( io->eintmask ) )
                                    s3c2410_intc_assert( x50ng, EINT8_23, 1 );
                            }
                            break;
                        case 4: /* Rising Edge */
                        case 5:
                            if ( ( change & ( 1 << n ) ) && ( newvalue & ( 1 << n ) ) ) {
                                io->eintpend |= 1 << ( n + 8 );
                                if ( io->eintpend & ~( io->eintmask ) )
                                    s3c2410_intc_assert( x50ng, EINT8_23, 1 );
                            }
                            break;
                        case 6: /* Any Edge */
                        case 7:
                            if ( change & ( 1 << n ) ) {
                                io->eintpend |= 1 << ( n + 8 );
                                if ( io->eintpend & ~( io->eintmask ) )
                                    s3c2410_intc_assert( x50ng, EINT8_23, 1 );
                            }
                            break;
                    }
                }
                break;
            case 0: /* Input */
            case 1: /* Output */
            case 3: /* Reserved */
                break;
        }
    }

    return;
}

void s3c2410_io_port_f_set_bit( x50ng_t* x50ng, int n, uint32_t value )
{
    s3c2410_io_port_t* io = x50ng->s3c2410_io_port;
    uint32_t change;
    int pending, level;

    if ( n > 7 )
        return;

    //	g_mutex_lock(x50ng->memlock);

    change = 0;
    switch ( ( io->gpfcon >> ( 2 * n ) ) & 3 ) {
        case 0: /* Input */
            io->gpfdat &= ~( 1 << n );
            io->gpfdat |= ( value << n );
            goto out;

        case 2: /* Interrupt */
            change = io->gpfdat ^ ( value << n );
            io->gpfdat &= ~( 1 << n );
            io->gpfdat |= ( value << n );
            break;

        case 1: /* Output */
        case 3: /* Reserved */
            goto out;
    }

    pending = -1;
    level = 0;
    switch ( ( io->extint0 >> ( 4 * n ) ) & 7 ) {
        case 0: /* Low Level */
            if ( !( io->gpfdat & ( 1 << n ) ) )
                pending = n;
            level = 1;
            break;

        case 1: /* High Level */
            if ( io->gpfdat & ( 1 << n ) )
                pending = n;
            level = 1;
            break;

        case 2: /* Falling Edge */
        case 3:
            if ( ( change & ( 1 << n ) ) && !( io->gpfdat & ( 1 << n ) ) )
                pending = n;
            break;

        case 4: /* Rising Edge */
        case 5:
            if ( ( change & ( 1 << n ) ) && ( io->gpfdat & ( 1 << n ) ) )
                pending = n;
            break;

        case 6: /* Any Edge */
        case 7:
            if ( change & ( 1 << n ) )
                pending = n;
            break;
    }

    if ( -1 == pending )
        goto out;

    switch ( n ) {
        case 0:
            s3c2410_intc_assert( x50ng, EINT0, level );
            break;
        case 1:
            s3c2410_intc_assert( x50ng, EINT1, level );
            break;
        case 2:
            s3c2410_intc_assert( x50ng, EINT2, level );
            break;
        case 3:
            s3c2410_intc_assert( x50ng, EINT3, level );
            break;
        case 4:
        case 5:
        case 6:
        case 7:
            io->eintpend |= ( 1 << n );
            if ( io->eintpend & ~( io->eintmask ) )
                s3c2410_intc_assert( x50ng, EINT4_7, 1 );
            break;
    }

out:
    //	g_mutex_unlock(x50ng->memlock);

    return;
}

static int s3c2410_io_port_load( x50ng_module_t* module, GKeyFile* key )
{
    s3c2410_io_port_t* io = module->user_data;
    s3c2410_offset_t* reg;
    int error = 0;
    unsigned int i;

#ifdef DEBUG_X50NG_MODULES
    printf( "%s: %s:%u\n", module->name, __func__, __LINE__ );
#endif

    for ( i = 0; i < io->nr_regs; i++ ) {
        reg = &io->regs[ i ];

        if ( NULL == reg->name )
            continue;

        if ( x50ng_module_get_u32( module, key, reg->name, reg->reset, reg->datap ) )
            error = -EAGAIN;
    }

    return error;
}

static int s3c2410_io_port_save( x50ng_module_t* module, GKeyFile* key )
{
    s3c2410_io_port_t* io = module->user_data;
    s3c2410_offset_t* reg;
    unsigned int i;

#ifdef DEBUG_X50NG_MODULES
    printf( "%s: %s:%u\n", module->name, __func__, __LINE__ );
#endif

    for ( i = 0; i < io->nr_regs; i++ ) {
        reg = &io->regs[ i ];

        if ( NULL == reg->name )
            continue;

        x50ng_module_set_u32( module, key, reg->name, *( reg->datap ) );
    }

    return 0;
}

static int s3c2410_io_port_reset( x50ng_module_t* module, x50ng_reset_t reset )
{
    s3c2410_io_port_t* io = module->user_data;
    s3c2410_offset_t* reg;
    unsigned int i;

#ifdef DEBUG_X50NG_MODULES
    printf( "%s: %s:%u\n", module->name, __func__, __LINE__ );
#endif

    if ( reset == X50NG_RESET_POWER_OFF ) {
        io->gstatus2 = 2;
        return 0;
    }

    for ( i = 0; i < io->nr_regs; i++ ) {
        reg = &io->regs[ i ];

        if ( NULL == reg->name )
            continue;

        *( reg->datap ) = reg->reset;
    }

    if ( reset == X50NG_RESET_WATCHDOG ) {
        io->gstatus2 = 4;
    }

    return 0;
}

static CPUReadMemoryFunc* s3c2410_io_port_readfn[] = { s3c2410_io_port_read, s3c2410_io_port_read, s3c2410_io_port_read };

static CPUWriteMemoryFunc* s3c2410_io_port_writefn[] = { s3c2410_io_port_write, s3c2410_io_port_write, s3c2410_io_port_write };

static int s3c2410_io_port_init( x50ng_module_t* module )
{
    s3c2410_io_port_t* io;
    int iotype;

#ifdef DEBUG_X50NG_MODULES
    printf( "%s: %s:%u\n", module->name, __func__, __LINE__ );
#endif

    io = malloc( sizeof( s3c2410_io_port_t ) );
    if ( NULL == io ) {
        fprintf( stderr, "%s:%u: Out of memory\n", __func__, __LINE__ );
        return -ENOMEM;
    }
    if ( s3c2410_io_port_data_init( io ) ) {
        free( io );
        return -ENOMEM;
    }

    module->user_data = io;
    module->x50ng->s3c2410_io_port = io;
    io->x50ng = module->x50ng;

    iotype = cpu_register_io_memory( s3c2410_io_port_readfn, s3c2410_io_port_writefn, io );
#ifdef DEBUG_S3C2410_IO_PORT
    printf( "%s: iotype %08x\n", __func__, iotype );
#endif
    cpu_register_physical_memory( S3C2410_IO_PORT_BASE, S3C2410_MAP_SIZE, iotype );
    return 0;
}

static int s3c2410_io_port_exit( x50ng_module_t* module )
{
    s3c2410_io_port_t* io;

#ifdef DEBUG_X50NG_MODULES
    printf( "%s: %s:%u\n", module->name, __func__, __LINE__ );
#endif

    if ( module->user_data ) {
        io = module->user_data;
        if ( io->regs )
            free( io->regs );
        free( io );
    }

    x50ng_module_unregister( module );
    free( module );

    return 0;
}

int x50ng_s3c2410_io_port_init( x50ng_t* x50ng )
{
    x50ng_module_t* module;

    if ( x50ng_module_init( x50ng, "s3c2410-io-port", s3c2410_io_port_init, s3c2410_io_port_exit, s3c2410_io_port_reset,
                            s3c2410_io_port_load, s3c2410_io_port_save, NULL, &module ) )
        return -1;

    return x50ng_module_register( module );
}
