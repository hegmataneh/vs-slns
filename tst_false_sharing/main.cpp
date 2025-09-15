// false_sharing_demo.c
// Demonstrates false sharing vs no sharing performance difference

#define v1
#define v2
#define v3
#define v4
#define v5
#define v6
#define v7
//#define v8


#ifndef v8

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
#include <time.h>

#define NUM_THREADS 4
#define ITERATIONS  1000000000   // Adjust to see clearer difference

#define CACHE_LINE_SIZE 64

typedef struct
{
	int val1;       // 4 bytes, but may cause padding/misalignment
	long val2;    // 8 bytes
} unaligned_t;

// Aligned struct example
typedef struct
{
	int val1;       // 4 bytes
	char padding1[ 64 - sizeof( int ) ]; // 64B typical cache line size

	long val2;    // 8 bytes
	char padding2[ 64 - sizeof( long ) ]; // 64B typical cache line size

} __attribute__( ( aligned( CACHE_LINE_SIZE ) ) ) aligned_t;

unaligned_t unaligned_shared_data[ NUM_THREADS ];
aligned_t aligned_padded_data[ NUM_THREADS ];

void * worker_false_sharing( void * arg )
{
	long id = ( long )arg;
	for ( long i = 0; i < ITERATIONS; i++ )
	{
		unaligned_shared_data[ id ].val1 += 1;
		unaligned_shared_data[ id ].val2 += 1;
	}
	return NULL;
}

void * worker_no_sharing( void * arg )
{
	long id = ( long )arg;
	for ( long i = 0; i < ITERATIONS; i++ )
	{
		aligned_padded_data[ id ].val1 += 1;
		aligned_padded_data[ id ].val2 += 1;
	}
	return NULL;
}

double run_test( int no_sharing )
{
	pthread_t threads[ NUM_THREADS ];
	struct timespec start , end;

	clock_gettime( CLOCK_MONOTONIC , &start );
	for ( long i = 0; i < NUM_THREADS; i++ )
	{
		switch ( no_sharing )
		{
			case 0:
			{
				pthread_create( &threads[ i ] , NULL , worker_false_sharing , ( void * )i );
				break;
			}
			case 1:
			{
				pthread_create( &threads[ i ] , NULL , worker_no_sharing , ( void * )i );
				break;
			}
		}
	}
	for ( int i = 0; i < NUM_THREADS; i++ )
	{
		pthread_join( threads[ i ] , NULL );
	}
	clock_gettime( CLOCK_MONOTONIC , &end );

	double elapsed = ( end.tv_sec - start.tv_sec ) +
		( end.tv_nsec - start.tv_nsec ) / 1e9;
	return elapsed;
}

int main()
{
	//printf( "Running false sharing test...\n" );
	double t1 = run_test( 0 );
	printf( "False sharing time: %.6f sec\n" , t1 );

	//printf( "Running no-sharing (padded) test...\n" );
	double t2 = run_test( 1 );
	printf( "No sharing time:    %.6f sec\n" , t2 );

	printf( "\nSpeedup: %.2fx faster without false sharing\n" , t1 / t2 );



	return 0;
}

#endif


#ifndef v7

// cache_alignment_false_sharing.c
// Benchmark False Sharing vs. Cache Alignment (No Data Locality test)
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

#define NUM_THREADS 4
#define ITERATIONS  100000000

// Will be set at runtime based on system cache line size
size_t CACHE_LINE_SIZE = 64;

// Structures for false sharing vs cache-aligned
typedef struct
{
	long long value;
} shared_t;

typedef struct
{
	long long value;
	char padding[ 128 ]; // allocate more than enough, real padding handled by alignment
} padded_t;

shared_t * shared_data;
padded_t * padded_data;

// ==================== THREAD FUNCTIONS ====================
void * worker_false_sharing( void * arg )
{
	long id = ( long )arg;
	for ( long i = 0; i < ITERATIONS; i++ )
	{
		shared_data[ id ].value += 1;
	}
	return NULL;
}

