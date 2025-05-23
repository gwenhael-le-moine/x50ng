/*
 * Block driver for the QCOW format
 *
 * Copyright (c) 2004-2006 Fabrice Bellard
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

#include "qemu-common.h"

#include "block.h"
#include "block_int.h"
#include <zlib.h>

/**************************************************************/
/* QEMU COW block driver with compression and encryption support */

#define QCOW_MAGIC ( ( 'Q' << 24 ) | ( 'F' << 16 ) | ( 'I' << 8 ) | 0xfb )
#define QCOW_VERSION 1

#define QCOW_CRYPT_NONE 0
#define QCOW_CRYPT_AES 1

#define QCOW_OFLAG_COMPRESSED ( 1LL << 63 )

typedef struct QCowHeader {
    uint32_t magic;
    uint32_t version;
    uint64_t backing_file_offset;
    uint32_t backing_file_size;
    uint32_t mtime;
    uint64_t size; /* in bytes */
    uint8_t cluster_bits;
    uint8_t l2_bits;
    uint32_t crypt_method;
    uint64_t l1_table_offset;
} QCowHeader;

#define L2_CACHE_SIZE 16

typedef struct BDRVQcowState {
    BlockDriverState* hd;
    int cluster_bits;
    int cluster_size;
    int cluster_sectors;
    int l2_bits;
    int l2_size;
    int l1_size;
    uint64_t cluster_offset_mask;
    uint64_t l1_table_offset;
    uint64_t* l1_table;
    uint64_t* l2_cache;
    uint64_t l2_cache_offsets[ L2_CACHE_SIZE ];
    uint32_t l2_cache_counts[ L2_CACHE_SIZE ];
    uint8_t* cluster_cache;
    uint8_t* cluster_data;
    uint64_t cluster_cache_offset;
} BDRVQcowState;

static int decompress_cluster( BDRVQcowState* s, uint64_t cluster_offset );

static int qcow_probe( const uint8_t* buf, int buf_size, const char* filename )
{
    const QCowHeader* cow_header = ( const void* )buf;

    if ( ( long unsigned int )buf_size >= sizeof( QCowHeader ) && be32_to_cpu( cow_header->magic ) == QCOW_MAGIC &&
         be32_to_cpu( cow_header->version ) == QCOW_VERSION )
        return 100;
    else
        return 0;
}

static int qcow_open( BlockDriverState* bs, const char* filename, int flags )
{
    BDRVQcowState* s = bs->opaque;
    int len, i, shift, ret;
    QCowHeader header;

    ret = bdrv_file_open( &s->hd, filename, flags );
    if ( ret < 0 )
        return ret;
    if ( bdrv_pread( s->hd, 0, &header, sizeof( header ) ) != sizeof( header ) )
        goto fail;
    be32_to_cpus( &header.magic );
    be32_to_cpus( &header.version );
    be64_to_cpus( &header.backing_file_offset );
    be32_to_cpus( &header.backing_file_size );
    be32_to_cpus( &header.mtime );
    be64_to_cpus( &header.size );
    be32_to_cpus( &header.crypt_method );
    be64_to_cpus( &header.l1_table_offset );

    if ( header.magic != QCOW_MAGIC || header.version != QCOW_VERSION )
        goto fail;
    if ( header.size <= 1 || header.cluster_bits < 9 )
        goto fail;
    if ( header.crypt_method > QCOW_CRYPT_AES )
        goto fail;
    s->cluster_bits = header.cluster_bits;
    s->cluster_size = 1 << s->cluster_bits;
    s->cluster_sectors = 1 << ( s->cluster_bits - 9 );
    s->l2_bits = header.l2_bits;
    s->l2_size = 1 << s->l2_bits;
    bs->total_sectors = header.size / 512;
    s->cluster_offset_mask = ( 1LL << ( 63 - s->cluster_bits ) ) - 1;

    /* read the level 1 table */
    shift = s->cluster_bits + s->l2_bits;
    s->l1_size = ( header.size + ( 1LL << shift ) - 1 ) >> shift;

    s->l1_table_offset = header.l1_table_offset;
    s->l1_table = qemu_malloc( s->l1_size * sizeof( uint64_t ) );
    if ( !s->l1_table )
        goto fail;
    if ( ( uint64_t )bdrv_pread( s->hd, s->l1_table_offset, s->l1_table, s->l1_size * sizeof( uint64_t ) ) !=
         s->l1_size * sizeof( uint64_t ) )
        goto fail;
    for ( i = 0; i < s->l1_size; i++ ) {
        be64_to_cpus( &s->l1_table[ i ] );
    }
    /* alloc L2 cache */
    s->l2_cache = qemu_malloc( s->l2_size * L2_CACHE_SIZE * sizeof( uint64_t ) );
    if ( !s->l2_cache )
        goto fail;
    s->cluster_cache = qemu_malloc( s->cluster_size );
    if ( !s->cluster_cache )
        goto fail;
    s->cluster_data = qemu_malloc( s->cluster_size );
    if ( !s->cluster_data )
        goto fail;
    s->cluster_cache_offset = -1;

    /* read the backing file name */
    if ( header.backing_file_offset != 0 ) {
        len = header.backing_file_size;
        if ( len > 1023 )
            len = 1023;
        if ( bdrv_pread( s->hd, header.backing_file_offset, bs->backing_file, len ) != len )
            goto fail;
        bs->backing_file[ len ] = '\0';
    }
    return 0;

fail:
    qemu_free( s->l1_table );
    qemu_free( s->l2_cache );
    qemu_free( s->cluster_cache );
    qemu_free( s->cluster_data );
    bdrv_delete( s->hd );
    return -1;
}

