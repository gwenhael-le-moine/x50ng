/*
 * Block driver for RAW files
 *
 * Copyright (c) 2006 Fabrice Bellard
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdbool.h>

#include "qemu-common.h"

#include "block.h"
#include "block_int.h"
#include <assert.h>

#ifndef _WIN32

#  define QEMU_TOOL

#  ifdef CONFIG_COCOA
#    include <paths.h>
#    include <sys/param.h>
#    include <IOKit/IOKitLib.h>
#    include <IOKit/IOBSD.h>
#    include <IOKit/storage/IOMediaBSDClient.h>
#    include <IOKit/storage/IOMedia.h>
#    include <IOKit/storage/IOCDMedia.h>
// #include <IOKit/storage/IOCDTypes.h>
#    include <CoreFoundation/CoreFoundation.h>
#  endif

#  ifdef __sun__
#    define _POSIX_PTHREAD_SEMANTICS 1
#    include <signal.h>
#    include <sys/dkio.h>
#  endif
#  ifdef __linux__
#    include <sys/ioctl.h>
#    include <linux/cdrom.h>
#    include <linux/fd.h>
#  endif
#  ifdef __FreeBSD__
#    include <sys/disk.h>
#  endif

// #define DEBUG_FLOPPY

#  define FTYPE_FILE 0
#  define FTYPE_CD 1
#  define FTYPE_FD 2

/* if the FD is not accessed during that time (in ms), we try to
   reopen it to see if the disk has been changed */
#  define FD_OPEN_TIMEOUT 1000

typedef struct BDRVRawState {
    int fd;
    int type;
#  if defined( __linux__ )
    /* linux floppy specific */
    int fd_open_flags;
    int64_t fd_open_time;
    int64_t fd_error_time;
    int fd_got_error;
    int fd_media_changed;
#  endif
} BDRVRawState;

static int fd_open( BlockDriverState* bs );

static int raw_open( BlockDriverState* bs, const char* filename, int flags )
{
    BDRVRawState* s = bs->opaque;
    int fd, open_flags, ret;

    open_flags = O_BINARY;
    if ( ( flags & BDRV_O_ACCESS ) == O_RDWR ) {
        open_flags |= O_RDWR;
    } else {
        open_flags |= O_RDONLY;
        bs->read_only = 1;
    }
    if ( flags & BDRV_O_CREAT )
        open_flags |= O_CREAT | O_TRUNC;

    s->type = FTYPE_FILE;

    fd = open( filename, open_flags, 0644 );
    if ( fd < 0 ) {
        ret = -errno;
        if ( ret == -EROFS )
            ret = -EACCES;
        return ret;
    }
    s->fd = fd;
    return 0;
}

/* XXX: use host sector size if necessary with:
#ifdef DIOCGSECTORSIZE
        {
            unsigned int sectorsize = 512;
            if (!ioctl(fd, DIOCGSECTORSIZE, &sectorsize) &&
                sectorsize > bufsize)
                bufsize = sectorsize;
        }
#endif
#ifdef CONFIG_COCOA
        u_int32_t   blockSize = 512;
        if ( !ioctl( fd, DKIOCGETBLOCKSIZE, &blockSize ) && blockSize > bufsize) {
            bufsize = blockSize;
        }
#endif
*/

static int raw_pread( BlockDriverState* bs, int64_t offset, uint8_t* buf, int count )
{
    BDRVRawState* s = bs->opaque;
    int ret;

    ret = fd_open( bs );
    if ( ret < 0 )
        return ret;

    lseek( s->fd, offset, SEEK_SET );
    ret = read( s->fd, buf, count );
    return ret;
}

static int raw_pwrite( BlockDriverState* bs, int64_t offset, const uint8_t* buf, int count )
{
    BDRVRawState* s = bs->opaque;
    int ret;

    ret = fd_open( bs );
    if ( ret < 0 )
        return ret;

    lseek( s->fd, offset, SEEK_SET );
    ret = write( s->fd, buf, count );
    return ret;
}

static void raw_close( BlockDriverState* bs )
{
    BDRVRawState* s = bs->opaque;
    if ( s->fd >= 0 ) {
        close( s->fd );
        s->fd = -1;
    }
}

