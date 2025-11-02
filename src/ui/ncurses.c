#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <locale.h>

#include <curses.h>

#include "../options.h"
#include "../emulator.h"

#include "inner.h"

#define LCD_OFFSET_X 1
#define LCD_OFFSET_Y 1
#define LCD_BOTTOM LCD_OFFSET_Y + ( LCD_HEIGHT / ( opt.tiny ? 4 : ( opt.small ? 2 : 1 ) ) )
#define LCD_RIGHT LCD_OFFSET_X + ( LCD_WIDTH / ( opt.small || opt.tiny ? 2 : 1 ) ) + 1

/*************/
/* variables */
/*************/
static int display_buffer_grayscale[ LCD_WIDTH * LCD_HEIGHT ];
static char last_annunciators = 0;

static bool keyboard_state[ NB_HP50g_KEYS ];

static WINDOW* lcd_window;
static WINDOW* help_window;

/****************************/
/* functions implementation */
/****************************/
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

static inline void ncurses_draw_lcd_tiny( void )
{
    bool b1, b2, b3, b4, b5, b6, b7, b8;
    int step_x = 2;
    int step_y = 4;
    bool last_column = false;

    wchar_t line[ 66 ]; /* ( LCD_WIDTH / step_x ) + 1 */
    wchar_t pixels;

    for ( int y = 0; y < LCD_HEIGHT; y += step_y ) {
        wcscpy( line, L"" );

        for ( int x = 0; x < LCD_WIDTH; x += step_x ) {
            last_column = x == ( LCD_WIDTH - 1 );

            b1 = display_buffer_grayscale[ ( y * LCD_WIDTH ) + x ] > 0;
            b2 = display_buffer_grayscale[ ( ( y + 1 ) * LCD_WIDTH ) + x ] > 0;
            b3 = display_buffer_grayscale[ ( ( y + 2 ) * LCD_WIDTH ) + x ] > 0;
            b7 = display_buffer_grayscale[ ( ( y + 3 ) * LCD_WIDTH ) + x ] > 0;

            if ( last_column )
                b4 = b5 = b6 = b8 = 0;
            else {
                b4 = display_buffer_grayscale[ ( y * LCD_WIDTH ) + x + 1 ] > 0 ? 1 : 0;
                b5 = display_buffer_grayscale[ ( ( y + 1 ) * LCD_WIDTH ) + x + 1 ] > 0 ? 1 : 0;
                b6 = display_buffer_grayscale[ ( ( y + 2 ) * LCD_WIDTH ) + x + 1 ] > 0 ? 1 : 0;
                b8 = display_buffer_grayscale[ ( ( y + 3 ) * LCD_WIDTH ) + x + 1 ] > 0 ? 1 : 0;
            }

            pixels = eight_bits_to_braille_char( b1, b2, b3, b4, b5, b6, b7, b8 );
            wcsncat( line, &pixels, 1 );
        }
        mvwaddwstr( lcd_window, LCD_OFFSET_Y + ( y / step_y ), LCD_OFFSET_X, line );
    }
}

static inline wchar_t four_bits_to_quadrant_char( bool top_left, bool top_right, bool bottom_left, bool bottom_right )
{
    if ( top_left ) {
        if ( top_right ) {
            if ( bottom_left )
                return bottom_right ? L'█' : L'▛'; /* 0x2588 0x2598 */
            else
                return bottom_right ? L'▜' : L'▀'; /* 0x259C 0x2580 */
        } else {
            if ( bottom_left )
                return bottom_right ? L'▙' : L'▌';
            else
                return bottom_right ? L'▚' : L'▘';
        }
    } else {
        if ( top_right ) {
            if ( bottom_left )
                return bottom_right ? L'▟' : L'▞';
            else
                return bottom_right ? L'▐' : L'▝';
        } else {
            if ( bottom_left )
                return bottom_right ? L'▄' : L'▖';
            else
                return bottom_right ? L'▗' : L' ';
        }
    }
}

