/*
 * gdb server stub
 *
 * Copyright (c) 2003-2005 Fabrice Bellard
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */
#include "qemu-git/qemu-common.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include "gdbstub.h"

#define MAX_PACKET_LENGTH 4096

enum { GDB_SIGNAL_0 = 0, GDB_SIGNAL_INT = 2, GDB_SIGNAL_TRAP = 5, GDB_SIGNAL_UNKNOWN = 143 };

/* In system mode we only need SIGINT and SIGTRAP; other signals
   are not yet supported.  */

enum { TARGET_SIGINT = 2, TARGET_SIGTRAP = 5 };

static int gdb_signal_table[] = { -1, -1, TARGET_SIGINT, -1, -1, TARGET_SIGTRAP };

static int target_signal_to_gdb( int sig )
{
    int i;
    for ( i = 0; i < ARRAY_SIZE( gdb_signal_table ); i++ )
        if ( gdb_signal_table[ i ] == sig )
            return i;
    return GDB_SIGNAL_UNKNOWN;
}

static int gdb_signal_to_target( int sig )
{
    if ( sig < ARRAY_SIZE( gdb_signal_table ) )
        return gdb_signal_table[ sig ];
    else
        return -1;
}

// #define DEBUG_GDB

typedef struct GDBRegisterState {
    int base_reg;
    int num_regs;
    gdb_reg_cb get_reg;
    gdb_reg_cb set_reg;
    const char* xml;
    struct GDBRegisterState* next;
} GDBRegisterState;

enum RSState {
    RS_INACTIVE,
    RS_IDLE,
    RS_GETLINE,
    RS_CHKSUM1,
    RS_CHKSUM2,
    RS_SYSCALL,
};
typedef struct GDBState {
    CPUState* c_cpu;     /* current CPU for step/continue ops */
    CPUState* g_cpu;     /* current CPU for other ops */
    CPUState* query_cpu; /* for q{f|s}ThreadInfo */
    enum RSState state;  /* parsing state */
    char line_buf[ MAX_PACKET_LENGTH ];
    int line_buf_index;
    int line_csum;
    uint8_t last_packet[ MAX_PACKET_LENGTH + 4 ];
    int last_packet_len;
    int signal;
    int fd;
    int running_state;
} GDBState;

/* By default use no IRQs and no timers while single stepping so as to
 * make single stepping like an ICE HW step.
 */
static int sstep_flags = SSTEP_ENABLE | SSTEP_NOIRQ | SSTEP_NOTIMER;

static GDBState* gdbserver_state;

/* This is an ugly hack to cope with both new and old gdb.
   If gdb sends qXfer:features:read then assume we're talking to a newish
   gdb that understands target descriptions.  */
static int gdb_has_xml;

/* XXX: This is not thread safe.  Do we care?  */
static int gdbserver_fd = -1;

static int get_char( GDBState* s )
{
    uint8_t ch;
    int ret;

    for ( ;; ) {
        ret = recv( s->fd, &ch, 1, 0 );
        if ( ret < 0 ) {
            if ( errno == ECONNRESET )
                s->fd = -1;
            if ( errno != EINTR && errno != EAGAIN )
                return -1;
        } else if ( ret == 0 ) {
            close( s->fd );
            s->fd = -1;
            return -1;
        } else {
            break;
        }
    }
    return ch;
}

static gdb_syscall_complete_cb gdb_current_syscall_cb;

static enum {
    GDB_SYS_UNKNOWN,
    GDB_SYS_ENABLED,
    GDB_SYS_DISABLED,
} gdb_syscall_mode;

/* If gdb is connected when the first semihosting syscall occurs then use
   remote gdb syscalls.  Otherwise use native file IO.  */
int use_gdb_syscalls( void )
{
    if ( gdb_syscall_mode == GDB_SYS_UNKNOWN ) {
        gdb_syscall_mode = ( gdbserver_state ? GDB_SYS_ENABLED : GDB_SYS_DISABLED );
    }
    return gdb_syscall_mode == GDB_SYS_ENABLED;
}

/* Resume execution.  */
static inline void gdb_continue( GDBState* s ) { s->running_state = 1; }

static void put_buffer( GDBState* s, const uint8_t* buf, int len )
{
    int ret;

    while ( len > 0 ) {
        ret = send( s->fd, buf, len, 0 );
        if ( ret < 0 ) {
            if ( errno != EINTR && errno != EAGAIN )
                return;
        } else {
            buf += ret;
            len -= ret;
        }
    }
}

static inline int fromhex( int v )
{
    if ( v >= '0' && v <= '9' )
        return v - '0';
    else if ( v >= 'A' && v <= 'F' )
        return v - 'A' + 10;
    else if ( v >= 'a' && v <= 'f' )
        return v - 'a' + 10;
    else
        return 0;
}

static inline int tohex( int v )
{
    if ( v < 10 )
        return v + '0';
    else
        return v - 10 + 'a';
}

