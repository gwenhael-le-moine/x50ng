#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "../module.h"

#include "s3c2410.h"

typedef struct {
    uint32_t func_addr_reg;
    uint32_t pwr_reg;
    uint32_t ep_int_reg;
    uint32_t usb_int_reg;
    uint32_t ep_int_en_reg;
    uint32_t usb_int_en_reg;
    uint32_t frame_num1_reg;
    uint32_t frame_num2_reg;
    uint32_t index_reg;
    uint32_t ep0_fifo_reg;
    uint32_t ep1_fifo_reg;
    uint32_t ep2_fifo_reg;
    uint32_t ep3_fifo_reg;
    uint32_t ep4_fifo_reg;
    uint32_t ep1_dma_con;
    uint32_t ep1_dma_unit;
    uint32_t ep1_dma_fifo;
    uint32_t ep1_dma_ttc_l;
    uint32_t ep1_dma_ttc_m;
    uint32_t ep1_dma_ttc_h;
    uint32_t ep2_dma_con;
    uint32_t ep2_dma_unit;
    uint32_t ep2_dma_fifo;
    uint32_t ep2_dma_ttc_l;
    uint32_t ep2_dma_ttc_m;
    uint32_t ep2_dma_ttc_h;
    uint32_t ep3_dma_con;
    uint32_t ep3_dma_unit;
    uint32_t ep3_dma_fifo;
    uint32_t ep3_dma_ttc_l;
    uint32_t ep3_dma_ttc_m;
    uint32_t ep3_dma_ttc_h;
    uint32_t ep4_dma_con;
    uint32_t ep4_dma_unit;
    uint32_t ep4_dma_fifo;
    uint32_t ep4_dma_ttc_l;
    uint32_t ep4_dma_ttc_m;
    uint32_t ep4_dma_ttc_h;
    uint32_t __wrong_maxp_reg;
    uint32_t in_csr1_reg_ep0_csr;
    uint32_t in_csr2_reg;
    uint32_t maxp_reg;
    uint32_t out_csr1_reg;
    uint32_t out_csr2_reg;
    uint32_t out_fifo_cnt1_reg;
    uint32_t out_fifo_cnt2_reg;

    unsigned int nr_regs;
    s3c2410_offset_t* regs;
} s3c2410_usbdev_t;

