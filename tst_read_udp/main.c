#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define LOCAL_IP   "192.168.4.2"
#define LOCAL_PORT 6000
#define BUF_SIZE   20480

int main( void )
{
	int sockfd;
	struct sockaddr_in local_addr , remote_addr;
	socklen_t addr_len = sizeof( remote_addr );
	char buffer[ BUF_SIZE ];
	ssize_t recv_len;

	// Create UDP socket
	sockfd = socket( AF_INET , SOCK_DGRAM , 0 );
	if ( sockfd < 0 )
	{
		perror( "socket" );
		return 1;
	}

	int optval = 1;
	setsockopt( sockfd , SOL_SOCKET , SO_REUSEADDR , &optval , sizeof( optval ) );

	//int flags = fcntl( sockfd , F_GETFL );
	//fcntl( sockfd , F_SETFL , flags | O_NONBLOCK );

	// Prepare local address
	memset( &local_addr , 0 , sizeof( local_addr ) );
	local_addr.sin_family = AF_INET;
	local_addr.sin_port = htons( LOCAL_PORT );
	//local_addr.sin_addr.s_addr = inet_addr( LOCAL_IP );
	local_addr.sin_addr.s_addr = INADDR_ANY;

	// Bind to IP and port
	if ( bind( sockfd , ( struct sockaddr * )&local_addr , sizeof( local_addr ) ) < 0 )
	{
		perror( "bind" );
		close( sockfd );
		return 1;
	}

	//printf( "Listening for UDP packets on %s:%d ...\n" , LOCAL_IP , LOCAL_PORT );

	// Loop to read packets iteratively
	while ( 1 )
	{
		recv_len = recvfrom( sockfd , buffer , BUF_SIZE - 1 , 0 ,
			( struct sockaddr * )&remote_addr , &addr_len );
		if ( recv_len < 0 )
		{
			perror( "recvfrom" );
			//break;
			continue;
		}

		buffer[ recv_len ] = '\0';  // Null-terminate for safe printing
		printf( "Received %zd bytes from %s:%d\n" ,
			recv_len ,
			inet_ntoa( remote_addr.sin_addr ) ,
			ntohs( remote_addr.sin_port ) );

		printf( "Data: %s\n" , buffer );
	}

	close( sockfd );
	return 0;
}
