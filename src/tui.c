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

static bool previous_keyboard_state[ NB_KEYS ];

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
    // each run records the state of the keyboard (pressed keys)
    // This allow to diff with previous state and issue PRESS and RELEASE calls

    bool new_keyboard_state[ NB_KEYS ];
    for ( int key = 0; key < NB_KEYS; ++key )
        new_keyboard_state[ key ] = false;

    // READ KB STATE
    uint32_t k;

    /* Iterate over all currently pressed keys and mark them as pressed */
    while ( ( k = getch() ) ) {
        if ( k == ( uint32_t )ERR )
            break;

        switch ( k ) {
            case '0':
                new_keyboard_state[ HPKEY_0 ] = true;
                break;
            case '1':
                new_keyboard_state[ HPKEY_1 ] = true;
                break;
            case '2':
                new_keyboard_state[ HPKEY_2 ] = true;
                break;
            case '3':
                new_keyboard_state[ HPKEY_3 ] = true;
                break;
            case '4':
                new_keyboard_state[ HPKEY_4 ] = true;
                break;
            case '5':
                new_keyboard_state[ HPKEY_5 ] = true;
                break;
            case '6':
                new_keyboard_state[ HPKEY_6 ] = true;
                break;
            case '7':
                new_keyboard_state[ HPKEY_7 ] = true;
                break;
            case '8':
                new_keyboard_state[ HPKEY_8 ] = true;
                break;
            case '9':
                new_keyboard_state[ HPKEY_9 ] = true;
                break;
            case 'a':
                new_keyboard_state[ HPKEY_A ] = true;
                break;
            case 'b':
                new_keyboard_state[ HPKEY_B ] = true;
                break;
            case 'c':
                new_keyboard_state[ HPKEY_C ] = true;
                break;
            case 'd':
                new_keyboard_state[ HPKEY_D ] = true;
                break;
            case 'e':
                new_keyboard_state[ HPKEY_E ] = true;
                break;
            case 'f':
                new_keyboard_state[ HPKEY_F ] = true;
                break;
            case 'g':
                new_keyboard_state[ HPKEY_G ] = true;
                break;
            case 'h':
                new_keyboard_state[ HPKEY_H ] = true;
                break;
            case 'i':
                new_keyboard_state[ HPKEY_I ] = true;
                break;
            case 'j':
                new_keyboard_state[ HPKEY_J ] = true;
                break;
            case 'k':
                new_keyboard_state[ HPKEY_K ] = true;
                break;
            case KEY_UP:
                new_keyboard_state[ HPKEY_UP ] = true;
                break;
            case 'l':
                new_keyboard_state[ HPKEY_L ] = true;
                break;
            case 'm':
                new_keyboard_state[ HPKEY_M ] = true;
                break;
            case 'n':
                new_keyboard_state[ HPKEY_N ] = true;
                break;
            case 'o':
                new_keyboard_state[ HPKEY_O ] = true;
                break;
            case 'p':
                new_keyboard_state[ HPKEY_P ] = true;
                break;
            case KEY_LEFT:
                new_keyboard_state[ HPKEY_LEFT ] = true;
                break;
            case 'q':
                new_keyboard_state[ HPKEY_Q ] = true;
                break;
            case KEY_DOWN:
                new_keyboard_state[ HPKEY_DOWN ] = true;
                break;
            case 'r':
                new_keyboard_state[ HPKEY_R ] = true;
                break;
            case KEY_RIGHT:
                new_keyboard_state[ HPKEY_RIGHT ] = true;
                break;
            case 's':
                new_keyboard_state[ HPKEY_S ] = true;
                break;
            case 't':
                new_keyboard_state[ HPKEY_T ] = true;
                break;
            case 'u':
                new_keyboard_state[ HPKEY_U ] = true;
                break;
            case 'v':
                new_keyboard_state[ HPKEY_V ] = true;
                break;
            case 'w':
                new_keyboard_state[ HPKEY_W ] = true;
                break;
            case 'x':
                new_keyboard_state[ HPKEY_X ] = true;
                break;
            case 'y':
                new_keyboard_state[ HPKEY_Y ] = true;
                break;
            case 'z':
            case '/':
                new_keyboard_state[ HPKEY_Z ] = true;
                break;
            case ' ':
                new_keyboard_state[ HPKEY_SPACE ] = true;
                break;
            case KEY_DC:
            case KEY_BACKSPACE:
            case 127:
            case '\b':
                new_keyboard_state[ HPKEY_BACKSPACE ] = true;
                break;
            case '.':
                new_keyboard_state[ HPKEY_PERIOD ] = true;
                break;
            case '+':
                new_keyboard_state[ HPKEY_PLUS ] = true;
                break;
            case '-':
                new_keyboard_state[ HPKEY_MINUS ] = true;
                break;
            case '*':
                new_keyboard_state[ HPKEY_MULTIPLY ] = true;
                break;

            case KEY_F( 1 ):
            case KEY_ENTER:
            case '\n':
            case ',':
            case 13:
                new_keyboard_state[ HPKEY_ENTER ] = true;
                break;
            case KEY_F( 2 ):
            case '[':
            case 339: /* PgUp */
                new_keyboard_state[ HPKEY_SHIFT_LEFT ] = true;
                break;
            case KEY_F( 3 ):
            case ']':
            case 338: /* PgDn */
                new_keyboard_state[ HPKEY_SHIFT_RIGHT ] = true;
                break;
            case KEY_F( 4 ):
            case ';':
            case KEY_IC: /* Ins */
                new_keyboard_state[ HPKEY_ALPHA ] = true;
                break;
            case KEY_F( 5 ):
            case '\\':
            case 27:  /* Esc */
            case 262: /* Home */
                new_keyboard_state[ HPKEY_ON ] = true;
                break;

            case KEY_F( 7 ):
            case '|':      /* Shift+\ */
            case KEY_SEND: /* Shift+End */
            case KEY_F( 10 ):
                x50ng->arm_exit = 1;
                cpu_exit( x50ng->env );
                break;
        }
    }

    for ( int key = 0; key < NB_KEYS; ++key ) {
        /* key pressed */
        if ( !previous_keyboard_state[ key ] && new_keyboard_state[ key ] )
            X50NG_PRESS_KEY( x50ng, &ui_keys[ key ] )

        /* key released */
        if ( previous_keyboard_state[ key ] && !new_keyboard_state[ key ] )
            X50NG_RELEASE_KEY( x50ng, &ui_keys[ key ] )

        /* mvaddwstr( 40, key, previous_keyboard_state[ key ] ? L"█" : L"_" ); */
        /* mvaddwstr( 41, key, new_keyboard_state[ key ] ? L"█" : L"_" ); */

        previous_keyboard_state[ key ] = new_keyboard_state[ key ];
    }
}

void tui_init( x50ng_t* x50ng )
{
    for ( int i = 0; i < NB_KEYS; ++i )
        previous_keyboard_state[ i ] = false;

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