void * worker_no_sharing( void * arg )
{
	long id = ( long )arg;
	for ( long i = 0; i < ITERATIONS; i++ )
	{
		padded_data[ id ].value += 1;
	}
	return NULL;
}

// ==================== BENCHMARK RUNNER ====================
double run_false_sharing( int no_sharing )
{
	pthread_t threads[ NUM_THREADS ];
	struct timespec start , end;

	clock_gettime( CLOCK_MONOTONIC , &start );
	for ( long i = 0; i < NUM_THREADS; i++ )
	{
		if ( no_sharing )
			pthread_create( &threads[ i ] , NULL , worker_no_sharing , ( void * )i );
		else
			pthread_create( &threads[ i ] , NULL , worker_false_sharing , ( void * )i );
	}
	for ( int i = 0; i < NUM_THREADS; i++ )
	{
		pthread_join( threads[ i ] , NULL );
	}
	clock_gettime( CLOCK_MONOTONIC , &end );

	return ( end.tv_sec - start.tv_sec ) +
		( end.tv_nsec - start.tv_nsec ) / 1e9;
}

// ==================== MAIN ====================
int main()
{
	// Detect cache line size (fallback to 64 if unknown)
	long line_size = sysconf( _SC_LEVEL1_DCACHE_LINESIZE );
	if ( line_size > 0 ) CACHE_LINE_SIZE = ( size_t )line_size;

	printf( "Detected L1 cache line size: %zu bytes\n\n" , CACHE_LINE_SIZE );

	// Allocate memory properly aligned
	posix_memalign( ( void ** )&shared_data , CACHE_LINE_SIZE , sizeof( shared_t ) * NUM_THREADS );
	posix_memalign( ( void ** )&padded_data , CACHE_LINE_SIZE , sizeof( padded_t ) * NUM_THREADS );

	// Initialize data
	for ( int i = 0; i < NUM_THREADS; i++ )
	{
		shared_data[ i ].value = 0;
		padded_data[ i ].value = 0;
	}

	printf( "=== FALSE SHARING vs CACHE ALIGNMENT ===\n" );

	double t1 = run_false_sharing( 0 );
	printf( "False sharing time: %.6f sec\n" , t1 );

	double t2 = run_false_sharing( 1 );
	printf( "Cache-aligned time: %.6f sec\n" , t2 );

	printf( "\nSpeedup from avoiding false sharing: %.2fx\n" , t1 / t2 );

	free( shared_data );
	free( padded_data );
	return 0;
}


#endif

#ifndef v6

#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#define SIZE 100000000 // Large array to exceed cache
#define ITERATIONS 10  // Repeat to amplify timing differences

// Global array for testing
int * array;

// Initialize array
void init_array()
{
	array = ( int * )malloc( SIZE * sizeof( int ) );
	for ( int i = 0; i < SIZE; i++ )
	{
		array[ i ] = i;
	}
}

// Sequential access (good data locality)
long long sum_sequential()
{
	long long sum = 0;
	for ( int iter = 0; iter < ITERATIONS; iter++ )
	{
		for ( int i = 0; i < SIZE; i++ )
		{
			sum += array[ i ];
		}
	}
	return sum;
}

// Strided access (poor data locality, every 16th element)
long long sum_strided()
{
	long long sum = 0;
	for ( int iter = 0; iter < ITERATIONS; iter++ )
	{
		for ( int i = 0; i < SIZE; i += 16 )
		{ // Stride of 16
			sum += array[ i ];
		}
	}
	return sum;
}

