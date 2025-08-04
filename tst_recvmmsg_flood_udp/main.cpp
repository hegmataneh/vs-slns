#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <signal.h>
#include <fcntl.h>
#include <sched.h>
#include <sys/mman.h>
#include <time.h>
#include <pthread.h>

#define BATCH_SIZE 128
#define PACKET_SIZE 2048
#define PORT 1234
#define RX_BUF_BYTES (4 * 1024 * 1024) // 4MB receive buffer

static volatile int keep_running = 1;

void handle_sigint( int sig )
{
	( void )sig;
	keep_running = 0;
}

int set_cpu_affinity( int cpu )
{
	cpu_set_t cpus;
	CPU_ZERO( &cpus );
	CPU_SET( cpu , &cpus );
	return sched_setaffinity( 0 , sizeof( cpus ) , &cpus );
}

int _byte_counter = 0;
int _continue_counter = 0;


void * config_loader( void * )
{
	while ( 1 )
	{
		printf( "%d - %d\n" , _byte_counter , _continue_counter );
		sleep( 1 );
	}

	return NULL;
}


int main()
{
	signal( SIGINT , handle_sigint );
	signal( SIGTERM , handle_sigint );

	// Optional: pin to CPU 0 for consistency
	if ( set_cpu_affinity( 0 ) != 0 )
	{
		perror( "sched_setaffinity" );
	}

	int sock = socket( AF_INET , SOCK_DGRAM , 0 );
	if ( sock < 0 )
	{
		perror( "socket" );
		return 1;
	}

	pthread_t trd_config_loader;
	pthread_create( &trd_config_loader , NULL , config_loader , NULL );

	// Allow reuse, helpful when restarting quickly
	int opt = 1;
	if ( setsockopt( sock , SOL_SOCKET , SO_REUSEADDR , &opt , sizeof( opt ) ) != 0 )
		perror( "SO_REUSEADDR" );
#ifdef SO_REUSEPORT
	if ( setsockopt( sock , SOL_SOCKET , SO_REUSEPORT , &opt , sizeof( opt ) ) != 0 )
		perror( "SO_REUSEPORT" );
#endif

	// Enlarge the receive buffer; may require permissions to set very large.
	int iii = RX_BUF_BYTES;
	if ( setsockopt( sock , SOL_SOCKET , SO_RCVBUF , &iii , sizeof( iii ) ) != 0 )
		perror( "SO_RCVBUF" );

	// Optional: enable busy polling (requires recent kernel and root; reduces latency by spinning)
	// int busy_poll = 50; // microseconds
	// setsockopt(sock, SOL_SOCKET, SO_BUSY_POLL, &busy_poll, sizeof(busy_poll)); // may fail if unsupported

	struct sockaddr_in addr = { 0 };
	addr.sin_family = AF_INET;
	addr.sin_port = htons( PORT );
	addr.sin_addr.s_addr = INADDR_ANY;

	if ( bind( sock , ( struct sockaddr * )&addr , sizeof( addr ) ) != 0 )
	{
		perror( "bind" );
		close( sock );
		return 1;
	}

	// Pre-allocate batch structures
	struct mmsghdr msgs[ BATCH_SIZE ];
	struct iovec iovecs[ BATCH_SIZE ];
	void * buffers[ BATCH_SIZE ];

	// Allocate and lock memory to avoid page faults
	for ( int i = 0; i < BATCH_SIZE; ++i )
	{
		// aligned allocation for potential cache benefits
		if ( posix_memalign( &buffers[ i ] , 64 , PACKET_SIZE ) != 0 )
		{
			fprintf( stderr , "posix_memalign failed\n" );
			return 1;
		}
		// Touch pages so they are faulted in
		memset( buffers[ i ] , 0 , PACKET_SIZE );
		if ( mlock( buffers[ i ] , PACKET_SIZE ) != 0 )
		{
			// not fatal; warn
			perror( "mlock" );
		}

		memset( &msgs[ i ] , 0 , sizeof( msgs[ i ] ) );
		iovecs[ i ].iov_base = buffers[ i ];
		iovecs[ i ].iov_len = 1;
		msgs[ i ].msg_hdr.msg_iov = &iovecs[ i ];
		msgs[ i ].msg_hdr.msg_iovlen = 1;
		// Optionally set msg_name to capture source address:
		static struct sockaddr_in src_addr[ BATCH_SIZE ];
		msgs[ i ].msg_hdr.msg_name = &src_addr[ i ];
		msgs[ i ].msg_hdr.msg_namelen = sizeof( src_addr[ i ] );
	}

	struct timespec timeout;
	timeout.tv_sec = 0;
	timeout.tv_nsec = 10000000; // 10ms; adjust depending on latency vs throughput tradeoff

	printf( "Listening on UDP port %d, batch size %d\n" , PORT , BATCH_SIZE );

	//uint64_t total_pkts = 0;
	//struct timespec start , now;
	//clock_gettime( CLOCK_MONOTONIC , &start );

	while ( keep_running )
	{
		int ret = recvmmsg( sock , msgs , BATCH_SIZE , 0 , &timeout );
		if ( ret < 0 )
		{
			if ( errno == EINTR )
			{
				_continue_counter++;
				continue;
			}
			perror( "recvmmsg" );
			break;
		}
		if ( ret == 0 )
		{
			// timeout expired with no packets
			_continue_counter++;
			continue;
		}

		// Process received packets
		//for ( int i = 0; i < ret; ++i )
		{
			//int len = msgs[ i ].msg_len;
			// Example: do minimal work (e.g., count, maybe inspect header)
			// Actual processing should avoid expensive calls here.
			_byte_counter += ret;
			// After processing, reset msg_len if needed (kernel auto resets on next call)
		}

		// Periodically report rate
		//clock_gettime( CLOCK_MONOTONIC , &now );
		//double elapsed = ( now.tv_sec - start.tv_sec ) + ( now.tv_nsec - start.tv_nsec ) / 1e9;
		//if ( elapsed >= 1.0 )
		//{
		//	printf( "Throughput: %.2f Mpps\n" , ( double )total_pkts / elapsed / 1e6 );
		//	total_pkts = 0;
		//	start = now;
		//}
	}

	printf( "Shutting down\n" );
	close( sock );
	for ( int i = 0; i < BATCH_SIZE; ++i )
	{
		munlock( buffers[ i ] , PACKET_SIZE );
		free( buffers[ i ] );
	}

	return 0;
}
