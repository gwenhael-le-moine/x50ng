#ifndef _X50NG_H
#define _X50NG_H

#include <memory.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/times.h>

#include <glib.h>

#include "target-arm/cpu.h"

#include "types.h"
#include "timer.h"
#include "list.h"

typedef enum { HDW_ARM_RUN = 0, HDW_ARM_SLEEP, HDW_ARM_OFF } hdw_arm_idle_t;
typedef enum { HDW_RESET_POWER_ON = 0, HDW_RESET_POWER_OFF, HDW_RESET_WATCHDOG } hdw_reset_t;

typedef struct hdw_module_t {
    const char* name;

    int ( *init )( struct hdw_module_t* );
    int ( *exit )( struct hdw_module_t* );

    int ( *reset )( struct hdw_module_t*, hdw_reset_t );

    int ( *load )( struct hdw_module_t*, GKeyFile* );
    int ( *save )( struct hdw_module_t*, GKeyFile* );

    void* user_data;

    struct hdw_t* hdw_state;
    struct list_head list;
} hdw_module_t;

struct hdw_t {
    CPUARMState* env;

    struct list_head modules;

    void* s3c2410_lcd;
    void* s3c2410_timer;
    void* s3c2410_watchdog;
    void* s3c2410_intc;
    void* s3c2410_io_port;
    void* s3c2410_sdi;

    void* timer;
    uint8_t* sram;

    uint32_t MCLK;
    uint32_t UCLK;

    uint32_t FCLK;
    uint32_t HCLK;
    uint32_t PCLK;
    int PCLK_ratio;

    clock_t clk_tck;
    unsigned long emulator_fclk;

    unsigned char keybycol[ 8 ];
    unsigned char keybyrow[ 8 ];

    hdw_timer_t* timer_ui_input;
    hdw_timer_t* timer_ui_output;

    hdw_arm_idle_t arm_idle;
    int arm_exit;

    GKeyFile* state;
};

/* LD TEMPO HACK */
extern uint8_t* phys_ram_base;
extern int phys_ram_size;

extern void hdw_set_idle( hdw_t*, hdw_arm_idle_t idle );
extern void hdw_stop( hdw_t* );

#endif /* !(_X50NG_H) */
