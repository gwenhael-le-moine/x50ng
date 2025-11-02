#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <gtk/gtk.h>

#include "s3c2410/s3c2410.h"

#include "ui/ui.h"

#include "list.h"
#include "ui/inner.h"
#include "x50ng.h"
#include "gdbstub.h"
#include "options.h"
#include "sram.h"
#include "flash.h"
#include "module.h"
#include "emulator.h"

x50ng_t* x50ng;

x50ng_key_t x50ng_keys[ NB_HP50g_KEYS ] = {
    {.column = 5, .row = 1, .eint = -1 /* 1 */, .pressed = false},
    {.column = 5, .row = 2, .eint = -1 /* 2 */, .pressed = false},
    {.column = 5, .row = 3, .eint = -1 /* 3 */, .pressed = false},
    {.column = 5, .row = 4, .eint = -1 /* 4 */, .pressed = false},
    {.column = 5, .row = 5, .eint = -1 /* 5 */, .pressed = false},
    {.column = 5, .row = 6, .eint = -1 /* 6 */, .pressed = false},

    {.column = 5, .row = 7, .eint = -1 /* 7 */, .pressed = false},
    {.column = 6, .row = 5, .eint = -1 /* 5 */, .pressed = false},
    {.column = 6, .row = 6, .eint = -1 /* 6 */, .pressed = false},
    {.column = 6, .row = 1, .eint = -1 /* 1 */, .pressed = false},
    {.column = 6, .row = 7, .eint = -1 /* 7 */, .pressed = false},
    {.column = 7, .row = 1, .eint = -1 /* 1 */, .pressed = false},
    {.column = 7, .row = 2, .eint = -1 /* 2 */, .pressed = false},

    {.column = 6, .row = 2, .eint = -1 /* 2 */, .pressed = false},
    {.column = 6, .row = 3, .eint = -1 /* 3 */, .pressed = false},
    {.column = 6, .row = 4, .eint = -1 /* 4 */, .pressed = false},

    {.column = 4, .row = 1, .eint = -1 /* 1 */, .pressed = false},
    {.column = 3, .row = 1, .eint = -1 /* 1 */, .pressed = false},
    {.column = 2, .row = 1, .eint = -1 /* 1 */, .pressed = false},
    {.column = 1, .row = 1, .eint = -1 /* 1 */, .pressed = false},
    {.column = 0, .row = 1, .eint = -1 /* 1 */, .pressed = false},

    {.column = 4, .row = 2, .eint = -1 /* 2 */, .pressed = false},
    {.column = 3, .row = 2, .eint = -1 /* 2 */, .pressed = false},
    {.column = 2, .row = 2, .eint = -1 /* 2 */, .pressed = false},
    {.column = 1, .row = 2, .eint = -1 /* 2 */, .pressed = false},
    {.column = 0, .row = 2, .eint = -1 /* 2 */, .pressed = false},

    {.column = 0, .row = 7, .eint = -1 /* 7 */, .pressed = false},
    {.column = 4, .row = 3, .eint = -1 /* 3 */, .pressed = false},
    {.column = 3, .row = 3, .eint = -1 /* 3 */, .pressed = false},
    {.column = 2, .row = 3, .eint = -1 /* 3 */, .pressed = false},
    {.column = 1, .row = 3, .eint = -1 /* 3 */, .pressed = false},

    {.column = 0, .row = 0, .eint = 4,          .pressed = false}, /* HP50g_KEY_ALPHA */
    {.column = 3, .row = 4, .eint = -1 /* 4 */, .pressed = false},
    {.column = 2, .row = 4, .eint = -1 /* 4 */, .pressed = false},
    {.column = 1, .row = 4, .eint = -1 /* 4 */, .pressed = false},
    {.column = 0, .row = 3, .eint = -1 /* 3 */, .pressed = false},

    {.column = 0, .row = 0, .eint = 5,          .pressed = false}, /* HP50g_KEY_SHIFT_LEFT */
    {.column = 3, .row = 5, .eint = -1 /* 5 */, .pressed = false},
    {.column = 2, .row = 5, .eint = -1 /* 5 */, .pressed = false},
    {.column = 1, .row = 5, .eint = -1 /* 5 */, .pressed = false},
    {.column = 0, .row = 4, .eint = -1 /* 4 */, .pressed = false},

    {.column = 0, .row = 0, .eint = 6,          .pressed = false}, /* HP50g_KEY_SHIFT_RIGHT */
    {.column = 3, .row = 6, .eint = -1 /* 6 */, .pressed = false},
    {.column = 2, .row = 6, .eint = -1 /* 6 */, .pressed = false},
    {.column = 1, .row = 6, .eint = -1 /* 6 */, .pressed = false},
    {.column = 0, .row = 5, .eint = -1 /* 5 */, .pressed = false},

    {.column = 0, .row = 0, .eint = 0,          .pressed = false}, /* HP50g_KEY_ON */
    {.column = 3, .row = 7, .eint = -1 /* 7 */, .pressed = false},
    {.column = 2, .row = 7, .eint = -1 /* 7 */, .pressed = false},
    {.column = 1, .row = 7, .eint = -1 /* 7 */, .pressed = false},
    {.column = 0, .row = 6, .eint = -1 /* 6 */, .pressed = false},
};
#define KEYBOARD x50ng_keys

