#include <gtk/gtk.h>
#include <glib.h>
#include <cairo.h>
#include <gdk/gdkkeysyms.h>

#include "../options.h"
#include "../module.h"
#include "../s3c2410/s3c2410.h"
#include "../gdbstub.h"
#include "../types.h"
#include "../emulator.h"

#include "ui_inner.h"

// #define TEST_PASTE true

typedef struct {
    x50ng_t* x50ng;
    const x50ng_ui_key_t* key;
    GtkWidget* button;
    bool down;
    bool hold;
} x50ng_ui_button_t;

/*************/
/* Variables */
/*************/
static GtkWidget* gui_annunciators[ NB_ANNUNCIATORS ] = { NULL, NULL, NULL, NULL, NULL, NULL };

static x50ng_ui_button_t* gui_buttons;

static GtkWidget* gui_window;

static GtkWidget* gui_lcd_canvas;
static cairo_surface_t* gui_lcd_surface;

static char last_annunciators = 0;
static int display_buffer_grayscale[ LCD_WIDTH * LCD_HEIGHT ];

/*************************/
/* Functions' prototypes */
/*************************/
static void gui_open_menu( int x, int y, x50ng_t* x50ng );

/*************/
/* Functions */
/*************/
static void gui_release_button( x50ng_ui_button_t* button )
{
    if ( !button->down )
        return;

    const x50ng_ui_key_t* key = button->key;

    button->down = false;
    button->hold = false;

    gtk_widget_remove_css_class( button->button, "key-down" );

    // X50NG_RELEASE_KEY( x50ng, key );
    release_key( key->hpkey );
}

static bool gui_press_button( x50ng_ui_button_t* button, bool hold )
{
    const x50ng_ui_key_t* key = button->key;

    if ( button->down ) {
        if ( button->hold && hold ) {
            gui_release_button( button );
            return GDK_EVENT_STOP;
        } else
            return GDK_EVENT_PROPAGATE;
    }

    button->down = true;
    button->hold = hold;

    gtk_widget_add_css_class( button->button, "key-down" );

    // X50NG_RELEASE_KEY( x50ng, key );
    press_key( key->hpkey );

    return GDK_EVENT_STOP;
}

static void gui_react_to_button_press( GtkGesture* _gesture, int _n_press, double _x, double _y, x50ng_ui_button_t* button )
{
    const x50ng_ui_key_t* key = button->key;

    gui_press_button( button, false );

    // X50NG_PRESS_KEY( x50ng, key );
    press_key( key->hpkey );
}

static void gui_react_to_button_release( GtkGesture* _gesture, int _n_press, double _x, double _y, x50ng_ui_button_t* button )
{
    gui_release_button( button );
}

static void gui_react_to_button_right_click_release( x50ng_ui_button_t* button, GtkGesture* _gesture, int _n_press, double _x, double _y )
{
    const x50ng_ui_key_t* key = button->key;

    button->down = true;
    button->hold = true;

    gui_press_button( button, true );

    // X50NG_PRESS_KEY( x50ng, key );
    press_key( key->hpkey );
}

static void gui_mount_sd_folder_file_dialog_callback( GtkFileDialog* dialog, GAsyncResult* result, x50ng_t* x50ng )
{
    g_autoptr( GFile ) file = gtk_file_dialog_select_folder_finish( dialog, result, NULL );

    if ( file != NULL )
        s3c2410_sdi_mount( x50ng, ( char* )g_file_peek_path( file ) );
}

static void gui_do_select_and_mount_sd_folder( x50ng_t* x50ng, GMenuItem* _menuitem )
{
    g_autoptr( GtkFileDialog ) dialog =
        g_object_new( GTK_TYPE_FILE_DIALOG, "title", "Choose SD folder…", "accept-label", "_Open", "modal", TRUE, NULL );

    gtk_file_dialog_select_folder( dialog, GTK_WINDOW( gui_window ), NULL, ( GAsyncReadyCallback )gui_mount_sd_folder_file_dialog_callback,
                                   x50ng );
}

static void gui_do_start_gdb_server( GMenuItem* _menuitem, x50ng_t* x50ng )
{
    if ( opt.debug_port != 0 && !gdbserver_isactive() ) {
        gdbserver_start( opt.debug_port );
        gdb_handlesig( x50ng->env, 0 );
    }
}

static void gui_do_reset( x50ng_t* x50ng, GMenuItem* _menuitem )
{
    x50ng_modules_reset( x50ng, X50NG_RESET_POWER_ON );
    cpu_reset( x50ng->env );
    x50ng_set_idle( x50ng, 0 );
}

