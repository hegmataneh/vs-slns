// multitab_ncurses.c
// Compile: gcc -std=c99 -Wall -Wextra -lncurses -o multitab_ncurses multitab_ncurses.c

#define _XOPEN_SOURCE 700

#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <locale.h>

#define MAX_COLS 8
#define MAX_ROWS 100
#define TAB_PADDING 2
#define MIN_COL_WIDTH 8

typedef struct
{
	const char * name;
	int ncols;
	const char * col_names[ MAX_COLS ];
	const char * rows[ MAX_ROWS ][ MAX_COLS ];
	int nrows;
} Tab;

static Tab tabs[ 3 ];
static int ntabs = 0;
static int active_tab = 0;

static int col_widths[ MAX_COLS ];
static int col_auto_fit[ MAX_COLS ]; // 1 = currently auto-fit to content
static int win_w = 0 , win_h = 0;

static void setup_sample_tabs( void )
{
	// Tab 0
	tabs[ 0 ].name = "Overview";
	tabs[ 0 ].ncols = 3;
	tabs[ 0 ].col_names[ 0 ] = "Metric";
	tabs[ 0 ].col_names[ 1 ] = "Value";
	tabs[ 0 ].col_names[ 2 ] = "Status";
	tabs[ 0 ].nrows = 4;
	tabs[ 0 ].rows[ 0 ][ 0 ] = "CPU Usage"; tabs[ 0 ].rows[ 0 ][ 1 ] = "12%"; tabs[ 0 ].rows[ 0 ][ 2 ] = "OK";
	tabs[ 0 ].rows[ 1 ][ 0 ] = "Memory"; tabs[ 0 ].rows[ 1 ][ 1 ] = "1.3G/4G"; tabs[ 0 ].rows[ 1 ][ 2 ] = "OK";
	tabs[ 0 ].rows[ 2 ][ 0 ] = "Disk /"; tabs[ 0 ].rows[ 2 ][ 1 ] = "80%"; tabs[ 0 ].rows[ 2 ][ 2 ] = "WARN";
	tabs[ 0 ].rows[ 3 ][ 0 ] = "Net In"; tabs[ 0 ].rows[ 3 ][ 1 ] = "120 KB/s"; tabs[ 0 ].rows[ 3 ][ 2 ] = "OK";

	// Tab 1
	tabs[ 1 ].name = "Connections";
	tabs[ 1 ].ncols = 4;
	tabs[ 1 ].col_names[ 0 ] = "Proto";
	tabs[ 1 ].col_names[ 1 ] = "Local";
	tabs[ 1 ].col_names[ 2 ] = "Remote";
	tabs[ 1 ].col_names[ 3 ] = "State";
	tabs[ 1 ].nrows = 5;
	tabs[ 1 ].rows[ 0 ][ 0 ] = "tcp"; tabs[ 1 ].rows[ 0 ][ 1 ] = "127.0.0.1:22"; tabs[ 1 ].rows[ 0 ][ 2 ] = "192.168.1.10:54321"; tabs[ 1 ].rows[ 0 ][ 3 ] = "ESTABLISHED";
	tabs[ 1 ].rows[ 1 ][ 0 ] = "udp"; tabs[ 1 ].rows[ 1 ][ 1 ] = "0.0.0.0:68"; tabs[ 1 ].rows[ 1 ][ 2 ] = "-"; tabs[ 1 ].rows[ 1 ][ 3 ] = "LISTEN";
	tabs[ 1 ].rows[ 2 ][ 0 ] = "tcp"; tabs[ 1 ].rows[ 2 ][ 1 ] = "10.0.0.1:80"; tabs[ 1 ].rows[ 2 ][ 2 ] = "93.184.216.34:44321"; tabs[ 1 ].rows[ 2 ][ 3 ] = "TIME_WAIT";
	tabs[ 1 ].rows[ 3 ][ 0 ] = "tcp"; tabs[ 1 ].rows[ 3 ][ 1 ] = "10.0.0.1:443"; tabs[ 1 ].rows[ 3 ][ 2 ] = "203.0.113.5:53100"; tabs[ 1 ].rows[ 3 ][ 3 ] = "ESTABLISHED";
	tabs[ 1 ].rows[ 4 ][ 0 ] = "udp"; tabs[ 1 ].rows[ 4 ][ 1 ] = "127.0.0.1:123"; tabs[ 1 ].rows[ 4 ][ 2 ] = "-"; tabs[ 1 ].rows[ 4 ][ 3 ] = "LISTEN";

	// Tab 2
	tabs[ 2 ].name = "Processes";
	tabs[ 2 ].ncols = 3;
	tabs[ 2 ].col_names[ 0 ] = "PID";
	tabs[ 2 ].col_names[ 1 ] = "Name";
	tabs[ 2 ].col_names[ 2 ] = "CPU%";
	tabs[ 2 ].nrows = 6;
	tabs[ 2 ].rows[ 0 ][ 0 ] = "1"; tabs[ 2 ].rows[ 0 ][ 1 ] = "systemd"; tabs[ 2 ].rows[ 0 ][ 2 ] = "0.1";
	tabs[ 2 ].rows[ 1 ][ 0 ] = "234"; tabs[ 2 ].rows[ 1 ][ 1 ] = "sshd"; tabs[ 2 ].rows[ 1 ][ 2 ] = "0.0";
	tabs[ 2 ].rows[ 2 ][ 0 ] = "567"; tabs[ 2 ].rows[ 2 ][ 1 ] = "mysqld"; tabs[ 2 ].rows[ 2 ][ 2 ] = "2.4";
	tabs[ 2 ].rows[ 3 ][ 0 ] = "1024"; tabs[ 2 ].rows[ 3 ][ 1 ] = "nginx"; tabs[ 2 ].rows[ 3 ][ 2 ] = "0.3";
	tabs[ 2 ].rows[ 4 ][ 0 ] = "2048"; tabs[ 2 ].rows[ 4 ][ 1 ] = "myapp_long_name_example"; tabs[ 2 ].rows[ 4 ][ 2 ] = "12.1";
	tabs[ 2 ].rows[ 5 ][ 0 ] = "9999"; tabs[ 2 ].rows[ 5 ][ 1 ] = "worker"; tabs[ 2 ].rows[ 5 ][ 2 ] = "0.0";

	ntabs = 3;
	active_tab = 0;
}

