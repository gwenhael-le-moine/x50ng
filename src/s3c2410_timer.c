/* $Id: s3c2410_timer.c,v 1.4 2008/12/11 12:18:17 ecd Exp $
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>

#include <x49gp.h>
#include <s3c2410.h>
#include <s3c2410_timer.h>
#include <s3c2410_intc.h>


typedef struct {
	uint32_t	reload_bit;
	uint32_t	update_bit;
	uint32_t	start_bit;
	unsigned int	pre_shift;
	unsigned int	mux_shift;
	int		irq;
} s3c2410_timer_config_t;

struct __s3c2410_timer_s__;
typedef struct __s3c2410_timer_s__ s3c2410_timer_t;

struct __s3c2410_timer_s__ {
	uint32_t		tcfg0;
	uint32_t		tcfg1;
	uint32_t		tcon;
	uint32_t		prev_tcon;

	x49gp_t			*x49gp;

	unsigned int		nr_regs;
	s3c2410_offset_t	*regs;

	struct s3c2410_timeout {
		uint32_t			tcntb;
		uint32_t			tcmpb;
		uint32_t			tcnt;
		uint32_t			tcmp;

		const s3c2410_timer_config_t	*tconfig;
		int				index;

		s3c2410_timer_t			*main;

		unsigned long			interval;
		x49gp_timer_t			*timer;
	} timeout[5];
};

static const s3c2410_timer_config_t s3c2410_timer_config[] =
{
	{
		TCON_TIMER0_RELOAD, TCON_TIMER0_UPDATE, TCON_TIMER0_START,
		TCFG0_PRE0_SHIFT, TCFG1_MUX0_SHIFT, INT_TIMER0
	},
	{
		TCON_TIMER1_RELOAD, TCON_TIMER1_UPDATE, TCON_TIMER1_START,
		TCFG0_PRE0_SHIFT, TCFG1_MUX1_SHIFT, INT_TIMER1
	},
	{
		TCON_TIMER2_RELOAD, TCON_TIMER2_UPDATE, TCON_TIMER2_START,
		TCFG0_PRE1_SHIFT, TCFG1_MUX2_SHIFT, INT_TIMER2
	},
	{
		TCON_TIMER3_RELOAD, TCON_TIMER3_UPDATE, TCON_TIMER3_START,
		TCFG0_PRE1_SHIFT, TCFG1_MUX3_SHIFT, INT_TIMER3
	},
	{
		TCON_TIMER4_RELOAD, TCON_TIMER4_UPDATE, TCON_TIMER4_START,
		TCFG0_PRE1_SHIFT, TCFG1_MUX4_SHIFT, INT_TIMER4
	},
};

static int
s3c2410_timer_data_init(s3c2410_timer_t *timer)
{
	s3c2410_offset_t regs[] = {
		S3C2410_OFFSET(TIMER, TCFG0, 0, timer->tcfg0),
		S3C2410_OFFSET(TIMER, TCFG1, 0, timer->tcfg1),
		S3C2410_OFFSET(TIMER, TCON, 0, timer->tcon),
		S3C2410_OFFSET(TIMER, TCNTB0, 0, timer->timeout[0].tcntb),
		S3C2410_OFFSET(TIMER, TCMPB0, 0, timer->timeout[0].tcmpb),
		S3C2410_OFFSET(TIMER, TCNTO0, 0, timer->timeout[0].tcnt),
		S3C2410_OFFSET(TIMER, TCNTB1, 0, timer->timeout[1].tcntb),
		S3C2410_OFFSET(TIMER, TCMPB1, 0, timer->timeout[1].tcmpb),
		S3C2410_OFFSET(TIMER, TCNTO1, 0, timer->timeout[1].tcnt),
		S3C2410_OFFSET(TIMER, TCNTB2, 0, timer->timeout[2].tcntb),
		S3C2410_OFFSET(TIMER, TCMPB2, 0, timer->timeout[2].tcmpb),
		S3C2410_OFFSET(TIMER, TCNTO2, 0, timer->timeout[2].tcnt),
		S3C2410_OFFSET(TIMER, TCNTB3, 0, timer->timeout[3].tcntb),
		S3C2410_OFFSET(TIMER, TCMPB3, 0, timer->timeout[3].tcmpb),
		S3C2410_OFFSET(TIMER, TCNTO3, 0, timer->timeout[3].tcnt),
		S3C2410_OFFSET(TIMER, TCNTB4, 0, timer->timeout[4].tcntb),
		S3C2410_OFFSET(TIMER, TCNTO4, 0, timer->timeout[4].tcnt)
	};

	memset(timer, 0, sizeof(s3c2410_timer_t));

	timer->regs = malloc(sizeof(regs));
	if (NULL == timer->regs) {
		fprintf(stderr, "%s:%u: Out of memory\n",
			__FUNCTION__, __LINE__);
		return -ENOMEM;
	}

	memcpy(timer->regs, regs, sizeof(regs));
	timer->nr_regs = sizeof(regs) / sizeof(regs[0]);

	return 0;
}

static void
s3c2410_timer_timeout(void *data)
{
	struct s3c2410_timeout *t = data;
	s3c2410_timer_t *timer = t->main;
	x49gp_t *x49gp = timer->x49gp;
	int64_t timeout;

#ifdef DEBUG_S3C2410_TIMER
	printf("s3c2410-timer: assert TIMER%u interrupt\n", t->index);
#endif

	s3c2410_intc_assert(timer->x49gp, t->tconfig->irq, 0);

	if (timer->tcon & t->tconfig->reload_bit) {
		t->tcnt = t->tcntb;
		t->tcmp = t->tcmpb;
	} else {
		timer->tcon &= ~(t->tconfig->start_bit);
		return;
	}

	timeout = 1000000LL * t->tcnt * t->interval / x49gp->PCLK;
#ifdef DEBUG_S3C2410_TIMER
	printf("s3c2410-timer: reload TIMER%u: CNT %u (%lu PCLKs): %llu us\n", t->index, t->tcnt, t->interval, (unsigned long long) timeout);
#endif
	x49gp_mod_timer(t->timer, x49gp_get_clock() + timeout);
}

unsigned long
s3c2410_timer_next_interrupt(x49gp_t *x49gp)
{
	s3c2410_timer_t *timer = x49gp->s3c2410_timer;
	struct s3c2410_timeout *t;
	unsigned long irq, next;
	unsigned long ticks;
	int i;

	ticks = x49gp_get_clock();

	next = ~(0);
	for (i = 0; i < 5; i++) {
		t = &timer->timeout[i];

		if (!(timer->tcon & t->tconfig->start_bit))
			continue;

		if (x49gp_timer_pending(t->timer)) {
			irq = x49gp_timer_expires(t->timer) - ticks;
		} else {
			irq = 0;
		}

		if (t->tcnt) {
			irq += (t->tcnt - 1) * t->interval;
		} else {
			if (!(timer->tcon & t->tconfig->reload_bit))
				continue;
			irq += t->tcntb * t->interval;
		}

		if (irq < next)
			next = irq;

#ifdef DEBUG_S3C2410_TIMER
		printf("s3c2410-timer: TIMER%u: tcnt %u, interval %lu, pending %u, next irq %lu\n",
			t->index, t->tcnt, t->interval, x49gp_timer_pending(t->timer), irq);
#endif
	}

	return next;
}

static void
s3c2410_update_tcfg(s3c2410_timer_t *timer)
{
	struct s3c2410_timeout *t;
	x49gp_t *x49gp = timer->x49gp;
	uint32_t pre, mux;
	int64_t timeout;
	int i;

	for (i = 0; i < 5; i++) {
		t = &timer->timeout[i];

		pre = (timer->tcfg0 >> t->tconfig->pre_shift) & TCFG0_PREx_MASK;
		mux = (timer->tcfg1 >> t->tconfig->mux_shift) & TCFG1_MUXx_MASK;

		if (mux > 3) {
			printf("s3c2410-timer: can't handle MUX %02x for TIMER%u\n",
				mux, t->index);
			mux = 3;
		}

		t->interval = (pre + 1) * (2 << mux);

#ifdef DEBUG_S3C2410_TIMER
		printf("s3c2410-timer: TIMER%u: pre %u, mux %u, tick %lu PCLKs\n",
			t->index, pre, mux, t->interval);
#endif
		if (x49gp_timer_pending(t->timer)) {
			timeout = 1000000LL * t->tcnt * t->interval / x49gp->PCLK;
#ifdef DEBUG_S3C2410_TIMER
			printf("s3c2410-timer: mod TIMER%u: CNT %u (%lu PCLKs): %llu us\n", t->index, t->tcnt, t->interval, (unsigned long long) timeout);
#endif
			x49gp_mod_timer(t->timer, x49gp_get_clock() + timeout);
		}
	}
}

static void
s3c2410_update_tcon(s3c2410_timer_t *timer)
{
	struct s3c2410_timeout *t;
	x49gp_t *x49gp = timer->x49gp;
	int64_t timeout;
	uint32_t change;
	int i;

	change = timer->prev_tcon ^ timer->tcon;
	timer->prev_tcon = timer->tcon;

	for (i = 0; i < 5; i++) {
		t = &timer->timeout[i];

		if (timer->tcon & t->tconfig->update_bit) {
			t->tcnt = t->tcntb;
			t->tcmp = t->tcmpb;

#ifdef DEBUG_S3C2410_TIMER
			printf("s3c2410-timer: update TIMER%u tcnt %u, tcmp %u\n", t->index, t->tcnt, t->tcmp);
#endif
		}

		if (change & t->tconfig->start_bit) {
			if (timer->tcon & t->tconfig->start_bit) {
				timeout = 1000000LL * t->tcnt * t->interval / x49gp->PCLK;
#ifdef DEBUG_S3C2410_TIMER
				printf("s3c2410-timer: start TIMER%u: CNT %u (%lu PCLKs): %llu us\n", t->index, t->tcnt, t->interval, (unsigned long long) timeout);
#endif
				x49gp_mod_timer(t->timer, x49gp_get_clock() + timeout);
			} else {
				x49gp_del_timer(t->timer);
#ifdef DEBUG_S3C2410_TIMER
				printf("s3c2410-timer: stop TIMER%u\n", t->index);
#endif
			}
		}
	}
}

static uint32_t
s3c2410_read_tcnt(s3c2410_timer_t *timer, int index)
{
	struct s3c2410_timeout *t = &timer->timeout[index];
	x49gp_t *x49gp = timer->x49gp;
	int64_t now, expires, timeout;

	if (!(timer->tcon & t->tconfig->start_bit))
		return t->tcnt;

	if (x49gp_timer_pending(t->timer)) {
		now = x49gp_get_clock();
		expires = x49gp_timer_expires(t->timer);

		timeout = expires - now;
		if (timeout <= 0)
			return 0;

		t->tcnt = timeout * x49gp->PCLK / (1000000LL * t->interval);
	}

	return t->tcnt;
}

static uint32_t
s3c2410_timer_read(void *opaque, target_phys_addr_t offset)
{
	s3c2410_timer_t *timer = opaque;
	s3c2410_offset_t *reg;
	uint32_t data;

	if (! S3C2410_OFFSET_OK(timer, offset)) {
		return ~(0);
	}

	reg = S3C2410_OFFSET_ENTRY(timer, offset);

	switch (offset) {
	case S3C2410_TIMER_TCNTO0:
		data = s3c2410_read_tcnt(timer, 0);
		break;
	case S3C2410_TIMER_TCNTO1:
		data = s3c2410_read_tcnt(timer, 1);
		break;
	case S3C2410_TIMER_TCNTO2:
		data = s3c2410_read_tcnt(timer, 2);
		break;
	case S3C2410_TIMER_TCNTO3:
		data = s3c2410_read_tcnt(timer, 3);
		break;
	case S3C2410_TIMER_TCNTO4:
		data = s3c2410_read_tcnt(timer, 4);
		break;
	default:
		data = *(reg->datap);
		break;
	}

#ifdef DEBUG_S3C2410_TIMER
	printf("read  %s [%08x] %s [%08lx] data %08x\n",
		"s3c2410-timer", S3C2410_TIMER_BASE,
		reg->name, (unsigned long) offset, data);
#endif

	return data;
}

static void
s3c2410_timer_write(void *opaque, target_phys_addr_t offset, uint32_t data)
{
	s3c2410_timer_t *timer = opaque;
	s3c2410_offset_t *reg;

	if (! S3C2410_OFFSET_OK(timer, offset)) {
		return;
	}

	reg = S3C2410_OFFSET_ENTRY(timer, offset);

#ifdef DEBUG_S3C2410_TIMER
	printf("write %s [%08x] %s [%08lx] data %08x\n",
		"s3c2410-timer", S3C2410_TIMER_BASE,
		reg->name, (unsigned long) offset, data);
#endif

	switch (offset) {
	case S3C2410_TIMER_TCFG0:
		*(reg->datap) = data;
		s3c2410_update_tcfg(timer);
		break;
	case S3C2410_TIMER_TCFG1:
		*(reg->datap) = data;
		s3c2410_update_tcfg(timer);
		break;
	case S3C2410_TIMER_TCON:
		*(reg->datap) = data;
		s3c2410_update_tcon(timer);
		break;
	case S3C2410_TIMER_TCNTO0:
	case S3C2410_TIMER_TCNTO1:
	case S3C2410_TIMER_TCNTO2:
	case S3C2410_TIMER_TCNTO3:
	case S3C2410_TIMER_TCNTO4:
		/* read only */
		break;
	default:
		*(reg->datap) = data;
		break;
	}
}

