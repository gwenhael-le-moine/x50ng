/* $Id: symbol.c,v 1.4 2008/12/11 12:18:17 ecd Exp $
 */

#include <stdlib.h>
#include <string.h>

#include <glib.h>
#include <cairo.h>

#include <symbol.h>

static const cairo_path_data_t symbol_square_path_data[] =
{
	SYMBOL_MOVE_TO(  0.100,  0.100 ),
	SYMBOL_LINE_TO(  0.500,  0.000 ),
	SYMBOL_LINE_TO(  0.000,  0.500 ),
	SYMBOL_LINE_TO( -0.500,  0.000 ),
	SYMBOL_CLOSE_PATH()
};

SYMBOL(square, 0.7, 0.0, 0.1, 0.1, 0.6, 0.6);


static const cairo_path_data_t symbol_triangleup_path_data[] =
{
	SYMBOL_MOVE_TO(  0.100,  0.000 ),
	SYMBOL_LINE_TO(  0.800,  0.000 ),
	SYMBOL_LINE_TO( -0.400,  0.693 ),
	SYMBOL_CLOSE_PATH()
};

SYMBOL(triangleup, 1.0, 0.0, 0.1, 0.0, 0.9, 0.693);


static const cairo_path_data_t symbol_triangleright_path_data[] =
{
	SYMBOL_MOVE_TO(  0.100,  0.000 ),
	SYMBOL_LINE_TO(  0.000,  0.800 ),
	SYMBOL_LINE_TO(  0.693, -0.400 ),
	SYMBOL_CLOSE_PATH()
};

SYMBOL(triangleright, 0.893, 0.0, 0.1, 0.0, 0.793, 0.8);


static const cairo_path_data_t symbol_arrowleftdblfull_path_data[] =
{
	SYMBOL_MOVE_TO(  0.100,  0.370 ),
	SYMBOL_LINE_TO(  0.652,  0.500 ),
	SYMBOL_LINE_TO(  0.000, -0.250 ),
	SYMBOL_LINE_TO(  1.000,  0.000 ),
	SYMBOL_LINE_TO(  0.000, -0.500 ),
	SYMBOL_LINE_TO( -1.000,  0.000 ),
	SYMBOL_LINE_TO(  0.000, -0.250 ),
	SYMBOL_CLOSE_PATH()
};

SYMBOL(arrowleftdblfull, 1.852, 0.0, 0.1, -0.13, 1.752, 0.87);

static const cairo_path_data_t symbol_uparrowleft_path_data[] =
{
	SYMBOL_MOVE_TO(  0.100,  0.500 ),
	SYMBOL_LINE_TO(  0.600,  0.200 ),
	SYMBOL_LINE_TO(  0.000, -0.150 ),
	SYMBOL_LINE_TO(  0.500,  0.000 ),
	SYMBOL_LINE_TO(  0.000, -0.550 ),
	SYMBOL_LINE_TO( -0.100,  0.000 ),
	SYMBOL_LINE_TO(  0.000,  0.450 ),
	SYMBOL_LINE_TO( -0.400,  0.000 ),
	SYMBOL_LINE_TO(  0.000, -0.150 ),
	SYMBOL_CLOSE_PATH()
};

SYMBOL(uparrowleft, 1.3, 0.0, 0.1, 0.0, 1.2, 0.7);


static const cairo_path_data_t symbol_uparrowright_path_data[] =
{
	SYMBOL_MOVE_TO(  1.200,  0.500 ),
	SYMBOL_LINE_TO( -0.600,  0.200 ),
	SYMBOL_LINE_TO(  0.000, -0.150 ),
	SYMBOL_LINE_TO( -0.500,  0.000 ),
	SYMBOL_LINE_TO(  0.000, -0.550 ),
	SYMBOL_LINE_TO(  0.100,  0.000 ),
	SYMBOL_LINE_TO(  0.000,  0.450 ),
	SYMBOL_LINE_TO(  0.400,  0.000 ),
	SYMBOL_LINE_TO(  0.000, -0.150 ),
	SYMBOL_CLOSE_PATH()
};