static void memtohex( char* buf, const uint8_t* mem, int len )
{
    int i, c;
    char* q;
    q = buf;
    for ( i = 0; i < len; i++ ) {
        c = mem[ i ];
        *q++ = tohex( c >> 4 );
        *q++ = tohex( c & 0xf );
    }
    *q = '\0';
}

static void hextomem( uint8_t* mem, const char* buf, int len )
{
    int i;

    for ( i = 0; i < len; i++ ) {
        mem[ i ] = ( fromhex( buf[ 0 ] ) << 4 ) | fromhex( buf[ 1 ] );
        buf += 2;
    }
}

/* return -1 if error, 0 if OK */
static int put_packet_binary( GDBState* s, const char* buf, int len )
{
    int csum, i;
    uint8_t* p;

    for ( ;; ) {
        p = s->last_packet;
        *( p++ ) = '$';
        memcpy( p, buf, len );
        p += len;
        csum = 0;
        for ( i = 0; i < len; i++ ) {
            csum += buf[ i ];
        }
        *( p++ ) = '#';
        *( p++ ) = tohex( ( csum >> 4 ) & 0xf );
        *( p++ ) = tohex( ( csum ) & 0xf );

        s->last_packet_len = p - s->last_packet;
        put_buffer( s, ( uint8_t* )s->last_packet, s->last_packet_len );

        i = get_char( s );
        if ( i < 0 )
            return -1;
        if ( i == '+' )
            break;
    }
    return 0;
}

/* return -1 if error, 0 if OK */
static int put_packet( GDBState* s, const char* buf )
{
#ifdef DEBUG_GDB
    printf( "reply='%s'\n", buf );
#endif

    return put_packet_binary( s, buf, strlen( buf ) );
}

/* The GDB remote protocol transfers values in target byte order.  This means
   we can use the raw memory access routines to access the value buffer.
   Conveniently, these also handle the case where the buffer is mis-aligned.
 */
#define GET_REG8( val )                                                                                                                    \
    do {                                                                                                                                   \
        stb_p( mem_buf, val );                                                                                                             \
        return 1;                                                                                                                          \
    } while ( 0 )
#define GET_REG16( val )                                                                                                                   \
    do {                                                                                                                                   \
        stw_p( mem_buf, val );                                                                                                             \
        return 2;                                                                                                                          \
    } while ( 0 )
#define GET_REG32( val )                                                                                                                   \
    do {                                                                                                                                   \
        stl_p( mem_buf, val );                                                                                                             \
        return 4;                                                                                                                          \
    } while ( 0 )
#define GET_REG64( val )                                                                                                                   \
    do {                                                                                                                                   \
        stq_p( mem_buf, val );                                                                                                             \
        return 8;                                                                                                                          \
    } while ( 0 )

#if TARGET_LONG_BITS == 64
#  define GET_REGL( val ) GET_REG64( val )
#  define ldtul_p( addr ) ldq_p( addr )
#else
#  define GET_REGL( val ) GET_REG32( val )
#  define ldtul_p( addr ) ldl_p( addr )
#endif

/* Old gdb always expect FPA registers.  Newer (xml-aware) gdb only expect
   whatever the target description contains.  Due to a historical mishap
   the FPA registers appear in between core integer regs and the CPSR.
   We hack round this by giving the FPA regs zero size when talking to a
   newer gdb.  */
#define NUM_CORE_REGS 26
// #define GDB_CORE_XML "arm-core.xml"

static int cpu_gdb_read_register( CPUState* env, uint8_t* mem_buf, int n )
{
    if ( n < 16 ) {
        /* Core integer register.  */
        GET_REG32( env->regs[ n ] );
    }
    if ( n < 24 ) {
        /* FPA registers.  */
        if ( gdb_has_xml )
            return 0;
        memset( mem_buf, 0, 12 );
        return 12;
    }
    switch ( n ) {
        case 24:
            /* FPA status register.  */
            if ( gdb_has_xml )
                return 0;
            GET_REG32( 0 );
        case 25:
            /* CPSR */
            GET_REG32( cpsr_read( env ) );
    }
    /* Unknown register.  */
    return 0;
}

static int cpu_gdb_write_register( CPUState* env, uint8_t* mem_buf, int n )
{
    uint32_t tmp;

    tmp = ldl_p( mem_buf );

    /* Mask out low bit of PC to workaround gdb bugs.  This will probably
       cause problems if we ever implement the Jazelle DBX extensions.  */
    if ( n == 15 )
        tmp &= ~1;

    if ( n < 16 ) {
        /* Core integer register.  */
        env->regs[ n ] = tmp;
        return 4;
    }
    if ( n < 24 ) { /* 16-23 */
        /* FPA registers (ignored).  */
        if ( gdb_has_xml )
            return 0;
        return 12;
    }
    switch ( n ) {
        case 24:
            /* FPA status register (ignored).  */
            if ( gdb_has_xml )
                return 0;
            return 4;
        case 25:
            /* CPSR */
            cpsr_write( env, tmp, 0xffffffff );
            return 4;
    }
    /* Unknown register.  */
    return 0;
}

static int num_g_regs = NUM_CORE_REGS;