static int raw_truncate( BlockDriverState* bs, int64_t offset )
{
    BDRVRawState* s = bs->opaque;
    if ( s->type != FTYPE_FILE )
        return -ENOTSUP;
    if ( ftruncate( s->fd, offset ) < 0 )
        return -errno;
    return 0;
}

static int64_t raw_getlength( BlockDriverState* bs )
{
    BDRVRawState* s = bs->opaque;
    int fd = s->fd;
    int64_t size;
#  ifdef _BSD
    struct stat sb;
#  endif
#  ifdef __sun__
    struct dk_minfo minfo;
    int rv;
#  endif
    int ret;

    ret = fd_open( bs );
    if ( ret < 0 )
        return ret;

#  ifdef _BSD
    if ( !fstat( fd, &sb ) && ( S_IFCHR & sb.st_mode ) ) {
#    ifdef DIOCGMEDIASIZE
        if ( ioctl( fd, DIOCGMEDIASIZE, ( off_t* )&size ) )
#    endif
#    ifdef CONFIG_COCOA
            size = LONG_LONG_MAX;
#    else
        size = lseek( fd, 0LL, SEEK_END );
#    endif
    } else
#  endif
#  ifdef __sun__
        /*
         * use the DKIOCGMEDIAINFO ioctl to read the size.
         */
        rv = ioctl( fd, DKIOCGMEDIAINFO, &minfo );
    if ( rv != -1 ) {
        size = minfo.dki_lbsize * minfo.dki_capacity;
    } else /* there are reports that lseek on some devices
              fails, but irc discussion said that contingency
              on contingency was overkill */
#  endif
    {
        size = lseek( fd, 0, SEEK_END );
    }
    return size;
}

static int raw_create( const char* filename, int64_t total_size, const char* backing_file, int flags )
{
    int fd;
    int result = 0;

    if ( flags || backing_file )
        return -ENOTSUP;

    fd = open( filename, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0644 );
    if ( fd < 0 ) {
        result = -errno;
    } else {
        if ( ftruncate( fd, total_size * 512 ) ) {
            result = -errno;
        }
        if ( close( fd ) != 0 ) {
            result = -errno;
        }
    }
    return result;
}

static void raw_flush( BlockDriverState* bs )
{
    BDRVRawState* s = bs->opaque;
    fsync( s->fd );
}

BlockDriver bdrv_raw = {
    "raw",
    sizeof( BDRVRawState ),
    NULL, /* no probe for protocols */
    raw_open,
    NULL,
    NULL,
    raw_close,
    raw_create,
    raw_flush,
    .protocol_name = "file",
    .bdrv_pread = raw_pread,
    .bdrv_pwrite = raw_pwrite,
    .bdrv_truncate = raw_truncate,
    .bdrv_getlength = raw_getlength,
};

/***********************************************/
/* host device */

#  ifdef CONFIG_COCOA
static kern_return_t FindEjectableCDMedia( io_iterator_t* mediaIterator );
static kern_return_t GetBSDPath( io_iterator_t mediaIterator, char* bsdPath, CFIndex maxPathSize );

kern_return_t FindEjectableCDMedia( io_iterator_t* mediaIterator )
{
    kern_return_t kernResult;
    mach_port_t masterPort;
    CFMutableDictionaryRef classesToMatch;

    kernResult = IOMasterPort( MACH_PORT_NULL, &masterPort );
    if ( KERN_SUCCESS != kernResult ) {
        printf( "IOMasterPort returned %d\n", kernResult );
    }

    classesToMatch = IOServiceMatching( kIOCDMediaClass );
    if ( classesToMatch == NULL ) {
        printf( "IOServiceMatching returned a NULL dictionary.\n" );
    } else {
        CFDictionarySetValue( classesToMatch, CFSTR( kIOMediaEjectableKey ), kCFBooleanTrue );
    }
    kernResult = IOServiceGetMatchingServices( masterPort, classesToMatch, mediaIterator );
    if ( KERN_SUCCESS != kernResult ) {
        printf( "IOServiceGetMatchingServices returned %d\n", kernResult );
    }

    return kernResult;
}

