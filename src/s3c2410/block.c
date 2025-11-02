#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include "qemu-common.h"

#include "block.h"
#include "block_int.h"

#define SECTOR_BITS 9
#define SECTOR_SIZE ( 1 << SECTOR_BITS )

static BlockDriverState* bdrv_first;
static BlockDriver* first_drv;

#ifndef ENOMEDIUM
#  define ENOMEDIUM ENODEV
#endif

static void bdrv_close( BlockDriverState* bs );

static bool path_is_absolute( const char* path )
{
    const char* p;
#ifdef _WIN32
    /* specific case for names like: "\\.\d:" */
    if ( *path == '/' || *path == '\\' )
        return true;
#endif
    p = strchr( path, ':' );
    if ( p )
        p++;
    else
        p = path;
#ifdef _WIN32
    return ( *p == '/' || *p == '\\' );
#else
    return ( *p == '/' );
#endif
}

/* if filename is absolute, just copy it to dest. Otherwise, build a
   path to it by considering it is relative to base_path. URL are
   supported. */
static void path_combine( char* dest, int dest_size, const char* base_path, const char* filename )
{
    const char *p, *p1;
    int len;

    if ( dest_size <= 0 )
        return;
    if ( path_is_absolute( filename ) ) {
        pstrcpy( dest, dest_size, filename );
    } else {
        p = strchr( base_path, ':' );
        if ( p )
            p++;
        else
            p = base_path;
        p1 = strrchr( base_path, '/' );
#ifdef _WIN32
        {
            const char* p2;
            p2 = strrchr( base_path, '\\' );
            if ( !p1 || p2 > p1 )
                p1 = p2;
        }
#endif
        if ( p1 )
            p1++;
        else
            p1 = base_path;
        if ( p1 > p )
            p = p1;
        len = p - base_path;
        if ( len > dest_size - 1 )
            len = dest_size - 1;
        memcpy( dest, base_path, len );
        dest[ len ] = '\0';
        pstrcat( dest, dest_size, filename );
    }
}

#ifdef _WIN32
void get_tmp_filename( char* filename, int size )
{
    char temp_dir[ MAX_PATH ];

    GetTempPath( MAX_PATH, temp_dir );
    GetTempFileName( temp_dir, "x49", 0, filename );
}
#else
void get_tmp_filename( char* filename, int size )
{
    int fd;

    /* XXX: race condition possible */
    pstrcpy( filename, size, "/tmp/x50ng.XXXXXX" );
    fd = mkstemp( filename );
    close( fd );
}
#endif

#ifdef _WIN32
static int is_windows_drive_prefix( const char* filename )
{
    return ( ( ( filename[ 0 ] >= 'a' && filename[ 0 ] <= 'z' ) || ( filename[ 0 ] >= 'A' && filename[ 0 ] <= 'Z' ) ) &&
             filename[ 1 ] == ':' );
}

static bool is_windows_drive( const char* filename )
{
    if ( is_windows_drive_prefix( filename ) && filename[ 2 ] == '\0' )
        return true;

    if ( strstart( filename, "\\\\.\\", NULL ) || strstart( filename, "//./", NULL ) )
        return true;

    return false;
}
#endif

static BlockDriver* find_protocol( const char* filename )
{
    BlockDriver* drv1;
    char protocol[ 128 ];
    unsigned int len;
    const char* p;

#ifdef _WIN32
    if ( is_windows_drive( filename ) || is_windows_drive_prefix( filename ) )
        return &bdrv_raw;
#endif
    p = strchr( filename, ':' );
    if ( !p )
        return &bdrv_raw;
    len = p - filename;
    if ( len > sizeof( protocol ) - 1 )
        len = sizeof( protocol ) - 1;
    memcpy( protocol, filename, len );
    protocol[ len ] = '\0';
    for ( drv1 = first_drv; drv1 != NULL; drv1 = drv1->next ) {
#ifdef DEBUG_X50NG_BLOCK
        fprintf( stderr, "%s:%u: protocol '%s', drv->protocol_name '%s'\n", __func__, __LINE__, protocol, drv1->protocol_name );
#endif
        if ( drv1->protocol_name && !strcmp( drv1->protocol_name, protocol ) ) {
#ifdef DEBUG_X50NG_BLOCK
            fprintf( stderr, "%s:%u: protocol '%s', drv %p\n", __func__, __LINE__, protocol, drv1 );
#endif
            return drv1;
        }
    }
#ifdef DEBUG_X50NG_BLOCK
    fprintf( stderr, "%s:%u: protocol '%s', NULL\n", __func__, __LINE__, protocol );
#endif
    return NULL;
}