#ifdef GDB_CORE_XML
/* Encode data using the encoding for 'x' packets.  */
static int memtox( char* buf, const char* mem, int len )
{
    char* p = buf;
    char c;

    while ( len-- ) {
        c = *( mem++ );
        switch ( c ) {
            case '#':
            case '$':
            case '*':
            case '}':
                *( p++ ) = '}';
                *( p++ ) = c ^ 0x20;
                break;
            default:
                *( p++ ) = c;
                break;
        }
    }
    return p - buf;
}

static const char* get_feature_xml( const char* p, const char** newp )
{
    extern const char* const xml_builtin[][ 2 ];
    size_t len;
    int i;
    const char* name;
    static char target_xml[ 1024 ];

    len = 0;
    while ( p[ len ] && p[ len ] != ':' )
        len++;
    *newp = p + len;

    name = NULL;
    if ( strncmp( p, "target.xml", len ) == 0 ) {
        /* Generate the XML description for this CPU.  */
        if ( !target_xml[ 0 ] ) {
            GDBRegisterState* r;

            snprintf( target_xml, sizeof( target_xml ),
                      "<?xml version=\"1.0\"?>"
                      "<!DOCTYPE target SYSTEM \"gdb-target.dtd\">"
                      "<target>"
                      "<xi:include href=\"%s\"/>",
                      GDB_CORE_XML );

            for ( r = first_cpu->gdb_regs; r; r = r->next ) {
                pstrcat( target_xml, sizeof( target_xml ), "<xi:include href=\"" );
                pstrcat( target_xml, sizeof( target_xml ), r->xml );
                pstrcat( target_xml, sizeof( target_xml ), "\"/>" );
            }
            pstrcat( target_xml, sizeof( target_xml ), "</target>" );
        }
        return target_xml;
    }
    for ( i = 0;; i++ ) {
        name = xml_builtin[ i ][ 0 ];
        if ( !name || ( strncmp( name, p, len ) == 0 && strlen( name ) == len ) )
            break;
    }
    return name ? xml_builtin[ i ][ 1 ] : NULL;
}
#endif

static int gdb_read_register( CPUState* env, uint8_t* mem_buf, int reg )
{
    GDBRegisterState* r;

    if ( reg < NUM_CORE_REGS )
        return cpu_gdb_read_register( env, mem_buf, reg );

    for ( r = env->gdb_regs; r; r = r->next ) {
        if ( r->base_reg <= reg && reg < r->base_reg + r->num_regs ) {
            return r->get_reg( env, mem_buf, reg - r->base_reg );
        }
    }
    return 0;
}

static int gdb_write_register( CPUState* env, uint8_t* mem_buf, int reg )
{
    GDBRegisterState* r;

    if ( reg < NUM_CORE_REGS )
        return cpu_gdb_write_register( env, mem_buf, reg );

    for ( r = env->gdb_regs; r; r = r->next ) {
        if ( r->base_reg <= reg && reg < r->base_reg + r->num_regs ) {
            return r->set_reg( env, mem_buf, reg - r->base_reg );
        }
    }
    return 0;
}

/* Register a supplemental set of CPU registers.  If g_pos is nonzero it
   specifies the first register number and these registers are included in
   a standard "g" packet.  Direction is relative to gdb, i.e. get_reg is
   gdb reading a CPU register, and set_reg is gdb modifying a CPU register.
 */

void gdb_register_coprocessor( CPUState* env, gdb_reg_cb get_reg, gdb_reg_cb set_reg, int num_regs, const char* xml, int g_pos )
{
    GDBRegisterState* s;
    GDBRegisterState** p;
    static int last_reg = NUM_CORE_REGS;

    s = ( GDBRegisterState* )qemu_mallocz( sizeof( GDBRegisterState ) );
    s->base_reg = last_reg;
    s->num_regs = num_regs;
    s->get_reg = get_reg;
    s->set_reg = set_reg;
    s->xml = xml;
    p = &env->gdb_regs;
    while ( *p ) {
        /* Check for duplicates.  */
        if ( strcmp( ( *p )->xml, xml ) == 0 )
            return;
        p = &( *p )->next;
    }
    /* Add to end of list.  */
    last_reg += num_regs;
    *p = s;
    if ( g_pos ) {
        if ( g_pos != s->base_reg ) {
            fprintf( stderr,
                     "Error: Bad gdb register numbering for '%s'\n"
                     "Expected %d got %d\n",
                     xml, g_pos, s->base_reg );
        } else {
            num_g_regs = last_reg;
        }
    }
}

#ifndef CONFIG_USER_ONLY
static const int xlat_gdb_type[] = {
    [GDB_WATCHPOINT_WRITE] = BP_GDB | BP_MEM_WRITE,
    [GDB_WATCHPOINT_READ] = BP_GDB | BP_MEM_READ,
    [GDB_WATCHPOINT_ACCESS] = BP_GDB | BP_MEM_ACCESS,
};
#endif

