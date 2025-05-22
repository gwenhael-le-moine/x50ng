#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <locale.h>

#include <curses.h>

#include "options.h"
#include "x50ng.h"
#include "ui.h"
#include "ui_inner.h"
#include "s3c2410.h"

/* #include "gdbstub.h" */
#include "types.h"

#define LCD_OFFSET_X 1
#define LCD_OFFSET_Y 1
#define LCD_BOTTOM LCD_OFFSET_Y + ( LCD_HEIGHT / 4 )
#define LCD_RIGHT LCD_OFFSET_X + ( LCD_WIDTH / 2 ) + 1

static inline wchar_t eight_bits_to_braille_char( bool b1, bool b2, bool b3, bool b4, bool b5, bool b6, bool b7, bool b8 )
{
    /*********/
    /* b1 b4 */
    /* b2 b5 */
    /* b3 b6 */
    /* b7 b8 */
    /*********/
    wchar_t chr = 0x2800;

    if ( b1 )
        chr |= 1; // 0b0000000000000001;
    if ( b2 )
        chr |= 2; // 0b0000000000000010;
    if ( b3 )
        chr |= 4; // 0b0000000000000100;
    if ( b4 )
        chr |= 8; // 0b0000000000001000;
    if ( b5 )
        chr |= 16; // 0b0000000000010000;
    if ( b6 )
        chr |= 32; // 0b0000000000100000;
    if ( b7 )
        chr |= 64; // 0b0000000001000000;
    if ( b8 )
        chr |= 128; // 0b0000000010000000;

    return chr;
}

/**********/
/* Public */
/**********/

/* void tui_update_lcd( x50ng_t* x50ng ) */
void tui_refresh_lcd( x50ng_t* x50ng )
{
    s3c2410_lcd_t* lcd = x50ng->s3c2410_lcd;

    if ( !( lcd->lcdcon1 & 1 ) )
        return;

    /* annunciators */
    const wchar_t* annunciators_icons[ 6 ] = { L"↰", L"↱", L"α", L"🪫", L"⌛", L"⇄" };
    const int annunciators_pixel_index[ 6 ] = { 1, 2, 3, 4, 5, 0 };

    for ( int i = 0; i < 6; i++ )
        mvaddwstr(
            0, 4 + ( i * 4 ),
            ( x50ng_s3c2410_get_pixel_color( lcd, LCD_WIDTH, annunciators_pixel_index[ i ] ) > 0 ? annunciators_icons[ i ] : L" " ) );

    /* pixels */
    bool b1, b2, b3, b4, b5, b6, b7, b8;
    int step_x = 2;
    int step_y = 4;

    wchar_t line[ 66 ]; /* ( LCD_WIDTH / step_x ) + 1 */

    for ( int y = 0; y < LCD_HEIGHT; y += step_y ) {
        wcscpy( line, L"" );

        for ( int x = 0; x < LCD_WIDTH; x += step_x ) {
            b1 = x50ng_s3c2410_get_pixel_color( lcd, x, y ) > 0;
            b4 = x50ng_s3c2410_get_pixel_color( lcd, x + 1, y ) > 0;

            b2 = x50ng_s3c2410_get_pixel_color( lcd, x, y + 1 ) > 0;
            b5 = x50ng_s3c2410_get_pixel_color( lcd, x + 1, y + 1 ) > 0;

            b3 = x50ng_s3c2410_get_pixel_color( lcd, x, y + 2 ) > 0;
            b6 = x50ng_s3c2410_get_pixel_color( lcd, x + 1, y + 2 ) > 0;

            b7 = x50ng_s3c2410_get_pixel_color( lcd, x, y + 3 ) > 0;
            b8 = x50ng_s3c2410_get_pixel_color( lcd, x + 1, y + 3 ) > 0;

            wchar_t pixels = eight_bits_to_braille_char( b1, b2, b3, b4, b5, b6, b7, b8 );
            wcsncat( line, &pixels, 1 );
        }
        mvaddwstr( LCD_OFFSET_Y + ( y / step_y ), LCD_OFFSET_X, line );
    }

    wrefresh( stdscr );
}

