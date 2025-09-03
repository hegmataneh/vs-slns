#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <time.h>
#include <stdint.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/socket.h>

#define DEST_IP "192.168.100.60"
#define DEST_PORT 1234
#define PACKET_COUNT 10000000
#define RATE 30000  // packets per second

#define BUF_PAYLOAD 1470

static inline uint64_t now_ns( void )
{
	struct timespec ts;
	clock_gettime( CLOCK_MONOTONIC_RAW , &ts );
	return ( uint64_t )ts.tv_sec * 1000000000ull + ts.tv_nsec;
}

int main()
{
	int sock;
	struct sockaddr_in addr;
	char buf[BUF_PAYLOAD];

	sock = socket( AF_INET , SOCK_DGRAM , 0 );

	memset( &addr , 0 , sizeof( addr ) );
	addr.sin_family = AF_INET;
	addr.sin_port = htons( DEST_PORT );
	inet_pton( AF_INET , DEST_IP , &addr.sin_addr );

	// interval per packet in nanoseconds
	long long interval = 1000000000LL / RATE;

	struct timespec start , now;
	clock_gettime( CLOCK_MONOTONIC_RAW , &start );

	uint64_t startttt = now_ns();

	for ( long long i = 0; i < PACKET_COUNT; i++ )
	{
		// expected time for this packet
		long long target_ns = start.tv_sec * 1000000000LL + start.tv_nsec + i * interval;

		// busy wait until target time
		do
		{
			clock_gettime( CLOCK_MONOTONIC_RAW , &now );
		} while ( ( now.tv_sec * 1000000000LL + now.tv_nsec ) < target_ns );

		sendto( sock , buf , sizeof( buf ) , 0 , ( struct sockaddr * )&addr , sizeof( addr ) );
	}

	uint64_t endttt = now_ns();
	uint64_t diffttt = endttt - startttt;

	printf( "Loop of iterations took %llu ns (%.3f seconds)\n" ,
		 ( unsigned long long )diffttt , diffttt / 1e9 );


	close( sock );
	return 0;
}

