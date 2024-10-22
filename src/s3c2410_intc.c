/* $Id: s3c2410_intc.c,v 1.8 2008/12/11 12:18:17 ecd Exp $
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "x49gp.h"
#include "s3c2410.h"
#include "s3c2410_intc.h"
#include "s3c2410_power.h"
#include "byteorder.h"

extern int do_trace;

typedef struct {
	int		sel_shift;
	uint32_t	mode_bit;
	int		index;
	int		req[6];
} s3c2410_arb_t;

typedef struct {
	uint32_t	sel;
	uint32_t	mode;
} s3c2410_arb_data_t;

static const int s3c2410_arb_order[4][6] =
{
	{ 0, 1, 2, 3, 4, 5 },
	{ 0, 2, 3, 4, 1, 5 },
	{ 0, 3, 4, 1, 2, 5 },
	{ 0, 4, 1, 2, 3, 5 }
};

static const s3c2410_arb_t s3c2410_arb_table[] =
{
	[0] = { ARB0_SEL_SHIFT, ARB0_MODE, 0,
		{ -1, EINT0, EINT1, EINT2, EINT3, -1 } },
	[1] = { ARB1_SEL_SHIFT, ARB1_MODE, 1,
		{ EINT4_7, EINT8_23, -1, nBATT_FLT, INT_TICK, INT_WDT } },
	[2] = { ARB2_SEL_SHIFT, ARB2_MODE, 2,
		{ INT_TIMER0, INT_TIMER1, INT_TIMER2, INT_TIMER3, INT_TIMER4, INT_UART2} },
	[3] = { ARB3_SEL_SHIFT, ARB3_MODE, 3,
		{ INT_LCD, INT_DMA0, INT_DMA1, INT_DMA2, INT_DMA3, INT_SDI } },
	[4] = { ARB4_SEL_SHIFT, ARB4_MODE, 4,
		{ INT_SPI0, INT_UART1, -1, INT_USBD, INT_USBH, INT_IIC } },
	[5] = { ARB5_SEL_SHIFT, ARB5_MODE, 5,
		{ -1, INT_UART0, INT_SPI1, INT_RTC, INT_ADC, -1, } },
	[6] = { ARB6_SEL_SHIFT, ARB6_MODE, 6,
		{ 0, 1, 2, 3, 4, 5 } },
};
#define INTC_NR_ARB (sizeof(s3c2410_arb_table) / sizeof(s3c2410_arb_table[0]))


typedef struct {
	uint32_t			srcpnd;
	uint32_t			intmod;
	uint32_t			intmsk;
	uint32_t			priority;
	uint32_t			intpnd;
	uint32_t			intoffset;
	uint32_t			subsrcpnd;
	uint32_t			intsubmsk;

	s3c2410_arb_data_t	arb_data[INTC_NR_ARB];

	x49gp_t			*x49gp;

	uint32_t		src_pending;
	uint32_t		subsrc_pending;

	unsigned int		nr_regs;
	s3c2410_offset_t	*regs;
} s3c2410_intc_t;


static void s3c2410_intc_gen_int(s3c2410_intc_t *intc);
static void s3c2410_intc_gen_int_from_sub_int(s3c2410_intc_t *intc);

static int
s3c2410_intc_data_init(s3c2410_intc_t *intc)
{
	int i;

	s3c2410_offset_t regs[] = {
		S3C2410_OFFSET(INTC, SRCPND, 0x00000000, intc->srcpnd),
		S3C2410_OFFSET(INTC, INTMOD, 0x00000000, intc->intmod),
		S3C2410_OFFSET(INTC, INTMSK, 0xffffffff, intc->intmsk),
		S3C2410_OFFSET(INTC, PRIORITY, 0x0000007f, intc->priority),
		S3C2410_OFFSET(INTC, INTPND, 0x00000000, intc->intpnd),
		S3C2410_OFFSET(INTC, INTOFFSET, 0x00000000, intc->intoffset),
		S3C2410_OFFSET(INTC, SUBSRCPND, 0x00000000, intc->subsrcpnd),
		S3C2410_OFFSET(INTC, INTSUBMSK, 0x000007ff, intc->intsubmsk)
	};

	memset(intc, 0, sizeof(s3c2410_intc_t));

	intc->regs = malloc(sizeof(regs));
	if (NULL == intc->regs) {
		fprintf(stderr, "%s:%u: Out of memory\n",
			__FUNCTION__, __LINE__);
		return -ENOMEM;
	}

	memcpy(intc->regs, regs, sizeof(regs));
	intc->nr_regs = sizeof(regs) / sizeof(regs[0]);

	for (i = 0; i < INTC_NR_ARB; i++) {
		intc->arb_data[i].sel = 0;
		intc->arb_data[i].mode = s3c2410_arb_table[i].mode_bit;
	}

	return 0;
}

static void
srcpnd_put_word(s3c2410_intc_t *intc, uint32_t data)
{
	intc->srcpnd &= ~(data);
	intc->srcpnd |= intc->src_pending;

	if (intc->src_pending & data) {
		s3c2410_intc_gen_int(intc);
	}
}

static void
intmod_put_word(s3c2410_intc_t *intc, uint32_t data)
{
	intc->intmod = data & 0xfeffffbf;

	s3c2410_intc_gen_int(intc);
}

static void
intmsk_put_word(s3c2410_intc_t *intc, uint32_t data)
{
#ifdef DEBUG_X49GP_ENABLE_IRQ
	uint32_t change;
	int i;

	change = intc->intmsk ^ data;
#endif

	intc->intmsk = data | 0x01000040;

#ifdef DEBUG_X49GP_ENABLE_IRQ
	for (i = 0; i < 32; i++) {
		if ((change & (1 << i)) && !(intc->intmsk & (1 << i))) {
			printf("INTC: Enable IRQ %u\n", i);
		}
	}
#endif

	s3c2410_intc_gen_int(intc);
}

static uint32_t
priority_get_word(s3c2410_intc_t *intc)
{
	const s3c2410_arb_t *arb;
	s3c2410_arb_data_t *arb_data;
	int i;

	intc->priority = 0;

	for (i = 0; i < INTC_NR_ARB; i++) {
		arb = &s3c2410_arb_table[i];
		arb_data = &intc->arb_data[i];

		intc->priority |= (arb_data->sel << arb->sel_shift) |
				  arb_data->mode;
	}

	return intc->priority;
}

static void
priority_put_word(s3c2410_intc_t *intc, uint32_t data)
{
	const s3c2410_arb_t *arb;
	s3c2410_arb_data_t *arb_data;
	int i;

	intc->priority = data & 0x001fffff;

	for (i = 0; i < INTC_NR_ARB; i++) {
		arb = &s3c2410_arb_table[i];
		arb_data = &intc->arb_data[i];

		arb_data->sel = (intc->priority >> arb->sel_shift) & ARBx_SEL_MASK;
		arb_data->mode = intc->priority & arb->mode_bit;
	}

	s3c2410_intc_gen_int(intc);
}

static void
intpnd_put_word(s3c2410_intc_t *intc, uint32_t data)
{
	intc->intpnd &= ~(data);

	s3c2410_intc_gen_int(intc);
}

static void
subsrcpnd_put_word(s3c2410_intc_t *intc, uint32_t data)
{
	intc->subsrcpnd &= ~(data);
	intc->subsrcpnd |= intc->subsrc_pending;

	if (intc->subsrc_pending & data) {
		s3c2410_intc_gen_int_from_sub_int(intc);
	}
}

static void
intsubmsk_put_word(s3c2410_intc_t *intc, uint32_t data)
{
	intc->intsubmsk = data & 0x000007ff;

	s3c2410_intc_gen_int_from_sub_int(intc);
}

static uint32_t
s3c2410_intc_select_int(s3c2410_intc_t *intc, const s3c2410_arb_t *arb,
			uint32_t service, int *offset)
{
	s3c2410_arb_data_t *arb_data = &intc->arb_data[arb->index];
	const int *order;
	int i, req;

	order = s3c2410_arb_order[arb_data->sel];

	for (i = 0; i < 6; i++) {
		req = order[i];

		if (-1 == arb->req[req])
			continue;

		if (service & (1 << arb->req[req])) {
			if (arb_data->mode)
				arb_data->sel = (arb_data->sel + 1) & ARBx_SEL_MASK;
			*offset = arb->req[req];
			return (1 << arb->index);
		}
	}

	*offset = -1;
	return 0;
}

void
s3c2410_FIQ (CPUState *env)
{
	cpu_interrupt(env, CPU_INTERRUPT_FIQ);
}

void
s3c2410_IRQ (CPUState *env)
{
	cpu_interrupt(env, CPU_INTERRUPT_HARD);
}


static void
s3c2410_intc_gen_int(s3c2410_intc_t *intc)
{
	x49gp_t *x49gp = intc->x49gp;
	uint32_t fiq, service;
	int offset[6], index;
	const s3c2410_arb_t *arb;
	uint32_t request;
	int i;

	fiq = intc->srcpnd & intc->intmod;

#ifdef DEBUG_S3C2410_INTC0
	printf("INTC: FIQ service request: %08x\n", fiq);
#endif

	if (fiq) {
		/*
		 * Generate FIQ.
		 */