void tui_handle_pending_inputs( x50ng_t* x50ng )
{
    // FIXME: not implemented, likely needs raw mode or something

    /* int hpkey = -1; */
    /* uint32_t k; */

    /* /\* Start fresh and mark all keys as released *\/ */
    /* for ( int i = 0; i < NB_KEYS; ++i ) */
    /*     X50NG_RELEASE_KEY( x50ng, &ui_keys[ i ] ) */

    /* /\* Iterate over all currently pressed keys and mark them as pressed *\/ */
    /* while ( ( k = getch() ) ) { */
    /*     if ( k == ( uint32_t )ERR ) */
    /*         break; */

    /*     switch ( k ) { */
    /*         case '0': */
    /*             hpkey = HPKEY_0; */
    /*             break; */
    /*         case '1': */
    /*             hpkey = HPKEY_1; */
    /*             break; */
    /*         case '2': */
    /*             hpkey = HPKEY_2; */
    /*             break; */
    /*         case '3': */
    /*             hpkey = HPKEY_3; */
    /*             break; */
    /*         case '4': */
    /*             hpkey = HPKEY_4; */
    /*             break; */
    /*         case '5': */
    /*             hpkey = HPKEY_5; */
    /*             break; */
    /*         case '6': */
    /*             hpkey = HPKEY_6; */
    /*             break; */
    /*         case '7': */
    /*             hpkey = HPKEY_7; */
    /*             break; */
    /*         case '8': */
    /*             hpkey = HPKEY_8; */
    /*             break; */
    /*         case '9': */
    /*             hpkey = HPKEY_9; */
    /*             break; */
    /*         case 'a': */
    /*             hpkey = HPKEY_A; */
    /*             break; */
    /*         case 'b': */
    /*             hpkey = HPKEY_B; */
    /*             break; */
    /*         case 'c': */
    /*             hpkey = HPKEY_C; */
    /*             break; */
    /*         case 'd': */
    /*             hpkey = HPKEY_D; */
    /*             break; */
    /*         case 'e': */
    /*             hpkey = HPKEY_E; */
    /*             break; */
    /*         case 'f': */
    /*             hpkey = HPKEY_F; */
    /*             break; */
    /*         case 'g': */
    /*             hpkey = HPKEY_G; */
    /*             break; */
    /*         case 'h': */
    /*             hpkey = HPKEY_H; */
    /*             break; */
    /*         case 'i': */
    /*             hpkey = HPKEY_I; */
    /*             break; */
    /*         case 'j': */
    /*             hpkey = HPKEY_J; */
    /*             break; */
    /*         case 'k': */
    /*             hpkey = HPKEY_K; */
    /*             break; */
    /*         case KEY_UP: */
    /*             hpkey = HPKEY_UP; */
    /*             break; */
    /*         case 'l': */
    /*             hpkey = HPKEY_L; */
    /*             break; */
    /*         case 'm': */
    /*             hpkey = HPKEY_M; */
    /*             break; */
    /*         case 'n': */
    /*             hpkey = HPKEY_N; */
    /*             break; */
    /*         case 'o': */
    /*             hpkey = HPKEY_O; */
    /*             break; */
    /*         case 'p': */
    /*             hpkey = HPKEY_P; */
    /*             break; */
    /*         case KEY_LEFT: */
    /*             hpkey = HPKEY_LEFT; */
    /*             break; */
    /*         case 'q': */
    /*             hpkey = HPKEY_Q; */
    /*             break; */
    /*         case KEY_DOWN: */
    /*             hpkey = HPKEY_DOWN; */
    /*             break; */
    /*         case 'r': */
    /*             hpkey = HPKEY_R; */
    /*             break; */
    /*         case KEY_RIGHT: */
    /*             hpkey = HPKEY_RIGHT; */
    /*             break; */
    /*         case 's': */
    /*             hpkey = HPKEY_S; */
    /*             break; */
    /*         case 't': */
    /*             hpkey = HPKEY_T; */
    /*             break; */
    /*         case 'u': */
    /*             hpkey = HPKEY_U; */
    /*             break; */
    /*         case 'v': */
    /*             hpkey = HPKEY_V; */
    /*             break; */
    /*         case 'w': */
    /*             hpkey = HPKEY_W; */
    /*             break; */
    /*         case 'x': */
    /*             hpkey = HPKEY_X; */
    /*             break; */
    /*         case 'y': */
    /*             hpkey = HPKEY_Y; */
    /*             break; */
    /*         case 'z': */
    /*         case '/': */
    /*             hpkey = HPKEY_Z; */
    /*             break; */
    /*         case ' ': */
    /*             hpkey = HPKEY_SPACE; */
    /*             break; */
    /*         case KEY_DC: */
    /*         case KEY_BACKSPACE: */
    /*         case 127: */
    /*         case '\b': */
    /*             hpkey = HPKEY_BACKSPACE; */
    /*             break; */
    /*         case '.': */
    /*             hpkey = HPKEY_PERIOD; */
    /*             break; */
    /*         case '+': */
    /*             hpkey = HPKEY_PLUS; */
    /*             break; */
    /*         case '-': */
    /*             hpkey = HPKEY_MINUS; */
    /*             break; */
    /*         case '*': */
    /*             hpkey = HPKEY_MULTIPLY; */
    /*             break; */

    /*         case KEY_F( 1 ): */
    /*         case KEY_ENTER: */
    /*         case '\n': */
    /*         case ',': */
    /*         case 13: */
    /*             hpkey = HPKEY_ENTER; */
    /*             break; */
    /*         case KEY_F( 2 ): */
    /*         case '[': */
    /*         case 339: /\* PgUp *\/ */
    /*             hpkey = HPKEY_SHIFT_LEFT; */
    /*             break; */
    /*         case KEY_F( 3 ): */
    /*         case ']': */
    /*         case 338: /\* PgDn *\/ */
    /*             hpkey = HPKEY_SHIFT_RIGHT; */
    /*             break; */
    /*         case KEY_F( 4 ): */
    /*         case ';': */
    /*         case KEY_IC: /\* Ins *\/ */
    /*             hpkey = HPKEY_ALPHA; */
    /*             break; */
    /*         case KEY_F( 5 ): */
    /*         case '\\': */
    /*         case 27:  /\* Esc *\/ */
    /*         case 262: /\* Home *\/ */
    /*             hpkey = HPKEY_ON; */
    /*             break; */

    /*         case KEY_F( 7 ): */
    /*         case '|':      /\* Shift+\ *\/ */
    /*         case KEY_SEND: /\* Shift+End *\/ */
    /*         case KEY_F( 10 ): */
    /*             x50ng->arm_exit = 1; */
    /*             cpu_exit( x50ng->env ); */
    /*             break; */
    /*     } */

    /*     X50NG_PRESS_KEY( x50ng, &ui_keys[ hpkey ] ) */
    /* } */
}

