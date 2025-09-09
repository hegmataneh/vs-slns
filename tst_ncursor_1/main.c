
#include <ncurses.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>

WINDOW * cell_win , * input_win;
bool resize_flag = false;

void handle_resize( int sig )
{
	resize_flag = true;
}

void init_windows()
{
	int maxy , maxx;
	getmaxyx( stdscr , maxy , maxx );

	// Calculate window sizes (60% for cells, 40% for input)
	int cell_height = maxy * 0.6;
	int input_height = maxy - cell_height;

	// Create or replace windows
	if ( cell_win ) delwin( cell_win );
	if ( input_win ) delwin( input_win );

	cell_win = newwin( cell_height , maxx , 0 , 0 );
	input_win = newwin( input_height , maxx , cell_height , 0 );

	// Enable scrolling and keypad for input window
	scrollok( input_win , TRUE );
	keypad( input_win , TRUE );

	// Set box borders
	box( cell_win , 0 , 0 );
	box( input_win , 0 , 0 );

	// Add titles
	mvwprintw( cell_win , 0 , 2 , " Cell Display " );
	mvwprintw( input_win , 0 , 2 , " Input Area " );

	// Refresh windows
	wrefresh( cell_win );
	wrefresh( input_win );
}

int main()
{
	// Initialize curses

	initscr();
	cbreak();
	noecho();
	curs_set( 1 );

	// Set up resize handler
	signal( SIGWINCH , handle_resize );

	// Initial window creation
	init_windows();

	// Main loop
	int ch;
	while ( ( ch = wgetch( input_win ) ) )
	{
		if ( resize_flag )
		{
			resize_flag = false;
			endwin();
			refresh();
			clear();
			init_windows();

			// You may need to redraw your cell content here
			// For example:
			//redraw_cells(cell_win);

			continue;
		}

		// Handle input
		switch ( ch )
		{
			// Your input handling logic here
			case KEY_RESIZE:
				// This might not be needed if SIGWINCH is handled
				break;
			default:
				waddch( input_win , ch );
				wrefresh( input_win );
				break;
		}
	}

	// Clean up
	delwin( cell_win );
	delwin( input_win );
	endwin();

	return 0;
}

