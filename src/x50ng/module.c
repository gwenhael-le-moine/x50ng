#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>

#include <glib.h>
#include <gio/gio.h>

#include "list.h"
#include "x50ng.h"
#include "options.h"

#define STATE_FILE_NAME "state"

int x50ng_modules_init( x50ng_t* x50ng )
{
    x50ng_module_t* module;
    int error;

#ifdef DEBUG_X50NG_MODULES
    printf( "%s:%u:\n", __FUNCTION__, __LINE__ );
#endif

    phys_ram_size = 0;

    list_for_each_entry( module, &x50ng->modules, list )
    {
        error = module->init( module );
        if ( error )
            return error;
    }

    phys_ram_base = mmap( 0, phys_ram_size, PROT_NONE, MAP_SHARED | MAP_ANON, -1, 0 );
    if ( phys_ram_base == ( uint8_t* )-1 ) {
        fprintf( stderr, "%s: can't mmap %08x anonymous bytes\n", __FUNCTION__, phys_ram_size );
        exit( EXIT_FAILURE );
    }

#ifdef DEBUG_X50NG_MODULES
    printf( "%s: phys_ram_base: %p\n", __FUNCTION__, phys_ram_base );
#endif

    phys_ram_dirty = qemu_vmalloc( phys_ram_size >> TARGET_PAGE_BITS );
    memset( phys_ram_dirty, 0xff, phys_ram_size >> TARGET_PAGE_BITS );

    ram_addr_t x50ng_ram_alloc( ram_addr_t size, uint8_t* base );
    x50ng_ram_alloc( phys_ram_size, phys_ram_base );

    return 0;
}

int x50ng_modules_exit( x50ng_t* x50ng )
{
    x50ng_module_t *module, *next;
    int error;

#ifdef DEBUG_X50NG_MODULES
    printf( "%s:%u:\n", __FUNCTION__, __LINE__ );
#endif

    list_for_each_entry_safe_reverse( module, next, &x50ng->modules, list )
    {
        error = module->exit( module );
        if ( error )
            return error;
    }

    return 0;
}

int x50ng_modules_reset( x50ng_t* x50ng, x50ng_reset_t reset )
{
    x50ng_module_t* module;
    int error;

#ifdef DEBUG_X50NG_MODULES
    printf( "%s:%u:\n", __FUNCTION__, __LINE__ );
#endif

    list_for_each_entry( module, &x50ng->modules, list )
    {
        error = module->reset( module, reset );
        if ( error )
            return error;
    }

    return 0;
}

int x50ng_modules_load( x50ng_t* x50ng )
{
    x50ng_module_t* module;
    GError* gerror = NULL;
    int error, result;
    const char* filename = g_build_filename( opt.datadir, STATE_FILE_NAME, NULL );

#ifdef DEBUG_X50NG_MODULES
    printf( "%s:%u:\n", __FUNCTION__, __LINE__ );
#endif

    if ( g_mkdir_with_parents( opt.datadir, 0755 ) ) {
        error = -errno;
        fprintf( stderr, "%s:%u: g_mkdir_with_parents: %s\n", __FUNCTION__, __LINE__, strerror( errno ) );
        return error;
    }

    x50ng->state = g_key_file_new();
    if ( NULL == x50ng->state ) {
        fprintf( stderr, "%s:%u: g_key_file_new: Out of memory\n", __FUNCTION__, __LINE__ );
        return -ENOMEM;
    }

    if ( !g_key_file_load_from_file( x50ng->state, filename, G_KEY_FILE_KEEP_COMMENTS, &gerror ) &&
         !g_error_matches( gerror, G_FILE_ERROR, G_FILE_ERROR_NOENT ) ) {
        fprintf( stderr, "%s:%u: g_key_file_load_from_file: %s\n", __FUNCTION__, __LINE__, gerror->message );
        g_key_file_free( x50ng->state );
        return -EIO;
    }

    result = 0;

    list_for_each_entry( module, &x50ng->modules, list )
    {
        error = module->load( module, x50ng->state );
        if ( error ) {
            if ( error == -EAGAIN )
                result = -EAGAIN;
            else
                return error;
        }
    }

#ifdef DEBUG_X50NG_MODULES
    {
        extern unsigned char* phys_ram_base;

        printf( "%s: phys_ram_base: %p\n", __FUNCTION__, phys_ram_base );
        printf( "\t%02x %02x %02x %02x %02x %02x %02x %02x\n", phys_ram_base[ 0 ], phys_ram_base[ 1 ], phys_ram_base[ 2 ],
                phys_ram_base[ 3 ], phys_ram_base[ 4 ], phys_ram_base[ 5 ], phys_ram_base[ 6 ], phys_ram_base[ 7 ] );
    }
#endif

    return result;
}