int main()
{
	struct timespec start , end;
	double time_seq , time_strided;
	long long sum;

	// Initialize array
	init_array();

	// Test sequential access (good locality)
	clock_gettime( CLOCK_MONOTONIC , &start );
	sum = sum_sequential();
	clock_gettime( CLOCK_MONOTONIC , &end );
	time_seq = ( end.tv_sec - start.tv_sec ) + ( end.tv_nsec - start.tv_nsec ) / 1e9;
	printf( "Sequential access (good locality) time: %.4f seconds, sum: %lld\n" , time_seq , sum );

	// Test strided access (poor locality)
	clock_gettime( CLOCK_MONOTONIC , &start );
	sum = sum_strided();
	clock_gettime( CLOCK_MONOTONIC , &end );
	time_strided = ( end.tv_sec - start.tv_sec ) + ( end.tv_nsec - start.tv_nsec ) / 1e9;
	printf( "Strided access (poor locality) time: %.4f seconds, sum: %lld\n" , time_strided , sum );

	// Calculate percentage improvement
	double improvement = ( ( time_strided - time_seq ) / time_strided ) * 100.0;
	printf( "Performance improvement with good data locality: %.2f%%\n" , improvement );

	// Clean up
	free( array );
	return 0;
}

#endif


#ifndef v5

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <string.h>

#define SIZE (64 * 1024 * 1024)  // 64 MB array
#define STRIDE 64                // Stride for bad locality (bytes)
#define CACHE_LINE 64            // Typical cache line size

// High resolution timer
static inline double now_sec( void )
{
	struct timespec ts;
	clock_gettime( CLOCK_MONOTONIC , &ts );
	return ts.tv_sec + ts.tv_nsec * 1e-9;
}

int main( void )
{
	// Allocate extra space for manual alignment
	void * raw = malloc( SIZE + CACHE_LINE );
	if ( !raw ) return 1;

	// Align memory manually
	uintptr_t addr = ( uintptr_t )raw;
	uintptr_t aligned = ( addr + ( CACHE_LINE - 1 ) ) & ~( CACHE_LINE - 1 );
	uint8_t * aligned_buf = ( uint8_t * )aligned;

	// Unaligned buffer is intentionally shifted
	uint8_t * unaligned_buf = aligned_buf + 1;

	volatile uint64_t sum = 0;
	double start , end;

	// -------------------------------
	// Test 1: Aligned sequential access
	// -------------------------------
	memset( aligned_buf , 1 , SIZE );
	start = now_sec();
	for ( size_t i = 0; i < SIZE; i++ )
	{
		sum += aligned_buf[ i ];
	}
	end = now_sec();
	double aligned_time = end - start;
	printf( "Aligned Sequential: %.6f sec\n" , aligned_time );

	// -------------------------------
	// Test 2: Unaligned sequential access
	// -------------------------------
	memset( unaligned_buf , 1 , SIZE );
	start = now_sec();
	for ( size_t i = 0; i < SIZE; i++ )
	{
		sum += unaligned_buf[ i ];
	}
	end = now_sec();
	double unaligned_time = end - start;
	printf( "Unaligned Sequential: %.6f sec\n" , unaligned_time );

	// -------------------------------
	// Test 3: Poor data locality (strided)
	// -------------------------------
	memset( aligned_buf , 1 , SIZE );
	start = now_sec();
	for ( size_t i = 0; i < SIZE; i += STRIDE )
	{
		sum += aligned_buf[ i ];
	}
	end = now_sec();
	double strided_time = end - start;
	printf( "Strided Access: %.6f sec\n" , strided_time );

	// -------------------------------
	// Show percentage improvement
	// -------------------------------
	double alignment_gain = ( ( unaligned_time - aligned_time ) / unaligned_time ) * 100.0;
	printf( "Cache alignment speedup: %.2f%%\n" , alignment_gain );

	free( raw );
	return 0;
}


#endif


#ifndef v4

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h> // For fixed-size types

// Define matrix size (adjust for your system; larger for more pronounced effects)
#define ROWS 4096
#define COLS 4096
#define ITERATIONS 10 // Repeat traversals to amplify timing differences

// Cache line size (typically 64 bytes)
#define CACHE_LINE_SIZE 64

// Unaligned struct example
typedef struct
{
	char a;      // 1 byte
	int b;       // 4 bytes, but may cause padding/misalignment
	double c;    // 8 bytes
} unaligned_t;

