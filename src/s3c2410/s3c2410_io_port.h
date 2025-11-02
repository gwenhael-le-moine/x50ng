#ifndef _S3C2410_IO_PORT_H
#  define _S3C2410_IO_PORT_H 1

#  include <stdint.h>

#  include "../types.h"

extern void s3c2410_io_port_g_update( hdw_t* hdw_state, int column, int row, uint32_t new_state );
extern void s3c2410_io_port_f_set_bit( hdw_t* hdw_state, int n, uint32_t set );

extern int s3c2410_init_module_io_port( hdw_t* hdw_state );

#endif /* !(_S3C2410_IO_PORT_H) */