void tui_init( x50ng_t* x50ng )
{

    setlocale( LC_ALL, "" );
    initscr();              /* initialize the curses library */
    keypad( stdscr, TRUE ); /* enable keyboard mapping */
    nodelay( stdscr, TRUE );
    curs_set( 0 );
    cbreak(); /* take input chars one at a time, no wait for \n */
    noecho();
    nonl(); /* tell curses not to do NL->CR/NL on output */

    mvaddch( 0, 0, ACS_ULCORNER );
    mvaddch( LCD_BOTTOM, 0, ACS_LLCORNER );
    mvaddch( 0, LCD_RIGHT, ACS_URCORNER );
    mvaddch( LCD_BOTTOM, LCD_RIGHT, ACS_LRCORNER );
    mvhline( 0, 1, ACS_HLINE, LCD_RIGHT - 1 );
    mvhline( LCD_BOTTOM, 1, ACS_HLINE, LCD_RIGHT - 1 );
    mvvline( 1, 0, ACS_VLINE, LCD_BOTTOM - 1 );
    mvvline( 1, LCD_RIGHT, ACS_VLINE, LCD_BOTTOM - 1 );

    mvprintw( 0, 2, "[   |   |   |   |   |   ]" ); /* annunciators */
    mvprintw( 0, 30, "< %s v%i.%i.%i >", opt.name, VERSION_MAJOR, VERSION_MINOR, PATCHLEVEL );

    mvprintw( LCD_BOTTOM + 1, 0, "F1: Enter, F2: Left-Shift, F3: Right-Shift, F4: Alpha, F5: On, F7: Quit" );
}

void tui_exit( void )
{
    nodelay( stdscr, FALSE );
    echo();
    endwin();
}