int x50ng_modules_save( x50ng_t* x50ng )
{
    x50ng_module_t* module;
    GError* gerror = NULL;
    gchar* data;
    gsize length;
    int error;
    int fd;
    const char* filename = g_build_filename( opt.datadir, STATE_FILE_NAME, NULL );

#ifdef DEBUG_X50NG_MODULES
    printf( "%s:%u:\n", __FUNCTION__, __LINE__ );
#endif

    list_for_each_entry( module, &x50ng->modules, list )
    {
        error = module->save( module, x50ng->state );
        if ( error )
            return error;
    }

    data = g_key_file_to_data( x50ng->state, &length, &gerror );
    if ( NULL == data ) {
        fprintf( stderr, "%s:%u: g_key_file_to_data: %s\n", __FUNCTION__, __LINE__, gerror->message );
        return -ENOMEM;
    }

    fd = open( filename, O_WRONLY | O_CREAT | O_TRUNC, 0644 );
    if ( fd < 0 ) {
        error = -errno;
        fprintf( stderr, "%s:%u: open %s: %s\n", __FUNCTION__, __LINE__, filename, strerror( errno ) );
        g_free( data );
        return error;
    }

    if ( write( fd, data, length ) != length ) {
        error = -errno;
        fprintf( stderr, "%s:%u: write %s: %s\n", __FUNCTION__, __LINE__, filename, strerror( errno ) );
        close( fd );
        g_free( data );
        return error;
    }

    close( fd );
    g_free( data );

    return 0;
}

int x50ng_module_register( x50ng_module_t* module )
{
    x50ng_t* x50ng = module->x50ng;

#ifdef DEBUG_X50NG_MODULES
    printf( "%s:%u: %s\n", __FUNCTION__, __LINE__, module->name );
#endif

    list_add_tail( &module->list, &x50ng->modules );

    return 0;
}

int x50ng_module_unregister( x50ng_module_t* module )
{
#ifdef DEBUG_X50NG_MODULES
    printf( "%s:%u: %s\n", __FUNCTION__, __LINE__, module->name );
#endif

    list_del( &module->list );

    return 0;
}

int x50ng_module_get_filename( x50ng_module_t* module, GKeyFile* key, const char* name, char* reset, char** valuep, char** path )
{
    int error;

    error = x50ng_module_get_string( module, key, name, reset, valuep );

    if ( g_path_is_absolute( *valuep ) ) {
        *path = g_strdup( *valuep );
        return error;
    }

    *path = g_build_filename( opt.datadir, *valuep, NULL );
    if ( NULL == path ) {
        fprintf( stderr, "%s: %s:%u: Out of memory\n", module->name, __FUNCTION__, __LINE__ );
        g_free( *valuep );
        *valuep = NULL;
    }

    return error;
}

int x50ng_module_set_filename( x50ng_module_t* module, GKeyFile* key, const char* name, const char* value )
{
    return x50ng_module_set_string( module, key, name, value );
}

int x50ng_module_get_int( x50ng_module_t* module, GKeyFile* key, const char* name, int reset, int* valuep )
{
    return x50ng_module_get_u32( module, key, name, reset, ( uint32_t* )valuep );
}

int x50ng_module_set_int( x50ng_module_t* module, GKeyFile* key, const char* name, int value )
{
    char data[ 16 ];

    snprintf( data, sizeof( data ), "%d", value );

    g_key_file_set_value( key, module->name, name, data );

    return 0;
}

int x50ng_module_get_uint( x50ng_module_t* module, GKeyFile* key, const char* name, unsigned int reset, unsigned int* valuep )
{
    return x50ng_module_get_u32( module, key, name, reset, valuep );
}

int x50ng_module_set_uint( x50ng_module_t* module, GKeyFile* key, const char* name, unsigned int value )
{
    char data[ 16 ];

    snprintf( data, sizeof( data ), "%u", value );

    g_key_file_set_value( key, module->name, name, data );

    return 0;
}

int x50ng_module_get_u32( x50ng_module_t* module, GKeyFile* key, const char* name, uint32_t reset, uint32_t* valuep )
{
    GError* gerror = NULL;
    char *data, *end;
    uint32_t value;

    data = g_key_file_get_value( key, module->name, name, &gerror );
    if ( NULL == data ) {
        fprintf( stderr, "%s: %s:%u: key \"%s\" not found\n", module->name, __FUNCTION__, __LINE__, name );
        *valuep = reset;
        return -EAGAIN;
    }

    value = strtoul( data, &end, 0 );
    if ( ( end == data ) || ( *end != '\0' ) ) {
        *valuep = reset;
        g_free( data );
        return -EAGAIN;
    }

    *valuep = value;

    g_free( data );
    return 0;
}

int x50ng_module_set_u32( x50ng_module_t* module, GKeyFile* key, const char* name, uint32_t value )
{
    char data[ 16 ];

    snprintf( data, sizeof( data ), "0x%08x", value );

    g_key_file_set_value( key, module->name, name, data );

    return 0;
}

int x50ng_module_set_u64( x50ng_module_t* module, GKeyFile* key, const char* name, uint64_t value )
{
    char data[ 32 ];

    snprintf( data, sizeof( data ), "0x%016" PRIx64 "", value );

    g_key_file_set_value( key, module->name, name, data );

    return 0;
}

