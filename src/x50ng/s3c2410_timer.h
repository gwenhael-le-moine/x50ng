#ifndef _S3C2410_TIMER_H
#define _S3C2410_TIMER_H 1

#include <time.h>

#include "x49gp_types.h"

#define TCFG0_DEAD_SHIFT 16
#define TCFG0_DEAD_MASK 0xff
#define TCFG0_PRE1_SHIFT 8
#define TCFG0_PRE0_SHIFT 0
#define TCFG0_PREx_MASK 0xff

#define TCFG1_DMA_SHIFT 20
#define TCFG1_DMA_MASK 0x0f
#define TCFG1_MUX4_SHIFT 16
#define TCFG1_MUX3_SHIFT 12
#define TCFG1_MUX2_SHIFT 8
#define TCFG1_MUX1_SHIFT 4
#define TCFG1_MUX0_SHIFT 0
#define TCFG1_MUXx_MASK 0x0f

#define TCON_TIMER4_RELOAD 0x00400000
#define TCON_TIMER4_UPDATE 0x00200000
#define TCON_TIMER4_START 0x00100000
#define TCON_TIMER3_RELOAD 0x00080000
#define TCON_TIMER3_INVERT 0x00040000
#define TCON_TIMER3_UPDATE 0x00020000
#define TCON_TIMER3_START 0x00010000
#define TCON_TIMER2_RELOAD 0x00008000
#define TCON_TIMER2_INVERT 0x00004000
#define TCON_TIMER2_UPDATE 0x00002000
#define TCON_TIMER2_START 0x00001000
#define TCON_TIMER1_RELOAD 0x00000800
#define TCON_TIMER1_INVERT 0x00000400
#define TCON_TIMER1_UPDATE 0x00000200
#define TCON_TIMER1_START 0x00000100
#define TCON_TIMER0_DEADZONE 0x00000010
#define TCON_TIMER0_RELOAD 0x00000008
#define TCON_TIMER0_INVERT 0x00000004
#define TCON_TIMER0_UPDATE 0x00000002
#define TCON_TIMER0_START 0x00000001

void s3c2410_run_timers( x49gp_t* x49gp );
clock_t s3c2410_next_timer( x49gp_t* x49gp );

#endif /* !(_S3C2410_TIMER_H) */