static void handle_resize( int sig )
{
	// just mark - ncurses KEY_RESIZE will also be delivered
	( void )sig;
	// noop: we'll catch KEY_RESIZE in main loop
}

static int max_content_width( Tab * t , int col )
{
	int max = ( int )strlen( t->col_names[ col ] );
	for ( int r = 0; r < t->nrows; ++r )
	{
		const char * s = t->rows[ r ][ col ];
		if ( !s ) continue;
		int l = ( int )strlen( s );
		if ( l > max ) max = l;
	}
	return max;
}

// recompute column widths based on content and available window width
static void compute_column_widths( Tab * t )
{
	int n = t->ncols;
	int total_min = 0;
	int content_max[ MAX_COLS ];
	for ( int c = 0; c < n; ++c )
	{
		content_max[ c ] = max_content_width( t , c );
		if ( col_auto_fit[ c ] )
		{
			col_widths[ c ] = content_max[ c ] + 2; // padding
		}
		else
		{
			// if not auto-fit, ensure at least header length + padding
			col_widths[ c ] = ( content_max[ c ] < MIN_COL_WIDTH ? MIN_COL_WIDTH : MIN_COL_WIDTH );
		}
		if ( col_widths[ c ] < MIN_COL_WIDTH ) col_widths[ c ] = MIN_COL_WIDTH;
		total_min += col_widths[ c ];
	}

	// available area for table (minus separators between columns)
	int available = win_w - ( n - 1 ) /*sep chars*/ - 2 /*margins*/;
	if ( available < total_min )
	{
		// have to shrink columns proportionally but not below MIN_COL_WIDTH
		int shrinkable = total_min - available;
		// simple greedy: reduce columns that are > MIN_COL_WIDTH
		while ( shrinkable > 0 )
		{
			int reduced = 0;
			for ( int c = 0; c < n && shrinkable > 0; ++c )
			{
				if ( col_widths[ c ] > MIN_COL_WIDTH )
				{
					col_widths[ c ]--;
					shrinkable--;
					reduced = 1;
				}
			}
			if ( !reduced ) break;
		}
	}
	else
	{
		// we have extra space: distribute to columns that are auto_fit or to last column
		int extra = available - total_min;
		if ( extra > 0 )
		{
			// prefer giving extra to last column for nicer look
			col_widths[ n - 1 ] += extra;
		}
	}

	// ensure each col not larger than available single column (avoid overflow)
	for ( int c = 0; c < n; ++c )
	{
		if ( col_widths[ c ] < MIN_COL_WIDTH ) col_widths[ c ] = MIN_COL_WIDTH;
	}
}

