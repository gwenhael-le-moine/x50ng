/*
 * QEMU System Emulator block driver
 *
 * Copyright (c) 2003 Fabrice Bellard
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
#ifndef BLOCK_INT_H
#define BLOCK_INT_H

#include "qemu-common.h"

#include "block.h"

struct BlockDriver {
    const char* format_name;
    int instance_size;
    int ( *bdrv_probe )( const uint8_t* buf, int buf_size, const char* filename );
    int ( *bdrv_open )( BlockDriverState* bs, const char* filename, int flags );
    int ( *bdrv_read )( BlockDriverState* bs, int64_t sector_num, uint8_t* buf, int nb_sectors );
    int ( *bdrv_write )( BlockDriverState* bs, int64_t sector_num, const uint8_t* buf, int nb_sectors );
    void ( *bdrv_close )( BlockDriverState* bs );
    int ( *bdrv_create )( const char* filename, int64_t total_sectors, const char* backing_file, int flags );
    void ( *bdrv_flush )( BlockDriverState* bs );
    int ( *bdrv_is_allocated )( BlockDriverState* bs, int64_t sector_num, int nb_sectors, int* pnum );
    int ( *bdrv_set_key )( BlockDriverState* bs, const char* key );
    int ( *bdrv_make_empty )( BlockDriverState* bs );

    const char* protocol_name;
    int ( *bdrv_pread )( BlockDriverState* bs, int64_t offset, uint8_t* buf, int count );
    int ( *bdrv_pwrite )( BlockDriverState* bs, int64_t offset, const uint8_t* buf, int count );
    int ( *bdrv_truncate )( BlockDriverState* bs, int64_t offset );
    int64_t ( *bdrv_getlength )( BlockDriverState* bs );
    int ( *bdrv_write_compressed )( BlockDriverState* bs, int64_t sector_num, const uint8_t* buf, int nb_sectors );

    int ( *bdrv_get_info )( BlockDriverState* bs, BlockDriverInfo* bdi );

    /* removable device specific */
    int ( *bdrv_is_inserted )( BlockDriverState* bs );
    int ( *bdrv_media_changed )( BlockDriverState* bs );
    int ( *bdrv_eject )( BlockDriverState* bs, int eject_flag );
    int ( *bdrv_set_locked )( BlockDriverState* bs, int locked );

    struct BlockDriver* next;
};

struct BlockDriverState {
    int64_t total_sectors; /* if we are reading a disk image, give its
                              size in sectors */
    int read_only;         /* if true, the media is read only */
    int removable;         /* if true, the media can be removed */
    int locked;            /* if true, the media cannot temporarily be ejected */
    int encrypted;         /* if true, the media is encrypted */
    /* event callback when inserting/removing */
    void ( *change_cb )( void* opaque );
    void* change_opaque;

    BlockDriver* drv; /* NULL means no media */
    void* opaque;

    int boot_sector_enabled;
    uint8_t boot_sector_data[ 512 ];

    char filename[ 1024 ];
    char backing_file[ 1024 ]; /* if non zero, the image is a diff of
                                  this file image */
    int is_temporary;
    int media_changed;

    BlockDriverState* backing_hd;
    /* async read/write emulation */

    void* sync_aiocb;

    /* NOTE: the following infos are only hints for real hardware
       drivers. They are not used by the block driver */
    int cyls, heads, secs, translation;
    int type;
    char device_name[ 32 ];
    BlockDriverState* next;
};

void get_tmp_filename( char* filename, int size );

#endif /* BLOCK_INT_H */