#ifdef DEBUG_S3C2410_INTC
		printf("INTC: vector to %08x\n", 0x1c);
#endif
		cpu_interrupt(x49gp->env, CPU_INTERRUPT_FIQ);

		x49gp_set_idle(x49gp, 0);
		return;
	} else {
		cpu_reset_interrupt(x49gp->env, CPU_INTERRUPT_FIQ);
	}

#ifdef DEBUG_S3C2410_INTC0
	printf("INTC: IRQ pending request: %08x\n", intc->intpnd);
#endif

	if (intc->intpnd) {
		/*
		 * Generate IRQ.
		 */
#ifdef DEBUG_S3C2410_INTC
		printf("INTC: vector to %08x\n", 0x18);
#endif
		cpu_interrupt(x49gp->env, CPU_INTERRUPT_HARD);

		x49gp_set_idle(x49gp, 0);
		return;
	}

#ifdef DEBUG_S3C2410_INTC0
	printf("INTC: srcpnd %08x, intmsk: %08x\n", intc->srcpnd, intc->intmsk);
#endif

	service = intc->srcpnd & ~(intc->intmsk);

#ifdef DEBUG_S3C2410_INTC0
	printf("INTC: IRQ service request: %08x\n", service);
#endif

	if (0 == service) {
		cpu_reset_interrupt(x49gp->env, CPU_INTERRUPT_HARD);
		return;
	}

	request = 0;
	for (i = 0; i < 6; i++) {
		arb = &s3c2410_arb_table[i];

		request |= s3c2410_intc_select_int(intc, arb, service, &offset[i]);

#ifdef DEBUG_S3C2410_INTC0
		printf("INTC: ARB%u highest %d\n", i, offset[i]);
#endif
	}

	arb = &s3c2410_arb_table[6];

