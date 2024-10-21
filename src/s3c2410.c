/* $Id: s3c2410.c,v 1.38 2008/12/11 12:18:17 ecd Exp $
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <sys/times.h>
#include <errno.h>

#include <x49gp.h>
#include <s3c2410.h>

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
int
x49gp_s3c2410_init(x49gp_t *x49gp)
{
	x49gp_s3c2410_sram_init(x49gp);
	x49gp_s3c2410_memc_init(x49gp);
		/* x49gp_s3c2410_usbhost_init(x49gp); */
	x49gp_s3c2410_intc_init(x49gp);
		/* x49gp_s3c2410_dma_init(x49gp); */
	x49gp_s3c2410_power_init(x49gp);
	x49gp_s3c2410_lcd_init(x49gp);
	x49gp_s3c2410_nand_init(x49gp);
	x49gp_s3c2410_uart_init(x49gp);
	x49gp_s3c2410_timer_init(x49gp);
	x49gp_s3c2410_usbdev_init(x49gp);
	x49gp_s3c2410_watchdog_init(x49gp);
		/* x49gp_s3c2410_i2c_init(x49gp); */
		/* x49gp_s3c2410_iis_init(x49gp); */
	x49gp_s3c2410_io_port_init(x49gp);
	x49gp_s3c2410_rtc_init(x49gp);
	x49gp_s3c2410_adc_init(x49gp);
	x49gp_s3c2410_spi_init(x49gp);
	x49gp_s3c2410_sdi_init(x49gp);

	return 0;
}

int
s3c2410_exit(x49gp_t *x49gp)
{
	return 0;
}
