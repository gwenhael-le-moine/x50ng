/* $Id: timer.c,v 1.4 2008/12/11 12:18:17 ecd Exp $
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <limits.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <errno.h>
#include <poll.h>

#include <x49gp.h>
#include <x49gp_timer.h>
#include <s3c2410.h>

#include <glib.h>

#include "gdbstub.h"

typedef struct {
	long			type;
} x49gp_clock_t;

struct x49gp_timer_s {
	long			type;
	int64_t			expires;
	x49gp_timer_cb_t	cb;
	void			*user_data;
	x49gp_timer_t		*next;
};

typedef x49gp_timer_cb_t	QEMUTimerCB;
typedef void *			QEMUClock;
QEMUClock			*rt_clock = (void *) X49GP_TIMER_REALTIME;
QEMUClock			*vm_clock = (void *) X49GP_TIMER_VIRTUAL;
int64_t				ticks_per_sec = 1000000;

static x49gp_timer_t	*x49gp_timer_lists[2];

int64_t
x49gp_get_clock(void)
{
	struct timeval tv;
	int64_t us;

	gettimeofday(&tv, NULL);

	us = tv.tv_sec * 1000000LL + tv.tv_usec;

	return us;
}

x49gp_timer_t *
x49gp_new_timer(long type, x49gp_timer_cb_t cb, void *user_data)
{
	x49gp_timer_t *ts;

	ts = malloc(sizeof(x49gp_timer_t));
	if (NULL == ts) {
		return NULL;
	}
	memset(ts, 0, sizeof(x49gp_timer_t));

	ts->type = type;
	ts->cb = cb;
	ts->user_data = user_data;

	return ts;
}

void
x49gp_free_timer(x49gp_timer_t *ts)
{
	free(ts);
}

void
x49gp_del_timer(x49gp_timer_t *ts)
{
	x49gp_timer_t **pt, *t;

// printf("%s: ts %p\n", __FUNCTION__, ts);
	pt = &x49gp_timer_lists[ts->type];
	while (1) {
		t = *pt;
		if (NULL == t)
			break;
		if (t == ts) {
			*pt = t->next;
			ts->next = NULL;
			break;
		}
		pt = &t->next;
	}
}

void
x49gp_mod_timer(x49gp_timer_t *ts, int64_t expires)
{
	x49gp_timer_t **pt, *t;

	x49gp_del_timer(ts);

// printf("%s: ts %p, expires %lld\n", __FUNCTION__, ts, expires);
	pt = &x49gp_timer_lists[ts->type];
	while (1) {
		t = *pt;
		if (NULL == t)
			break;
		if (t->expires > expires)
			break;
		pt = &t->next;
	}

	ts->expires = expires;
	ts->next = *pt;
	*pt = ts;
}

int
x49gp_timer_pending(x49gp_timer_t *ts)
{
	x49gp_timer_t *t;

	for (t = x49gp_timer_lists[ts->type]; t; t = t->next) {
		if (t == ts)
			return 1;
	}

	return 0;
}

int64_t
x49gp_timer_expires(x49gp_timer_t *ts)
{
	return ts->expires;
}

static int
x49gp_timer_expired(x49gp_timer_t *timer_head, int64_t current_time)
{
	if (NULL == timer_head)
		return 0;
	return (timer_head->expires <= current_time);
}

/* LD TEMPO HACK */

QEMUTimer *
qemu_new_timer(QEMUClock *clock, QEMUTimerCB cb, void *opaque)
{
	return (void *) x49gp_new_timer((long) clock, cb, opaque);
}

void
qemu_free_timer(QEMUTimer *ts)
{
	return x49gp_free_timer((void *) ts);
}

void
qemu_mod_timer(QEMUTimer *ts, int64_t expire_time)
{
	return x49gp_mod_timer((void *) ts, expire_time);
}

void
qemu_del_timer(QEMUTimer *ts)
{
	return x49gp_del_timer((void *) ts);
}

int
qemu_timer_pending(QEMUTimer *ts)
{
	return x49gp_timer_pending((void *) ts);
}

int64_t
qemu_get_clock(QEMUClock *clock)
{
	return x49gp_get_clock();
}

