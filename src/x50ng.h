#ifndef _X50NG_H
#define _X50NG_H

#include <sys/types.h>
#include <stdint.h>
#include <sys/times.h>

#include <glib.h>

#include <memory.h>

#include "target-arm/cpu.h"

#include "types.h"
#include "timer.h"
#include "list.h"

/* LD TEMPO HACK */
extern uint8_t* phys_ram_base;
extern int phys_ram_size;

typedef enum { X50NG_ARM_RUN = 0, X50NG_ARM_SLEEP, X50NG_ARM_OFF } x50ng_arm_idle_t;

typedef enum { X50NG_RESET_POWER_ON = 0, X50NG_RESET_POWER_OFF, X50NG_RESET_WATCHDOG } x50ng_reset_t;

struct __x50ng_module_s__;
typedef struct __x50ng_module_s__ x50ng_module_t;

struct __x50ng_module_s__ {
    const char* name;

    int ( *init )( x50ng_module_t* );
    int ( *exit )( x50ng_module_t* );

    int ( *reset )( x50ng_module_t*, x50ng_reset_t );

    int ( *load )( x50ng_module_t*, GKeyFile* );
    int ( *save )( x50ng_module_t*, GKeyFile* );

    void* user_data;

    x50ng_t* x50ng;
    struct list_head list;
};

typedef enum { X50NG_REINIT_NONE = 0, X50NG_REINIT_REBOOT_ONLY, X50NG_REINIT_FLASH, X50NG_REINIT_FLASH_FULL } x50ng_reinit_t;

struct __x50ng_s__ {
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

    x50ng_timer_t* timer_ui_input;
    x50ng_timer_t* timer_ui_output;

    x50ng_arm_idle_t arm_idle;
    int arm_exit;

    GKeyFile* state;
    const char* progname;
    const char* progpath;
};

/* main.c */
extern void x50ng_set_idle( x50ng_t*, x50ng_arm_idle_t idle );

/* s3c2410/s3c2410_sdi.c */
extern void s3c2410_sdi_unmount( x50ng_t* x50ng );
extern int s3c2410_sdi_mount( x50ng_t* x50ng, char* filename );
extern bool s3c2410_sdi_is_mounted( x50ng_t* x50ng );
extern void s3c2410_sdi_get_path( x50ng_t* x50ng, char** filename );

/* module.c */
extern int x50ng_module_init( x50ng_t* x50ng, const char* name, int ( *init )( x50ng_module_t* ), int ( *exit )( x50ng_module_t* ),
                              int ( *reset )( x50ng_module_t*, x50ng_reset_t ), int ( *load )( x50ng_module_t*, GKeyFile* ),
                              int ( *save )( x50ng_module_t*, GKeyFile* ), void* user_data, x50ng_module_t** module );

extern int x50ng_module_register( x50ng_module_t* module );
extern int x50ng_module_unregister( x50ng_module_t* module );

extern int x50ng_module_get_filename( x50ng_module_t* module, GKeyFile*, const char*, char*, char**, char** );
extern int x50ng_module_set_filename( x50ng_module_t* module, GKeyFile*, const char*, const char* );
extern int x50ng_module_get_int( x50ng_module_t* module, GKeyFile*, const char*, int, int* );
extern int x50ng_module_set_int( x50ng_module_t* module, GKeyFile*, const char*, int );
extern int x50ng_module_get_uint( x50ng_module_t* module, GKeyFile*, const char*, unsigned int, unsigned int* );
extern int x50ng_module_set_uint( x50ng_module_t* module, GKeyFile*, const char*, unsigned int );
extern int x50ng_module_get_u32( x50ng_module_t* module, GKeyFile*, const char*, uint32_t, uint32_t* );
extern int x50ng_module_set_u32( x50ng_module_t* module, GKeyFile*, const char*, uint32_t );
extern int x50ng_module_get_u64( x50ng_module_t* module, GKeyFile*, const char*, uint64_t, uint64_t* );
extern int x50ng_module_set_u64( x50ng_module_t* module, GKeyFile*, const char*, uint64_t );
extern int x50ng_module_get_string( x50ng_module_t* module, GKeyFile*, const char*, char*, char** );
extern int x50ng_module_set_string( x50ng_module_t* module, GKeyFile*, const char*, const char* );
extern int x50ng_module_open_rodata( x50ng_module_t* module, const char* name, char** path );

extern int x50ng_modules_init( x50ng_t* );
extern int x50ng_modules_exit( x50ng_t* );
extern int x50ng_modules_reset( x50ng_t*, x50ng_reset_t );
extern int x50ng_modules_load( x50ng_t* );
extern int x50ng_modules_save( x50ng_t* );

/* flash.c */
extern int x50ng_flash_init( x50ng_t* );

/* sram.c */
extern int x50ng_sram_init( x50ng_t* );

#endif /* !(_X50NG_H) */
