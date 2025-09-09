
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <ncurses.h>

#define INPUT_MAX 64

float cpu_usage = 0.0;
float mem_usage = 0.0;
float io_rate = 0.0;
char last_command[ INPUT_MAX ] = "";

pthread_mutex_t data_lock;
WINDOW * main_win;

float random_float( float min , float max )
{
	return min + ( ( float )rand() / RAND_MAX ) * ( max - min );
}

// Centered cell printing
void print_cell( WINDOW * win , int y , int x , int width , const char * text )
{
	int len = strlen( text );
	int pad = ( width - len ) / 2;
	if ( pad < 0 ) pad = 0;
	mvwprintw( win , y , x + pad , "%s" , text );
}

// Drawing the full table
void draw_table()
{
	int cell_w = 11;
	int start_x = 2;
	int y = 1;

	// Top border
	mvwprintw( main_win , y++ , start_x , "+-----------+-----------+" );

	// Header
	wattron( main_win , COLOR_PAIR( 1 ) );
	mvwprintw( main_win , y , start_x , "|" );
	print_cell( main_win , y , start_x + 1 , cell_w , "Metric" );
	mvwprintw( main_win , y , start_x + cell_w + 1 , "|" );
	print_cell( main_win , y , start_x + cell_w + 2 , cell_w , "Value" );
	mvwprintw( main_win , y++ , start_x + 2 * cell_w + 2 , "|" );
	wattroff( main_win , COLOR_PAIR( 1 ) );

	// Header border
	mvwprintw( main_win , y++ , start_x , "+-----------+-----------+" );

	// Data rows
	wattron( main_win , COLOR_PAIR( 2 ) );
	char buf[ 32 ];

	// CPU
	mvwprintw( main_win , y , start_x , "|" );
	print_cell( main_win , y , start_x + 1 , cell_w , "CPU %" );
	snprintf( buf , sizeof( buf ) , "%.2f" , cpu_usage );
	mvwprintw( main_win , y , start_x + cell_w + 1 , "|" );
	print_cell( main_win , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( main_win , y++ , start_x + 2 * cell_w + 2 , "|" );

	// Memory
	mvwprintw( main_win , y , start_x , "|" );
	print_cell( main_win , y , start_x + 1 , cell_w , "MemoryMB" );
	snprintf( buf , sizeof( buf ) , "%.2f" , mem_usage );
	mvwprintw( main_win , y , start_x + cell_w + 1 , "|" );
	print_cell( main_win , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( main_win , y++ , start_x + 2 * cell_w + 2 , "|" );

	// IO
	mvwprintw( main_win , y , start_x , "|" );
	print_cell( main_win , y , start_x + 1 , cell_w , "IO KB/s" );
	snprintf( buf , sizeof( buf ) , "%.2f" , io_rate );
	mvwprintw( main_win , y , start_x + cell_w + 1 , "|" );
	print_cell( main_win , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( main_win , y++ , start_x + 2 * cell_w + 2 , "|" );

	wattroff( main_win , COLOR_PAIR( 2 ) );

	// Mid border
	mvwprintw( main_win , y++ , start_x , "+-----------+-----------+" );

	// Last Command Row
	wattron( main_win , COLOR_PAIR( 3 ) );
	mvwprintw( main_win , y , start_x , "|" );
	print_cell( main_win , y , start_x + 1 , cell_w , "Last Cmd" );
	mvwprintw( main_win , y , start_x + cell_w + 1 , "|" );
	print_cell( main_win , y , start_x + cell_w + 2 , cell_w , last_command );
	mvwprintw( main_win , y++ , start_x + 2 * cell_w + 2 , "|" );
	wattroff( main_win , COLOR_PAIR( 3 ) );

	// Bottom border
	mvwprintw( main_win , y++ , start_x , "+-----------+-----------+" );
}

// Stats update thread
void * stats_thread( void * arg )
{
	struct timespec ts = { 1, 0 };

	while ( 1 )
	{
		pthread_mutex_lock( &data_lock );
		cpu_usage = random_float( 10.0 , 90.0 );
		mem_usage = random_float( 500.0 , 1600.0 );
		io_rate = random_float( 5.0 , 500.0 );

		werase( main_win );
		box( main_win , 0 , 0 );
		draw_table();
		wrefresh( main_win );
		pthread_mutex_unlock( &data_lock );

		sleep( 1 );
	}
	return NULL;
}

// Input thread
void * input_thread( void * arg )
{
	while ( 1 )
	{
		pthread_mutex_lock( &data_lock );
		int y = 8; // row of "Last Cmd" entry cell
		int x = 14; // start of Value cell (roughly)
		wmove( main_win , y , x );
		wrefresh( main_win );
		pthread_mutex_unlock( &data_lock );

		echo();
		curs_set( 1 );
		pthread_mutex_lock( &data_lock );
		wgetnstr( main_win , last_command , INPUT_MAX - 1 );
		pthread_mutex_unlock( &data_lock );
		noecho();
		curs_set( 0 );
	}
	return NULL;
}

int main()
{
	initscr();
	start_color();
	cbreak(); 
	noecho();
	curs_set( 0 );

	init_pair( 1 , COLOR_WHITE , COLOR_BLUE );   // Header
	init_pair( 2 , COLOR_GREEN , COLOR_BLACK );  // Data
	init_pair( 3 , COLOR_YELLOW , COLOR_BLACK ); // Last Command

	srand( time( NULL ) );
	pthread_mutex_init( &data_lock , NULL );

	int height , width;
	getmaxyx( stdscr , height , width );
	main_win = newwin( height , width , 0 , 0 );

	pthread_t tid_stats , tid_input;
	pthread_create( &tid_stats , NULL , stats_thread , NULL );
	pthread_create( &tid_input , NULL , input_thread , NULL );

	pthread_join( tid_stats , NULL );
	pthread_join( tid_input , NULL );

	delwin( main_win );
	endwin();
	pthread_mutex_destroy( &data_lock );
	return 0;
}