static int s3c2410_usbdev_data_init( s3c2410_usbdev_t* usbdev )
{
    s3c2410_offset_t regs[] = { S3C2410_OFFSET( USBDEV, FUNC_ADDR_REG, 0x00, usbdev->func_addr_reg ),
                                S3C2410_OFFSET( USBDEV, PWR_REG, 0x00, usbdev->pwr_reg ),
                                S3C2410_OFFSET( USBDEV, EP_INT_REG, 0x00, usbdev->ep_int_reg ),
                                S3C2410_OFFSET( USBDEV, USB_INT_REG, 0x00, usbdev->usb_int_reg ),
                                S3C2410_OFFSET( USBDEV, EP_INT_EN_REG, 0xff, usbdev->ep_int_en_reg ),
                                S3C2410_OFFSET( USBDEV, USB_INT_EN_REG, 0x04, usbdev->usb_int_en_reg ),
                                S3C2410_OFFSET( USBDEV, FRAME_NUM1_REG, 0x00, usbdev->frame_num1_reg ),
                                S3C2410_OFFSET( USBDEV, FRAME_NUM2_REG, 0x00, usbdev->frame_num2_reg ),
                                S3C2410_OFFSET( USBDEV, INDEX_REG, 0x00, usbdev->index_reg ),
                                S3C2410_OFFSET( USBDEV, EP0_FIFO_REG, 0, usbdev->ep0_fifo_reg ),
                                S3C2410_OFFSET( USBDEV, EP1_FIFO_REG, 0, usbdev->ep1_fifo_reg ),
                                S3C2410_OFFSET( USBDEV, EP2_FIFO_REG, 0, usbdev->ep2_fifo_reg ),
                                S3C2410_OFFSET( USBDEV, EP3_FIFO_REG, 0, usbdev->ep3_fifo_reg ),
                                S3C2410_OFFSET( USBDEV, EP4_FIFO_REG, 0, usbdev->ep4_fifo_reg ),
                                S3C2410_OFFSET( USBDEV, EP1_DMA_CON, 0x00, usbdev->ep1_dma_con ),
                                S3C2410_OFFSET( USBDEV, EP1_DMA_UNIT, 0x00, usbdev->ep1_dma_unit ),
                                S3C2410_OFFSET( USBDEV, EP1_DMA_FIFO, 0x00, usbdev->ep1_dma_fifo ),
                                S3C2410_OFFSET( USBDEV, EP1_DMA_TTC_L, 0x00, usbdev->ep1_dma_ttc_l ),
                                S3C2410_OFFSET( USBDEV, EP1_DMA_TTC_M, 0x00, usbdev->ep1_dma_ttc_m ),
                                S3C2410_OFFSET( USBDEV, EP1_DMA_TTC_H, 0x00, usbdev->ep1_dma_ttc_h ),
                                S3C2410_OFFSET( USBDEV, EP2_DMA_CON, 0x00, usbdev->ep2_dma_con ),
                                S3C2410_OFFSET( USBDEV, EP2_DMA_UNIT, 0x00, usbdev->ep2_dma_unit ),
                                S3C2410_OFFSET( USBDEV, EP2_DMA_FIFO, 0x00, usbdev->ep2_dma_fifo ),
                                S3C2410_OFFSET( USBDEV, EP2_DMA_TTC_L, 0x00, usbdev->ep2_dma_ttc_l ),
                                S3C2410_OFFSET( USBDEV, EP2_DMA_TTC_M, 0x00, usbdev->ep2_dma_ttc_m ),
                                S3C2410_OFFSET( USBDEV, EP2_DMA_TTC_H, 0x00, usbdev->ep2_dma_ttc_h ),
                                S3C2410_OFFSET( USBDEV, EP3_DMA_CON, 0x00, usbdev->ep3_dma_con ),
                                S3C2410_OFFSET( USBDEV, EP3_DMA_UNIT, 0x00, usbdev->ep3_dma_unit ),
                                S3C2410_OFFSET( USBDEV, EP3_DMA_FIFO, 0x00, usbdev->ep3_dma_fifo ),
                                S3C2410_OFFSET( USBDEV, EP3_DMA_TTC_L, 0x00, usbdev->ep3_dma_ttc_l ),
                                S3C2410_OFFSET( USBDEV, EP3_DMA_TTC_M, 0x00, usbdev->ep3_dma_ttc_m ),
                                S3C2410_OFFSET( USBDEV, EP3_DMA_TTC_H, 0x00, usbdev->ep3_dma_ttc_h ),
                                S3C2410_OFFSET( USBDEV, EP4_DMA_CON, 0x00, usbdev->ep4_dma_con ),
                                S3C2410_OFFSET( USBDEV, EP4_DMA_UNIT, 0x00, usbdev->ep4_dma_unit ),
                                S3C2410_OFFSET( USBDEV, EP4_DMA_FIFO, 0x00, usbdev->ep4_dma_fifo ),
                                S3C2410_OFFSET( USBDEV, EP4_DMA_TTC_L, 0x00, usbdev->ep4_dma_ttc_l ),
                                S3C2410_OFFSET( USBDEV, EP4_DMA_TTC_M, 0x00, usbdev->ep4_dma_ttc_m ),
                                S3C2410_OFFSET( USBDEV, EP4_DMA_TTC_H, 0x00, usbdev->ep4_dma_ttc_h ),
                                S3C2410_OFFSET( USBDEV, MAXP_REG_WRONG, 0x01, usbdev->__wrong_maxp_reg ),
                                S3C2410_OFFSET( USBDEV, IN_CSR1_REG_EP0_CSR, 0x00, usbdev->in_csr1_reg_ep0_csr ),
                                S3C2410_OFFSET( USBDEV, IN_CSR2_REG, 0x20, usbdev->in_csr2_reg ),
                                S3C2410_OFFSET( USBDEV, MAXP_REG, 0x01, usbdev->maxp_reg ),
                                S3C2410_OFFSET( USBDEV, OUT_CSR1_REG, 0x00, usbdev->out_csr1_reg ),
                                S3C2410_OFFSET( USBDEV, OUT_CSR2_REG, 0x00, usbdev->out_csr2_reg ),
                                S3C2410_OFFSET( USBDEV, OUT_FIFO_CNT1_REG, 0x00, usbdev->out_fifo_cnt1_reg ),
                                S3C2410_OFFSET( USBDEV, OUT_FIFO_CNT2_REG, 0x00, usbdev->out_fifo_cnt2_reg ) };

    memset( usbdev, 0, sizeof( s3c2410_usbdev_t ) );

    usbdev->regs = malloc( sizeof( regs ) );
    if ( NULL == usbdev->regs ) {
        fprintf( stderr, "%s:%u: Out of memory\n", __func__, __LINE__ );
        return -ENOMEM;
    }

    memcpy( usbdev->regs, regs, sizeof( regs ) );
    usbdev->nr_regs = sizeof( regs ) / sizeof( regs[ 0 ] );

    return 0;
}

static uint32_t s3c2410_usbdev_read( void* opaque, target_phys_addr_t offset )
{
    s3c2410_usbdev_t* usbdev = opaque;
    s3c2410_offset_t* reg;

    if ( !S3C2410_OFFSET_OK( usbdev, offset ) )
        return ~( 0 );

    reg = S3C2410_OFFSET_ENTRY( usbdev, offset );

#ifdef DEBUG_S3C2410_USBDEV
    printf( "read  %s [%08x] %s [%08lx] data %08x\n", "s3c2410-usbdev", S3C2410_USBDEV_BASE, reg->name, ( unsigned long )offset,
            *( reg->datap ) );
#endif

    return *( reg->datap );
}

