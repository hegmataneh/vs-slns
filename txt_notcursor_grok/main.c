#include <ncurses.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>

// Define number of tabs
#define NUM_TABS 3

// Tab names
const char * tab_names[ NUM_TABS ] = { "Tab 1", "Tab 2", "Tab 3" };

// Sample statistics data for each tab (simple table: rows of "Label: Value")
const char * tab_data[ NUM_TABS ][ 5 ][ 2 ] = {
	{{"Stat A", "100"}, {"Stat B", "200"}, {"Stat C", "300"}, {"Long Stat D", "400"}, {NULL, NULL}},
	{{"Stat X", "500"}, {"Stat Y", "600"}, {"Very Long Stat Z", "700"}, {NULL, NULL}, {NULL, NULL}},
	{{"Stat P", "800"}, {"Stat Q", "900"}, {NULL, NULL}, {NULL, NULL}, {NULL, NULL}}
};

// Default min column width
#define MIN_COL_WIDTH 10

// Global variables
int current_tab = 0;
int screen_rows , screen_cols;
WINDOW * tab_win;  // Window for tabs
WINDOW * content_win;  // Window for content
volatile sig_atomic_t resize_flag = 0;

// Signal handler for resize
void handle_resize( int sig )
{
	resize_flag = 1;
}

// Function to calculate max length for label and value columns
void get_max_lengths( int tab , int * max_label , int * max_value )
{
	*max_label = 0;
	*max_value = 0;
	for ( int i = 0; tab_data[ tab ][ i ][ 0 ] != NULL; i++ )
	{
		int len_label = strlen( tab_data[ tab ][ i ][ 0 ] );
		int len_value = strlen( tab_data[ tab ][ i ][ 1 ] );
		if ( len_label > *max_label ) *max_label = len_label;
		if ( len_value > *max_value ) *max_value = len_value;
	}
	// Apply min width
	if ( *max_label < MIN_COL_WIDTH ) *max_label = MIN_COL_WIDTH;
	if ( *max_value < MIN_COL_WIDTH ) *max_value = MIN_COL_WIDTH;
}

// Function to draw tabs
void draw_tabs()
{
	werase( tab_win );
	wattron( tab_win , A_BOLD );
	for ( int i = 0; i < NUM_TABS; i++ )
	{
		if ( i == current_tab )
		{
			wattron( tab_win , A_REVERSE );
		}
		mvwprintw( tab_win , 0 , i * ( strlen( tab_names[ i ] ) + 3 ) + 1 , " %s " , tab_names[ i ] );
		if ( i == current_tab )
		{
			wattroff( tab_win , A_REVERSE );
		}
	}
	wattroff( tab_win , A_BOLD );
	wrefresh( tab_win );
}

// Function to draw content
void draw_content()
{
	werase( content_win );
	int max_label , max_value;
	get_max_lengths( current_tab , &max_label , &max_value );

	// Calculate total width needed
	int total_needed = max_label + max_value + 3;  // +3 for ": " and space

	// Cap to window width minus borders/padding
	int avail_width = screen_cols - 2;  // Assuming 1 char border each side
	if ( total_needed > avail_width )
	{
		// Proportionally reduce, but not below min
		int excess = total_needed - avail_width;
		if ( max_label - excess / 2 >= MIN_COL_WIDTH )
		{
			max_label -= excess / 2;
		}
		else
		{
			max_label = MIN_COL_WIDTH;
		}
		if ( max_value - ( excess - ( excess / 2 ) ) >= MIN_COL_WIDTH )
		{
			max_value -= ( excess - ( excess / 2 ) );
		}
		else
		{
			max_value = MIN_COL_WIDTH;
		}
	}

	// Draw the table
	int row = 1;
	for ( int i = 0; tab_data[ current_tab ][ i ][ 0 ] != NULL; i++ )
	{
		mvwprintw( content_win , row , 1 , "%-*s: %-*s" , max_label , tab_data[ current_tab ][ i ][ 0 ] , max_value , tab_data[ current_tab ][ i ][ 1 ] );
		row++;
	}
	wrefresh( content_win );
}

// Function to handle mouse click
void handle_mouse( MEVENT * event )
{
	if ( event->y == 0 )
	{  // Tabs are on row 0
		int tab_width;
		int x_pos = 0;
		for ( int i = 0; i < NUM_TABS; i++ )
		{
			tab_width = strlen( tab_names[ i ] ) + 3;  // " name "
			if ( event->x >= x_pos && event->x < x_pos + tab_width )
			{
				current_tab = i;
				draw_tabs();
				draw_content();
				return;
			}
			x_pos += tab_width;
		}
	}
}

// Function to resize windows
void resize_windows()
{
	getmaxyx( stdscr , screen_rows , screen_cols );
	wresize( tab_win , 1 , screen_cols );
	wresize( content_win , screen_rows - 1 , screen_cols );
	mvwin( content_win , 1 , 0 );
	draw_tabs();
	draw_content();
}

int main()
{
	// Initialize ncurses
	initscr();
	cbreak();
	noecho();
	keypad( stdscr , TRUE );
	mousemask( ALL_MOUSE_EVENTS , NULL );  // Enable mouse
	curs_set( 0 );  // Hide cursor

	// Set up signal handler for resize
	signal( SIGWINCH , handle_resize );

	// Get initial screen size
	getmaxyx( stdscr , screen_rows , screen_cols );

	// Create windows
	tab_win = newwin( 1 , screen_cols , 0 , 0 );
	content_win = newwin( screen_rows - 1 , screen_cols , 1 , 0 );

	// Initial draw
	draw_tabs();
	draw_content();

	int ch;
	while ( ( ch = getch() ) != 'q' )
	{  // Quit on 'q'
		if ( resize_flag )
		{
			resize_flag = 0;
			endwin();
			refresh();
			resize_windows();
		}
		if ( ch == KEY_MOUSE )
		{
			MEVENT event;
			if ( getmouse( &event ) == OK )
			{
				if ( event.bstate & BUTTON1_PRESSED )
				{
					handle_mouse( &event );
				}
			}
		}
	}

	// Cleanup
	delwin( tab_win );
	delwin( content_win );
	endwin();
	return 0;
}

