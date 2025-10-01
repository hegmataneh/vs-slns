#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <sys/socket.h>


#define DEST_IP   "172.17.0.60"
#define DEST_PORT 1234
#define SRC_PORT  54321
#define PACKET_SIZE 2000    // total UDP payload size
#define MTU        1500     // standard Ethernet MTU
#define IP_HDRLEN  20
#define UDP_HDRLEN 8

struct ip
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
	unsigned int ip_hl : 4;		/* header length */
	unsigned int ip_v : 4;		/* version */
#endif
#if __BYTE_ORDER == __BIG_ENDIAN
	unsigned int ip_v : 4;		/* version */
	unsigned int ip_hl : 4;		/* header length */
#endif
	uint8_t ip_tos;			/* type of service */
	unsigned short ip_len;		/* total length */
	unsigned short ip_id;		/* identification */
	unsigned short ip_off;		/* fragment offset field */
#define	IP_RF 0x8000			/* reserved fragment flag */
#define	IP_DF 0x4000			/* dont fragment flag */
#define	IP_MF 0x2000			/* more fragments flag */
#define	IP_OFFMASK 0x1fff		/* mask for fragmenting bits */
	uint8_t ip_ttl;			/* time to live */
	uint8_t ip_p;			/* protocol */
	unsigned short ip_sum;		/* checksum */
	struct in_addr ip_src , ip_dst;	/* source and dest address */
};

// Simple checksum function
unsigned short checksum( unsigned short * buf , int len )
{
	unsigned long sum = 0;
	while ( len > 1 )
	{
		sum += *buf++;
		len -= 2;
	}
	if ( len ) sum += *( unsigned char * )buf;
	while ( sum >> 16 ) sum = ( sum & 0xffff ) + ( sum >> 16 );
	return ( unsigned short )( ~sum );
}

int main()
{
	int sockfd;
	char datagram[ PACKET_SIZE + IP_HDRLEN + UDP_HDRLEN ];
	struct ip * iph = ( struct ip * )datagram;
	struct udphdr * udph = ( struct udphdr * )( datagram + IP_HDRLEN );

	// Fill UDP payload with 0x01
	memset( datagram , 0 , sizeof( datagram ) );
	memset( datagram + IP_HDRLEN + UDP_HDRLEN , 0x01 , PACKET_SIZE );

	// Prepare UDP header
	udph->source = htons( SRC_PORT );
	udph->dest = htons( DEST_PORT );
	udph->len = ( UDP_HDRLEN + PACKET_SIZE );
	udph->len = htons( udph->len );
	udph->check = 0; // kernel won't compute UDP checksum for raw socket, so we leave 0

	// Prepare IP header (common for all fragments)
	iph->ip_hl = 5;
	iph->ip_v = 4;
	iph->ip_tos = 0;
	iph->ip_len = htons( IP_HDRLEN + UDP_HDRLEN + PACKET_SIZE );
	iph->ip_id = htons( 12345 ); // identification number (same for all fragments)
	iph->ip_off = 0;           // will be set per fragment
	iph->ip_ttl = 64;
	iph->ip_p = IPPROTO_UDP;
	iph->ip_sum = 0;
	iph->ip_src.s_addr = inet_addr( "172.17.0.59" ); // CHANGE to your real source IP
	iph->ip_dst.s_addr = inet_addr( DEST_IP );

	// Open raw socket
	sockfd = socket( AF_INET , SOCK_RAW , IPPROTO_RAW );
	if ( sockfd < 0 )
	{
		perror( "socket" );
		return 1;
	}

	int enable = 1;
	if ( setsockopt( sockfd , IPPROTO_IP , IP_HDRINCL , &enable , sizeof( enable ) ) < 0 )
	{
		perror( "setsockopt" );
		close( sockfd );
		return 1;
	}

	// Calculate fragment sizes
	int max_payload_per_frag = ( ( MTU - IP_HDRLEN ) & ~7 ); // multiple of 8 bytes
	int total_len = UDP_HDRLEN + PACKET_SIZE;
	int offset = 0;

	while ( offset < total_len )
	{
		int frag_payload_len = total_len - offset;
		if ( frag_payload_len > max_payload_per_frag )
			frag_payload_len = max_payload_per_frag;

		// Construct fragment buffer
		char frag[ IP_HDRLEN + frag_payload_len ];
		memcpy( frag , iph , IP_HDRLEN ); // copy IP header
		memcpy( frag + IP_HDRLEN , ( ( char * )udph ) + offset , frag_payload_len );

		struct ip * frag_iph = ( struct ip * )frag;
		frag_iph->ip_len = htons( IP_HDRLEN + frag_payload_len );

		// Set fragment offset and MF flag
		int frag_off = offset >> 3;
		if ( offset + frag_payload_len < total_len )
			frag_off |= 0x2000; // More Fragments flag
		frag_iph->ip_off = htons( frag_off );

		frag_iph->ip_sum = 0;
		frag_iph->ip_sum = checksum( ( unsigned short * )frag , IP_HDRLEN );

		struct sockaddr_in sin;
		sin.sin_family = AF_INET;
		sin.sin_addr.s_addr = iph->ip_dst.s_addr;

		if ( sendto( sockfd , frag , IP_HDRLEN + frag_payload_len , 0 ,
			( struct sockaddr * )&sin , sizeof( sin ) ) < 0 )
		{
			perror( "sendto" );
		}

		offset += frag_payload_len;
	}

	close( sockfd );
	return 0;
}
