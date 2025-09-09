
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