// Aligned struct example
typedef struct
{
	char a;      // 1 byte
	char padding1[ 3 ]; // Pad to align next to 4-byte boundary
	int b;       // 4 bytes
	char padding2[ 4 ]; // Pad to align next to 8-byte boundary (for double)
	double c;    // 8 bytes
} __attribute__( ( aligned( CACHE_LINE_SIZE ) ) ) aligned_t;

// Global matrices for locality test (use int for simplicity)
int matrix[ ROWS ][ COLS ];

// Function to initialize matrix
void init_matrix()
{
	for ( int i = 0; i < ROWS; i++ )
	{
		for ( int j = 0; j < COLS; j++ )
		{
			matrix[ i ][ j ] = i + j;
		}
	}
}

// Benchmark row-major traversal (good spatial locality in C)
long long sum_row_major()
{
	long long sum = 0;
	for ( int iter = 0; iter < ITERATIONS; iter++ )
	{
		for ( int i = 0; i < ROWS; i++ )
		{
			for ( int j = 0; j < COLS; j++ )
			{
				sum += matrix[ i ][ j ];
			}
		}
	}
	return sum;
}

// Benchmark column-major traversal (poor spatial locality)
long long sum_col_major()
{
	long long sum = 0;
	for ( int iter = 0; iter < ITERATIONS; iter++ )
	{
		for ( int j = 0; j < COLS; j++ )
		{
			for ( int i = 0; i < ROWS; i++ )
			{
				sum += matrix[ i ][ j ];
			}
		}
	}
	return sum;
}

// Benchmark unaligned struct access
double access_unaligned( unaligned_t * arr , size_t size )
{
	double total = 0.0;
	for ( int iter = 0; iter < ITERATIONS; iter++ )
	{
		for ( size_t i = 0; i < size; i++ )
		{
			arr[ i ].b += 1;
			total += arr[ i ].c;
		}
	}
	return total;
}

// Benchmark aligned struct access
double access_aligned( aligned_t * arr , size_t size )
{
	double total = 0.0;
	for ( int iter = 0; iter < ITERATIONS; iter++ )
	{
		for ( size_t i = 0; i < size; i++ )
		{
			arr[ i ].b += 1;
			total += arr[ i ].c;
		}
	}
	return total;
}

int main()
{
	struct timespec start , end;
	double time_row , time_col , time_unaligned , time_aligned;
	long long sum;
	double total;

	// Initialize matrix for locality test
	init_matrix();

	// Test data locality: row-major (good)
	clock_gettime( CLOCK_MONOTONIC , &start );
	sum = sum_row_major();
	clock_gettime( CLOCK_MONOTONIC , &end );
	time_row = ( end.tv_sec - start.tv_sec ) + ( end.tv_nsec - start.tv_nsec ) / 1e9;
	printf( "Row-major (good locality) time: %.4f seconds, sum: %lld\n" , time_row , sum );

	// Test data locality: column-major (bad)
	clock_gettime( CLOCK_MONOTONIC , &start );
	sum = sum_col_major();
	clock_gettime( CLOCK_MONOTONIC , &end );
	time_col = ( end.tv_sec - start.tv_sec ) + ( end.tv_nsec - start.tv_nsec ) / 1e9;
	printf( "Column-major (poor locality) time: %.4f seconds, sum: %lld\n" , time_col , sum );

	// Calculate percentage improvement for locality
	double locality_improvement = ( ( time_col - time_row ) / time_col ) * 100.0;
	printf( "Performance improvement with good data locality: %.2f%%\n" , locality_improvement );

	// For alignment test: allocate arrays
	size_t arr_size = 1000000; // 1 million elements
	unaligned_t * unaligned_arr = ( unaligned_t * )malloc( arr_size * sizeof( unaligned_t ) );
	aligned_t * aligned_arr = ( aligned_t * )malloc( arr_size * sizeof( aligned_t ) );

	// Initialize arrays
	for ( size_t i = 0; i < arr_size; i++ )
	{
		unaligned_arr[ i ].a = 'x';
		unaligned_arr[ i ].b = 0;
		unaligned_arr[ i ].c = 1.5;
		aligned_arr[ i ].a = 'x';
		aligned_arr[ i ].b = 0;
		aligned_arr[ i ].c = 1.5;
	}

	// Test unaligned access
	clock_gettime( CLOCK_MONOTONIC , &start );
	total = access_unaligned( unaligned_arr , arr_size );
	clock_gettime( CLOCK_MONOTONIC , &end );
	time_unaligned = ( end.tv_sec - start.tv_sec ) + ( end.tv_nsec - start.tv_nsec ) / 1e9;
	printf( "Unaligned struct access time: %.4f seconds, total: %.2f\n" , time_unaligned , total );

	// Test aligned access
	clock_gettime( CLOCK_MONOTONIC , &start );
	total = access_aligned( aligned_arr , arr_size );
	clock_gettime( CLOCK_MONOTONIC , &end );
	time_aligned = ( end.tv_sec - start.tv_sec ) + ( end.tv_nsec - start.tv_nsec ) / 1e9;
	printf( "Aligned struct access time: %.4f seconds, total: %.2f\n" , time_aligned , total );

	// Calculate percentage improvement for alignment
	double alignment_improvement = ( ( time_unaligned - time_aligned ) / time_unaligned ) * 100.0;
	printf( "Performance improvement with cache alignment: %.2f%%\n" , alignment_improvement );

	// Clean up
	free( unaligned_arr );
	free( aligned_arr );

	return 0;
}