/* 'allocate' is:
 *
 * 0 to not allocate.
 *
 * 1 to allocate a normal cluster (for sector indexes 'n_start' to
 * 'n_end')
 *
 * 2 to allocate a compressed cluster of size
 * 'compressed_size'. 'compressed_size' must be > 0 and <
 * cluster_size
 *
 * return 0 if not allocated.
 */
static uint64_t get_cluster_offset( BlockDriverState* bs, uint64_t offset, int allocate, int compressed_size, int n_start, int n_end )
{
    BDRVQcowState* s = bs->opaque;
    int min_index, i, j, l1_index, l2_index;
    uint64_t l2_offset, *l2_table, cluster_offset, tmp;
    uint32_t min_count;
    int new_l2_table;

    l1_index = offset >> ( s->l2_bits + s->cluster_bits );
    l2_offset = s->l1_table[ l1_index ];
    new_l2_table = 0;
    if ( !l2_offset ) {
        if ( !allocate )
            return 0;
        /* allocate a new l2 entry */
        l2_offset = bdrv_getlength( s->hd );
        /* round to cluster size */
        l2_offset = ( l2_offset + s->cluster_size - 1 ) & ~( s->cluster_size - 1 );
        /* update the L1 entry */
        s->l1_table[ l1_index ] = l2_offset;
        tmp = cpu_to_be64( l2_offset );
        if ( bdrv_pwrite( s->hd, s->l1_table_offset + l1_index * sizeof( tmp ), &tmp, sizeof( tmp ) ) != sizeof( tmp ) )
            return 0;
        new_l2_table = 1;
    }
    for ( i = 0; i < L2_CACHE_SIZE; i++ ) {
        if ( l2_offset == s->l2_cache_offsets[ i ] ) {
            /* increment the hit count */
            if ( ++s->l2_cache_counts[ i ] == 0xffffffff ) {
                for ( j = 0; j < L2_CACHE_SIZE; j++ ) {
                    s->l2_cache_counts[ j ] >>= 1;
                }
            }
            l2_table = s->l2_cache + ( i << s->l2_bits );
            goto found;
        }
    }
    /* not found: load a new entry in the least used one */
    min_index = 0;
    min_count = 0xffffffff;
    for ( i = 0; i < L2_CACHE_SIZE; i++ ) {
        if ( s->l2_cache_counts[ i ] < min_count ) {
            min_count = s->l2_cache_counts[ i ];
            min_index = i;
        }
    }
    l2_table = s->l2_cache + ( min_index << s->l2_bits );
    if ( new_l2_table ) {
        memset( l2_table, 0, s->l2_size * sizeof( uint64_t ) );
        if ( ( uint64_t )bdrv_pwrite( s->hd, l2_offset, l2_table, s->l2_size * sizeof( uint64_t ) ) != s->l2_size * sizeof( uint64_t ) )
            return 0;
    } else {
        if ( ( uint64_t )bdrv_pread( s->hd, l2_offset, l2_table, s->l2_size * sizeof( uint64_t ) ) != s->l2_size * sizeof( uint64_t ) )
            return 0;
    }
    s->l2_cache_offsets[ min_index ] = l2_offset;
    s->l2_cache_counts[ min_index ] = 1;
found:
    l2_index = ( offset >> s->cluster_bits ) & ( s->l2_size - 1 );
    cluster_offset = be64_to_cpu( l2_table[ l2_index ] );
    if ( !cluster_offset || ( ( cluster_offset & QCOW_OFLAG_COMPRESSED ) && allocate == 1 ) ) {
        if ( !allocate )
            return 0;
        /* allocate a new cluster */
        if ( ( cluster_offset & QCOW_OFLAG_COMPRESSED ) && ( n_end - n_start ) < s->cluster_sectors ) {
            /* if the cluster is already compressed, we must
               decompress it in the case it is not completely
               overwritten */
            if ( decompress_cluster( s, cluster_offset ) < 0 )
                return 0;
            cluster_offset = bdrv_getlength( s->hd );
            cluster_offset = ( cluster_offset + s->cluster_size - 1 ) & ~( s->cluster_size - 1 );
            /* write the cluster content */
            if ( bdrv_pwrite( s->hd, cluster_offset, s->cluster_cache, s->cluster_size ) != s->cluster_size )
                return -1;
        } else {
            cluster_offset = bdrv_getlength( s->hd );
            if ( allocate == 1 ) {
                /* round to cluster size */
                cluster_offset = ( cluster_offset + s->cluster_size - 1 ) & ~( s->cluster_size - 1 );
                bdrv_truncate( s->hd, cluster_offset + s->cluster_size );
            } else {
                cluster_offset |= QCOW_OFLAG_COMPRESSED | ( uint64_t )compressed_size << ( 63 - s->cluster_bits );
            }
        }
        /* update L2 table */
        tmp = cpu_to_be64( cluster_offset );
        l2_table[ l2_index ] = tmp;
        if ( bdrv_pwrite( s->hd, l2_offset + l2_index * sizeof( tmp ), &tmp, sizeof( tmp ) ) != sizeof( tmp ) )
            return 0;
    }
    return cluster_offset;
}