#ifdef DEBUG_S3C2410_INTC0
	printf("INTC: ARB%u request: %08x\n", 6, request);
#endif

	if (s3c2410_intc_select_int(intc, arb, request, &index)) {
		intc->intoffset = offset[index];
		intc->intpnd |= (1 << intc->intoffset);

#ifdef DEBUG_S3C2410_INTC
		printf("INTC: irq pending: %u (%08x)\n", intc->intoffset, intc->intpnd);
#endif
		/*
		 * Generate IRQ.
		 */
#ifdef DEBUG_S3C2410_INTC
		printf("INTC: vector to %08x\n", 0x18);
#endif
		cpu_interrupt(x49gp->env, CPU_INTERRUPT_HARD);

		x49gp_set_idle(x49gp, 0);
		return;
	}

#ifdef DEBUG_S3C2410_INTC0
	printf("INTC: No irq pending\n");
#endif

	cpu_reset_interrupt(x49gp->env, CPU_INTERRUPT_HARD);
}

void
s3c2410_intc_assert(x49gp_t *x49gp, int irq, int level)
{
	s3c2410_intc_t *intc = x49gp->s3c2410_intc;

	if (irq > 31)
		return;

#ifdef DEBUG_S3C2410_INTC
	printf("INTC: assert irq %u (%08x)\n", irq, 1 << irq);
#endif

	if (! (intc->src_pending & (1 << irq))) {
		if (level)
			intc->src_pending |= (1 << irq);
		intc->srcpnd |= (1 << irq);

		s3c2410_intc_gen_int(intc);
	}

	if (x49gp->arm_idle == 2) {
		if (irq == EINT0 || irq == INT_RTC)
			x49gp_set_idle(x49gp, 0);
	}
}

void
s3c2410_intc_deassert(x49gp_t *x49gp, int irq)
{
	s3c2410_intc_t *intc = x49gp->s3c2410_intc;

	if (irq > 31)
		return;

#ifdef DEBUG_S3C2410_INTC
	printf("INTC: deassert irq %u (%08x)\n", irq, 1 << irq);
#endif

	intc->src_pending &= ~(1 << irq);
}

