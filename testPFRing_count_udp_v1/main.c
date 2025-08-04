#define _POSIX_C_SOURCE 200809L

typedef unsigned char u_char;
typedef unsigned int u_int;
typedef unsigned short u_short;

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <time.h>

#include <pfring.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

volatile sig_atomic_t keep_running = 1;

void handle_sigint( int sig )
{
	( void )sig;
	keep_running = 0;
}


int _byte_counter1 = 0;
int _byte_counter2 = 0;
int _continue_counter = 0;

void * config_loader( void * )
{
	while ( 1 )
	{
		printf( "%d - %d - %d\n" , _byte_counter1 , _byte_counter2 , _continue_counter );
		sleep( 1 );
	}

	return NULL;
}

int main()
{
	//if ( argc != 2 )
	//{
	//	fprintf( stderr , "Usage: %s <interface>\nExample: %s eth0\n" , argv[ 0 ] , argv[ 0 ] );
	//	return 1;
	//}

	const char * device = "enp0s3";
	pfring * ring = NULL;
	u_int32_t flags = PF_RING_PROMISC | PF_RING_TIMESTAMP; // promiscuous + timestamps
	const int snaplen = 1500; // typical MTU
	char * bpf_filter = "udp and dst port 1234";

	signal( SIGINT , handle_sigint );
	signal( SIGTERM , handle_sigint );

	pthread_t trd_config_loader;
	pthread_create( &trd_config_loader , NULL , config_loader , NULL );

	// Open PF_RING on the given device
	ring = pfring_open( ( char * )device , ( u_int32_t )snaplen , flags );
	if ( ring == NULL )
	{
		fprintf( stderr , "ERROR: pfring_open failed on device %s (are you root and is PF_RING loaded?)\n" , device );
		return 1;
	}

	// Enable the ring
	if ( pfring_enable_ring( ring ) != 0 )
	{
		fprintf( stderr , "ERROR: pfring_enable_ring failed\n" );
		pfring_close( ring );
		return 1;
	}

	// Apply BPF filter: only UDP dst port 1234
	if ( pfring_set_bpf_filter( ring , bpf_filter ) != 0 )
	{
		fprintf( stderr , "WARNING: failed to set BPF filter '%s', continuing without it\n" , bpf_filter );
	}

	printf( "Listening on %s for UDP packets to port 1234. Press Ctrl-C to stop.\n" , device );

	//uint64_t total_count = 0;
	//time_t last_report = time( NULL );

	while ( keep_running )
	{
		// Wait up to 100ms for packets
		if ( pfring_poll( ring , 100 ) <= 0 )
		{
			// no packets this interval
		}

		u_char * buffer;
		struct pfring_pkthdr hdr;

		// Retrieve as many packets as available without blocking
		while ( pfring_recv( ring , &buffer , 0 , &hdr , 1 ) > 0 )
		{
			// We already filtered by BPF, so assume UDP dst port 1234, but for safety we can do a quick sanity parse:
			if ( hdr.caplen >= ( int )( sizeof( struct ethhdr ) + sizeof( struct iphdr ) + sizeof( struct udphdr ) ) )
			{
				// Skip Ethernet if present: PF_RING may give Ethernet frames depending on mode
				// Assume standard Ethernet header
				const struct iphdr * ip = ( const struct iphdr * )( buffer + 14 );
				if ( ip->protocol == IPPROTO_UDP )
				{
					int ip_header_len = ip->ihl * 4;
					const struct udphdr * udp = ( const struct udphdr * )( buffer + 14 + ip_header_len );
					uint16_t dst_port = ntohs( udp->dest );
					if ( dst_port == 1234 )
					{
						_byte_counter1++;
					}
				}
			}
			else
			{
				// Fallback: count anyway if BPF did the work
				_byte_counter2++;
			}
		}

		time_t now = time( NULL );
		//if ( now - last_report >= 1 )
		//{
		//	//printf( "Total UDP packets to port 1234 so far: %" PRIu64 "\n" , total_count );
		//	last_report = now;
		//}
	}

	//printf( "Exiting. Final count: %" PRIu64 "\n" , total_count );
	pfring_close( ring );
	return 0;
}
