#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 5000
#define BUFSIZE 50000

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

int main( void )
{
	int server_fd , client_fd;
	struct sockaddr_in addr;
	char buffer[ BUFSIZE ];

	unsigned long long total_bytes = 0;
	unsigned long long packet_count = 0;
	int expected_next = 1;  // pattern expected: 1 2 3 ... 9 then repeat
	int ignore_packet_aggregation = 0;

	server_fd = socket( AF_INET , SOCK_STREAM , 0 );
	if ( server_fd < 0 )
	{
		perror( "socket()" );
		return 1;
	}

	int opt = 1;
	setsockopt( server_fd, SOL_SOCKET , SO_REUSEADDR , &opt , sizeof( opt ) );

	memset( &addr , 0 , sizeof( addr ) );
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;  // listen on any IP
	addr.sin_port = htons( PORT );

	if ( bind( server_fd , ( struct sockaddr * )&addr , sizeof( addr ) ) < 0 )
	{
		perror( "bind()" );
		close( server_fd );
		return 1;
	}

	if ( listen( server_fd , 1 ) < 0 )
	{
		perror( "listen()" );
		close( server_fd );
		return 1;
	}

	printf( "Listening on port %d...\n" , PORT );

	client_fd = accept( server_fd , NULL , NULL );
	if ( client_fd < 0 )
	{
		perror( "accept()" );
		close( server_fd );
		return 1;
	}

	printf( "Client connected.\n" );

	while ( 1 )
	{
		ssize_t n = recv( client_fd , buffer , BUFSIZE , 0 );
		if ( n < 0 )
		{
			perror( "recv()" );
			break;
		}
		if ( n == 0 )
		{
			printf( "Client disconnected.\n" );
			break;
		}

		total_bytes += n;
		packet_count++;

		// --- Pattern check logic ---
		for ( ssize_t i = 0; i < n; i++ )
		{
			int received_value = buffer[ i ] - '0';  // assuming ASCII digits

			if ( received_value != expected_next )
			{
				if ( !ignore_packet_aggregation )
				{
					printf( "Pattern mismatch at byte %llu: expected %d but got %d\n" ,
						total_bytes , expected_next , received_value );
				}
			}
			else
			{
				ignore_packet_aggregation = 1;
			}

			expected_next = MAX( ( expected_next + 1 ) % 10 , 1 );  // cycle 0~9
		}
		// ----------------------------

		if ( !( packet_count % 1000 ) )
		{
			printf( "Packets: %llu | Total bytes: %llu\r" , packet_count , total_bytes );
			fflush( stdout );
		}
	}

	printf( "Packets: %llu | Total bytes: %llu\r" , packet_count , total_bytes );
	fflush( stdout );

	close( client_fd );
	close( server_fd );
	return 0;
}