static void
s3c2410_intc_gen_int_from_sub_int(s3c2410_intc_t *intc)
{
	x49gp_t *x49gp = intc->x49gp;
	uint32_t service;

	service = intc->subsrcpnd & ~(intc->intsubmsk);

#ifdef DEBUG_S3C2410_INTC
	printf("INTC: subirq service request: %08x\n", service);
#endif

	if (service & ((1 << SUB_INT_ERR0) | (1 << SUB_INT_TXD0) | (1 << SUB_INT_RXD0))) {
		s3c2410_intc_assert(x49gp, INT_UART0, 1);
	} else {
		s3c2410_intc_deassert(x49gp, INT_UART0);
	}

	if (service & ((1 << SUB_INT_ERR1) | (1 << SUB_INT_TXD1) | (1 << SUB_INT_RXD1))) {
		s3c2410_intc_assert(x49gp, INT_UART1, 1);
	} else {
		s3c2410_intc_deassert(x49gp, INT_UART1);
	}

	if (service & ((1 << SUB_INT_ERR2) | (1 << SUB_INT_TXD2) | (1 << SUB_INT_RXD2))) {
		s3c2410_intc_assert(x49gp, INT_UART2, 1);
	} else {
		s3c2410_intc_deassert(x49gp, INT_UART2);
	}

	if (service & ((1 << SUB_INT_ADC) | (1 << SUB_INT_TC))) {
		s3c2410_intc_assert(x49gp, INT_ADC, 1);
	} else {
		s3c2410_intc_deassert(x49gp, INT_ADC);
	}

	intc->subsrcpnd = intc->subsrc_pending;
}

void
s3c2410_intc_sub_assert(x49gp_t *x49gp, int sub_irq, int level)
{
	s3c2410_intc_t *intc = x49gp->s3c2410_intc;

	if (sub_irq > 31)
		return;

#ifdef DEBUG_S3C2410_INTC
	printf("INTC: assert subirq %u (%08x)\n", sub_irq, 1 << sub_irq);
#endif

	if (! (intc->subsrc_pending & (1 << sub_irq))) {
		if (level)
			intc->subsrc_pending |= (1 << sub_irq);
		intc->subsrcpnd |= (1 << sub_irq);

		s3c2410_intc_gen_int_from_sub_int(intc);
	}
}

void
s3c2410_intc_sub_deassert(x49gp_t *x49gp, int sub_irq)
{
	s3c2410_intc_t *intc = x49gp->s3c2410_intc;

	if (sub_irq > 31)
		return;

#ifdef DEBUG_S3C2410_INTC
	printf("INTC: deassert subirq %u (%08x)\n", sub_irq, 1 << sub_irq);
#endif

	intc->subsrc_pending &= ~(1 << sub_irq);
}

static uint32_t
s3c2410_intc_read(void *opaque, target_phys_addr_t offset)
{
	s3c2410_intc_t *intc = opaque;
	s3c2410_offset_t *reg;
	uint32_t data;

	if (! S3C2410_OFFSET_OK(intc, offset)) {
		return ~(0);
	}

	reg = S3C2410_OFFSET_ENTRY(intc, offset);

	switch (offset) {
	case S3C2410_INTC_PRIORITY:
		data = priority_get_word(intc);
		break;
	default:
		data = *(reg->datap);
		break;
	}

#ifdef DEBUG_S3C2410_INTC
	printf("read  %s [%08x] %s [%08lx] data %08x\n",
		"s3c2410-intc", S3C2410_INTC_BASE,
		reg->name, (unsigned long) offset, data);
#endif

	return data;
}

static void
s3c2410_intc_write(void *opaque, target_phys_addr_t offset, uint32_t data)
{
#ifdef DEBUG_S3C2410_INTC
	s3c2410_offset_t *reg;
#endif
	s3c2410_intc_t *intc = opaque;

	if (! S3C2410_OFFSET_OK(intc, offset)) {
		return;
	}

#ifdef DEBUG_S3C2410_INTC
	reg = S3C2410_OFFSET_ENTRY(intc, offset);

	printf("write %s [%08x] %s [%08lx] data %08x\n",
		"s3c2410-intc", S3C2410_INTC_BASE,
		reg->name, (unsigned long) offset, data);
#endif

	switch (offset) {
	case S3C2410_INTC_SRCPND:
		srcpnd_put_word(intc, data);
		break;
	case S3C2410_INTC_INTMOD:
		intmod_put_word(intc, data);
		break;
	case S3C2410_INTC_INTMSK:
		intmsk_put_word(intc, data);
		break;
	case S3C2410_INTC_PRIORITY:
		priority_put_word(intc, data);
		break;
	case S3C2410_INTC_INTPND:
		intpnd_put_word(intc, data);
		break;
	case S3C2410_INTC_SUBSRCPND:
		subsrcpnd_put_word(intc, data);
		break;
	case S3C2410_INTC_INTSUBMSK:
		intsubmsk_put_word(intc, data);
		break;
	default:
		break;
	}
}

