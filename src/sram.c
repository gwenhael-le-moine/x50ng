/* $Id: sram.c,v 1.18 2008/12/11 12:18:17 ecd Exp $
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>

#include <memory.h>

#include "x49gp.h"

typedef struct {
    void* data;
    void* shadow;
    char* filename;
    int fd;
    size_t size;
    uint32_t offset;
    x49gp_t* x49gp;
} x49gp_sram_t;

#define S3C2410_SRAM_BASE 0x08000000
#define S3C2410_SRAM_SIZE 0x00080000

#define BASE 0xbcbb5

#define SATURN( r ) ( ( target_phys_addr_t ) & ( ( saturn_cpu_t* )0 )->r )

#if defined( DEBUG_X49GP_SYSRAM_READ ) || defined( DEBUG_X49GP_SYSRAM_WRITE ) || defined( DEBUG_X49GP_IRAM_READ ) ||                       \
    defined( DEBUG_X49GP_IRAM_WRITE ) || defined( DEBUG_X49GP_ERAM_READ ) || defined( DEBUG_X49GP_ERAM_WRITE )
#  define DEBUG_X49GP_SRAM 1
#endif

typedef struct {
    uint32_t x;
    uint32_t ml;
    uint32_t mh;
    uint8_t m;
    uint8_t s;
} hp_real_t;

#ifdef DEBUG_X49GP_SRAM

static uint32_t saturn_map_s2a( saturn_cpu_t* saturn, uint32_t saddr )
{
    uint32_t addr;

    addr = ldl_p( &saturn->read_map[ saddr >> 12 ] ) | ( ( saddr >> 1 ) & 0x7ff );

    //	printf("SATURN: saddr %05x, addr %08x\n", saddr, addr);

    return addr;
}

static uint32_t saturn_peek( saturn_cpu_t* saturn, uint32_t saddr, uint32_t size )
{
    uint32_t addr, rot, mask, data;
    uint64_t value;

    addr = saturn_map_s2a( saturn, saddr ) & 0xfffffffc;
    if ( addr > 0x08080000 )
        return 0;

    // printf("SATURN: addr %08x\n", addr);
    value = ( ( uint64_t )ldl_phys( addr ) ) | ( ( ( uint64_t )ldl_phys( addr + 4 ) ) << 32 );
    // printf("SATURN: value %016llx\n", value);

    rot = ( saddr & 7 ) << 2;
    mask = ( 1ULL << ( size << 2 ) ) - 1;
    // printf("SATURN: rot %u, mask %08x\n", rot, mask);

    data = ( ( uint32_t )( value >> rot ) ) & mask;
    // printf("SATURN: data %08x\n", data);

    return data;
}

static uint32_t saturn_peek_address( saturn_cpu_t* saturn, uint32_t saddr ) { return saturn_peek( saturn, saddr, 5 ); }

static int hxs2real( int hxs )
{
    int n = 0, c = 1;

    while ( hxs ) {
        n += ( hxs & 0xf ) * c;
        c *= 10;
        hxs >>= 4;
    }
    return n;
}

static char* real_number( saturn_cpu_t* saturn, uint32_t saddr, char* buffer, int ml, int xl )
{
    char* p = buffer;
    char fmt[ 20 ];
    char m[ 16 ];
    hp_real_t r;
    int re, xs;
    int i;
    uint32_t pc;

    pc = saddr;

    /*
     * Read the number
     */
    r.x = saturn_peek( saturn, pc, xl );
    pc += xl;
    r.ml = saturn_peek( saturn, pc, ml - 8 );
    pc += ml - 8;
    r.mh = saturn_peek( saturn, pc, 8 );
    pc += 8;
    r.m = saturn_peek( saturn, pc, 1 );
    pc += 1;
    r.s = saturn_peek( saturn, pc, 1 );
    pc += 1;

    /*
     * Figure out the exponent
     */
    xs = 5;
    while ( --xl )
        xs *= 10;
    re = hxs2real( r.x );
    if ( re >= xs )
        re = re - 2 * xs;

    if ( ( re >= 0 ) && ( re < ml + 1 ) ) {
        if ( r.s >= 5 )
            *p++ = '-';

        sprintf( fmt, "%%.1X%%.8X%%.%dX", ml - 8 );
        sprintf( m, fmt, r.m, r.mh, r.ml );

        for ( i = 0; i <= re; i++ )
            *p++ = m[ i ];
        *p++ = '.';
        for ( ; i < ml + 1; i++ )
            *p++ = m[ i ];
        p--;
        while ( *p == '0' )
            p--;
        if ( *p == '.' )
            p--;
        *++p = '\0';

        return buffer;
    }

    if ( ( re < 0 ) && ( re >= -ml - 1 ) ) {
        sprintf( fmt, "%%.1X%%.8X%%.%dX", ml - 8 );
        sprintf( m, fmt, r.m, r.mh, r.ml );

        for ( i = ml; m[ i ] == '0'; i-- )
            ;

        if ( -re <= ml - i + 1 ) {
            if ( r.s >= 5 )
                *p++ = '-';

            *p++ = '.';

            for ( i = 1; i < -re; i++ )
                *p++ = '0';

            for ( i = 0; i < ml + 1; i++ )
                *p++ = m[ i ];
            p--;
            while ( *p == '0' )
                p--;
            *++p = '\0';

            return buffer;
        }
    }

    sprintf( fmt, "%%s%%X.%%.8X%%.%dX", ml - 8 );
    sprintf( p, fmt, ( r.s >= 5 ) ? "-" : "", r.m, r.mh, r.ml );

    p += strlen( p ) - 1;

    while ( *p == '0' )
        p--;
    *++p = '\0';

    if ( re ) {
        sprintf( p, "E%d", re );
        p += strlen( p );
        *p = '\0';
    }

    return buffer;
}