static int gdb_breakpoint_insert( target_ulong addr, target_ulong len, int type )
{
    CPUState* env;
    int err = 0;

    switch ( type ) {
        case GDB_BREAKPOINT_SW:
        case GDB_BREAKPOINT_HW:
            for ( env = first_cpu; env != NULL; env = env->next_cpu ) {
                err = cpu_breakpoint_insert( env, addr, BP_GDB, NULL );
                if ( err )
                    break;
            }
            return err;
#ifndef CONFIG_USER_ONLY
        case GDB_WATCHPOINT_WRITE:
        case GDB_WATCHPOINT_READ:
        case GDB_WATCHPOINT_ACCESS:
            for ( env = first_cpu; env != NULL; env = env->next_cpu ) {
                err = cpu_watchpoint_insert( env, addr, len, xlat_gdb_type[ type ], NULL );
                if ( err )
                    break;
            }
            return err;
#endif
        default:
            return -ENOSYS;
    }
}

static int gdb_breakpoint_remove( target_ulong addr, target_ulong len, int type )
{
    CPUState* env;
    int err = 0;

    switch ( type ) {
        case GDB_BREAKPOINT_SW:
        case GDB_BREAKPOINT_HW:
            for ( env = first_cpu; env != NULL; env = env->next_cpu ) {
                err = cpu_breakpoint_remove( env, addr, BP_GDB );
                if ( err )
                    break;
            }
            return err;
#ifndef CONFIG_USER_ONLY
        case GDB_WATCHPOINT_WRITE:
        case GDB_WATCHPOINT_READ:
        case GDB_WATCHPOINT_ACCESS:
            for ( env = first_cpu; env != NULL; env = env->next_cpu ) {
                err = cpu_watchpoint_remove( env, addr, len, xlat_gdb_type[ type ] );
                if ( err )
                    break;
            }
            return err;
#endif
        default:
            return -ENOSYS;
    }
}

static void gdb_breakpoint_remove_all( void )
{
    CPUState* env;

    for ( env = first_cpu; env != NULL; env = env->next_cpu ) {
        cpu_breakpoint_remove_all( env, BP_GDB );
#ifndef CONFIG_USER_ONLY
        cpu_watchpoint_remove_all( env, BP_GDB );
#endif
    }
}

static void gdb_set_cpu_pc( GDBState* s, target_ulong pc ) { s->c_cpu->regs[ 15 ] = pc; }

static inline int gdb_id( CPUState* env )
{
#if defined( CONFIG_USER_ONLY ) && defined( CONFIG_USE_NPTL )
    return env->host_tid;
#else
    return env->cpu_index + 1;
#endif
}

static CPUState* find_cpu( uint32_t thread_id )
{
    CPUState* env;

    for ( env = first_cpu; env != NULL; env = env->next_cpu ) {
        if ( gdb_id( env ) == thread_id ) {
            return env;
        }
    }

    return NULL;
}