static int qcow_is_allocated( BlockDriverState* bs, int64_t sector_num, int nb_sectors, int* pnum )
{
    BDRVQcowState* s = bs->opaque;
    int index_in_cluster, n;
    uint64_t cluster_offset;

    cluster_offset = get_cluster_offset( bs, sector_num << 9, 0, 0, 0, 0 );
    index_in_cluster = sector_num & ( s->cluster_sectors - 1 );
    n = s->cluster_sectors - index_in_cluster;
    if ( n > nb_sectors )
        n = nb_sectors;
    *pnum = n;
    return ( cluster_offset != 0 );
}

static int decompress_buffer( uint8_t* out_buf, int out_buf_size, const uint8_t* buf, int buf_size )
{
    z_stream strm1, *strm = &strm1;
    int ret, out_len;

    memset( strm, 0, sizeof( *strm ) );

    strm->next_in = ( uint8_t* )buf;
    strm->avail_in = buf_size;
    strm->next_out = out_buf;
    strm->avail_out = out_buf_size;

    ret = inflateInit2( strm, -12 );
    if ( ret != Z_OK )
        return -1;
    ret = inflate( strm, Z_FINISH );
    out_len = strm->next_out - out_buf;
    if ( ( ret != Z_STREAM_END && ret != Z_BUF_ERROR ) || out_len != out_buf_size ) {
        inflateEnd( strm );
        return -1;
    }
    inflateEnd( strm );
    return 0;
}

static int decompress_cluster( BDRVQcowState* s, uint64_t cluster_offset )
{
    int ret, csize;
    uint64_t coffset;

    coffset = cluster_offset & s->cluster_offset_mask;
    if ( s->cluster_cache_offset != coffset ) {
        csize = cluster_offset >> ( 63 - s->cluster_bits );
        csize &= ( s->cluster_size - 1 );
        ret = bdrv_pread( s->hd, coffset, s->cluster_data, csize );
        if ( ret != csize )
            return -1;
        if ( decompress_buffer( s->cluster_cache, s->cluster_size, s->cluster_data, csize ) < 0 ) {
            return -1;
        }
        s->cluster_cache_offset = coffset;
    }
    return 0;
}

