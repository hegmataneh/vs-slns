#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 6000
#define BUFSIZE 50000

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

//size_t dump_buffer_ascii( const char * in , size_t in_len , char * out , size_t out_size )
//{
//	size_t oi = 0;  // output index
//
//	for ( size_t i = 0; i < in_len; i++ )
//	{
//		unsigned char c = in[ i ];
//
//		if ( isprint( c ) )
//		{
//			// Writes 1 byte if space available
//			if ( oi + 1 >= out_size )
//				break;
//			out[ oi++ ] = c;
//		}
//		else
//		{
//			// Non-printable → "\xHH" (4 chars)
//			// Check space for 4 chars
//			if ( oi + 4 >= out_size )
//				break;
//
//			// Write hex escape
//			out[ oi++ ] = '\\';
//			out[ oi++ ] = 'x';
//			out[ oi++ ] = "0123456789ABCDEF"[ c >> 4 ];
//			out[ oi++ ] = "0123456789ABCDEF"[ c & 0x0F ];
//		}
//	}
//
//	// Null terminate
//	if ( out_size > 0 )
//		out[ oi < out_size ? oi : out_size - 1 ] = '\0';
//
//	return oi;
//}

void dump_partition_hex( const unsigned char * buf , size_t size )
{
	size_t offset = 0;
	const size_t PART = 16;   /* partition size */

	while ( offset < size )
	{
		size_t chunk = PART;
		if ( offset + chunk > size )
			chunk = size - offset;

		/* header line for each partition */
		printf( "\n--- PARTITION at offset 0x%08zx (%zu bytes) ---\n" ,
			offset , chunk );

		/* print hex */
		printf( "%08zx  " , offset );
		for ( size_t i = 0; i < PART; i++ )
		{
			if ( i < chunk )
				printf( "%02x " , buf[ offset + i ] );
			else
				printf( "   " );

			if ( i == 7 ) printf( " " );
		}

		/* ASCII */
		printf( " |" );
		for ( size_t i = 0; i < chunk; i++ )
		{
			unsigned char c = buf[ offset + i ];
			putchar( isprint( c ) ? c : '.' );
		}
		printf( "|\n" );

		offset += chunk;
	}
}

int main( void )
{
	int server_fd , client_fd;
	struct sockaddr_in addr;
	char buffer[ BUFSIZE ];

	char out_buffer[ BUFSIZE ];

	unsigned long long total_bytes = 0;
	unsigned long long packet_count = 0;
	//int expected_next = 1;  // pattern expected: 1 2 3 ... 9 then repeat
	//int ignore_packet_aggregation = 0;

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

		dump_partition_hex( buffer , n );
		printf( "\n\n" );
		//size_t out_s = dump_buffer_ascii( buffer , n , out_buffer , sizeof( out_buffer ) );
		//printf( "%s\n\n\n" , out_buffer );
		fflush( stdout );

		total_bytes += n;
		packet_count++;

		// --- Pattern check logic ---
		//for ( ssize_t i = 0; i < n; i++ )
		//{
		//	int received_value = buffer[ i ] - '0';  // assuming ASCII digits

		//	if ( received_value != expected_next )
		//	{
		//		if ( !ignore_packet_aggregation )
		//		{
		//			printf( "Pattern mismatch at byte %llu: expected %d but got %d\n" ,
		//				total_bytes , expected_next , received_value );
		//		}
		//	}
		//	else
		//	{
		//		ignore_packet_aggregation = 1;
		//	}

		//	expected_next = MAX( ( expected_next + 1 ) % 10 , 1 );  // cycle 0~9
		//}
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
