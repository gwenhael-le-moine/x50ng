/* $Id: ui.c,v 1.34 2008/12/11 12:18:17 ecd Exp $
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/times.h>
#include <math.h>
#include <errno.h>
#include <arpa/inet.h> /* For ntohl() */

#include <gtk/gtk.h>
#include <glib.h>
#include <cairo.h>
#include <gdk/gdkkeysyms.h>

#include "x49gp.h"
#include "x49gp_ui.h"
#include "s3c2410.h"
#include "bitmaps.h"
#include "bitmap_font.h"
#include "symbol.h"
#include "glyphname.h"

#include "gdbstub.h"

#if defined( __linux__ )
#  define X49GP_UI_NORMAL_FONT "urw gothic l"
#else
#  define X49GP_UI_NORMAL_FONT "Century Gothic"
#endif

static const x49gp_ui_key_t x49gp_ui_keys[] = {
    {"F1",                              "A",  "Y=",                    NULL,      NULL,     UI_COLOR_BLACK,  12.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_TINY,   12.0, UI_LAYOUT_LEFT,          0,   0,
     36,                                                                                                                                                                                                        22, 5, 1, ( 1 << 5 ), ( 1 << 1 ), 1},
    {"F2",                              "B",  "WIN",                   NULL,      NULL,     UI_COLOR_BLACK,  12.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_TINY,   12.0, UI_LAYOUT_LEFT,          50,  0,
     36,                                                                                                                                                                                                        22, 5, 2, ( 1 << 5 ), ( 1 << 2 ), 2},
    {"F3",                              "C",  "GRAPH",                 NULL,      NULL,     UI_COLOR_BLACK,  12.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_TINY,   12.0, UI_LAYOUT_LEFT,          99,  0,
     36,                                                                                                                                                                                                        22, 5, 3, ( 1 << 5 ), ( 1 << 3 ), 3},
    {"F4",
     "D",                                     "2D/3D",
     NULL,                                                                        NULL,
     UI_COLOR_BLACK,                                                                                         12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                               UI_SHAPE_BUTTON_TINY,
     12.0,                                                                                                                                                               UI_LAYOUT_LEFT,
     149,                                                                                                                                                                                              0,
     36,                                                                                                                                                                                                        22,
     5,                                                                                                                                                                                                                4,
     ( 1 << 5 ),
     ( 1 << 4 ),
     4                                                                                                                                                                                                                                             },
    {"F5",
     "E",                                     "TBLSET",
     NULL,                                                                        NULL,
     UI_COLOR_BLACK,                                                                                         12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                               UI_SHAPE_BUTTON_TINY,
     12.0,                                                                                                                                                               UI_LAYOUT_LEFT,
     198,                                                                                                                                                                                              0,
     36,                                                                                                                                                                                                        22,
     5,                                                                                                                                                                                                                5,
     ( 1 << 5 ),
     ( 1 << 5 ),
     5                                                                                                                                                                                                                                             },
    {"F6",
     "F",                                     "TABLE",
     NULL,                                                                        NULL,
     UI_COLOR_BLACK,                                                                                         12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                               UI_SHAPE_BUTTON_TINY,
     12.0,                                                                                                                                                               UI_LAYOUT_LEFT,
     247,                                                                                                                                                                                              0,
     36,                                                                                                                                                                                                        22,
     5,                                                                                                                                                                                                                6,
     ( 1 << 5 ),
     ( 1 << 6 ),
     6                                                                                                                                                                                                                                             },
    {"APPS",
     "G",                                     "FILES",
     "BEGIN",                                                                     NULL,
     UI_COLOR_WHITE,                                                                                         10.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                               UI_SHAPE_BUTTON_SMALL,
     10.0,                                                                                                                                                               UI_LAYOUT_BELOW,
     0,                                                                                                                                                                                                44,
     36,                                                                                                                                                                                                        28,
     5,                                                                                                                                                                                                                7,
     ( 1 << 5 ),
     ( 1 << 7 ),
     7                                                                                                                                                                                                                                             },
    {"MODE",
     "H",                                     "CUSTOM",
     "END",                                                                       NULL,
     UI_COLOR_WHITE,                                                                                         10.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                               UI_SHAPE_BUTTON_SMALL,
     10.0,                                                                                                                                                               UI_LAYOUT_BELOW,
     50,                                                                                                                                                                                               44,
     36,                                                                                                                                                                                                        28,
     6,                                                                                                                                                                                                                5,
     ( 1 << 6 ),
     ( 1 << 5 ),
     5                                                                                                                                                                                                                                             },
    {"TOOL",                            "I",  "i",                     "I",       NULL,     UI_COLOR_WHITE,  10.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_SMALL,  10.0, UI_LAYOUT_BELOW,         99,  44,
     36,                                                                                                                                                                                                        28, 6, 6, ( 1 << 6 ), ( 1 << 6 ), 6},
    {"V\\kern-1 AR",
     "J",                                     "UPDIR",
     "COPY",                                                                      NULL,
     UI_COLOR_WHITE,                                                                                         10.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                               UI_SHAPE_BUTTON_SMALL,
     10.0,                                                                                                                                                               UI_LAYOUT_BELOW,
     0,                                                                                                                                                                                                92,
     36,                                                                                                                                                                                                        28,
     6,                                                                                                                                                                                                                7,
     ( 1 << 6 ),
     ( 1 << 7 ),
     7                                                                                                                                                                                                                                             },
    {"STO \\triangleright",
     "K",                                     "RCL",
     "CUT",                                                                       NULL,
     UI_COLOR_WHITE,                                                                                         10.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                               UI_SHAPE_BUTTON_SMALL,
     10.0,                                                                                                                                                               UI_LAYOUT_BELOW,
     50,                                                                                                                                                                                               92,
     36,                                                                                                                                                                                                        28,
     7,                                                                                                                                                                                                                1,
     ( 1 << 7 ),
     ( 1 << 1 ),
     1                                                                                                                                                                                                                                             },
    {"NXT",
     "L",                                     "PREV",
     "PASTE",                                                                     NULL,
     UI_COLOR_WHITE,                                                                                         10.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                               UI_SHAPE_BUTTON_SMALL,
     10.0,                                                                                                                                                               UI_LAYOUT_BELOW,
     99,                                                                                                                                                                                               92,
     36,                                                                                                                                                                                                        28,
     7,                                                                                                                                                                                                                2,
     ( 1 << 7 ),
     ( 1 << 2 ),
     2                                                                                                                                                                                                                                             },
    {NULL,                              NULL, NULL,                    NULL,      NULL,     UI_COLOR_SILVER, 0.0,  0,                      UI_SHAPE_BUTTON_ROUND,  0.0,  0,                       200,
     38,                                                                                                                                                                                                    33, 33, 6, 1, ( 1 << 6 ), ( 1 << 1 ), 1},
    {NULL,                              NULL, NULL,                    NULL,      NULL,     UI_COLOR_SILVER, 0.0,  0,                      UI_SHAPE_BUTTON_ROUND,  0.0,  0,                       164,
     66,                                                                                                                                                                                                    33, 33, 6, 2, ( 1 << 6 ), ( 1 << 2 ), 2},
    {NULL,                              NULL, NULL,                    NULL,      NULL,     UI_COLOR_SILVER, 0.0,  0,                      UI_SHAPE_BUTTON_ROUND,  0.0,  0,                       200,
     94,                                                                                                                                                                                                    33, 33, 6, 3, ( 1 << 6 ), ( 1 << 3 ), 3},
    {NULL,                              NULL, NULL,                    NULL,      NULL,     UI_COLOR_SILVER, 0.0,  0,                      UI_SHAPE_BUTTON_ROUND,  0.0,  0,                       236,
     66,                                                                                                                                                                                                    33, 33, 6, 4, ( 1 << 6 ), ( 1 << 4 ), 4},
    {"HIST",
     "M",                                     "CMD",
     "UNDO",                                                                      NULL,
     UI_COLOR_WHITE,                                                                                         12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                               UI_SHAPE_BUTTON_NORMAL,
     12.0,                                                                                                                                                               UI_LAYOUT_LEFT,
     0,                                                                                                                                                                                                141,
     46,                                                                                                                                                                                                        28,
     4,                                                                                                                                                                                                                1,
     ( 1 << 4 ),
     ( 1 << 1 ),
     1                                                                                                                                                                                                                                             },
    {"EV\\kern-1 AL",
     "N",                                     "PRG",
     "CHARS",                                                                     NULL,
     UI_COLOR_WHITE,                                                                                         12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                               UI_SHAPE_BUTTON_NORMAL,
     12.0,                                                                                                                                                               UI_LAYOUT_LEFT,
     59,                                                                                                                                                                                               141,
     46,                                                                                                                                                                                                        28,
     3,                                                                                                                                                                                                                1,
     ( 1 << 3 ),
     ( 1 << 1 ),
     1                                                                                                                                                                                                                                             },
    {"\\tick",
     "O",                                     "MTRW",
     "EQW",                                                                       NULL,
     UI_COLOR_WHITE,                                                                                         12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                               UI_SHAPE_BUTTON_NORMAL,
     12.0,                                                                                                                                                               UI_LAYOUT_LEFT,
     119,                                                                                                                                                                                              141,
     46,                                                                                                                                                                                                        28,
     2,                                                                                                                                                                                                                1,
     ( 1 << 2 ),
     ( 1 << 1 ),
     1                                                                                                                                                                                                                                             },
    {"S\\kern-1 Y\\kern-1 M\\kern-1 B",
     "P",                                     "MTH",
     "CAT",                                                                       NULL,
     UI_COLOR_WHITE,                                                                                         12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                               UI_SHAPE_BUTTON_NORMAL,
     12.0,                                                                                                                                                               UI_LAYOUT_LEFT,
     179,                                                                                                                                                                                              141,
     46,                                                                                                                                                                                                        28,
     1,                                                                                                                                                                                                                1,
     ( 1 << 1 ),
     ( 1 << 1 ),
     1                                                                                                                                                                                                                                             },
    {"\\arrowleftdblfull",
     NULL,                                    "DEL",
     "CLEAR",                                                                     NULL,
     UI_COLOR_WHITE,                                                                                         12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                               UI_SHAPE_BUTTON_NORMAL,
     0.0,                                                                                                                                                                0,
     238,                                                                                                                                                                                              141,
     46,                                                                                                                                                                                                        28,
     0,                                                                                                                                                                                                                1,
     ( 1 << 0 ),
     ( 1 << 1 ),
     1                                                                                                                                                                                                                                             },
    {"Y\\super x\\/super",
     "Q",                                     "\\math_e\\xsuperior",
     "LN",                                                                        NULL,
     UI_COLOR_WHITE,                                                                                         12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                               UI_SHAPE_BUTTON_NORMAL,
     12.0,                                                                                                                                                               UI_LAYOUT_LEFT,
     0,                                                                                                                                                                                                183,
     46,                                                                                                                                                                                                        28,
     4,                                                                                                                                                                                                                2,
     ( 1 << 4 ),
     ( 1 << 2 ),
     2                                                                                                                                                                                                                                             },
    {"\\radical\\overscore\\kern-7 X",
     "R",                                     "\\math_x\\twosuperior",
     "\\xsuperior\\kern-4\\math_radical\\overscore\\kern-5\\math_y",              NULL,
     UI_COLOR_WHITE,                                                                                         12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                               UI_SHAPE_BUTTON_NORMAL,
     12.0,                                                                                                                                                               UI_LAYOUT_LEFT,
     59,                                                                                                                                                                                               183,
     46,                                                                                                                                                                                                        28,
     3,                                                                                                                                                                                                                2,
     ( 1 << 3 ),
     ( 1 << 2 ),
     2                                                                                                                                                                                                                                             },
    {"SIN",
     "S",                                     "ASIN",
     "\\math_summation",                                                          NULL,
     UI_COLOR_WHITE,                                                                                         12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                               UI_SHAPE_BUTTON_NORMAL,
     12.0,                                                                                                                                                               UI_LAYOUT_LEFT,
     119,                                                                                                                                                                                              183,
     46,                                                                                                                                                                                                        28,
     2,                                                                                                                                                                                                                2,
     ( 1 << 2 ),
     ( 1 << 2 ),
     2                                                                                                                                                                                                                                             },
    {"COS",
     "T",                                     "ACOS",
     "\\math_partialdiff",                                                        NULL,
     UI_COLOR_WHITE,                                                                                         12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                               UI_SHAPE_BUTTON_NORMAL,
     12.0,                                                                                                                                                               UI_LAYOUT_LEFT,
     179,                                                                                                                                                                                              183,
     46,                                                                                                                                                                                                        28,
     1,                                                                                                                                                                                                                2,
     ( 1 << 1 ),
     ( 1 << 2 ),
     2                                                                                                                                                                                                                                             },
    {"TAN",
     "U",                                     "ATAN",
     "\\math_integral",                                                           NULL,
     UI_COLOR_WHITE,                                                                                         12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                               UI_SHAPE_BUTTON_NORMAL,
     12.0,                                                                                                                                                               UI_LAYOUT_LEFT,
     238,                                                                                                                                                                                              183,
     46,                                                                                                                                                                                                        28,
     0,                                                                                                                                                                                                                2,
     ( 1 << 0 ),
     ( 1 << 2 ),
     2                                                                                                                                                                                                                                             },
    {"EEX",
     "V",                                     "10\\xsuperior",
     "LOG",                                                                       NULL,
     UI_COLOR_WHITE,                                                                                         12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                               UI_SHAPE_BUTTON_NORMAL,
     12.0,                                                                                                                                                               UI_LAYOUT_LEFT,
     0,                                                                                                                                                                                                225,
     46,                                                                                                                                                                                                        28,
     4,                                                                                                                                                                                                                3,
     ( 1 << 4 ),
     ( 1 << 3 ),
     3                                                                                                                                                                                                                                             },
    {"+\\divisionslash\\minus",
     "W",                                     "\\math_notequal",
     "\\math_equal",                                                              NULL,
     UI_COLOR_WHITE,                                                                                         12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                               UI_SHAPE_BUTTON_NORMAL,
     12.0,                                                                                                                                                               UI_LAYOUT_LEFT,
     59,                                                                                                                                                                                               225,
     46,                                                                                                                                                                                                        28,
     3,                                                                                                                                                                                                                3,
     ( 1 << 3 ),
     ( 1 << 3 ),
     3                                                                                                                                                                                                                                             },
    {"X",
     "X",                                     "\\math_lessequal",
     "\\math_less",                                                               NULL,
     UI_COLOR_WHITE,                                                                                         12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                               UI_SHAPE_BUTTON_NORMAL,
     12.0,                                                                                                                                                               UI_LAYOUT_LEFT,
     119,                                                                                                                                                                                              225,
     46,                                                                                                                                                                                                        28,
     2,                                                                                                                                                                                                                3,
     ( 1 << 2 ),
     ( 1 << 3 ),
     3                                                                                                                                                                                                                                             },
    {"1/X",
     "Y",                                     "\\math_greaterequal",
     "\\math_greater",                                                            NULL,
     UI_COLOR_WHITE,                                                                                         12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                               UI_SHAPE_BUTTON_NORMAL,
     12.0,                                                                                                                                                               UI_LAYOUT_LEFT,
     179,                                                                                                                                                                                              225,
     46,                                                                                                                                                                                                        28,
     1,                                                                                                                                                                                                                3,
     ( 1 << 1 ),
     ( 1 << 3 ),
     3                                                                                                                                                                                                                                             },
    {"\\divide",
     "Z",                                     "ABS",
     "ARG",                                                                       NULL,
     UI_COLOR_WHITE,                                                                                         19.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                               UI_SHAPE_BUTTON_NORMAL,
     12.0,                                                                                                                                                               UI_LAYOUT_LEFT_NO_SPACE,
     238,                                                                                                                                                                                              225,
     46,                                                                                                                                                                                                        28,
     0,                                                                                                                                                                                                                3,
     ( 1 << 0 ),
     ( 1 << 3 ),
     3                                                                                                                                                                                                                                             },
    {"ALPHA",                           NULL, "USER",                  "ENTRY",   NULL,     UI_COLOR_BLACK,  12.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_LARGE,  0.0,  0,                       0,   267, 46, 32,
     0,                                                                                                                                                                                                                0, 0,          0,          4},
    {"7",                               NULL, "S.SLV",                 "NUM.SLV", NULL,     UI_COLOR_WHITE,  19.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_LARGE,  0.0,  0,                       59,  267,
     46,                                                                                                                                                                                                        32, 3, 4, ( 1 << 3 ), ( 1 << 4 ), 4},
    {"8",                               NULL, "EXP&LN",                "TRIG",    NULL,     UI_COLOR_WHITE,  19.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_LARGE,  0.0,  0,                       119, 267,
     46,                                                                                                                                                                                                        32, 2, 4, ( 1 << 2 ), ( 1 << 4 ), 4},
    {"9",                               NULL, "FINANCE",               "TIME",    NULL,     UI_COLOR_WHITE,  19.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_LARGE,  0.0,  0,                       179, 267,
     46,                                                                                                                                                                                                        32, 1, 4, ( 1 << 1 ), ( 1 << 4 ), 4},
    {"\\multiply",                      NULL, "[ ]",                   "\" \"",   NULL,     UI_COLOR_WHITE,  19.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_LARGE,  0.0,  0,                       238, 267,
     46,                                                                                                                                                                                                        32, 0, 4, ( 1 << 0 ), ( 1 << 4 ), 4},
    {"\\uparrowleft",
     NULL,                                    NULL,
     NULL,                                                                        NULL,
     UI_COLOR_WHITE,                                                                                         19.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                               UI_SHAPE_BUTTON_LARGE,
     0.0,                                                                                                                                                                0,
     0,                                                                                                                                                                                                313,
     46,                                                                                                                                                                                                        32,
     0,                                                                                                                                                                                                                0,
     0,                                                                                                                                                                                                                               0,
     5                                                                                                                                                                                                                                             },
    {"4",                               NULL, "CALC",                  "ALG",     NULL,     UI_COLOR_WHITE,  19.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_LARGE,  0.0,  0,                       59,  313,
     46,                                                                                                                                                                                                        32, 3, 5, ( 1 << 3 ), ( 1 << 5 ), 5},
    {"5",                               NULL, "MATRICES",              "STAT",    NULL,     UI_COLOR_WHITE,  19.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_LARGE,  0.0,  0,                       119, 313,
     46,                                                                                                                                                                                                        32, 2, 5, ( 1 << 2 ), ( 1 << 5 ), 5},
    {"6",                               NULL, "CONVERT",               "UNITS",   NULL,     UI_COLOR_WHITE,  19.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_LARGE,  0.0,  0,                       179, 313,
     46,                                                                                                                                                                                                        32, 1, 5, ( 1 << 1 ), ( 1 << 5 ), 5},
    {"\\minus",                         NULL, "( )",                   "_",       NULL,     UI_COLOR_WHITE,  19.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_LARGE,  0.0,  0,                       238, 313,
     46,                                                                                                                                                                                                        32, 0, 5, ( 1 << 0 ), ( 1 << 5 ), 5},
    {"\\uparrowright",
     NULL,                                    NULL,
     NULL,                                                                        NULL,
     UI_COLOR_WHITE,                                                                                         19.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                               UI_SHAPE_BUTTON_LARGE,
     0.0,                                                                                                                                                                0,
     0,                                                                                                                                                                                                359,
     46,                                                                                                                                                                                                        32,
     0,                                                                                                                                                                                                                0,
     0,                                                                                                                                                                                                                               0,
     6                                                                                                                                                                                                                                             },
    {"1",                               NULL, "ARITH",                 "CMPLX",   NULL,     UI_COLOR_WHITE,  19.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_LARGE,  0.0,  0,                       59,  359,
     46,                                                                                                                                                                                                        32, 3, 6, ( 1 << 3 ), ( 1 << 6 ), 6},
    {"2",                               NULL, "DEF",                   "LIB",     NULL,     UI_COLOR_WHITE,  19.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_LARGE,  0.0,  0,                       119, 359,
     46,                                                                                                                                                                                                        32, 2, 6, ( 1 << 2 ), ( 1 << 6 ), 6},
    {"3",
     NULL,                                    "\\math_numbersign",
     "BASE",                                                                      NULL,
     UI_COLOR_WHITE,                                                                                         19.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                               UI_SHAPE_BUTTON_LARGE,
     0.0,                                                                                                                                                                0,
     179,                                                                                                                                                                                              359,
     46,                                                                                                                                                                                                        32,
     1,                                                                                                                                                                                                                6,
     ( 1 << 1 ),
     ( 1 << 6 ),
     6                                                                                                                                                                                                                                             },
    {"+",
     NULL,                                    "{ }",
     "\\guillemotleft\\ \\guillemotright",                                        NULL,
     UI_COLOR_WHITE,                                                                                         19.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                               UI_SHAPE_BUTTON_LARGE,
     0.0,                                                                                                                                                                0,
     238,                                                                                                                                                                                              359,
     46,                                                                                                                                                                                                        32,
     0,                                                                                                                                                                                                                6,
     ( 1 << 0 ),
     ( 1 << 6 ),
     6                                                                                                                                                                                                                                             },
    {"ON",                              NULL, "CONT",                  "OFF",     "CANCEL", UI_COLOR_WHITE,  12.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_LARGE,  0.0,  0,                       0,   405, 46, 32,
     0,                                                                                                                                                                                                                0, 0,          0,          0},
    {"0",
     NULL,                                    "\\math_infinity",
     "\\math_arrowright",                                                         NULL,
     UI_COLOR_WHITE,                                                                                         19.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                               UI_SHAPE_BUTTON_LARGE,
     0.0,                                                                                                                                                                0,
     59,                                                                                                                                                                                               405,
     46,                                                                                                                                                                                                        32,
     3,                                                                                                                                                                                                                7,
     ( 1 << 3 ),
     ( 1 << 7 ),
     7                                                                                                                                                                                                                                             },
    {"\\bullet",
     NULL,                                    ": :",
     "\\math_downarrowleft",                                                      NULL,
     UI_COLOR_WHITE,                                                                                         19.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                               UI_SHAPE_BUTTON_LARGE,
     0.0,                                                                                                                                                                0,
     119,                                                                                                                                                                                              405,
     46,                                                                                                                                                                                                        32,
     2,                                                                                                                                                                                                                7,
     ( 1 << 2 ),
     ( 1 << 7 ),
     7                                                                                                                                                                                                                                             },
    {"SPC",
     NULL,                                    "\\math_pi",
     "\\large_comma",                                                             NULL,
     UI_COLOR_WHITE,                                                                                         12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                               UI_SHAPE_BUTTON_LARGE,
     0.0,                                                                                                                                                                0,
     179,                                                                                                                                                                                              405,
     46,                                                                                                                                                                                                        32,
     1,                                                                                                                                                                                                                7,
     ( 1 << 1 ),
     ( 1 << 7 ),
     7                                                                                                                                                                                                                                             },
    {"ENTER",
     NULL,                                    "ANS",
     "\\arrowright NUM",                                                          NULL,
     UI_COLOR_WHITE,                                                                                         12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                               UI_SHAPE_BUTTON_LARGE,
     0.0,                                                                                                                                                                0,
     238,                                                                                                                                                                                              405,
     46,                                                                                                                                                                                                        32,
     0,                                                                                                                                                                                                                7,
     ( 1 << 0 ),
     ( 1 << 7 ),
     7                                                                                                                                                                                                                                             },
};
#define X49GP_UI_NR_KEYS ( sizeof( x49gp_ui_keys ) / sizeof( x49gp_ui_keys[ 0 ] ) )

