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
int init_s3c2410( hdw_t* hdw_state )
{
    s3c2410_init_module_sram( hdw_state );
    s3c2410_init_module_memc( hdw_state );
    s3c2410_init_module_intc( hdw_state );
    s3c2410_init_module_power( hdw_state );
    s3c2410_init_module_lcd( hdw_state );
    s3c2410_init_module_nand( hdw_state );
    s3c2410_init_module_uart( hdw_state );
    s3c2410_init_module_timer( hdw_state );
    s3c2410_init_module_usbdev( hdw_state );
    s3c2410_init_module_watchdog( hdw_state );
    s3c2410_init_module_io_port( hdw_state );
    s3c2410_init_module_rtc( hdw_state );
    s3c2410_init_module_adc( hdw_state );
    s3c2410_init_module_spi( hdw_state );
    s3c2410_init_module_sdi( hdw_state );

    return 0;
}

int s3c2410_exit( hdw_t* _hdw_state ) { return 0; }
