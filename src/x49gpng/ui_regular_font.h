/* $Id: bitmap_font.h,v 1.5 2008/12/11 12:18:17 ecd Exp $
 */

#ifndef _REGULAR_FONT_H
#define _REGULAR_FONT_H 1

#include <cairo.h>

void regular_font_measure_text( cairo_t* cr, double size, double* x_bearing, double* y_bearing, double* width, double* height,
                               double* ascent, double* descent, ... );
void regular_font_draw_text( cairo_t* cr, GdkColor* color, double size, double line_width, int xoffset, int yoffset, ... );

#endif /* !(_REGULAR_FONT_H) */