static const x49gp_ui_key_t x49gp_newrpl_ui_keys[] = {
    {"F1",                                           "A",  NULL,                    NULL,      NULL, UI_COLOR_BLACK,  12.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_TINY,   12.0, UI_LAYOUT_LEFT,          0,   0,
     36,                                                                                                                                                                                                                 22, 5, 1, ( 1 << 5 ), ( 1 << 1 ), 1},
    {"F2",                                           "B",  NULL,                    NULL,      NULL, UI_COLOR_BLACK,  12.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_TINY,   12.0, UI_LAYOUT_LEFT,          50,  0,
     36,                                                                                                                                                                                                                 22, 5, 2, ( 1 << 5 ), ( 1 << 2 ), 2},
    {"F3",                                           "C",  NULL,                    NULL,      NULL, UI_COLOR_BLACK,  12.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_TINY,   12.0, UI_LAYOUT_LEFT,          99,  0,
     36,                                                                                                                                                                                                                 22, 5, 3, ( 1 << 5 ), ( 1 << 3 ), 3},
    {"F4",                                           "D",  NULL,                    NULL,      NULL, UI_COLOR_BLACK,  12.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_TINY,   12.0, UI_LAYOUT_LEFT,          149, 0,
     36,                                                                                                                                                                                                                 22, 5, 4, ( 1 << 5 ), ( 1 << 4 ), 4},
    {"F5",                                           "E",  NULL,                    NULL,      NULL, UI_COLOR_BLACK,  12.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_TINY,   12.0, UI_LAYOUT_LEFT,          198, 0,
     36,                                                                                                                                                                                                                 22, 5, 5, ( 1 << 5 ), ( 1 << 5 ), 5},
    {"F6",                                           "F",  NULL,                    NULL,      NULL, UI_COLOR_BLACK,  12.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_TINY,   12.0, UI_LAYOUT_LEFT,          247, 0,
     36,                                                                                                                                                                                                                 22, 5, 6, ( 1 << 5 ), ( 1 << 6 ), 6},
    {"",                                             "G",  NULL,                    NULL,      NULL, UI_COLOR_WHITE,  10.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_SMALL,  10.0, UI_LAYOUT_BELOW,         0,   44,
     36,                                                                                                                                                                                                                 28, 5, 7, ( 1 << 5 ), ( 1 << 7 ), 7},
    {"",                                             "H",  NULL,                    NULL,      NULL, UI_COLOR_WHITE,  10.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_SMALL,  10.0, UI_LAYOUT_BELOW,         50,  44,
     36,                                                                                                                                                                                                                 28, 6, 5, ( 1 << 6 ), ( 1 << 5 ), 5},
    {"",                                             "I",  NULL,                    NULL,      NULL, UI_COLOR_WHITE,  10.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_SMALL,  10.0, UI_LAYOUT_BELOW,         99,  44,
     36,                                                                                                                                                                                                                 28, 6, 6, ( 1 << 6 ), ( 1 << 6 ), 6},
    {"",                                             "J",  NULL,                    NULL,      NULL, UI_COLOR_WHITE,  10.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_SMALL,  10.0, UI_LAYOUT_BELOW,         0,   92,
     36,                                                                                                                                                                                                                 28, 6, 7, ( 1 << 6 ), ( 1 << 7 ), 7},
    {"",                                             "K",  NULL,                    NULL,      NULL, UI_COLOR_WHITE,  10.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_SMALL,  10.0, UI_LAYOUT_BELOW,         50,  92,
     36,                                                                                                                                                                                                                 28, 7, 1, ( 1 << 7 ), ( 1 << 1 ), 1},
    {"",                                             "L",  NULL,                    NULL,      NULL, UI_COLOR_WHITE,  10.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_SMALL,  10.0, UI_LAYOUT_BELOW,         99,  92,
     36,                                                                                                                                                                                                                 28, 7, 2, ( 1 << 7 ), ( 1 << 2 ), 2},
    {"",                                             "",   "UPDIR",                 NULL,      NULL, UI_COLOR_SILVER, 10.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_ROUND,  0.0,  0,                       200, 38,
     33,                                                                                                                                                                                                                 33, 6, 1, ( 1 << 6 ), ( 1 << 1 ), 1},
    {"",                                             "",   "BEG COPY",              NULL,      NULL, UI_COLOR_SILVER, 10.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_ROUND,  0.0,  0,                       164, 66,
     33,                                                                                                                                                                                                                 33, 6, 2, ( 1 << 6 ), ( 1 << 2 ), 2},
    {"",                                             "",   "CUT",                   NULL,      NULL, UI_COLOR_SILVER, 10.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_ROUND,  0.0,  0,                       200, 94,
     33,                                                                                                                                                                                                                 33, 6, 3, ( 1 << 6 ), ( 1 << 3 ), 3},
    {"",                                             "",   "END PASTE",             NULL,      NULL, UI_COLOR_SILVER, 10.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_ROUND,  0.0,  0,                       236, 66,
     33,                                                                                                                                                                                                                 33, 6, 4, ( 1 << 6 ), ( 1 << 4 ), 4},
    {"S\\kern-1 T\\kern-1 O\\kern-1\\triangleright",
     "M",                                                  "RCL",
     "PREV.M",                                                                                 NULL,
     UI_COLOR_WHITE,                                                                                                  12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                                        UI_SHAPE_BUTTON_NORMAL,
     12.0,                                                                                                                                                                        UI_LAYOUT_LEFT,
     0,                                                                                                                                                                                                         141,
     46,                                                                                                                                                                                                                 28,
     4,                                                                                                                                                                                                                         1,
     ( 1 << 4 ),
     ( 1 << 1 ),
     1                                                                                                                                                                                                                                                      },
    {"EV\\kern-1 AL",
     "N",                                                  NULL,
     NULL,                                                                                     NULL,
     UI_COLOR_WHITE,                                                                                                  12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                                        UI_SHAPE_BUTTON_NORMAL,
     12.0,                                                                                                                                                                        UI_LAYOUT_LEFT,
     59,                                                                                                                                                                                                        141,
     46,                                                                                                                                                                                                                 28,
     3,                                                                                                                                                                                                                         1,
     ( 1 << 3 ),
     ( 1 << 1 ),
     1                                                                                                                                                                                                                                                      },
    {"\\tick",
     "O",                                                  NULL,
     NULL,                                                                                     NULL,
     UI_COLOR_WHITE,                                                                                                  12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                                        UI_SHAPE_BUTTON_NORMAL,
     12.0,                                                                                                                                                                        UI_LAYOUT_LEFT,
     119,                                                                                                                                                                                                       141,
     46,                                                                                                                                                                                                                 28,
     2,                                                                                                                                                                                                                         1,
     ( 1 << 2 ),
     ( 1 << 1 ),
     1                                                                                                                                                                                                                                                      },
    {"M\\kern-1 E\\kern-1 N\\kern-1 U",
     "P",                                                  NULL,
     NULL,                                                                                     NULL,
     UI_COLOR_WHITE,                                                                                                  12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                                        UI_SHAPE_BUTTON_NORMAL,
     12.0,                                                                                                                                                                        UI_LAYOUT_LEFT,
     179,                                                                                                                                                                                                       141,
     46,                                                                                                                                                                                                                 28,
     1,                                                                                                                                                                                                                         1,
     ( 1 << 1 ),
     ( 1 << 1 ),
     1                                                                                                                                                                                                                                                      },
    {"\\arrowleftdblfull",
     NULL,                                                 NULL,
     "CLEAR",                                                                                  NULL,
     UI_COLOR_WHITE,                                                                                                  12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                                        UI_SHAPE_BUTTON_NORMAL,
     0.0,                                                                                                                                                                         0,
     238,                                                                                                                                                                                                       141,
     46,                                                                                                                                                                                                                 28,
     0,                                                                                                                                                                                                                         1,
     ( 1 << 0 ),
     ( 1 << 1 ),
     1                                                                                                                                                                                                                                                      },
    {"Y\\super x\\/super",
     "Q",                                                  "\\math_e\\xsuperior",
     "LN",                                                                                     NULL,
     UI_COLOR_WHITE,                                                                                                  12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                                        UI_SHAPE_BUTTON_NORMAL,
     12.0,                                                                                                                                                                        UI_LAYOUT_LEFT,
     0,                                                                                                                                                                                                         183,
     46,                                                                                                                                                                                                                 28,
     4,                                                                                                                                                                                                                         2,
     ( 1 << 4 ),
     ( 1 << 2 ),
     2                                                                                                                                                                                                                                                      },
    {"\\radical\\overscore\\kern-7 X",
     "R",                                                  "\\math_x\\twosuperior",
     "\\xsuperior\\kern-4\\math_radical\\overscore\\kern-5\\math_y",                           NULL,
     UI_COLOR_WHITE,                                                                                                  12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                                        UI_SHAPE_BUTTON_NORMAL,
     12.0,                                                                                                                                                                        UI_LAYOUT_LEFT,
     59,                                                                                                                                                                                                        183,
     46,                                                                                                                                                                                                                 28,
     3,                                                                                                                                                                                                                         2,
     ( 1 << 3 ),
     ( 1 << 2 ),
     2                                                                                                                                                                                                                                                      },
    {"SIN",
     "S",                                                  "ASIN",
     NULL,                                                                                     NULL,
     UI_COLOR_WHITE,                                                                                                  12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                                        UI_SHAPE_BUTTON_NORMAL,
     12.0,                                                                                                                                                                        UI_LAYOUT_LEFT,
     119,                                                                                                                                                                                                       183,
     46,                                                                                                                                                                                                                 28,
     2,                                                                                                                                                                                                                         2,
     ( 1 << 2 ),
     ( 1 << 2 ),
     2                                                                                                                                                                                                                                                      },
    {"COS",
     "T",                                                  "ACOS",
     NULL,                                                                                     NULL,
     UI_COLOR_WHITE,                                                                                                  12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                                        UI_SHAPE_BUTTON_NORMAL,
     12.0,                                                                                                                                                                        UI_LAYOUT_LEFT,
     179,                                                                                                                                                                                                       183,
     46,                                                                                                                                                                                                                 28,
     1,                                                                                                                                                                                                                         2,
     ( 1 << 1 ),
     ( 1 << 2 ),
     2                                                                                                                                                                                                                                                      },
    {"TAN",
     "U",                                                  "ATAN",
     NULL,                                                                                     NULL,
     UI_COLOR_WHITE,                                                                                                  12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                                        UI_SHAPE_BUTTON_NORMAL,
     12.0,                                                                                                                                                                        UI_LAYOUT_LEFT,
     238,                                                                                                                                                                                                       183,
     46,                                                                                                                                                                                                                 28,
     0,                                                                                                                                                                                                                         2,
     ( 1 << 0 ),
     ( 1 << 2 ),
     2                                                                                                                                                                                                                                                      },
    {"EEX",
     "V",                                                  "10\\xsuperior",
     "LOG",                                                                                    NULL,
     UI_COLOR_WHITE,                                                                                                  12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                                        UI_SHAPE_BUTTON_NORMAL,
     12.0,                                                                                                                                                                        UI_LAYOUT_LEFT,
     0,                                                                                                                                                                                                         225,
     46,                                                                                                                                                                                                                 28,
     4,                                                                                                                                                                                                                         3,
     ( 1 << 4 ),
     ( 1 << 3 ),
     3                                                                                                                                                                                                                                                      },
    {"+\\divisionslash\\minus",
     "W",                                                  "\\math_notequal",
     "\\math_equal",                                                                           NULL,
     UI_COLOR_WHITE,                                                                                                  12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                                        UI_SHAPE_BUTTON_NORMAL,
     12.0,                                                                                                                                                                        UI_LAYOUT_LEFT,
     59,                                                                                                                                                                                                        225,
     46,                                                                                                                                                                                                                 28,
     3,                                                                                                                                                                                                                         3,
     ( 1 << 3 ),
     ( 1 << 3 ),
     3                                                                                                                                                                                                                                                      },
    {"X",
     "X",                                                  "\\math_lessequal",
     "\\math_less",                                                                            NULL,
     UI_COLOR_WHITE,                                                                                                  12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                                        UI_SHAPE_BUTTON_NORMAL,
     12.0,                                                                                                                                                                        UI_LAYOUT_LEFT,
     119,                                                                                                                                                                                                       225,
     46,                                                                                                                                                                                                                 28,
     2,                                                                                                                                                                                                                         3,
     ( 1 << 2 ),
     ( 1 << 3 ),
     3                                                                                                                                                                                                                                                      },
    {"1/X",
     "Y",                                                  "\\math_greaterequal",
     "\\math_greater",                                                                         NULL,
     UI_COLOR_WHITE,                                                                                                  12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                                        UI_SHAPE_BUTTON_NORMAL,
     12.0,                                                                                                                                                                        UI_LAYOUT_LEFT,
     179,                                                                                                                                                                                                       225,
     46,                                                                                                                                                                                                                 28,
     1,                                                                                                                                                                                                                         3,
     ( 1 << 1 ),
     ( 1 << 3 ),
     3                                                                                                                                                                                                                                                      },
    {"\\divide",
     "Z",                                                  "ABS",
     "ARG",                                                                                    NULL,
     UI_COLOR_WHITE,                                                                                                  19.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                                        UI_SHAPE_BUTTON_NORMAL,
     12.0,                                                                                                                                                                        UI_LAYOUT_LEFT_NO_SPACE,
     238,                                                                                                                                                                                                       225,
     46,                                                                                                                                                                                                                 28,
     0,                                                                                                                                                                                                                         3,
     ( 1 << 0 ),
     ( 1 << 3 ),
     3                                                                                                                                                                                                                                                      },
    {"ALPHA",                                        "",   "",                      "ENTRY",   NULL, UI_COLOR_BLACK,  12.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_LARGE,  0.0,  0,                       0,   267, 46, 32,
     0,                                                                                                                                                                                                                         0, 0,          0,          4},
    {"7",                                            "",   "",                      "NUM.SLV", NULL, UI_COLOR_WHITE,  19.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_LARGE,  0.0,  0,                       59,  267,
     46,                                                                                                                                                                                                                 32, 3, 4, ( 1 << 3 ), ( 1 << 4 ), 4},
    {"8",                                            NULL, NULL,                    NULL,      NULL, UI_COLOR_WHITE,  19.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_LARGE,  0.0,  0,                       119, 267,
     46,                                                                                                                                                                                                                 32, 2, 4, ( 1 << 2 ), ( 1 << 4 ), 4},
    {"9",                                            "",   "",                      "TIME",    NULL, UI_COLOR_WHITE,  19.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_LARGE,  0.0,  0,                       179, 267,
     46,                                                                                                                                                                                                                 32, 1, 4, ( 1 << 1 ), ( 1 << 4 ), 4},
    {"\\multiply",                                   NULL, "[ ]",                   "\" \"",   NULL, UI_COLOR_WHITE,  19.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_LARGE,  0.0,  0,                       238, 267,
     46,                                                                                                                                                                                                                 32, 0, 4, ( 1 << 0 ), ( 1 << 4 ), 4},
    {"\\uparrowleft",
     NULL,                                                 NULL,
     NULL,                                                                                     NULL,
     UI_COLOR_BLACK,                                                                                                  19.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                                        UI_SHAPE_BUTTON_LARGE,
     0.0,                                                                                                                                                                         0,
     0,                                                                                                                                                                                                         313,
     46,                                                                                                                                                                                                                 32,
     0,                                                                                                                                                                                                                         0,
     0,                                                                                                                                                                                                                                        0,
     5                                                                                                                                                                                                                                                      },
    {"4",                                            NULL, NULL,                    NULL,      NULL, UI_COLOR_WHITE,  19.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_LARGE,  0.0,  0,                       59,  313,
     46,                                                                                                                                                                                                                 32, 3, 5, ( 1 << 3 ), ( 1 << 5 ), 5},
    {"5",                                            NULL, NULL,                    NULL,      NULL, UI_COLOR_WHITE,  19.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_LARGE,  0.0,  0,                       119, 313,
     46,                                                                                                                                                                                                                 32, 2, 5, ( 1 << 2 ), ( 1 << 5 ), 5},
    {"6",                                            NULL, "CONVERT",               "UNITS",   NULL, UI_COLOR_WHITE,  19.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_LARGE,  0.0,  0,                       179, 313,
     46,                                                                                                                                                                                                                 32, 1, 5, ( 1 << 1 ), ( 1 << 5 ), 5},
    {"\\minus",                                      NULL, "( )",                   "_",       NULL, UI_COLOR_WHITE,  19.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_LARGE,  0.0,  0,                       238, 313,
     46,                                                                                                                                                                                                                 32, 0, 5, ( 1 << 0 ), ( 1 << 5 ), 5},
    {"\\uparrowright",
     NULL,                                                 NULL,
     NULL,                                                                                     NULL,
     UI_COLOR_BLACK,                                                                                                  19.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                                        UI_SHAPE_BUTTON_LARGE,
     0.0,                                                                                                                                                                         0,
     0,                                                                                                                                                                                                         359,
     46,                                                                                                                                                                                                                 32,
     0,                                                                                                                                                                                                                         0,
     0,                                                                                                                                                                                                                                        0,
     6                                                                                                                                                                                                                                                      },
    {"1",                                            "",   "ARITH",                 "CMPLX",   NULL, UI_COLOR_WHITE,  19.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_LARGE,  0.0,  0,                       59,  359,
     46,                                                                                                                                                                                                                 32, 3, 6, ( 1 << 3 ), ( 1 << 6 ), 6},
    {"2",                                            "",   "",                      "LIB",     NULL, UI_COLOR_WHITE,  19.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_LARGE,  0.0,  0,                       119, 359,
     46,                                                                                                                                                                                                                 32, 2, 6, ( 1 << 2 ), ( 1 << 6 ), 6},
    {"3",
     NULL,                                                 "\\math_numbersign",
     "BASE",                                                                                   NULL,
     UI_COLOR_WHITE,                                                                                                  19.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                                        UI_SHAPE_BUTTON_LARGE,
     0.0,                                                                                                                                                                         0,
     179,                                                                                                                                                                                                       359,
     46,                                                                                                                                                                                                                 32,
     1,                                                                                                                                                                                                                         6,
     ( 1 << 1 ),
     ( 1 << 6 ),
     6                                                                                                                                                                                                                                                      },
    {"+",
     NULL,                                                 "{ }",
     "\\guillemotleft\\ \\guillemotright",                                                     NULL,
     UI_COLOR_WHITE,                                                                                                  19.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                                        UI_SHAPE_BUTTON_LARGE,
     0.0,                                                                                                                                                                         0,
     238,                                                                                                                                                                                                       359,
     46,                                                                                                                                                                                                                 32,
     0,                                                                                                                                                                                                                         6,
     ( 1 << 0 ),
     ( 1 << 6 ),
     6                                                                                                                                                                                                                                                      },
    {"ON",                                           NULL, NULL,                    "OFF",     NULL, UI_COLOR_WHITE,  12.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_LARGE,  0.0,  0,                       0,   405, 46, 32,
     0,                                                                                                                                                                                                                         0, 0,          0,          0},
    {"0",
     NULL,                                                 "\\math_infinity",
     "\\math_arrowright",                                                                      NULL,
     UI_COLOR_WHITE,                                                                                                  19.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                                        UI_SHAPE_BUTTON_LARGE,
     0.0,                                                                                                                                                                         0,
     59,                                                                                                                                                                                                        405,
     46,                                                                                                                                                                                                                 32,
     3,                                                                                                                                                                                                                         7,
     ( 1 << 3 ),
     ( 1 << 7 ),
     7                                                                                                                                                                                                                                                      },
    {"\\bullet",
     NULL,                                                 ": :",
     "\\math_downarrowleft",                                                                   NULL,
     UI_COLOR_WHITE,                                                                                                  19.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                                        UI_SHAPE_BUTTON_LARGE,
     0.0,                                                                                                                                                                         0,
     119,                                                                                                                                                                                                       405,
     46,                                                                                                                                                                                                                 32,
     2,                                                                                                                                                                                                                         7,
     ( 1 << 2 ),
     ( 1 << 7 ),
     7                                                                                                                                                                                                                                                      },
    {"SPC",
     NULL,                                                 "\\math_pi",
     "\\large_comma",                                                                          NULL,
     UI_COLOR_WHITE,                                                                                                  12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                                        UI_SHAPE_BUTTON_LARGE,
     0.0,                                                                                                                                                                         0,
     179,                                                                                                                                                                                                       405,
     46,                                                                                                                                                                                                                 32,
     1,                                                                                                                                                                                                                         7,
     ( 1 << 1 ),
     ( 1 << 7 ),
     7                                                                                                                                                                                                                                                      },
    {"ENTER",
     NULL,                                                 NULL,
     "\\arrowright NUM",                                                                       NULL,
     UI_COLOR_WHITE,                                                                                                  12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                                        UI_SHAPE_BUTTON_LARGE,
     0.0,                                                                                                                                                                         0,
     238,                                                                                                                                                                                                       405,
     46,                                                                                                                                                                                                                 32,
     0,                                                                                                                                                                                                                         7,
     ( 1 << 0 ),
     ( 1 << 7 ),
     7                                                                                                                                                                                                                                                      },
};