static uint32_t dump_object( x49gp_t* x49gp, x49gp_sram_t* sram, uint32_t saddr )
{
    saturn_cpu_t* saturn = ( sram->data + 0x3340 );
    char buffer[ 128 ];
    uint32_t prolog, pc;
    char c;
    int i, n;

    pc = saddr;

    prolog = saturn_peek( saturn, pc, 5 );
    pc += 5;

    switch ( prolog ) {
        case 0x02e48:
        case 0x02e6d:
            printf( " " );
            n = saturn_peek( saturn, pc, 2 );
            pc += 2;
            for ( i = 0; i < n; i++ ) {
                c = saturn_peek( saturn, pc, 2 );
                printf( "%c", c );
                pc += 2;
            }
            break;

        case 0x02a4e:
            n = saturn_peek( saturn, pc, 5 );
            pc += 5;
            if ( n <= 16 ) {
                printf( " #%08x%08xh", saturn_peek( saturn, pc + 8, 8 ), saturn_peek( saturn, pc + 0, 8 ) );
            } else {
                printf( " C#" );
                for ( i = 0; i < n; i++ ) {
                    printf( "%x", saturn_peek( saturn, pc + i, 1 ) );
                }
            }
            pc += n;
            break;

        case 0x02911:
            printf( " <%05x>", saturn_peek( saturn, pc, 5 ) );
            pc += 5;
            break;

        case 0x02933:
            printf( " %%%s", real_number( saturn, pc, buffer, 11, 3 ) );
            pc += 16;
            break;

        case 0x02955:
            printf( " %%%%%s", real_number( saturn, pc, buffer, 14, 5 ) );
            pc += 21;
            break;

        case 0x02e92:
            printf( " <%03x %03x>", saturn_peek( saturn, pc + 0, 3 ), saturn_peek( saturn, pc + 3, 3 ) );
            pc += 6;
            break;

        case 0x026ac:
            printf( " <%03x %04x>", saturn_peek( saturn, pc + 0, 3 ), saturn_peek( saturn, pc + 3, 4 ) );
            pc += 7;
            break;

        case 0x02ab8:
            printf( " '" );
            while ( 1 ) {
                prolog = saturn_peek( saturn, pc, 5 );
                if ( prolog == 0x0312b ) {
                    pc += 5;
                    break;
                }
                pc += dump_object( x49gp, sram, pc );
            }
            printf( " '" );
            break;

        case 0x02d9d:
            printf( " :" );
            while ( 1 ) {
                prolog = saturn_peek( saturn, pc, 5 );
                if ( prolog == 0x0312b ) {
                    pc += 5;
                    break;
                }
                pc += dump_object( x49gp, sram, pc );
            }
            printf( " ;" );
            break;

        default:
            printf( " %05x", prolog );
            break;
    }

    return pc - saddr;
}