static int gdb_handle_packet( GDBState* s, const char* line_buf )
{
    CPUState* env;
    const char* p;
    uint32_t thread;
    int ch, reg_size, type, res;
    char buf[ MAX_PACKET_LENGTH ];
    uint8_t mem_buf[ MAX_PACKET_LENGTH ];
    uint8_t* registers;
    target_ulong addr, len;

#ifdef DEBUG_GDB
    printf( "command='%s'\n", line_buf );
#endif
    p = line_buf;
    ch = *p++;
    switch ( ch ) {
        case '?':
            /* TODO: Make this return the correct value for user-mode.  */
            snprintf( buf, sizeof( buf ), "T%02xthread:%02x;", GDB_SIGNAL_TRAP, gdb_id( s->c_cpu ) );
            put_packet( s, buf );
            /* Remove all the breakpoints when this query is issued,
             * because gdb is doing and initial connect and the state
             * should be cleaned up.
             */
            gdb_breakpoint_remove_all();
            break;
        case 'c':
            if ( *p != '\0' ) {
                addr = strtoull( p, ( char** )&p, 16 );
                gdb_set_cpu_pc( s, addr );
            }
            s->signal = 0;
            gdb_continue( s );
            return RS_IDLE;
        case 'C':
            s->signal = gdb_signal_to_target( strtoul( p, ( char** )&p, 16 ) );
            if ( s->signal == -1 )
                s->signal = 0;
            gdb_continue( s );
            return RS_IDLE;
        case 'v':
            if ( strncmp( p, "Cont", 4 ) == 0 ) {
                int res_signal, res_thread;

                p += 4;
                if ( *p == '?' ) {
                    put_packet( s, "vCont;c;C;s;S" );
                    break;
                }
                res = 0;
                res_signal = 0;
                res_thread = 0;
                while ( *p ) {
                    int action, signal;

                    if ( *p++ != ';' ) {
                        res = 0;
                        break;
                    }
                    action = *p++;
                    signal = 0;
                    if ( action == 'C' || action == 'S' ) {
                        signal = strtoul( p, ( char** )&p, 16 );
                    } else if ( action != 'c' && action != 's' ) {
                        res = 0;
                        break;
                    }
                    thread = 0;
                    if ( *p == ':' ) {
                        thread = strtoull( p + 1, ( char** )&p, 16 );
                    }
                    action = tolower( action );
                    if ( res == 0 || ( res == 'c' && action == 's' ) ) {
                        res = action;
                        res_signal = signal;
                        res_thread = thread;
                    }
                }
                if ( res ) {
                    if ( res_thread != -1 && res_thread != 0 ) {
                        env = find_cpu( res_thread );
                        if ( env == NULL ) {
                            put_packet( s, "E22" );
                            break;
                        }
                        s->c_cpu = env;
                    }
                    if ( res == 's' ) {
                        cpu_single_step( s->c_cpu, sstep_flags );
                    }
                    s->signal = res_signal;
                    gdb_continue( s );
                    return RS_IDLE;
                }
                break;
            } else {
                goto unknown_command;
            }
        case 'k':
            /* Kill the target */
            fprintf( stderr, "\nQEMU: Terminated via GDBstub\n" );
            exit( EXIT_SUCCESS );
        case 'D':
            /* Detach packet */
            gdb_breakpoint_remove_all();
            gdb_continue( s );
            put_packet( s, "OK" );
            break;
        case 's':
            if ( *p != '\0' ) {
                addr = strtoull( p, ( char** )&p, 16 );
                gdb_set_cpu_pc( s, addr );
            }
            cpu_single_step( s->c_cpu, sstep_flags );
            gdb_continue( s );
            return RS_IDLE;
        case 'F':
            {
                target_ulong ret;
                target_ulong err;

                ret = strtoull( p, ( char** )&p, 16 );
                if ( *p == ',' ) {
                    p++;
                    err = strtoull( p, ( char** )&p, 16 );
                } else {
                    err = 0;
                }
                if ( *p == ',' )
                    p++;
                type = *p;
                if ( gdb_current_syscall_cb )
                    gdb_current_syscall_cb( s->c_cpu, ret, err );
                if ( type == 'C' ) {
                    put_packet( s, "T02" );
                } else {
                    gdb_continue( s );
                }
            }
            break;
        case 'g':
            len = 0;
            for ( addr = 0; addr < num_g_regs; addr++ ) {
                reg_size = gdb_read_register( s->g_cpu, mem_buf + len, addr );
                len += reg_size;
            }
            memtohex( buf, mem_buf, len );
            put_packet( s, buf );
            break;
        case 'G':
            registers = mem_buf;
            len = strlen( p ) / 2;
            hextomem( ( uint8_t* )registers, p, len );
            for ( addr = 0; addr < num_g_regs && len > 0; addr++ ) {
                reg_size = gdb_write_register( s->g_cpu, registers, addr );
                len -= reg_size;
                registers += reg_size;
            }
            put_packet( s, "OK" );
            break;
        case 'm':
            addr = strtoull( p, ( char** )&p, 16 );
            if ( *p == ',' )
                p++;
            len = strtoull( p, NULL, 16 );
            if ( cpu_memory_rw_debug( s->g_cpu, addr, mem_buf, len, 0 ) != 0 ) {
                put_packet( s, "E14" );
            } else {
                memtohex( buf, mem_buf, len );
                put_packet( s, buf );
            }
            break;
        case 'M':
            addr = strtoull( p, ( char** )&p, 16 );
            if ( *p == ',' )
                p++;
            len = strtoull( p, ( char** )&p, 16 );
            if ( *p == ':' )
                p++;
            hextomem( mem_buf, p, len );
            if ( cpu_memory_rw_debug( s->g_cpu, addr, mem_buf, len, 1 ) != 0 )
                put_packet( s, "E14" );
            else
                put_packet( s, "OK" );
            break;
        case 'p':
            /* Older gdb are really dumb, and don't use 'g' if 'p' is avaialable.
               This works, but can be very slow.  Anything new enough to
               understand XML also knows how to use this properly.  */
            if ( !gdb_has_xml )
                goto unknown_command;
            addr = strtoull( p, ( char** )&p, 16 );
            reg_size = gdb_read_register( s->g_cpu, mem_buf, addr );
            if ( reg_size ) {
                memtohex( buf, mem_buf, reg_size );
                put_packet( s, buf );
            } else {
                put_packet( s, "E14" );
            }
            break;
        case 'P':
            if ( !gdb_has_xml )
                goto unknown_command;
            addr = strtoull( p, ( char** )&p, 16 );
            if ( *p == '=' )
                p++;
            reg_size = strlen( p ) / 2;
            hextomem( mem_buf, p, reg_size );
            gdb_write_register( s->g_cpu, mem_buf, addr );
            put_packet( s, "OK" );
            break;
        case 'Z':
        case 'z':
            type = strtoul( p, ( char** )&p, 16 );
            if ( *p == ',' )
                p++;
            addr = strtoull( p, ( char** )&p, 16 );
            if ( *p == ',' )
                p++;
            len = strtoull( p, ( char** )&p, 16 );
            if ( ch == 'Z' )
                res = gdb_breakpoint_insert( addr, len, type );
            else
                res = gdb_breakpoint_remove( addr, len, type );
            if ( res >= 0 )
                put_packet( s, "OK" );
            else if ( res == -ENOSYS )
                put_packet( s, "" );
            else
                put_packet( s, "E22" );
            break;
        case 'H':
            type = *p++;
            thread = strtoull( p, ( char** )&p, 16 );
            if ( thread == -1 || thread == 0 ) {
                put_packet( s, "OK" );
                break;
            }
            env = find_cpu( thread );
            if ( env == NULL ) {
                put_packet( s, "E22" );
                break;
            }
            switch ( type ) {
                case 'c':
                    s->c_cpu = env;
                    put_packet( s, "OK" );
                    break;
                case 'g':
                    s->g_cpu = env;
                    put_packet( s, "OK" );
                    break;
                default:
                    put_packet( s, "E22" );
                    break;
            }
            break;
        case 'T':
            thread = strtoull( p, ( char** )&p, 16 );
            env = find_cpu( thread );

            if ( env != NULL ) {
                put_packet( s, "OK" );
            } else {
                put_packet( s, "E22" );
            }
            break;
        case 'q':
        case 'Q':
            /* parse any 'q' packets here */
            if ( !strcmp( p, "qemu.sstepbits" ) ) {
                /* Query Breakpoint bit definitions */
                snprintf( buf, sizeof( buf ), "ENABLE=%x,NOIRQ=%x,NOTIMER=%x", SSTEP_ENABLE, SSTEP_NOIRQ, SSTEP_NOTIMER );
                put_packet( s, buf );
                break;
            } else if ( strncmp( p, "qemu.sstep", 10 ) == 0 ) {
                /* Display or change the sstep_flags */
                p += 10;
                if ( *p != '=' ) {
                    /* Display current setting */
                    snprintf( buf, sizeof( buf ), "0x%x", sstep_flags );
                    put_packet( s, buf );
                    break;
                }
                p++;
                type = strtoul( p, ( char** )&p, 16 );
                sstep_flags = type;
                put_packet( s, "OK" );
                break;
            } else if ( strcmp( p, "C" ) == 0 ) {
                /* "Current thread" remains vague in the spec, so always return
                 *  the first CPU (gdb returns the first thread). */
                put_packet( s, "QC1" );
                break;
            } else if ( strcmp( p, "fThreadInfo" ) == 0 ) {
                s->query_cpu = first_cpu;
                goto report_cpuinfo;
            } else if ( strcmp( p, "sThreadInfo" ) == 0 ) {
report_cpuinfo:
                if ( s->query_cpu ) {
                    snprintf( buf, sizeof( buf ), "m%x", gdb_id( s->query_cpu ) );
                    put_packet( s, buf );
                    s->query_cpu = s->query_cpu->next_cpu;
                } else
                    put_packet( s, "l" );
                break;
            } else if ( strncmp( p, "ThreadExtraInfo,", 16 ) == 0 ) {
                thread = strtoull( p + 16, ( char** )&p, 16 );
                env = find_cpu( thread );
                if ( env != NULL ) {
                    len =
                        snprintf( ( char* )mem_buf, sizeof( mem_buf ), "CPU#%d [%s]", env->cpu_index, env->halted ? "halted " : "running" );
                    memtohex( buf, mem_buf, len );
                    put_packet( s, buf );
                }
                break;
            }
            if ( strncmp( p, "Supported", 9 ) == 0 ) {
                snprintf( buf, sizeof( buf ), "PacketSize=%x", MAX_PACKET_LENGTH );
#ifdef GDB_CORE_XML
                pstrcat( buf, sizeof( buf ), ";qXfer:features:read+" );
#endif
                put_packet( s, buf );
                break;
            }
#ifdef GDB_CORE_XML
            if ( strncmp( p, "Xfer:features:read:", 19 ) == 0 ) {
                const char* xml;
                target_ulong total_len;

                gdb_has_xml = 1;
                p += 19;
                xml = get_feature_xml( p, &p );
                if ( !xml ) {
                    snprintf( buf, sizeof( buf ), "E00" );
                    put_packet( s, buf );
                    break;
                }

                if ( *p == ':' )
                    p++;
                addr = strtoul( p, ( char** )&p, 16 );
                if ( *p == ',' )
                    p++;
                len = strtoul( p, ( char** )&p, 16 );

                total_len = strlen( xml );
                if ( addr > total_len ) {
                    snprintf( buf, sizeof( buf ), "E00" );
                    put_packet( s, buf );
                    break;
                }
                if ( len > ( MAX_PACKET_LENGTH - 5 ) / 2 )
                    len = ( MAX_PACKET_LENGTH - 5 ) / 2;
                if ( len < total_len - addr ) {
                    buf[ 0 ] = 'm';
                    len = memtox( buf + 1, xml + addr, len );
                } else {
                    buf[ 0 ] = 'l';
                    len = memtox( buf + 1, xml + addr, total_len - addr );
                }
                put_packet_binary( s, buf, len + 1 );
                break;
            }
#endif
            /* Unrecognised 'q' command.  */
            goto unknown_command;

        default:
unknown_command:
            /* put empty packet */
            buf[ 0 ] = '\0';
            put_packet( s, buf );
            break;
    }
    return RS_IDLE;
}

