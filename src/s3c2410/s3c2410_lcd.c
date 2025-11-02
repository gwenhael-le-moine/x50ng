#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "../module.h"

#include "s3c2410.h"

static int s3c2410_lcd_data_init( s3c2410_lcd_t* lcd )
{
    s3c2410_offset_t regs[] = {
        S3C2410_OFFSET( LCD, LCDCON1, 0x00000000, lcd->lcdcon1 ),     S3C2410_OFFSET( LCD, LCDCON2, 0x00000000, lcd->lcdcon2 ),
        S3C2410_OFFSET( LCD, LCDCON3, 0x00000000, lcd->lcdcon3 ),     S3C2410_OFFSET( LCD, LCDCON4, 0x00000000, lcd->lcdcon4 ),
        S3C2410_OFFSET( LCD, LCDCON5, 0x00000000, lcd->lcdcon5 ),     S3C2410_OFFSET( LCD, LCDSADDR1, 0x00000000, lcd->lcdsaddr1 ),
        S3C2410_OFFSET( LCD, LCDSADDR2, 0x00000000, lcd->lcdsaddr2 ), S3C2410_OFFSET( LCD, LCDSADDR3, 0x00000000, lcd->lcdsaddr3 ),
        S3C2410_OFFSET( LCD, REDLUT, 0x00000000, lcd->redlut ),       S3C2410_OFFSET( LCD, GREENLUT, 0x00000000, lcd->greenlut ),
        S3C2410_OFFSET( LCD, BLUELUT, 0x00000000, lcd->bluelut ),     S3C2410_OFFSET( LCD, DITHMODE, 0x00000000, lcd->dithmode ),
        S3C2410_OFFSET( LCD, TPAL, 0x00000000, lcd->tpal ),           S3C2410_OFFSET( LCD, LCDINTPND, 0x00000000, lcd->lcdintpnd ),
        S3C2410_OFFSET( LCD, LCDSRCPND, 0x00000000, lcd->lcdsrcpnd ), S3C2410_OFFSET( LCD, LCDINTMSK, 0x00000003, lcd->lcdintmsk ),
        S3C2410_OFFSET( LCD, LPCSEL, 0x00000004, lcd->lpcsel ),       S3C2410_OFFSET( LCD, UNKNOWN_68, 0x00000000, lcd->__unknown_68 ) };

    memset( lcd, 0, sizeof( s3c2410_lcd_t ) );

    lcd->regs = malloc( sizeof( regs ) );
    if ( NULL == lcd->regs ) {
        fprintf( stderr, "%s:%u: Out of memory\n", __func__, __LINE__ );
        return -ENOMEM;
    }

    memcpy( lcd->regs, regs, sizeof( regs ) );
    lcd->nr_regs = sizeof( regs ) / sizeof( regs[ 0 ] );

    return 0;
}

static uint32_t s3c2410_lcd_read( void* opaque, target_phys_addr_t offset )
{
    s3c2410_lcd_t* lcd = opaque;
    s3c2410_offset_t* reg;
    uint32_t linecnt;

    if ( !S3C2410_OFFSET_OK( lcd, offset ) )
        return ~( 0 );

    reg = S3C2410_OFFSET_ENTRY( lcd, offset );

    switch ( offset ) {
        case S3C2410_LCD_LCDCON1:
            linecnt = ( lcd->lcdcon1 >> 18 ) & 0x3ff;
            if ( linecnt > 0 )
                linecnt--;
            else
                linecnt = ( lcd->lcdcon2 >> 14 ) & 0x3ff;

            lcd->lcdcon1 &= ~( 0x3ff << 18 );
            lcd->lcdcon1 |= ( linecnt << 18 );
    }

#ifdef DEBUG_S3C2410_LCD
    printf( "read  %s [%08x] %s [%08lx] data %08x\n", "s3c2410-lcd", S3C2410_LCD_BASE, reg->name, ( unsigned long )offset,
            *( reg->datap ) );
#endif

    return *( reg->datap );
}

static void s3c2410_lcd_write( void* opaque, target_phys_addr_t offset, uint32_t data )
{
    s3c2410_lcd_t* lcd = opaque;
    s3c2410_offset_t* reg;

    if ( !S3C2410_OFFSET_OK( lcd, offset ) )
        return;

    reg = S3C2410_OFFSET_ENTRY( lcd, offset );

#ifdef DEBUG_S3C2410_LCD
    printf( "write %s [%08x] %s [%08lx] data %08x\n", "s3c2410-lcd", S3C2410_LCD_BASE, reg->name, ( unsigned long )offset, data );
#endif

    switch ( offset ) {
        case S3C2410_LCD_LCDCON1:
            lcd->lcdcon1 = ( lcd->lcdcon1 & ( 0x3ff << 18 ) ) | ( data & ~( 0x3ff << 18 ) );
            break;
        default:
            *( reg->datap ) = data;
            break;
    }
}