static void debug_saturn( x49gp_t* x49gp, x49gp_sram_t* sram )
{
    saturn_cpu_t* saturn = ( sram->data + 0x3340 );
    uint32_t sp, se;
    uint32_t prolog;
    uint32_t rsp;
    uint32_t first;
    static uint32_t prev_first = 0;
    static int state = 0;
    int depth;
    int i;

    //	printf("SATURN: %p, RPLTOP: %08x\n", saturn, saturn_map_s2a(saturn, SAT_RPLTOP));
    //	printf("SATURN: %p, RSKTOP: %08x\n", saturn, saturn_map_s2a(saturn, SAT_RSKTOP));

    //	rpltop = saturn_peek_address(saturn, SAT_RPLTOP);
    //	printf("SATURN: RPLTOP: %05x\n", rpltop);

    rsp = saturn_peek_address( saturn, SAT_RSKTOP );
    if ( rsp == 0 )
        return;

    first = saturn_peek_address( saturn, rsp - 5 );
    if ( first == prev_first ) {
        return;
    }
    prev_first = first;

    if ( first == ( BASE + 0x14a54 ) ) {
        state = 1;
    }
    if ( state == 0 )
        return;

    printf( "SATURN: %p, rsp: %05x %08x\n", saturn, rsp, saturn_map_s2a( saturn, rsp ) );

    for ( i = 0;; i++ ) {
        rsp -= 5;

        se = saturn_peek_address( saturn, rsp );
        if ( se == 0 )
            break;

        if ( se > BASE )
            printf( "SATURN: RSTK %02u: %05x <%05x>\n", i, se, se - BASE );
        else
            printf( "SATURN: RSTK %02u: %05x\n", i, se );
    }

    depth = ( saturn_peek_address( saturn, SAT_EDITLINE ) - saturn_peek_address( saturn, SAT_DSKTOP ) - 5 ) / 5;

    printf( "SATURN: depth %d\n", depth );

    sp = saturn_peek_address( saturn, SAT_EDITLINE ) - 10;

    for ( i = 0; i < depth; i++ ) {
        se = saturn_peek_address( saturn, sp );
        sp -= 5;

        prolog = saturn_peek( saturn, se, 5 );

        printf( "SATURN: %02u: <%05x> <%05x>", depth - i, se, prolog );

        dump_object( x49gp, sram, se );

        printf( "\n" );
    }
}

static uint32_t sram_get_word( void* opaque, target_phys_addr_t offset )
{
    x49gp_sram_t* sram = opaque;
    uint32_t data;

    data = ldl_p( sram->data + offset );

#  if 0
        if (offset == 0x00000a1c)
                printf("read  SRAM at offset %08x: %08x (pc %08x)\n", offset, data, x49gp->arm->Reg[15]);
#  endif

#  ifdef DEBUG_X49GP_SYSRAM_READ
    if ( ( offset & ~( 0x0001ffff ) ) == 0x00000000 )
        printf( "read  SRAM 4 at offset %08x: %08x\n", offset, data );
#  endif
#  ifdef DEBUG_X49GP_ERAM_READ
    if ( ( offset & ~( 0x0001ffff ) ) == 0x00020000 )
        printf( "read  SRAM 4 at offset %08x: %08x\n", offset, data );
#  endif
#  ifdef DEBUG_X49GP_IRAM_READ
    if ( ( offset & ~( 0x0003ffff ) ) == 0x00040000 )
        printf( "read  SRAM 4 at offset %08x: %08x\n", offset, data );
#  endif

    return data;
}

