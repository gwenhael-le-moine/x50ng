#include <unistd.h>
#include <fcntl.h>
#include <sys/times.h>

#include "s3c2410.h"

/*
 * Boot SRAM:			0x40000000
 * Memory Controller:		0x48000000
 * USB Host:			0x49000000
 * Interrupt Controller:	0x4a000000
 * DMA:				0x4b000000
 * Clock and Power Management:	0x4c000000
 * LCD Controller:		0x4d000000
 * NAND Flash Controller:	0x4e000000
 * UART:			0x50000000
 * PWM Timers:			0x51000000
 * USB Device:			0x52000000
 * Watchdog:			0x53000000
 * I2C Master:			0x54000000
 * IIS Interface:		0x55000000
 * GPIO Ports:			0x56000000
 * RTC:				0x57000000
 * ADC Controller:		0x58000000
 * SPI Interface:		0x59000000
 * SDI Interface:		0x5a000000
 */
int x50ng_s3c2410_init( hdw_t* hdw_state )
{
    x50ng_s3c2410_sram_init( hdw_state );
    x50ng_s3c2410_memc_init( hdw_state );
    /* x50ng_s3c2410_usbhost_init(x50ng); */
    x50ng_s3c2410_intc_init( hdw_state );
    /* x50ng_s3c2410_dma_init(x50ng); */
    x50ng_s3c2410_power_init( hdw_state );
    x50ng_s3c2410_lcd_init( hdw_state );
    x50ng_s3c2410_nand_init( hdw_state );
    x50ng_s3c2410_uart_init( hdw_state );
    x50ng_s3c2410_timer_init( hdw_state );
    x50ng_s3c2410_usbdev_init( hdw_state );
    x50ng_s3c2410_watchdog_init( hdw_state );
    /* x50ng_s3c2410_i2c_init(x50ng); */
    /* x50ng_s3c2410_iis_init(x50ng); */
    x50ng_s3c2410_io_port_init( hdw_state );
    x50ng_s3c2410_rtc_init( hdw_state );
    x50ng_s3c2410_adc_init( hdw_state );
    x50ng_s3c2410_spi_init( hdw_state );
    x50ng_s3c2410_sdi_init( hdw_state );

    return 0;
}

int s3c2410_exit( hdw_t* hdw_state ) { return 0; }