kern_return_t GetBSDPath( io_iterator_t mediaIterator, char* bsdPath, CFIndex maxPathSize )
{
    io_object_t nextMedia;
    kern_return_t kernResult = KERN_FAILURE;
    *bsdPath = '\0';
    nextMedia = IOIteratorNext( mediaIterator );
    if ( nextMedia ) {
        CFTypeRef bsdPathAsCFString;
        bsdPathAsCFString = IORegistryEntryCreateCFProperty( nextMedia, CFSTR( kIOBSDNameKey ), kCFAllocatorDefault, 0 );
        if ( bsdPathAsCFString ) {
            size_t devPathLength;
            strcpy( bsdPath, _PATH_DEV );
            strcat( bsdPath, "r" );
            devPathLength = strlen( bsdPath );
            if ( CFStringGetCString( bsdPathAsCFString, bsdPath + devPathLength, maxPathSize - devPathLength, kCFStringEncodingASCII ) ) {
                kernResult = KERN_SUCCESS;
            }
            CFRelease( bsdPathAsCFString );
        }
        IOObjectRelease( nextMedia );
    }

    return kernResult;
}

#  endif

static int hdev_open( BlockDriverState* bs, const char* filename, int flags )
{
    BDRVRawState* s = bs->opaque;
    int fd, open_flags, ret;

#  ifdef CONFIG_COCOA
    if ( strstart( filename, "/dev/cdrom", NULL ) ) {
        kern_return_t kernResult;
        io_iterator_t mediaIterator;
        char bsdPath[ MAXPATHLEN ];
        int fd;

        kernResult = FindEjectableCDMedia( &mediaIterator );
        kernResult = GetBSDPath( mediaIterator, bsdPath, sizeof( bsdPath ) );

        if ( bsdPath[ 0 ] != '\0' ) {
            strcat( bsdPath, "s0" );
            /* some CDs don't have a partition 0 */
            fd = open( bsdPath, O_RDONLY | O_BINARY | O_LARGEFILE );
            if ( fd < 0 ) {
                bsdPath[ strlen( bsdPath ) - 1 ] = '1';
            } else {
                close( fd );
            }
            filename = bsdPath;
        }

        if ( mediaIterator )
            IOObjectRelease( mediaIterator );
    }
#  endif
    open_flags = O_BINARY;
    if ( ( flags & BDRV_O_ACCESS ) == O_RDWR ) {
        open_flags |= O_RDWR;
    } else {
        open_flags |= O_RDONLY;
        bs->read_only = 1;
    }

    s->type = FTYPE_FILE;
#  if defined( __linux__ )
    if ( strstart( filename, "/dev/cd", NULL ) ) {
        /* open will not fail even if no CD is inserted */
        open_flags |= O_NONBLOCK;
        s->type = FTYPE_CD;
    } else if ( strstart( filename, "/dev/fd", NULL ) ) {
        s->type = FTYPE_FD;
        s->fd_open_flags = open_flags;
        /* open will not fail even if no floppy is inserted */
        open_flags |= O_NONBLOCK;
    }
#  endif
    fd = open( filename, open_flags, 0644 );
    if ( fd < 0 ) {
        ret = -errno;
        if ( ret == -EROFS )
            ret = -EACCES;
        return ret;
    }
    s->fd = fd;
#  if defined( __linux__ )
    /* close fd so that we can reopen it as needed */
    if ( s->type == FTYPE_FD ) {
        close( s->fd );
        s->fd = -1;
        s->fd_media_changed = 1;
    }
#  endif
    return 0;
}

static int fd_open( BlockDriverState* bs ) { return 0; }

#  if defined( __linux__ )

static int raw_is_inserted( BlockDriverState* bs )
{
    BDRVRawState* s = bs->opaque;
    int ret;

    switch ( s->type ) {
        case FTYPE_CD:
            ret = ioctl( s->fd, CDROM_DRIVE_STATUS, CDSL_CURRENT );
            if ( ret == CDS_DISC_OK )
                return 1;
            else
                return 0;
            break;
        case FTYPE_FD:
            ret = fd_open( bs );
            return ( ret >= 0 );
        default:
            return 1;
    }
}

/* currently only used by fdc.c, but a CD version would be good too */
static int raw_media_changed( BlockDriverState* bs )
{
    BDRVRawState* s = bs->opaque;

    switch ( s->type ) {
        case FTYPE_FD:
            {
                int ret;
                /* XXX: we do not have a true media changed indication. It
                   does not work if the floppy is changed without trying
                   to read it */
                fd_open( bs );
                ret = s->fd_media_changed;
                s->fd_media_changed = 0;
#    ifdef DEBUG_FLOPPY
                printf( "Floppy changed=%d\n", ret );
#    endif
                return ret;
            }
        default:
            return -ENOTSUP;
    }
}

