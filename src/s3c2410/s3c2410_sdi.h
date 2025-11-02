#ifndef _S3C2410_SDI_H
#  define _S3C2410_SDI_H 1

#  include <stdbool.h>

#  include "../types.h"

extern void s3c2410_sdi_unmount( hdw_t* hdw_state );
extern int s3c2410_sdi_mount( hdw_t* hdw_state, char* filename );
extern bool s3c2410_sdi_is_mounted( hdw_t* hdw_state );
extern void s3c2410_sdi_get_path( hdw_t* hdw_state, char** filename );

extern int s3c2410_init_module_sdi( hdw_t* hdw_state );

#endif /* !(_S3C2410_SDI_H) */
