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
#include <errno.h>

#include <getopt.h>

#include <gtk/gtk.h>
#include <glib.h>

#include <memory.h>

#include "x49gp.h"
#include "ui.h"
#include "s3c2410.h"
#include "x49gp_timer.h"
#include "gdbstub.h"
#include "options.h"

extern struct options opt;

static x49gp_t* x49gp;

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
#ifdef DEBUG_X49GP_SYSCALL
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

void x49gp_set_idle( x49gp_t* x49gp, x49gp_arm_idle_t idle )
{
#ifdef DEBUG_X49GP_ARM_IDLE
    if ( idle != x49gp->arm_idle ) {
        printf( "%s: arm_idle %u, idle %u\n", __FUNCTION__, x49gp->arm_idle, idle );
    }
#endif

    x49gp->arm_idle = idle;

    if ( x49gp->arm_idle == X49GP_ARM_RUN )
        x49gp->env->halted = 0;
    else {
        x49gp->env->halted = 1;
        cpu_exit( x49gp->env );
    }
}

static void arm_sighnd( int sig )
{
    switch ( sig ) {
        case SIGUSR1:
            //		stop_simulator = 1;
            //		x49gp->arm->CallDebug ^= 1;
            break;
        default:
            fprintf( stderr, "%s: sig %u\n", __FUNCTION__, sig );
            break;
    }
}

void x49gp_gtk_timer( void* data )
{
#if GTK_MAJOR_VERSION == 4
    while ( g_main_context_pending( NULL ) )
        g_main_context_iteration( NULL, false );
#else
    while ( gtk_events_pending() )
        gtk_main_iteration_do( false );
#endif

    x49gp_mod_timer( x49gp->gtk_timer, x49gp_get_clock() + X49GP_GTK_REFRESH_INTERVAL );
}

void x49gp_lcd_timer( void* data )
{
    x49gp_t* x49gp = data;
    int64_t now, expires;

    gui_update_lcd( x49gp );
    gdk_display_flush( gdk_display_get_default() );

    now = x49gp_get_clock();
    expires = now + X49GP_LCD_REFRESH_INTERVAL;

    x49gp_mod_timer( x49gp->lcd_timer, expires );
}

void ui_sighnd( int sig )
{
    switch ( sig ) {
        case SIGINT:
        case SIGQUIT:
        case SIGTERM:
            x49gp->arm_exit = 1;
            cpu_exit( x49gp->env );
            break;
    }
}

int main( int argc, char** argv )
{
    char* progname = g_path_get_basename( argv[ 0 ] );
    char* progpath = g_path_get_dirname( argv[ 0 ] );

#if GTK_MAJOR_VERSION == 4
    gtk_init();
#else
    gtk_init( &argc, &argv );
#endif

    config_init( progname, argc, argv );

    x49gp = malloc( sizeof( x49gp_t ) );
    if ( NULL == x49gp ) {
        fprintf( stderr, "%s: %s:%u: Out of memory\n", progname, __FUNCTION__, __LINE__ );
        exit( EXIT_FAILURE );
    }
    memset( x49gp, 0, sizeof( x49gp_t ) );

#ifdef DEBUG_X49GP_MAIN
    fprintf( stderr, "_SC_PAGE_SIZE: %08lx\n", sysconf( _SC_PAGE_SIZE ) );

    printf( "%s:%u: x49gp: %p\n", __FUNCTION__, __LINE__, x49gp );
#endif

    INIT_LIST_HEAD( &x49gp->modules );

    x49gp->progname = progname;
    x49gp->progpath = progpath;
    x49gp->clk_tck = sysconf( _SC_CLK_TCK );
    x49gp->emulator_fclk = 75000000;
    x49gp->PCLK_ratio = 4;
    x49gp->PCLK = 75000000 / 4;

    // cpu_set_log(0xffffffff);
    cpu_exec_init_all( 0 );
    x49gp->env = cpu_init( "arm926" );
    __GLOBAL_env = x49gp->env;

    //	cpu_set_log(cpu_str_to_log_mask("all"));

    x49gp_timer_init( x49gp );

    x49gp->gtk_timer = x49gp_new_timer( X49GP_TIMER_REALTIME, x49gp_gtk_timer, x49gp );
    x49gp->lcd_timer = x49gp_new_timer( X49GP_TIMER_VIRTUAL, x49gp_lcd_timer, x49gp );

    gui_init( x49gp );

    x49gp_s3c2410_arm_init( x49gp );
    x49gp_flash_init( x49gp );
    x49gp_sram_init( x49gp );
    x49gp_s3c2410_init( x49gp );

    if ( x49gp_modules_init( x49gp ) )
        exit( EXIT_FAILURE );

    x49gp->basename = g_path_get_dirname( opt.state_filename );
    x49gp->debug_port = opt.debug_port;
    x49gp->startup_reinit = opt.reinit;
    x49gp->firmware = opt.firmware;

    int error = x49gp_modules_load( x49gp, opt.state_filename );
    if ( error || opt.reinit >= X49GP_REINIT_REBOOT_ONLY ) {
        if ( error && error != -EAGAIN )
            exit( EXIT_FAILURE );

        x49gp_modules_reset( x49gp, X49GP_RESET_POWER_ON );
    }
    // x49gp_modules_reset(x49gp, X49GP_RESET_POWER_ON);

    signal( SIGINT, ui_sighnd );
    signal( SIGTERM, ui_sighnd );
    signal( SIGQUIT, ui_sighnd );

    signal( SIGUSR1, arm_sighnd );

    x49gp_set_idle( x49gp, 0 );

    // stl_phys(0x08000a1c, 0x55555555);

    x49gp_mod_timer( x49gp->gtk_timer, x49gp_get_clock() );
    x49gp_mod_timer( x49gp->lcd_timer, x49gp_get_clock() );

    if ( opt.debug_port != 0 && opt.start_debugger ) {
        gdbserver_start( opt.debug_port );
        gdb_handlesig( x49gp->env, 0 );
    }

    x49gp_main_loop( x49gp );

    x49gp_modules_save( x49gp, opt.state_filename );
    x49gp_modules_exit( x49gp );

#if false
        printf("ClkTicks: %lu\n", ARMul_Time(x49gp->arm));
        printf("D TLB: hit0 %lu, hit1 %lu, search %lu (%lu), walk %lu\n",
                x49gp->mmu->dTLB.hit0, x49gp->mmu->dTLB.hit1,
                x49gp->mmu->dTLB.search, x49gp->mmu->dTLB.nsearch,
                x49gp->mmu->dTLB.walk);
        printf("I TLB: hit0 %lu, hit1 %lu, search %lu (%lu), walk %lu\n",
                x49gp->mmu->iTLB.hit0, x49gp->mmu->iTLB.hit1,
                x49gp->mmu->iTLB.search, x49gp->mmu->iTLB.nsearch,
                x49gp->mmu->iTLB.walk);
#endif
    return 0;
}