static uint32_t sram_get_halfword( void* opaque, target_phys_addr_t offset )
{
    x49gp_sram_t* sram = opaque;
    unsigned short data;

    data = lduw_p( sram->data + offset );

#  ifdef DEBUG_X49GP_SYSRAM_READ
    if ( ( offset & ~( 0x0001ffff ) ) == 0x00000000 ) {
        printf( "read  SRAM 2 at offset %08x: %04x\n", offset, data );
    }
#  endif
#  ifdef DEBUG_X49GP_ERAM_READ
    if ( ( offset & ~( 0x0001ffff ) ) == 0x00020000 ) {
        printf( "read  SRAM 2 at offset %08x: %04x\n", offset, data );
    }
#  endif
#  ifdef DEBUG_X49GP_IRAM_READ
    if ( ( offset & ~( 0x0003ffff ) ) == 0x00040000 ) {
        printf( "read  SRAM 2 at offset %08x: %04x\n", offset, data );
    }
#  endif

    return data;
}

static uint32_t sram_get_byte( void* opaque, target_phys_addr_t offset )
{
    x49gp_sram_t* sram = opaque;
    unsigned char data;

    data = ldub_p( sram->data + offset );

#  ifdef DEBUG_X49GP_SYSRAM_READ
    if ( ( offset & ~( 0x0001ffff ) ) == 0x00000000 ) {
        printf( "read  SRAM 1 at offset %08x: %02x\n", offset, data );
    }
#  endif
#  ifdef DEBUG_X49GP_ERAM_READ
    if ( ( offset & ~( 0x0001ffff ) ) == 0x00020000 ) {
        printf( "read  SRAM 1 at offset %08x: %02x\n", offset, data );
    }
#  endif
#  ifdef DEBUG_X49GP_IRAM_READ
    if ( ( offset & ~( 0x0003ffff ) ) == 0x00040000 ) {
        printf( "read  SRAM 1 at offset %08x: %02x\n", offset, data );
    }
#  endif

    return data;
}

static void sram_put_word( void* opaque, target_phys_addr_t offset, uint32_t data )
{
    x49gp_sram_t* sram = opaque;

    if ( offset == 0x00000a1c ) {
        printf( "write SRAM 4 at offset %08x: %08x (pc %08x)\n", offset, data, sram->x49gp->env->regs[ 15 ] );
    }

    debug_saturn( sram->x49gp, sram );

#  if 0
        if (offset == 0x3340 + SATURN(D1)) {
                printf("write D1 at offset %08x: %08x (pc %08x)\n",
                        offset, data, sram->x49gp->env->regs[15]);
        }
        if (offset == 0x3340 + SATURN(A)) {
                printf("write A at offset %08x: %08x (pc %08x)\n",
                        offset, data, sram->x49gp->env->regs[15]);
        }
        if (offset == 0x3340 + SATURN(A) + 8) {
                printf("write Al at offset %08x: %08x (pc %08x)\n",
                        offset, data, sram->x49gp->env->regs[15]);
        }
#  endif

#  ifdef DEBUG_X49GP_SYSRAM_WRITE
    if ( ( offset & ~( 0x0001ffff ) ) == 0x00000000 ) {
        printf( "write SRAM 4 at offset %08x: %08x\n", offset, data );
    }
#  endif
#  ifdef DEBUG_X49GP_ERAM_WRITE
    if ( ( offset & ~( 0x0001ffff ) ) == 0x00020000 ) {
        printf( "write SRAM 4 at offset %08x: %08x\n", offset, data );
    }
#  endif
#  ifdef DEBUG_X49GP_IRAM_WRITE
    if ( ( offset & ~( 0x0003ffff ) ) == 0x00040000 ) {
        printf( "write SRAM 4 at offset %08x: %08x\n", offset, data );
    }
#  endif

    stl_p( sram->data + offset, data );
}

static void sram_put_halfword( void* opaque, target_phys_addr_t offset, uint32_t data )
{
    x49gp_sram_t* sram = opaque;

    data &= 0xffff;

#  ifdef DEBUG_X49GP_SYSRAM_WRITE
    if ( ( offset & ~( 0x0001ffff ) ) == 0x00000000 ) {
        printf( "write SRAM 2 at offset %08x: %04x\n", offset, data );
    }
#  endif
#  ifdef DEBUG_X49GP_ERAM_WRITE
    if ( ( offset & ~( 0x0001ffff ) ) == 0x00020000 ) {
        printf( "write SRAM 2 at offset %08x: %04x\n", offset, data );
    }
#  endif
#  ifdef DEBUG_X49GP_IRAM_WRITE
    if ( ( offset & ~( 0x0003ffff ) ) == 0x00040000 ) {
        printf( "write SRAM 2 at offset %08x: %04x\n", offset, data );
    }
#  endif

    stw_p( sram->data + offset, data );
}