/* XXX: force raw format if block or character device ? It would
   simplify the BSD case */
static BlockDriver* find_image_format( const char* filename )
{
    int ret, score, score_max;
    BlockDriver *drv1, *drv;
    uint8_t buf[ 2048 ];
    BlockDriverState* bs;

    drv = find_protocol( filename );
    /* no need to test disk image formats for vvfat */
    if ( drv == &bdrv_vvfat )
        return drv;

    ret = bdrv_file_open( &bs, filename, BDRV_O_RDONLY );
    if ( ret < 0 )
        return NULL;
    ret = bdrv_pread( bs, 0, buf, sizeof( buf ) );
    bdrv_delete( bs );
    if ( ret < 0 ) {
        return NULL;
    }

    score_max = 0;
    for ( drv1 = first_drv; drv1 != NULL; drv1 = drv1->next ) {
        if ( drv1->bdrv_probe ) {
            score = drv1->bdrv_probe( buf, ret, filename );
            if ( score > score_max ) {
                score_max = score;
                drv = drv1;
            }
        }
    }
    return drv;
}

int bdrv_create( BlockDriver* drv, const char* filename, int64_t size_in_sectors, const char* backing_file, int flags )
{
    if ( !drv->bdrv_create )
        return -ENOTSUP;
    return drv->bdrv_create( filename, size_in_sectors, backing_file, flags );
}

static void bdrv_register( BlockDriver* bdrv )
{
    bdrv->next = first_drv;
    first_drv = bdrv;
}

/* create a new block device (by default it is empty) */
BlockDriverState* bdrv_new( const char* device_name )
{
    BlockDriverState **pbs, *bs;

    bs = qemu_mallocz( sizeof( BlockDriverState ) );
    if ( !bs )
        return NULL;
    pstrcpy( bs->device_name, sizeof( bs->device_name ), device_name );
    if ( device_name[ 0 ] != '\0' ) {
        /* insert at the end */
        pbs = &bdrv_first;
        while ( *pbs != NULL )
            pbs = &( *pbs )->next;
        *pbs = bs;
    }
    return bs;
}

/**
 * Truncate file to 'offset' bytes (needed only for file protocols)
 */
int bdrv_truncate( BlockDriverState* bs, int64_t offset )
{
    BlockDriver* drv = bs->drv;
    if ( !drv )
        return -ENOMEDIUM;
    if ( !drv->bdrv_truncate )
        return -ENOTSUP;
    return drv->bdrv_truncate( bs, offset );
}

/**
 * Length of a file in bytes. Return < 0 if error or unknown.
 */
int64_t bdrv_getlength( BlockDriverState* bs )
{
    BlockDriver* drv = bs->drv;
    if ( !drv )
        return -ENOMEDIUM;
    if ( !drv->bdrv_getlength ) {
        /* legacy mode */
        return bs->total_sectors * SECTOR_SIZE;
    }
    return drv->bdrv_getlength( bs );
}

int bdrv_file_open( BlockDriverState** pbs, const char* filename, int flags )
{
    BlockDriverState* bs;
    int ret;
#ifdef DEBUG_X50NG_BLOCK
    fprintf( stderr, "%s:%u: filename '%s'\n", __func__, __LINE__, filename );
#endif

    bs = bdrv_new( "" );
    if ( !bs )
        return -ENOMEM;
    ret = bdrv_open( bs, filename, flags | BDRV_O_FILE );
    if ( ret < 0 ) {
        bdrv_delete( bs );
#ifdef DEBUG_X50NG_BLOCK
        fprintf( stderr, "%s:%u: '%s': %d\n", __func__, __LINE__, filename, ret );
#endif
        return ret;
    }
    *pbs = bs;
#ifdef DEBUG_X50NG_BLOCK
    fprintf( stderr, "%s:%u: return 0\n", __func__, __LINE__ );
#endif
    return 0;
}