#ifdef TEST_PASTE
static void x50g_string_to_keys_sequence( x50ng_t* x50ng, const char* input )
{
    for ( int i = 0; i < strlen( input ); i++ ) {
        fprintf( stderr, "%c", input[ i ] );
    }
    fprintf( stderr, "\n" );
}

static void gui_paste_callback( GdkClipboard* source, GAsyncResult* result, x50ng_t* x50ng )
{
    g_autofree char* text = NULL;
    g_autoptr( GError ) error = NULL;

    text = gdk_clipboard_read_text_finish( source, result, &error );

    if ( error ) {
        g_critical( "Couldn't paste text: %s\n", error->message );
        return;
    }

    x50g_string_to_keys_sequence( x50ng, text );
}

static void gui_do_paste( x50ng_t* x50ng, GtkWidget* _menuitem )
{
    gdk_clipboard_read_text_async( gdk_display_get_clipboard( gdk_display_get_default() ), NULL, ( GAsyncReadyCallback )gui_paste_callback,
                                   x50ng );
}
#endif

static void gui_do_quit( x50ng_t* x50ng, GtkWidget* _menuitem ) { x50ng->arm_exit++; }

static void gui_open_menu( int x, int y, x50ng_t* x50ng )
{
    g_autoptr( GMenu ) menu = g_menu_new();
    g_autoptr( GSimpleActionGroup ) action_group = g_simple_action_group_new();

#ifdef TEST_PASTE
    g_autoptr( GSimpleAction ) act_paste = g_simple_action_new( "paste", NULL );
    g_signal_connect_swapped( act_paste, "activate", G_CALLBACK( gui_do_paste ), x50ng );
    g_action_map_add_action( G_ACTION_MAP( action_group ), G_ACTION( act_paste ) );
    g_menu_append( menu, "Paste", "app.paste" );
#endif

    g_autoptr( GSimpleAction ) act_mount_SD = g_simple_action_new( "mount_SD", NULL );
    g_signal_connect_swapped( act_mount_SD, "activate", G_CALLBACK( gui_do_select_and_mount_sd_folder ), x50ng );
    if ( !s3c2410_sdi_is_mounted( x50ng ) )
        g_action_map_add_action( G_ACTION_MAP( action_group ), G_ACTION( act_mount_SD ) );
    g_menu_append( menu, "Mount SD folder…", "app.mount_SD" );

    g_autoptr( GSimpleAction ) act_unmount_SD = g_simple_action_new( "unmount_SD", NULL );
    g_signal_connect_swapped( act_unmount_SD, "activate", G_CALLBACK( s3c2410_sdi_unmount ), x50ng );
    char* unmount_label;
    if ( s3c2410_sdi_is_mounted( x50ng ) ) {
        g_action_map_add_action( G_ACTION_MAP( action_group ), G_ACTION( act_unmount_SD ) );
        char* sd_path;
        s3c2410_sdi_get_path( x50ng, &sd_path );
        if ( -1 == asprintf( &unmount_label, "Unmount SD (%s)", sd_path ) )
            exit( EXIT_FAILURE );
        free( sd_path );
    } else if ( -1 == asprintf( &unmount_label, "Unmount SD" ) )
        exit( EXIT_FAILURE );
    g_menu_append( menu, unmount_label, "app.unmount_SD" );
    free( unmount_label );

    g_autoptr( GSimpleAction ) act_debug = g_simple_action_new( "debug", NULL );
    g_signal_connect_swapped( act_debug, "activate", G_CALLBACK( gui_do_start_gdb_server ), x50ng );
    if ( opt.debug_port != 0 )
        g_action_map_add_action( G_ACTION_MAP( action_group ), G_ACTION( act_debug ) );
    g_menu_append( menu, "Start gdb server", "app.debug" );

    g_autoptr( GSimpleAction ) act_reset = g_simple_action_new( "reset", NULL );
    g_signal_connect_swapped( act_reset, "activate", G_CALLBACK( gui_do_reset ), x50ng );
    g_action_map_add_action( G_ACTION_MAP( action_group ), G_ACTION( act_reset ) );
    g_menu_append( menu, "Reset", "app.reset" );

    g_autoptr( GSimpleAction ) act_quit = g_simple_action_new( "quit", NULL );
    g_signal_connect_swapped( act_quit, "activate", G_CALLBACK( gui_do_quit ), x50ng );
    g_action_map_add_action( G_ACTION_MAP( action_group ), G_ACTION( act_quit ) );
    g_menu_append( menu, "Quit", "app.quit" );

    GtkWidget* popup = gtk_popover_menu_new_from_model( G_MENU_MODEL( menu ) );
    gtk_widget_insert_action_group( popup, "app", G_ACTION_GROUP( action_group ) );

    GdkRectangle rect;
    rect.x = x;
    rect.y = y;
    rect.width = rect.height = 1;
    gtk_popover_set_pointing_to( GTK_POPOVER( popup ), &rect );

    gtk_widget_set_parent( GTK_WIDGET( popup ), gui_window );
    gtk_popover_set_position( GTK_POPOVER( popup ), GTK_POS_BOTTOM );
    gtk_popover_popup( GTK_POPOVER( popup ) );
}