static int raw_eject( BlockDriverState* bs, int eject_flag )
{
    BDRVRawState* s = bs->opaque;

    switch ( s->type ) {
        case FTYPE_CD:
            if ( eject_flag ) {
                if ( ioctl( s->fd, CDROMEJECT, NULL ) < 0 )
                    perror( "CDROMEJECT" );
            } else {
                if ( ioctl( s->fd, CDROMCLOSETRAY, NULL ) < 0 )
                    perror( "CDROMEJECT" );
            }
            break;
        case FTYPE_FD:
            {
                int fd;
                if ( s->fd >= 0 ) {
                    close( s->fd );
                    s->fd = -1;
                }
                fd = open( bs->filename, s->fd_open_flags | O_NONBLOCK );
                if ( fd >= 0 ) {
                    if ( ioctl( fd, FDEJECT, 0 ) < 0 )
                        perror( "FDEJECT" );
                    close( fd );
                }
            }
            break;
        default:
            return -ENOTSUP;
    }
    return 0;
}

static int raw_set_locked( BlockDriverState* bs, int locked )
{
    BDRVRawState* s = bs->opaque;

    switch ( s->type ) {
        case FTYPE_CD:
            if ( ioctl( s->fd, CDROM_LOCKDOOR, locked ) < 0 ) {
                /* Note: an error can happen if the distribution automatically
                   mounts the CD-ROM */
                //        perror("CDROM_LOCKDOOR");
            }
            break;
        default:
            return -ENOTSUP;
    }
    return 0;
}

#  else

static int raw_is_inserted( BlockDriverState* bs ) { return 1; }

static int raw_media_changed( BlockDriverState* bs ) { return -ENOTSUP; }

static int raw_eject( BlockDriverState* bs, int eject_flag ) { return -ENOTSUP; }

static int raw_set_locked( BlockDriverState* bs, int locked ) { return -ENOTSUP; }

#  endif /* !linux */

BlockDriver bdrv_host_device = {
    "host_device",
    sizeof( BDRVRawState ),
    NULL, /* no probe for protocols */
    hdev_open,
    NULL,
    NULL,
    raw_close,
    NULL,
    raw_flush,
    .bdrv_pread = raw_pread,
    .bdrv_pwrite = raw_pwrite,
    .bdrv_getlength = raw_getlength,

    /* removable device support */
    .bdrv_is_inserted = raw_is_inserted,
    .bdrv_media_changed = raw_media_changed,
    .bdrv_eject = raw_eject,
    .bdrv_set_locked = raw_set_locked,
};

#else /* _WIN32 */

/* XXX: use another file ? */
#  include <winioctl.h>

#  define FTYPE_FILE 0
#  define FTYPE_CD 1
#  define FTYPE_HARDDISK 2

typedef struct BDRVRawState {
    HANDLE hfile;
    int type;
    char drive_path[ 16 ]; /* format: "d:\" */
} BDRVRawState;

int qemu_ftruncate64( int fd, int64_t length )
{
    LARGE_INTEGER li;
    LONG high;
    HANDLE h;
    BOOL res;

    if ( ( GetVersion() & 0x80000000UL ) && ( length >> 32 ) != 0 )
        return -1;

    h = ( HANDLE )_get_osfhandle( fd );

    /* get current position, ftruncate do not change position */
    li.HighPart = 0;
    li.LowPart = SetFilePointer( h, 0, &li.HighPart, FILE_CURRENT );
    if ( li.LowPart == 0xffffffffUL && GetLastError() != NO_ERROR )
        return -1;

    high = length >> 32;
    if ( !SetFilePointer( h, ( DWORD )length, &high, FILE_BEGIN ) )
        return -1;
    res = SetEndOfFile( h );

    /* back to old position */
    SetFilePointer( h, li.LowPart, &li.HighPart, FILE_BEGIN );
    return res ? 0 : -1;
}

static int set_sparse( int fd )
{
    DWORD returned;
    return ( int )DeviceIoControl( ( HANDLE )_get_osfhandle( fd ), FSCTL_SET_SPARSE, NULL, 0, NULL, 0, &returned, NULL );
}