void gdb_set_stop_cpu( CPUState* env )
{
    gdbserver_state->c_cpu = env;
    gdbserver_state->g_cpu = env;
}

/* Send a gdb syscall request.
   This accepts limited printf-style format specifiers, specifically:
    %x  - target_ulong argument printed in hex.
    %lx - 64-bit argument printed in hex.
    %s  - string pointer (target_ulong) and length (int) pair.  */
void gdb_do_syscall( gdb_syscall_complete_cb cb, const char* fmt, ... )
{
    va_list va;
    char buf[ 256 ];
    char* p;
    target_ulong addr;
    uint64_t i64;
    GDBState* s;

    s = gdbserver_state;
    if ( !s )
        return;
    gdb_current_syscall_cb = cb;
    s->state = RS_IDLE;
    va_start( va, fmt );
    p = buf;
    *( p++ ) = 'F';
    while ( *fmt ) {
        if ( *fmt == '%' ) {
            fmt++;
            switch ( *fmt++ ) {
                case 'x':
                    addr = va_arg( va, target_ulong );
                    p += snprintf( p, &buf[ sizeof( buf ) ] - p, TARGET_FMT_lx, addr );
                    break;
                case 'l':
                    if ( *( fmt++ ) != 'x' )
                        goto bad_format;
                    i64 = va_arg( va, uint64_t );
                    p += snprintf( p, &buf[ sizeof( buf ) ] - p, "%" PRIx64, i64 );
                    break;
                case 's':
                    addr = va_arg( va, target_ulong );
                    p += snprintf( p, &buf[ sizeof( buf ) ] - p, TARGET_FMT_lx "/%x", addr, va_arg( va, int ) );
                    break;
                default:
bad_format:
                    fprintf( stderr, "gdbstub: Bad syscall format string '%s'\n", fmt - 1 );
                    break;
            }
        } else {
            *( p++ ) = *( fmt++ );
        }
    }
    *p = 0;
    va_end( va );
    put_packet( s, buf );
    cpu_exit( s->c_cpu );
}