int x50ng_module_get_u64( x50ng_module_t* module, GKeyFile* key, const char* name, uint64_t reset, uint64_t* valuep )
{
    GError* gerror = NULL;
    char *data, *end;
    uint64_t value;

    data = g_key_file_get_value( key, module->name, name, &gerror );
    if ( NULL == data ) {
        fprintf( stderr, "%s: %s:%u: key \"%s\" not found\n", module->name, __FUNCTION__, __LINE__, name );
        *valuep = reset;
        return -EAGAIN;
    }

    value = strtoull( data, &end, 0 );
    if ( ( end == data ) || ( *end != '\0' ) ) {
        *valuep = reset;
        g_free( data );
        return -EAGAIN;
    }

    *valuep = value;

    g_free( data );
    return 0;
}

int x50ng_module_get_string( x50ng_module_t* module, GKeyFile* key, const char* name, char* reset, char** valuep )
{
    GError* gerror = NULL;
    char* data;

    data = g_key_file_get_value( key, module->name, name, &gerror );
    if ( NULL == data ) {
        fprintf( stderr, "%s: %s:%u: key \"%s\" not found\n", module->name, __FUNCTION__, __LINE__, name );
        *valuep = g_strdup( reset );
        return -EAGAIN;
    }

    *valuep = data;
    return 0;
}

int x50ng_module_set_string( x50ng_module_t* module, GKeyFile* key, const char* name, const char* value )
{
    g_key_file_set_value( key, module->name, name, value );

    return 0;
}

int x50ng_module_open_rodata( x50ng_module_t* module, const char* name, char** path )
{
    x50ng_t* x50ng = module->x50ng;
    int fd;
    int error;

    *path = g_build_filename( name, NULL );
    if ( opt.verbose )
        fprintf( stderr, "reading %s\n", *path );
    if ( NULL == *path ) {
        fprintf( stderr, "%s: %s:%u: Out of memory\n", module->name, __FUNCTION__, __LINE__ );
        return -ENOMEM;
    }
    fd = open( *path, O_RDONLY );

    if ( fd < 0 && ( errno == EACCES || errno == ENOENT ) ) {
        g_free( *path );

        *path = g_build_filename( x50ng->progpath, name, NULL );
        if ( opt.verbose )
            fprintf( stderr, "reading %s\n", *path );
        if ( NULL == *path ) {
            fprintf( stderr, "%s: %s:%u: Out of memory\n", module->name, __FUNCTION__, __LINE__ );
            return -ENOMEM;
        }
        fd = open( *path, O_RDONLY );
    }

    if ( fd < 0 && ( errno == EACCES || errno == ENOENT ) ) {
        g_free( *path );

        *path = g_build_filename( opt.datadir, name, NULL );
        if ( opt.verbose )
            fprintf( stderr, "reading %s\n", *path );
        if ( NULL == *path ) {
            fprintf( stderr, "%s: %s:%u: Out of memory\n", module->name, __FUNCTION__, __LINE__ );
            return -ENOMEM;
        }
        fd = open( *path, O_RDONLY );
    }

#ifdef X50NG_DATADIR
    if ( fd < 0 && ( errno == EACCES || errno == ENOENT ) ) {
        g_free( *path );

        *path = g_build_filename( X50NG_DATADIR, name, NULL );
        if ( opt.verbose )
            fprintf( stderr, "reading %s\n", *path );
        if ( NULL == *path ) {
            fprintf( stderr, "%s: %s:%u: Out of memory\n", module->name, __FUNCTION__, __LINE__ );
            return -ENOMEM;
        }
        fd = open( *path, O_RDONLY );
    }
#endif

    if ( fd < 0 ) {
        error = -errno;
        fprintf( stderr, "%s: %s:%u: open %s: %s\n", module->name, __FUNCTION__, __LINE__, *path, strerror( errno ) );
        g_free( *path );
        *path = NULL;
        return error;
    }

    return fd;
}

int x50ng_module_init( x50ng_t* x50ng, const char* name, int ( *init )( x50ng_module_t* ), int ( *exit )( x50ng_module_t* ),
                       int ( *reset )( x50ng_module_t*, x50ng_reset_t ), int ( *load )( x50ng_module_t*, GKeyFile* ),
                       int ( *save )( x50ng_module_t*, GKeyFile* ), void* user_data, x50ng_module_t** modulep )
{
    x50ng_module_t* module;

    module = malloc( sizeof( x50ng_module_t ) );
    if ( NULL == module ) {
        fprintf( stderr, "%s: %s:%u: Out of memory\n", name, __FUNCTION__, __LINE__ );
        return -1;
    }
    memset( module, 0, sizeof( x50ng_module_t ) );

    module->name = name;

    module->init = init;
    module->exit = exit;
    module->reset = reset;
    module->load = load;
    module->save = save;

    module->user_data = user_data;

    //	module->mutex = g_mutex_new();
    module->x50ng = x50ng;

    *modulep = module;
    return 0;
}