static int
s3c2410_timer_load(x49gp_module_t *module, GKeyFile *key)
{
	s3c2410_timer_t *timer = module->user_data;
	s3c2410_offset_t *reg;
	int error = 0;
	int i;

#ifdef DEBUG_X49GP_MODULES
	printf("%s: %s:%u\n", module->name, __FUNCTION__, __LINE__);
#endif

	for (i = 0; i < timer->nr_regs; i++) {
		reg = &timer->regs[i];

		if (NULL == reg->name)
			continue;

		if (x49gp_module_get_u32(module, key, reg->name,
					 reg->reset, reg->datap))
			error = -EAGAIN;
	}

	s3c2410_update_tcon(timer);
	s3c2410_update_tcfg(timer);

	return error;
}

static int
s3c2410_timer_save(x49gp_module_t *module, GKeyFile *key)
{
	s3c2410_timer_t *timer = module->user_data;
	s3c2410_offset_t *reg;
	int i;

#ifdef DEBUG_X49GP_MODULES
	printf("%s: %s:%u\n", module->name, __FUNCTION__, __LINE__);
#endif

	for (i = 0; i < timer->nr_regs; i++) {
		reg = &timer->regs[i];

		if (NULL == reg->name)
			continue;

		x49gp_module_set_u32(module, key, reg->name, *(reg->datap));
	}

	return 0;
}