int bdrv_open( BlockDriverState* bs, const char* filename, int flags )
{
    int ret, open_flags;
    char backing_filename[ 1024 ];
    BlockDriver* drv = NULL;

#ifdef DEBUG_X50NG_BLOCK
    fprintf( stderr, "%s:%u: filename '%s'\n", __func__, __LINE__, filename );
#endif

    bs->read_only = 0;
    bs->is_temporary = 0;
    bs->encrypted = 0;

    pstrcpy( bs->filename, sizeof( bs->filename ), filename );
    if ( flags & BDRV_O_FILE ) {
        drv = find_protocol( filename );
        if ( !drv ) {
#ifdef DEBUG_X50NG_BLOCK
            fprintf( stderr, "%s:%u: drv: %p\n", __func__, __LINE__, drv );
#endif
            return -ENOENT;
        }
    } else {
        if ( !drv ) {
            drv = find_image_format( filename );
            if ( !drv ) {
#ifdef DEBUG_X50NG_BLOCK
                fprintf( stderr, "%s:%u: drv: %p\n", __func__, __LINE__, drv );
#endif
                return -1;
            }
        }
    }

#ifdef DEBUG_X50NG_BLOCK
    fprintf( stderr, "%s:%u: drv: %p\n", __func__, __LINE__, drv );
#endif
    bs->drv = drv;
    bs->opaque = qemu_mallocz( drv->instance_size );
    if ( bs->opaque == NULL && drv->instance_size > 0 ) {
#ifdef DEBUG_X50NG_BLOCK
        fprintf( stderr, "%s:%u: no opaque\n", __func__, __LINE__ );
#endif
        return -1;
    }
    /* Note: for compatibility, we open disk image files as RDWR, and
       RDONLY as fallback */
    if ( !( flags & BDRV_O_FILE ) )
        open_flags = BDRV_O_RDWR;
    else
        open_flags = flags & ~( BDRV_O_FILE );
    ret = drv->bdrv_open( bs, filename, open_flags );
#ifdef DEBUG_X50NG_BLOCK
    fprintf( stderr, "%s:%u: drv->bdrv_open: %d\n", __func__, __LINE__, ret );
#endif
    if ( ret == -EACCES && !( flags & BDRV_O_FILE ) ) {
        ret = drv->bdrv_open( bs, filename, BDRV_O_RDONLY );
        bs->read_only = 1;
    }
    if ( ret < 0 ) {
        qemu_free( bs->opaque );
        bs->opaque = NULL;
        bs->drv = NULL;
#ifdef DEBUG_X50NG_BLOCK
        fprintf( stderr, "%s:%u: return %d\n", __func__, __LINE__, ret );
#endif
        return ret;
    }
    if ( drv->bdrv_getlength ) {
        bs->total_sectors = bdrv_getlength( bs ) >> SECTOR_BITS;
    }
#ifndef _WIN32
    if ( bs->is_temporary ) {
        unlink( filename );
    }
#endif
    if ( bs->backing_file[ 0 ] != '\0' ) {
        /* if there is a backing file, use it */
        bs->backing_hd = bdrv_new( "" );
        if ( !bs->backing_hd ) {
fail:
            bdrv_close( bs );
#ifdef DEBUG_X50NG_BLOCK
            fprintf( stderr, "%s:%u: return -ENOMEM\n", __func__, __LINE__ );
#endif
            return -ENOMEM;
        }
#ifdef DEBUG_X50NG_BLOCK
        fprintf( stderr, "%s:%u: combine '%s' '%s'\n", __func__, __LINE__, filename, bs->backing_file );
#endif
        path_combine( backing_filename, sizeof( backing_filename ), filename, bs->backing_file );
#ifdef DEBUG_X50NG_BLOCK
        fprintf( stderr, "%s:%u: combine: '%s'\n", __func__, __LINE__, backing_filename );
#endif
        if ( bdrv_open( bs->backing_hd, backing_filename, 0 ) < 0 ) {
#ifdef DEBUG_X50NG_BLOCK
            fprintf( stderr, "%s:%u: backing fail\n", __func__, __LINE__ );
#endif
            goto fail;
        }
    }

    /* call the change callback */
    bs->media_changed = 1;
    if ( bs->change_cb )
        bs->change_cb( bs->change_opaque );

#ifdef DEBUG_X50NG_BLOCK
    fprintf( stderr, "%s:%u: return 0\n", __func__, __LINE__ );
#endif
    return 0;
}

