#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <time.h>
#include <stdint.h>

static inline uint64_t now_ns( void )
{
	struct timespec ts;
	clock_gettime( CLOCK_MONOTONIC_RAW , &ts );
	return ( uint64_t )ts.tv_sec * 1000000000ull + ts.tv_nsec;
}

int main( void )
{
	const long long N = 10000000000LL;  // 1 billion iterations
	volatile long long dummy = 0;      // volatile prevents optimization

	uint64_t start = now_ns();

	for ( long long i = 0; i < N; i++ )
	{
		dummy += i;  // something trivial
	}

	uint64_t end = now_ns();
	uint64_t diff = end - start;

	printf( "Loop of %lld iterations took %llu ns (%.3f seconds)\n" ,
		N , ( unsigned long long )diff , diff / 1e9 );

	return 0;
}