static void gui_redraw_lcd( GtkDrawingArea* _widget, cairo_t* cr, int width, int height, gpointer _user_data )
{
    cairo_pattern_t* lcd_pattern = cairo_pattern_create_for_surface( gui_lcd_surface );
    cairo_pattern_set_filter( lcd_pattern, CAIRO_FILTER_FAST );
    cairo_scale( cr, ( double )width / ( double )LCD_WIDTH, ( double )height / ( double )LCD_HEIGHT );
    cairo_set_source( cr, lcd_pattern );

    cairo_paint( cr );
}

static bool gui_handle_key_event( int keyval, x50ng_t* x50ng, int event_type )
{
    int hpkey;
    switch ( keyval ) {
        case GDK_KEY_a:
        case GDK_KEY_F1:
            hpkey = HPKEY_A;
            break;
        case GDK_KEY_b:
        case GDK_KEY_F2:
            hpkey = HPKEY_B;
            break;
        case GDK_KEY_c:
        case GDK_KEY_F3:
            hpkey = HPKEY_C;
            break;
        case GDK_KEY_d:
        case GDK_KEY_F4:
            hpkey = HPKEY_D;
            break;
        case GDK_KEY_e:
        case GDK_KEY_F5:
            hpkey = HPKEY_E;
            break;
        case GDK_KEY_f:
        case GDK_KEY_F6:
            hpkey = HPKEY_F;
            break;
        case GDK_KEY_g:
            hpkey = HPKEY_G;
            break;
        case GDK_KEY_h:
            hpkey = HPKEY_H;
            break;
        case GDK_KEY_i:
            hpkey = HPKEY_I;
            break;
        case GDK_KEY_j:
            hpkey = HPKEY_J;
            break;
        case GDK_KEY_k:
            hpkey = HPKEY_K;
            break;
        case GDK_KEY_l:
            hpkey = HPKEY_L;
            break;
        case GDK_KEY_Up:
        case GDK_KEY_KP_Up:
            hpkey = HPKEY_UP;
            break;
        case GDK_KEY_Left:
        case GDK_KEY_KP_Left:
            hpkey = HPKEY_LEFT;
            break;
        case GDK_KEY_Down:
        case GDK_KEY_KP_Down:
            hpkey = HPKEY_DOWN;
            break;
        case GDK_KEY_Right:
        case GDK_KEY_KP_Right:
            hpkey = HPKEY_RIGHT;
            break;
        case GDK_KEY_m:
            hpkey = HPKEY_M;
            break;
        case GDK_KEY_n:
            hpkey = HPKEY_N;
            break;
        case GDK_KEY_o:
        case GDK_KEY_apostrophe:
            hpkey = HPKEY_O;
            break;
        case GDK_KEY_p:
            hpkey = HPKEY_P;
            break;
        case GDK_KEY_BackSpace:
        case GDK_KEY_Delete:
        case GDK_KEY_KP_Delete:
            hpkey = HPKEY_BACKSPACE;
            break;
        case GDK_KEY_dead_circumflex:
        case GDK_KEY_asciicircum:
        case GDK_KEY_q:
        case GDK_KEY_caret:
            hpkey = HPKEY_Q;
            break;
        case GDK_KEY_r:
            hpkey = HPKEY_R;
            break;
        case GDK_KEY_s:
            hpkey = HPKEY_S;
            break;
        case GDK_KEY_t:
            hpkey = HPKEY_T;
            break;
        case GDK_KEY_u:
            hpkey = HPKEY_U;
            break;
        case GDK_KEY_v:
            hpkey = HPKEY_V;
            break;
        case GDK_KEY_w:
            hpkey = HPKEY_W;
            break;
        case GDK_KEY_x:
            hpkey = HPKEY_X;
            break;
        case GDK_KEY_y:
            hpkey = HPKEY_Y;
            break;
        case GDK_KEY_z:
        case GDK_KEY_slash:
        case GDK_KEY_KP_Divide:
            hpkey = HPKEY_Z;
            break;
        case GDK_KEY_Tab:
#ifndef __APPLE__
        case GDK_KEY_Alt_L:
        case GDK_KEY_Alt_R:
        case GDK_KEY_Meta_L:
        case GDK_KEY_Meta_R:
        case GDK_KEY_Mode_switch:
#endif
            hpkey = HPKEY_ALPHA;
            break;
        case GDK_KEY_7:
        case GDK_KEY_KP_7:
            hpkey = HPKEY_7;
            break;
        case GDK_KEY_8:
        case GDK_KEY_KP_8:
            hpkey = HPKEY_8;
            break;
        case GDK_KEY_9:
        case GDK_KEY_KP_9:
            hpkey = HPKEY_9;
            break;
        case GDK_KEY_multiply:
        case GDK_KEY_KP_Multiply:
            hpkey = HPKEY_MULTIPLY;
            break;
        case GDK_KEY_Shift_L:
        case GDK_KEY_Shift_R:
            hpkey = HPKEY_SHIFT_LEFT;
            break;
        case GDK_KEY_4:
        case GDK_KEY_KP_4:
            hpkey = HPKEY_4;
            break;
        case GDK_KEY_5:
        case GDK_KEY_KP_5:
            hpkey = HPKEY_5;
            break;
        case GDK_KEY_6:
        case GDK_KEY_KP_6:
            hpkey = HPKEY_6;
            break;
        case GDK_KEY_minus:
        case GDK_KEY_KP_Subtract:
            hpkey = HPKEY_MINUS;
            break;
        case GDK_KEY_Control_L:
        case GDK_KEY_Control_R:
            hpkey = HPKEY_SHIFT_RIGHT;
            break;
        case GDK_KEY_1:
        case GDK_KEY_KP_1:
            hpkey = HPKEY_1;
            break;
        case GDK_KEY_2:
        case GDK_KEY_KP_2:
            hpkey = HPKEY_2;
            break;
        case GDK_KEY_3:
        case GDK_KEY_KP_3:
            hpkey = HPKEY_3;
            break;
        case GDK_KEY_plus:
        case GDK_KEY_KP_Add:
            hpkey = HPKEY_PLUS;
            break;
        case GDK_KEY_Escape:
            hpkey = HPKEY_ON;
            break;
        case GDK_KEY_0:
        case GDK_KEY_KP_0:
            hpkey = HPKEY_0;
            break;
        case GDK_KEY_period:
        case GDK_KEY_comma:
        case GDK_KEY_KP_Decimal:
        case GDK_KEY_KP_Separator:
            hpkey = HPKEY_PERIOD;
            break;
        case GDK_KEY_space:
        case GDK_KEY_KP_Space:
            hpkey = HPKEY_SPACE;
            break;
        case GDK_KEY_Return:
        case GDK_KEY_KP_Enter:
            hpkey = HPKEY_ENTER;
            break;

        /* QWERTY compat: US English, UK English, International English */
        case GDK_KEY_backslash:
            hpkey = HPKEY_MULTIPLY;
            break;
        case GDK_KEY_equal:
            hpkey = HPKEY_PLUS;
            break;

        /* QWERTZ compat: German */
        case GDK_KEY_ssharp:
            hpkey = HPKEY_Z;
            break;
        case GDK_KEY_numbersign:
            hpkey = HPKEY_MULTIPLY;
            break;

        case GDK_KEY_F7:
        case GDK_KEY_F10:
            x50ng->arm_exit = 1;
            cpu_exit( x50ng->env );
            return GDK_EVENT_STOP;

        case GDK_KEY_F12:
            switch ( event_type ) {
                case KEY_PRESS:
                    x50ng_modules_reset( x50ng, X50NG_RESET_POWER_ON );
                    cpu_reset( x50ng->env );
                    x50ng_set_idle( x50ng, 1 );
                    break;
                case KEY_RELEASE:
                    x50ng_set_idle( x50ng, 0 );
                    break;
                default:
                    break;
            }
            return GDK_EVENT_STOP;

        case GDK_KEY_Menu:
            gui_open_menu( ( LCD_WIDTH * opt.zoom ) / 2, ( LCD_HEIGHT * opt.zoom ) / 2, x50ng );
            return GDK_EVENT_STOP;

        default:
            return GDK_EVENT_PROPAGATE;
    }

    // Using GUI buttons:
    switch ( event_type ) {
        case KEY_PRESS:
            gui_react_to_button_press( NULL, 0, 0, 0, &gui_buttons[ hpkey ] );
            break;
        case KEY_RELEASE:
            gui_react_to_button_release( NULL, 0, 0, 0, &gui_buttons[ hpkey ] );
            break;
        default:
            return GDK_EVENT_PROPAGATE;
    }

    return GDK_EVENT_STOP;
}

