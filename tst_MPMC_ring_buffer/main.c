// mpmc_ringbuffer.c

#define _POSIX_C_SOURCE 199309L

#include <stdatomic.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#include <time.h>

#include <unistd.h>

#include <arpa/inet.h>
#include <sys/socket.h>

typedef struct
{
	void * data;
	atomic_size_t seq;
} ring_slot_t;

typedef struct
{
	ring_slot_t * buffer; // Array of slots
	size_t capacity; // Total number of slots (must be power of 2)
	atomic_size_t head; // Consumer index (next to dequeue)
	atomic_size_t tail; // Producer index (next to enqueue)

	/*tmp*/ int full_count;
	/*tmp*/ int enqueue_retry_count;
	/*tmp*/ int dequeue_retry_count;

} mpmc_ring_t;

// --------------------------------------------------------
// Initialize MPMC ring buffer
// --------------------------------------------------------
mpmc_ring_t * mpmc_ring_init( size_t capacity )
{
	if ( ( capacity & ( capacity - 1 ) ) != 0 )
	{
		fprintf( stderr , "Capacity must be power of two\n" );
		return NULL;
	}

	mpmc_ring_t * rb = malloc( sizeof( mpmc_ring_t ) );
	memset( rb , 0 , sizeof( mpmc_ring_t ) );
	rb->capacity = capacity;
	rb->head = ATOMIC_VAR_INIT( 0 );// initialize consumer index
	rb->tail = ATOMIC_VAR_INIT( 0 );// initialize producer index
	// Allocate array of slots with 64-byte alignment (cache-friendly)
	rb->buffer = aligned_alloc( 64 , sizeof( ring_slot_t ) * capacity ); // cache-line align
	if ( !rb->buffer )
	{
		free( rb ); return NULL;
	}
	// Initialize each slot's sequence number to its index
	for ( size_t i = 0; i < capacity; i++ )
	{
		rb->buffer[ i ].seq = ATOMIC_VAR_INIT( i );
		rb->buffer[ i ].data = NULL; // initially empty
	}
	return rb;
}

// --------------------------------------------------------
// Enqueue (multiple producers safe)
// Return true if success, false if queue full
// --------------------------------------------------------
bool mpmc_ring_enqueue( mpmc_ring_t * rb , void * data )
{
	ring_slot_t * slot;
	size_t pos;
	for ( ;;) // retry loop in case of contention
	{
		pos = atomic_load_explicit( &rb->tail , memory_order_relaxed ); // current tail
		slot = &rb->buffer[ pos & ( rb->capacity - 1 ) ]; // wrap-around with mask

		size_t seq = atomic_load_explicit( &slot->seq , memory_order_acquire ); // Sequence number shows whether slot is free (seq == pos)
		intptr_t dif = ( intptr_t )seq - ( intptr_t )pos; // check slot status

		if ( dif == 0 ) // slot is free
		{
			if ( atomic_compare_exchange_weak_explicit( // Attempt to claim this slot atomically
				&rb->tail , &pos , pos + 1 ,
				memory_order_relaxed , memory_order_relaxed ) )
			{
				break; // successfully reserved slot
			}
		}
		else if ( dif < 0 ) // otherwise retry (another producer got it first)
		{
			rb->full_count++;
			return false; // queue full
		}
		else // else: another producer updated seq, retry
		{
			rb->enqueue_retry_count++;
			// someone else updated, retry
		}
	}
	slot->data = data; // store data in reserved slot
	// update sequence so consumers know this slot has data
	atomic_store_explicit( &slot->seq , pos + 1 , memory_order_release );
	return true;
}

// --------------------------------------------------------
// Dequeue (multiple consumers safe)
// Return true if success, false if queue empty
// --------------------------------------------------------
bool mpmc_ring_dequeue( mpmc_ring_t * rb , void ** data_out )
{
	ring_slot_t * slot;
	size_t pos;
	for ( ;;) // retry loop in case of contention
	{
		pos = atomic_load_explicit( &rb->head , memory_order_relaxed ); // current head
		slot = &rb->buffer[ pos & ( rb->capacity - 1 ) ]; // wrap-around

		size_t seq = atomic_load_explicit( &slot->seq , memory_order_acquire );
		intptr_t dif = ( intptr_t )seq - ( intptr_t )( pos + 1 ); // check if slot ready

		if ( dif == 0 ) // slot has valid data
		{
			if ( atomic_compare_exchange_weak_explicit( // Attempt to claim this slot atomically
				&rb->head , &pos , pos + 1 ,
				memory_order_relaxed , memory_order_relaxed ) )
			{
				break; // successfully reserved slot
			}
		}
		else if ( dif < 0 ) // else retry (another consumer got it)
		{
			return false; // queue empty
		}
		else // else: producer has not yet finished writing, retry
		{
			rb->dequeue_retry_count++;
			// someone else updated, retry
		}
	}
	*data_out = slot->data; // read data
	atomic_store_explicit( &slot->seq , pos + rb->capacity , memory_order_release ); // Mark slot as free for future producers
	return true; // success
}

