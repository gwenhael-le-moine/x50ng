#include <stdio.h>
#include <stdlib.h>

#include "options.h"
#include "x50ng.h"
#include "ui.h"
#include "ui_inner.h"
#include "s3c2410.h"

/* #include "gdbstub.h" */
#include "types.h"

void tui_update_lcd( x50ng_t* x50ng )
{
    s3c2410_lcd_t* lcd = x50ng->s3c2410_lcd;

    if ( !( lcd->lcdcon1 & 1 ) )
        return;

    fprintf( stderr, "// TODO\n" );
    exit( 1 );
}

void tui_events_handling_step( x50ng_t* _x50ng )
{
    fprintf( stderr, "// TODO\n" );
    exit( 1 );
}

void tui_refresh_lcd( x50ng_t* x50ng )
{
    fprintf( stderr, "// TODO\n" );
    exit( 1 );
}

void tui_init( x50ng_t* x50ng )
{
    fprintf( stderr, "// TODO\n" );
    exit( 1 );
}