#endif


#ifndef v3

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>

// Define the number of threads and iterations
#define NUM_THREADS 4
#define ITERATIONS 100000000

// Cache line size (typically 64 bytes on x86)
#define CACHE_LINE_SIZE 64

// Structure for false sharing: counters are adjacent
typedef struct
{
	long counter;
} unpadded_t;

// Structure to avoid false sharing: pad to cache line size
typedef struct
{
	long counter;
	char padding[ CACHE_LINE_SIZE - sizeof( long ) ];
} padded_t;

// Global arrays for both versions
unpadded_t unpadded_counters[ NUM_THREADS ];
padded_t padded_counters[ NUM_THREADS ];

// Thread function for incrementing counter
void * increment_counter( void * arg )
{
	int id = *( int * )arg;
	for ( long i = 0; i < ITERATIONS; i++ )
	{
		unpadded_counters[ id ].counter++;
	}
	return NULL;
}

// Thread function for padded version
void * increment_padded_counter( void * arg )
{
	int id = *( int * )arg;
	for ( long i = 0; i < ITERATIONS; i++ )
	{
		padded_counters[ id ].counter++;
	}
	return NULL;
}

int main()
{
	pthread_t threads[ NUM_THREADS ];
	int thread_ids[ NUM_THREADS ];
	struct timespec start , end;
	double elapsed;

	// Initialize counters
	for ( int i = 0; i < NUM_THREADS; i++ )
	{
		unpadded_counters[ i ].counter = 0;
		padded_counters[ i ].counter = 0;
		thread_ids[ i ] = i;
	}

	// Test with false sharing
	clock_gettime( CLOCK_MONOTONIC , &start );
	for ( int i = 0; i < NUM_THREADS; i++ )
	{
		pthread_create( &threads[ i ] , NULL , increment_counter , &thread_ids[ i ] );
	}
	for ( int i = 0; i < NUM_THREADS; i++ )
	{
		pthread_join( threads[ i ] , NULL );
	}
	clock_gettime( CLOCK_MONOTONIC , &end );
	elapsed = ( end.tv_sec - start.tv_sec ) + ( end.tv_nsec - start.tv_nsec ) / 1e9;
	printf( "False sharing time: %.2f seconds\n" , elapsed );

	// Verify counters (optional, for correctness)
	long total = 0;
	for ( int i = 0; i < NUM_THREADS; i++ )
	{
		total += unpadded_counters[ i ].counter;
	}
	printf( "Total (false sharing): %ld\n" , total );

	// Test without false sharing (padded)
	clock_gettime( CLOCK_MONOTONIC , &start );
	for ( int i = 0; i < NUM_THREADS; i++ )
	{
		pthread_create( &threads[ i ] , NULL , increment_padded_counter , &thread_ids[ i ] );
	}
	for ( int i = 0; i < NUM_THREADS; i++ )
	{
		pthread_join( threads[ i ] , NULL );
	}
	clock_gettime( CLOCK_MONOTONIC , &end );
	elapsed = ( end.tv_sec - start.tv_sec ) + ( end.tv_nsec - start.tv_nsec ) / 1e9;
	printf( "No false sharing time: %.2f seconds\n" , elapsed );

	// Verify counters
	total = 0;
	for ( int i = 0; i < NUM_THREADS; i++ )
	{
		total += padded_counters[ i ].counter;
	}
	printf( "Total (padded): %ld\n" , total );

	return 0;
}

