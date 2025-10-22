#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <sys/socket.h>
#include <threads.h>

int main()
{
	int sock = socket( AF_INET , SOCK_DGRAM , 0 );
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons( 10200 );
	inet_pton( AF_INET , "192.168.1.2" , &addr.sin_addr );

	char buf[ 2000 ] = {0};

	for ( size_t i = 0; i < sizeof(buf); i++ )
	{
		buf[ i ] = ( char )( 'A' + ( i % 26 ) );   // store as character '1'..'9'
	}

	for ( long ii = 0 ; ii < 10000000 ; ii++ )
	{
		sendto( sock , buf , sizeof( buf ) , 0 , ( struct sockaddr * )&addr , sizeof( addr ) );

		//struct timespec ts;
		//memset( &ts , 0 , sizeof( ts ) );
		//ts.tv_sec = 0;   // seconds
		//ts.tv_nsec = 100000000;  // nanoseconds
		//thrd_sleep( &ts , NULL );
	}
    return 0;
}