int x50ng_annunciators_index[ 6 ] = { 1, 2, 3, 4, 5, 0 };

/* LD TEMPO HACK */
CPUState* __GLOBAL_env;

int semihosting_enabled = 1;

uint8_t* phys_ram_base;
int phys_ram_size;
ram_addr_t ram_size = 0x80000; // LD ???

/* for qemu */
int singlestep;

#if !( defined( __APPLE__ ) || defined( _POSIX_C_SOURCE ) && !defined( __sun__ ) )
static void* oom_check( void* ptr )
{
    if ( ptr == NULL )
        abort();

    return ptr;
}
#endif

void* qemu_memalign( size_t alignment, size_t size )
{
#if defined( __APPLE__ ) || defined( _POSIX_C_SOURCE ) && !defined( __sun__ )
    int ret;
    void* ptr;
    ret = posix_memalign( &ptr, alignment, size );
    if ( ret != 0 )
        abort();
    return ptr;
#elif defined( CONFIG_BSD )
    return oom_check( valloc( size ) );
#else
    return oom_check( memalign( alignment, size ) );
#endif
}

void qemu_init_vcpu( void* env )
{
    ( ( CPUState* )env )->nr_cores = 1;
    ( ( CPUState* )env )->nr_threads = 1;
}

int qemu_cpu_self( void* env ) { return 1; }

void qemu_cpu_kick( void* env ) {}

void armv7m_nvic_set_pending( void* opaque, int irq ) { abort(); }
int armv7m_nvic_acknowledge_irq( void* opaque ) { abort(); }
void armv7m_nvic_complete_irq( void* opaque, int irq ) { abort(); }

void* qemu_malloc( size_t size ) { return malloc( size ); }

void* qemu_mallocz( size_t size )
{
    void* ptr;

    ptr = qemu_malloc( size );
    if ( NULL == ptr )
        return NULL;
    memset( ptr, 0, size );
    return ptr;
}

void qemu_free( void* ptr ) { free( ptr ); }

void* qemu_vmalloc( size_t size )
{
#if defined( __linux__ )
    void* mem;
    if ( 0 == posix_memalign( &mem, sysconf( _SC_PAGE_SIZE ), size ) )
        return mem;
    return NULL;
#else
    return valloc( size );
#endif
}

#define SWI_Breakpoint 0x180000

uint32_t do_arm_semihosting( CPUState* env )
{
    uint32_t number;
    if ( env->thumb )
        number = lduw_code( env->regs[ 15 ] - 2 ) & 0xff;
    else
        number = ldl_code( env->regs[ 15 ] - 4 ) & 0xffffff;

    switch ( number ) {
        case SWI_Breakpoint:
            break;

        case 0:
#ifdef DEBUG_X50NG_SYSCALL
            printf( "%s: SWI LR %08x: syscall %u: args %08x %08x %08x %08x %08x %08x %08x\n", __func__, env->regs[ 14 ], env->regs[ 0 ],
                    env->regs[ 1 ], env->regs[ 2 ], env->regs[ 3 ], env->regs[ 4 ], env->regs[ 5 ], env->regs[ 6 ], env->regs[ 7 ] );
#endif

            switch ( env->regs[ 0 ] ) {
                case 305: /* Beep */
                    printf( "%s: BEEP: frequency %u, time %u, override %u\n", __func__, env->regs[ 1 ], env->regs[ 2 ], env->regs[ 3 ] );

                    gdk_display_beep( gdk_display_get_default() );
                    env->regs[ 0 ] = 0;
                    return 1;

                case 28: /* CheckBeepEnd */
                    env->regs[ 0 ] = 0;
                    return 1;

                case 29: /* StopBeep */
                    env->regs[ 0 ] = 0;
                    return 1;

                default:
                    break;
            }
            break;

        default:
            break;
    }

    return 0;
}

void x50ng_set_idle( x50ng_t* x50ng, x50ng_arm_idle_t idle )
{
#ifdef DEBUG_X50NG_ARM_IDLE
    if ( idle != x50ng->arm_idle ) {
        printf( "%s: arm_idle %u, idle %u\n", __func__, x50ng->arm_idle, idle );
    }
#endif

    x50ng->arm_idle = idle;

    if ( x50ng->arm_idle == X50NG_ARM_RUN )
        x50ng->env->halted = 0;
    else {
        x50ng->env->halted = 1;
        cpu_exit( x50ng->env );
    }
}