static int qcow_read( BlockDriverState* bs, int64_t sector_num, uint8_t* buf, int nb_sectors )
{
    BDRVQcowState* s = bs->opaque;
    int ret, index_in_cluster, n;
    uint64_t cluster_offset;

    while ( nb_sectors > 0 ) {
        cluster_offset = get_cluster_offset( bs, sector_num << 9, 0, 0, 0, 0 );
        index_in_cluster = sector_num & ( s->cluster_sectors - 1 );
        n = s->cluster_sectors - index_in_cluster;
        if ( n > nb_sectors )
            n = nb_sectors;
        if ( !cluster_offset ) {
            if ( bs->backing_hd ) {
                /* read from the base image */
                ret = bdrv_read( bs->backing_hd, sector_num, buf, n );
                if ( ret < 0 )
                    return -1;
            } else {
                memset( buf, 0, 512 * n );
            }
        } else if ( cluster_offset & QCOW_OFLAG_COMPRESSED ) {
            if ( decompress_cluster( s, cluster_offset ) < 0 )
                return -1;
            memcpy( buf, s->cluster_cache + index_in_cluster * 512, 512 * n );
        } else {
            ret = bdrv_pread( s->hd, cluster_offset + index_in_cluster * 512, buf, n * 512 );
            if ( ret != n * 512 )
                return -1;
        }
        nb_sectors -= n;
        sector_num += n;
        buf += n * 512;
    }
    return 0;
}

static int qcow_write( BlockDriverState* bs, int64_t sector_num, const uint8_t* buf, int nb_sectors )
{
    BDRVQcowState* s = bs->opaque;
    int ret, index_in_cluster, n;
    uint64_t cluster_offset;

    while ( nb_sectors > 0 ) {
        index_in_cluster = sector_num & ( s->cluster_sectors - 1 );
        n = s->cluster_sectors - index_in_cluster;
        if ( n > nb_sectors )
            n = nb_sectors;
        cluster_offset = get_cluster_offset( bs, sector_num << 9, 1, 0, index_in_cluster, index_in_cluster + n );
        if ( !cluster_offset )
            return -1;
        {
            ret = bdrv_pwrite( s->hd, cluster_offset + index_in_cluster * 512, buf, n * 512 );
        }
        if ( ret != n * 512 )
            return -1;
        nb_sectors -= n;
        sector_num += n;
        buf += n * 512;
    }
    s->cluster_cache_offset = -1; /* disable compressed cache */
    return 0;
}

static void qcow_close( BlockDriverState* bs )
{
    BDRVQcowState* s = bs->opaque;
    qemu_free( s->l1_table );
    qemu_free( s->l2_cache );
    qemu_free( s->cluster_cache );
    qemu_free( s->cluster_data );
    bdrv_delete( s->hd );
}

static int qcow_create( const char* filename, int64_t total_size, const char* backing_file, int flags )
{
    int fd, header_size, backing_filename_len, l1_size, i, shift;
    QCowHeader header;
    uint64_t tmp;
    int ret;

    fd = open( filename, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0644 );
    if ( fd < 0 )
        return -1;
    memset( &header, 0, sizeof( header ) );
    header.magic = cpu_to_be32( QCOW_MAGIC );
    header.version = cpu_to_be32( QCOW_VERSION );
    header.size = cpu_to_be64( total_size * 512 );
    header_size = sizeof( header );
    backing_filename_len = 0;
    if ( backing_file ) {
        header.backing_file_offset = cpu_to_be64( header_size );
        backing_filename_len = strlen( backing_file );
        header.backing_file_size = cpu_to_be32( backing_filename_len );
        header_size += backing_filename_len;
        header.mtime = cpu_to_be32( 0 );
        header.cluster_bits = 9; /* 512 byte cluster to avoid copying
                                    unmodifyed sectors */
        header.l2_bits = 12;     /* 32 KB L2 tables */
    } else {
        header.cluster_bits = 12; /* 4 KB clusters */
        header.l2_bits = 9;       /* 4 KB L2 tables */
    }
    header_size = ( header_size + 7 ) & ~7;
    shift = header.cluster_bits + header.l2_bits;
    l1_size = ( ( total_size * 512 ) + ( 1LL << shift ) - 1 ) >> shift;

    header.l1_table_offset = cpu_to_be64( header_size );
    if ( flags ) {
        header.crypt_method = cpu_to_be32( QCOW_CRYPT_AES );
    } else {
        header.crypt_method = cpu_to_be32( QCOW_CRYPT_NONE );
    }

    /* write all the data */
    ret = write( fd, &header, sizeof( header ) );
    if ( ret != sizeof( header ) ) {
        ret = -1;
        goto exit;
    }

    if ( backing_file ) {
        ret = write( fd, backing_file, backing_filename_len );
        if ( ret != backing_filename_len ) {
            ret = -1;
            goto exit;
        }
    }
    lseek( fd, header_size, SEEK_SET );
    tmp = 0;
    for ( i = 0; i < l1_size; i++ ) {
        ret = write( fd, &tmp, sizeof( tmp ) );
        if ( ret != sizeof( tmp ) ) {
            ret = -1;
            goto exit;
        }
    }

    ret = 0;
exit:
    close( fd );
    return ret;
}