static int s3c2410_lcd_load( hdw_module_t* module, GKeyFile* key )
{
    s3c2410_lcd_t* lcd = module->user_data;
    s3c2410_offset_t* reg;
    int error = 0;

#ifdef DEBUG_X50NG_MODULES
    printf( "%s: %s:%u\n", module->name, __func__, __LINE__ );
#endif

    for ( unsigned int i = 0; i < lcd->nr_regs; i++ ) {
        reg = &lcd->regs[ i ];

        if ( NULL == reg->name )
            continue;

        if ( module_get_u32( module, key, reg->name, reg->reset, reg->datap ) )
            error = -EAGAIN;
    }

    return error;
}

static int s3c2410_lcd_save( hdw_module_t* module, GKeyFile* key )
{
    s3c2410_lcd_t* lcd = module->user_data;
    s3c2410_offset_t* reg;

#ifdef DEBUG_X50NG_MODULES
    printf( "%s: %s:%u\n", module->name, __func__, __LINE__ );
#endif

    for ( unsigned int i = 0; i < lcd->nr_regs; i++ ) {
        reg = &lcd->regs[ i ];

        if ( NULL == reg->name )
            continue;

        module_set_u32( module, key, reg->name, *( reg->datap ) );
    }

    return 0;
}

static int s3c2410_lcd_reset( hdw_module_t* module, hdw_reset_t reset )
{
    s3c2410_lcd_t* lcd = module->user_data;
    s3c2410_offset_t* reg;

#ifdef DEBUG_X50NG_MODULES
    printf( "%s: %s:%u\n", module->name, __func__, __LINE__ );
#endif

    for ( unsigned int i = 0; i < lcd->nr_regs; i++ ) {
        reg = &lcd->regs[ i ];

        if ( NULL == reg->name )
            continue;

        *( reg->datap ) = reg->reset;
    }

    return 0;
}

static CPUReadMemoryFunc* s3c2410_lcd_readfn[] = { s3c2410_lcd_read, s3c2410_lcd_read, s3c2410_lcd_read };

static CPUWriteMemoryFunc* s3c2410_lcd_writefn[] = { s3c2410_lcd_write, s3c2410_lcd_write, s3c2410_lcd_write };

static int s3c2410_lcd_init( hdw_module_t* module )
{
    s3c2410_lcd_t* lcd;
    int iotype;

#ifdef DEBUG_X50NG_MODULES
    printf( "%s: %s:%u\n", module->name, __func__, __LINE__ );
#endif

    lcd = malloc( sizeof( s3c2410_lcd_t ) );
    if ( NULL == lcd ) {
        fprintf( stderr, "%s:%u: Out of memory\n", __func__, __LINE__ );
        return -ENOMEM;
    }
    if ( s3c2410_lcd_data_init( lcd ) ) {
        free( lcd );
        return -ENOMEM;
    }

    module->user_data = lcd;
    module->hdw_state->s3c2410_lcd = lcd;
    lcd->hdw_state = module->hdw_state;

    iotype = cpu_register_io_memory( s3c2410_lcd_readfn, s3c2410_lcd_writefn, lcd );
#ifdef DEBUG_S3C2410_LCD
    printf( "%s: iotype %08x\n", __func__, iotype );
#endif
    cpu_register_physical_memory( S3C2410_LCD_BASE, S3C2410_MAP_SIZE, iotype );

    return 0;
}

static int s3c2410_lcd_exit( hdw_module_t* module )
{
    s3c2410_lcd_t* lcd;

#ifdef DEBUG_X50NG_MODULES
    printf( "%s: %s:%u\n", module->name, __func__, __LINE__ );
#endif

    if ( module->user_data ) {
        lcd = module->user_data;
        if ( lcd->regs )
            free( lcd->regs );
        free( lcd );
    }

    module_unregister( module );
    free( module );

    return 0;
}

int s3c2410_get_pixel_color( s3c2410_lcd_t* lcd, int x, int y )
{
    uint32_t bank, addr, data, offset, pixel_offset;
    int bits_per_pixel = lcd->lcdcon5 > 2 ? 1 : 4 >> lcd->lcdcon5;

    bank = ( lcd->lcdsaddr1 << 1 ) & 0x7fc00000;
    addr = bank | ( ( lcd->lcdsaddr1 << 1 ) & 0x003ffffe );

    pixel_offset = ( 160 * y + x ) * bits_per_pixel;
    offset = ( pixel_offset >> 3 ) & 0xfffffffc;

    data = ldl_phys( addr + offset );
    data >>= pixel_offset & 31;
    data &= ( 1 << bits_per_pixel ) - 1;

    switch ( bits_per_pixel ) {
        case 1:
            return 15 * data;
        case 2:
            return 15 & ( lcd->bluelut >> ( 4 * data ) );
        default:
            return data;
    }
}

int s3c2410_init_module_lcd( hdw_t* hdw_state )
{
    hdw_module_t* module;

    if ( x50ng_module_init( hdw_state, "s3c2410-lcd", s3c2410_lcd_init, s3c2410_lcd_exit, s3c2410_lcd_reset, s3c2410_lcd_load,
                            s3c2410_lcd_save, NULL, &module ) )
        return -1;

    return module_register( module );
}