static bool gui_react_to_key_press( GtkEventControllerKey* controller, guint keyval, guint keycode, GdkModifierType state, x50ng_t* x50ng )
{
    return gui_handle_key_event( keyval, x50ng, KEY_PRESS );
}

static bool gui_react_to_key_release( GtkEventControllerKey* controller, guint keyval, guint keycode, GdkModifierType state,
                                      x50ng_t* x50ng )
{
    return gui_handle_key_event( keyval, x50ng, KEY_RELEASE );
}

static void gui_react_to_display_click( x50ng_t* x50ng, GtkEventController* _gesture, gdouble x, gdouble y )
{
    gui_open_menu( ( int )x, ( int )y, x50ng );
}

static GtkWidget* _gui_activate__create_annunciator_widget( const char* label )
{
    GtkWidget* gui_ann = gtk_label_new( NULL );
    gtk_widget_add_css_class( gui_ann, "annunciator" );
    gtk_widget_set_name( gui_ann, label );

    gtk_label_set_use_markup( GTK_LABEL( gui_ann ), true );
    gtk_label_set_markup( GTK_LABEL( gui_ann ), label );

    gtk_widget_set_opacity( gui_ann, 0 );

    return gui_ann;
}

static GtkWidget* _gui_activate__create_label( const char* css_class, const char* text )
{
    GtkWidget* gui_label = gtk_label_new( NULL );
    gtk_widget_add_css_class( gui_label, css_class );

    gtk_label_set_use_markup( GTK_LABEL( gui_label ), true );
    gtk_label_set_markup( GTK_LABEL( gui_label ), text );

    return gui_label;
}

