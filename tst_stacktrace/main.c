//#define TEST1
#define TEST2
#define TEST3

#ifndef TEST3

#include <stdio.h>
#include <stdlib.h>
#include <backtrace.h>

struct last_three
{
	const char * function[ 3 ];
	int count;
};

static int callback( void * data , uintptr_t pc , const char * filename , int lineno , const char * function )
{
	struct last_three * lt = ( struct last_three * )data;

	if ( lt->count < 3 )
	{
		lt->function[ lt->count ] = function ? function : "??";
		lt->count++;
	}
	else
	{
		// shift older frames out
		lt->function[ 0 ] = lt->function[ 1 ];
		lt->function[ 1 ] = lt->function[ 2 ];
		lt->function[ 2 ] = function ? function : "??";
	}
	return 0; // continue backtrace
}

static void error_callback( void * data , const char * msg , int errnum )
{
	fprintf( stderr , "Error: %s\n" , msg );
}

void print_last_three()
{
	struct backtrace_state * state = backtrace_create_state( NULL , 0 , error_callback , NULL );
	struct last_three lt = { .count = 0 };

	backtrace_full( state , 0 , callback , error_callback , &lt );

	printf( "Last 3 functions in call stack:\n" );
	for ( int i = 0; i < lt.count; i++ )
	{
		printf( "%s\n" , lt.function[ i ] );
	}
}

void func3()
{
	print_last_three();
}
void func2()
{
	func3();
}
void func1()
{
	func2();
}

int main()
{
	func1();
	return 0;
}


#endif

#ifndef TEST2

#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>

void print_last_three()
{
	void * buffer[ 10 ];  // capture up to 10 frames
	int nptrs = backtrace( buffer , 10 );

	printf( "Total frames captured: %d\n" , nptrs );

	int start = nptrs - 3; // last 3 frames
	if ( start < 0 ) start = 0;

	printf( "Last 3 frames:\n" );
	char ** symbols = backtrace_symbols( buffer , nptrs );
	if ( symbols == NULL )
	{
		perror( "backtrace_symbols" );
		exit( EXIT_FAILURE );
	}

	for ( int i = start; i < nptrs; i++ )
	{
		printf( "%s\n" , symbols[ i ] );
	}

	free( symbols );
}

void func3()
{
	print_last_three();
}
void func2()
{
	func3();
}
void func1()
{
	func2();
}

int main()
{
	func1();
	return 0;
}

#endif

#ifndef TEST1

#include <stdio.h>
#include <stdlib.h>
#include <backtrace.h>

// Callback when we successfully get a frame
static int full_callback( void * data , uintptr_t pc ,
	const char * filename , int lineno ,
	const char * function )
{
	( void )data;
	printf( "  %s at %s:%d (pc=0x%lx)\n" ,
		function ? function : "??" ,
		filename ? filename : "??" ,
		lineno ,
		( unsigned long )pc );
	return 0; // continue
}

// Callback for errors
static void error_callback( void * data , const char * msg , int errnum )
{
	( void )data;
	fprintf( stderr , "libbacktrace error: %s (%d)\n" , msg , errnum );
}

// Your function to print a stack trace
void print_stacktrace()
{
	struct backtrace_state * state =
		backtrace_create_state( NULL , /* filename = current executable */
			1 ,    /* threaded = yes */
			error_callback ,
			NULL );

	backtrace_full( state , 0 , full_callback , error_callback , NULL );
}

// Dummy call chain
void deep3()
{
	print_stacktrace();
}
void deep2()
{
	deep3();
}
void deep1()
{
	deep2();
}

int main()
{
	deep1();
	return 0;
}

#endif