#define X49GP_NEWRPL_UI_NR_KEYS ( sizeof( x49gp_newrpl_ui_keys ) / sizeof( x49gp_newrpl_ui_keys[ 0 ] ) )

static const x49gp_ui_key_t x50g_ui_keys[] = {
    {"F1",                              "A",  "Y=",                    NULL,      NULL,     UI_COLOR_BLACK,  12.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_TINY,   12.0, UI_LAYOUT_LEFT,          0,   0,
     36,                                                                                                                                                                                                        22, 5, 1, ( 1 << 5 ), ( 1 << 1 ), 1},
    {"F2",                              "B",  "WIN",                   NULL,      NULL,     UI_COLOR_BLACK,  12.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_TINY,   12.0, UI_LAYOUT_LEFT,          50,  0,
     36,                                                                                                                                                                                                        22, 5, 2, ( 1 << 5 ), ( 1 << 2 ), 2},
    {"F3",                              "C",  "GRAPH",                 NULL,      NULL,     UI_COLOR_BLACK,  12.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_TINY,   12.0, UI_LAYOUT_LEFT,          99,  0,
     36,                                                                                                                                                                                                        22, 5, 3, ( 1 << 5 ), ( 1 << 3 ), 3},
    {"F4",
     "D",                                     "2D/3D",
     NULL,                                                                        NULL,
     UI_COLOR_BLACK,                                                                                         12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                               UI_SHAPE_BUTTON_TINY,
     12.0,                                                                                                                                                               UI_LAYOUT_LEFT,
     149,                                                                                                                                                                                              0,
     36,                                                                                                                                                                                                        22,
     5,                                                                                                                                                                                                                4,
     ( 1 << 5 ),
     ( 1 << 4 ),
     4                                                                                                                                                                                                                                             },
    {"F5",
     "E",                                     "TBLSET",
     NULL,                                                                        NULL,
     UI_COLOR_BLACK,                                                                                         12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                               UI_SHAPE_BUTTON_TINY,
     12.0,                                                                                                                                                               UI_LAYOUT_LEFT,
     198,                                                                                                                                                                                              0,
     36,                                                                                                                                                                                                        22,
     5,                                                                                                                                                                                                                5,
     ( 1 << 5 ),
     ( 1 << 5 ),
     5                                                                                                                                                                                                                                             },
    {"F6",
     "F",                                     "TABLE",
     NULL,                                                                        NULL,
     UI_COLOR_BLACK,                                                                                         12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                               UI_SHAPE_BUTTON_TINY,
     12.0,                                                                                                                                                               UI_LAYOUT_LEFT,
     247,                                                                                                                                                                                              0,
     36,                                                                                                                                                                                                        22,
     5,                                                                                                                                                                                                                6,
     ( 1 << 5 ),
     ( 1 << 6 ),
     6                                                                                                                                                                                                                                             },
    {"APPS",
     "G",                                     "FILES",
     "BEGIN",                                                                     NULL,
     UI_COLOR_WHITE,                                                                                         10.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                               UI_SHAPE_BUTTON_SMALL,
     10.0,                                                                                                                                                               UI_LAYOUT_BELOW,
     0,                                                                                                                                                                                                44,
     36,                                                                                                                                                                                                        28,
     5,                                                                                                                                                                                                                7,
     ( 1 << 5 ),
     ( 1 << 7 ),
     7                                                                                                                                                                                                                                             },
    {"MODE",
     "H",                                     "CUSTOM",
     "END",                                                                       NULL,
     UI_COLOR_WHITE,                                                                                         10.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                               UI_SHAPE_BUTTON_SMALL,
     10.0,                                                                                                                                                               UI_LAYOUT_BELOW,
     50,                                                                                                                                                                                               44,
     36,                                                                                                                                                                                                        28,
     6,                                                                                                                                                                                                                5,
     ( 1 << 6 ),
     ( 1 << 5 ),
     5                                                                                                                                                                                                                                             },
    {"TOOL",                            "I",  "i",                     "I",       NULL,     UI_COLOR_WHITE,  10.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_SMALL,  10.0, UI_LAYOUT_BELOW,         99,  44,
     36,                                                                                                                                                                                                        28, 6, 6, ( 1 << 6 ), ( 1 << 6 ), 6},
    {"V\\kern-1 AR",
     "J",                                     "UPDIR",
     "COPY",                                                                      NULL,
     UI_COLOR_WHITE,                                                                                         10.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                               UI_SHAPE_BUTTON_SMALL,
     10.0,                                                                                                                                                               UI_LAYOUT_BELOW,
     0,                                                                                                                                                                                                92,
     36,                                                                                                                                                                                                        28,
     6,                                                                                                                                                                                                                7,
     ( 1 << 6 ),
     ( 1 << 7 ),
     7                                                                                                                                                                                                                                             },
    {"STO \\triangleright",
     "K",                                     "RCL",
     "CUT",                                                                       NULL,
     UI_COLOR_WHITE,                                                                                         10.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                               UI_SHAPE_BUTTON_SMALL,
     10.0,                                                                                                                                                               UI_LAYOUT_BELOW,
     50,                                                                                                                                                                                               92,
     36,                                                                                                                                                                                                        28,
     7,                                                                                                                                                                                                                1,
     ( 1 << 7 ),
     ( 1 << 1 ),
     1                                                                                                                                                                                                                                             },
    {"NXT",
     "L",                                     "PREV",
     "PASTE",                                                                     NULL,
     UI_COLOR_WHITE,                                                                                         10.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                               UI_SHAPE_BUTTON_SMALL,
     10.0,                                                                                                                                                               UI_LAYOUT_BELOW,
     99,                                                                                                                                                                                               92,
     36,                                                                                                                                                                                                        28,
     7,                                                                                                                                                                                                                2,
     ( 1 << 7 ),
     ( 1 << 2 ),
     2                                                                                                                                                                                                                                             },
    {NULL,                              NULL, NULL,                    NULL,      NULL,     UI_COLOR_SILVER, 0.0,  0,                      UI_SHAPE_BUTTON_ROUND,  0.0,  0,                       200,
     38,                                                                                                                                                                                                    33, 33, 6, 1, ( 1 << 6 ), ( 1 << 1 ), 1},
    {NULL,                              NULL, NULL,                    NULL,      NULL,     UI_COLOR_SILVER, 0.0,  0,                      UI_SHAPE_BUTTON_ROUND,  0.0,  0,                       164,
     66,                                                                                                                                                                                                    33, 33, 6, 2, ( 1 << 6 ), ( 1 << 2 ), 2},
    {NULL,                              NULL, NULL,                    NULL,      NULL,     UI_COLOR_SILVER, 0.0,  0,                      UI_SHAPE_BUTTON_ROUND,  0.0,  0,                       200,
     94,                                                                                                                                                                                                    33, 33, 6, 3, ( 1 << 6 ), ( 1 << 3 ), 3},
    {NULL,                              NULL, NULL,                    NULL,      NULL,     UI_COLOR_SILVER, 0.0,  0,                      UI_SHAPE_BUTTON_ROUND,  0.0,  0,                       236,
     66,                                                                                                                                                                                                    33, 33, 6, 4, ( 1 << 6 ), ( 1 << 4 ), 4},
    {"HIST",
     "M",                                     "CMD",
     "UNDO",                                                                      NULL,
     UI_COLOR_WHITE,                                                                                         12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                               UI_SHAPE_BUTTON_NORMAL,
     12.0,                                                                                                                                                               UI_LAYOUT_LEFT,
     0,                                                                                                                                                                                                141,
     46,                                                                                                                                                                                                        28,
     4,                                                                                                                                                                                                                1,
     ( 1 << 4 ),
     ( 1 << 1 ),
     1                                                                                                                                                                                                                                             },
    {"EV\\kern-1 AL",
     "N",                                     "PRG",
     "CHARS",                                                                     NULL,
     UI_COLOR_WHITE,                                                                                         12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                               UI_SHAPE_BUTTON_NORMAL,
     12.0,                                                                                                                                                               UI_LAYOUT_LEFT,
     59,                                                                                                                                                                                               141,
     46,                                                                                                                                                                                                        28,
     3,                                                                                                                                                                                                                1,
     ( 1 << 3 ),
     ( 1 << 1 ),
     1                                                                                                                                                                                                                                             },
    {"\\tick",
     "O",                                     "MTRW",
     "EQW",                                                                       NULL,
     UI_COLOR_WHITE,                                                                                         12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                               UI_SHAPE_BUTTON_NORMAL,
     12.0,                                                                                                                                                               UI_LAYOUT_LEFT,
     119,                                                                                                                                                                                              141,
     46,                                                                                                                                                                                                        28,
     2,                                                                                                                                                                                                                1,
     ( 1 << 2 ),
     ( 1 << 1 ),
     1                                                                                                                                                                                                                                             },
    {"S\\kern-1 Y\\kern-1 M\\kern-1 B",
     "P",                                     "MTH",
     "CAT",                                                                       NULL,
     UI_COLOR_WHITE,                                                                                         12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                               UI_SHAPE_BUTTON_NORMAL,
     12.0,                                                                                                                                                               UI_LAYOUT_LEFT,
     179,                                                                                                                                                                                              141,
     46,                                                                                                                                                                                                        28,
     1,                                                                                                                                                                                                                1,
     ( 1 << 1 ),
     ( 1 << 1 ),
     1                                                                                                                                                                                                                                             },
    {"\\arrowleftdblfull",
     NULL,                                    "DEL",
     "CLEAR",                                                                     NULL,
     UI_COLOR_WHITE,                                                                                         12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                               UI_SHAPE_BUTTON_NORMAL,
     0.0,                                                                                                                                                                0,
     238,                                                                                                                                                                                              141,
     46,                                                                                                                                                                                                        28,
     0,                                                                                                                                                                                                                1,
     ( 1 << 0 ),
     ( 1 << 1 ),
     1                                                                                                                                                                                                                                             },
    {"Y\\super x\\/super",
     "Q",                                     "\\math_e\\xsuperior",
     "LN",                                                                        NULL,
     UI_COLOR_WHITE,                                                                                         12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                               UI_SHAPE_BUTTON_NORMAL,
     12.0,                                                                                                                                                               UI_LAYOUT_LEFT,
     0,                                                                                                                                                                                                183,
     46,                                                                                                                                                                                                        28,
     4,                                                                                                                                                                                                                2,
     ( 1 << 4 ),
     ( 1 << 2 ),
     2                                                                                                                                                                                                                                             },
    {"\\radical\\overscore\\kern-7 X",
     "R",                                     "\\math_x\\twosuperior",
     "\\xsuperior\\kern-4\\math_radical\\overscore\\kern-5\\math_y",              NULL,
     UI_COLOR_WHITE,                                                                                         12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                               UI_SHAPE_BUTTON_NORMAL,
     12.0,                                                                                                                                                               UI_LAYOUT_LEFT,
     59,                                                                                                                                                                                               183,
     46,                                                                                                                                                                                                        28,
     3,                                                                                                                                                                                                                2,
     ( 1 << 3 ),
     ( 1 << 2 ),
     2                                                                                                                                                                                                                                             },
    {"SIN",
     "S",                                     "ASIN",
     "\\math_summation",                                                          NULL,
     UI_COLOR_WHITE,                                                                                         12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                               UI_SHAPE_BUTTON_NORMAL,
     12.0,                                                                                                                                                               UI_LAYOUT_LEFT,
     119,                                                                                                                                                                                              183,
     46,                                                                                                                                                                                                        28,
     2,                                                                                                                                                                                                                2,
     ( 1 << 2 ),
     ( 1 << 2 ),
     2                                                                                                                                                                                                                                             },
    {"COS",
     "T",                                     "ACOS",
     "\\math_partialdiff",                                                        NULL,
     UI_COLOR_WHITE,                                                                                         12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                               UI_SHAPE_BUTTON_NORMAL,
     12.0,                                                                                                                                                               UI_LAYOUT_LEFT,
     179,                                                                                                                                                                                              183,
     46,                                                                                                                                                                                                        28,
     1,                                                                                                                                                                                                                2,
     ( 1 << 1 ),
     ( 1 << 2 ),
     2                                                                                                                                                                                                                                             },
    {"TAN",
     "U",                                     "ATAN",
     "\\math_integral",                                                           NULL,
     UI_COLOR_WHITE,                                                                                         12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                               UI_SHAPE_BUTTON_NORMAL,
     12.0,                                                                                                                                                               UI_LAYOUT_LEFT,
     238,                                                                                                                                                                                              183,
     46,                                                                                                                                                                                                        28,
     0,                                                                                                                                                                                                                2,
     ( 1 << 0 ),
     ( 1 << 2 ),
     2                                                                                                                                                                                                                                             },
    {"EEX",
     "V",                                     "10\\xsuperior",
     "LOG",                                                                       NULL,
     UI_COLOR_WHITE,                                                                                         12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                               UI_SHAPE_BUTTON_NORMAL,
     12.0,                                                                                                                                                               UI_LAYOUT_LEFT,
     0,                                                                                                                                                                                                225,
     46,                                                                                                                                                                                                        28,
     4,                                                                                                                                                                                                                3,
     ( 1 << 4 ),
     ( 1 << 3 ),
     3                                                                                                                                                                                                                                             },
    {"+\\divisionslash\\minus",
     "W",                                     "\\math_notequal",
     "\\math_equal",                                                              NULL,
     UI_COLOR_WHITE,                                                                                         12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                               UI_SHAPE_BUTTON_NORMAL,
     12.0,                                                                                                                                                               UI_LAYOUT_LEFT,
     59,                                                                                                                                                                                               225,
     46,                                                                                                                                                                                                        28,
     3,                                                                                                                                                                                                                3,
     ( 1 << 3 ),
     ( 1 << 3 ),
     3                                                                                                                                                                                                                                             },
    {"X",
     "X",                                     "\\math_lessequal",
     "\\math_less",                                                               NULL,
     UI_COLOR_WHITE,                                                                                         12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                               UI_SHAPE_BUTTON_NORMAL,
     12.0,                                                                                                                                                               UI_LAYOUT_LEFT,
     119,                                                                                                                                                                                              225,
     46,                                                                                                                                                                                                        28,
     2,                                                                                                                                                                                                                3,
     ( 1 << 2 ),
     ( 1 << 3 ),
     3                                                                                                                                                                                                                                             },
    {"1/X",
     "Y",                                     "\\math_greaterequal",
     "\\math_greater",                                                            NULL,
     UI_COLOR_WHITE,                                                                                         12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                               UI_SHAPE_BUTTON_NORMAL,
     12.0,                                                                                                                                                               UI_LAYOUT_LEFT,
     179,                                                                                                                                                                                              225,
     46,                                                                                                                                                                                                        28,
     1,                                                                                                                                                                                                                3,
     ( 1 << 1 ),
     ( 1 << 3 ),
     3                                                                                                                                                                                                                                             },
    {"\\divide",
     "Z",                                     "ABS",
     "ARG",                                                                       NULL,
     UI_COLOR_WHITE,                                                                                         19.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                               UI_SHAPE_BUTTON_NORMAL,
     12.0,                                                                                                                                                               UI_LAYOUT_LEFT_NO_SPACE,
     238,                                                                                                                                                                                              225,
     46,                                                                                                                                                                                                        28,
     0,                                                                                                                                                                                                                3,
     ( 1 << 0 ),
     ( 1 << 3 ),
     3                                                                                                                                                                                                                                             },
    {"ALPHA",                           NULL, "USER",                  "ENTRY",   NULL,     UI_COLOR_BLACK,  12.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_LARGE,  0.0,  0,                       0,   267, 46, 32,
     0,                                                                                                                                                                                                                0, 0,          0,          4},
    {"7",                               NULL, "S.SLV",                 "NUM.SLV", NULL,     UI_COLOR_WHITE,  19.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_LARGE,  0.0,  0,                       59,  267,
     46,                                                                                                                                                                                                        32, 3, 4, ( 1 << 3 ), ( 1 << 4 ), 4},
    {"8",                               NULL, "EXP&LN",                "TRIG",    NULL,     UI_COLOR_WHITE,  19.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_LARGE,  0.0,  0,                       119, 267,
     46,                                                                                                                                                                                                        32, 2, 4, ( 1 << 2 ), ( 1 << 4 ), 4},
    {"9",                               NULL, "FINANCE",               "TIME",    NULL,     UI_COLOR_WHITE,  19.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_LARGE,  0.0,  0,                       179, 267,
     46,                                                                                                                                                                                                        32, 1, 4, ( 1 << 1 ), ( 1 << 4 ), 4},
    {"\\multiply",                      NULL, "[ ]",                   "\" \"",   NULL,     UI_COLOR_WHITE,  19.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_LARGE,  0.0,  0,                       238, 267,
     46,                                                                                                                                                                                                        32, 0, 4, ( 1 << 0 ), ( 1 << 4 ), 4},
    {"\\uparrowleft",
     NULL,                                    NULL,
     NULL,                                                                        NULL,
     UI_COLOR_BLACK,                                                                                         19.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                               UI_SHAPE_BUTTON_LARGE,
     0.0,                                                                                                                                                                0,
     0,                                                                                                                                                                                                313,
     46,                                                                                                                                                                                                        32,
     0,                                                                                                                                                                                                                0,
     0,                                                                                                                                                                                                                               0,
     5                                                                                                                                                                                                                                             },
    {"4",                               NULL, "CALC",                  "ALG",     NULL,     UI_COLOR_WHITE,  19.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_LARGE,  0.0,  0,                       59,  313,
     46,                                                                                                                                                                                                        32, 3, 5, ( 1 << 3 ), ( 1 << 5 ), 5},
    {"5",                               NULL, "MATRICES",              "STAT",    NULL,     UI_COLOR_WHITE,  19.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_LARGE,  0.0,  0,                       119, 313,
     46,                                                                                                                                                                                                        32, 2, 5, ( 1 << 2 ), ( 1 << 5 ), 5},
    {"6",                               NULL, "CONVERT",               "UNITS",   NULL,     UI_COLOR_WHITE,  19.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_LARGE,  0.0,  0,                       179, 313,
     46,                                                                                                                                                                                                        32, 1, 5, ( 1 << 1 ), ( 1 << 5 ), 5},
    {"\\minus",                         NULL, "( )",                   "_",       NULL,     UI_COLOR_WHITE,  19.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_LARGE,  0.0,  0,                       238, 313,
     46,                                                                                                                                                                                                        32, 0, 5, ( 1 << 0 ), ( 1 << 5 ), 5},
    {"\\uparrowright",
     NULL,                                    NULL,
     NULL,                                                                        NULL,
     UI_COLOR_BLACK,                                                                                         19.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                               UI_SHAPE_BUTTON_LARGE,
     0.0,                                                                                                                                                                0,
     0,                                                                                                                                                                                                359,
     46,                                                                                                                                                                                                        32,
     0,                                                                                                                                                                                                                0,
     0,                                                                                                                                                                                                                               0,
     6                                                                                                                                                                                                                                             },
    {"1",                               NULL, "ARITH",                 "CMPLX",   NULL,     UI_COLOR_WHITE,  19.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_LARGE,  0.0,  0,                       59,  359,
     46,                                                                                                                                                                                                        32, 3, 6, ( 1 << 3 ), ( 1 << 6 ), 6},
    {"2",                               NULL, "DEF",                   "LIB",     NULL,     UI_COLOR_WHITE,  19.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_LARGE,  0.0,  0,                       119, 359,
     46,                                                                                                                                                                                                        32, 2, 6, ( 1 << 2 ), ( 1 << 6 ), 6},
    {"3",
     NULL,                                    "\\math_numbersign",
     "BASE",                                                                      NULL,
     UI_COLOR_WHITE,                                                                                         19.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                               UI_SHAPE_BUTTON_LARGE,
     0.0,                                                                                                                                                                0,
     179,                                                                                                                                                                                              359,
     46,                                                                                                                                                                                                        32,
     1,                                                                                                                                                                                                                6,
     ( 1 << 1 ),
     ( 1 << 6 ),
     6                                                                                                                                                                                                                                             },
    {"+",
     NULL,                                    "{ }",
     "\\guillemotleft\\ \\guillemotright",                                        NULL,
     UI_COLOR_WHITE,                                                                                         19.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                               UI_SHAPE_BUTTON_LARGE,
     0.0,                                                                                                                                                                0,
     238,                                                                                                                                                                                              359,
     46,                                                                                                                                                                                                        32,
     0,                                                                                                                                                                                                                6,
     ( 1 << 0 ),
     ( 1 << 6 ),
     6                                                                                                                                                                                                                                             },
    {"ON",                              NULL, "CONT",                  "OFF",     "CANCEL", UI_COLOR_WHITE,  12.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_LARGE,  0.0,  0,                       0,   405, 46, 32,
     0,                                                                                                                                                                                                                0, 0,          0,          0},
    {"0",
     NULL,                                    "\\math_infinity",
     "\\math_arrowright",                                                         NULL,
     UI_COLOR_WHITE,                                                                                         19.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                               UI_SHAPE_BUTTON_LARGE,
     0.0,                                                                                                                                                                0,
     59,                                                                                                                                                                                               405,
     46,                                                                                                                                                                                                        32,
     3,                                                                                                                                                                                                                7,
     ( 1 << 3 ),
     ( 1 << 7 ),
     7                                                                                                                                                                                                                                             },
    {"\\bullet",
     NULL,                                    ": :",
     "\\math_downarrowleft",                                                      NULL,
     UI_COLOR_WHITE,                                                                                         19.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                               UI_SHAPE_BUTTON_LARGE,
     0.0,                                                                                                                                                                0,
     119,                                                                                                                                                                                              405,
     46,                                                                                                                                                                                                        32,
     2,                                                                                                                                                                                                                7,
     ( 1 << 2 ),
     ( 1 << 7 ),
     7                                                                                                                                                                                                                                             },
    {"SPC",
     NULL,                                    "\\math_pi",
     "\\large_comma",                                                             NULL,
     UI_COLOR_WHITE,                                                                                         12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                               UI_SHAPE_BUTTON_LARGE,
     0.0,                                                                                                                                                                0,
     179,                                                                                                                                                                                              405,
     46,                                                                                                                                                                                                        32,
     1,                                                                                                                                                                                                                7,
     ( 1 << 1 ),
     ( 1 << 7 ),
     7                                                                                                                                                                                                                                             },
    {"ENTER",
     NULL,                                    "ANS",
     "\\arrowright NUM",                                                          NULL,
     UI_COLOR_WHITE,                                                                                         12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                               UI_SHAPE_BUTTON_LARGE,
     0.0,                                                                                                                                                                0,
     238,                                                                                                                                                                                              405,
     46,                                                                                                                                                                                                        32,
     0,                                                                                                                                                                                                                7,
     ( 1 << 0 ),
     ( 1 << 7 ),
     7                                                                                                                                                                                                                                             },
};
#define X50G_UI_NR_KEYS ( sizeof( x50g_ui_keys ) / sizeof( x50g_ui_keys[ 0 ] ) )