static void _gui_activate__load_and_apply_CSS( x50ng_t* x50ng )
{
    char* style_full_path = g_build_filename( opt.style_filename, NULL );
    if ( !g_file_test( style_full_path, G_FILE_TEST_EXISTS ) )
        style_full_path = g_build_filename( opt.datadir, opt.style_filename, NULL );
    if ( !g_file_test( style_full_path, G_FILE_TEST_EXISTS ) )
        style_full_path = g_build_filename( GLOBAL_DATADIR, opt.style_filename, NULL );
    if ( !g_file_test( style_full_path, G_FILE_TEST_EXISTS ) )
        style_full_path = g_build_filename( x50ng->progpath, opt.style_filename, NULL );

    if ( !g_file_test( style_full_path, G_FILE_TEST_EXISTS ) )
        fprintf( stderr, "Can't load style %s neither from %s/%s nor from %s/%s nor from %s/%s\n", opt.style_filename, opt.datadir,
                 opt.style_filename, GLOBAL_DATADIR, opt.style_filename, x50ng->progpath, opt.style_filename );
    else {
        g_autoptr( GtkCssProvider ) style_provider = gtk_css_provider_new();
        gtk_css_provider_load_from_path( style_provider, style_full_path );

        gtk_style_context_add_provider_for_display( gdk_display_get_default(), GTK_STYLE_PROVIDER( style_provider ),
                                                    GTK_STYLE_PROVIDER_PRIORITY_USER + 1 );

        if ( opt.verbose )
            fprintf( stderr, "Loaded style from %s\n", style_full_path );
    }

    free( style_full_path );
}

