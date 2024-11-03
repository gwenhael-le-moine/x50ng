#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <gdk/gdk.h>

#include "ui_tiny_font.h"

#include "bitmaps/tiny_notdef.xbm"

#include "bitmaps/tiny_quotedbl.xbm"
#include "bitmaps/tiny_numbersign.xbm"
#include "bitmaps/tiny_ampersand.xbm"
#include "bitmaps/tiny_parenleft.xbm"
#include "bitmaps/tiny_parenright.xbm"
#include "bitmaps/tiny_comma.xbm"
#include "bitmaps/tiny_hyphen.xbm"
#include "bitmaps/tiny_period.xbm"
#include "bitmaps/tiny_slash.xbm"
#include "bitmaps/tiny_colon.xbm"
#include "bitmaps/tiny_less.xbm"
#include "bitmaps/tiny_equal.xbm"
#include "bitmaps/tiny_greater.xbm"
#include "bitmaps/tiny_bracketleft.xbm"
#include "bitmaps/tiny_bracketright.xbm"
#include "bitmaps/tiny_underscore.xbm"
#include "bitmaps/tiny_braceleft.xbm"
#include "bitmaps/tiny_braceright.xbm"

#include "bitmaps/tiny_guillemotleft.xbm"
#include "bitmaps/tiny_guillemotright.xbm"
#include "bitmaps/tiny_arrowleft.xbm"
#include "bitmaps/tiny_arrowright.xbm"

#include "bitmaps/tiny_large_comma.xbm"
#include "bitmaps/tiny_overscore.xbm"

#include "bitmaps/tiny_xsuperior.xbm"
#include "bitmaps/tiny_twosuperior.xbm"

#include "bitmaps/tiny_math_e.xbm"
#include "bitmaps/tiny_math_x.xbm"
#include "bitmaps/tiny_math_y.xbm"
#include "bitmaps/tiny_math_pi.xbm"
#include "bitmaps/tiny_math_summation.xbm"
#include "bitmaps/tiny_math_radical.xbm"
#include "bitmaps/tiny_math_partialdiff.xbm"
#include "bitmaps/tiny_math_integral.xbm"
#include "bitmaps/tiny_math_infinity.xbm"

#include "bitmaps/tiny_math_numbersign.xbm"
#include "bitmaps/tiny_math_less.xbm"
#include "bitmaps/tiny_math_greater.xbm"
#include "bitmaps/tiny_math_lessequal.xbm"
#include "bitmaps/tiny_math_greaterequal.xbm"
#include "bitmaps/tiny_math_equal.xbm"
#include "bitmaps/tiny_math_notequal.xbm"

#include "bitmaps/tiny_math_arrowleft.xbm"
#include "bitmaps/tiny_math_arrowright.xbm"
#include "bitmaps/tiny_math_downarrowleft.xbm"
#include "bitmaps/tiny_math_downarrowright.xbm"

#include "bitmaps/tiny_zero.xbm"
#include "bitmaps/tiny_one.xbm"
#include "bitmaps/tiny_two.xbm"
#include "bitmaps/tiny_three.xbm"

#include "bitmaps/tiny_A.xbm"
#include "bitmaps/tiny_B.xbm"
#include "bitmaps/tiny_C.xbm"
#include "bitmaps/tiny_D.xbm"
#include "bitmaps/tiny_E.xbm"
#include "bitmaps/tiny_F.xbm"
#include "bitmaps/tiny_G.xbm"
#include "bitmaps/tiny_H.xbm"
#include "bitmaps/tiny_I.xbm"
#include "bitmaps/tiny_J.xbm"
#include "bitmaps/tiny_K.xbm"
#include "bitmaps/tiny_L.xbm"
#include "bitmaps/tiny_M.xbm"
#include "bitmaps/tiny_N.xbm"
#include "bitmaps/tiny_O.xbm"
#include "bitmaps/tiny_P.xbm"
#include "bitmaps/tiny_Q.xbm"
#include "bitmaps/tiny_R.xbm"
#include "bitmaps/tiny_S.xbm"
#include "bitmaps/tiny_T.xbm"
#include "bitmaps/tiny_U.xbm"
#include "bitmaps/tiny_V.xbm"
#include "bitmaps/tiny_W.xbm"
#include "bitmaps/tiny_X.xbm"
#include "bitmaps/tiny_Y.xbm"
#include "bitmaps/tiny_Z.xbm"

#include "bitmaps/tiny__i.xbm"