SYMBOL(uparrowright, 1.3, 0.0, 0.1, 0.0, 1.2, 0.7);


static const cairo_path_data_t symbol_tick_path_data[] =
{
	SYMBOL_MOVE_TO(  0.100,  0.400 ),
	SYMBOL_LINE_TO(  0.000,  0.450 ),
	SYMBOL_LINE_TO(  0.150,  0.000 ),
	SYMBOL_LINE_TO(  0.000, -0.450 ),
	SYMBOL_CLOSE_PATH()
};

SYMBOL(tick, 0.35, 0.0, 0.1, 0.4, 0.25, 0.85);


static const cairo_path_data_t symbol_radical_path_data[] =
{
	SYMBOL_MOVE_TO(  0.000,  0.500 ),
	SYMBOL_LINE_TO(  0.214,  0.100 ),
	SYMBOL_LINE_TO(  0.213, -0.456 ),
	SYMBOL_LINE_TO(  0.229,  0.856 ),
	SYMBOL_LINE_TO(  0.077,  0.000 ),
	SYMBOL_LINE_TO(  0.000, -0.100 ),
	SYMBOL_LINE_TO( -0.281, -1.050 ),
	SYMBOL_LINE_TO( -0.287,  0.616 ),
	SYMBOL_LINE_TO( -0.123, -0.057 ),
	SYMBOL_CLOSE_PATH()
};

SYMBOL(radical, 0.733, 0.0, 0.0, -0.15, 0.733, 1.0);


static const cairo_path_data_t symbol_overscore_path_data[] =
{
	SYMBOL_MOVE_TO(   0.000,  1.000 ),
	SYMBOL_LINE_TO(   0.900,  0.000 ),
	SYMBOL_LINE_TO(   0.000, -0.100 ),
	SYMBOL_LINE_TO(  -0.900,  0.000 ),
	SYMBOL_CLOSE_PATH()
};

SYMBOL(overscore, 0.8, 0.0, 0.0, 0.9, 0.9, 1.0);


static const cairo_path_data_t symbol_minus_path_data[] =
{
	SYMBOL_MOVE_TO(   0.050,  0.312 ),
	SYMBOL_LINE_TO(   0.000,  0.118 ),
	SYMBOL_LINE_TO(   0.500,  0.000 ),
	SYMBOL_LINE_TO(   0.000, -0.118 ),
	SYMBOL_CLOSE_PATH()
};

SYMBOL(minus, 0.6, 0.0, 0.05, 0.312, 0.55, 0.430);


static const cairo_path_data_t symbol_divide_path_data[] =
{
	SYMBOL_MOVE_TO(   0.050,  0.312 ),
	SYMBOL_LINE_TO(   0.000,  0.118 ),
	SYMBOL_LINE_TO(   0.500,  0.000 ),
	SYMBOL_LINE_TO(   0.000, -0.118 ),
	SYMBOL_CLOSE_PATH(),
	SYMBOL_MOVE_TO(   0.180, -0.108 ),
	SYMBOL_LINE_TO(   0.135,  0.000 ),
	SYMBOL_LINE_TO(   0.000, -0.135 ),
	SYMBOL_LINE_TO(  -0.135,  0.000 ),
	SYMBOL_CLOSE_PATH(),
	SYMBOL_MOVE_TO(   0.000,  0.334 ),
	SYMBOL_LINE_TO(   0.000,  0.135 ),
	SYMBOL_LINE_TO(   0.135,  0.000 ),
	SYMBOL_LINE_TO(   0.000, -0.135 ),
	SYMBOL_CLOSE_PATH()
};

SYMBOL(divide, 0.6, 0.0, 0.05, 0.069, 0.55, 0.673);