static void s3c2410_usbdev_write( void* opaque, target_phys_addr_t offset, uint32_t data )
{
    s3c2410_usbdev_t* usbdev = opaque;
    s3c2410_offset_t* reg;

    if ( !S3C2410_OFFSET_OK( usbdev, offset ) )
        return;

    reg = S3C2410_OFFSET_ENTRY( usbdev, offset );

#ifdef DEBUG_S3C2410_USBDEV
    printf( "write %s [%08x] %s [%08lx] data %08x\n", "s3c2410-usbdev", S3C2410_USBDEV_BASE, reg->name, ( unsigned long )offset, data );
#endif

    *( reg->datap ) = data;
}

static int s3c2410_usbdev_load( hdw_module_t* module, GKeyFile* key )
{
    s3c2410_usbdev_t* usbdev = module->user_data;
    s3c2410_offset_t* reg;
    int error = 0;
    unsigned int i;

#ifdef DEBUG_X50NG_MODULES
    printf( "%s: %s:%u\n", module->name, __func__, __LINE__ );
#endif

    for ( i = 0; i < usbdev->nr_regs; i++ ) {
        reg = &usbdev->regs[ i ];

        if ( NULL == reg->name )
            continue;

        if ( module_get_u32( module, key, reg->name, reg->reset, reg->datap ) )
            error = -EAGAIN;
    }

    return error;
}

static int s3c2410_usbdev_save( hdw_module_t* module, GKeyFile* key )
{
    s3c2410_usbdev_t* usbdev = module->user_data;
    s3c2410_offset_t* reg;
    unsigned int i;

#ifdef DEBUG_X50NG_MODULES
    printf( "%s: %s:%u\n", module->name, __func__, __LINE__ );
#endif

    for ( i = 0; i < usbdev->nr_regs; i++ ) {
        reg = &usbdev->regs[ i ];

        if ( NULL == reg->name )
            continue;

        module_set_u32( module, key, reg->name, *( reg->datap ) );
    }

    return 0;
}

static int s3c2410_usbdev_reset( hdw_module_t* module, hdw_reset_t reset )
{
    s3c2410_usbdev_t* usbdev = module->user_data;
    s3c2410_offset_t* reg;
    unsigned int i;

#ifdef DEBUG_X50NG_MODULES
    printf( "%s: %s:%u\n", module->name, __func__, __LINE__ );
#endif

    for ( i = 0; i < usbdev->nr_regs; i++ ) {
        reg = &usbdev->regs[ i ];

        if ( NULL == reg->name )
            continue;

        *( reg->datap ) = reg->reset;
    }

    return 0;
}

static CPUReadMemoryFunc* s3c2410_usbdev_readfn[] = { s3c2410_usbdev_read, s3c2410_usbdev_read, s3c2410_usbdev_read };

static CPUWriteMemoryFunc* s3c2410_usbdev_writefn[] = { s3c2410_usbdev_write, s3c2410_usbdev_write, s3c2410_usbdev_write };

static int s3c2410_usbdev_init( hdw_module_t* module )
{
    s3c2410_usbdev_t* usbdev;
    int iotype;

#ifdef DEBUG_X50NG_MODULES
    printf( "%s: %s:%u\n", module->name, __func__, __LINE__ );
#endif

    usbdev = malloc( sizeof( s3c2410_usbdev_t ) );
    if ( NULL == usbdev ) {
        fprintf( stderr, "%s:%u: Out of memory\n", __func__, __LINE__ );
        return -ENOMEM;
    }
    if ( s3c2410_usbdev_data_init( usbdev ) ) {
        free( usbdev );
        return -ENOMEM;
    }

    module->user_data = usbdev;

    iotype = cpu_register_io_memory( s3c2410_usbdev_readfn, s3c2410_usbdev_writefn, usbdev );
#ifdef DEBUG_S3C2410_USBDEV
    printf( "%s: iotype %08x\n", __func__, iotype );
#endif
    cpu_register_physical_memory( S3C2410_USBDEV_BASE, S3C2410_MAP_SIZE, iotype );
    return 0;
}

static int s3c2410_usbdev_exit( hdw_module_t* module )
{
    s3c2410_usbdev_t* usbdev;

#ifdef DEBUG_X50NG_MODULES
    printf( "%s: %s:%u\n", module->name, __func__, __LINE__ );
#endif

    if ( module->user_data ) {
        usbdev = module->user_data;
        if ( usbdev->regs )
            free( usbdev->regs );
        free( usbdev );
    }

    module_unregister( module );
    free( module );

    return 0;
}

int s3c2410_init_module_usbdev( hdw_t* hdw_state )
{
    hdw_module_t* module;

    if ( x50ng_module_init( hdw_state, "s3c2410-usbdev", s3c2410_usbdev_init, s3c2410_usbdev_exit, s3c2410_usbdev_reset,
                            s3c2410_usbdev_load, s3c2410_usbdev_save, NULL, &module ) )
        return -1;

    return module_register( module );
}