static inline void ncurses_draw_lcd_small( void )
{
    bool top_left, top_right, bottom_left, bottom_right;
    int step_x = 2;
    int step_y = 2;
    bool last_column = false;

    wchar_t line[ 66 ]; /* ( LCD_WIDTH / step_x ) + 1 */
    wchar_t pixels;

    for ( int y = 0; y < LCD_HEIGHT; y += step_y ) {
        wcscpy( line, L"" );

        for ( int x = 0; x < LCD_WIDTH; x += step_x ) {
            last_column = x == ( LCD_WIDTH - 1 );

            top_left = display_buffer_grayscale[ ( y * LCD_WIDTH ) + x ] > 0 ? 1 : 0;
            bottom_left = display_buffer_grayscale[ ( ( y + 1 ) * LCD_WIDTH ) + x ] > 0 ? 1 : 0;

            if ( last_column )
                top_right = bottom_right = 0;
            else {
                top_right = display_buffer_grayscale[ ( y * LCD_WIDTH ) + x + 1 ] > 0 ? 1 : 0;
                bottom_right = display_buffer_grayscale[ ( ( y + 1 ) * LCD_WIDTH ) + x + 1 ] > 0 ? 1 : 0;
            }

            pixels = four_bits_to_quadrant_char( top_left, top_right, bottom_left, bottom_right );
            wcsncat( line, &pixels, 1 );
        }
        mvwaddwstr( lcd_window, LCD_OFFSET_Y + ( y / step_y ), LCD_OFFSET_X, line );
    }
}

static inline void ncurses_draw_lcd_fullsize( void )
{
    int val;
    wchar_t pixel;

    wchar_t line[ LCD_WIDTH ];

    for ( int y = 0; y < LCD_HEIGHT; ++y ) {
        wcscpy( line, L"" );
        for ( int x = 0; x < LCD_WIDTH; ++x ) {
            val = display_buffer_grayscale[ ( y * LCD_WIDTH ) + x ];

            switch ( val ) {
                case 0:
                    pixel = L' ';
                    break;
                case 1:
                    pixel = L'░';
                    break;
                case 2:
                    pixel = L'▒';
                    break;
                case 3:
                    pixel = L'█';
                    break;
            }
            wcsncat( line, &pixel, 1 );
        }
        mvwaddwstr( lcd_window, LCD_OFFSET_Y + y, LCD_OFFSET_X, line );
    }
}

static inline void ncurses_show_help( void )
{
    if ( help_window == NULL ) {
        help_window = newwin( 7, LCD_RIGHT + 1, LCD_BOTTOM + 1, 0 );
        refresh();

        wborder( help_window, 0, 0, 0, 0, 0, 0, 0, 0 );

        mvwprintw( help_window, 0, 2, "[ Help ]" );
        mvwprintw( help_window, 1, 1, "Special keys:" );
        mvwprintw( help_window, 2, 2, "F1: Help, F7: Quit" );

        mvwprintw( help_window, 3, 1, "Calculator keys:" );
        mvwprintw( help_window, 4, 2, "all alpha-numerical keys " );
        mvwprintw( help_window, 5, 2, "F2: Left-Shift, F3: Right-Shift, F4: Alpha, F5: On, F6: Enter" );

        wrefresh( help_window );
    } else {
        wclear( help_window );
        wrefresh( help_window );
        // delwin( help_window );
        refresh();

        help_window = NULL;
    }
}

static void ncurses_refresh_annunciators( void )
{
    int annunciators = get_annunciators();

    if ( last_annunciators == annunciators )
        return;

    last_annunciators = annunciators;

    for ( int i = 0; i < NB_ANNUNCIATORS; i++ )
        mvwaddstr( lcd_window, 0, 4 + ( i * 4 ), ( ( annunciators >> i ) & 0x01 ) ? ui_annunciators[ i ] : " " );
}

/**********/
/* Public */
/**********/

void ncurses_refresh_lcd( x50ng_t* x50ng )
{
    if ( !is_display_on() )
        return;

    ncurses_refresh_annunciators();

    get_lcd_buffer( display_buffer_grayscale );

    if ( opt.small )
        ncurses_draw_lcd_small();
    else if ( opt.tiny )
        ncurses_draw_lcd_tiny();
    else
        ncurses_draw_lcd_fullsize();

    wrefresh( lcd_window );
}