static void sram_put_byte( void* opaque, target_phys_addr_t offset, uint32_t data )
{
    x49gp_sram_t* sram = opaque;

    data &= 0xff;

#  ifdef DEBUG_X49GP_SYSRAM_WRITE
    if ( ( offset & ~( 0x0001ffff ) ) == 0x00000000 ) {
        printf( "write SRAM 1 at offset %08x: %02x\n", offset, data );
    }
#  endif
#  ifdef DEBUG_X49GP_ERAM_WRITE
    if ( ( offset & ~( 0x0001ffff ) ) == 0x00020000 ) {
        printf( "write SRAM 1 at offset %08x: %02x\n", offset, data );
    }
#  endif
#  ifdef DEBUG_X49GP_IRAM_WRITE
    if ( ( offset & ~( 0x0003ffff ) ) == 0x00040000 ) {
        printf( "write SRAM 1 at offset %08x: %02x\n", offset, data );
    }
#  endif

    stb_p( sram->data + offset, data );
}

static CPUReadMemoryFunc* sram_readfn[] = { sram_get_byte, sram_get_halfword, sram_get_word };

static CPUWriteMemoryFunc* sram_writefn[] = { sram_put_byte, sram_put_halfword, sram_put_word };

#endif /* DEBUG_X49GP_SRAM */

static int sram_load( x49gp_module_t* module, GKeyFile* key )
{
    x49gp_sram_t* sram = module->user_data;
    char* filename;
    int error;

#ifdef DEBUG_X49GP_MODULES
    printf( "%s: %s:%u\n", module->name, __FUNCTION__, __LINE__ );
#endif

    error = x49gp_module_get_filename( module, key, "filename", "sram", &( sram->filename ), &filename );

    sram->fd = open( filename, O_RDWR | O_CREAT, 0644 );
    if ( sram->fd < 0 ) {
        error = -errno;
        fprintf( stderr, "%s: %s:%u: open %s: %s\n", module->name, __FUNCTION__, __LINE__, filename, strerror( errno ) );
        g_free( filename );
        return error;
    }

    sram->size = 0x00080000;
    if ( ftruncate( sram->fd, sram->size ) < 0 ) {
        error = -errno;
        fprintf( stderr, "%s: %s:%u: ftruncate %s: %s\n", module->name, __FUNCTION__, __LINE__, filename, strerror( errno ) );
        g_free( filename );
        close( sram->fd );
        sram->fd = -1;
        return error;
    }

    sram->data = mmap( phys_ram_base + sram->offset, sram->size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, sram->fd, 0 );
    if ( sram->data == ( void* )-1 ) {
        error = -errno;
        fprintf( stderr, "%s: %s:%u: mmap %s: %s\n", module->name, __FUNCTION__, __LINE__, filename, strerror( errno ) );
        g_free( filename );
        close( sram->fd );
        sram->fd = -1;
        return error;
    }

    sram->shadow =
        mmap( phys_ram_base + sram->offset + sram->size, sram->size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, sram->fd, 0 );
    if ( sram->shadow == ( void* )-1 ) {
        error = -errno;
        fprintf( stderr, "%s: %s:%u: mmap %s (shadow): %s\n", module->name, __FUNCTION__, __LINE__, filename, strerror( errno ) );
        g_free( filename );
        close( sram->fd );
        sram->fd = -1;
        return error;
    }

    sram->x49gp->sram = phys_ram_base + sram->offset;

    g_free( filename );
    return error;
}

