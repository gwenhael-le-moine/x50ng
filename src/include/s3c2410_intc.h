/* $Id: s3c2410_intc.h,v 1.3 2008/12/11 12:18:17 ecd Exp $
 */

#ifndef _X49GP_S3C2410_INTC_H
#define _X49GP_S3C2410_INTC_H 1

#define INT_ADC		31
#define INT_RTC		30
#define INT_SPI1	29
#define INT_UART0	28
#define INT_IIC		27
#define INT_USBH	26
#define INT_USBD	25
#define INT_UART1	23
#define INT_SPI0	22
#define INT_SDI		21
#define INT_DMA3	20
#define INT_DMA2	19
#define INT_DMA1	18
#define INT_DMA0	17
#define INT_LCD		16
#define INT_UART2	15
#define INT_TIMER4	14
#define INT_TIMER3	13
#define INT_TIMER2	12
#define INT_TIMER1	11
#define INT_TIMER0	10
#define INT_WDT		 9
#define INT_TICK	 8
#define nBATT_FLT	 7
#define EINT8_23	 5
#define EINT4_7		 4
#define EINT3		 3
#define EINT2		 2
#define EINT1		 1
#define EINT0		 0

#define SUB_INT_ADC	10
#define SUB_INT_TC	 9
#define SUB_INT_ERR2	 8
#define SUB_INT_TXD2	 7
#define SUB_INT_RXD2	 6
#define SUB_INT_ERR1	 5
#define SUB_INT_TXD1	 4
#define SUB_INT_RXD1	 3
#define SUB_INT_ERR0	 2
#define SUB_INT_TXD0	 1
#define SUB_INT_RXD0	 0

#define ARB0_MODE	(1 << 0)
#define ARB1_MODE	(1 << 1)
#define ARB2_MODE	(1 << 2)
#define ARB3_MODE	(1 << 3)
#define ARB4_MODE	(1 << 4)
#define ARB5_MODE	(1 << 5)
#define ARB6_MODE	(1 << 6)
#define ARB0_SEL_SHIFT	 7
#define ARB1_SEL_SHIFT	 9
#define ARB2_SEL_SHIFT	11
#define ARB3_SEL_SHIFT	13
#define ARB4_SEL_SHIFT	15
#define ARB5_SEL_SHIFT	17
#define ARB6_SEL_SHIFT	19
#define ARBx_SEL_MASK	3

void s3c2410_intc_sub_assert(x49gp_t *x49gp, int sub_irq, int level);
void s3c2410_intc_sub_deassert(x49gp_t *x49gp, int sub_irq);

void s3c2410_intc_assert(x49gp_t *x49gp, int irq, int level);
void s3c2410_intc_deassert(x49gp_t *x49gp, int irq);

#endif /* !(_X49GP_S3C2410_INTC_H) */
