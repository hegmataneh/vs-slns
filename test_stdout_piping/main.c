#include <unistd.h>

#include <stdio.h>
//#include <stdlib.h>

//#include <string.h>
//#include <ncurses.h>

#define BUF_SIZE 1024

int main()
{

	fileno( stdout );

	//int pipefd[ 2 ];
	//char buffer[ BUF_SIZE ];
	//fd_set readfds;

	//// ncurses init
	//initscr();
	//cbreak();
	//noecho();
	//curs_set( FALSE );
	//WINDOW * win = newwin( 10 , 50 , 0 , 0 );
	//box( win , 0 , 0 );
	//wrefresh( win );

	//// Make pipe
	//pipe( pipefd );

	//// Redirect stdout
	//fflush( stdout );
	//dup2( pipefd[ 1 ] , fileno( stdout ) );

	//// Example: print something to stdout
	//printf( "Initial statistics output\n" );
	//fflush( stdout );

	//// Main loop
	//while ( 1 )
	//{
	//	FD_ZERO( &readfds );
	//	FD_SET( pipefd[ 0 ] , &readfds );

	//	struct timeval timeout = { 1, 0 };
	//	int ready = select( pipefd[ 0 ] + 1 , &readfds , NULL , NULL , &timeout );

	//	if ( ready > 0 && FD_ISSET( pipefd[ 0 ] , &readfds ) )
	//	{
	//		memset( buffer , 0 , BUF_SIZE );
	//		int n = read( pipefd[ 0 ] , buffer , BUF_SIZE - 1 );
	//		if ( n > 0 )
	//		{
	//			werase( win );
	//			box( win , 0 , 0 );
	//			mvwprintw( win , 1 , 1 , "%s" , buffer );
	//			wrefresh( win );
	//		}
	//	}

	//	// Example: periodically write to stdout
	//	static int counter = 0;
	//	if ( counter++ % 5 == 0 )
	//	{
	//		printf( "Stat update: counter=%d\n" , counter );
	//		fflush( stdout );
	//	}

	//	usleep( 200000 ); // Slow down loop a bit
	//}

	//endwin();
	return 0;
}