void emulator_init( void )
{
    x50ng = malloc( sizeof( x50ng_t ) );
    if ( NULL == x50ng ) {
        fprintf( stderr, "%s: %s:%u: Out of memory\n", opt.progname, __func__, __LINE__ );
        exit( EXIT_FAILURE );
    }
    memset( x50ng, 0, sizeof( x50ng_t ) );

#ifdef DEBUG_X50NG_MAIN
    fprintf( stderr, "_SC_PAGE_SIZE: %08lx\n", sysconf( _SC_PAGE_SIZE ) );

    printf( "%s:%u: x50ng: %p\n", __func__, __LINE__, x50ng );
#endif

    INIT_LIST_HEAD( &x50ng->modules );

    x50ng->progname = opt.progname;
    x50ng->progpath = opt.progpath;
    x50ng->clk_tck = sysconf( _SC_CLK_TCK );
    x50ng->emulator_fclk = 75000000;
    x50ng->PCLK_ratio = 4;
    x50ng->PCLK = 75000000 / 4;

    // cpu_set_log(0xffffffff);
    cpu_exec_init_all( 0 );
    x50ng->env = cpu_init( "arm926" );
    __GLOBAL_env = x50ng->env;

    //	cpu_set_log(cpu_str_to_log_mask("all"));

    x50ng_timer_init( x50ng );

    x50ng->timer_ui_input = x50ng_new_timer( X50NG_TIMER_REALTIME, ui_handle_pending_inputs, x50ng );
    x50ng->timer_ui_output = x50ng_new_timer( X50NG_TIMER_VIRTUAL, ui_refresh_output, x50ng );

    x50ng_s3c2410_arm_init( x50ng );
    x50ng_flash_init( x50ng );
    x50ng_sram_init( x50ng );
    x50ng_s3c2410_init( x50ng );

    if ( x50ng_modules_init( x50ng ) )
        exit( EXIT_FAILURE );

    int error = x50ng_modules_load( x50ng );
    if ( error || opt.reinit >= X50NG_REINIT_REBOOT_ONLY ) {
        if ( error && error != -EAGAIN )
            exit( EXIT_FAILURE );

        x50ng_modules_reset( x50ng, X50NG_RESET_POWER_ON );
    }

    x50ng_set_idle( x50ng, 0 );

    // stl_phys(0x08000a1c, 0x55555555);

    x50ng_mod_timer( x50ng->timer_ui_input, x50ng_get_clock() );
    x50ng_mod_timer( x50ng->timer_ui_output, x50ng_get_clock() );

    if ( opt.debug_port != 0 && opt.start_debugger ) {
        gdbserver_start( opt.debug_port );
        gdb_handlesig( x50ng->env, 0 );
    }

    if ( opt.sd_dir != NULL ) {
        if ( opt.verbose )
            fprintf( stderr, "> mounting --sd-dir %s\n", opt.sd_dir );
        s3c2410_sdi_mount( x50ng, strdup( opt.sd_dir ) );
    }
}

void emulator_exit( void )
{
    x50ng_modules_save( x50ng );

    if ( !opt.haz_config_file )
        save_config();

    x50ng_modules_exit( x50ng );
}

static void x50ng_set_key_state( const x50ng_key_t key, bool state )
{
    if ( key.eint >= 0 /* && key.row == 0 && key.column == 0 */ )
        s3c2410_io_port_f_set_bit( x50ng, key.eint, state );
    else
        s3c2410_io_port_g_update( x50ng, key.column, key.row, state );
}

void press_key( int hpkey )
{
    x50ng_set_key_state( KEYBOARD[ hpkey ], true );
    KEYBOARD[ hpkey ].pressed = true;
}

void release_key( int hpkey )
{
    x50ng_set_key_state( KEYBOARD[ hpkey ], false );
    KEYBOARD[ hpkey ].pressed = false;
}

bool is_key_pressed( int hpkey )
{
    if ( hpkey < 0 || hpkey > NB_HP50g_KEYS )
        return false;

    return KEYBOARD[ hpkey ].pressed;
}

bool is_display_on( void )
{
    s3c2410_lcd_t* lcd = x50ng->s3c2410_lcd;

    return ( lcd->lcdcon1 & 1 );
}

unsigned char get_annunciators( void )
{
    s3c2410_lcd_t* lcd = x50ng->s3c2410_lcd;

    char annunciators = 0;

    for ( int i = 0; i < NB_ANNUNCIATORS; ++i )
        if ( x50ng_s3c2410_get_pixel_color( lcd, LCD_WIDTH, x50ng_annunciators_index[ i ] ) > 0 )
            annunciators |= 0x01 << i;

    return annunciators;
}

int get_contrast( void ) { return 19; }

void get_lcd_buffer( int* target )
{
    s3c2410_lcd_t* lcd = x50ng->s3c2410_lcd;

    for ( int y = 0; y < LCD_HEIGHT; ++y )
        for ( int x = 0; x < LCD_WIDTH; ++x )
            target[ ( y * LCD_WIDTH ) + x ] = x50ng_s3c2410_get_pixel_color( lcd, x, y );
}