static void gui_activate( GtkApplication* app, x50ng_t* x50ng )
{
    // create gui_window and widgets/stuff
    if ( app == NULL )
        gui_window = gtk_window_new();
    else
        gui_window = gtk_application_window_new( app );

    gtk_window_set_decorated( GTK_WINDOW( gui_window ), true );
    gtk_window_set_resizable( GTK_WINDOW( gui_window ), true );
    gtk_window_set_title( GTK_WINDOW( gui_window ), opt.name );
    gtk_window_set_decorated( GTK_WINDOW( gui_window ), true );
    // Sets the title of this instance
    g_set_application_name( opt.name );
    // Sets the app_id of all instances
    g_set_prgname( x50ng->progname );

    GtkWidget* window_container = gtk_box_new( opt.netbook ? GTK_ORIENTATION_HORIZONTAL : GTK_ORIENTATION_VERTICAL, 0 );
    gtk_widget_add_css_class( window_container, "window-container" );
    gtk_widget_set_name( window_container, "window-container" );

    gtk_window_set_child( ( GtkWindow* )gui_window, window_container );

    g_signal_connect_swapped( G_OBJECT( gui_window ), "destroy", G_CALLBACK( gui_do_quit ), x50ng );

    GtkEventController* keys_controller = gtk_event_controller_key_new();
    g_signal_connect( keys_controller, "key-pressed", G_CALLBACK( gui_react_to_key_press ), x50ng );
    g_signal_connect( keys_controller, "key-released", G_CALLBACK( gui_react_to_key_release ), x50ng );
    gtk_widget_add_controller( gui_window, keys_controller );

    /* for --netbook */
    GtkWidget* upper_left_container = gtk_box_new( GTK_ORIENTATION_VERTICAL, 0 );
    gtk_widget_add_css_class( upper_left_container, "upper-left-container" );
    gtk_widget_set_name( upper_left_container, "upper-left-container" );
    gtk_box_append( ( GTK_BOX( window_container ) ), upper_left_container );

    GtkWidget* downer_right_container = gtk_box_new( GTK_ORIENTATION_VERTICAL, 0 );
    gtk_widget_set_vexpand( GTK_WIDGET( downer_right_container ), true );
    gtk_widget_add_css_class( downer_right_container, "downer-right-container" );
    gtk_widget_set_name( downer_right_container, "downer-right-container" );
    gtk_box_append( GTK_BOX( window_container ), downer_right_container );

    GtkWidget* header_container = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 0 );
    gtk_widget_add_css_class( header_container, "header-container" );
    gtk_widget_set_name( header_container, "header-container" );
    gtk_box_append( ( GTK_BOX( upper_left_container ) ), header_container );

    gui_lcd_canvas = gtk_drawing_area_new();
    gtk_widget_add_css_class( gui_lcd_canvas, "lcd" );
    gtk_widget_set_name( gui_lcd_canvas, "lcd" );

    gtk_drawing_area_set_content_width( GTK_DRAWING_AREA( gui_lcd_canvas ), ( LCD_WIDTH * opt.zoom ) );
    gtk_drawing_area_set_content_height( GTK_DRAWING_AREA( gui_lcd_canvas ), ( LCD_HEIGHT * opt.zoom ) );
    gtk_drawing_area_set_draw_func( GTK_DRAWING_AREA( gui_lcd_canvas ), gui_redraw_lcd, x50ng, NULL );

    GtkWidget* lcd_container = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 0 );
    gtk_widget_set_halign( lcd_container, GTK_ALIGN_CENTER );
    gtk_widget_add_css_class( lcd_container, "lcd-container" );
    gtk_widget_set_name( lcd_container, "lcd-container" );

    gtk_widget_set_size_request( lcd_container, ( LCD_WIDTH * opt.zoom ), ( LCD_HEIGHT * opt.zoom ) );
    gtk_box_append( GTK_BOX( lcd_container ), gui_lcd_canvas );
    gtk_widget_set_halign( GTK_WIDGET( gui_lcd_canvas ), GTK_ALIGN_CENTER );
    gtk_widget_set_hexpand( GTK_WIDGET( gui_lcd_canvas ), false );

    GtkWidget* annunciators_container = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 0 );
    gtk_box_set_homogeneous( GTK_BOX( annunciators_container ), true );
    gtk_widget_add_css_class( annunciators_container, "annunciators-container" );
    gtk_widget_set_name( annunciators_container, "annunciators-container" );

    for ( int i = 0; i < NB_ANNUNCIATORS; i++ ) {
        gui_annunciators[ i ] = _gui_activate__create_annunciator_widget( ui_annunciators[ i ].icon );
        gtk_box_append( GTK_BOX( annunciators_container ), gui_annunciators[ i ] );
    }

    GtkWidget* display_container = gtk_box_new( GTK_ORIENTATION_VERTICAL, 0 );
    gtk_widget_add_css_class( annunciators_container, "display-container" );
    gtk_widget_set_name( display_container, "display-container" );

    gtk_box_append( GTK_BOX( display_container ), annunciators_container );
    gtk_box_append( GTK_BOX( display_container ), lcd_container );

    gtk_box_append( GTK_BOX( upper_left_container ), display_container );

    GtkGesture* right_click_controller = gtk_gesture_click_new();
    gtk_gesture_single_set_button( GTK_GESTURE_SINGLE( right_click_controller ), 3 );
    g_signal_connect_swapped( right_click_controller, "pressed", G_CALLBACK( gui_react_to_display_click ), x50ng );
    gtk_widget_add_controller( display_container, GTK_EVENT_CONTROLLER( right_click_controller ) );

    // keyboard
    GtkWidget* high_keyboard_container = gtk_box_new( GTK_ORIENTATION_VERTICAL, 0 );
    gtk_widget_add_css_class( high_keyboard_container, "keyboard-container" );
    gtk_widget_set_name( high_keyboard_container, "high-keyboard-container" );

    gtk_box_set_homogeneous( GTK_BOX( high_keyboard_container ), true );

    gtk_box_append( GTK_BOX( upper_left_container ), high_keyboard_container );

    GtkWidget* low_keyboard_container = gtk_box_new( GTK_ORIENTATION_VERTICAL, 0 );
    gtk_widget_set_valign( GTK_WIDGET( low_keyboard_container ), GTK_ALIGN_END );
    gtk_widget_set_vexpand( GTK_WIDGET( low_keyboard_container ), true );
    gtk_widget_add_css_class( low_keyboard_container, "keyboard-container" );
    gtk_widget_set_name( low_keyboard_container, "low-keyboard-container" );

    gtk_box_set_homogeneous( GTK_BOX( low_keyboard_container ), true );

    gtk_box_append( GTK_BOX( downer_right_container ), low_keyboard_container );

    x50ng_ui_button_t* button;

    GtkWidget* rows_containers[ KB_NB_ROWS ];
    GtkWidget* keys_containers[ NB_KEYS ];
    GtkWidget* keys_top_labels_containers[ NB_KEYS ];

    gui_buttons = malloc( NB_KEYS * sizeof( x50ng_ui_button_t ) );
    if ( NULL == gui_buttons ) {
        fprintf( stderr, "%s:%u: Out of memory\n", __func__, __LINE__ );
        return;
    }
    memset( gui_buttons, 0, NB_KEYS * sizeof( x50ng_ui_button_t ) );

    int key_index = 0;
    int nb_keys_in_row = 0;
    for ( int row = 0; row < KB_NB_ROWS; row++ ) {
        rows_containers[ row ] = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 0 );
        gtk_widget_add_css_class( rows_containers[ row ], "row-container" );
        gtk_box_set_homogeneous( GTK_BOX( rows_containers[ row ] ), true );
        gtk_box_append( ( GTK_BOX( row < opt.netbook_pivot_line ? high_keyboard_container : low_keyboard_container ) ),
                        rows_containers[ row ] );

        switch ( row ) {
            case 1:
                nb_keys_in_row = 4;
                break;
            case 0:
            case 2:
                nb_keys_in_row = 6;
                break;
            default:
                nb_keys_in_row = 5;
        }

        for ( int column = 0; column < nb_keys_in_row; column++ ) {
            keys_containers[ key_index ] = gtk_box_new( GTK_ORIENTATION_VERTICAL, 0 );
            gtk_widget_add_css_class( keys_containers[ key_index ], "key-container" );
            gtk_box_set_homogeneous( GTK_BOX( keys_containers[ key_index ] ), false );
            if ( row == 1 && column == 3 )
                gtk_box_append( GTK_BOX( rows_containers[ row ] ), gtk_box_new( GTK_ORIENTATION_VERTICAL, 2 ) );
            gtk_box_append( GTK_BOX( rows_containers[ row ] ), keys_containers[ key_index ] );
            if ( row == 1 && column == 3 )
                gtk_box_append( GTK_BOX( rows_containers[ row ] ), gtk_box_new( GTK_ORIENTATION_VERTICAL, 2 ) );

            button = &gui_buttons[ NORMALIZED_KEYS_ORDER( key_index ) ];
            button->x50ng = x50ng;
            button->key = &ui_keys[ NORMALIZED_KEYS_ORDER( key_index ) ];

            keys_top_labels_containers[ key_index ] = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 0 );
            gtk_widget_add_css_class( keys_top_labels_containers[ key_index ], "top-labels-container" );

            gtk_box_append( GTK_BOX( keys_containers[ key_index ] ), keys_top_labels_containers[ key_index ] );

            GtkWidget* label_left = NULL;
            if ( button->key->left )
                label_left = _gui_activate__create_label( "label-left", button->key->left );

            GtkWidget* label_right = NULL;
            if ( button->key->right )
                label_right = _gui_activate__create_label( "label-right", button->key->right );

            if ( button->key->left && button->key->right ) {
                gtk_box_append( GTK_BOX( keys_top_labels_containers[ key_index ] ), label_left );
                gtk_widget_set_halign( GTK_WIDGET( label_left ), GTK_ALIGN_START );

                gtk_box_append( GTK_BOX( keys_top_labels_containers[ key_index ] ), label_right );
                gtk_widget_set_halign( GTK_WIDGET( label_right ), GTK_ALIGN_END );
                gtk_widget_set_hexpand( label_right, true );
            } else if ( button->key->left ) {
                gtk_widget_set_halign( GTK_WIDGET( keys_top_labels_containers[ key_index ] ), GTK_ALIGN_CENTER );

                gtk_box_append( GTK_BOX( keys_top_labels_containers[ key_index ] ), label_left );
                gtk_widget_set_halign( GTK_WIDGET( label_left ), GTK_ALIGN_CENTER );
                gtk_widget_set_hexpand( GTK_WIDGET( label_left ), false );
            }

            button->button = gtk_button_new();
            gtk_widget_add_css_class( button->button, "key" );
            gtk_widget_add_css_class( button->button, button->key->css_class );
            gtk_widget_set_name( button->button, button->key->css_id );

            // There's always a label, even if it's empty.
            GtkWidget* label = _gui_activate__create_label( "label-key", button->key->label );
            gtk_button_set_child( GTK_BUTTON( button->button ), label );

            gtk_widget_set_can_focus( button->button, false );
            GtkGesture* left_click_controller = gtk_gesture_click_new();
            gtk_gesture_single_set_button( GTK_GESTURE_SINGLE( left_click_controller ), 1 );
            g_signal_connect( left_click_controller, "pressed", G_CALLBACK( gui_react_to_button_press ), button );
            g_signal_connect( left_click_controller, /* "released" */ "end", G_CALLBACK( gui_react_to_button_release ), button );
            /* Here we attach the controller to the label because… gtk4 reasons? gtk4 button only handles 'clicked' event now but we
             * actually need pressed and released (AKA end?) */
            gtk_widget_add_controller( label, GTK_EVENT_CONTROLLER( left_click_controller ) );

            GtkGesture* right_click_controller = gtk_gesture_click_new();
            gtk_gesture_single_set_button( GTK_GESTURE_SINGLE( right_click_controller ), 3 );
            g_signal_connect_swapped( right_click_controller, /* "released" */ "pressed",
                                      G_CALLBACK( gui_react_to_button_right_click_release ), button );
            gtk_widget_add_controller( label, GTK_EVENT_CONTROLLER( right_click_controller ) );

            gtk_box_append( GTK_BOX( keys_containers[ key_index ] ), button->button );

            if ( button->key->below )
                gtk_box_append( GTK_BOX( keys_containers[ key_index ] ), _gui_activate__create_label( "label-below", button->key->below ) );
            if ( button->key->letter )
                gtk_box_append( GTK_BOX( keys_containers[ key_index ] ),
                                _gui_activate__create_label( "label-letter", button->key->letter ) );

            key_index++;
        }
    }

    _gui_activate__load_and_apply_CSS( x50ng );

    // finally show the window
    gtk_widget_realize( gui_window );
    gtk_window_present( GTK_WINDOW( gui_window ) );
}