// --------------------------------------------------------
// Destroy queue
// --------------------------------------------------------
void mpmc_ring_free( mpmc_ring_t * rb )
{
	if (!rb) return;
	free( rb->buffer ); // free slot array
	free( rb ); // free ring buffer struct
}



#define NUM_PRODUCERS 4
#define NUM_CONSUMERS 4
#define ITEMS_PER_PRODUCER 1000000
#define RING_CAPACITY 1048576

mpmc_ring_t * rb;

// atomic counters
atomic_size_t produced_count = 0;
atomic_size_t consumed_count = 0;

// bitmap to track consumed items
unsigned char * bitmap;

// helper functions to set/check bitmap
void set_bit( size_t idx )
{
	bitmap[ idx / 8 ] |= ( 1 << ( idx % 8 ) );
}
int get_bit( size_t idx )
{
	return ( bitmap[ idx / 8 ] & ( 1 << ( idx % 8 ) ) ) != 0;
}

//static inline uint64_t now_ns( void )
//{
//	struct timespec ts;
//	clock_gettime( CLOCK_MONOTONIC_RAW , &ts );
//	return ( uint64_t )ts.tv_sec * 1000000000ull + ts.tv_nsec;
//}
//
//
//#define RATE 80000  // packets per second

// Producer thread
void * producer( void * arg )
{
	int id = *( int * )arg;
	
	//struct timespec start = {};
	//static struct timespec now = {};
	//clock_gettime( CLOCK_MONOTONIC_RAW , &start );

	// interval per packet in nanoseconds
	//long long interval = 1000000000LL / RATE;

	for ( int i = 0; i < ITEMS_PER_PRODUCER; i++ )
	{
		//// expected time for this packet
		//long long target_ns = start.tv_sec * 1000000000LL + start.tv_nsec + i * interval;

		//// busy wait until target time
		//do
		//{
		//	clock_gettime( CLOCK_MONOTONIC_RAW , &now );
		//} while ( ( now.tv_sec * 1000000000LL + now.tv_nsec ) < target_ns );


		
		int * item = malloc( sizeof( int ) );
		*item = id * ITEMS_PER_PRODUCER + i;
		while ( !mpmc_ring_enqueue( rb , item ) )
		{
			// queue full, retry
		}
		atomic_fetch_add( &produced_count , 1 );
	}
	return NULL;
}

// Consumer thread
void * consumer( void * arg )
{
	( void )arg;
	void * data;
	while ( atomic_load( &consumed_count ) < NUM_PRODUCERS * ITEMS_PER_PRODUCER )
	{
		if ( mpmc_ring_dequeue( rb , &data ) )
		{
			int val = *( int * )data;
			free( data );

			// mark consumed
			if ( get_bit( val ) )
			{
				fprintf( stderr , "Error: duplicate value consumed: %d\n" , val );
			}
			set_bit( val );

			atomic_fetch_add( &consumed_count , 1 );
		}
	}
	return NULL;
}

int main()
{
	size_t total_items = NUM_PRODUCERS * ITEMS_PER_PRODUCER;

	// allocate bitmap
	bitmap = calloc( ( total_items + 7 ) / 8 , sizeof( unsigned char ) );
	if ( !bitmap )
	{
		perror( "calloc" ); return 1;
	}

	rb = mpmc_ring_init( RING_CAPACITY );

	pthread_t producers[ NUM_PRODUCERS ];
	pthread_t consumers[ NUM_CONSUMERS ];
	int ids[ NUM_PRODUCERS ];

	// start consumers first
	for ( int i = 0; i < NUM_CONSUMERS; i++ )
		pthread_create( &consumers[ i ] , NULL , consumer , NULL );

	// start producers
	for ( int i = 0; i < NUM_PRODUCERS; i++ )
	{
		ids[ i ] = i;
		pthread_create( &producers[ i ] , NULL , producer , &ids[ i ] );
	}

	// join producers
	for ( int i = 0; i < NUM_PRODUCERS; i++ )
		pthread_join( producers[ i ] , NULL );

	// join consumers
	for ( int i = 0; i < NUM_CONSUMERS; i++ )
		pthread_join( consumers[ i ] , NULL );

	printf( "full_count%d , enqueue_retry_count%d , dequeue_retry_count%d\n" , rb->full_count , rb->enqueue_retry_count , rb->dequeue_retry_count );

	printf( "Produced: %zu, Consumed: %zu\n" ,
		atomic_load( &produced_count ) ,
		atomic_load( &consumed_count ) );

	// Verify correctness
	int errors = 0;
	for ( size_t i = 0; i < total_items; i++ )
	{
		if ( !get_bit( i ) )
		{
			fprintf( stderr , "Error: item %zu not consumed!\n" , i );
			errors++;
		}
	}

	if ( errors == 0 )
	{
		printf( "All items consumed exactly once. Test passed!\n" );
	}
	else
	{
		printf( "Test failed: %d missing items\n" , errors );
	}

	mpmc_ring_free( rb );
	free( bitmap );
	return 0;
}