static int raw_open( BlockDriverState* bs, const char* filename, int flags )
{
    BDRVRawState* s = bs->opaque;
    int access_flags, create_flags;
    DWORD overlapped;

    s->type = FTYPE_FILE;

    if ( ( flags & BDRV_O_ACCESS ) == O_RDWR ) {
        access_flags = GENERIC_READ | GENERIC_WRITE;
    } else {
        access_flags = GENERIC_READ;
    }
    if ( flags & BDRV_O_CREAT ) {
        create_flags = CREATE_ALWAYS;
    } else {
        create_flags = OPEN_EXISTING;
    }
#  ifdef QEMU_TOOL
    overlapped = FILE_ATTRIBUTE_NORMAL;
#  else
    overlapped = FILE_FLAG_OVERLAPPED;
#  endif
    s->hfile = CreateFile( filename, access_flags, FILE_SHARE_READ, NULL, create_flags, overlapped, NULL );
    if ( s->hfile == INVALID_HANDLE_VALUE ) {
        int err = GetLastError();

        if ( err == ERROR_ACCESS_DENIED )
            return -EACCES;
        return -1;
    }
    return 0;
}

static int raw_pread( BlockDriverState* bs, int64_t offset, uint8_t* buf, int count )
{
    BDRVRawState* s = bs->opaque;
    OVERLAPPED ov;
    DWORD ret_count;
    int ret;

    memset( &ov, 0, sizeof( ov ) );
    ov.Offset = offset;
    ov.OffsetHigh = offset >> 32;
    ret = ReadFile( s->hfile, buf, count, &ret_count, &ov );
    if ( !ret ) {
        ret = GetOverlappedResult( s->hfile, &ov, &ret_count, true );
        if ( !ret )
            return -EIO;
        else
            return ret_count;
    }
    return ret_count;
}

static int raw_pwrite( BlockDriverState* bs, int64_t offset, const uint8_t* buf, int count )
{
    BDRVRawState* s = bs->opaque;
    OVERLAPPED ov;
    DWORD ret_count;
    int ret;

    memset( &ov, 0, sizeof( ov ) );
    ov.Offset = offset;
    ov.OffsetHigh = offset >> 32;
    ret = WriteFile( s->hfile, buf, count, &ret_count, &ov );
    if ( !ret ) {
        ret = GetOverlappedResult( s->hfile, &ov, &ret_count, true );
        if ( !ret )
            return -EIO;
        else
            return ret_count;
    }
    return ret_count;
}

static void raw_flush( BlockDriverState* bs )
{
    BDRVRawState* s = bs->opaque;
    FlushFileBuffers( s->hfile );
}

static void raw_close( BlockDriverState* bs )
{
    BDRVRawState* s = bs->opaque;
    CloseHandle( s->hfile );
}

static int raw_truncate( BlockDriverState* bs, int64_t offset )
{
    BDRVRawState* s = bs->opaque;
    DWORD low, high;

    low = offset;
    high = offset >> 32;
    if ( !SetFilePointer( s->hfile, low, &high, FILE_BEGIN ) )
        return -EIO;
    if ( !SetEndOfFile( s->hfile ) )
        return -EIO;
    return 0;
}

static int64_t raw_getlength( BlockDriverState* bs )
{
    BDRVRawState* s = bs->opaque;
    LARGE_INTEGER l;
    ULARGE_INTEGER available, total, total_free;
    DISK_GEOMETRY dg;
    DWORD count;
    BOOL status;

    switch ( s->type ) {
        case FTYPE_FILE:
            l.LowPart = GetFileSize( s->hfile, &l.HighPart );
            if ( l.LowPart == 0xffffffffUL && GetLastError() != NO_ERROR )
                return -EIO;
            break;
        case FTYPE_CD:
            if ( !GetDiskFreeSpaceEx( s->drive_path, &available, &total, &total_free ) )
                return -EIO;
            l.QuadPart = total.QuadPart;
            break;
        case FTYPE_HARDDISK:
            status = DeviceIoControl( s->hfile, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0, &dg, sizeof( dg ), &count, NULL );
            if ( status != false ) {
                l.QuadPart = dg.Cylinders.QuadPart * dg.TracksPerCylinder * dg.SectorsPerTrack * dg.BytesPerSector;
            }
            break;
        default:
            return -EIO;
    }
    return l.QuadPart;
}

