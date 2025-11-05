#ifndef u_char
typedef unsigned char u_char;
#endif

#ifndef u_int
typedef unsigned int u_int;
#endif

#ifndef u_short
typedef unsigned short u_short;
#endif


#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>

#include <pcap.h>

/* Default network interface */
#define INTERFACE "ens194"

/* Destination filter */
#define DEST_IP "172.17.0.60"
#define DEST_PORT 10202



/* Callback function called by pcap_loop() for every captured packet */
void packet_handler( u_char * user , const struct pcap_pkthdr * header , const u_char * packet )
{
	( void )user; // unused

	printf( "Captured a packet with length: %d bytes\n" , header->len );

	// Optionally print first bytes for inspection
	for ( int i = 0; i < header->len && i < 64; i++ )
	{
		printf( "%02x " , packet[ i ] );
		if ( ( i + 1 ) % 16 == 0 ) printf( "\n" );
	}
	printf( "\n\n" );
}

int main()
{
	char errbuf[ PCAP_ERRBUF_SIZE ];
	pcap_t * handle;

	/* Open live capture on the interface */
	handle = pcap_open_live( INTERFACE , BUFSIZ , 1 , 1000 , errbuf );
	if ( handle == NULL )
	{
		fprintf( stderr , "Could not open device %s: %s\n" , INTERFACE , errbuf );
		return 1;
	}

	/* Build the filter expression */
	char filter_exp[ 128 ];
	//snprintf( filter_exp , sizeof( filter_exp ) ,
	//	"udp and dst host %s and dst port %d" , DEST_IP , DEST_PORT );

	snprintf( filter_exp , sizeof( filter_exp ) ,
		"udp and dst port %d" , DEST_PORT );

	/* Compile the filter */
	struct bpf_program fp;
	if ( pcap_compile( handle , &fp , filter_exp , 0 , PCAP_NETMASK_UNKNOWN ) == -1 )
	{
		fprintf( stderr , "Could not parse filter %s: %s\n" , filter_exp , pcap_geterr( handle ) );
		pcap_close( handle );
		return 1;
	}

	/* Apply the filter */
	if ( pcap_setfilter( handle , &fp ) == -1 )
	{
		fprintf( stderr , "Could not install filter %s: %s\n" , filter_exp , pcap_geterr( handle ) );
		pcap_freecode( &fp );
		pcap_close( handle );
		return 1;
	}

	pcap_freecode( &fp );

	printf( "Listening on %s for packets to %s:%d ...\n" , INTERFACE , DEST_IP , DEST_PORT );

	/* Capture indefinitely (CTRL+C to stop) */
	pcap_loop( handle , -1 , packet_handler , NULL );

	pcap_close( handle );
	return 0;
}
