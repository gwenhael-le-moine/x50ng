#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>

#include <memory.h>

#include "module.h"

typedef struct hdw_sram_t {
    void* data;
    void* shadow;
    char* filename;
    int fd;
    size_t size;
    uint32_t offset;
    hdw_t* x50ng;
} hdw_sram_t;

#define S3C2410_SRAM_BASE 0x08000000
#define S3C2410_SRAM_SIZE 0x00080000

typedef struct {
    uint32_t x;
    uint32_t ml;
    uint32_t mh;
    uint8_t m;
    uint8_t s;
} hp_real_t;

static int sram_load( hdw_module_t* module, GKeyFile* key )
{
    hdw_sram_t* sram = module->user_data;
    char* filename;
    int error;

#ifdef DEBUG_X50NG_MODULES
    printf( "%s: %s:%u\n", module->name, __func__, __LINE__ );
#endif

    error = x50ng_module_get_filename( module, key, "filename", "sram", &( sram->filename ), &filename );

    sram->fd = open( filename, O_RDWR | O_CREAT, 0644 );
    if ( sram->fd < 0 ) {
        error = -errno;
        fprintf( stderr, "%s: %s:%u: open %s: %s\n", module->name, __func__, __LINE__, filename, strerror( errno ) );
        g_free( filename );
        return error;
    }

    sram->size = 0x00080000;
    if ( ftruncate( sram->fd, sram->size ) < 0 ) {
        error = -errno;
        fprintf( stderr, "%s: %s:%u: ftruncate %s: %s\n", module->name, __func__, __LINE__, filename, strerror( errno ) );
        g_free( filename );
        close( sram->fd );
        sram->fd = -1;
        return error;
    }

    sram->data = mmap( phys_ram_base + sram->offset, sram->size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, sram->fd, 0 );
    if ( sram->data == ( void* )-1 ) {
        error = -errno;
        fprintf( stderr, "%s: %s:%u: mmap %s: %s\n", module->name, __func__, __LINE__, filename, strerror( errno ) );
        g_free( filename );
        close( sram->fd );
        sram->fd = -1;
        return error;
    }

    sram->shadow =
        mmap( phys_ram_base + sram->offset + sram->size, sram->size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, sram->fd, 0 );
    if ( sram->shadow == ( void* )-1 ) {
        error = -errno;
        fprintf( stderr, "%s: %s:%u: mmap %s (shadow): %s\n", module->name, __func__, __LINE__, filename, strerror( errno ) );
        g_free( filename );
        close( sram->fd );
        sram->fd = -1;
        return error;
    }

    sram->x50ng->sram = phys_ram_base + sram->offset;

    g_free( filename );
    return error;
}

static int sram_save( hdw_module_t* module, GKeyFile* key )
{
    hdw_sram_t* sram = module->user_data;
    int error;

#ifdef DEBUG_X50NG_MODULES
    printf( "%s: %s:%u\n", module->name, __func__, __LINE__ );
#endif

    x50ng_module_set_filename( module, key, "filename", sram->filename );

    error = msync( sram->data, sram->size, MS_ASYNC );
    if ( error ) {
        fprintf( stderr, "%s:%u: msync: %s\n", __func__, __LINE__, strerror( errno ) );
        return error;
    }

    error = fsync( sram->fd );
    if ( error ) {
        fprintf( stderr, "%s:%u: fsync: %s\n", __func__, __LINE__, strerror( errno ) );
        return error;
    }

    return 0;
}

static int sram_reset( hdw_module_t* module, x50ng_reset_t reset )
{
#ifdef DEBUG_X50NG_MODULES
    printf( "%s: %s:%u\n", module->name, __func__, __LINE__ );
#endif

    return 0;
}

static int sram_init( hdw_module_t* module )
{
    hdw_sram_t* sram;

#ifdef DEBUG_X50NG_MODULES
    printf( "%s: %s:%u\n", module->name, __func__, __LINE__ );
#endif

    sram = malloc( sizeof( hdw_sram_t ) );
    if ( NULL == sram ) {
        fprintf( stderr, "%s:%u: Out of memory\n", __func__, __LINE__ );
        return -ENOMEM;
    }
    memset( sram, 0, sizeof( hdw_sram_t ) );

    sram->fd = -1;

    module->user_data = sram;
    sram->x50ng = module->x50ng;

    sram->data = ( void* )-1;
    sram->shadow = ( void* )-1;
    sram->offset = phys_ram_size;
    phys_ram_size += S3C2410_SRAM_SIZE;
    phys_ram_size += S3C2410_SRAM_SIZE;

    cpu_register_physical_memory( S3C2410_SRAM_BASE, S3C2410_SRAM_SIZE, sram->offset | IO_MEM_RAM );
    cpu_register_physical_memory( S3C2410_SRAM_BASE + S3C2410_SRAM_SIZE, S3C2410_SRAM_SIZE,
                                  ( sram->offset + S3C2410_SRAM_SIZE ) | IO_MEM_RAM );

    return 0;
}

static int sram_exit( hdw_module_t* module )
{
    hdw_sram_t* sram;

#ifdef DEBUG_X50NG_MODULES
    printf( "%s: %s:%u\n", module->name, __func__, __LINE__ );
#endif

    if ( module->user_data ) {
        sram = module->user_data;

        if ( sram->shadow != ( void* )-1 )
            munmap( sram->shadow, sram->size );

        if ( sram->data != ( void* )-1 )
            munmap( sram->data, sram->size );

        if ( sram->fd >= 0 )
            close( sram->fd );

        free( sram );
    }

    x50ng_module_unregister( module );
    free( module );

    return 0;
}

int x50ng_sram_init( hdw_t* hdw_state )
{
    hdw_module_t* module;

    if ( x50ng_module_init( hdw_state, "sram", sram_init, sram_exit, sram_reset, sram_load, sram_save, NULL, &module ) )
        return -1;

    return x50ng_module_register( module );
}
