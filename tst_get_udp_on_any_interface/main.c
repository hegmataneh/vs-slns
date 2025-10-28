#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 1234
#define BUFSZ 2048

int main( void )
{
	int sfd = -1;
	struct sockaddr_in srv;
	char buf[ BUFSZ ];

	// 1) create socket (IPv4 UDP)
	sfd = socket( AF_INET , SOCK_DGRAM , 0 );
	if ( sfd < 0 )
	{
		perror( "socket" );
		return 1;
	}

	// 2) allow address reuse so restarting quickly doesn't fail
	int opt = 1;
	if ( setsockopt( sfd , SOL_SOCKET , SO_REUSEADDR , &opt , sizeof( opt ) ) < 0 )
	{
		perror( "setsockopt SO_REUSEADDR" );
		close( sfd );
		return 1;
	}

	// 3) bind to all interfaces (0.0.0.0) on PORT
	memset( &srv , 0 , sizeof( srv ) );
	srv.sin_family = AF_INET;
	srv.sin_addr.s_addr = htonl( INADDR_ANY ); // bind to any local address
	srv.sin_port = htons( PORT );

	if ( bind( sfd , ( struct sockaddr * )&srv , sizeof( srv ) ) < 0 )
	{
		perror( "bind" );
		close( sfd );
		return 1;
	}

	printf( "Listening UDP port %d on all interfaces (0.0.0.0:%d)\n" , PORT , PORT );

	// 4) receive loop
	for ( ;;)
	{
		struct sockaddr_in cli;
		socklen_t cli_len = sizeof( cli );
		ssize_t n = recvfrom( sfd , buf , sizeof( buf ) - 1 , 0 ,
			( struct sockaddr * )&cli , &cli_len );
		if ( n < 0 )
		{
			// Interrupted by signal? continue; otherwise exit.
			if ( errno == EINTR ) continue;
			perror( "recvfrom" );
			break;
		}

		buf[ n ] = '\0'; // treat as text for printing (careful with binary)
		char ipstr[ INET_ADDRSTRLEN ];
		if ( inet_ntop( AF_INET , &cli.sin_addr , ipstr , sizeof( ipstr ) ) == NULL )
			strncpy( ipstr , "unknown" , sizeof( ipstr ) );

		printf( "Received %zd bytes from %s:%d -> \"%s\"\n" ,
			n , ipstr , ntohs( cli.sin_port ) , buf );
	}

	close( sfd );
	return 0;
}
