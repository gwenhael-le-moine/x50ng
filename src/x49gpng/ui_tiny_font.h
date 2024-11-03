/* $Id: bitmap_font.h,v 1.5 2008/12/11 12:18:17 ecd Exp $
 */

#ifndef _X49GP_TINY_FONT_H
#define _X49GP_TINY_FONT_H 1

typedef struct {
    const char* name;
    int width;
    int kern;
    int ascent;
    int descent;
    const unsigned char* bits;
} tiny_glyph_t;

typedef struct {
    int ascent;
    int descent;
    tiny_glyph_t glyphs[];
} tiny_font_t;

#define GLYPH( font, name )                                                                                                                \
    { #name,                                                                                                                               \
      font##_##name##_width - font##_##name##_x_hot,                                                                                       \
      -font##_##name##_x_hot,                                                                                                              \
      font##_##name##_y_hot + 1,                                                                                                           \
      font##_##name##_y_hot + 1 - font##_##name##_height,                                                                                  \
      font##_##name##_bits }

#define SPACE( name, width, kern ) { name, width, kern, 0, 0, NULL }

extern const tiny_font_t tiny_font;

void tiny_font_measure_text( const char* text, int* width, int* height, int* ascent, int* descent );
void tiny_font_draw_text( GdkDrawable* drawable, GdkColor* color, int x, int y, const char* text );

#endif /* !(_X49GP_TINY_FONT_H) */
