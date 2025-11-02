#ifndef _S3C2410_LCD_H
#  define _S3C2410_LCD_H 1

#  include <stdint.h>

#  include "../types.h"

#  include "s3c2410.h"

typedef struct s3c2410_lcd_t {
    uint32_t lcdcon1;
    uint32_t lcdcon2;
    uint32_t lcdcon3;
    uint32_t lcdcon4;
    uint32_t lcdcon5;
    uint32_t lcdsaddr1;
    uint32_t lcdsaddr2;
    uint32_t lcdsaddr3;
    uint32_t redlut;
    uint32_t greenlut;
    uint32_t bluelut;
    uint32_t dithmode;
    uint32_t tpal;
    uint32_t lcdintpnd;
    uint32_t lcdsrcpnd;
    uint32_t lcdintmsk;
    uint32_t lpcsel;
    uint32_t __unknown_68;

    unsigned int nr_regs;
    s3c2410_offset_t* regs;

    hdw_t* hdw_state;
} s3c2410_lcd_t;

extern int s3c2410_get_pixel_color( s3c2410_lcd_t* lcd, int x, int y );

extern int s3c2410_init_module_lcd( hdw_t* hdw_state );

#endif /* !(_S3C2410_LCD_H) */
