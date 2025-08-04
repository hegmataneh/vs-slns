#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
//#include <linux/if.h>
#include <net/if.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <poll.h>
#include <pthread.h>

#define BLOCK_SIZE (1 << 22)   // 4MB
#define FRAME_SIZE 2048        // fits typical MTU
#define BLOCK_NR 64            // number of blocks in ring
#define TARGET_UDP_PORT 1234

static volatile sig_atomic_t stop = 0;

int _byte_counter = 0;
int _continue_counter1 = 0;
int _continue_counter2 = 0;

//#define TPACKET_ALIGNMENT(x) (((x) + TPACKET_HDRLEN - 1) & ~(TPACKET_HDRLEN - 1))

void * config_loader( void * )
{
	while ( 1 )
	{
		printf( "%d - %d - %d\n" , _byte_counter , _continue_counter1 , _continue_counter2 );
		sleep( 1 );
	}

	return NULL;
}

void handle_sigint( int signo )
{
	( void )signo;
	stop = 1;
}

int bind_iface( int sock , const char * ifname )
{
	struct sockaddr_ll sll;
	memset( &sll , 0 , sizeof( sll ) );
	sll.sll_family = AF_PACKET;
	sll.sll_ifindex = ( int )if_nametoindex( ifname );
	if ( !sll.sll_ifindex ) return -1;
	sll.sll_protocol = htons( ETH_P_ALL );
	return bind( sock , ( struct sockaddr * )&sll , sizeof( sll ) );
}

int set_promisc( int sock , const char * ifname )
{
	struct packet_mreq mreq;
	memset( &mreq , 0 , sizeof( mreq ) );
	mreq.mr_ifindex = ( int )if_nametoindex( ifname );
	if ( !mreq.mr_ifindex ) return -1;
	mreq.mr_type = PACKET_MR_PROMISC;
	return setsockopt( sock , SOL_PACKET , PACKET_ADD_MEMBERSHIP , &mreq , sizeof( mreq ) );
}

int main( int argc , char ** argv )
{
	//if ( argc != 2 )
	//{
	//	fprintf( stderr , "Usage: %s <ifname>\n" , argv[ 0 ] );
	//	return 1;
	//}
	const char * ifname = "enp0s3";

	signal( SIGINT , handle_sigint );

	pthread_t trd_config_loader;
	pthread_create( &trd_config_loader , NULL , config_loader , NULL );

	int sock = socket( AF_PACKET , SOCK_RAW , htons( ETH_P_ALL ) );
	if ( sock < 0 )
	{
		perror( "socket" );
		return 1;
	}

	if ( set_promisc( sock , ifname ) < 0 )
	{
		perror( "set_promisc (continuing without it)" );
	}

	if ( bind_iface( sock , ifname ) < 0 )
	{
		perror( "bind" );
		close( sock );
		return 1;
	}

	struct tpacket_req3 req = { 0 };
	req.tp_block_size = BLOCK_SIZE;
	req.tp_block_nr = BLOCK_NR;
	req.tp_frame_size = FRAME_SIZE;
	req.tp_frame_nr = ( BLOCK_SIZE * BLOCK_NR ) / FRAME_SIZE;
	req.tp_retire_blk_tov = 60; // flush timeout in ms

	int ver = TPACKET_V3;
	if ( setsockopt( sock , SOL_PACKET , PACKET_VERSION , &ver , sizeof( ver ) ) < 0 )
	{
		perror( "setsockopt PACKET_VERSION" );
		// handle/fail
	}

	if ( setsockopt( sock , SOL_PACKET , PACKET_RX_RING , &req , sizeof( req ) ) < 0 )
	{
		perror( "setsockopt PACKET_RX_RING" );
		close( sock );
		return 1;
	}

	size_t map_size = req.tp_block_size * req.tp_block_nr;
	void * map = mmap( NULL , map_size , PROT_READ | PROT_WRITE , MAP_SHARED , sock , 0 );
	if ( map == MAP_FAILED )
	{
		perror( "mmap" );
		close( sock );
		return 1;
	}

	printf( "Counting UDP packets to port %d on interface %s (TPACKET_V3)... Ctrl-C to stop.\n" ,
		TARGET_UDP_PORT , ifname );

	//unsigned long total_matching = 0;
	//unsigned long last_total = 0;
	//time_t last_print = time( NULL );
	struct pollfd pfd = { .fd = sock, .events = POLLIN };

	while ( !stop )
	{
		int ret = poll( &pfd , 1 , 1000 ); // 1s timeout
		if ( ret < 0 )
		{
			if ( errno == EINTR ) continue;
			perror( "poll" );
			break;
		}
		// iterate blocks
		for ( int b = 0; b < req.tp_block_nr; b++ )
		{
			struct tpacket_block_desc * block = ( void * )map + ( b * ( int )req.tp_block_size );
			if ( ( block->hdr.bh1.block_status & TP_STATUS_USER ) == 0 )
				continue; // not ready

			// walk packets in block
			uint8_t * ptr = ( uint8_t * )block + block->hdr.bh1.offset_to_first_pkt;
			for ( int i = 0; i < block->hdr.bh1.num_pkts; i++ )
			{
				struct tpacket3_hdr * hdr = ( struct tpacket3_hdr * )ptr;

				// sanity: ensure we have enough for Ethernet + IP header
				uint8_t * pkt = ( uint8_t * )hdr + hdr->tp_mac;
				size_t snaplen = hdr->tp_snaplen;

				if ( snaplen >= sizeof( struct ethhdr ) + sizeof( struct iphdr ) )
				{
					struct ethhdr * eth = ( struct ethhdr * )pkt;
					if ( ntohs( eth->h_proto ) == ETH_P_IP )
					{
						struct iphdr * ip = ( struct iphdr * )( pkt + sizeof( struct ethhdr ) );
						if ( ip->version == 4 && ip->protocol == IPPROTO_UDP )
						{
							size_t ip_hdr_len = ip->ihl * 4;
							if ( snaplen >= sizeof( struct ethhdr ) + ip_hdr_len + sizeof( struct udphdr ) )
							{
								struct udphdr * udp = ( struct udphdr * )( pkt + sizeof( struct ethhdr ) + ip_hdr_len );
								if ( ntohs( udp->dest ) == TARGET_UDP_PORT )
								{
									//total_matching++;
									_byte_counter++;
								}
							}
						}
					}
				}

				if ( hdr->tp_next_offset == 0 )
					break;
				ptr += hdr->tp_next_offset;
			}

			// release block
			block->hdr.bh1.block_status = TP_STATUS_KERNEL;
		}

		//time_t now = time( NULL );
		//if ( now != last_print )
		//{
		//	unsigned long delta = total_matching - last_total;
		//	printf( "Total UDP:%lu  rate/sec:%lu\n" , total_matching , delta );
		//	last_total = total_matching;
		//	last_print = now;
		//}
	}

	//printf( "\nFinal count of UDP packets to port %d: %lu\n" , TARGET_UDP_PORT , total_matching );

	munmap( map , map_size );
	close( sock );
	return 0;
}
