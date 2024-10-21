/* $Id: tiny_font.c,v 1.6 2008/12/11 12:18:17 ecd Exp $
 */

#include <stdlib.h>
#include <bitmap_font.h>

#include <tiny_notdef.xbm>

#include <tiny_quotedbl.xbm>
#include <tiny_numbersign.xbm>
#include <tiny_ampersand.xbm>
#include <tiny_parenleft.xbm>
#include <tiny_parenright.xbm>
#include <tiny_comma.xbm>
#include <tiny_hyphen.xbm>
#include <tiny_period.xbm>
#include <tiny_slash.xbm>
#include <tiny_colon.xbm>
#include <tiny_less.xbm>
#include <tiny_equal.xbm>
#include <tiny_greater.xbm>
#include <tiny_bracketleft.xbm>
#include <tiny_bracketright.xbm>
#include <tiny_underscore.xbm>
#include <tiny_braceleft.xbm>
#include <tiny_braceright.xbm>

#include <tiny_guillemotleft.xbm>
#include <tiny_guillemotright.xbm>
#include <tiny_arrowleft.xbm>
#include <tiny_arrowright.xbm>

#include <tiny_large_comma.xbm>
#include <tiny_overscore.xbm>

#include <tiny_xsuperior.xbm>
#include <tiny_twosuperior.xbm>

#include <tiny_math_e.xbm>
#include <tiny_math_x.xbm>
#include <tiny_math_y.xbm>
#include <tiny_math_pi.xbm>
#include <tiny_math_summation.xbm>
#include <tiny_math_radical.xbm>
#include <tiny_math_partialdiff.xbm>
#include <tiny_math_integral.xbm>
#include <tiny_math_infinity.xbm>

#include <tiny_math_numbersign.xbm>
#include <tiny_math_less.xbm>
#include <tiny_math_greater.xbm>
#include <tiny_math_lessequal.xbm>
#include <tiny_math_greaterequal.xbm>
#include <tiny_math_equal.xbm>
#include <tiny_math_notequal.xbm>

#include <tiny_math_arrowleft.xbm>
#include <tiny_math_arrowright.xbm>
#include <tiny_math_downarrowleft.xbm>
#include <tiny_math_downarrowright.xbm>

#include <tiny_zero.xbm>
#include <tiny_one.xbm>
#include <tiny_two.xbm>
#include <tiny_three.xbm>

#include <tiny_A.xbm>
#include <tiny_B.xbm>
#include <tiny_C.xbm>
#include <tiny_D.xbm>
#include <tiny_E.xbm>
#include <tiny_F.xbm>
#include <tiny_G.xbm>
#include <tiny_H.xbm>
#include <tiny_I.xbm>
#include <tiny_J.xbm>
#include <tiny_K.xbm>
#include <tiny_L.xbm>
#include <tiny_M.xbm>
#include <tiny_N.xbm>
#include <tiny_O.xbm>
#include <tiny_P.xbm>
#include <tiny_Q.xbm>
#include <tiny_R.xbm>
#include <tiny_S.xbm>
#include <tiny_T.xbm>
#include <tiny_U.xbm>
#include <tiny_V.xbm>
#include <tiny_W.xbm>
#include <tiny_X.xbm>
#include <tiny_Y.xbm>
#include <tiny_Z.xbm>

#include <tiny__i.xbm>

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
