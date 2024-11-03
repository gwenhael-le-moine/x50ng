#ifndef _X49GP_S3C2410_H
#define _X49GP_S3C2410_H 1

#include <stdint.h>

#include "x49gp_types.h"

typedef struct {
	const char	*name;
	uint32_t	reset;
	uint32_t	*datap;
} s3c2410_offset_t;

#define S3C2410_OFFSET(module, name, reset, data) \
	[S3C2410_ ## module ## _ ## name >> 2] = { #name, reset, &(data) }

#define S3C2410_OFFSET_OK(p, offset) \
	((((offset) >> 2) < (p)->nr_regs) && (p)->regs[(offset) >> 2].name)

#define S3C2410_OFFSET_ENTRY(p, offset)	&((p)->regs[(offset) >> 2])


#define S3C2410_MAP_SIZE	0x00010000


#define S3C2410_SRAM_BASE	0x40000000
#define S3C2410_SRAM_SIZE	0x00001000

#define S3C2410_MEMC_BASE	0x48000000

#define S3C2410_MEMC_BWSCON		0x0000
#define S3C2410_MEMC_BANKCON0		0x0004
#define S3C2410_MEMC_BANKCON1		0x0008
#define S3C2410_MEMC_BANKCON2		0x000c
#define S3C2410_MEMC_BANKCON3		0x0010
#define S3C2410_MEMC_BANKCON4		0x0014
#define S3C2410_MEMC_BANKCON5		0x0018
#define S3C2410_MEMC_BANKCON6		0x001c
#define S3C2410_MEMC_BANKCON7		0x0020
#define S3C2410_MEMC_REFRESH		0x0024
#define S3C2410_MEMC_BANKSIZE		0x0028
#define S3C2410_MEMC_MRSRB6		0x002c
#define S3C2410_MEMC_MRSRB7		0x0030

#define S3C2410_USBHOST_BASE	0x49000000

#define S3C2410_INTC_BASE	0x4a000000

#define S3C2410_INTC_SRCPND		0x0000
#define S3C2410_INTC_INTMOD		0x0004
#define S3C2410_INTC_INTMSK		0x0008
#define S3C2410_INTC_PRIORITY		0x000c
#define S3C2410_INTC_INTPND		0x0010
#define S3C2410_INTC_INTOFFSET		0x0014
#define S3C2410_INTC_SUBSRCPND		0x0018
#define S3C2410_INTC_INTSUBMSK		0x001c

#define S3C2410_POWER_BASE	0x4c000000

#define S3C2410_POWER_LOCKTIME		0x0000
#define S3C2410_POWER_MPLLCON		0x0004
#define S3C2410_POWER_UPLLCON		0x0008
#define S3C2410_POWER_CLKCON		0x000c
#define S3C2410_POWER_CLKSLOW		0x0010
#define S3C2410_POWER_CLKDIVN		0x0014

#define S3C2410_LCD_BASE	0x4d000000

#define S3C2410_LCD_LCDCON1		0x0000
#define S3C2410_LCD_LCDCON2		0x0004
#define S3C2410_LCD_LCDCON3		0x0008
#define S3C2410_LCD_LCDCON4		0x000c
#define S3C2410_LCD_LCDCON5		0x0010
#define S3C2410_LCD_LCDSADDR1		0x0014
#define S3C2410_LCD_LCDSADDR2		0x0018
#define S3C2410_LCD_LCDSADDR3		0x001c
#define S3C2410_LCD_REDLUT		0x0020
#define S3C2410_LCD_GREENLUT		0x0024
#define S3C2410_LCD_BLUELUT		0x0028
#define S3C2410_LCD_DITHMODE		0x004c
#define S3C2410_LCD_TPAL		0x0050
#define S3C2410_LCD_LCDINTPND		0x0054
#define S3C2410_LCD_LCDSRCPND		0x0058
#define S3C2410_LCD_LCDINTMSK		0x005c
#define S3C2410_LCD_LPCSEL		0x0060
#define S3C2410_LCD_UNKNOWN_68		0x0068

#define S3C2410_LCD_PALETTE_START	0x0400
#define S3C2410_LCD_PALETTE_SIZE	0x0400

#define S3C2410_NAND_BASE	0x4e000000

#define S3C2410_NAND_NFCONF		0x0000
#define S3C2410_NAND_NFCMD		0x0004
#define S3C2410_NAND_NFADDR		0x0008
#define S3C2410_NAND_NFDATA		0x000c
#define S3C2410_NAND_NFSTAT		0x0010
#define S3C2410_NAND_NFECC		0x0014

#define S3C2410_UART0_BASE	0x50000000
#define S3C2410_UART1_BASE	0x50004000
#define S3C2410_UART2_BASE	0x50008000

#define S3C2410_UART0_ULCON		0x0000
#define S3C2410_UART0_UCON		0x0004
#define S3C2410_UART0_UFCON		0x0008
#define S3C2410_UART0_UMCON		0x000c
#define S3C2410_UART0_UTRSTAT		0x0010
#define S3C2410_UART0_UERSTAT		0x0014
#define S3C2410_UART0_UFSTAT		0x0018
#define S3C2410_UART0_UMSTAT		0x001c
#define S3C2410_UART0_UTXH		0x0020
#define S3C2410_UART0_URXH		0x0024
#define S3C2410_UART0_UBRDIV		0x0028

#define S3C2410_UART1_ULCON		0x0000
#define S3C2410_UART1_UCON		0x0004
#define S3C2410_UART1_UFCON		0x0008
#define S3C2410_UART1_UMCON		0x000c
#define S3C2410_UART1_UTRSTAT		0x0010
#define S3C2410_UART1_UERSTAT		0x0014
#define S3C2410_UART1_UFSTAT		0x0018
#define S3C2410_UART1_UMSTAT		0x001c
#define S3C2410_UART1_UTXH		0x0020
#define S3C2410_UART1_URXH		0x0024
#define S3C2410_UART1_UBRDIV		0x0028

#define S3C2410_UART2_ULCON		0x0000
#define S3C2410_UART2_UCON		0x0004
#define S3C2410_UART2_UFCON		0x0008
#define S3C2410_UART2_UTRSTAT		0x0010
#define S3C2410_UART2_UERSTAT		0x0014
#define S3C2410_UART2_UFSTAT		0x0018
#define S3C2410_UART2_UTXH		0x0020
#define S3C2410_UART2_URXH		0x0024
#define S3C2410_UART2_UBRDIV		0x0028

#define S3C2410_TIMER_BASE	0x51000000

#define S3C2410_TIMER_TCFG0		0x0000
#define S3C2410_TIMER_TCFG1		0x0004
#define S3C2410_TIMER_TCON		0x0008
#define S3C2410_TIMER_TCNTB0		0x000c
#define S3C2410_TIMER_TCMPB0		0x0010
#define S3C2410_TIMER_TCNTO0		0x0014
#define S3C2410_TIMER_TCNTB1		0x0018
#define S3C2410_TIMER_TCMPB1		0x001c
#define S3C2410_TIMER_TCNTO1		0x0020
#define S3C2410_TIMER_TCNTB2		0x0024
#define S3C2410_TIMER_TCMPB2		0x0028
#define S3C2410_TIMER_TCNTO2		0x002c
#define S3C2410_TIMER_TCNTB3		0x0030
#define S3C2410_TIMER_TCMPB3		0x0034
#define S3C2410_TIMER_TCNTO3		0x0038
#define S3C2410_TIMER_TCNTB4		0x003c
#define S3C2410_TIMER_TCNTO4		0x0040

#define S3C2410_USBDEV_BASE	0x52000000

#define S3C2410_USBDEV_FUNC_ADDR_REG		0x0140
#define S3C2410_USBDEV_PWR_REG			0x0144
#define S3C2410_USBDEV_EP_INT_REG		0x0148
#define S3C2410_USBDEV_USB_INT_REG		0x0158
#define S3C2410_USBDEV_EP_INT_EN_REG		0x015c
#define S3C2410_USBDEV_USB_INT_EN_REG		0x016c
#define S3C2410_USBDEV_FRAME_NUM1_REG		0x0170
#define S3C2410_USBDEV_FRAME_NUM2_REG		0x0174
#define S3C2410_USBDEV_INDEX_REG		0x0178
#define S3C2410_USBDEV_EP0_FIFO_REG		0x01C0
#define S3C2410_USBDEV_EP1_FIFO_REG		0x01C4
#define S3C2410_USBDEV_EP2_FIFO_REG		0x01C8
#define S3C2410_USBDEV_EP3_FIFO_REG		0x01CC
#define S3C2410_USBDEV_EP4_FIFO_REG		0x01D0
#define S3C2410_USBDEV_EP1_DMA_CON		0x0200
#define S3C2410_USBDEV_EP1_DMA_UNIT		0x0204
#define S3C2410_USBDEV_EP1_DMA_FIFO		0x0208
#define S3C2410_USBDEV_EP1_DMA_TTC_L		0x020C
#define S3C2410_USBDEV_EP1_DMA_TTC_M		0x0210
#define S3C2410_USBDEV_EP1_DMA_TTC_H		0x0214
#define S3C2410_USBDEV_EP2_DMA_CON		0x0218
#define S3C2410_USBDEV_EP2_DMA_UNIT		0x021C
#define S3C2410_USBDEV_EP2_DMA_FIFO		0x0220
#define S3C2410_USBDEV_EP2_DMA_TTC_L		0x0224
#define S3C2410_USBDEV_EP2_DMA_TTC_M		0x0228
#define S3C2410_USBDEV_EP2_DMA_TTC_H		0x022C
#define S3C2410_USBDEV_EP3_DMA_CON		0x0240
#define S3C2410_USBDEV_EP3_DMA_UNIT		0x0244
#define S3C2410_USBDEV_EP3_DMA_FIFO		0x0248
#define S3C2410_USBDEV_EP3_DMA_TTC_L		0x024C
#define S3C2410_USBDEV_EP3_DMA_TTC_M		0x0250
#define S3C2410_USBDEV_EP3_DMA_TTC_H		0x0254
#define S3C2410_USBDEV_EP4_DMA_CON		0x0258
#define S3C2410_USBDEV_EP4_DMA_UNIT		0x025C
#define S3C2410_USBDEV_EP4_DMA_FIFO		0x0260
#define S3C2410_USBDEV_EP4_DMA_TTC_L		0x0264
#define S3C2410_USBDEV_EP4_DMA_TTC_M		0x0268
#define S3C2410_USBDEV_EP4_DMA_TTC_H		0x026C
#define S3C2410_USBDEV_MAXP_REG_WRONG		0x0180
#define S3C2410_USBDEV_IN_CSR1_REG_EP0_CSR	0x0184
#define S3C2410_USBDEV_IN_CSR2_REG		0x0188
#define S3C2410_USBDEV_MAXP_REG			0x018c
#define S3C2410_USBDEV_OUT_CSR1_REG		0x0190
#define S3C2410_USBDEV_OUT_CSR2_REG		0x0194
#define S3C2410_USBDEV_OUT_FIFO_CNT1_REG	0x0198
#define S3C2410_USBDEV_OUT_FIFO_CNT2_REG	0x019C

#define S3C2410_WATCHDOG_BASE	0x53000000

#define S3C2410_WATCHDOG_WTCON		0x0000
#define S3C2410_WATCHDOG_WTDAT		0x0004
#define S3C2410_WATCHDOG_WTCNT		0x0008

#define S3C2410_IO_PORT_BASE	0x56000000

#define S3C2410_IO_PORT_GPACON		0x0000
#define S3C2410_IO_PORT_GPADAT		0x0004
#define S3C2410_IO_PORT_GPBCON		0x0010
#define S3C2410_IO_PORT_GPBDAT		0x0014
#define S3C2410_IO_PORT_GPBUP		0x0018
#define S3C2410_IO_PORT_GPCCON		0x0020
#define S3C2410_IO_PORT_GPCDAT		0x0024
#define S3C2410_IO_PORT_GPCUP		0x0028
#define S3C2410_IO_PORT_GPDCON		0x0030
#define S3C2410_IO_PORT_GPDDAT		0x0034
#define S3C2410_IO_PORT_GPDUP		0x0038
#define S3C2410_IO_PORT_GPECON		0x0040
#define S3C2410_IO_PORT_GPEDAT		0x0044
#define S3C2410_IO_PORT_GPEUP		0x0048
#define S3C2410_IO_PORT_GPFCON		0x0050
#define S3C2410_IO_PORT_GPFDAT		0x0054
#define S3C2410_IO_PORT_GPFUP		0x0058
#define S3C2410_IO_PORT_GPGCON		0x0060
#define S3C2410_IO_PORT_GPGDAT		0x0064
#define S3C2410_IO_PORT_GPGUP		0x0068
#define S3C2410_IO_PORT_GPHCON		0x0070
#define S3C2410_IO_PORT_GPHDAT		0x0074
#define S3C2410_IO_PORT_GPHUP		0x0078
#define S3C2410_IO_PORT_MISCCR		0x0080
#define S3C2410_IO_PORT_DCLKCON		0x0084
#define S3C2410_IO_PORT_EXTINT0		0x0088
#define S3C2410_IO_PORT_EXTINT1		0x008c
#define S3C2410_IO_PORT_EXTINT2		0x0090
#define S3C2410_IO_PORT_EINTFLT0	0x0094
#define S3C2410_IO_PORT_EINTFLT1	0x0098
#define S3C2410_IO_PORT_EINTFLT2	0x009c
#define S3C2410_IO_PORT_EINTFLT3	0x00a0
#define S3C2410_IO_PORT_EINTMASK	0x00a4
#define S3C2410_IO_PORT_EINTPEND	0x00a8
#define S3C2410_IO_PORT_GSTATUS0	0x00ac
#define S3C2410_IO_PORT_GSTATUS1	0x00b0
#define S3C2410_IO_PORT_GSTATUS2	0x00b4
#define S3C2410_IO_PORT_GSTATUS3	0x00b8
#define S3C2410_IO_PORT_GSTATUS4	0x00bc

#define S3C2410_RTC_BASE	0x57000000

#define S3C2410_RTC_RTCCON		0x0040
#define S3C2410_RTC_TICNT		0x0044
#define S3C2410_RTC_RTCALM		0x0050
#define S3C2410_RTC_ALMSEC		0x0054
#define S3C2410_RTC_ALMMIN		0x0058
#define S3C2410_RTC_ALMHOUR		0x005c
#define S3C2410_RTC_ALMDATE		0x0060
#define S3C2410_RTC_ALMMON		0x0064
#define S3C2410_RTC_ALMYEAR		0x0068
#define S3C2410_RTC_RTCRST		0x006c
#define S3C2410_RTC_BCDSEC		0x0070
#define S3C2410_RTC_BCDMIN		0x0074
#define S3C2410_RTC_BCDHOUR		0x0078
#define S3C2410_RTC_BCDDATE		0x007c
#define S3C2410_RTC_BCDDAY		0x0080
#define S3C2410_RTC_BCDMON		0x0084
#define S3C2410_RTC_BCDYEAR		0x0088

#define S3C2410_ADC_BASE	0x58000000

#define S3C2410_ADC_ADCCON		0x0000
#define S3C2410_ADC_ADCTSC		0x0004
#define S3C2410_ADC_ADCDLY		0x0008
#define S3C2410_ADC_ADCDAT0		0x000c
#define S3C2410_ADC_ADCDAT1		0x0010

#define S3C2410_SPI_BASE	0x59000000

#define S3C2410_SPI_SPICON0		0x0000
#define S3C2410_SPI_SPISTA0		0x0004
#define S3C2410_SPI_SPPIN0		0x0008
#define S3C2410_SPI_SPPRE0		0x000c
#define S3C2410_SPI_SPTDAT0		0x0010
#define S3C2410_SPI_SPRDAT0		0x0014
#define S3C2410_SPI_SPICON1		0x0020
#define S3C2410_SPI_SPISTA1		0x0024
#define S3C2410_SPI_SPPIN1		0x0028
#define S3C2410_SPI_SPPRE1		0x002c
#define S3C2410_SPI_SPTDAT1		0x0030
#define S3C2410_SPI_SPRDAT1		0x0034

#define S3C2410_SDI_BASE	0x5a000000

#define S3C2410_SDI_SDICON		0x0000
#define S3C2410_SDI_SDIPRE		0x0004
#define S3C2410_SDI_SDICARG		0x0008
#define S3C2410_SDI_SDICCON		0x000c
#define S3C2410_SDI_SDICSTA		0x0010
#define S3C2410_SDI_SDIRSP0		0x0014
#define S3C2410_SDI_SDIRSP1		0x0018
#define S3C2410_SDI_SDIRSP2		0x001c
#define S3C2410_SDI_SDIRSP3		0x0020
#define S3C2410_SDI_SDIDTIMER		0x0024
#define S3C2410_SDI_SDIBSIZE		0x0028
#define S3C2410_SDI_SDIDCON		0x002c
#define S3C2410_SDI_SDIDCNT		0x0030
#define S3C2410_SDI_SDIDSTA		0x0034
#define S3C2410_SDI_SDIFSTA		0x0038
#define S3C2410_SDI_SDIDAT		0x003c
#define S3C2410_SDI_SDIIMSK		0x0040


extern int x49gp_s3c2410_init(x49gp_t *x49gp);
extern int x49gp_s3c2410_arm_init(x49gp_t *x49gp);
extern int x49gp_s3c2410_mmu_init(x49gp_t *x49gp);
extern int x49gp_s3c2410_sram_init(x49gp_t *x49gp);
extern int x49gp_s3c2410_memc_init(x49gp_t *x49gp);
extern int x49gp_s3c2410_intc_init(x49gp_t *x49gp);
extern int x49gp_s3c2410_power_init(x49gp_t *x49gp);
extern int x49gp_s3c2410_lcd_init(x49gp_t *x49gp);
extern int x49gp_s3c2410_nand_init(x49gp_t *x49gp);
extern int x49gp_s3c2410_uart_init(x49gp_t *x49gp);
extern int x49gp_s3c2410_timer_init(x49gp_t *x49gp);
extern int x49gp_s3c2410_usbdev_init(x49gp_t *x49gp);
extern int x49gp_s3c2410_watchdog_init(x49gp_t *x49gp);
extern int x49gp_s3c2410_io_port_init(x49gp_t *x49gp);
extern int x49gp_s3c2410_rtc_init(x49gp_t *x49gp);
extern int x49gp_s3c2410_adc_init(x49gp_t *x49gp);
extern int x49gp_s3c2410_spi_init(x49gp_t *x49gp);
extern int x49gp_s3c2410_sdi_init(x49gp_t *x49gp);

extern void s3c2410_io_port_g_update(x49gp_t *x49gp, int column, int row, unsigned char columnbit, unsigned char rowbit, uint32_t new_state);
extern void s3c2410_io_port_f_set_bit(x49gp_t *x49gp, int n, uint32_t set);

extern void x49gp_schedule_lcd_update(x49gp_t *x49gp);
extern void x49gp_lcd_update(x49gp_t *x49gp);

extern unsigned long s3c2410_timer_next_interrupt(x49gp_t *x49gp);
extern unsigned long s3c2410_watchdog_next_interrupt(x49gp_t *x49gp);

#endif /* !(_X49GP_S3C2410_H) */
