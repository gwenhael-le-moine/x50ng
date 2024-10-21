/* $Id: bitmap_font.h,v 1.5 2008/12/11 12:18:17 ecd Exp $
 */

#ifndef _X49GP_BITMAP_FONT_H
#define _X49GP_BITMAP_FONT_H 1

typedef struct {
	const char		*name;
	int			width;
	int			kern;
	int			ascent;
	int			descent;
	const unsigned char	*bits;
} bitmap_glyph_t;

typedef struct {
	int		ascent;
	int		descent;
	bitmap_glyph_t	glyphs[];
} bitmap_font_t;

#define GLYPH(font, name)						\
	{								\
		#name,							\
		font##_##name##_width - font##_##name##_x_hot,		\
		-font##_##name##_x_hot,					\
		font##_##name##_y_hot + 1,				\
		font##_##name##_y_hot + 1 - font##_##name##_height,	\
		font##_##name##_bits					\
	}

#define SPACE(name, width, kern) \
	{ name, width, kern, 0, 0, NULL }

extern const bitmap_font_t tiny_font;

#endif /* !(_X49GP_BITMAP_FONT_H) */