static int qcow_make_empty( BlockDriverState* bs )
{
    BDRVQcowState* s = bs->opaque;
    uint32_t l1_length = s->l1_size * sizeof( uint64_t );
    int ret;

    memset( s->l1_table, 0, l1_length );
    if ( bdrv_pwrite( s->hd, s->l1_table_offset, s->l1_table, l1_length ) < 0 )
        return -1;
    ret = bdrv_truncate( s->hd, s->l1_table_offset + l1_length );
    if ( ret < 0 )
        return ret;

    memset( s->l2_cache, 0, s->l2_size * L2_CACHE_SIZE * sizeof( uint64_t ) );
    memset( s->l2_cache_offsets, 0, L2_CACHE_SIZE * sizeof( uint64_t ) );
    memset( s->l2_cache_counts, 0, L2_CACHE_SIZE * sizeof( uint32_t ) );

    return 0;
}

/* XXX: put compressed sectors first, then all the cluster aligned
   tables to avoid losing bytes in alignment */
static int qcow_write_compressed( BlockDriverState* bs, int64_t sector_num, const uint8_t* buf, int nb_sectors )
{
    BDRVQcowState* s = bs->opaque;
    z_stream strm;
    int ret, out_len;
    uint8_t* out_buf;
    uint64_t cluster_offset;

    if ( nb_sectors != s->cluster_sectors )
        return -EINVAL;

    out_buf = qemu_malloc( s->cluster_size + ( s->cluster_size / 1000 ) + 128 );
    if ( !out_buf )
        return -1;

    /* best compression, small window, no zlib header */
    memset( &strm, 0, sizeof( strm ) );
    ret = deflateInit2( &strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, -12, 9, Z_DEFAULT_STRATEGY );
    if ( ret != 0 ) {
        qemu_free( out_buf );
        return -1;
    }

    strm.avail_in = s->cluster_size;
    strm.next_in = ( uint8_t* )buf;
    strm.avail_out = s->cluster_size;
    strm.next_out = out_buf;

    ret = deflate( &strm, Z_FINISH );
    if ( ret != Z_STREAM_END && ret != Z_OK ) {
        qemu_free( out_buf );
        deflateEnd( &strm );
        return -1;
    }
    out_len = strm.next_out - out_buf;

    deflateEnd( &strm );

    if ( ret != Z_STREAM_END || out_len >= s->cluster_size ) {
        /* could not compress: write normal cluster */
        qcow_write( bs, sector_num, buf, s->cluster_sectors );
    } else {
        cluster_offset = get_cluster_offset( bs, sector_num << 9, 2, out_len, 0, 0 );
        cluster_offset &= s->cluster_offset_mask;
        if ( bdrv_pwrite( s->hd, cluster_offset, out_buf, out_len ) != out_len ) {
            qemu_free( out_buf );
            return -1;
        }
    }

    qemu_free( out_buf );
    return 0;
}

static void qcow_flush( BlockDriverState* bs )
{
    BDRVQcowState* s = bs->opaque;
    bdrv_flush( s->hd );
}

static int qcow_get_info( BlockDriverState* bs, BlockDriverInfo* bdi )
{
    BDRVQcowState* s = bs->opaque;
    bdi->cluster_size = s->cluster_size;
    return 0;
}

BlockDriver bdrv_qcow = {
    "qcow",
    sizeof( BDRVQcowState ),
    qcow_probe,
    qcow_open,
    qcow_read,
    qcow_write,
    qcow_close,
    qcow_create,
    qcow_flush,
    qcow_is_allocated,
    NULL,
    qcow_make_empty,

    .bdrv_write_compressed = qcow_write_compressed,
    .bdrv_get_info = qcow_get_info,
};
