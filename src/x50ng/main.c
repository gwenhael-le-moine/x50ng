#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>

#include <getopt.h>

#include <gtk/gtk.h>
#include <glib.h>

#include <memory.h>

#include "list.h"
#include "x50ng.h"
#include "ui.h"
#include "s3c2410.h"
#include "timer.h"
#include "gdbstub.h"
#include "options.h"

extern struct options opt;

static x50ng_t* x50ng;

/* LD TEMPO HACK */
CPUState* __GLOBAL_env;

int semihosting_enabled = 1;

uint8_t* phys_ram_base;
int phys_ram_size;
ram_addr_t ram_size = 0x80000; // LD ???

/* for qemu-git */
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

void qemu_init_vcpu( void* _env )
{
    CPUState* env = _env;

    env->nr_cores = 1;
    env->nr_threads = 1;
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
            printf( "%s: SWI LR %08x: syscall %u: args %08x %08x %08x %08x %08x %08x %08x\n", __FUNCTION__, env->regs[ 14 ], env->regs[ 0 ],
                    env->regs[ 1 ], env->regs[ 2 ], env->regs[ 3 ], env->regs[ 4 ], env->regs[ 5 ], env->regs[ 6 ], env->regs[ 7 ] );
#endif

            switch ( env->regs[ 0 ] ) {
                case 305: /* Beep */
                    printf( "%s: BEEP: frequency %u, time %u, override %u\n", __FUNCTION__, env->regs[ 1 ], env->regs[ 2 ],
                            env->regs[ 3 ] );

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
        printf( "%s: arm_idle %u, idle %u\n", __FUNCTION__, x50ng->arm_idle, idle );
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

/**********/
/* timers */
/**********/
void x50ng_gtk_timer( void* data )
{
    while ( g_main_context_pending( NULL ) )
        g_main_context_iteration( NULL, false );

    x50ng_mod_timer( x50ng->gtk_timer, x50ng_get_clock() + X50NG_GTK_REFRESH_INTERVAL );
}

void x50ng_lcd_timer( void* data )
{
    x50ng_t* x50ng = data;
    int64_t now, expires;

    gui_update_lcd( x50ng );
    gdk_display_flush( gdk_display_get_default() );

    now = x50ng_get_clock();
    expires = now + X50NG_LCD_REFRESH_INTERVAL;

    x50ng_mod_timer( x50ng->lcd_timer, expires );
}

/*******************/
/* signal handlers */
/*******************/
void ui_sighnd( int sig )
{
    switch ( sig ) {
        case SIGINT:
        case SIGQUIT:
        case SIGTERM:
            x50ng->arm_exit = 1;
            cpu_exit( x50ng->env );
            break;
    }
}

static void arm_sighnd( int sig )
{
    switch ( sig ) {
        case SIGUSR1:
            //		stop_simulator = 1;
            //		x50ng->arm->CallDebug ^= 1;
            break;
        default:
            fprintf( stderr, "%s: sig %u\n", __FUNCTION__, sig );
            break;
    }
}

int main( int argc, char** argv )
{
    char* progname = g_path_get_basename( argv[ 0 ] );
    char* progpath = g_path_get_dirname( argv[ 0 ] );

    config_init( progname, argc, argv );

    x50ng = malloc( sizeof( x50ng_t ) );
    if ( NULL == x50ng ) {
        fprintf( stderr, "%s: %s:%u: Out of memory\n", progname, __FUNCTION__, __LINE__ );
        exit( EXIT_FAILURE );
    }
    memset( x50ng, 0, sizeof( x50ng_t ) );

#ifdef DEBUG_X50NG_MAIN
    fprintf( stderr, "_SC_PAGE_SIZE: %08lx\n", sysconf( _SC_PAGE_SIZE ) );

    printf( "%s:%u: x50ng: %p\n", __FUNCTION__, __LINE__, x50ng );
#endif

    INIT_LIST_HEAD( &x50ng->modules );

    x50ng->progname = progname;
    x50ng->progpath = progpath;
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

    x50ng->gtk_timer = x50ng_new_timer( X50NG_TIMER_REALTIME, x50ng_gtk_timer, x50ng );
    x50ng->lcd_timer = x50ng_new_timer( X50NG_TIMER_VIRTUAL, x50ng_lcd_timer, x50ng );

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

    signal( SIGINT, ui_sighnd );
    signal( SIGTERM, ui_sighnd );
    signal( SIGQUIT, ui_sighnd );

    signal( SIGUSR1, arm_sighnd );

    x50ng_set_idle( x50ng, 0 );

    // stl_phys(0x08000a1c, 0x55555555);

    x50ng_mod_timer( x50ng->gtk_timer, x50ng_get_clock() );
    x50ng_mod_timer( x50ng->lcd_timer, x50ng_get_clock() );

    if ( opt.debug_port != 0 && opt.start_debugger ) {
        gdbserver_start( opt.debug_port );
        gdb_handlesig( x50ng->env, 0 );
    }

    gui_init( x50ng ); /* return gtk_application here ? */
    // GtkApplication* app = gtk_application_new ("org.gtk.example", G_APPLICATION_DEFAULT_FLAGS);
    // g_signal_connect (app, "activate", G_CALLBACK (gui_init), x50ng);

    /* run gtk_application here ? */
    // int status = g_application_run (G_APPLICATION (app), 0, NULL);

    x50ng_main_loop( x50ng );
    // g_object_unref (app);

    x50ng_modules_save( x50ng );
    if ( !opt.haz_config_file )
        save_config();

    x50ng_modules_exit( x50ng );

    return 0;
}
