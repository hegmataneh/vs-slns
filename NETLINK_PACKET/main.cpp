#include <netlink/netlink.h>
#include <netlink/socket.h>
#include <netlink/msg.h>
#include <netlink/attr.h>
#include <linux/if_packet.h>
#include <linux/netlink.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>


#define MAX_PAYLOAD 8192

static int recv_handler( struct nl_msg * msg , void * arg )
{
	struct nlmsghdr * nlh = nlmsg_hdr( msg );
	struct nlattr * attr;
	int len = nlh->nlmsg_len - NLMSG_LENGTH( 0 );

	unsigned char * data = ( unsigned char * )NLMSG_DATA( nlh );

	if ( len > sizeof( struct ethhdr ) )
	{
		struct iphdr * iph = ( struct iphdr * )( data + sizeof( struct ethhdr ) );

		if ( iph->protocol == IPPROTO_UDP )
		{
			struct udphdr * udph = ( struct udphdr * )( ( unsigned char * )iph + iph->ihl * 4 );

			char src[ INET_ADDRSTRLEN ] , dst[ INET_ADDRSTRLEN ];
			inet_ntop( AF_INET , &iph->saddr , src , sizeof( src ) );
			inet_ntop( AF_INET , &iph->daddr , dst , sizeof( dst ) );

			printf( "UDP packet: %s:%d -> %s:%d, len=%d\n" ,
				src , ntohs( udph->source ) , dst , ntohs( udph->dest ) ,
				ntohs( udph->len ) );
		}
	}
	return NL_OK;
}

int main()
{
	struct nl_sock * sock;
	int err;

	sock = nl_socket_alloc();
	if ( !sock )
	{
		fprintf( stderr , "Failed to allocate netlink socket.\n" );
		return -1;
	}

	// Connect to NETLINK_PACKET
	if ( ( err = nl_connect( sock , NETLINK_PACKET ) ) < 0 )
	{
		fprintf( stderr , "nl_connect error: %s\n" , nl_geterror( err ) );
		nl_socket_free( sock );
		return -1;
	}

	// Increase recv buffer
	nl_socket_set_buffer_size( sock , MAX_PAYLOAD , 0 );

	// Set callback
	nl_socket_modify_cb( sock , NL_CB_VALID , NL_CB_CUSTOM , recv_handler , NULL );

	printf( "Listening for UDP packets via NETLINK_PACKET...\n" );

	while ( 1 )
	{
		err = nl_recvmsgs_default( sock );
		if ( err < 0 )
		{
			fprintf( stderr , "recv error: %s\n" , nl_geterror( err ) );
			break;
		}
	}

	nl_socket_free( sock );
	return 0;
}