#endif


#ifndef v2

#include <pthread.h>
#include <stdio.h>
#include <stdatomic.h>

#define ITER 100000000

typedef struct
{
	int counter;
	// no padding → risk of false sharing
} Shared;

Shared shared[ 2 ];

void * worker( void * arg )
{
	int i = ( int )( size_t )arg;
	for ( int j = 0; j < ITER; j++ )
	{
		shared[ i ].counter++;
	}
	return NULL;
}

int main()
{
	pthread_t t1 , t2;
	pthread_create( &t1 , NULL , worker , ( void * )0 );
	pthread_create( &t2 , NULL , worker , ( void * )1 );

	pthread_join( t1 , NULL );
	pthread_join( t2 , NULL );

	printf( "counters = %d %d\n" , shared[ 0 ].counter , shared[ 1 ].counter );
}


#endif

#ifndef v1

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
#include <time.h>

#define NUM_THREADS 4
#define ITERATIONS  1000000000   // Adjust to see clearer difference

// Structure WITHOUT padding (likely false sharing)
typedef struct
{
	long value;
} shared_t;

// Structure WITH padding (no false sharing)
typedef struct
{
	long value;
	char padding[ 64 - sizeof( long ) ]; // 64B typical cache line size
} padded_t;

#define CACHE_LINE_SIZE 64

typedef struct
{
	char a;      // 1 byte
	int b;       // 4 bytes, but may cause padding/misalignment
	long value;    // 8 bytes
} unaligned_t;

// Aligned struct example
typedef struct
{
	char a;      // 1 byte
	char padding1[ 3 ]; // Pad to align next to 4-byte boundary
	int b;       // 4 bytes
	char padding2[ 4 ]; // Pad to align next to 8-byte boundary (for double)
	long value;    // 8 bytes
} __attribute__( ( aligned( CACHE_LINE_SIZE ) ) ) aligned_t;




typedef struct
{
	char a;      // 1 byte
	int b;       // 4 bytes, but may cause padding/misalignment
	long value;    // 8 bytes
} unaligned_shared_t;

// Aligned struct example
typedef struct
{
	char a;      // 1 byte
	char padding1[ 3 ]; // Pad to align next to 4-byte boundary
	int b;       // 4 bytes
	char padding2[ 4 ]; // Pad to align next to 8-byte boundary (for double)
	long value;    // 8 bytes

	char padding[ 64 - sizeof( char ) - sizeof( char[3] ) - sizeof( int ) - sizeof( char[4] ) - sizeof( long ) ]; // 64B typical cache line size

} __attribute__(( aligned(CACHE_LINE_SIZE) )) aligned_padded_t;




shared_t shared_data[ NUM_THREADS ];
padded_t padded_data[ NUM_THREADS ];

shared_t unaligned_data[ NUM_THREADS ];
padded_t aligned_data[ NUM_THREADS ];


unaligned_shared_t unaligned_shared[ NUM_THREADS ];
aligned_padded_t aligned_no_sharing[ NUM_THREADS ];

void * worker_false_sharing( void * arg )
{
	long id = ( long )arg;
	for ( long i = 0; i < ITERATIONS; i++ )
	{
		shared_data[ id ].value += 1;
	}
	return NULL;
}

