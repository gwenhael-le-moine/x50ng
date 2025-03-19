#ifndef _X49GP_H
#define _X49GP_H

#include <sys/types.h>
#include <stdint.h>
#include <sys/times.h>

#include <glib.h>

#include <memory.h>

#include "target-arm/cpu.h"

#include "x49gp_types.h"
#include "x49gp_timer.h"
#include "s3c2410_mmu.h"
#include "list.h"

/* LD TEMPO HACK */
extern uint8_t* phys_ram_base;
extern int phys_ram_size;

typedef enum { X49GP_ARM_RUN = 0, X49GP_ARM_SLEEP, X49GP_ARM_OFF } x49gp_arm_idle_t;

typedef enum { X49GP_RESET_POWER_ON = 0, X49GP_RESET_POWER_OFF, X49GP_RESET_WATCHDOG } x49gp_reset_t;

struct __x49gp_module_s__;
typedef struct __x49gp_module_s__ x49gp_module_t;

struct __x49gp_module_s__ {
    const char* name;

    int ( *init )( x49gp_module_t* );
    int ( *exit )( x49gp_module_t* );

    int ( *reset )( x49gp_module_t*, x49gp_reset_t );

    int ( *load )( x49gp_module_t*, GKeyFile* );
    int ( *save )( x49gp_module_t*, GKeyFile* );

    void* user_data;

    x49gp_t* x49gp;
    struct list_head list;
};

typedef enum { X49GP_REINIT_NONE = 0, X49GP_REINIT_REBOOT_ONLY, X49GP_REINIT_FLASH, X49GP_REINIT_FLASH_FULL } x49gp_reinit_t;

struct __x49gp_s__ {
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

    x49gp_timer_t* gtk_timer;
    x49gp_timer_t* lcd_timer;

    x49gp_arm_idle_t arm_idle;
    int arm_exit;

    x49gp_ui_t* ui;

    GKeyFile* state;
    const char* progname;
    const char* progpath;
};

extern void x49gp_set_idle( x49gp_t*, x49gp_arm_idle_t idle );

extern int x49gp_module_init( x49gp_t* x49gp, const char* name, int ( *init )( x49gp_module_t* ), int ( *exit )( x49gp_module_t* ),
                              int ( *reset )( x49gp_module_t*, x49gp_reset_t ), int ( *load )( x49gp_module_t*, GKeyFile* ),
                              int ( *save )( x49gp_module_t*, GKeyFile* ), void* user_data, x49gp_module_t** module );

extern int x49gp_module_register( x49gp_module_t* module );
extern int x49gp_module_unregister( x49gp_module_t* module );

extern int x49gp_module_get_filename( x49gp_module_t* module, GKeyFile*, const char*, char*, char**, char** );
extern int x49gp_module_set_filename( x49gp_module_t* module, GKeyFile*, const char*, const char* );
extern int x49gp_module_get_int( x49gp_module_t* module, GKeyFile*, const char*, int, int* );
extern int x49gp_module_set_int( x49gp_module_t* module, GKeyFile*, const char*, int );
extern int x49gp_module_get_uint( x49gp_module_t* module, GKeyFile*, const char*, unsigned int, unsigned int* );
extern int x49gp_module_set_uint( x49gp_module_t* module, GKeyFile*, const char*, unsigned int );
extern int x49gp_module_get_u32( x49gp_module_t* module, GKeyFile*, const char*, uint32_t, uint32_t* );
extern int x49gp_module_set_u32( x49gp_module_t* module, GKeyFile*, const char*, uint32_t );
extern int x49gp_module_get_u64( x49gp_module_t* module, GKeyFile*, const char*, uint64_t, uint64_t* );
extern int x49gp_module_set_u64( x49gp_module_t* module, GKeyFile*, const char*, uint64_t );
extern int x49gp_module_get_string( x49gp_module_t* module, GKeyFile*, const char*, char*, char** );
extern int x49gp_module_set_string( x49gp_module_t* module, GKeyFile*, const char*, const char* );
extern int x49gp_module_open_rodata( x49gp_module_t* module, const char* name, char** path );

extern void s3c2410_sdi_unmount( x49gp_t* x49gp );
extern int s3c2410_sdi_mount( x49gp_t* x49gp, char* filename );
extern int s3c2410_sdi_is_mounted( x49gp_t* x49gp );
extern int s3c2410_sdi_get_path( x49gp_t* x49gp, char** filename );

extern int x49gp_modules_init( x49gp_t* );
extern int x49gp_modules_exit( x49gp_t* );
extern int x49gp_modules_reset( x49gp_t*, x49gp_reset_t );
extern int x49gp_modules_load( x49gp_t* );
extern int x49gp_modules_save( x49gp_t* );

extern int x49gp_flash_init( x49gp_t* );
extern int x49gp_sram_init( x49gp_t* );

#endif /* !(_X49GP_H) */