static const x49gp_ui_key_t x50g_newrpl_ui_keys[] = {
    {"F1",                                           "A",  NULL,                    NULL,      NULL, UI_COLOR_BLACK,  12.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_TINY,   12.0, UI_LAYOUT_LEFT,          0,   0,
     36,                                                                                                                                                                                                                 22, 5, 1, ( 1 << 5 ), ( 1 << 1 ), 1},
    {"F2",                                           "B",  NULL,                    NULL,      NULL, UI_COLOR_BLACK,  12.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_TINY,   12.0, UI_LAYOUT_LEFT,          50,  0,
     36,                                                                                                                                                                                                                 22, 5, 2, ( 1 << 5 ), ( 1 << 2 ), 2},
    {"F3",                                           "C",  NULL,                    NULL,      NULL, UI_COLOR_BLACK,  12.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_TINY,   12.0, UI_LAYOUT_LEFT,          99,  0,
     36,                                                                                                                                                                                                                 22, 5, 3, ( 1 << 5 ), ( 1 << 3 ), 3},
    {"F4",                                           "D",  NULL,                    NULL,      NULL, UI_COLOR_BLACK,  12.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_TINY,   12.0, UI_LAYOUT_LEFT,          149, 0,
     36,                                                                                                                                                                                                                 22, 5, 4, ( 1 << 5 ), ( 1 << 4 ), 4},
    {"F5",                                           "E",  NULL,                    NULL,      NULL, UI_COLOR_BLACK,  12.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_TINY,   12.0, UI_LAYOUT_LEFT,          198, 0,
     36,                                                                                                                                                                                                                 22, 5, 5, ( 1 << 5 ), ( 1 << 5 ), 5},
    {"F6",                                           "F",  NULL,                    NULL,      NULL, UI_COLOR_BLACK,  12.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_TINY,   12.0, UI_LAYOUT_LEFT,          247, 0,
     36,                                                                                                                                                                                                                 22, 5, 6, ( 1 << 5 ), ( 1 << 6 ), 6},
    {"",                                             "G",  NULL,                    NULL,      NULL, UI_COLOR_WHITE,  10.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_SMALL,  10.0, UI_LAYOUT_BELOW,         0,   44,
     36,                                                                                                                                                                                                                 28, 5, 7, ( 1 << 5 ), ( 1 << 7 ), 7},
    {"",                                             "H",  NULL,                    NULL,      NULL, UI_COLOR_WHITE,  10.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_SMALL,  10.0, UI_LAYOUT_BELOW,         50,  44,
     36,                                                                                                                                                                                                                 28, 6, 5, ( 1 << 6 ), ( 1 << 5 ), 5},
    {"",                                             "I",  NULL,                    NULL,      NULL, UI_COLOR_WHITE,  10.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_SMALL,  10.0, UI_LAYOUT_BELOW,         99,  44,
     36,                                                                                                                                                                                                                 28, 6, 6, ( 1 << 6 ), ( 1 << 6 ), 6},
    {"",                                             "J",  NULL,                    NULL,      NULL, UI_COLOR_WHITE,  10.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_SMALL,  10.0, UI_LAYOUT_BELOW,         0,   92,
     36,                                                                                                                                                                                                                 28, 6, 7, ( 1 << 6 ), ( 1 << 7 ), 7},
    {"",                                             "K",  NULL,                    NULL,      NULL, UI_COLOR_WHITE,  10.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_SMALL,  10.0, UI_LAYOUT_BELOW,         50,  92,
     36,                                                                                                                                                                                                                 28, 7, 1, ( 1 << 7 ), ( 1 << 1 ), 1},
    {"",                                             "L",  NULL,                    NULL,      NULL, UI_COLOR_WHITE,  10.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_SMALL,  10.0, UI_LAYOUT_BELOW,         99,  92,
     36,                                                                                                                                                                                                                 28, 7, 2, ( 1 << 7 ), ( 1 << 2 ), 2},
    {"",                                             "",   "UPDIR",                 NULL,      NULL, UI_COLOR_SILVER, 10.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_ROUND,  0.0,  0,                       200, 38,
     33,                                                                                                                                                                                                                 33, 6, 1, ( 1 << 6 ), ( 1 << 1 ), 1},
    {"",                                             "",   "BEG COPY",              NULL,      NULL, UI_COLOR_SILVER, 10.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_ROUND,  0.0,  0,                       164, 66,
     33,                                                                                                                                                                                                                 33, 6, 2, ( 1 << 6 ), ( 1 << 2 ), 2},
    {"",                                             "",   "CUT",                   NULL,      NULL, UI_COLOR_SILVER, 10.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_ROUND,  0.0,  0,                       200, 94,
     33,                                                                                                                                                                                                                 33, 6, 3, ( 1 << 6 ), ( 1 << 3 ), 3},
    {"",                                             "",   "END PASTE",             NULL,      NULL, UI_COLOR_SILVER, 10.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_ROUND,  0.0,  0,                       236, 66,
     33,                                                                                                                                                                                                                 33, 6, 4, ( 1 << 6 ), ( 1 << 4 ), 4},
    {"S\\kern-1 T\\kern-1 O\\kern-1\\triangleright",
     "M",                                                  "RCL",
     "PREV.M",                                                                                 NULL,
     UI_COLOR_WHITE,                                                                                                  12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                                        UI_SHAPE_BUTTON_NORMAL,
     12.0,                                                                                                                                                                        UI_LAYOUT_LEFT,
     0,                                                                                                                                                                                                         141,
     46,                                                                                                                                                                                                                 28,
     4,                                                                                                                                                                                                                         1,
     ( 1 << 4 ),
     ( 1 << 1 ),
     1                                                                                                                                                                                                                                                      },
    {"EV\\kern-1 AL",
     "N",                                                  NULL,
     NULL,                                                                                     NULL,
     UI_COLOR_WHITE,                                                                                                  12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                                        UI_SHAPE_BUTTON_NORMAL,
     12.0,                                                                                                                                                                        UI_LAYOUT_LEFT,
     59,                                                                                                                                                                                                        141,
     46,                                                                                                                                                                                                                 28,
     3,                                                                                                                                                                                                                         1,
     ( 1 << 3 ),
     ( 1 << 1 ),
     1                                                                                                                                                                                                                                                      },
    {"\\tick",
     "O",                                                  NULL,
     NULL,                                                                                     NULL,
     UI_COLOR_WHITE,                                                                                                  12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                                        UI_SHAPE_BUTTON_NORMAL,
     12.0,                                                                                                                                                                        UI_LAYOUT_LEFT,
     119,                                                                                                                                                                                                       141,
     46,                                                                                                                                                                                                                 28,
     2,                                                                                                                                                                                                                         1,
     ( 1 << 2 ),
     ( 1 << 1 ),
     1                                                                                                                                                                                                                                                      },
    {"M\\kern-1 E\\kern-1 N\\kern-1 U",
     "P",                                                  NULL,
     NULL,                                                                                     NULL,
     UI_COLOR_WHITE,                                                                                                  12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                                        UI_SHAPE_BUTTON_NORMAL,
     12.0,                                                                                                                                                                        UI_LAYOUT_LEFT,
     179,                                                                                                                                                                                                       141,
     46,                                                                                                                                                                                                                 28,
     1,                                                                                                                                                                                                                         1,
     ( 1 << 1 ),
     ( 1 << 1 ),
     1                                                                                                                                                                                                                                                      },
    {"\\arrowleftdblfull",
     NULL,                                                 NULL,
     "CLEAR",                                                                                  NULL,
     UI_COLOR_WHITE,                                                                                                  12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                                        UI_SHAPE_BUTTON_NORMAL,
     0.0,                                                                                                                                                                         0,
     238,                                                                                                                                                                                                       141,
     46,                                                                                                                                                                                                                 28,
     0,                                                                                                                                                                                                                         1,
     ( 1 << 0 ),
     ( 1 << 1 ),
     1                                                                                                                                                                                                                                                      },
    {"Y\\super x\\/super",
     "Q",                                                  "\\math_e\\xsuperior",
     "LN",                                                                                     NULL,
     UI_COLOR_WHITE,                                                                                                  12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                                        UI_SHAPE_BUTTON_NORMAL,
     12.0,                                                                                                                                                                        UI_LAYOUT_LEFT,
     0,                                                                                                                                                                                                         183,
     46,                                                                                                                                                                                                                 28,
     4,                                                                                                                                                                                                                         2,
     ( 1 << 4 ),
     ( 1 << 2 ),
     2                                                                                                                                                                                                                                                      },
    {"\\radical\\overscore\\kern-7 X",
     "R",                                                  "\\math_x\\twosuperior",
     "\\xsuperior\\kern-4\\math_radical\\overscore\\kern-5\\math_y",                           NULL,
     UI_COLOR_WHITE,                                                                                                  12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                                        UI_SHAPE_BUTTON_NORMAL,
     12.0,                                                                                                                                                                        UI_LAYOUT_LEFT,
     59,                                                                                                                                                                                                        183,
     46,                                                                                                                                                                                                                 28,
     3,                                                                                                                                                                                                                         2,
     ( 1 << 3 ),
     ( 1 << 2 ),
     2                                                                                                                                                                                                                                                      },
    {"SIN",
     "S",                                                  "ASIN",
     NULL,                                                                                     NULL,
     UI_COLOR_WHITE,                                                                                                  12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                                        UI_SHAPE_BUTTON_NORMAL,
     12.0,                                                                                                                                                                        UI_LAYOUT_LEFT,
     119,                                                                                                                                                                                                       183,
     46,                                                                                                                                                                                                                 28,
     2,                                                                                                                                                                                                                         2,
     ( 1 << 2 ),
     ( 1 << 2 ),
     2                                                                                                                                                                                                                                                      },
    {"COS",
     "T",                                                  "ACOS",
     NULL,                                                                                     NULL,
     UI_COLOR_WHITE,                                                                                                  12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                                        UI_SHAPE_BUTTON_NORMAL,
     12.0,                                                                                                                                                                        UI_LAYOUT_LEFT,
     179,                                                                                                                                                                                                       183,
     46,                                                                                                                                                                                                                 28,
     1,                                                                                                                                                                                                                         2,
     ( 1 << 1 ),
     ( 1 << 2 ),
     2                                                                                                                                                                                                                                                      },
    {"TAN",
     "U",                                                  "ATAN",
     NULL,                                                                                     NULL,
     UI_COLOR_WHITE,                                                                                                  12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                                        UI_SHAPE_BUTTON_NORMAL,
     12.0,                                                                                                                                                                        UI_LAYOUT_LEFT,
     238,                                                                                                                                                                                                       183,
     46,                                                                                                                                                                                                                 28,
     0,                                                                                                                                                                                                                         2,
     ( 1 << 0 ),
     ( 1 << 2 ),
     2                                                                                                                                                                                                                                                      },
    {"EEX",
     "V",                                                  "10\\xsuperior",
     "LOG",                                                                                    NULL,
     UI_COLOR_WHITE,                                                                                                  12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                                        UI_SHAPE_BUTTON_NORMAL,
     12.0,                                                                                                                                                                        UI_LAYOUT_LEFT,
     0,                                                                                                                                                                                                         225,
     46,                                                                                                                                                                                                                 28,
     4,                                                                                                                                                                                                                         3,
     ( 1 << 4 ),
     ( 1 << 3 ),
     3                                                                                                                                                                                                                                                      },
    {"+\\divisionslash\\minus",
     "W",                                                  "\\math_notequal",
     "\\math_equal",                                                                           NULL,
     UI_COLOR_WHITE,                                                                                                  12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                                        UI_SHAPE_BUTTON_NORMAL,
     12.0,                                                                                                                                                                        UI_LAYOUT_LEFT,
     59,                                                                                                                                                                                                        225,
     46,                                                                                                                                                                                                                 28,
     3,                                                                                                                                                                                                                         3,
     ( 1 << 3 ),
     ( 1 << 3 ),
     3                                                                                                                                                                                                                                                      },
    {"X",
     "X",                                                  "\\math_lessequal",
     "\\math_less",                                                                            NULL,
     UI_COLOR_WHITE,                                                                                                  12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                                        UI_SHAPE_BUTTON_NORMAL,
     12.0,                                                                                                                                                                        UI_LAYOUT_LEFT,
     119,                                                                                                                                                                                                       225,
     46,                                                                                                                                                                                                                 28,
     2,                                                                                                                                                                                                                         3,
     ( 1 << 2 ),
     ( 1 << 3 ),
     3                                                                                                                                                                                                                                                      },
    {"1/X",
     "Y",                                                  "\\math_greaterequal",
     "\\math_greater",                                                                         NULL,
     UI_COLOR_WHITE,                                                                                                  12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                                        UI_SHAPE_BUTTON_NORMAL,
     12.0,                                                                                                                                                                        UI_LAYOUT_LEFT,
     179,                                                                                                                                                                                                       225,
     46,                                                                                                                                                                                                                 28,
     1,                                                                                                                                                                                                                         3,
     ( 1 << 1 ),
     ( 1 << 3 ),
     3                                                                                                                                                                                                                                                      },
    {"\\divide",
     "Z",                                                  "ABS",
     "ARG",                                                                                    NULL,
     UI_COLOR_WHITE,                                                                                                  19.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                                        UI_SHAPE_BUTTON_NORMAL,
     12.0,                                                                                                                                                                        UI_LAYOUT_LEFT_NO_SPACE,
     238,                                                                                                                                                                                                       225,
     46,                                                                                                                                                                                                                 28,
     0,                                                                                                                                                                                                                         3,
     ( 1 << 0 ),
     ( 1 << 3 ),
     3                                                                                                                                                                                                                                                      },
    {"ALPHA",                                        "",   "",                      "ENTRY",   NULL, UI_COLOR_BLACK,  12.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_LARGE,  0.0,  0,                       0,   267, 46, 32,
     0,                                                                                                                                                                                                                         0, 0,          0,          4},
    {"7",                                            "",   "",                      "NUM.SLV", NULL, UI_COLOR_WHITE,  19.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_LARGE,  0.0,  0,                       59,  267,
     46,                                                                                                                                                                                                                 32, 3, 4, ( 1 << 3 ), ( 1 << 4 ), 4},
    {"8",                                            NULL, NULL,                    NULL,      NULL, UI_COLOR_WHITE,  19.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_LARGE,  0.0,  0,                       119, 267,
     46,                                                                                                                                                                                                                 32, 2, 4, ( 1 << 2 ), ( 1 << 4 ), 4},
    {"9",                                            "",   "",                      "TIME",    NULL, UI_COLOR_WHITE,  19.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_LARGE,  0.0,  0,                       179, 267,
     46,                                                                                                                                                                                                                 32, 1, 4, ( 1 << 1 ), ( 1 << 4 ), 4},
    {"\\multiply",                                   NULL, "[ ]",                   "\" \"",   NULL, UI_COLOR_WHITE,  19.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_LARGE,  0.0,  0,                       238, 267,
     46,                                                                                                                                                                                                                 32, 0, 4, ( 1 << 0 ), ( 1 << 4 ), 4},
    {"\\uparrowleft",
     NULL,                                                 NULL,
     NULL,                                                                                     NULL,
     UI_COLOR_BLACK,                                                                                                  19.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                                        UI_SHAPE_BUTTON_LARGE,
     0.0,                                                                                                                                                                         0,
     0,                                                                                                                                                                                                         313,
     46,                                                                                                                                                                                                                 32,
     0,                                                                                                                                                                                                                         0,
     0,                                                                                                                                                                                                                                        0,
     5                                                                                                                                                                                                                                                      },
    {"4",                                            NULL, NULL,                    NULL,      NULL, UI_COLOR_WHITE,  19.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_LARGE,  0.0,  0,                       59,  313,
     46,                                                                                                                                                                                                                 32, 3, 5, ( 1 << 3 ), ( 1 << 5 ), 5},
    {"5",                                            NULL, NULL,                    NULL,      NULL, UI_COLOR_WHITE,  19.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_LARGE,  0.0,  0,                       119, 313,
     46,                                                                                                                                                                                                                 32, 2, 5, ( 1 << 2 ), ( 1 << 5 ), 5},
    {"6",                                            NULL, "CONVERT",               "UNITS",   NULL, UI_COLOR_WHITE,  19.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_LARGE,  0.0,  0,                       179, 313,
     46,                                                                                                                                                                                                                 32, 1, 5, ( 1 << 1 ), ( 1 << 5 ), 5},
    {"\\minus",                                      NULL, "( )",                   "_",       NULL, UI_COLOR_WHITE,  19.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_LARGE,  0.0,  0,                       238, 313,
     46,                                                                                                                                                                                                                 32, 0, 5, ( 1 << 0 ), ( 1 << 5 ), 5},
    {"\\uparrowright",
     NULL,                                                 NULL,
     NULL,                                                                                     NULL,
     UI_COLOR_BLACK,                                                                                                  19.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                                        UI_SHAPE_BUTTON_LARGE,
     0.0,                                                                                                                                                                         0,
     0,                                                                                                                                                                                                         359,
     46,                                                                                                                                                                                                                 32,
     0,                                                                                                                                                                                                                         0,
     0,                                                                                                                                                                                                                                        0,
     6                                                                                                                                                                                                                                                      },
    {"1",                                            "",   "ARITH",                 "CMPLX",   NULL, UI_COLOR_WHITE,  19.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_LARGE,  0.0,  0,                       59,  359,
     46,                                                                                                                                                                                                                 32, 3, 6, ( 1 << 3 ), ( 1 << 6 ), 6},
    {"2",                                            "",   "",                      "LIB",     NULL, UI_COLOR_WHITE,  19.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_LARGE,  0.0,  0,                       119, 359,
     46,                                                                                                                                                                                                                 32, 2, 6, ( 1 << 2 ), ( 1 << 6 ), 6},
    {"3",
     NULL,                                                 "\\math_numbersign",
     "BASE",                                                                                   NULL,
     UI_COLOR_WHITE,                                                                                                  19.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                                        UI_SHAPE_BUTTON_LARGE,
     0.0,                                                                                                                                                                         0,
     179,                                                                                                                                                                                                       359,
     46,                                                                                                                                                                                                                 32,
     1,                                                                                                                                                                                                                         6,
     ( 1 << 1 ),
     ( 1 << 6 ),
     6                                                                                                                                                                                                                                                      },
    {"+",
     NULL,                                                 "{ }",
     "\\guillemotleft\\ \\guillemotright",                                                     NULL,
     UI_COLOR_WHITE,                                                                                                  19.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                                        UI_SHAPE_BUTTON_LARGE,
     0.0,                                                                                                                                                                         0,
     238,                                                                                                                                                                                                       359,
     46,                                                                                                                                                                                                                 32,
     0,                                                                                                                                                                                                                         6,
     ( 1 << 0 ),
     ( 1 << 6 ),
     6                                                                                                                                                                                                                                                      },
    {"ON",                                           NULL, NULL,                    "OFF",     NULL, UI_COLOR_WHITE,  12.0, CAIRO_FONT_WEIGHT_BOLD, UI_SHAPE_BUTTON_LARGE,  0.0,  0,                       0,   405, 46, 32,
     0,                                                                                                                                                                                                                         0, 0,          0,          0},
    {"0",
     NULL,                                                 "\\math_infinity",
     "\\math_arrowright",                                                                      NULL,
     UI_COLOR_WHITE,                                                                                                  19.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                                        UI_SHAPE_BUTTON_LARGE,
     0.0,                                                                                                                                                                         0,
     59,                                                                                                                                                                                                        405,
     46,                                                                                                                                                                                                                 32,
     3,                                                                                                                                                                                                                         7,
     ( 1 << 3 ),
     ( 1 << 7 ),
     7                                                                                                                                                                                                                                                      },
    {"\\bullet",
     NULL,                                                 ": :",
     "\\math_downarrowleft",                                                                   NULL,
     UI_COLOR_WHITE,                                                                                                  19.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                                        UI_SHAPE_BUTTON_LARGE,
     0.0,                                                                                                                                                                         0,
     119,                                                                                                                                                                                                       405,
     46,                                                                                                                                                                                                                 32,
     2,                                                                                                                                                                                                                         7,
     ( 1 << 2 ),
     ( 1 << 7 ),
     7                                                                                                                                                                                                                                                      },
    {"SPC",
     NULL,                                                 "\\math_pi",
     "\\large_comma",                                                                          NULL,
     UI_COLOR_WHITE,                                                                                                  12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                                        UI_SHAPE_BUTTON_LARGE,
     0.0,                                                                                                                                                                         0,
     179,                                                                                                                                                                                                       405,
     46,                                                                                                                                                                                                                 32,
     1,                                                                                                                                                                                                                         7,
     ( 1 << 1 ),
     ( 1 << 7 ),
     7                                                                                                                                                                                                                                                      },
    {"ENTER",
     NULL,                                                 NULL,
     "\\arrowright NUM",                                                                       NULL,
     UI_COLOR_WHITE,                                                                                                  12.0,
     CAIRO_FONT_WEIGHT_BOLD,                                                                                                                        UI_SHAPE_BUTTON_LARGE,
     0.0,                                                                                                                                                                         0,
     238,                                                                                                                                                                                                       405,
     46,                                                                                                                                                                                                                 32,
     0,                                                                                                                                                                                                                         7,
     ( 1 << 0 ),
     ( 1 << 7 ),
     7                                                                                                                                                                                                                                                      },
};
#define X50G_NEWRPL_UI_NR_KEYS ( sizeof( x50g_newrpl_ui_keys ) / sizeof( x50g_newrpl_ui_keys[ 0 ] ) )