static const cairo_path_data_t symbol_divisionslash_path_data[] =
{
	SYMBOL_MOVE_TO(   0.050,  0.000 ),
	SYMBOL_LINE_TO(   0.345,  0.739 ),
	SYMBOL_LINE_TO(   0.130,  0.000 ),
	SYMBOL_LINE_TO(  -0.345, -0.739 ),
	SYMBOL_CLOSE_PATH()
};

SYMBOL(divisionslash, 0.575, 0.000, 0.050, 0.000, 0.525, 0.739);


CONTROL(beginsuperscript, 0.0, 0.5, 1.0, 0.8);
CONTROL(endsuperscript, 0.0, 0.5, 1.25, 1.0);

CONTROL(kern_m1, -0.1, 0.0, 1.0, 1.0);
CONTROL(kern_m2, -0.2, 0.0, 1.0, 1.0);
CONTROL(kern_m3, -0.3, 0.0, 1.0, 1.0);
CONTROL(kern_m4, -0.4, 0.0, 1.0, 1.0);
CONTROL(kern_m5, -0.5, 0.0, 1.0, 1.0);
CONTROL(kern_m6, -0.6, 0.0, 1.0, 1.0);
CONTROL(kern_m7, -0.7, 0.0, 1.0, 1.0);
CONTROL(kern_m8, -0.8, 0.0, 1.0, 1.0);
CONTROL(kern_m9, -0.9, 0.0, 1.0, 1.0);


typedef struct {
	const char		*name;
	const x49gp_symbol_t	*symbol;
} symbol_name_t;

static const symbol_name_t symbol_names[] =
{
	{ ".notdef",		&symbol_square },
	{ "arrowleftdblfull",	&symbol_arrowleftdblfull },
	{ "divide",		&symbol_divide },
	{ "divisionslash",	&symbol_divisionslash },
	{ "minus",		&symbol_minus },
	{ "overscore",		&symbol_overscore },
	{ "radical",		&symbol_radical },
	{ "square",		&symbol_square },
	{ "tick",		&symbol_tick },
	{ "triangleright",	&symbol_triangleright },
	{ "triangleup",		&symbol_triangleup },
	{ "uparrowleft",	&symbol_uparrowleft },
	{ "uparrowright",	&symbol_uparrowright },
	{ "super",		&symbol_beginsuperscript },
	{ "/super",		&symbol_endsuperscript },
	{ "kern-1",		&symbol_kern_m1 },
	{ "kern-2",		&symbol_kern_m2 },
	{ "kern-3",		&symbol_kern_m3 },
	{ "kern-4",		&symbol_kern_m4 },
	{ "kern-5",		&symbol_kern_m5 },
	{ "kern-6",		&symbol_kern_m6 },
	{ "kern-7",		&symbol_kern_m7 },
	{ "kern-8",		&symbol_kern_m8 },
	{ "kern-9",		&symbol_kern_m9 },
	{ NULL, NULL }
};
#define NR_SYMBOLS	(sizeof(symbol_names) / sizeof(symbol_names[0]) - 1)

int
symbol_lookup_glyph_by_name(const char *name, int namelen, gunichar *glyph)
{
	const symbol_name_t *symname;
	int i = 0;

	/*
	 * Code symbols as Unicode Private Use from U+E000 on...
	 */

	symname = symbol_names;
	while (symname->name) {
		if ((strlen(symname->name) == namelen) &&
		    !strncmp(symname->name, name, namelen)) {
			if (glyph) {
				*glyph = 0xe000 + i;
			}
			return 1;
		}

		symname++;
		i++;
	}

	return 0;
}

const x49gp_symbol_t *
symbol_get_by_glyph(gunichar glyph)
{
	int index = glyph - 0xe000;

	if ((index >= 0) && (index < NR_SYMBOLS)) {
		return symbol_names[index].symbol;
	}

	return NULL;
}

const x49gp_symbol_t *
symbol_get_by_name(const char *name)
{
	gunichar glyph;

	if (symbol_lookup_glyph_by_name(name, strlen(name), &glyph)) {
		return symbol_get_by_glyph(glyph);
	}

	return NULL;
}