static int sram_save( x49gp_module_t* module, GKeyFile* key )
{
    x49gp_sram_t* sram = module->user_data;
    int error;

#ifdef DEBUG_X49GP_MODULES
    printf( "%s: %s:%u\n", module->name, __FUNCTION__, __LINE__ );
#endif

    x49gp_module_set_filename( module, key, "filename", sram->filename );

    error = msync( sram->data, sram->size, MS_ASYNC );
    if ( error ) {
        fprintf( stderr, "%s:%u: msync: %s\n", __FUNCTION__, __LINE__, strerror( errno ) );
        return error;
    }

    error = fsync( sram->fd );
    if ( error ) {
        fprintf( stderr, "%s:%u: fsync: %s\n", __FUNCTION__, __LINE__, strerror( errno ) );
        return error;
    }

    return 0;
}

static int sram_reset( x49gp_module_t* module, x49gp_reset_t reset )
{
#ifdef DEBUG_X49GP_MODULES
    printf( "%s: %s:%u\n", module->name, __FUNCTION__, __LINE__ );
#endif

    return 0;
}

static int sram_init( x49gp_module_t* module )
{
    x49gp_sram_t* sram;

#ifdef DEBUG_X49GP_MODULES
    printf( "%s: %s:%u\n", module->name, __FUNCTION__, __LINE__ );
#endif

    sram = malloc( sizeof( x49gp_sram_t ) );
    if ( NULL == sram ) {
        fprintf( stderr, "%s:%u: Out of memory\n", __FUNCTION__, __LINE__ );
        return -ENOMEM;
    }
    memset( sram, 0, sizeof( x49gp_sram_t ) );

    sram->fd = -1;

    module->user_data = sram;
    sram->x49gp = module->x49gp;

    sram->data = ( void* )-1;
    sram->shadow = ( void* )-1;
    sram->offset = phys_ram_size;
    phys_ram_size += S3C2410_SRAM_SIZE;
    phys_ram_size += S3C2410_SRAM_SIZE;

#ifdef DEBUG_X49GP_SRAM
    {
        int iotype;

        iotype = cpu_register_io_memory( 0, sram_readfn, sram_writefn, sram );
        printf( "%s: iotype %08x\n", __FUNCTION__, iotype );
        cpu_register_physical_memory( S3C2410_SRAM_BASE, S3C2410_SRAM_SIZE, sram->offset | iotype | IO_MEM_ROMD );

        iotype = cpu_register_io_memory( 0, sram_readfn, sram_writefn, sram );
        printf( "%s: iotype %08x\n", __FUNCTION__, iotype );
        cpu_register_physical_memory( S3C2410_SRAM_BASE + S3C2410_SRAM_SIZE, S3C2410_SRAM_SIZE,
                                      ( sram->offset + S3C2410_SRAM_SIZE ) | iotype | IO_MEM_ROMD );
    }
#else
    cpu_register_physical_memory( S3C2410_SRAM_BASE, S3C2410_SRAM_SIZE, sram->offset | IO_MEM_RAM );
    cpu_register_physical_memory( S3C2410_SRAM_BASE + S3C2410_SRAM_SIZE, S3C2410_SRAM_SIZE,
                                  ( sram->offset + S3C2410_SRAM_SIZE ) | IO_MEM_RAM );
#endif

    return 0;
}

static int sram_exit( x49gp_module_t* module )
{
    x49gp_sram_t* sram;

#ifdef DEBUG_X49GP_MODULES
    printf( "%s: %s:%u\n", module->name, __FUNCTION__, __LINE__ );
#endif

    if ( module->user_data ) {
        sram = module->user_data;

        if ( sram->shadow != ( void* )-1 ) {
            munmap( sram->shadow, sram->size );
        }
        if ( sram->data != ( void* )-1 ) {
            munmap( sram->data, sram->size );
        }
        if ( sram->fd >= 0 ) {
            close( sram->fd );
        }

        free( sram );
    }

    x49gp_module_unregister( module );
    free( module );

    return 0;
}

int x49gp_sram_init( x49gp_t* x49gp )
{
    x49gp_module_t* module;

    if ( x49gp_module_init( x49gp, "sram", sram_init, sram_exit, sram_reset, sram_load, sram_save, NULL, &module ) ) {
        return -1;
    }

    return x49gp_module_register( module );
}
