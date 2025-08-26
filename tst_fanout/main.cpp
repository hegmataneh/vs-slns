#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/filter.h>
#include <net/if.h>
#include <sys/ioctl.h>

#define THREAD_COUNT 4
#define INTERFACE "enp0s3"

#define BUF_SIZE 2048

void * recv_thread( void * arg )
{
	int idx = *( int * )arg;
	int sock;
	struct sockaddr_ll sll;
	char buf[ 2048 ];

	if ( idx == 0 )
	{
		perror( "idx == 0" );
	}
	else if (idx == 1)
	{
		perror( "idx == 1" );
	}
	else if ( idx == 2 )
	{
		perror( "idx == 2" );
	}
	else if ( idx == 3 )
	{
		perror( "idx == 3" );
	}

	sock = socket( AF_PACKET , SOCK_RAW , htons( ETH_P_IP ) );
	if ( sock < 0 )
	{
		perror( "socket" );
		pthread_exit( NULL );
	}

	// Set interface index
	struct ifreq ifr = { 0 };
	strncpy( ifr.ifr_name , INTERFACE , IFNAMSIZ - 1 );
	if ( ioctl( sock , SIOCGIFINDEX , &ifr ) < 0 )
	{
		perror( "SIOCGIFINDEX" );
		close( sock );
		pthread_exit( NULL );
	}

	// Bind to interface
	memset( &sll , 0 , sizeof( sll ) );
	sll.sll_family = AF_PACKET;
	sll.sll_protocol = htons( ETH_P_IP );
	sll.sll_ifindex = ifr.ifr_ifindex;
	if ( bind( sock , ( struct sockaddr * )&sll , sizeof( sll ) ) < 0 )
	{
		perror( "bind" );
		close( sock );
		pthread_exit( NULL );
	}

	// Enable PACKET_FANOUT
	int fanout_type = PACKET_FANOUT_ROLLOVER;
	int fanout_id = 0x1234; // all threads must share same ID
	int fanout_arg = ( fanout_id | ( fanout_type << 16 ) );
	if ( setsockopt( sock , SOL_PACKET , PACKET_FANOUT , &fanout_arg , sizeof( fanout_arg ) ) < 0 )
	{
		perror( "setsockopt PACKET_FANOUT" );
		close( sock );
		pthread_exit( NULL );
	}

	int val = 50; // microseconds to spin per syscall
	if ( setsockopt( sock , SOL_SOCKET , SO_BUSY_POLL , &val , sizeof( val ) ) < 0 )
	{
		perror( "setsockopt SO_BUSY_POLL" );
		close( sock );
		pthread_exit( NULL );
	}

	static long long ar[THREAD_COUNT][BUF_SIZE] = {0};
	static int i = 0;

	while ( 1 )
	{
		ssize_t len = recv( sock , buf , sizeof( buf ) , MSG_DONTWAIT );

		if ( len <= 0 ) continue;

		ar[ idx ][ len ]++;

		if ( !( i++ % 1000 ) )
		{
			for ( int t = 0 ; t < THREAD_COUNT ; t++ )
			{
				for ( int b = 0 ; b < BUF_SIZE ; b++ )
				{
					if ( ar[t][b] )
					{
						printf( "t:%d b:%d %lld\n" , t , b , ar[t][b] );
					}
				}
			}
			printf( "-----------------------------\n" );
		}
		
		

		//if ( len > 0 )
		//{
		//	printf( "Thread %d received packet (%ld bytes)\n" , idx , len );
		//}
	}


	close( sock );
	pthread_exit( NULL );
}

int main()
{
	pthread_t threads[ THREAD_COUNT ];
	int thread_ids[ THREAD_COUNT ];

	for ( int i = 0; i < THREAD_COUNT; ++i )
	{
		thread_ids[ i ] = i;
		pthread_create( &threads[ i ] , NULL , recv_thread , &thread_ids[ i ] );
	}

	for ( int i = 0; i < THREAD_COUNT; ++i )
	{
		pthread_join( threads[ i ] , NULL );
	}

	return 0;
}
