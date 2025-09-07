#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sched.h>
#include <numa.h>
#include <numaif.h>

#define SIZE (100*1024*1024) // 100 MB

// pin current thread to a specific CPU
void pin_to_cpu( int cpu )
{
	cpu_set_t set;
	CPU_ZERO( &set );
	CPU_SET( cpu , &set );
	if ( sched_setaffinity( 0 , sizeof( set ) , &set ) != 0 )
	{
		perror( "sched_setaffinity" );
		exit( 1 );
	}
}

int main( int argc , char ** argv )
{
	if ( argc < 2 )
	{
		printf( "Usage: %s <cpu_id>\n" , argv[ 0 ] );
		return 1;
	}

	int cpu = atoi( argv[ 1 ] );
	pin_to_cpu( cpu );

	// allocate memory (malloc = virtual only)
	char * buf = malloc( SIZE );
	if ( !buf )
	{
		perror( "malloc" );
		return 1;
	}

	// touch memory (forces real allocation)
	for ( size_t i = 0; i < SIZE; i += 4096 )
	{
		buf[ i ] = 1;
	}

	printf( "Allocated and touched %d MB on CPU %d\n" , SIZE / ( 1024 * 1024 ) , cpu );

	// keep running so we can inspect with numastat/pagemap
	sleep( 30 );
	return 0;
}