const tiny_font_t tiny_font = {
    7,
    -3,
    { GLYPH( tiny, notdef ),

       SPACE( "space", 4, 0 ),
       GLYPH( tiny, quotedbl ),
       GLYPH( tiny, numbersign ),
       GLYPH( tiny, ampersand ),
       GLYPH( tiny, parenleft ),
       GLYPH( tiny, parenright ),
       GLYPH( tiny, comma ),
       GLYPH( tiny, hyphen ),
       GLYPH( tiny, period ),
       GLYPH( tiny, slash ),

       GLYPH( tiny, zero ),
       GLYPH( tiny, one ),
       GLYPH( tiny, two ),
       GLYPH( tiny, three ),

       GLYPH( tiny, colon ),

       GLYPH( tiny, less ),
       GLYPH( tiny, equal ),
       GLYPH( tiny, greater ),

       GLYPH( tiny, A ),
       GLYPH( tiny, B ),
       GLYPH( tiny, C ),
       GLYPH( tiny, D ),
       GLYPH( tiny, E ),
       GLYPH( tiny, F ),
       GLYPH( tiny, G ),
       GLYPH( tiny, H ),
       GLYPH( tiny, I ),
       GLYPH( tiny, J ),
       GLYPH( tiny, K ),
       GLYPH( tiny, L ),
       GLYPH( tiny, M ),
       GLYPH( tiny, N ),
       GLYPH( tiny, O ),
       GLYPH( tiny, P ),
       GLYPH( tiny, Q ),
       GLYPH( tiny, R ),
       GLYPH( tiny, S ),
       GLYPH( tiny, T ),
       GLYPH( tiny, U ),
       GLYPH( tiny, V ),
       GLYPH( tiny, W ),
       GLYPH( tiny, X ),
       GLYPH( tiny, Y ),
       GLYPH( tiny, Z ),

       GLYPH( tiny, bracketleft ),
       GLYPH( tiny, bracketright ),
       GLYPH( tiny, underscore ),

       GLYPH( tiny, i ),

       GLYPH( tiny, overscore ),
       GLYPH( tiny, arrowleft ),
       GLYPH( tiny, arrowright ),
       GLYPH( tiny, guillemotleft ),
       GLYPH( tiny, guillemotright ),

       GLYPH( tiny, braceleft ),
       GLYPH( tiny, braceright ),

       GLYPH( tiny, large_comma ),

       GLYPH( tiny, xsuperior ),
       GLYPH( tiny, twosuperior ),

       GLYPH( tiny, math_e ),
       GLYPH( tiny, math_x ),
       GLYPH( tiny, math_y ),
       GLYPH( tiny, math_pi ),
       GLYPH( tiny, math_summation ),
       GLYPH( tiny, math_radical ),
       GLYPH( tiny, math_partialdiff ),
       GLYPH( tiny, math_integral ),
       GLYPH( tiny, math_infinity ),

       GLYPH( tiny, math_numbersign ),
       GLYPH( tiny, math_less ),
       GLYPH( tiny, math_greater ),
       GLYPH( tiny, math_lessequal ),
       GLYPH( tiny, math_greaterequal ),
       GLYPH( tiny, math_equal ),
       GLYPH( tiny, math_notequal ),

       GLYPH( tiny, math_arrowleft ),
       GLYPH( tiny, math_arrowright ),
       GLYPH( tiny, math_downarrowleft ),
       GLYPH( tiny, math_downarrowright ),

       SPACE( "kern-1", -1, -1 ),
       SPACE( "kern-2", -2, -2 ),
       SPACE( "kern-3", -3, -3 ),
       SPACE( "kern-4", -4, -4 ),
       SPACE( "kern-5", -5, -5 ),
       SPACE( "kern-6", -6, -6 ),
       SPACE( "kern-7", -7, -7 ),

       { NULL } }
};

static unsigned char _tiny_font_lookup_glyph( const char* name, int namelen )
{
    for ( int i = 0; tiny_font.glyphs[ i ].name; i++ )
        if ( ( strlen( tiny_font.glyphs[ i ].name ) == namelen ) && !strncmp( tiny_font.glyphs[ i ].name, name, namelen ) )
            return i;

    return 0;
}

