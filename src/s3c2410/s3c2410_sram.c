#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>

#include "../for_qemu.h"
#include "../hdw.h"
#include "../module.h"

#include "s3c2410.h"

typedef struct {
    void* data;
    char* filename;
    int fd;
    uint32_t offset;
    size_t size;
} filemap_t;

static int s3c2410_sram_load( hdw_module_t* module, GKeyFile* key )
{
    filemap_t* filemap = module->user_data;
    char* filename;
    int error;

#ifdef DEBUG_X50NG_MODULES
    printf( "%s: %s:%u\n", module->name, __func__, __LINE__ );
#endif

    error = module_get_filename( module, key, "filename", "s3c2410-sram", &( filemap->filename ), &filename );

    filemap->fd = open( filename, O_RDWR | O_CREAT, 0644 );
    if ( filemap->fd < 0 ) {
        error = -errno;
        fprintf( stderr, "%s: %s:%u: open %s: %s\n", module->name, __func__, __LINE__, filename, strerror( errno ) );
        g_free( filename );
        return error;
    }

    filemap->size = S3C2410_SRAM_SIZE;
    if ( ftruncate( filemap->fd, filemap->size ) < 0 ) {
        error = -errno;
        fprintf( stderr, "%s: %s:%u: ftruncate %s: %s\n", module->name, __func__, __LINE__, filename, strerror( errno ) );
        g_free( filename );
        close( filemap->fd );
        filemap->fd = -1;
        return error;
    }

    filemap->data = mmap( phys_ram_base + filemap->offset, filemap->size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, filemap->fd, 0 );
    if ( filemap->data == ( void* )-1 ) {
        error = -errno;
        fprintf( stderr, "%s: %s:%u: mmap %s: %s\n", module->name, __func__, __LINE__, filename, strerror( errno ) );
        g_free( filename );
        close( filemap->fd );
        filemap->fd = -1;
        return error;
    }

    g_free( filename );

    return error;
}

static int s3c2410_sram_save( hdw_module_t* module, GKeyFile* key )
{
    filemap_t* filemap = module->user_data;
    int error;

#ifdef DEBUG_X50NG_MODULES
    printf( "%s: %s:%u\n", module->name, __func__, __LINE__ );
#endif

    module_set_filename( module, key, "filename", filemap->filename );

    error = msync( filemap->data, filemap->size, MS_ASYNC );
    if ( error ) {
        fprintf( stderr, "%s:%u: msync: %s\n", __func__, __LINE__, strerror( errno ) );
        return error;
    }

    error = fsync( filemap->fd );
    if ( error ) {
        fprintf( stderr, "%s:%u: fsync: %s\n", __func__, __LINE__, strerror( errno ) );
        return error;
    }

    return 0;
}

static int s3c2410_sram_reset( hdw_module_t* module, hdw_reset_t reset )
{
#ifdef DEBUG_X50NG_MODULES
    printf( "%s: %s:%u\n", module->name, __func__, __LINE__ );
#endif

    return 0;
}

static int s3c2410_sram_init( hdw_module_t* module )
{
    filemap_t* filemap;

#ifdef DEBUG_X50NG_MODULES
    printf( "%s: %s:%u\n", module->name, __func__, __LINE__ );
#endif

    filemap = malloc( sizeof( filemap_t ) );
    if ( NULL == filemap ) {
        fprintf( stderr, "%s:%u: Out of memory\n", __func__, __LINE__ );
        return -ENOMEM;
    }

    filemap->size = 0;
    filemap->fd = -1;

    module->user_data = filemap;

    filemap->data = ( void* )-1;
    filemap->offset = phys_ram_size;
    phys_ram_size += S3C2410_SRAM_SIZE;

    cpu_register_physical_memory( S3C2410_SRAM_BASE, S3C2410_SRAM_SIZE, filemap->offset | IO_MEM_RAM );

    return 0;
}

static int s3c2410_sram_exit( hdw_module_t* module )
{
    filemap_t* filemap;

#ifdef DEBUG_X50NG_MODULES
    printf( "%s: %s:%u\n", module->name, __func__, __LINE__ );
#endif

    if ( module->user_data ) {
        filemap = module->user_data;

        if ( filemap->data != ( void* )-1 ) {
            munmap( filemap->data, filemap->size );
        }
        if ( filemap->fd >= 0 ) {
            close( filemap->fd );
        }

        free( filemap );
    }

    module_unregister( module );
    free( module );

    return 0;
}

int s3c2410_init_module_sram( hdw_t* hdw_state )
{
    hdw_module_t* module;

    if ( x50ng_module_init( hdw_state, "s3c2410-sram", s3c2410_sram_init, s3c2410_sram_exit, s3c2410_sram_reset, s3c2410_sram_load,
                            s3c2410_sram_save, NULL, &module ) )
        return -1;

    return module_register( module );
}