static void draw_tabs( void )
{
	// top row: tabs
	move( 0 , 0 );
	clrtoeol();
	int x = 0;
	for ( int i = 0; i < ntabs; ++i )
	{
		const char * name = tabs[ i ].name;
		//int len = ( int )strlen( name ) + TAB_PADDING * 2;
		//int start = x;
		for ( int j = 0; j < TAB_PADDING; ++j )
		{
			mvaddch( 0 , x++ , ' ' );
		}
		if ( i == active_tab ) attron( A_REVERSE );
		mvprintw( 0 , x , "%s" , name );
		if ( i == active_tab ) attroff( A_REVERSE );
		x += ( int )strlen( name );
		for ( int j = 0; j < TAB_PADDING; ++j )
		{
			mvaddch( 0 , x++ , ' ' );
		}
		// simple separator
		mvaddch( 0 , x , ' ' );
		x++;
	}
	// instructions line beneath tabs
	move( 1 , 0 );
	clrtoeol();
	mvprintw( 1 , 0 , "Click tab to switch. Click column separator (|) to auto-fit column. Resize terminal to reflow." );
}

static void draw_table( Tab * t )
{
	// header row at y=2, then rows at y=3...
	int y = 2;
	move( y , 0 );
	clrtoeol();

	int n = t->ncols;
	int x = 1; // left margin
	// draw header cells
	for ( int c = 0; c < n; ++c )
	{
		// print header truncated/padded to width
		int w = col_widths[ c ];
		char buf[ 256 ];
		snprintf( buf , sizeof( buf ) , " %.*s" , w - 1 , t->col_names[ c ] );
		mvaddnstr( y , x , buf , w );
		x += w;
		if ( c < n - 1 )
		{
			mvaddch( y , x , '|' ); // separator clickable
			x += 1;
		}
	}
	// draw rows
	for ( int r = 0; r < t->nrows; ++r )
	{
		int rowy = y + 1 + r;
		if ( rowy >= win_h ) break;
		x = 1;
		for ( int c = 0; c < n; ++c )
		{
			int w = col_widths[ c ];
			const char * text = t->rows[ r ][ c ] ? t->rows[ r ][ c ] : "";
			// truncate if too long
			if ( ( int )strlen( text ) > w - 1 )
			{
				char tmp[ 256 ];
				strncpy( tmp , text , w - 4 );
				tmp[ w - 4 ] = '\0';
				strcat( tmp , "..." );
				mvaddnstr( rowy , x , tmp , w );
			}
			else
			{
				// pad right
				mvaddnstr( rowy , x , text , w );
			}
			x += w;
			if ( c < n - 1 )
			{
				mvaddch( rowy , x , '|' );
				x += 1;
			}
		}
		// clear to end of line
		int rem = win_w - x;
		if ( rem > 0 )
		{
			for ( int i = 0; i < rem; ++i ) mvaddch( rowy , x + i , ' ' );
		}
	}
}

static void redraw_all( void )
{
	erase();
	getmaxyx( stdscr , win_h , win_w );
	compute_column_widths( &tabs[ active_tab ] );
	draw_tabs();
	draw_table( &tabs[ active_tab ] );
	refresh();
}