void ncurses_handle_pending_inputs( x50ng_t* x50ng )
{
    bool new_keyboard_state[ NB_HP50g_KEYS ];
    uint32_t k;

    // each run records the state of the keyboard (pressed keys)
    // This allow to diff with previous state and issue PRESS and RELEASE calls
    for ( int key = 0; key < NB_HP50g_KEYS; key++ )
        new_keyboard_state[ key ] = false;

    // READ KB STATE
    /* Iterate over all currently pressed keys and mark them as pressed */
    while ( ( k = getch() ) ) {
        if ( k == ( uint32_t )ERR )
            break;

        switch ( k ) {
            case '0':
                new_keyboard_state[ HP50g_KEY_0 ] = true;
                break;
            case '1':
                new_keyboard_state[ HP50g_KEY_1 ] = true;
                break;
            case '2':
                new_keyboard_state[ HP50g_KEY_2 ] = true;
                break;
            case '3':
                new_keyboard_state[ HP50g_KEY_3 ] = true;
                break;
            case '4':
                new_keyboard_state[ HP50g_KEY_4 ] = true;
                break;
            case '5':
                new_keyboard_state[ HP50g_KEY_5 ] = true;
                break;
            case '6':
                new_keyboard_state[ HP50g_KEY_6 ] = true;
                break;
            case '7':
                new_keyboard_state[ HP50g_KEY_7 ] = true;
                break;
            case '8':
                new_keyboard_state[ HP50g_KEY_8 ] = true;
                break;
            case '9':
                new_keyboard_state[ HP50g_KEY_9 ] = true;
                break;
            case 'a':
                new_keyboard_state[ HP50g_KEY_A ] = true;
                break;
            case 'b':
                new_keyboard_state[ HP50g_KEY_B ] = true;
                break;
            case 'c':
                new_keyboard_state[ HP50g_KEY_C ] = true;
                break;
            case 'd':
                new_keyboard_state[ HP50g_KEY_D ] = true;
                break;
            case 'e':
                new_keyboard_state[ HP50g_KEY_E ] = true;
                break;
            case 'f':
                new_keyboard_state[ HP50g_KEY_F ] = true;
                break;
            case 'g':
                new_keyboard_state[ HP50g_KEY_G ] = true;
                break;
            case 'h':
                new_keyboard_state[ HP50g_KEY_H ] = true;
                break;
            case 'i':
                new_keyboard_state[ HP50g_KEY_I ] = true;
                break;
            case 'j':
                new_keyboard_state[ HP50g_KEY_J ] = true;
                break;
            case 'k':
                new_keyboard_state[ HP50g_KEY_K ] = true;
                break;
            case KEY_UP:
                new_keyboard_state[ HP50g_KEY_UP ] = true;
                break;
            case 'l':
                new_keyboard_state[ HP50g_KEY_L ] = true;
                break;
            case 'm':
                new_keyboard_state[ HP50g_KEY_M ] = true;
                break;
            case 'n':
                new_keyboard_state[ HP50g_KEY_N ] = true;
                break;
            case 'o':
                new_keyboard_state[ HP50g_KEY_O ] = true;
                break;
            case 'p':
                new_keyboard_state[ HP50g_KEY_P ] = true;
                break;
            case KEY_LEFT:
                new_keyboard_state[ HP50g_KEY_LEFT ] = true;
                break;
            case 'q':
                new_keyboard_state[ HP50g_KEY_Q ] = true;
                break;
            case KEY_DOWN:
                new_keyboard_state[ HP50g_KEY_DOWN ] = true;
                break;
            case 'r':
                new_keyboard_state[ HP50g_KEY_R ] = true;
                break;
            case KEY_RIGHT:
                new_keyboard_state[ HP50g_KEY_RIGHT ] = true;
                break;
            case 's':
                new_keyboard_state[ HP50g_KEY_S ] = true;
                break;
            case 't':
                new_keyboard_state[ HP50g_KEY_T ] = true;
                break;
            case 'u':
                new_keyboard_state[ HP50g_KEY_U ] = true;
                break;
            case 'v':
                new_keyboard_state[ HP50g_KEY_V ] = true;
                break;
            case 'w':
                new_keyboard_state[ HP50g_KEY_W ] = true;
                break;
            case 'x':
                new_keyboard_state[ HP50g_KEY_X ] = true;
                break;
            case 'y':
                new_keyboard_state[ HP50g_KEY_Y ] = true;
                break;
            case 'z':
            case '/':
                new_keyboard_state[ HP50g_KEY_Z ] = true;
                break;
            case ' ':
                new_keyboard_state[ HP50g_KEY_SPACE ] = true;
                break;
            case KEY_DC:
            case KEY_BACKSPACE:
            case 127:
            case '\b':
                new_keyboard_state[ HP50g_KEY_BACKSPACE ] = true;
                break;
            case '.':
                new_keyboard_state[ HP50g_KEY_PERIOD ] = true;
                break;
            case '+':
                new_keyboard_state[ HP50g_KEY_PLUS ] = true;
                break;
            case '-':
                new_keyboard_state[ HP50g_KEY_MINUS ] = true;
                break;
            case '*':
                new_keyboard_state[ HP50g_KEY_MULTIPLY ] = true;
                break;

            case KEY_F( 2 ):
            case '[':
            case 339: /* PgUp */
                new_keyboard_state[ HP50g_KEY_SHIFT_LEFT ] = true;
                break;
            case KEY_F( 3 ):
            case ']':
            case 338: /* PgDn */
                new_keyboard_state[ HP50g_KEY_SHIFT_RIGHT ] = true;
                break;
            case KEY_F( 4 ):
            case ';':
            case KEY_IC: /* Ins */
                new_keyboard_state[ HP50g_KEY_ALPHA ] = true;
                break;
            case KEY_F( 5 ):
            case '\\':
            case 27:  /* Esc */
            case 262: /* Home */
                new_keyboard_state[ HP50g_KEY_ON ] = true;
                break;
            case KEY_F( 6 ):
            case KEY_ENTER:
            case '\n':
            case ',':
            case 13:
                new_keyboard_state[ HP50g_KEY_ENTER ] = true;
                break;

            case KEY_F( 1 ):
                ncurses_show_help();
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

    for ( int key = 0; key < NB_HP50g_KEYS; key++ ) {
        if ( keyboard_state[ key ] == new_keyboard_state[ key ] )
            continue; /* key hasn't changed state */

        if ( !keyboard_state[ key ] && new_keyboard_state[ key ] && !is_key_pressed( key ) )
            press_key( key );
        else if ( keyboard_state[ key ] && !new_keyboard_state[ key ] && is_key_pressed( key ) )
            release_key( key );

        keyboard_state[ key ] = new_keyboard_state[ key ];
    }
}

void ncurses_exit( void )
{
    delwin( lcd_window );
    delwin( help_window );

    nodelay( stdscr, FALSE );
    echo();
    endwin();
}

void ncurses_init( x50ng_t* x50ng )
{
    for ( int i = 0; i < NB_HP50g_KEYS; ++i )
        keyboard_state[ i ] = false;

    setlocale( LC_ALL, "" );
    initscr();              /* initialize the curses library */
    keypad( stdscr, TRUE ); /* enable keyboard mapping */
    nodelay( stdscr, TRUE );
    curs_set( 0 );
    cbreak(); /* take input chars one at a time, no wait for \n */
    noecho();
    nonl(); /* tell curses not to do NL->CR/NL on output */

    lcd_window = newwin( LCD_BOTTOM + 1, LCD_RIGHT + 1, 0, 0 );
    refresh();

    wborder( lcd_window, 0, 0, 0, 0, 0, 0, 0, 0 );

    mvwprintw( lcd_window, 0, 2, "[   |   |   |   |   |   ]" ); /* annunciators */
    mvwprintw( lcd_window, 0, LCD_RIGHT / 2, "< %s v%i.%i.%i >", opt.name, VERSION_MAJOR, VERSION_MINOR, PATCHLEVEL );

    wrefresh( lcd_window );
}