void gui_handle_pending_inputs( x50ng_t* _x50ng )
{
    while ( g_main_context_pending( NULL ) )
        g_main_context_iteration( NULL, false );
}

static void gui_refresh_annunciators( void )
{
    int annunciators = get_annunciators();

    if ( last_annunciators == annunciators )
        return;

    last_annunciators = annunciators;

    for ( int i = 0; i < NB_ANNUNCIATORS; i++ )
        gtk_widget_set_opacity( gui_annunciators[ i ], ( annunciators >> i ) & 0x01 ? 1 : 0 );
}

void gui_refresh_lcd( x50ng_t* _x50ng )
{
    if ( !get_display_state() )
        return;

    gui_refresh_annunciators();

    if ( NULL != gui_lcd_surface )
        g_free( gui_lcd_surface );
    gui_lcd_surface = cairo_image_surface_create( CAIRO_FORMAT_ARGB32, LCD_WIDTH, LCD_HEIGHT );
    cairo_t* cr = cairo_create( gui_lcd_surface );

    get_lcd_buffer( display_buffer_grayscale );
    for ( int y = 0; y < LCD_HEIGHT; y++ ) {
        for ( int x = 0; x < LCD_WIDTH; x++ ) {
            cairo_set_source_rgba( cr, 0, 0, 0, display_buffer_grayscale[ ( y * LCD_WIDTH ) + x ] / 15.0 );
            cairo_rectangle( cr, x, y, 1.0, 1.0 );
            cairo_fill( cr );
        }
    }

    cairo_destroy( cr );

    gtk_widget_queue_draw( gui_lcd_canvas );

    gdk_display_flush( gdk_display_get_default() );
}

void gui_init( x50ng_t* x50ng )
{
    /* g_autoptr( GtkApplication ) app = gtk_application_new( NULL, 0 ); */

    /* g_signal_connect( app, "activate", G_CALLBACK( gui_activate ), x50ng ); */

    /* g_application_run( G_APPLICATION( app ), 0, NULL ); */

    gtk_init();
    gui_activate( NULL, x50ng );
}

void gui_exit( void ) {}