static void bdrv_close( BlockDriverState* bs )
{
    if ( NULL == bs->drv )
        return;

    /* call the change callback */
    bs->media_changed = 1;
    if ( bs->change_cb )
        bs->change_cb( bs->change_opaque );

    if ( bs->backing_hd )
        bdrv_delete( bs->backing_hd );

    bs->drv->bdrv_close( bs );

#ifdef _WIN32
    if ( bs->is_temporary ) {
        unlink( bs->filename );
    }
#endif

    qemu_free( bs->opaque );
    bs->opaque = NULL;
    bs->drv = NULL;
}

void bdrv_delete( BlockDriverState* bs )
{
    /* XXX: remove the driver list */
    bdrv_close( bs );
    qemu_free( bs );
}

/* return < 0 if error. See bdrv_write() for the return codes */
int bdrv_read( BlockDriverState* bs, int64_t sector_num, uint8_t* buf, int nb_sectors )
{
    BlockDriver* drv = bs->drv;

    if ( !drv )
        return -ENOMEDIUM;

    if ( sector_num == 0 && bs->boot_sector_enabled && nb_sectors > 0 ) {
        memcpy( buf, bs->boot_sector_data, 512 );
        sector_num++;
        nb_sectors--;
        buf += 512;
        if ( nb_sectors == 0 )
            return 0;
    }
    if ( drv->bdrv_pread ) {
        int ret, len;

        len = nb_sectors * 512;
        ret = drv->bdrv_pread( bs, sector_num * 512, buf, len );
        if ( ret < 0 )
            return ret;
        else if ( ret != len )
            return -EINVAL;
        else
            return 0;
    } else {
        return drv->bdrv_read( bs, sector_num, buf, nb_sectors );
    }
}

/* Return < 0 if error. Important errors are:
  -EIO         generic I/O error (may happen for all errors)
  -ENOMEDIUM   No media inserted.
  -EINVAL      Invalid sector number or nb_sectors
  -EACCES      Trying to write a read-only device
*/
static int bdrv_write( BlockDriverState* bs, int64_t sector_num, const uint8_t* buf, int nb_sectors )
{
    BlockDriver* drv = bs->drv;

    if ( !bs->drv )
        return -ENOMEDIUM;
    if ( bs->read_only )
        return -EACCES;
    if ( sector_num == 0 && bs->boot_sector_enabled && nb_sectors > 0 ) {
        memcpy( bs->boot_sector_data, buf, 512 );
    }
    if ( drv->bdrv_pwrite ) {
        int ret, len;

        len = nb_sectors * 512;
        ret = drv->bdrv_pwrite( bs, sector_num * 512, buf, len );
        if ( ret < 0 )
            return ret;
        else if ( ret != len )
            return -EIO;
        else
            return 0;
    } else {
        return drv->bdrv_write( bs, sector_num, buf, nb_sectors );
    }
}

