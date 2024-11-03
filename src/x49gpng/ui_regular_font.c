#include <stdbool.h>
#include <stddef.h>

#include <gdk/gdk.h>
#include <cairo.h>

#include "options.h"
#include "symbol.h"
#include "glyphname.h"
#include "ui_regular_font.h"

static void _regular_font_symbol_path( cairo_t* cr, double size, double xoffset, double yoffset, const x49gp_symbol_t* symbol )
{
    const symbol_path_t* path;
    const cairo_path_data_t* data;

    path = symbol->path;
    if ( NULL == path )
        return;

    cairo_move_to( cr, xoffset, yoffset );

    for ( int i = 0; i < path->num_data; i += path->data[ i ].header.length ) {
        data = &path->data[ i ];

        switch ( data->header.type ) {
            case CAIRO_PATH_MOVE_TO:
                cairo_rel_move_to( cr, size * data[ 1 ].point.x, -size * data[ 1 ].point.y );
                break;
            case CAIRO_PATH_LINE_TO:
                cairo_rel_line_to( cr, size * data[ 1 ].point.x, -size * data[ 1 ].point.y );
                break;
            case CAIRO_PATH_CURVE_TO:
                cairo_rel_curve_to( cr, size * data[ 1 ].point.x, -size * data[ 1 ].point.y, size * data[ 2 ].point.x,
                                    -size * data[ 2 ].point.y, size * data[ 3 ].point.x, -size * data[ 3 ].point.y );
                break;
            case CAIRO_PATH_CLOSE_PATH:
                cairo_close_path( cr );
                break;
        }
    }
}

static bool _regular_font_lookup_glyph( const char* name, int namelen, gunichar* glyph )
{
    for ( int i = 0; i < NR_GLYPHNAMES; i++ ) {
        if ( ( strlen( x49gp_glyphs[ i ].name ) == namelen ) && !strncmp( x49gp_glyphs[ i ].name, name, namelen ) ) {
            if ( glyph )
                *glyph = x49gp_glyphs[ i ].unichar;

            return true;
        }
    }

    return false;
}

static int _regular_font_text_strlen( const char* text )
{
    const char *p, *q;
    char c;
    int namelen;
    int n = 0;

    p = text;
    while ( ( c = *p++ ) ) {
        if ( c != '\\' ) {
            n++;
            continue;
        }

        q = p;
        while ( *q ) {
            if ( ( *q == '\\' ) || ( *q == ' ' ) )
                break;
            q++;
        }
        if ( q == p ) {
            n++;
            p++;
            continue;
        }
        namelen = q - p;
        if ( *q == ' ' )
            q++;

        if ( symbol_lookup_glyph_by_name( p, namelen, NULL ) ) {
            p = q;
            n++;
            continue;
        }

        if ( _regular_font_lookup_glyph( p, namelen, NULL ) ) {
            p = q;
            n++;
            continue;
        }

        /*
         * Insert symbol .notdef here...
         */
        p = q;
        n++;
    }

    return n;
}

static int _regular_font_text_to_ucs4( const char* text, gunichar** ucs4p )
{
    const char *p, *q;
    gunichar glyph;
    gunichar* ucs4;
    char c;
    int namelen;
    int i = 0;
    int n = _regular_font_text_strlen( text );

    if ( n <= 0 )
        return n;

    ucs4 = malloc( n * sizeof( gunichar ) );

    p = text;
    while ( ( c = *p++ ) ) {
        if ( i == n ) {
            free( ucs4 );
            return -1;
        }

        if ( c != '\\' ) {
            ucs4[ i++ ] = c;
            continue;
        }

        q = p;
        while ( *q ) {
            if ( ( *q == '\\' ) || ( *q == ' ' ) )
                break;
            q++;
        }
        if ( q == p ) {
            ucs4[ i++ ] = *p++;
            continue;
        }
        namelen = q - p;
        if ( *q == ' ' )
            q++;

        if ( symbol_lookup_glyph_by_name( p, namelen, &glyph ) ) {
            ucs4[ i++ ] = glyph;
            p = q;
            continue;
        }

        if ( _regular_font_lookup_glyph( p, namelen, &glyph ) ) {
            ucs4[ i++ ] = glyph;
            p = q;
            continue;
        }

        /*
         * Insert symbol .notdef here...
         */
        ucs4[ i++ ] = 0xe000;
        p = q;
    }

    *ucs4p = ucs4;

    return n;
}