static void
x49gp_run_timers(x49gp_timer_t **ptimer_head, int64_t current_time)
{
	x49gp_timer_t *ts;

// printf("%s: now %lld\n", __FUNCTION__, current_time);
	while (1) {
		ts = *ptimer_head;
		if (NULL == ts || ts->expires > current_time)
			break;

		*ptimer_head = ts->next;
		ts->next = NULL;

// printf("%s: call ts %p\n", __FUNCTION__, ts);
		ts->cb(ts->user_data);
// printf("%s: ts %p done\n", __FUNCTION__, ts);
	}

// printf("%s: timers done\n", __FUNCTION__);
}

static void
x49gp_alarm_handler(int sig)
{
	if (x49gp_timer_expired(x49gp_timer_lists[X49GP_TIMER_VIRTUAL],
				x49gp_get_clock()) ||
	    x49gp_timer_expired(x49gp_timer_lists[X49GP_TIMER_REALTIME],
				x49gp_get_clock())) {
		if (cpu_single_env && ! cpu_single_env->exit_request) {
			cpu_exit(cpu_single_env);
		}
	}
}

static void
x49gp_main_loop_wait(x49gp_t *x49gp, int timeout)
{
// printf("%s: timeout: %d\n", __FUNCTION__, timeout);

	if (gdb_poll(x49gp->env)) {
		gdb_handlesig(x49gp->env, 0);
	} else
	poll(NULL, 0, timeout);

	if (x49gp->arm_idle != X49GP_ARM_OFF) {
		x49gp_run_timers(&x49gp_timer_lists[X49GP_TIMER_VIRTUAL],
				 x49gp_get_clock());
	}

	x49gp_run_timers(&x49gp_timer_lists[X49GP_TIMER_REALTIME],
			 x49gp_get_clock());

// printf("%s: done\n", __FUNCTION__);
}

int
x49gp_main_loop(x49gp_t *x49gp)
{
	int prev_idle;
	int ret, timeout;

	while (! x49gp->arm_exit) {
		prev_idle = x49gp->arm_idle;

		if (x49gp->arm_idle == X49GP_ARM_RUN) {
#ifdef DEBUG_X49GP_TIMER_IDLE
			printf("%lld: %s: call cpu_exec(%p)\n", (unsigned long long) x49gp_get_clock(),  __FUNCTION__, x49gp->env);
#endif
			ret = cpu_exec(x49gp->env);
#ifdef DEBUG_X49GP_TIMER_IDLE
			printf("%lld: %s: cpu_exec(): %d, PC %08x\n", (unsigned long long) x49gp_get_clock(), __FUNCTION__, ret, x49gp->env->regs[15]);
#endif

if (x49gp->env->regs[15] == 0x8620) {
printf("PC %08x: SRAM %08x: %08x %08x %08x <%08x>\n", x49gp->env->regs[15], 0x08000a0c,
	* ((uint32_t *) &x49gp->sram[0x0a00]),
	* ((uint32_t *) &x49gp->sram[0x0a04]),
	* ((uint32_t *) &x49gp->sram[0x0a08]),
	* ((uint32_t *) &x49gp->sram[0x0a0c]));
* ((uint32_t *) &x49gp->sram[0x0a0c]) = 0x00000000;
}

			if (ret == EXCP_DEBUG) {
				gdb_handlesig(x49gp->env, SIGTRAP);
				continue;
			}

			if (x49gp->arm_idle != prev_idle) {
				if (x49gp->arm_idle == X49GP_ARM_OFF) {
					x49gp_lcd_update(x49gp);
					cpu_reset(x49gp->env);
				}
			}

			if (ret == EXCP_HALTED) {
				timeout = 10;
			} else {
				timeout = 0;
			}
		} else {
			timeout = 1;
		}

		x49gp_main_loop_wait(x49gp, timeout);
	}

	return 0;
}

int
x49gp_timer_init(x49gp_t *x49gp)
{
	struct sigaction sa;
	struct itimerval it;

	x49gp_timer_lists[X49GP_TIMER_VIRTUAL] = NULL;
	x49gp_timer_lists[X49GP_TIMER_REALTIME] = NULL;

	sigfillset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	sa.sa_handler = x49gp_alarm_handler;
	sigaction(SIGALRM, &sa, NULL);

	it.it_interval.tv_sec = 0;
	it.it_interval.tv_usec = 1000;
	it.it_value.tv_sec = 0;
	it.it_value.tv_usec = 1000;

	setitimer(ITIMER_REAL, &it, NULL);
	return 0;
}