static void gdb_read_byte( GDBState* s, int ch )
{
    char buf[ 256 ];
    int i, csum;
    uint8_t reply;

#ifdef DEBUG_GDB
    printf( "%s: state %u, byte %02x (%c)\n", __FUNCTION__, s->state, ch, ch );
    fflush( stdout );
#endif

    {
        switch ( s->state ) {
            case RS_IDLE:
                if ( ch == '$' ) {
                    s->line_buf_index = 0;
                    s->state = RS_GETLINE;
                } else if ( ch == 0x03 ) {
                    snprintf( buf, sizeof( buf ), "S%02x", SIGINT );
                    put_packet( s, buf );
                }
                break;
            case RS_GETLINE:
                if ( ch == '#' ) {
                    s->state = RS_CHKSUM1;
                } else if ( s->line_buf_index >= sizeof( s->line_buf ) - 1 ) {
                    s->state = RS_IDLE;
                } else {
                    s->line_buf[ s->line_buf_index++ ] = ch;
                }
                break;
            case RS_CHKSUM1:
                s->line_buf[ s->line_buf_index ] = '\0';
                s->line_csum = fromhex( ch ) << 4;
                s->state = RS_CHKSUM2;
                break;
            case RS_CHKSUM2:
                s->line_csum |= fromhex( ch );
                csum = 0;
                for ( i = 0; i < s->line_buf_index; i++ ) {
                    csum += s->line_buf[ i ];
                }
                if ( s->line_csum != ( csum & 0xff ) ) {
                    reply = '-';
                    put_buffer( s, &reply, 1 );
                    s->state = RS_IDLE;
                } else {
                    reply = '+';
                    put_buffer( s, &reply, 1 );
                    s->state = gdb_handle_packet( s, s->line_buf );
                }
                break;
            default:
                abort();
        }
    }
}

int gdb_queuesig( void )
{
    GDBState* s;

    s = gdbserver_state;

    if ( gdbserver_fd < 0 || s->fd < 0 )
        return 0;
    else
        return 1;
}

int gdb_handlesig( CPUState* env, int sig )
{
    GDBState* s;
    char buf[ 256 ];
    int n;

    s = gdbserver_state;
    if ( gdbserver_fd < 0 || s->fd < 0 )
        return sig;

#ifdef DEBUG_GDB
    printf( "%s: sig: %u\n", __FUNCTION__, sig );
    fflush( stdout );
#endif

    /* disable single step if it was enabled */
    cpu_single_step( env, 0 );
    tb_flush( env );

    if ( sig != 0 ) {
        snprintf( buf, sizeof( buf ), "S%02x", target_signal_to_gdb( sig ) );
        put_packet( s, buf );
    }
    /* put_packet() might have detected that the peer terminated the
       connection.  */
    if ( s->fd < 0 )
        return sig;

    sig = 0;
    s->state = RS_IDLE;
    s->running_state = 0;
    while ( s->running_state == 0 ) {
        n = read( s->fd, buf, 256 );
        if ( n > 0 ) {
            int i;

#ifdef DEBUG_GDB
            printf( "%s: read: %d\n", __FUNCTION__, n );
            fflush( stdout );
#endif

            for ( i = 0; i < n; i++ )
                gdb_read_byte( s, buf[ i ] );
        } else if ( n == 0 || errno != EAGAIN ) {
            /* XXX: Connection closed.  Should probably wait for annother
               connection before continuing.  */
            gdbserver_fd = -1;
            return sig;
        }
    }
    sig = s->signal;
    s->signal = 0;
    return sig;
}