void * worker_no_sharing( void * arg )
{
	long id = ( long )arg;
	for ( long i = 0; i < ITERATIONS; i++ )
	{
		padded_data[ id ].value += 1;
	}
	return NULL;
}

void * worker_unaligned( void * arg )
{
	long id = ( long )arg;
	for ( long i = 0; i < ITERATIONS; i++ )
	{
		unaligned_data[ id ].value += 1;
	}
	return NULL;
}

void * worker_aligned( void * arg )
{
	long id = ( long )arg;
	for ( long i = 0; i < ITERATIONS; i++ )
	{
		aligned_data[ id ].value += 1;
	}
	return NULL;
}



void * worker_unaligned_shared( void * arg )
{
	long id = ( long )arg;
	for ( long i = 0; i < ITERATIONS; i++ )
	{
		unaligned_shared[ id ].value += 1;
	}
	return NULL;
}

void * worker_aligned_no_sharing( void * arg )
{
	long id = ( long )arg;
	for ( long i = 0; i < ITERATIONS; i++ )
	{
		aligned_no_sharing[ id ].value += 1;
	}
	return NULL;
}



double run_test( int no_sharing )
{
	pthread_t threads[ NUM_THREADS ];
	struct timespec start , end;

	clock_gettime( CLOCK_MONOTONIC , &start );
	for ( long i = 0; i < NUM_THREADS; i++ )
	{
		switch ( no_sharing )
		{
			case 0 :
			{
				pthread_create( &threads[ i ] , NULL , worker_false_sharing , ( void * )i );
				break;
			}
			case 1:
			{
				pthread_create( &threads[ i ] , NULL , worker_no_sharing , ( void * )i );
				break;
			}
			case 2:
			{
				pthread_create( &threads[ i ] , NULL , worker_unaligned , ( void * )i );
				break;
			}
			case 3:
			{
				pthread_create( &threads[ i ] , NULL , worker_aligned , ( void * )i );
				break;
			}
			case 4:
			{
				pthread_create( &threads[ i ] , NULL , worker_unaligned_shared , ( void * )i );
				break;
			}
			case 5:
			{
				pthread_create( &threads[ i ] , NULL , worker_aligned_no_sharing , ( void * )i );
				break;
			}
		}
	}
	for ( int i = 0; i < NUM_THREADS; i++ )
	{
		pthread_join( threads[ i ] , NULL );
	}
	clock_gettime( CLOCK_MONOTONIC , &end );

	double elapsed = ( end.tv_sec - start.tv_sec ) +
		( end.tv_nsec - start.tv_nsec ) / 1e9;
	return elapsed;
}

int main()
{
	//printf( "Running false sharing test...\n" );
	double t1 = run_test( 0 );
	printf( "False sharing time: %.6f sec\n" , t1 );

	//printf( "Running no-sharing (padded) test...\n" );
	double t2 = run_test( 1 );
	printf( "No sharing time:    %.6f sec\n" , t2 );


	//printf( "Running unaligned test...\n" );
	double t3 = run_test( 2 );
	printf( "unaligned time:    %.6f sec\n" , t3 );

	//printf( "Running aligned test...\n" );
	double t4 = run_test( 3 );
	printf( "aligned time:    %.6f sec\n" , t4 );


	//printf( "Running unaligned_shared test...\n" );
	double t5 = run_test( 4 );
	printf( "unaligned_shared time:    %.6f sec\n" , t5 );

	//printf( "Running aligned_no_sharing test...\n" );
	double t6 = run_test( 5 );
	printf( "aligned_no_sharing time:    %.6f sec\n" , t6 );


	printf( "\nSpeedup: %.2fx faster without false sharing\n" , t1 / t2 );
	printf( "\nSpeedup: %.2fx faster without false sharing\n" , t3 / t4 );
	printf( "\nSpeedup: %.2fx faster without false sharing\n" , t5 / t6 );


	return 0;
}

#endif
