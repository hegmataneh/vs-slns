
//#include <ncurses.h>
//#include <signal.h>
//#include <stdbool.h>
//#include <stdlib.h>

//WINDOW *cell_win, *input_win;
//bool resize_flag = false;
//
//void handle_resize(int sig) {
//    resize_flag = true;
//}
//
//void init_windows() {
//    int maxy, maxx;
//    getmaxyx(stdscr, maxy, maxx);
//    
//    // Calculate window sizes (60% for cells, 40% for input)
//    int cell_height = maxy * 0.6;
//    int input_height = maxy - cell_height;
//    
//    // Create or replace windows
//    if (cell_win) delwin(cell_win);
//    if (input_win) delwin(input_win);
//    
//    cell_win = newwin(cell_height, maxx, 0, 0);
//    input_win = newwin(input_height, maxx, cell_height, 0);
//    
//    // Enable scrolling and keypad for input window
//    scrollok(input_win, TRUE);
//    keypad(input_win, TRUE);
//    
//    // Set box borders
//    box(cell_win, 0, 0);
//    box(input_win, 0, 0);
//    
//    // Add titles
//    mvwprintw(cell_win, 0, 2, " Cell Display ");
//    mvwprintw(input_win, 0, 2, " Input Area ");
//    
//    // Refresh windows
//    wrefresh(cell_win);
//    wrefresh(input_win);
//}

#include <stdio.h>
#include <stdbool.h>

// Function to print the size of a type
#define PRINT_SIZE(type) printf("%-20s: %2zu bytes\n", #type, sizeof(type))

typedef union
{
	char ar[ 4 ];
	int  ii;
} A;

int main()
{
	


	A aa;

	aa.ar[ 2 ]  = 12;

	aa.ii = !!aa.ii;

	printf( "sdfsdf" ) ;


	//printf( "=== Basic Data Types ===\n" );
	//PRINT_SIZE( char );
	//PRINT_SIZE( signed char );
	//PRINT_SIZE( unsigned char );
	//PRINT_SIZE( short );
	//PRINT_SIZE( unsigned short );
	//PRINT_SIZE( int );
	//PRINT_SIZE( unsigned int );
	//PRINT_SIZE( long );
	//PRINT_SIZE( unsigned long );
	//PRINT_SIZE( long long );
	//PRINT_SIZE( unsigned long long );
	//PRINT_SIZE( float );
	//PRINT_SIZE( double );
	//PRINT_SIZE( long double );
	//PRINT_SIZE( bool );
	//PRINT_SIZE( void * );
	//PRINT_SIZE( int * );
	//PRINT_SIZE( const char * );
	//PRINT_SIZE( size_t );

	//printf( "\n=== Type Combinations ===\n" );

	//

	//printf( "\nNote: Sizes may vary depending on platform and compiler.\n" );
	//printf( "Padding and alignment can affect struct sizes.\n" );

	return 0;
}

//#define MALLOC_AR( p , count ) ( p = ( ( __typeof__( *p ) * )malloc( count * sizeof( *p ) ) ) )

//int main() {
//    // Initialize curses
//    
//    //int * p = NULL;
//
//    
//
//
//
//    //int _cond=(int)( !(p = MALLOC_AR( p , 1 )) );
//
//    //initscr();
//    //cbreak();
//    //noecho();
//    //curs_set(1);
//    //
//    //// Set up resize handler
//    //signal(SIGWINCH, handle_resize);
//    //
//    //// Initial window creation
//    //init_windows();
//    //
//    //// Main loop
//    //int ch;
//    //while ((ch = wgetch(input_win))) {
//    //    if (resize_flag) {
//    //        resize_flag = false;
//    //        endwin();
//    //        refresh();
//    //        clear();
//    //        init_windows();
//    //        
//    //        // You may need to redraw your cell content here
//    //        // For example:
//    //        // redraw_cells(cell_win);
//    //        
//    //        continue;
//    //    }
//    //    
//    //    // Handle input
//    //    switch(ch) {
//    //        // Your input handling logic here
//    //        case KEY_RESIZE:
//    //            // This might not be needed if SIGWINCH is handled
//    //            break;
//    //        default:
//    //            waddch(input_win, ch);
//    //            wrefresh(input_win);
//    //            break;
//    //    }
//    //}
//    //
//    //// Clean up
//    //delwin(cell_win);
//    //delwin(input_win);
//    //endwin();
//    
//    return 0;
//}
//