static void x49gp_ui_color_init( GdkColor* color, u8 red, u8 green, u8 blue )
{
    color->red = ( red << 8 ) | red;
    color->green = ( green << 8 ) | green;
    color->blue = ( blue << 8 ) | blue;
}

static void x49gp_ui_style_init( GtkStyle* style, GtkWidget* widget, GdkColor* fg, GdkColor* bg )
{
    for ( int i = 0; i < 5; i++ ) {
        style->fg[ i ] = *fg;
        style->bg[ i ] = *bg;

        style->text[ i ] = style->fg[ i ];
        style->base[ i ] = style->bg[ i ];
    }

    style->xthickness = 0;
    style->ythickness = 0;
}

static const uint32_t PIXBUF_MAGIC = 0x47646b50;

typedef struct {
    uint32_t magic;
    uint32_t length;
    uint32_t pixdata_type;
    uint32_t rowstride;
    uint32_t width;
    uint32_t height;
} pixbuf_inline_data_t;

static GdkPixbuf* x49gp_pixbuf_new_from_inline( gint data_length, const guint8* data )
{
    const pixbuf_inline_data_t* pbd = ( const pixbuf_inline_data_t* )data;
    uint32_t magic, rowstride, width, height, length;
    const guint8* pixdata;

    magic = ntohl( pbd->magic );
    if ( magic != PIXBUF_MAGIC )
        return NULL;

    length = ntohl( pbd->length );
    /* The data comes from a C string, which contains a trailing \0. */
    /* This will be counted by sizeof(), but the embedded header */
    /* specifies the true size. Compensate by adding 1 to it. */
    if ( length + 1 != data_length )
        return NULL;

    width = ntohl( pbd->width );
    height = ntohl( pbd->height );
    rowstride = ntohl( pbd->rowstride );

    pixdata = ( void* )( pbd + 1 );

    return gdk_pixbuf_new_from_data( pixdata, GDK_COLORSPACE_RGB, TRUE, 8, width, height, rowstride, NULL, NULL );
}

static int x49gp_ui_button_pixmaps_init( x49gp_t* x49gp, x49gp_ui_button_t* button, x49gp_ui_color_t color )
{
    x49gp_ui_t* ui = x49gp->ui;
    GdkPixbuf *src, *dst;
    GtkStyle* style;

    style = gtk_style_new();
    x49gp_ui_style_init( style, button->button, &ui->colors[ button->key->color ], &ui->colors[ UI_COLOR_BLACK ] );

    for ( int i = 0; i < 5; i++ ) {
        style->bg_pixmap[ i ] = gdk_pixmap_new( ui->window->window, button->key->width, button->key->height, -1 );

        if ( i == GTK_STATE_ACTIVE ) {
            if ( color == UI_COLOR_SILVER ) {
                src = gdk_pixbuf_new_subpixbuf( ui->bg_pixbuf, ui->kb_x_offset + button->key->x, ui->kb_y_offset + button->key->y,
                                                button->key->width, button->key->height );
                dst = gdk_pixbuf_copy( src );
                g_object_unref( src );

                src = x49gp_pixbuf_new_from_inline( sizeof( button_round ), button_round );

                gdk_pixbuf_composite( src, dst, 0, 0, button->key->width, button->key->height, 0.0, 0.0, 1.0, 1.0, GDK_INTERP_HYPER, 0xff );

                g_object_unref( src );
                src = dst;
            } else
                src = gdk_pixbuf_new_subpixbuf( ui->bg_pixbuf, ui->kb_x_offset + button->key->x, ui->kb_y_offset + button->key->y + 1,
                                                button->key->width, button->key->height );

        } else
            src = gdk_pixbuf_new_subpixbuf( ui->bg_pixbuf, ui->kb_x_offset + button->key->x, ui->kb_y_offset + button->key->y,
                                            button->key->width, button->key->height );

        gdk_draw_pixbuf( style->bg_pixmap[ i ], ui->window->style->black_gc, src, 0, 0, 0, 0, button->key->width, button->key->height,
                         GDK_RGB_DITHER_NORMAL, 0, 0 );

        g_object_unref( src );
    }

    gtk_widget_set_style( button->button, style );

    return 0;
}

static void x49gp_ui_symbol_path( cairo_t* cr, double size, double xoffset, double yoffset, const x49gp_symbol_t* symbol )
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

static void x49gp_ui_draw_symbol( cairo_t* cr, GdkColor* color, double size, double line_width, gboolean fill, double xoffset,
                                  double yoffset, const x49gp_symbol_t* symbol )
{
    cairo_set_line_cap( cr, CAIRO_LINE_CAP_BUTT );
    cairo_set_line_join( cr, CAIRO_LINE_JOIN_MITER );
    cairo_set_line_width( cr, line_width );
    cairo_set_source_rgb( cr, ( ( double )color->red ) / 65535.0, ( ( double )color->green ) / 65535.0,
                          ( ( double )color->blue ) / 65535.0 );

    x49gp_ui_symbol_path( cr, size, xoffset, yoffset, symbol );

    if ( fill )
        cairo_fill( cr );
    else
        cairo_stroke( cr );
}

static int x49gp_ui_lookup_glyph( const char* name, int namelen, gunichar* glyph )
{
    for ( int i = 0; i < NR_GLYPHNAMES; i++ ) {
        if ( ( strlen( x49gp_glyphs[ i ].name ) == namelen ) && !strncmp( x49gp_glyphs[ i ].name, name, namelen ) ) {
            if ( glyph )
                *glyph = x49gp_glyphs[ i ].unichar;

            return 1;
        }
    }

    return 0;
}

