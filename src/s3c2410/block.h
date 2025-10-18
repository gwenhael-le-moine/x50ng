#ifndef BLOCK_H
#  define BLOCK_H 1

#  include <stdint.h>

#  include "qemu-common.h"

#  define BDRV_O_RDONLY 0x0000
#  define BDRV_O_RDWR 0x0002
#  define BDRV_O_ACCESS 0x0003
#  define BDRV_O_CREAT 0x0004 /* create an empty file */
#  define BDRV_O_SNAPSHOT                                                                                                                  \
      0x0008 /* open the file read only and save                                                                                           \
                writes in a snapshot */
#  define BDRV_O_FILE                                                                                                                      \
      0x0010 /* open as a raw file (do not try to                                                                                          \
                use a disk image format on top of                                                                                          \
                it (default for                                                                                                            \
                bdrv_file_open()) */

typedef struct BlockDriver BlockDriver;
typedef struct SnapshotInfo QEMUSnapshotInfo;
typedef struct BlockDriverInfo BlockDriverInfo;
typedef struct BlockDriverAIOCB BlockDriverAIOCB;
typedef void BlockDriverCompletionFunc( void* opaque, int ret );

extern BlockDriver bdrv_raw;
extern BlockDriver bdrv_host_device;
extern BlockDriver bdrv_qcow;
extern BlockDriver bdrv_vvfat;

void bdrv_init( void );
int bdrv_create( BlockDriver* drv, const char* filename, int64_t size_in_sectors, const char* backing_file, int flags );
BlockDriverState* bdrv_new( const char* device_name );
void bdrv_delete( BlockDriverState* bs );
int bdrv_file_open( BlockDriverState** pbs, const char* filename, int flags );
int bdrv_open( BlockDriverState* bs, const char* filename, int flags );

int bdrv_read( BlockDriverState* bs, int64_t sector_num, uint8_t* buf, int nb_sectors );
int bdrv_pread( BlockDriverState* bs, int64_t offset, void* buf, int count );
int bdrv_pwrite( BlockDriverState* bs, int64_t offset, const void* buf, int count );

int bdrv_truncate( BlockDriverState* bs, int64_t offset );
int64_t bdrv_getlength( BlockDriverState* bs );
void bdrv_flush( BlockDriverState* bs );

/* timers */

typedef struct QEMUClock QEMUClock;
typedef void QEMUTimerCB( void* opaque );

/* The real time clock should be used only for stuff which does not
   change the virtual machine state, as it is run even if the virtual
   machine is stopped. The real time clock has a frequency of 1000
   Hz. */
extern QEMUClock* rt_clock;

int64_t qemu_get_clock( QEMUClock* clock );

QEMUTimer* qemu_new_timer( QEMUClock* clock, QEMUTimerCB* cb, void* opaque );
void qemu_del_timer( QEMUTimer* ts );
void qemu_mod_timer( QEMUTimer* ts, int64_t expire_time );
int qemu_timer_pending( QEMUTimer* ts );

extern int64_t ticks_per_sec;

struct BlockDriverInfo {
    /* in bytes, 0 if irrelevant */
    int cluster_size;
    /* offset at which the VM state can be saved (0 if not possible) */
    int64_t vm_state_offset;
};

#endif /* !(BLOCK_H) */