static int raw_create( const char* filename, int64_t total_size, const char* backing_file, int flags )
{
    int fd;

    if ( flags || backing_file )
        return -ENOTSUP;

    fd = open( filename, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0644 );
    if ( fd < 0 )
        return -EIO;
    set_sparse( fd );
    ftruncate( fd, total_size * 512 );
    close( fd );
    return 0;
}

BlockDriver bdrv_raw = {
    "raw",
    sizeof( BDRVRawState ),
    NULL, /* no probe for protocols */
    raw_open,
    NULL,
    NULL,
    raw_close,
    raw_create,
    raw_flush,

    .protocol_name = "file",
    .bdrv_pread = raw_pread,
    .bdrv_pwrite = raw_pwrite,
    .bdrv_truncate = raw_truncate,
    .bdrv_getlength = raw_getlength,
};

/***********************************************/
/* host device */

static int find_cdrom( char* cdrom_name, int cdrom_name_size )
{
    char drives[ 256 ], *pdrv = drives;
    UINT type;

    memset( drives, 0, sizeof( drives ) );
    GetLogicalDriveStrings( sizeof( drives ), drives );
    while ( pdrv[ 0 ] != '\0' ) {
        type = GetDriveType( pdrv );
        switch ( type ) {
            case DRIVE_CDROM:
                snprintf( cdrom_name, cdrom_name_size, "\\\\.\\%c:", pdrv[ 0 ] );
                return 0;
                break;
        }
        pdrv += lstrlen( pdrv ) + 1;
    }
    return -1;
}

static int find_device_type( BlockDriverState* bs, const char* filename )
{
    BDRVRawState* s = bs->opaque;
    UINT type;
    const char* p;

    if ( strstart( filename, "\\\\.\\", &p ) || strstart( filename, "//./", &p ) ) {
        if ( stristart( p, "PhysicalDrive", NULL ) )
            return FTYPE_HARDDISK;
        snprintf( s->drive_path, sizeof( s->drive_path ), "%c:\\", p[ 0 ] );
        type = GetDriveType( s->drive_path );
        if ( type == DRIVE_CDROM )
            return FTYPE_CD;
        else
            return FTYPE_FILE;
    } else {
        return FTYPE_FILE;
    }
}

static int hdev_open( BlockDriverState* bs, const char* filename, int flags )
{
    BDRVRawState* s = bs->opaque;
    int access_flags, create_flags;
    DWORD overlapped;
    char device_name[ 64 ];

    if ( strstart( filename, "/dev/cdrom", NULL ) ) {
        if ( find_cdrom( device_name, sizeof( device_name ) ) < 0 )
            return -ENOENT;
        filename = device_name;
    } else {
        /* transform drive letters into device name */
        if ( ( ( filename[ 0 ] >= 'a' && filename[ 0 ] <= 'z' ) || ( filename[ 0 ] >= 'A' && filename[ 0 ] <= 'Z' ) ) &&
             filename[ 1 ] == ':' && filename[ 2 ] == '\0' ) {
            snprintf( device_name, sizeof( device_name ), "\\\\.\\%c:", filename[ 0 ] );
            filename = device_name;
        }
    }
    s->type = find_device_type( bs, filename );

    if ( ( flags & BDRV_O_ACCESS ) == O_RDWR ) {
        access_flags = GENERIC_READ | GENERIC_WRITE;
    } else {
        access_flags = GENERIC_READ;
    }
    create_flags = OPEN_EXISTING;

#  ifdef QEMU_TOOL
    overlapped = FILE_ATTRIBUTE_NORMAL;
#  else
    overlapped = FILE_FLAG_OVERLAPPED;
#  endif
    s->hfile = CreateFile( filename, access_flags, FILE_SHARE_READ, NULL, create_flags, overlapped, NULL );
    if ( s->hfile == INVALID_HANDLE_VALUE ) {
        int err = GetLastError();

        if ( err == ERROR_ACCESS_DENIED )
            return -EACCES;
        return -1;
    }
    return 0;
}

BlockDriver bdrv_host_device = {
    "host_device",
    sizeof( BDRVRawState ),
    NULL, /* no probe for protocols */
    hdev_open,
    NULL,
    NULL,
    raw_close,
    NULL,
    raw_flush,

    .bdrv_pread = raw_pread,
    .bdrv_pwrite = raw_pwrite,
    .bdrv_getlength = raw_getlength,
};
#endif /* _WIN32 */