static void _regular_font_vtext_path( cairo_t* cr, double size, double x, double y, va_list ap )
{
    cairo_text_extents_t extents;
    cairo_font_weight_t weight;
    cairo_font_slant_t slant;
    const x49gp_symbol_t* symbol;
    const char* text;
    gunichar* ucs4;
    char out[ 8 ];
    int bytes;
    int len;

    slant = va_arg( ap, cairo_font_slant_t );
    weight = va_arg( ap, cairo_font_weight_t );
    text = va_arg( ap, const char* );

    cairo_select_font_face( cr, opt.font, slant, weight );
    cairo_set_font_size( cr, size );

    ucs4 = NULL;
    len = _regular_font_text_to_ucs4( text, &ucs4 );
    if ( len <= 0 )
        return;

    for ( int j = 0; j < len; j++ ) {
        if ( g_unichar_type( ucs4[ j ] ) == G_UNICODE_PRIVATE_USE ) {
            /*
             * Draw Symbol, Increment x...
             */
            symbol = symbol_get_by_glyph( ucs4[ j ] );
            if ( NULL == symbol )
                symbol = symbol_get_by_glyph( 0xe000 );

            size *= symbol->prescale;

            _regular_font_symbol_path( cr, size, x, y, symbol );

            x += size * symbol->x_advance;
            y -= size * symbol->y_advance;

            size *= symbol->postscale;

            if ( symbol->prescale * symbol->postscale != 1. )
                cairo_set_font_size( cr, size );

            continue;
        }

        bytes = g_unichar_to_utf8( ucs4[ j ], out );
        out[ bytes ] = '\0';

        cairo_text_extents( cr, out, &extents );

        cairo_move_to( cr, x, y );

        cairo_text_path( cr, out );

        x += extents.x_advance;
        y += extents.y_advance;
    }

    free( ucs4 );
}

void regular_font_measure_text( cairo_t* cr, double size, double* x_bearing, double* y_bearing, double* width, double* height,
                                       double* ascent, double* descent, ... )
{
    va_list ap0, ap1;
    cairo_font_extents_t font_extents;
    cairo_font_weight_t weight;
    cairo_font_slant_t slant;
    double x1, y1, x2, y2, a, d;
    const char* text;

    va_start( ap0, descent );
    va_copy( ap1, ap0 );

    _regular_font_vtext_path( cr, size, 0.0, 0.0, ap0 );

    va_end( ap0 );

    cairo_fill_extents( cr, &x1, &y1, &x2, &y2 );

    if ( y2 < 0.0 )
        y2 = 0.0;

    a = 0.0;
    d = 0.0;

    slant = va_arg( ap1, cairo_font_slant_t );
    weight = va_arg( ap1, cairo_font_weight_t );
    text = va_arg( ap1, const char* );
    ( void )text;

    cairo_select_font_face( cr, opt.font, slant, weight );
    cairo_set_font_size( cr, size );

    cairo_font_extents( cr, &font_extents );

    /*
     * Cairo seems to return overall height in ascent,
     * so fix this by calculating ascent = height - descent.
     */
    if ( font_extents.ascent - font_extents.descent > a )
        a = font_extents.ascent - font_extents.descent;
    if ( font_extents.descent > -d )
        d = -font_extents.descent;

    *x_bearing = x1;
    *y_bearing = y2;
    *width = x2 - x1;
    *height = y2 - y1;
    *ascent = a;
    *descent = d;

    va_end( ap1 );
}

void regular_font_draw_text( cairo_t* cr, GdkColor* color, double size, double line_width, int xoffset, int yoffset, ... )
{
    va_list ap;

    va_start( ap, yoffset );

    cairo_set_line_width( cr, line_width );
    cairo_set_source_rgb( cr, ( ( double )color->red ) / 65535.0, ( ( double )color->green ) / 65535.0,
                          ( ( double )color->blue ) / 65535.0 );

    _regular_font_vtext_path( cr, size, xoffset, yoffset, ap );

    if ( line_width == 0.0 )
        cairo_fill( cr );
    else
        cairo_stroke( cr );

    va_end( ap );
}