int gdb_poll( CPUState* env )
{
    GDBState* s;
    struct pollfd pfd;

    if ( gdbserver_fd < 0 )
        return 0;

    s = gdbserver_state;

    pfd.fd = s->fd;
    pfd.events = POLLIN | POLLHUP;

    if ( poll( &pfd, 1, 0 ) <= 0 ) {
        if ( errno != EAGAIN )
            return 0;
        return 0;
    }

#ifdef DEBUG_GDB
    printf( "%s: revents: %08x\n", __FUNCTION__, pfd.revents );
    fflush( stdout );
#endif

    if ( pfd.revents & ( POLLIN | POLLHUP ) )
        return 1;

    return 0;
}

/* Tell the remote gdb that the process has exited.  */
void gdb_exit( CPUState* env, int code )
{
    GDBState* s;
    char buf[ 4 ];

    s = gdbserver_state;
    if ( gdbserver_fd < 0 || s->fd < 0 )
        return;

    snprintf( buf, sizeof( buf ), "W%02x", code );
    put_packet( s, buf );
}

/* Tell the remote gdb that the process has exited due to SIG.  */
void gdb_signalled( CPUState* env, int sig )
{
    GDBState* s;
    char buf[ 4 ];

    s = gdbserver_state;
    if ( gdbserver_fd < 0 || s->fd < 0 )
        return;

    snprintf( buf, sizeof( buf ), "X%02x", target_signal_to_gdb( sig ) );
    put_packet( s, buf );
}

static void gdb_accept( void )
{
    GDBState* s;
    struct sockaddr_in sockaddr;
    socklen_t len;
    int val, fd;

    for ( ;; ) {
        len = sizeof( sockaddr );
        fd = accept( gdbserver_fd, ( struct sockaddr* )&sockaddr, &len );
        if ( fd < 0 && errno != EINTR ) {
            perror( "accept" );
            return;
        } else if ( fd >= 0 ) {
#ifndef _WIN32
            fcntl( fd, F_SETFD, FD_CLOEXEC );
#endif
            break;
        }
    }

    /* set short latency */
    val = 1;
    setsockopt( fd, IPPROTO_TCP, TCP_NODELAY, ( char* )&val, sizeof( val ) );

    s = qemu_mallocz( sizeof( GDBState ) );
    s->c_cpu = first_cpu;
    s->g_cpu = first_cpu;
    s->fd = fd;
    gdb_has_xml = 0;

    gdbserver_state = s;

    fcntl( fd, F_SETFL, O_NONBLOCK );

    /* When the debugger is connected, stop accepting connections */
    /* to free the port up for other concurrent instances. */
    close( gdbserver_fd );
}

static int gdbserver_open( int port )
{
    struct sockaddr_in sockaddr;
    int fd, val, ret;

    fd = socket( PF_INET, SOCK_STREAM, 0 );
    if ( fd < 0 ) {
        perror( "socket" );
        return -1;
    }
#ifndef _WIN32
    fcntl( fd, F_SETFD, FD_CLOEXEC );
#endif

    /* allow fast reuse */
    val = 1;
    setsockopt( fd, SOL_SOCKET, SO_REUSEADDR, ( char* )&val, sizeof( val ) );

    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons( port );
    sockaddr.sin_addr.s_addr = 0;
    ret = bind( fd, ( struct sockaddr* )&sockaddr, sizeof( sockaddr ) );
    if ( ret < 0 ) {
        perror( "bind" );
        return -1;
    }
    ret = listen( fd, 0 );
    if ( ret < 0 ) {
        perror( "listen" );
        return -1;
    }
    return fd;
}

int gdbserver_start( int port )
{
    if ( gdbserver_fd >= 0 )
        return -1;

    gdbserver_fd = gdbserver_open( port );
    if ( gdbserver_fd < 0 )
        return -1;
    /* accept connections */
    gdb_accept();
    return 0;
}

/* Disable gdb stub for child processes.  */
void gdbserver_fork( CPUState* env )
{
    GDBState* s = gdbserver_state;
    if ( gdbserver_fd < 0 || s->fd < 0 )
        return;
    close( s->fd );
    s->fd = -1;
    cpu_breakpoint_remove_all( env, BP_GDB );
    cpu_watchpoint_remove_all( env, BP_GDB );
}

int gdbserver_isactive() { return ( gdbserver_fd >= 0 ); }
