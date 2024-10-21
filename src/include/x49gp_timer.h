/* $Id: x49gp_timer.h,v 1.3 2008/12/11 12:18:17 ecd Exp $
 */

#ifndef _X49GP_TIMER_H
#define _X49GP_TIMER_H 1

#include <time.h>
#include <list.h>

#define X49GP_TIMER_VIRTUAL	0
#define X49GP_TIMER_REALTIME	1

int64_t x49gp_get_clock(void);

typedef void (*x49gp_timer_cb_t) (void *);
typedef struct x49gp_timer_s x49gp_timer_t;

x49gp_timer_t *x49gp_new_timer(long type, x49gp_timer_cb_t, void *user_data);
void x49gp_free_timer(x49gp_timer_t *);

void x49gp_mod_timer(x49gp_timer_t *, int64_t expires);
void x49gp_del_timer(x49gp_timer_t *);
int x49gp_timer_pending(x49gp_timer_t *);
int64_t x49gp_timer_expires(x49gp_timer_t *);

#define X49GP_GTK_REFRESH_INTERVAL	30000LL
#define X49GP_LCD_REFRESH_INTERVAL	50000LL

int x49gp_main_loop(x49gp_t *);
int x49gp_timer_init(x49gp_t *);

#endif /* !(_X49GP_TIMER_H) */
