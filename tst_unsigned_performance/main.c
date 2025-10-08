#define _POSIX_C_SOURCE 199309L

//#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
//#include <errno.h>
//#include <pthread.h>
//#include <time.h>

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
#include <time.h>

int f( int a , int b )
{
	return a + b;  // signed overflow = UB, compiler must be cautious
}

unsigned int g( unsigned int a , unsigned int b )
{
	return a + b;  // unsigned overflow = defined, just wrap
}

double run_test( int no_sharing )
{
	struct timespec start , end;

	char ia = 0 , ib = 10;
	unsigned char ua = 0 , ub = 10;

	clock_gettime( CLOCK_MONOTONIC , &start );
	for ( long i = 0; i < 10000000000; i++ )
	{
		switch ( no_sharing )
		{
			case 0:
			{
				ia += ib;
				break;
			}
			case 1:
			{
				ua += ub;
				break;
			}
		}
	}
	
	clock_gettime( CLOCK_MONOTONIC , &end );

	double elapsed = ( end.tv_sec - start.tv_sec ) +
		( end.tv_nsec - start.tv_nsec ) / 1e9;
	return elapsed;
}

int main()
{
	int i = MAX_PATH;

	//printf( "Running false sharing test...\n" );
	double t1 = run_test( 0 );
	printf( " %.6f sec\n" , t1 );

	//printf( "Running no-sharing (padded) test...\n" );
	double t2 = run_test( 1 );
	printf( "    %.6f sec\n" , t2 );

	


	return 0;
}