int main( void )
{
	setlocale( LC_ALL , "" );
	setup_sample_tabs();

	// init ncurses
	initscr();
	noecho();
	cbreak();
	keypad( stdscr , TRUE );
	curs_set( 0 );
	// enable mouse
	mousemask( ALL_MOUSE_EVENTS | BUTTON1_CLICKED | BUTTON1_RELEASED , NULL );
	// Avoid mouse click timeout delaying
	mouseinterval( 0 );

	// initialize col_auto_fit to 1 (fit to content) as default
	for ( int i = 0; i < MAX_COLS; ++i ) col_auto_fit[ i ] = 1;

	// handle resize signals (ncurses also gives KEY_RESIZE)
	struct sigaction sa;
	sa.sa_handler = handle_resize;
	sigemptyset( &sa.sa_mask );
	sa.sa_flags = 0;
	sigaction( SIGWINCH , &sa , NULL );

	// initial size and layout
	getmaxyx( stdscr , win_h , win_w );
	compute_column_widths( &tabs[ active_tab ] );
	redraw_all();

	MEVENT ev;
	int ch;
	while ( 1 )
	{
		ch = getch();
		if ( ch == KEY_MOUSE )
		{
			if ( getmouse( &ev ) == OK )
			{
				int mx = ev.x , my = ev.y;
				// Tab row (y==0): switch tab if clicked inside tab names
				if ( my == 0 && ( ev.bstate & BUTTON1_CLICKED ) )
				{
					// find which tab was clicked by re-walking the tab layout
					int x = 0;
					for ( int i = 0; i < ntabs; ++i )
					{
						int name_len = ( int )strlen( tabs[ i ].name );
						int tab_len = name_len + TAB_PADDING * 2 + 1; // plus spacer
						int start = x;
						int end = x + tab_len - 1;
						if ( mx >= start && mx <= end )
						{
							active_tab = i;
							// recompute widths for new tab, keep auto_fit flags as-is
							compute_column_widths( &tabs[ active_tab ] );
							redraw_all();
							break;
						}
						x += tab_len;
					}
				}
				else if ( ( ev.bstate & BUTTON1_CLICKED ) && my == 2 )
				{
					// header row: check if clicked on separator char '|'
					// compute separator positions
					int x = 1;
					int n = tabs[ active_tab ].ncols;
					for ( int c = 0; c < n; ++c )
					{
						x += col_widths[ c ];
						if ( c < n - 1 )
						{
							// separator at position x
							if ( mx == x )
							{
								// toggle auto-fit for column c
								col_auto_fit[ c ] = !col_auto_fit[ c ];
								// if enabling auto-fit, set width to content width; if disabling, reset to MIN_COL_WIDTH
								if ( col_auto_fit[ c ] )
								{
									int content = max_content_width( &tabs[ active_tab ] , c );
									col_widths[ c ] = content + 2;
								}
								else
								{
									col_widths[ c ] = MIN_COL_WIDTH;
								}
								compute_column_widths( &tabs[ active_tab ] );
								redraw_all();
								break;
							}
							x += 1; // sep width
						}
					}
				}
			}
		}
		else if ( ch == 'q' || ch == 'Q' )
		{
			break;
		}
		else if ( ch == KEY_RESIZE )
		{
			getmaxyx( stdscr , win_h , win_w );
			compute_column_widths( &tabs[ active_tab ] );
			redraw_all();
		}
		else if ( ch == KEY_RIGHT )
		{
			active_tab = ( active_tab + 1 ) % ntabs;
			compute_column_widths( &tabs[ active_tab ] );
			redraw_all();
		}
		else if ( ch == KEY_LEFT )
		{
			active_tab = ( active_tab - 1 + ntabs ) % ntabs;
			compute_column_widths( &tabs[ active_tab ] );
			redraw_all();
		}
		else
		{
			// ignore other keys
		}
	}

	endwin();
	return 0;
}