static int bdrv_pread_em( BlockDriverState* bs, int64_t offset, uint8_t* buf, int count1 )
{
    uint8_t tmp_buf[ SECTOR_SIZE ];
    int len, nb_sectors, count;
    int64_t sector_num;

    count = count1;
    /* first read to align to sector start */
    len = ( SECTOR_SIZE - offset ) & ( SECTOR_SIZE - 1 );
    if ( len > count )
        len = count;
    sector_num = offset >> SECTOR_BITS;
    if ( len > 0 ) {
        if ( bdrv_read( bs, sector_num, tmp_buf, 1 ) < 0 )
            return -EIO;
        memcpy( buf, tmp_buf + ( offset & ( SECTOR_SIZE - 1 ) ), len );
        count -= len;
        if ( count == 0 )
            return count1;
        sector_num++;
        buf += len;
    }

    /* read the sectors "in place" */
    nb_sectors = count >> SECTOR_BITS;
    if ( nb_sectors > 0 ) {
        if ( bdrv_read( bs, sector_num, buf, nb_sectors ) < 0 )
            return -EIO;
        sector_num += nb_sectors;
        len = nb_sectors << SECTOR_BITS;
        buf += len;
        count -= len;
    }

    /* add data from the last sector */
    if ( count > 0 ) {
        if ( bdrv_read( bs, sector_num, tmp_buf, 1 ) < 0 )
            return -EIO;
        memcpy( buf, tmp_buf, count );
    }
    return count1;
}

static int bdrv_pwrite_em( BlockDriverState* bs, int64_t offset, const uint8_t* buf, int count1 )
{
    uint8_t tmp_buf[ SECTOR_SIZE ];
    int len, nb_sectors, count;
    int64_t sector_num;

    count = count1;
    /* first write to align to sector start */
    len = ( SECTOR_SIZE - offset ) & ( SECTOR_SIZE - 1 );
    if ( len > count )
        len = count;
    sector_num = offset >> SECTOR_BITS;
    if ( len > 0 ) {
        if ( bdrv_read( bs, sector_num, tmp_buf, 1 ) < 0 )
            return -EIO;
        memcpy( tmp_buf + ( offset & ( SECTOR_SIZE - 1 ) ), buf, len );
        if ( bdrv_write( bs, sector_num, tmp_buf, 1 ) < 0 )
            return -EIO;
        count -= len;
        if ( count == 0 )
            return count1;
        sector_num++;
        buf += len;
    }

    /* write the sectors "in place" */
    nb_sectors = count >> SECTOR_BITS;
    if ( nb_sectors > 0 ) {
        if ( bdrv_write( bs, sector_num, buf, nb_sectors ) < 0 )
            return -EIO;
        sector_num += nb_sectors;
        len = nb_sectors << SECTOR_BITS;
        buf += len;
        count -= len;
    }

    /* add data from the last sector */
    if ( count > 0 ) {
        if ( bdrv_read( bs, sector_num, tmp_buf, 1 ) < 0 )
            return -EIO;
        memcpy( tmp_buf, buf, count );
        if ( bdrv_write( bs, sector_num, tmp_buf, 1 ) < 0 )
            return -EIO;
    }
    return count1;
}

/*
 * Read with byte offsets (needed only for file protocols)
 */
int bdrv_pread( BlockDriverState* bs, int64_t offset, void* buf1, int count1 )
{
    BlockDriver* drv = bs->drv;

    if ( !drv )
        return -ENOMEDIUM;
    if ( !drv->bdrv_pread )
        return bdrv_pread_em( bs, offset, buf1, count1 );
    return drv->bdrv_pread( bs, offset, buf1, count1 );
}

/*
 * Write with byte offsets (needed only for file protocols)
 */
int bdrv_pwrite( BlockDriverState* bs, int64_t offset, const void* buf1, int count1 )
{
    BlockDriver* drv = bs->drv;

    if ( !drv )
        return -ENOMEDIUM;
    if ( !drv->bdrv_pwrite )
        return bdrv_pwrite_em( bs, offset, buf1, count1 );
    return drv->bdrv_pwrite( bs, offset, buf1, count1 );
}

void bdrv_flush( BlockDriverState* bs )
{
    if ( bs->drv->bdrv_flush )
        bs->drv->bdrv_flush( bs );
    if ( bs->backing_hd )
        bdrv_flush( bs->backing_hd );
}

void bdrv_init( void )
{
    /* bdrv_register(&bdrv_raw); */
    /* bdrv_register(&bdrv_host_device); */
    bdrv_register( &bdrv_qcow );
    bdrv_register( &bdrv_vvfat );
}