static int x49gp_text_strlen( const char* text )
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

        if ( x49gp_ui_lookup_glyph( p, namelen, NULL ) ) {
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

static int x49gp_text_to_ucs4( const char* text, gunichar** ucs4p )
{
    const char *p, *q;
    gunichar glyph;
    gunichar* ucs4;
    char c;
    int namelen;
    int i = 0;
    int n = x49gp_text_strlen( text );

    if ( n <= 0 )
        return n;

    ucs4 = malloc( n * sizeof( gunichar ) );

    i = 0;

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

        if ( x49gp_ui_lookup_glyph( p, namelen, &glyph ) ) {
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

static void x49gp_ui_vtext_path( cairo_t* cr, const char* family, double size, double x, double y, int n, va_list ap )
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

    for ( int i = 0; i < n; i++ ) {
        slant = va_arg( ap, cairo_font_slant_t );
        weight = va_arg( ap, cairo_font_weight_t );
        text = va_arg( ap, const char* );

        cairo_select_font_face( cr, family, slant, weight );
        cairo_set_font_size( cr, size );

        ucs4 = NULL;
        len = x49gp_text_to_ucs4( text, &ucs4 );
        if ( len <= 0 )
            continue;

        for ( int j = 0; j < len; j++ ) {
            if ( g_unichar_type( ucs4[ j ] ) == G_UNICODE_PRIVATE_USE ) {
                /*
                 * Draw Symbol, Increment x...
                 */
                symbol = symbol_get_by_glyph( ucs4[ j ] );
                if ( NULL == symbol )
                    symbol = symbol_get_by_glyph( 0xe000 );

                size *= symbol->prescale;

                x49gp_ui_symbol_path( cr, size, x, y, symbol );

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
}

static void x49gp_ui_text_size( cairo_t* cr, const char* family, double size, double* x_bearing, double* y_bearing, double* width,
                                double* height, double* ascent, double* descent, int n, ... )
{
    va_list ap0, ap1;
    cairo_font_extents_t font_extents;
    cairo_font_weight_t weight;
    cairo_font_slant_t slant;
    double x1, y1, x2, y2, a, d;
    const char* text;

    if ( n < 1 )
        return;

    va_start( ap0, n );
    va_copy( ap1, ap0 );

    x49gp_ui_vtext_path( cr, family, size, 0.0, 0.0, n, ap0 );

    va_end( ap0 );

    cairo_fill_extents( cr, &x1, &y1, &x2, &y2 );

    if ( y2 < 0.0 )
        y2 = 0.0;

    a = 0.0;
    d = 0.0;

    for ( int i = 0; i < n; i++ ) {
        slant = va_arg( ap1, cairo_font_slant_t );
        weight = va_arg( ap1, cairo_font_weight_t );
        text = va_arg( ap1, const char* );
        ( void )text;

        cairo_select_font_face( cr, family, slant, weight );
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
    }

    *x_bearing = x1;
    *y_bearing = y2;
    *width = x2 - x1;
    *height = y2 - y1;
    *ascent = a;
    *descent = d;

    va_end( ap1 );
}

static void x49gp_ui_draw_text( cairo_t* cr, GdkColor* color, const char* family, double size, double line_width, int xoffset, int yoffset,
                                int n, ... )
{
    va_list ap;

    if ( n < 1 )
        return;

    va_start( ap, n );

    cairo_set_line_width( cr, line_width );
    cairo_set_source_rgb( cr, ( ( double )color->red ) / 65535.0, ( ( double )color->green ) / 65535.0,
                          ( ( double )color->blue ) / 65535.0 );

    x49gp_ui_vtext_path( cr, family, size, xoffset, yoffset, n, ap );

    if ( line_width == 0.0 )
        cairo_fill( cr );
    else
        cairo_stroke( cr );

    va_end( ap );
}

#if 0
static void
x49gp_ui_dump_path(cairo_t *cr, const char *family, int n, ...)
{
	va_list ap;
	const cairo_path_t *path;
	const cairo_path_data_t *data;

	if (n < 1)
		return;

	va_start(ap, n);

	x49gp_ui_vtext_path(cr, family, 1000.0, 0.0, 0.0, n, ap);

	path = cairo_copy_path(cr);
	if (NULL == path)
		return;

	cairo_new_path(cr);

	for (int i = 0; i < path->num_data; i += path->data[i].header.length) {
		data = &path->data[i];

		switch (data->header.type) {
		case CAIRO_PATH_MOVE_TO:
			printf("path: move to  %4.0f %4.0f\n",
				data[1].point.x, -data[1].point.y);
			break;
		case CAIRO_PATH_LINE_TO:
			printf("path: line to  %4.0f %4.0f\n",
				data[1].point.x, -data[1].point.y);
			break;
		case CAIRO_PATH_CURVE_TO:
			printf("path: curve to %4.0f %4.0f\n"
			       "               %4.0f %4.0f\n"
			       "               %4.0f %4.0f\n",
				data[1].point.x, -data[1].point.y,
				data[2].point.x, -data[2].point.y,
				data[3].point.x, -data[3].point.y);
			break;
		case CAIRO_PATH_CLOSE_PATH:
			printf("path: close path\n");
			break;
		}
	}

	va_end(ap);
}
#endif

static unsigned char bitmap_font_lookup_glyph( const bitmap_font_t* font, const char* name, int namelen )
{
    for ( int i = 0; font->glyphs[ i ].name; i++ )
        if ( ( strlen( font->glyphs[ i ].name ) == namelen ) && !strncmp( font->glyphs[ i ].name, name, namelen ) )
            return i;

    return 0;
}

static unsigned char bitmap_font_lookup_ascii( const bitmap_font_t* font, char c )
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

    return bitmap_font_lookup_glyph( font, name, namelen );
}

static int bitmap_font_strlen( const char* text )
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

static int bitmap_font_text_to_glyphs( const bitmap_font_t* font, const char* text, unsigned char** glyphp )
{
    unsigned char* glyphs;
    const char *p, *q;
    unsigned char c;
    int namelen;
    int i, n;

    n = bitmap_font_strlen( text );
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
            glyphs[ i++ ] = bitmap_font_lookup_ascii( font, c );
            continue;
        }

        q = p;
        while ( *q ) {
            if ( ( *q == '\\' ) || ( *q == ' ' ) )
                break;
            q++;
        }
        if ( q == p ) {
            glyphs[ i++ ] = bitmap_font_lookup_ascii( font, *p++ );
            continue;
        }
        namelen = q - p;
        if ( *q == ' ' )
            q++;

        glyphs[ i++ ] = bitmap_font_lookup_glyph( font, p, namelen );
        p = q;
    }

    *glyphp = glyphs;
    return n;
}

static void bitmap_font_text_size( const bitmap_font_t* font, const char* text, int* width, int* height, int* ascent, int* descent )
{
    const bitmap_glyph_t* glyph;
    unsigned char* glyphs;
    int n, w, a, d;

    w = 0;
    a = 0;
    d = 0;

    n = bitmap_font_text_to_glyphs( font, text, &glyphs );

    for ( int i = 0; i < n; i++ ) {
        glyph = &font->glyphs[ glyphs[ i ] ];

        w += glyph->width;

        if ( glyph->ascent > a )
            a = glyph->ascent;
        if ( glyph->descent < d )
            d = glyph->descent;
    }

    *width = w - 1;
    *height = font->ascent - font->descent;
    *ascent = a;
    *descent = d;

    if ( n > 0 )
        free( glyphs );
}

static void bitmap_font_draw_text( GdkDrawable* drawable, GdkColor* color, const bitmap_font_t* font, int x, int y, const char* text )
{
    const bitmap_glyph_t* glyph;
    unsigned char* glyphs;
    GdkBitmap* bitmap;
    GdkGC* gc;
    int n, w, h;

    gc = gdk_gc_new( drawable );
    gdk_gc_set_rgb_fg_color( gc, color );

    n = bitmap_font_text_to_glyphs( font, text, &glyphs );

    for ( int i = 0; i < n; i++ ) {
        glyph = &font->glyphs[ glyphs[ i ] ];

        w = glyph->width - glyph->kern;
        h = glyph->ascent - glyph->descent;

        if ( w <= 0 || h <= 0 ) {
            x += glyph->width;
            continue;
        }

        bitmap = gdk_bitmap_create_from_data( NULL, ( char* )glyph->bits, w, h );

        gdk_gc_set_ts_origin( gc, x + glyph->kern, y + font->ascent - glyph->ascent );
        gdk_gc_set_stipple( gc, bitmap );
        gdk_gc_set_fill( gc, GDK_STIPPLED );

        gdk_draw_rectangle( drawable, gc, TRUE, x + glyph->kern, y + font->ascent - glyph->ascent, w, h );

        g_object_unref( bitmap );

        x += glyph->width;
    }

    g_object_unref( gc );

    if ( n > 0 )
        free( glyphs );
}

void x49gp_ui_show_error( x49gp_t* x49gp, const char* text )
{
    GtkWidget* dialog;
    x49gp_ui_t* ui = x49gp->ui;

    dialog =
        gtk_message_dialog_new( GTK_WINDOW( ui->window ), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "%s", text );

    gtk_dialog_run( GTK_DIALOG( dialog ) );
    gtk_widget_destroy( dialog );
}

static void x49gp_ui_choose_file( x49gp_t* x49gp, const char* prompt, GtkFileChooserAction action, char** filename )
{
    GtkWidget* dialog;
    x49gp_ui_t* ui = x49gp->ui;

    dialog = gtk_file_chooser_dialog_new( prompt, GTK_WINDOW( ui->window ), action, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN,
                                          GTK_RESPONSE_ACCEPT, NULL );

    gtk_file_chooser_set_local_only( GTK_FILE_CHOOSER( dialog ), TRUE );
    gtk_file_chooser_set_select_multiple( GTK_FILE_CHOOSER( dialog ), FALSE );

    if ( gtk_dialog_run( GTK_DIALOG( dialog ) ) == GTK_RESPONSE_ACCEPT )
        *filename = gtk_file_chooser_get_filename( GTK_FILE_CHOOSER( dialog ) );
    else
        *filename = NULL;

    gtk_widget_destroy( dialog );
}

void x49gp_ui_open_firmware( x49gp_t* x49gp, char** filename )
{
    x49gp_ui_choose_file( x49gp, "Choose firmware ...", GTK_FILE_CHOOSER_ACTION_OPEN, filename );
}

static gboolean x49gp_ui_button_press( GtkWidget* widget, GdkEventButton* event, gpointer user_data )
{
    x49gp_ui_button_t* button = user_data;
    const x49gp_ui_key_t* key = button->key;
    x49gp_t* x49gp = button->x49gp;
    x49gp_ui_t* ui = x49gp->ui;

#ifdef DEBUG_X49GP_UI
    fprintf( stderr, "%s:%u: type %u, button %u\n", __FUNCTION__, __LINE__, event->type, event->button );
#endif

    if ( event->type != GDK_BUTTON_PRESS )
        return FALSE;

    switch ( event->button ) {
        case 1:
            ui->buttons_down++;
            if ( button->down )
                return FALSE;
            button->down = TRUE;
            break;
        case 3:
            button->hold = TRUE;
            if ( button->down )
                return FALSE;
            gtk_button_pressed( GTK_BUTTON( button->button ) );
            button->down = TRUE;
            break;
        default:
            return TRUE;
    }

#ifdef DEBUG_X49GP_UI
    printf( "%s: button %u: col %u, row %u, eint %u\n", __FUNCTION__, event->button, button->key->column, button->key->row,
            button->key->eint );
#endif

    if ( key->rowbit )
        s3c2410_io_port_g_update( x49gp, key->column, key->row, key->columnbit, key->rowbit, 1 );
    else
        s3c2410_io_port_f_set_bit( x49gp, key->eint, 1 );

    return FALSE;
}

static void x49gp_release_single_button( x49gp_ui_button_t* button, x49gp_ui_button_t* cause )
{
    x49gp_t* x49gp = button->x49gp;
    const x49gp_ui_key_t* key;
    GtkButton* gtkbutton;

#ifdef DEBUG_X49GP_UI
    printf( "%s: button %u: col %u, row %u, eint %u\n", __FUNCTION__, event->button, button->key->column, button->key->row,
            button->key->eint );
#endif

    button->down = FALSE;
    button->hold = FALSE;

    gtkbutton = GTK_BUTTON( button->button );

    if ( button != cause )
        gtkbutton->in_button = FALSE;
    gtk_button_released( gtkbutton );

    key = button->key;

    if ( key->rowbit )
        s3c2410_io_port_g_update( x49gp, key->column, key->row, key->columnbit, key->rowbit, 0 );
    else
        s3c2410_io_port_f_set_bit( x49gp, key->eint, 0 );
}

static void x49gp_release_all_buttons( x49gp_t* x49gp, x49gp_ui_button_t* cause )
{
    x49gp_ui_button_t* button;
    x49gp_ui_t* ui = x49gp->ui;

    for ( int i = 0; i < ui->nr_buttons; i++ ) {
        button = &ui->buttons[ i ];

        if ( !button->down )
            continue;

        x49gp_release_single_button( button, cause );
    }
}

static gboolean x49gp_ui_button_release( GtkWidget* widget, GdkEventButton* event, gpointer user_data )
{
    x49gp_ui_button_t* button = user_data;
    x49gp_t* x49gp = button->x49gp;
    x49gp_ui_t* ui = x49gp->ui;

    if ( event->type != GDK_BUTTON_RELEASE )
        return FALSE;

    switch ( event->button ) {
        case 1:
            break;
        default:
            return TRUE;
    }

    if ( ui->buttons_down > 0 )
        ui->buttons_down--;

    if ( ui->buttons_down == 0 )
        x49gp_release_all_buttons( x49gp, button );
    else
        x49gp_release_single_button( button, button );

    return FALSE;
}

static gboolean x49gp_ui_show_menu( GtkWidget* widget, GdkEventButton* event, gpointer user_data )
{
    x49gp_t* x49gp = user_data;
    x49gp_ui_t* ui = x49gp->ui;

    gtk_widget_set_sensitive( ui->menu_unmount, s3c2410_sdi_is_mounted( x49gp ) );
    if ( ui->menu_debug )
        gtk_widget_set_sensitive( ui->menu_debug, !gdbserver_isactive() );

    if ( event->type == GDK_BUTTON_PRESS && event->button == 3 ) {
        gtk_menu_popup( GTK_MENU( ui->menu ), NULL, NULL, NULL, NULL, event->button, event->time );
        return TRUE;
    }

    return FALSE;
}

static gboolean x49gp_ui_button_leave( GtkWidget* widget, GdkEventCrossing* event, gpointer user_data )
{
    x49gp_ui_button_t* button = user_data;

    if ( event->type != GDK_LEAVE_NOTIFY )
        return FALSE;

    if ( !button->hold )
        return FALSE;

    return TRUE;
}

static gboolean x49gp_ui_focus_lost( GtkWidget* widget, GdkEventFocus* event, gpointer user_data )
{
    x49gp_t* x49gp = user_data;
    x49gp_ui_t* ui = x49gp->ui;

    if ( event->type != GDK_FOCUS_CHANGE )
        return FALSE;

    ui->buttons_down = 0;
    x49gp_release_all_buttons( x49gp, NULL );

    return FALSE;
}

static void x49gp_ui_popup_at_widget( GtkMenu* menu, gint* x, gint* y, gboolean* push_in, gpointer user_data )
{
    GtkWidget* widget = GTK_WIDGET( user_data );

    gdk_window_get_origin( widget->window, x, y );
    *x += widget->allocation.x;
    *y += widget->allocation.y;
}

static void x49gp_ui_mount_sd_folder( GtkMenuItem* menuitem, gpointer user_data )
{
    x49gp_t* x49gp = user_data;
    char* filename;

    x49gp_ui_choose_file( x49gp, "Choose SD folder ...", GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER, &filename );
    if ( filename != NULL )
        s3c2410_sdi_mount( x49gp, filename );
}

static void x49gp_ui_mount_sd_image( GtkMenuItem* menuitem, gpointer user_data )
{
    x49gp_t* x49gp = user_data;
    char* filename;

    x49gp_ui_choose_file( x49gp, "Choose SD image ...", GTK_FILE_CHOOSER_ACTION_OPEN, &filename );
    if ( filename != NULL )
        s3c2410_sdi_mount( x49gp, filename );
}

static void x49gp_ui_debug( GtkMenuItem* menuitem, gpointer user_data )
{
    x49gp_t* x49gp = user_data;

    if ( x49gp->debug_port != 0 && !gdbserver_isactive() ) {
        gdbserver_start( x49gp->debug_port );
        gdb_handlesig( x49gp->env, 0 );
    }
}

static void x49gp_ui_calculator_reset( GtkMenuItem* menuitem, gpointer user_data )
{
    x49gp_t* x49gp = user_data;

    x49gp_modules_reset( x49gp, X49GP_RESET_POWER_ON );
    cpu_reset( x49gp->env );
    x49gp_set_idle( x49gp, 0 );
}

static gboolean x49gp_ui_key_event( GtkWidget* widget, GdkEventKey* event, gpointer user_data )
{
    x49gp_t* x49gp = user_data;
    x49gp_ui_t* ui = x49gp->ui;
    x49gp_ui_button_t* button;
    GdkEventButton bev;
    gboolean save_in;
    int index;
    guint keyval;

#ifdef DEBUG_X49GP_UI
    fprintf( stderr, "%s:%u: type %u, keyval %04x\n", __FUNCTION__, __LINE__, event->type, event->keyval );
#endif
    /* We want to know the keyval as interpreted without modifiers. */
    /* However, there is one modifier we do care about: NumLock, */
    /* which normally is represented by MOD2. */
    if ( !gdk_keymap_translate_keyboard_state( gdk_keymap_get_default(), event->hardware_keycode, event->state & GDK_MOD2_MASK,
                                               event->group, &keyval, NULL, NULL, NULL ) )
        return FALSE;
#ifdef DEBUG_X49GP_UI
    fprintf( stderr, "%s:%u: state %u, base keyval %04x\n", __FUNCTION__, __LINE__, event->state, keyval );
#endif

    switch ( keyval ) {
        case GDK_KEY_a:
        case GDK_KEY_F1:
            index = 0;
            break;
        case GDK_KEY_b:
        case GDK_KEY_F2:
            index = 1;
            break;
        case GDK_KEY_c:
        case GDK_KEY_F3:
            index = 2;
            break;
        case GDK_KEY_d:
        case GDK_KEY_F4:
            index = 3;
            break;
        case GDK_KEY_e:
        case GDK_KEY_F5:
            index = 4;
            break;
        case GDK_KEY_f:
        case GDK_KEY_F6:
            index = 5;
            break;
        case GDK_KEY_g:
            index = 6;
            break;
        case GDK_KEY_h:
            index = 7;
            break;
        case GDK_KEY_i:
            index = 8;
            break;
        case GDK_KEY_j:
            index = 9;
            break;
        case GDK_KEY_k:
            index = 10;
            break;
        case GDK_KEY_l:
            index = 11;
            break;
        case GDK_KEY_Up:
        case GDK_KEY_KP_Up:
            index = 12;
            break;
        case GDK_KEY_Left:
        case GDK_KEY_KP_Left:
            index = 13;
            break;
        case GDK_KEY_Down:
        case GDK_KEY_KP_Down:
            index = 14;
            break;
        case GDK_KEY_Right:
        case GDK_KEY_KP_Right:
            index = 15;
            break;
        case GDK_KEY_m:
            index = 16;
            break;
        case GDK_KEY_n:
            index = 17;
            break;
        case GDK_KEY_o:
        case GDK_KEY_apostrophe:
            index = 18;
            break;
        case GDK_KEY_p:
            index = 19;
            break;
        case GDK_KEY_BackSpace:
        case GDK_KEY_Delete:
        case GDK_KEY_KP_Delete:
            index = 20;
            break;
        case GDK_KEY_dead_circumflex:
        case GDK_KEY_asciicircum:
        case GDK_KEY_q:
        case GDK_KEY_caret:
            index = 21;
            break;
        case GDK_KEY_r:
            index = 22;
            break;
        case GDK_KEY_s:
            index = 23;
            break;
        case GDK_KEY_t:
            index = 24;
            break;
        case GDK_KEY_u:
            index = 25;
            break;
        case GDK_KEY_v:
            index = 26;
            break;
        case GDK_KEY_w:
            index = 27;
            break;
        case GDK_KEY_x:
            index = 28;
            break;
        case GDK_KEY_y:
            index = 29;
            break;
        case GDK_KEY_z:
        case GDK_KEY_slash:
        case GDK_KEY_KP_Divide:
            index = 30;
            break;
        case GDK_KEY_Tab:
            index = 31;
            break;
#ifndef __APPLE__
        // case GDK_KEY_Alt_L: case GDK_KEY_Alt_R:
        // case GDK_KEY_Meta_L: case GDK_KEY_Meta_R:
        case GDK_KEY_Mode_switch:
            index = 31;
            break;
#endif
        case GDK_KEY_7:
        case GDK_KEY_KP_7:
            index = 32;
            break;
        case GDK_KEY_8:
        case GDK_KEY_KP_8:
            index = 33;
            break;
        case GDK_KEY_9:
        case GDK_KEY_KP_9:
            index = 34;
            break;
        case GDK_KEY_multiply:
        case GDK_KEY_KP_Multiply:
            index = 35;
            break;
        case GDK_KEY_Shift_L:
        case GDK_KEY_Shift_R:
            index = 36;
            break;
        case GDK_KEY_4:
        case GDK_KEY_KP_4:
            index = 37;
            break;
        case GDK_KEY_5:
        case GDK_KEY_KP_5:
            index = 38;
            break;
        case GDK_KEY_6:
        case GDK_KEY_KP_6:
            index = 39;
            break;
        case GDK_KEY_minus:
        case GDK_KEY_KP_Subtract:
            index = 40;
            break;
        case GDK_KEY_Control_L:
        case GDK_KEY_Control_R:
            index = 41;
            break;
        case GDK_KEY_1:
        case GDK_KEY_KP_1:
            index = 42;
            break;
        case GDK_KEY_2:
        case GDK_KEY_KP_2:
            index = 43;
            break;
        case GDK_KEY_3:
        case GDK_KEY_KP_3:
            index = 44;
            break;
        case GDK_KEY_plus:
        case GDK_KEY_KP_Add:
            index = 45;
            break;
        case GDK_KEY_Escape:
            index = 46;
            break;
        case GDK_KEY_0:
        case GDK_KEY_KP_0:
            index = 47;
            break;
        case GDK_KEY_period:
        case GDK_KEY_comma:
        case GDK_KEY_KP_Decimal:
        case GDK_KP_Separator:
            index = 48;
            break;
        case GDK_KEY_space:
        case GDK_KEY_KP_Space:
            index = 49;
            break;
        case GDK_KEY_Return:
        case GDK_KEY_KP_Enter:
            index = 50;
            break;

        /* QWERTY compat: US English, UK English, International English */
        case GDK_KEY_backslash:
            index = 35;
            break;
        case GDK_KEY_equal:
            index = 45;
            break;

        /* QWERTZ compat: German */
        case GDK_KEY_ssharp:
            index = 30;
            break;
        case GDK_KEY_numbersign:
            index = 35;
            break;

        case GDK_KEY_F7:
        case GDK_KEY_F10:
            x49gp->arm_exit = 1;
            cpu_exit( x49gp->env );
            return FALSE;

        case GDK_KEY_F12:
            switch ( event->type ) {
                case GDK_KEY_PRESS:
                    x49gp_modules_reset( x49gp, X49GP_RESET_POWER_ON );
                    cpu_reset( x49gp->env );
                    x49gp_set_idle( x49gp, 1 );
                    break;
                case GDK_KEY_RELEASE:
                    x49gp_set_idle( x49gp, 0 );
                    break;
                default:
                    break;
            }
            return FALSE;

        case GDK_KEY_Menu:
            gtk_widget_set_sensitive( ui->menu_unmount, s3c2410_sdi_is_mounted( x49gp ) );
            if ( ui->menu_debug )
                gtk_widget_set_sensitive( ui->menu_debug, !gdbserver_isactive() );

            gtk_menu_popup( GTK_MENU( ui->menu ), NULL, NULL, x49gp_ui_popup_at_widget, ui->lcd_canvas, 0, event->time );
            return FALSE;

        default:
            return FALSE;
    }

    button = &ui->buttons[ index ];

    memset( &bev, 0, sizeof( GdkEventButton ) );

    bev.time = event->time;
    bev.button = 1;
    bev.state = event->state;

    save_in = GTK_BUTTON( button->button )->in_button;

    switch ( event->type ) {
        case GDK_KEY_PRESS:
            bev.type = GDK_BUTTON_PRESS;
            x49gp_ui_button_press( button->button, &bev, button );
            GTK_BUTTON( button->button )->in_button = TRUE;
            gtk_button_pressed( GTK_BUTTON( button->button ) );
            GTK_BUTTON( button->button )->in_button = save_in;
            break;
        case GDK_KEY_RELEASE:
            bev.type = GDK_BUTTON_RELEASE;
            GTK_BUTTON( button->button )->in_button = TRUE;
            gtk_button_released( GTK_BUTTON( button->button ) );
            GTK_BUTTON( button->button )->in_button = save_in;
            x49gp_ui_button_release( button->button, &bev, button );
            break;
        default:
            return FALSE;
    }

    return TRUE;
}

static int x49gp_button_expose_event( GtkWidget* widget, GdkEventExpose* event, gpointer user_data )
{
    x49gp_ui_button_t* button = user_data;
    int x, y;

    x = widget->allocation.x;
    y = widget->allocation.y;

    if ( GTK_WIDGET_STATE( widget ) == GTK_STATE_ACTIVE )
        y -= 1;

    gdk_draw_drawable( widget->window, widget->style->black_gc, button->pixmap, 0, 0, x, y, widget->allocation.width,
                       widget->allocation.height );

    return FALSE;
}

static void x49gp_button_realize( GtkWidget* widget, gpointer user_data )
{
    x49gp_ui_button_t* button = user_data;
    x49gp_ui_t* ui = button->x49gp->ui;
    const x49gp_ui_key_t* key = button->key;
    cairo_t* cr;
    double xoff, yoff, width, height, ascent, descent;
    unsigned int w, h;
    int xoffset, yoffset, x, y;

    xoffset = widget->allocation.x;
    yoffset = widget->allocation.y;
    w = widget->allocation.width;
    h = widget->allocation.height;

    button->pixmap = gdk_pixmap_new( widget->style->bg_pixmap[ 0 ], w, h, -1 );

    gdk_draw_drawable( button->pixmap, widget->style->black_gc, widget->style->bg_pixmap[ 0 ], xoffset, yoffset, 0, 0, button->key->width,
                       button->key->height );

    xoffset += 2;
    yoffset += 2;
    w -= 4;
    h -= 4;

    cr = gdk_cairo_create( button->pixmap );
    cairo_set_line_cap( cr, CAIRO_LINE_CAP_BUTT );
    cairo_set_line_join( cr, CAIRO_LINE_JOIN_MITER );

#if 0 /* Layout Debug */
	cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
	cairo_set_line_width(cr, 1.0);
	cairo_move_to(cr, xoffset, yoffset);
	cairo_line_to(cr, xoffset + w - 1, yoffset);
	cairo_line_to(cr, xoffset + w - 1, yoffset + h - 1);
	cairo_line_to(cr, xoffset, yoffset + h - 1);
	cairo_close_path(cr);
	cairo_stroke(cr);
#endif

    if ( key->letter ) {
        x49gp_ui_text_size( cr, X49GP_UI_NORMAL_FONT, key->letter_size, &xoff, &yoff, &width, &height, &ascent, &descent, 1,
                            CAIRO_FONT_SLANT_NORMAL, key->font_weight, key->letter );

        switch ( key->layout ) {
            case UI_LAYOUT_LEFT:
            default:
                x = ( int )floor( w - 1.0 - width - xoff + 0.5 );
                y = ( int )floor( ( h - 1.0 + ascent ) / 2.0 + 0.5 );
                w -= width;
                break;
            case UI_LAYOUT_LEFT_NO_SPACE:
                x = ( int )floor( w - 1.0 - width - xoff + 0.5 );
                y = ( int )floor( ( h - 1.0 + ascent ) / 2.0 + 0.5 );
                break;
            case UI_LAYOUT_BELOW:
                x = ( int )floor( ( w - 1.0 - width ) / 2.0 - xoff + 0.5 );
                y = ( int )h - 1.0;
                h -= ascent;
                break;
        }

        x49gp_ui_draw_text( cr, &ui->colors[ UI_COLOR_YELLOW ], X49GP_UI_NORMAL_FONT, key->letter_size, 0.0, x + xoffset, y + yoffset, 1,
                            CAIRO_FONT_SLANT_NORMAL, key->font_weight, key->letter );
    }

    x49gp_ui_text_size( cr, X49GP_UI_NORMAL_FONT, key->font_size, &xoff, &yoff, &width, &height, &ascent, &descent, 1,
                        CAIRO_FONT_SLANT_NORMAL, key->font_weight, key->label );

    x = ( int )floor( ( w - 1.0 - width ) / 2.0 - xoff + 0.5 );
    y = ( int )floor( ( h - 1.0 + ascent ) / 2.0 + 0.5 );

    x49gp_ui_draw_text( cr, &widget->style->text[ 0 ], X49GP_UI_NORMAL_FONT, key->font_size, 0.0, x + xoffset, y + yoffset, 1,
                        CAIRO_FONT_SLANT_NORMAL, key->font_weight, key->label );

    cairo_destroy( cr );
}

static int x49gp_lcd_expose_event( GtkWidget* widget, GdkEventExpose* event, gpointer user_data )
{
    x49gp_t* x49gp = user_data;
    x49gp_ui_t* ui = x49gp->ui;
    GdkRectangle* rects;
    int n;

    gdk_region_get_rectangles( event->region, &rects, &n );
    for ( int i = 0; i < n; i++ ) {
        gdk_draw_drawable( widget->window, widget->style->black_gc, ui->lcd_pixmap, rects[ i ].x, rects[ i ].y, rects[ i ].x, rects[ i ].y,
                           rects[ i ].width, rects[ i ].height );
    }

    g_free( rects );

    return FALSE;
}

static int x49gp_lcd_configure_event( GtkWidget* widget, GdkEventConfigure* event, gpointer user_data )
{
    x49gp_t* x49gp = user_data;
    x49gp_ui_t* ui = x49gp->ui;

    if ( NULL != ui->lcd_pixmap )
        return FALSE;

    ui->ann_left = gdk_bitmap_create_from_data( ui->lcd_canvas->window, ( char* )ann_left_bits, ann_left_width, ann_left_height );
    ui->ann_right = gdk_bitmap_create_from_data( ui->lcd_canvas->window, ( char* )ann_right_bits, ann_right_width, ann_right_height );
    ui->ann_alpha = gdk_bitmap_create_from_data( ui->lcd_canvas->window, ( char* )ann_alpha_bits, ann_alpha_width, ann_alpha_height );
    ui->ann_battery =
        gdk_bitmap_create_from_data( ui->lcd_canvas->window, ( char* )ann_battery_bits, ann_battery_width, ann_battery_height );
    ui->ann_busy = gdk_bitmap_create_from_data( ui->lcd_canvas->window, ( char* )ann_busy_bits, ann_busy_width, ann_busy_height );
    ui->ann_io = gdk_bitmap_create_from_data( ui->lcd_canvas->window, ( char* )ann_io_bits, ann_io_width, ann_io_height );

    ui->ann_left_gc = gdk_gc_new( ui->lcd_canvas->window );
    gdk_gc_copy( ui->ann_left_gc, widget->style->black_gc );
    gdk_gc_set_ts_origin( ui->ann_left_gc, 11, 0 );
    gdk_gc_set_stipple( ui->ann_left_gc, ui->ann_left );
    gdk_gc_set_fill( ui->ann_left_gc, GDK_STIPPLED );

    ui->ann_right_gc = gdk_gc_new( ui->lcd_canvas->window );
    gdk_gc_copy( ui->ann_right_gc, widget->style->black_gc );
    gdk_gc_set_ts_origin( ui->ann_right_gc, 56, 0 );
    gdk_gc_set_stipple( ui->ann_right_gc, ui->ann_right );
    gdk_gc_set_fill( ui->ann_right_gc, GDK_STIPPLED );

    ui->ann_alpha_gc = gdk_gc_new( ui->lcd_canvas->window );
    gdk_gc_copy( ui->ann_alpha_gc, widget->style->black_gc );
    gdk_gc_set_ts_origin( ui->ann_alpha_gc, 101, 0 );
    gdk_gc_set_stipple( ui->ann_alpha_gc, ui->ann_alpha );
    gdk_gc_set_fill( ui->ann_alpha_gc, GDK_STIPPLED );

    ui->ann_battery_gc = gdk_gc_new( ui->lcd_canvas->window );
    gdk_gc_copy( ui->ann_battery_gc, widget->style->black_gc );
    gdk_gc_set_ts_origin( ui->ann_battery_gc, 146, 0 );
    gdk_gc_set_stipple( ui->ann_battery_gc, ui->ann_battery );
    gdk_gc_set_fill( ui->ann_battery_gc, GDK_STIPPLED );

    ui->ann_busy_gc = gdk_gc_new( ui->lcd_canvas->window );
    gdk_gc_copy( ui->ann_busy_gc, widget->style->black_gc );
    gdk_gc_set_ts_origin( ui->ann_busy_gc, 191, 0 );
    gdk_gc_set_stipple( ui->ann_busy_gc, ui->ann_busy );
    gdk_gc_set_fill( ui->ann_busy_gc, GDK_STIPPLED );

    ui->ann_io_gc = gdk_gc_new( ui->lcd_canvas->window );
    gdk_gc_copy( ui->ann_io_gc, widget->style->black_gc );
    gdk_gc_set_ts_origin( ui->ann_io_gc, 236, 0 );
    gdk_gc_set_stipple( ui->ann_io_gc, ui->ann_io );
    gdk_gc_set_fill( ui->ann_io_gc, GDK_STIPPLED );

    ui->lcd_pixmap = gdk_pixmap_new( ui->lcd_canvas->window, ui->lcd_width, ui->lcd_height, -1 );

#if 0 /* Debug Symbols on LCD screen ;) */
{
	cairo_t *cr;

	cr = gdk_cairo_create(ui->bg_pixmap);
	cairo_set_line_cap(cr, CAIRO_LINE_CAP_BUTT);
	cairo_set_line_join(cr, CAIRO_LINE_JOIN_MITER);

#  if 1
	x49gp_ui_draw_text(cr, &widget->style->black,
			   X49GP_UI_NORMAL_FONT, 100.0, 1.0,
			   ui->lcd_x_offset + 10, ui->lcd_y_offset + 160,
			   1, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD,
			   "\\arrowleftdblfull");
#  else
	x49gp_ui_dump_path(cr, X49GP_UI_NORMAL_FONT,
			   1, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD,
			   "\\arrowleftdblfull");
#  endif

	cairo_destroy(cr);
}
#endif

    gdk_draw_drawable( ui->lcd_pixmap, widget->style->black_gc, ui->bg_pixmap, ui->lcd_x_offset, ui->lcd_y_offset, 0, 0, ui->lcd_width,
                       ui->lcd_height );

    return FALSE;
}

static int x49gp_window_configure_event( GtkWidget* widget, GdkEventConfigure* event, gpointer user_data )
{
    x49gp_t* x49gp = user_data;
    x49gp_ui_t* ui = x49gp->ui;
    const x49gp_ui_key_t* key;
    cairo_t* cr;
    int left_color;
    int right_color;
    int below_color;
    int xl, xr, a;
    int wl = 0, wr = 0;
    int hl = 0, hr = 0;
    int dl = 0, dr = 0;

    if ( NULL != ui->bg_pixmap )
        return FALSE;

    ui->bg_pixmap = gdk_pixmap_new( widget->window, ui->width, ui->height, -1 );

    gdk_draw_pixbuf( ui->bg_pixmap, widget->style->black_gc, ui->bg_pixbuf, 0, 0, 0, 0, ui->width, ui->height, GDK_RGB_DITHER_NORMAL, 0,
                     0 );

    cr = gdk_cairo_create( ui->bg_pixmap );
    cairo_set_line_cap( cr, CAIRO_LINE_CAP_BUTT );
    cairo_set_line_join( cr, CAIRO_LINE_JOIN_MITER );

    switch ( ui->calculator ) {
        case UI_CALCULATOR_HP49GP:
        case UI_CALCULATOR_HP49GP_NEWRPL:
            x49gp_ui_draw_text( cr, &widget->style->black, X49GP_UI_NORMAL_FONT, 15.0, 0.0, 14 /* 38 */, 20 /* 42 */, 2,
                                CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD, "hp", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL,
                                " 49g+" );

            x49gp_ui_draw_text( cr, &widget->style->black, X49GP_UI_NORMAL_FONT, 13.0, 0.0, 14 /* 38 */, 34 /* 56 */, 1,
                                CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL, "graphing calculator" );

            x49gp_ui_draw_symbol( cr, &widget->style->black, 10.0, 0.0, TRUE, 114 /* 138 */, 8 /* 25 */,
                                  symbol_get_by_name( "triangleup" ) );

            left_color = UI_COLOR_GREEN;
            right_color = UI_COLOR_RED;
            below_color = UI_COLOR_BLACK;
            break;

        default:
            ui->calculator = UI_CALCULATOR_HP50G;
            /* fall through */

        case UI_CALCULATOR_HP50G:
        case UI_CALCULATOR_HP50G_NEWRPL:
            x49gp_ui_draw_text( cr, &widget->style->white, X49GP_UI_NORMAL_FONT, 15.0, 0.0, 14 /* 38 */, 20 /* 42 */, 2,
                                CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL, "HP", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL,
                                " 50g" );

            x49gp_ui_draw_text( cr, &widget->style->white, X49GP_UI_NORMAL_FONT, 13.0, 0.0, 14 /* 38 */, 34 /* 56 */, 1,
                                CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL, "Graphing Calculator" );

            x49gp_ui_draw_symbol( cr, &widget->style->white, 10.0, 0.0, TRUE, 134 /* 168 */, 8 /* 25 */,
                                  symbol_get_by_name( "triangleup" ) );

            left_color = UI_COLOR_WHITE;
            right_color = UI_COLOR_ORANGE;
            below_color = UI_COLOR_BLUE;
            break;
    }

    cairo_destroy( cr );

    for ( int i = 0; i < ui->nr_buttons; i++ ) {
        key = ui->buttons[ i ].key;

        if ( key->left ) {
            bitmap_font_text_size( &tiny_font, key->left, &wl, &hl, &a, &dl );
            if ( !key->right ) {
                xl = key->x + ( key->width - wl ) / 2;
                bitmap_font_draw_text( ui->bg_pixmap, &ui->colors[ left_color ], &tiny_font, ui->kb_x_offset + xl,
                                       ui->kb_y_offset + key->y - hl + dl + 1, key->left );
            }
        }

        if ( key->right ) {
            bitmap_font_text_size( &tiny_font, key->right, &wr, &hr, &a, &dr );
            if ( !key->left ) {
                xr = key->x + ( key->width - wr ) / 2;
                bitmap_font_draw_text( ui->bg_pixmap, &ui->colors[ right_color ], &tiny_font, ui->kb_x_offset + xr,
                                       ui->kb_y_offset + key->y - hr + dr + 1, key->right );
            }
        }

        if ( key->left && key->right ) {
            xl = key->x;
            xr = key->x + key->width - 1 - wr;

            if ( wl + wr > key->width - 4 ) {
                xl += ( key->width - 4 - ( wl + wr ) ) / 2;
                xr -= ( key->width - 4 - ( wl + wr ) ) / 2;
            }

            bitmap_font_draw_text( ui->bg_pixmap, &ui->colors[ left_color ], &tiny_font, ui->kb_x_offset + xl,
                                   ui->kb_y_offset + key->y - hl + dl + 1, key->left );

            bitmap_font_draw_text( ui->bg_pixmap, &ui->colors[ right_color ], &tiny_font, ui->kb_x_offset + xr,
                                   ui->kb_y_offset + key->y - hr + dr + 1, key->right );
        }

        if ( key->below ) {
            bitmap_font_text_size( &tiny_font, key->below, &wl, &hl, &a, &dl );
            xl = key->x + ( key->width - wl ) / 2;

            bitmap_font_draw_text( ui->bg_pixmap, &ui->colors[ below_color ], &tiny_font, ui->kb_x_offset + xl,
                                   ui->kb_y_offset + key->y + key->height + 2, key->below );
        }

#if 0 /* Debug Button Layout */
		gdk_draw_rectangle(ui->bg_pixmap, ui->window->style->white_gc,
				   FALSE,
				   ui->kb_x_offset + key->x,
				   ui->kb_y_offset + key->y,
				   key->width, key->height);
#endif
    }

    gdk_window_set_back_pixmap( widget->window, ui->bg_pixmap, FALSE );

    return FALSE;
}

static gboolean x49gp_window_button_press( GtkWidget* widget, GdkEventButton* event, gpointer user_data )
{
#ifdef DEBUG_X49GP_UI
    fprintf( stderr, "%s:%u: type %u, button %u\n", __FUNCTION__, __LINE__, event->type, event->button );
#endif

    gdk_window_focus( widget->window, event->time );
    gdk_window_raise( widget->window );

    if ( event->type != GDK_BUTTON_PRESS )
        return FALSE;

    if ( event->button != 1 )
        return FALSE;

    gdk_window_begin_move_drag( widget->window, event->button, event->x_root, event->y_root, event->time );

    return FALSE;
}

static void x49gp_ui_quit( gpointer user_data, GtkWidget* widget, GdkEvent* event )
{
    x49gp_t* x49gp = user_data;

    x49gp->arm_exit++;
}

static void x49gp_ui_place_at( x49gp_t* x49gp, GtkFixed* fixed, GtkWidget* widget, gint x, gint y, gint width, gint height )
{
    gtk_widget_set_size_request( widget, width, height );
    gtk_fixed_put( fixed, widget, x, y );
}

static int gui_init( x49gp_module_t* module )
{
    x49gp_t* x49gp = module->x49gp;
    x49gp_ui_t* ui;

    ui = malloc( sizeof( x49gp_ui_t ) );
    if ( NULL == ui ) {
        fprintf( stderr, "%s: %s:%u: Out of memory\n", x49gp->progname, __FUNCTION__, __LINE__ );
        return -ENOMEM;
    }
    memset( ui, 0, sizeof( x49gp_ui_t ) );

    ui->nr_buttons = X49GP_UI_NR_KEYS;
    ui->buttons = malloc( ui->nr_buttons * sizeof( x49gp_ui_button_t ) );
    if ( NULL == ui->buttons ) {
        fprintf( stderr, "%s: %s:%u: Out of memory\n", x49gp->progname, __FUNCTION__, __LINE__ );
        free( ui );
        return -ENOMEM;
    }
    memset( ui->buttons, 0, ui->nr_buttons * sizeof( x49gp_ui_button_t ) );

    module->user_data = ui;
    x49gp->ui = ui;

    return 0;
}

static int gui_exit( x49gp_module_t* module ) { return 0; }

static int gui_reset( x49gp_module_t* module, x49gp_reset_t reset ) { return 0; }

static int gui_load( x49gp_module_t* module, GKeyFile* keyfile )
{
    x49gp_t* x49gp = module->x49gp;
    x49gp_ui_t* ui = module->user_data;
    x49gp_ui_button_t* button;
    const x49gp_ui_key_t* key;
    GtkWidget* screen_box;
    GtkWidget *menu_mount_folder, *menu_mount_image, *menu_unmount;
    GtkWidget *menu_debug, *menu_reset, *menu_quit;
    GError* gerror = NULL;
    GdkBitmap* shape;
    char* typestr;
    char* imagefile;
    int fd;

    x49gp_module_get_string( module, keyfile, "type", "hp50g", &typestr );
    if ( !strcmp( typestr, "hp49g+" ) )
        ui->calculator = UI_CALCULATOR_HP49GP;
    else if ( !strcmp( typestr, "hp49g+/newrpl" ) )
        ui->calculator = UI_CALCULATOR_HP49GP_NEWRPL;
    else if ( !strcmp( typestr, "hp50g" ) )
        ui->calculator = UI_CALCULATOR_HP50G;
    else if ( !strcmp( typestr, "hp50g/newrpl" ) )
        ui->calculator = UI_CALCULATOR_HP50G_NEWRPL;
    else {
        fprintf( stderr, "Invalid calculator type, reverting to default\n" );
        ui->calculator = UI_CALCULATOR_HP50G;
    }

    x49gp_module_get_string( module, keyfile, "name",
                             ui->calculator == UI_CALCULATOR_HP49GP          ? "HP 49g+"
                             : ui->calculator == UI_CALCULATOR_HP49GP_NEWRPL ? "HP 49g+ / newRPL"
                             : ui->calculator == UI_CALCULATOR_HP50G         ? "HP 50g"
                                                                             : "HP 50g / newRPL",
                             &( ui->name ) );

    fd = x49gp_module_open_rodata( module,
                                   ui->calculator == UI_CALCULATOR_HP49GP || ui->calculator == UI_CALCULATOR_HP49GP_NEWRPL
                                       ? "hp49g+-cropped.png"
                                       : "hp50g-cropped.png",
                                   &imagefile );
    if ( fd < 0 )
        return fd;

    gdk_pixbuf_get_file_info( imagefile, &ui->width, &ui->height );

    ui->lcd_width = 131 * 2;
    ui->lcd_top_margin = 16;
    ui->lcd_height = 80 * 2 + ui->lcd_top_margin;

    ui->lcd_x_offset = ( ui->width - ui->lcd_width ) / 2;
    ui->lcd_y_offset = 48; // 69;

    ui->kb_x_offset = 10;  // 36;
    ui->kb_y_offset = 280; // 301;

    ui->bg_pixbuf = gdk_pixbuf_new_from_file( imagefile, &gerror );
    close( fd );

    ui->window = gtk_window_new( GTK_WINDOW_TOPLEVEL );
    gtk_widget_set( ui->window, "can-focus", TRUE, NULL );
    gtk_widget_set( ui->window, "accept-focus", TRUE, NULL );
    gtk_widget_set( ui->window, "focus-on-map", TRUE, NULL );
    gtk_widget_set( ui->window, "resizable", FALSE, NULL );
    gtk_window_set_decorated( GTK_WINDOW( ui->window ), TRUE );

    gtk_widget_set_name( ui->window, ui->name );
    gtk_window_set_title( GTK_WINDOW( ui->window ), ui->name );

    //	gtk_window_set_icon(GTK_WINDOW(ui->window), ui->bg_pixbuf);

    gdk_pixbuf_render_pixmap_and_mask( ui->bg_pixbuf, NULL, &shape, 255 );

    gtk_widget_set_size_request( ui->window, ui->width, ui->height );
    gtk_widget_shape_combine_mask( ui->window, shape, 0, 0 );

    g_object_unref( shape );

    gtk_widget_realize( ui->window );

    ui->shapes[ UI_SHAPE_BUTTON_TINY ] =
        gdk_bitmap_create_from_data( NULL, ( char* )button_tiny_bits, button_tiny_width, button_tiny_height );
    ui->shapes[ UI_SHAPE_BUTTON_SMALL ] =
        gdk_bitmap_create_from_data( NULL, ( char* )button_small_bits, button_small_width, button_small_height );
    ui->shapes[ UI_SHAPE_BUTTON_NORMAL ] =
        gdk_bitmap_create_from_data( NULL, ( char* )button_normal_bits, button_normal_width, button_normal_height );
    ui->shapes[ UI_SHAPE_BUTTON_LARGE ] =
        gdk_bitmap_create_from_data( NULL, ( char* )button_large_bits, button_large_width, button_large_height );
    ui->shapes[ UI_SHAPE_BUTTON_ROUND ] =
        gdk_bitmap_create_from_data( NULL, ( char* )button_round_bits, button_round_width, button_round_height );

    ui->fixed = gtk_fixed_new();
    gtk_container_add( GTK_CONTAINER( ui->window ), ui->fixed );

    ui->background = gtk_drawing_area_new();
    gtk_drawing_area_size( GTK_DRAWING_AREA( ui->background ), ui->width, ui->height );
    x49gp_ui_place_at( x49gp, GTK_FIXED( ui->fixed ), ui->background, 0, 0, ui->width, ui->height );

    x49gp_ui_color_init( &ui->colors[ UI_COLOR_BLACK ], 0x00, 0x00, 0x00 );
    x49gp_ui_color_init( &ui->colors[ UI_COLOR_WHITE ], 0xff, 0xff, 0xff );
    x49gp_ui_color_init( &ui->colors[ UI_COLOR_YELLOW ], 0xfa, 0xe8, 0x2c );
    x49gp_ui_color_init( &ui->colors[ UI_COLOR_RED ], 0x8e, 0x25, 0x18 );
    x49gp_ui_color_init( &ui->colors[ UI_COLOR_GREEN ], 0x14, 0x4d, 0x49 );
    x49gp_ui_color_init( &ui->colors[ UI_COLOR_SILVER ], 0xe0, 0xe0, 0xe0 );
    x49gp_ui_color_init( &ui->colors[ UI_COLOR_ORANGE ], 0xc0, 0x6e, 0x60 );
    x49gp_ui_color_init( &ui->colors[ UI_COLOR_BLUE ], 0x40, 0x60, 0xa4 );

    x49gp_ui_color_init( &ui->colors[ UI_COLOR_GRAYSCALE_0 ], 0xab, 0xd2, 0xb4 );
    x49gp_ui_color_init( &ui->colors[ UI_COLOR_GRAYSCALE_1 ], 0xa0, 0xc4, 0xa8 );
    x49gp_ui_color_init( &ui->colors[ UI_COLOR_GRAYSCALE_2 ], 0x94, 0xb6, 0x9c );
    x49gp_ui_color_init( &ui->colors[ UI_COLOR_GRAYSCALE_3 ], 0x89, 0xa8, 0x90 );
    x49gp_ui_color_init( &ui->colors[ UI_COLOR_GRAYSCALE_4 ], 0x7d, 0x9a, 0x84 );
    x49gp_ui_color_init( &ui->colors[ UI_COLOR_GRAYSCALE_5 ], 0x72, 0x8c, 0x78 );
    x49gp_ui_color_init( &ui->colors[ UI_COLOR_GRAYSCALE_6 ], 0x67, 0x7e, 0x6c );
    x49gp_ui_color_init( &ui->colors[ UI_COLOR_GRAYSCALE_7 ], 0x5b, 0x70, 0x60 );
    x49gp_ui_color_init( &ui->colors[ UI_COLOR_GRAYSCALE_8 ], 0x50, 0x62, 0x54 );
    x49gp_ui_color_init( &ui->colors[ UI_COLOR_GRAYSCALE_9 ], 0x44, 0x54, 0x48 );
    x49gp_ui_color_init( &ui->colors[ UI_COLOR_GRAYSCALE_10 ], 0x39, 0x46, 0x3c );
    x49gp_ui_color_init( &ui->colors[ UI_COLOR_GRAYSCALE_11 ], 0x2e, 0x38, 0x30 );
    x49gp_ui_color_init( &ui->colors[ UI_COLOR_GRAYSCALE_12 ], 0x22, 0x2a, 0x24 );
    x49gp_ui_color_init( &ui->colors[ UI_COLOR_GRAYSCALE_13 ], 0x17, 0x1c, 0x18 );
    x49gp_ui_color_init( &ui->colors[ UI_COLOR_GRAYSCALE_14 ], 0x0b, 0x03, 0x0c );
    x49gp_ui_color_init( &ui->colors[ UI_COLOR_GRAYSCALE_15 ], 0x00, 0x00, 0x00 );

    ui->lcd_canvas = gtk_drawing_area_new();
    gtk_drawing_area_size( GTK_DRAWING_AREA( ui->lcd_canvas ), ui->lcd_width, ui->lcd_height );
    screen_box = gtk_event_box_new();
    gtk_event_box_set_visible_window( GTK_EVENT_BOX( screen_box ), TRUE );
    gtk_event_box_set_above_child( GTK_EVENT_BOX( screen_box ), FALSE );
    gtk_container_add( GTK_CONTAINER( screen_box ), ui->lcd_canvas );
    x49gp_ui_place_at( x49gp, GTK_FIXED( ui->fixed ), screen_box, ui->lcd_x_offset, ui->lcd_y_offset, ui->lcd_width, ui->lcd_height );

    for ( int i = 0; i < ui->nr_buttons; i++ ) {
        switch ( ui->calculator ) {
            case UI_CALCULATOR_HP49GP:
                key = &x49gp_ui_keys[ i ];
                break;
            case UI_CALCULATOR_HP49GP_NEWRPL:
                key = &x49gp_newrpl_ui_keys[ i ];
                break;
            default:
                ui->calculator = UI_CALCULATOR_HP50G;
                /* fall through */
            case UI_CALCULATOR_HP50G:
                key = &x50g_ui_keys[ i ];
                break;
            case UI_CALCULATOR_HP50G_NEWRPL:
                key = &x50g_newrpl_ui_keys[ i ];
                break;
        }
        button = &ui->buttons[ i ];

        button->x49gp = x49gp;
        button->key = key;

        button->button = gtk_button_new();
        gtk_widget_set_size_request( button->button, key->width, key->height );
        gtk_widget_set( button->button, "can-focus", FALSE, NULL );

        x49gp_ui_button_pixmaps_init( x49gp, button, key->color );

        if ( key->label ) {
            button->label = gtk_label_new( "" );
            gtk_widget_set_style( button->label, button->button->style );
            gtk_container_add( GTK_CONTAINER( button->button ), button->label );

            g_signal_connect( G_OBJECT( button->label ), "expose-event", G_CALLBACK( x49gp_button_expose_event ), button );

            g_signal_connect_after( G_OBJECT( button->label ), "realize", G_CALLBACK( x49gp_button_realize ), button );
        }

        button->box = gtk_event_box_new();
        gtk_event_box_set_visible_window( GTK_EVENT_BOX( button->box ), TRUE );
        gtk_event_box_set_above_child( GTK_EVENT_BOX( button->box ), FALSE );
        gtk_widget_shape_combine_mask( button->box, ui->shapes[ key->shape ], 0, 0 );
        gtk_container_add( GTK_CONTAINER( button->box ), button->button );

        x49gp_ui_place_at( x49gp, GTK_FIXED( ui->fixed ), button->box, ui->kb_x_offset + key->x, ui->kb_y_offset + key->y, key->width,
                           key->height );

        g_signal_connect( G_OBJECT( button->button ), "button-press-event", G_CALLBACK( x49gp_ui_button_press ), button );

        g_signal_connect( G_OBJECT( button->button ), "button-release-event", G_CALLBACK( x49gp_ui_button_release ), button );

        g_signal_connect( G_OBJECT( button->button ), "leave-notify-event", G_CALLBACK( x49gp_ui_button_leave ), button );

        gtk_widget_add_events( button->button, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_LEAVE_NOTIFY_MASK );
    }

    ui->menu = gtk_menu_new();

    menu_mount_folder = gtk_menu_item_new_with_label( "Mount SD folder ..." );
    gtk_menu_shell_append( GTK_MENU_SHELL( ui->menu ), menu_mount_folder );
    g_signal_connect( G_OBJECT( menu_mount_folder ), "activate", G_CALLBACK( x49gp_ui_mount_sd_folder ), x49gp );
    menu_mount_image = gtk_menu_item_new_with_label( "Mount SD image ..." );
    gtk_menu_shell_append( GTK_MENU_SHELL( ui->menu ), menu_mount_image );
    g_signal_connect( G_OBJECT( menu_mount_image ), "activate", G_CALLBACK( x49gp_ui_mount_sd_image ), x49gp );
    menu_unmount = gtk_menu_item_new_with_label( "Unmount SD" );
    gtk_menu_shell_append( GTK_MENU_SHELL( ui->menu ), menu_unmount );
    g_signal_connect_swapped( G_OBJECT( menu_unmount ), "activate", G_CALLBACK( s3c2410_sdi_unmount ), x49gp );
    ui->menu_unmount = menu_unmount;

    if ( x49gp->debug_port != 0 ) {
        gtk_menu_shell_append( GTK_MENU_SHELL( ui->menu ), gtk_separator_menu_item_new() );
        menu_debug = gtk_menu_item_new_with_label( "Start debugger" );
        gtk_menu_shell_append( GTK_MENU_SHELL( ui->menu ), menu_debug );
        g_signal_connect( G_OBJECT( menu_debug ), "activate", G_CALLBACK( x49gp_ui_debug ), x49gp );
        ui->menu_debug = menu_debug;
    } else
        ui->menu_debug = NULL;

    gtk_menu_shell_append( GTK_MENU_SHELL( ui->menu ), gtk_separator_menu_item_new() );
    menu_reset = gtk_menu_item_new_with_label( "Reset" );
    gtk_menu_shell_append( GTK_MENU_SHELL( ui->menu ), menu_reset );
    g_signal_connect( G_OBJECT( menu_reset ), "activate", G_CALLBACK( x49gp_ui_calculator_reset ), x49gp );
    menu_quit = gtk_menu_item_new_with_label( "Quit" );
    gtk_menu_shell_append( GTK_MENU_SHELL( ui->menu ), menu_quit );
    g_signal_connect_swapped( G_OBJECT( menu_quit ), "activate", G_CALLBACK( x49gp_ui_quit ), x49gp );

    gtk_widget_show_all( ui->menu );

    g_signal_connect( G_OBJECT( screen_box ), "button-press-event", G_CALLBACK( x49gp_ui_show_menu ), x49gp );

    g_signal_connect( G_OBJECT( ui->background ), "configure-event", G_CALLBACK( x49gp_window_configure_event ), x49gp );

    g_signal_connect( G_OBJECT( ui->lcd_canvas ), "expose-event", G_CALLBACK( x49gp_lcd_expose_event ), x49gp );
    g_signal_connect( G_OBJECT( ui->lcd_canvas ), "configure-event", G_CALLBACK( x49gp_lcd_configure_event ), x49gp );

    g_signal_connect_swapped( G_OBJECT( ui->window ), "delete-event", G_CALLBACK( x49gp_ui_quit ), x49gp );
    g_signal_connect_swapped( G_OBJECT( ui->window ), "destroy", G_CALLBACK( x49gp_ui_quit ), x49gp );

    g_signal_connect( G_OBJECT( ui->window ), "focus-out-event", G_CALLBACK( x49gp_ui_focus_lost ), x49gp );

    g_signal_connect( G_OBJECT( ui->window ), "key-press-event", G_CALLBACK( x49gp_ui_key_event ), x49gp );
    g_signal_connect( G_OBJECT( ui->window ), "key-release-event", G_CALLBACK( x49gp_ui_key_event ), x49gp );

    g_signal_connect( G_OBJECT( ui->window ), "button-press-event", G_CALLBACK( x49gp_window_button_press ), x49gp );

    gtk_widget_add_events( ui->window, GDK_FOCUS_CHANGE_MASK | GDK_BUTTON_PRESS_MASK | GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK );

    gtk_widget_show_all( ui->window );
    return 0;
}

static int gui_save( x49gp_module_t* module, GKeyFile* keyfile )
{
    x49gp_ui_t* ui = module->user_data;

    x49gp_module_set_string( module, keyfile, "type",
                             ui->calculator == UI_CALCULATOR_HP49GP          ? "hp49g+"
                             : ui->calculator == UI_CALCULATOR_HP49GP_NEWRPL ? "hp49g+/newrpl"
                             : ui->calculator == UI_CALCULATOR_HP50G         ? "hp50g"
                                                                             : "hp50g/newrpl" );
    x49gp_module_set_string( module, keyfile, "name", ui->name );

    return 0;
}

int x49gp_ui_init( x49gp_t* x49gp )
{
    x49gp_module_t* module;

    if ( x49gp_module_init( x49gp, "gui", gui_init, gui_exit, gui_reset, gui_load, gui_save, NULL, &module ) )
        return -1;

    return x49gp_module_register( module );
}