static unsigned char _tiny_font_lookup_ascii( char c )
{
    int namelen = 0;
    char* name;

    switch ( c ) {
        case ' ':
            name = "space";
            break;
        case '!':
            name = "exclam";
            break;
        case '"':
            name = "quotedbl";
            break;
        case '#':
            name = "numbersign";
            break;
        case '$':
            name = "dollar";
            break;
        case '%':
            name = "percent";
            break;
        case '&':
            name = "ampersand";
            break;
        case '(':
            name = "parenleft";
            break;
        case ')':
            name = "parenright";
            break;
        case '*':
            name = "asterisk";
            break;
        case '+':
            name = "plus";
            break;
        case ',':
            name = "comma";
            break;
        case '-':
            name = "hyphen";
            break;
        case '.':
            name = "period";
            break;
        case '/':
            name = "slash";
            break;
        case '0':
            name = "zero";
            break;
        case '1':
            name = "one";
            break;
        case '2':
            name = "two";
            break;
        case '3':
            name = "three";
            break;
        case '4':
            name = "four";
            break;
        case '5':
            name = "five";
            break;
        case '6':
            name = "six";
            break;
        case '7':
            name = "seven";
            break;
        case '8':
            name = "eight";
            break;
        case '9':
            name = "nine";
            break;
        case ':':
            name = "colon";
            break;
        case ';':
            name = "semicolon";
            break;
        case '<':
            name = "less";
            break;
        case '=':
            name = "equal";
            break;
        case '>':
            name = "greater";
            break;
        case '?':
            name = "question";
            break;
        case '@':
            name = "at";
            break;
        case '[':
            name = "bracketleft";
            break;
        case '\\':
            name = "backslash";
            break;
        case ']':
            name = "bracketright";
            break;
        case '^':
            name = "asciicircum";
            break;
        case '_':
            name = "underscore";
            break;
        case '`':
            name = "quoteleft";
            break;
        case '{':
            name = "braceleft";
            break;
        case '|':
            name = "bar";
            break;
        case '}':
            name = "braceright";
            break;
        case '~':
            name = "asciitilde";
            break;
        default:
            name = &c;
            namelen = 1;
            break;
    }

    if ( 0 == namelen )
        namelen = strlen( name );

    return _tiny_font_lookup_glyph( name, namelen );
}

static inline int _tiny_font_strlen( const char* text )
{
    const char *p, *q;
    char c;
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
        if ( *q == ' ' )
            q++;

        n++;
        p = q;
    }

    return n;
}

static int _tiny_font_text_to_glyphs( const char* text, unsigned char** glyphp )
{
    unsigned char* glyphs;
    const char *p, *q;
    unsigned char c;
    int namelen;
    int i, n;

    n = _tiny_font_strlen( text );
    if ( n <= 0 )
        return n;

    glyphs = malloc( n );

    i = 0;

    p = text;
    while ( ( c = *p++ ) ) {
        if ( i == n ) {
            free( glyphs );
            return -1;
        }

        if ( c != '\\' ) {
            glyphs[ i++ ] = _tiny_font_lookup_ascii( c );
            continue;
        }

        q = p;
        while ( *q ) {
            if ( ( *q == '\\' ) || ( *q == ' ' ) )
                break;
            q++;
        }
        if ( q == p ) {
            glyphs[ i++ ] = _tiny_font_lookup_ascii( *p++ );
            continue;
        }
        namelen = q - p;
        if ( *q == ' ' )
            q++;

        glyphs[ i++ ] = _tiny_font_lookup_glyph( p, namelen );
        p = q;
    }

    *glyphp = glyphs;
    return n;
}

void tiny_font_measure_text( const char* text, int* width, int* height, int* ascent, int* descent )
{
    const tiny_glyph_t* glyph;
    unsigned char* glyphs;
    int n, w, a, d;

    w = 0;
    a = 0;
    d = 0;

    n = _tiny_font_text_to_glyphs( text, &glyphs );

    for ( int i = 0; i < n; i++ ) {
        glyph = &tiny_font.glyphs[ glyphs[ i ] ];

        w += glyph->width;

        if ( glyph->ascent > a )
            a = glyph->ascent;
        if ( glyph->descent < d )
            d = glyph->descent;
    }

    *width = w - 1;
    *height = tiny_font.ascent - tiny_font.descent;
    *ascent = a;
    *descent = d;

    if ( n > 0 )
        free( glyphs );
}

void tiny_font_draw_text( GdkDrawable* drawable, GdkColor* color, int x, int y, const char* text )
{
    const tiny_glyph_t* glyph;
    unsigned char* glyphs;
    GdkBitmap* bitmap;
    GdkGC* gc;
    int n, w, h;

    gc = gdk_gc_new( drawable );
    gdk_gc_set_rgb_fg_color( gc, color );

    n = _tiny_font_text_to_glyphs( text, &glyphs );

    for ( int i = 0; i < n; i++ ) {
        glyph = &tiny_font.glyphs[ glyphs[ i ] ];

        w = glyph->width - glyph->kern;
        h = glyph->ascent - glyph->descent;

        if ( w <= 0 || h <= 0 ) {
            x += glyph->width;
            continue;
        }

        bitmap = gdk_bitmap_create_from_data( NULL, ( char* )glyph->bits, w, h );

        gdk_gc_set_ts_origin( gc, x + glyph->kern, y + tiny_font.ascent - glyph->ascent );
        gdk_gc_set_stipple( gc, bitmap );
        gdk_gc_set_fill( gc, GDK_STIPPLED );

        gdk_draw_rectangle( drawable, gc, true, x + glyph->kern, y + tiny_font.ascent - glyph->ascent, w, h );

        g_object_unref( bitmap );

        x += glyph->width;
    }

    g_object_unref( gc );

    if ( n > 0 )
        free( glyphs );
}
