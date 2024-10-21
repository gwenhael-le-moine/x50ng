/* $Id: tiny_font.c,v 1.6 2008/12/11 12:18:17 ecd Exp $
 */

#include <stdlib.h>

#include "include/bitmap_font.h"

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

const bitmap_font_t tiny_font =
{
	7,
	-3,
	{
		GLYPH(tiny, notdef),

		SPACE("space", 4, 0),
		GLYPH(tiny, quotedbl),
		GLYPH(tiny, numbersign),
		GLYPH(tiny, ampersand),
		GLYPH(tiny, parenleft),
		GLYPH(tiny, parenright),
		GLYPH(tiny, comma),
		GLYPH(tiny, hyphen),
		GLYPH(tiny, period),
		GLYPH(tiny, slash),

		GLYPH(tiny, zero),
		GLYPH(tiny, one),
		GLYPH(tiny, two),
		GLYPH(tiny, three),

		GLYPH(tiny, colon),

		GLYPH(tiny, less),
		GLYPH(tiny, equal),
		GLYPH(tiny, greater),

		GLYPH(tiny, A),
		GLYPH(tiny, B),
		GLYPH(tiny, C),
		GLYPH(tiny, D),
		GLYPH(tiny, E),
		GLYPH(tiny, F),
		GLYPH(tiny, G),
		GLYPH(tiny, H),
		GLYPH(tiny, I),
		GLYPH(tiny, J),
		GLYPH(tiny, K),
		GLYPH(tiny, L),
		GLYPH(tiny, M),
		GLYPH(tiny, N),
		GLYPH(tiny, O),
		GLYPH(tiny, P),
		GLYPH(tiny, Q),
		GLYPH(tiny, R),
		GLYPH(tiny, S),
		GLYPH(tiny, T),
		GLYPH(tiny, U),
		GLYPH(tiny, V),
		GLYPH(tiny, W),
		GLYPH(tiny, X),
		GLYPH(tiny, Y),
		GLYPH(tiny, Z),

		GLYPH(tiny, bracketleft),
		GLYPH(tiny, bracketright),
		GLYPH(tiny, underscore),

		GLYPH(tiny, i),

		GLYPH(tiny, overscore),
		GLYPH(tiny, arrowleft),
		GLYPH(tiny, arrowright),
		GLYPH(tiny, guillemotleft),
		GLYPH(tiny, guillemotright),

		GLYPH(tiny, braceleft),
		GLYPH(tiny, braceright),

		GLYPH(tiny, large_comma),

		GLYPH(tiny, xsuperior),
		GLYPH(tiny, twosuperior),

		GLYPH(tiny, math_e),
		GLYPH(tiny, math_x),
		GLYPH(tiny, math_y),
		GLYPH(tiny, math_pi),
		GLYPH(tiny, math_summation),
		GLYPH(tiny, math_radical),
		GLYPH(tiny, math_partialdiff),
		GLYPH(tiny, math_integral),
		GLYPH(tiny, math_infinity),

		GLYPH(tiny, math_numbersign),
		GLYPH(tiny, math_less),
		GLYPH(tiny, math_greater),
		GLYPH(tiny, math_lessequal),
		GLYPH(tiny, math_greaterequal),
		GLYPH(tiny, math_equal),
		GLYPH(tiny, math_notequal),

		GLYPH(tiny, math_arrowleft),
		GLYPH(tiny, math_arrowright),
		GLYPH(tiny, math_downarrowleft),
		GLYPH(tiny, math_downarrowright),

		SPACE("kern-1", -1, -1),
		SPACE("kern-2", -2, -2),
		SPACE("kern-3", -3, -3),
		SPACE("kern-4", -4, -4),
		SPACE("kern-5", -5, -5),
		SPACE("kern-6", -6, -6),
		SPACE("kern-7", -7, -7),

		{ NULL }
	}
};
