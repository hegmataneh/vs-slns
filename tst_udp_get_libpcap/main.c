#define _POSIX_C_SOURCE 200809L

typedef unsigned char u_char;
typedef unsigned int u_int;
typedef unsigned short u_short;

#include <pthread.h>
#include <pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sched.h>
#include <sys/mman.h>
#include <time.h>


//static volatile sig_atomic_t packet_count = 0;
static pcap_t * handle = NULL;

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

void handle_packet( u_char * user , const struct pcap_pkthdr * hdr , const u_char * packet )
{
	( void )user;
	( void )hdr;
	( void )packet;
	//packet_count++;
	_byte_counter1++;
}

void cleanup_and_exit( int sig )
{
	if ( sig ) ( void )sig;
	if ( handle )
	{
		pcap_breakloop( handle ); // in case we're inside pcap_loop
	}
	//printf( "\nTotal UDP packets on port 1234 captured: %u\n" , packet_count );
	if ( handle ) pcap_close( handle );
	exit( 0 );
}

int main( int argc , char * argv[] )
{
	char * dev = NULL;
	char errbuf[ PCAP_ERRBUF_SIZE ];
	struct bpf_program fp;
	char filter_exp[] = "udp and port 1234";
	bpf_u_int32 net = 0 , mask = 0;

	pthread_t trd_config_loader;
	pthread_create( &trd_config_loader , NULL , config_loader , NULL );


	if ( argc >= 2 )
	{
		dev = argv[ 1 ];
	}
	else
	{
		dev = pcap_lookupdev( errbuf );
		if ( !dev )
		{
			fprintf( stderr , "Couldn't find default device: %s\n" , errbuf );
			return 1;
		}
	}

	if ( pcap_lookupnet( dev , &net , &mask , errbuf ) == -1 )
	{
		fprintf( stderr , "Warning: couldn't get netmask for device %s: %s\n" , dev , errbuf );
		net = 0;
		mask = 0;
	}

	// Open in promiscuous mode, snapshot length 65535, no timeout (0 means immediate)
	handle = pcap_open_live( dev , 65535 , 1 , 1000 , errbuf );
	if ( !handle )
	{
		fprintf( stderr , "Couldn't open device %s: %s\n" , dev , errbuf );
		return 1;
	}

	// Compile and apply filter
	if ( pcap_compile( handle , &fp , filter_exp , 1 , mask ) == -1 )
	{
		fprintf( stderr , "Couldn't parse filter \"%s\": %s\n" , filter_exp , pcap_geterr( handle ) );
		pcap_close( handle );
		return 1;
	}
	if ( pcap_setfilter( handle , &fp ) == -1 )
	{
		fprintf( stderr , "Couldn't install filter \"%s\": %s\n" , filter_exp , pcap_geterr( handle ) );
		pcap_freecode( &fp );
		pcap_close( handle );
		return 1;
	}
	pcap_freecode( &fp );


	// set a large buffer (e.g., 10 MB)
	//if ( pcap_set_buffer_size( handle , 100 * 1024 * 1024 ) != 0 )
	//{
	//	fprintf( stderr , "failed to set buffer size\n" );
	//}

	printf( "Listening on device %s, counting UDP packets on port 1234. Press Ctrl-C to stop.\n" , dev );
	signal( SIGINT , cleanup_and_exit );
	signal( SIGTERM , cleanup_and_exit );

	// Capture indefinitely
	if ( pcap_loop( handle , -1 , handle_packet , NULL ) == -1 )
	{
		fprintf( stderr , "pcap_loop failed: %s\n" , pcap_geterr( handle ) );
		cleanup_and_exit( 0 );
	}

	// In normal flow, we should never reach here because SIGINT exits.
	cleanup_and_exit( 0 );
	return 0;
}
