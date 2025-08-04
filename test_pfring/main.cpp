#include <stdio.h>
#include <pfring.h>

int main()
{
	const char * device = "enp0s3";
	pfring * ring;
	u_char * buffer;
	struct pfring_pkthdr hdr;
	int ret;
	unsigned long pkt_count = 0;

	ring = pfring_open( device , 1500 , PF_RING_PROMISC );
	if ( ring == NULL )
	{

		fprintf(stderr, "pfring_open failed: %s\n", strerror(errno));

		fprintf( stderr , "pfring_open(%s) failed. Are you root and is PF_RING loaded?\n" , device );
		return 1;
	}

	if ( pfring_enable_ring( ring ) != 0 )
	{
		fprintf( stderr , "pfring_enable_ring failed\n" );
		pfring_close( ring );
		return 1;
	}

	printf( "Capturing 10 packets on %s\n" , device );
	while ( pkt_count < 10 )
	{
		ret = pfring_recv( ring , &buffer , 0 , &hdr , 1 );  // block until packet
		if ( ret > 0 )
		{
			pkt_count++;
			printf( "Packet %lu: captured length=%u original length=%u\n" ,
				pkt_count , hdr.caplen , hdr.len );
		}
	}

	pfring_close( ring );
	printf( "Done. Total packets: %lu\n" , pkt_count );
	return 0;
}
