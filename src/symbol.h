/* $Id: symbol.h,v 1.4 2008/12/11 12:18:17 ecd Exp $
 */

#ifndef _X49GP_SYMBOL_H
#define _X49GP_SYMBOL_H 1

#include <math.h>

typedef struct {
	const cairo_path_data_t	*data;
	int			num_data;
} symbol_path_t;

typedef struct {
	double			x_advance;
	double			y_advance;
	double			llx;
	double			lly;
	double			urx;
	double			ury;
	double			prescale;
	double			postscale;
	const symbol_path_t	*path;
} x49gp_symbol_t;

#define SYMBOL_MOVE_TO(x, y)				\
	{ header:	{ CAIRO_PATH_MOVE_TO, 2 } },	\
	{ point:	{ x, y } }

#define SYMBOL_LINE_TO(x, y)				\
	{ header:	{ CAIRO_PATH_LINE_TO, 2 } },	\
	{ point:	{ x, y } }

#define SYMBOL_CURVE_TO(x1, y1, x2, y2, x3, y3)		\
	{ header:	{ CAIRO_PATH_CURVE_TO, 4 } },	\
	{ point:	{ x1, y1 } },			\
	{ point:	{ x2, y2 } },			\
	{ point:	{ x3, y3 } }

#define SYMBOL_CLOSE_PATH()				\
	{ header:	{ CAIRO_PATH_CLOSE_PATH, 1 } }

#define SYMBOL(name, x_advance, y_advance, llx, lly, urx, ury)		\
static const symbol_path_t symbol_##name##_path =			\
{									\
	symbol_##name##_path_data,					\
	sizeof(symbol_##name##_path_data) / sizeof(cairo_path_data_t)	\
};									\
									\
static const x49gp_symbol_t symbol_##name =				\
{									\
	x_advance, y_advance, llx, lly, urx, ury, 1.0, 1.0,		\
	&symbol_##name##_path						\
}

#define CONTROL(name, x_advance, y_advance, prescale, postscale)	\
static const x49gp_symbol_t symbol_##name =				\
{									\
	x_advance, y_advance, 0.0, 0.0, 0.0, 0.0,			\
	prescale, postscale, NULL					\
}

int symbol_lookup_glyph_by_name(const char *name, int namelen, gunichar *);

const x49gp_symbol_t *symbol_get_by_name(const char *name);
const x49gp_symbol_t *symbol_get_by_glyph(gunichar glyph);

#endif /* !(_X49GP_SYMBOL_H) */