static int
s3c2410_intc_load(x49gp_module_t *module, GKeyFile *key)
{
	s3c2410_intc_t *intc = module->user_data;
	s3c2410_offset_t *reg;
	int error = 0;
	int i;

#ifdef DEBUG_X49GP_MODULES
	printf("%s: %s:%u\n", module->name, __FUNCTION__, __LINE__);
#endif

	for (i = 0; i < intc->nr_regs; i++) {
		reg = &intc->regs[i];

		if (NULL == reg->name)
			continue;

		if (x49gp_module_get_u32(module, key, reg->name,
					 reg->reset, reg->datap))
			error = -EAGAIN;
	}

	intc->src_pending = intc->srcpnd;
	intc->subsrc_pending = intc->subsrcpnd;
	intc->srcpnd = 0;
	intc->subsrcpnd = 0;

	priority_put_word(intc, intc->priority);
	intsubmsk_put_word(intc, intc->intsubmsk);
	intmsk_put_word(intc, intc->intmsk);

	return error;
}

static int
s3c2410_intc_save(x49gp_module_t *module, GKeyFile *key)
{
	s3c2410_intc_t *intc = module->user_data;
	s3c2410_offset_t *reg;
	int i;

#ifdef DEBUG_X49GP_MODULES
	printf("%s: %s:%u\n", module->name, __FUNCTION__, __LINE__);
#endif

	intc->srcpnd = intc->src_pending;
	intc->subsrcpnd = intc->subsrc_pending;

	for (i = 0; i < intc->nr_regs; i++) {
		reg = &intc->regs[i];

		if (NULL == reg->name)
			continue;

		x49gp_module_set_u32(module, key, reg->name, *(reg->datap));
	}

	return 0;
}

static int
s3c2410_intc_reset(x49gp_module_t *module, x49gp_reset_t reset)
{
	s3c2410_intc_t *intc = module->user_data;
	s3c2410_offset_t *reg;
	int i;

#ifdef DEBUG_X49GP_MODULES
	printf("%s: %s:%u\n", module->name, __FUNCTION__, __LINE__);
#endif

	if (reset == X49GP_RESET_POWER_OFF) {
		return 0;
	}

	for (i = 0; i < intc->nr_regs; i++) {
		reg = &intc->regs[i];

		if (NULL == reg->name)
			continue;

		*(reg->datap) = reg->reset;
	}

	return 0;
}

static CPUReadMemoryFunc *s3c2410_intc_readfn[] =
{
	s3c2410_intc_read,
	s3c2410_intc_read,
	s3c2410_intc_read
};

static CPUWriteMemoryFunc *s3c2410_intc_writefn[] =
{
	s3c2410_intc_write,
	s3c2410_intc_write,
	s3c2410_intc_write
};

static int
s3c2410_intc_init(x49gp_module_t *module)
{
	s3c2410_intc_t *intc;
	int iotype;

#ifdef DEBUG_X49GP_MODULES
	printf("%s: %s:%u\n", module->name, __FUNCTION__, __LINE__);
#endif

	intc = malloc(sizeof(s3c2410_intc_t));
	if (NULL == intc) {
		fprintf(stderr, "%s:%u: Out of memory\n",
			__FUNCTION__, __LINE__);
		return -ENOMEM;
	}
	if (s3c2410_intc_data_init(intc)) {
		free(intc);
		return -ENOMEM;
	}

	module->user_data = intc;

	intc->x49gp = module->x49gp;
	intc->x49gp->s3c2410_intc = intc;

	iotype = cpu_register_io_memory(s3c2410_intc_readfn,
					s3c2410_intc_writefn, intc);
#ifdef DEBUG_S3C2410_INTC
	printf("%s: iotype %08x\n", __FUNCTION__, iotype);
#endif
	cpu_register_physical_memory(S3C2410_INTC_BASE, S3C2410_MAP_SIZE, iotype);
	return 0;
}

static int
s3c2410_intc_exit(x49gp_module_t *module)
{
	s3c2410_intc_t *intc;

#ifdef DEBUG_X49GP_MODULES
	printf("%s: %s:%u\n", module->name, __FUNCTION__, __LINE__);
#endif

	if (module->user_data) {
		intc = module->user_data;
		if (intc->regs)
			free(intc->regs);
		free(intc);
	}

	x49gp_module_unregister(module);
	free(module);

	return 0;
}

int
x49gp_s3c2410_intc_init(x49gp_t *x49gp)
{
	x49gp_module_t *module;

	if (x49gp_module_init(x49gp, "s3c2410-intc",
			      s3c2410_intc_init,
			      s3c2410_intc_exit,
			      s3c2410_intc_reset,
			      s3c2410_intc_load,
			      s3c2410_intc_save,
			      NULL, &module)) {
		return -1;
	}

	return x49gp_module_register(module);
}