static int
s3c2410_timer_reset(x49gp_module_t *module, x49gp_reset_t reset)
{
	s3c2410_timer_t *timer = module->user_data;
	s3c2410_offset_t *reg;
	int i;
	
#ifdef DEBUG_X49GP_MODULES
	printf("%s: %s:%u\n", module->name, __FUNCTION__, __LINE__);
#endif

	for (i = 0; i < timer->nr_regs; i++) {
		reg = &timer->regs[i];

		if (NULL == reg->name)
			continue;

		*(reg->datap) = reg->reset;
	}

	s3c2410_update_tcon(timer);
	s3c2410_update_tcfg(timer);

	return 0;
}

static CPUReadMemoryFunc *s3c2410_timer_readfn[] =
{
	s3c2410_timer_read,
	s3c2410_timer_read,
	s3c2410_timer_read
};

static CPUWriteMemoryFunc *s3c2410_timer_writefn[] =
{
	s3c2410_timer_write,
	s3c2410_timer_write,
	s3c2410_timer_write
};

static int
s3c2410_timer_init(x49gp_module_t *module)
{
	s3c2410_timer_t *timer;
	struct s3c2410_timeout *t;
	int iotype;
	int i;

#ifdef DEBUG_X49GP_MODULES
	printf("%s: %s:%u\n", module->name, __FUNCTION__, __LINE__);
#endif

	timer = malloc(sizeof(s3c2410_timer_t));
	if (NULL == timer) {
		fprintf(stderr, "%s: %s:%u: Out of memory\n",
			module->x49gp->progname, __FUNCTION__, __LINE__);
		return -ENOMEM;
	}
	if (s3c2410_timer_data_init(timer)) {
		free(timer);
		return -ENOMEM;
	}

	module->user_data = timer;

	timer->x49gp = module->x49gp;
	module->x49gp->s3c2410_timer = timer;

	for (i = 0; i < 5; i++) {
		t = &timer->timeout[i];

		t->tconfig = &s3c2410_timer_config[i];
		t->index = i;

		t->main = timer;

		t->timer = x49gp_new_timer(X49GP_TIMER_VIRTUAL, s3c2410_timer_timeout, t);
	}

	iotype = cpu_register_io_memory(s3c2410_timer_readfn,
					s3c2410_timer_writefn, timer);
#ifdef DEBUG_S3C2410_TIMER
	printf("%s: iotype %08x\n", __FUNCTION__, iotype);
#endif
	cpu_register_physical_memory(S3C2410_TIMER_BASE, S3C2410_MAP_SIZE, iotype);
	return 0;
}

static int
s3c2410_timer_exit(x49gp_module_t *module)
{
	s3c2410_timer_t *timer;

#ifdef DEBUG_X49GP_MODULES
	printf("%s: %s:%u\n", module->name, __FUNCTION__, __LINE__);
#endif

	if (module->user_data) {
		timer = module->user_data;
		if (timer->regs)
			free(timer->regs);
		free(timer);
	}

	x49gp_module_unregister(module);
	free(module);

	return 0;
}

int
x49gp_s3c2410_timer_init(x49gp_t *x49gp)
{
	x49gp_module_t *module;

	if (x49gp_module_init(x49gp, "s3c2410-timer",
			      s3c2410_timer_init,
			      s3c2410_timer_exit,
			      s3c2410_timer_reset,
			      s3c2410_timer_load,
			      s3c2410_timer_save,
			      NULL, &module)) {
		return -1;
	}

	return x49gp_module_register(module);
}
