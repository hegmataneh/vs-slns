#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_ADDR "192.168.56.61"
#define SERVER_PORT 5000

// Initialize SSL library and create SSL context
SSL_CTX * initialize_client_context()
{
	const SSL_METHOD * method;
	SSL_CTX * ctx;

	OpenSSL_add_all_algorithms();
	SSL_load_error_strings();
	//OpenSSL_add_ssl_module();

	method = TLS_client_method();  // Use TLS method
	ctx = SSL_CTX_new( method ); // Create SSL context

	if ( !ctx )
	{
		ERR_print_errors_fp( stderr );
		exit( EXIT_FAILURE );
	}

	return ctx;
}

// Connect to the server via SSL
SSL * connect_to_server( SSL_CTX * ctx )
{
	int server_sock;
	struct sockaddr_in server_addr;
	SSL * ssl;

	// Create TCP socket
	server_sock = socket( AF_INET , SOCK_STREAM , 0 );
	if ( server_sock < 0 )
	{
		perror( "Socket creation failed" );
		exit( EXIT_FAILURE );
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons( SERVER_PORT );
	server_addr.sin_addr.s_addr = inet_addr( SERVER_ADDR );

	// Connect to the server
	if ( connect( server_sock , ( struct sockaddr * )&server_addr , sizeof( server_addr ) ) < 0 )
	{
		perror( "Connection failed" );
		exit( EXIT_FAILURE );
	}

	// Create SSL object and associate it with the socket
	ssl = SSL_new( ctx ); // Create an SSL Object
	SSL_set_fd( ssl , server_sock );

	// Perform SSL handshake
	if ( SSL_connect( ssl ) <= 0 )
	{
		ERR_print_errors_fp( stderr );
	}
	else
	{
		printf( "SSL handshake successful\n" );
	}

	return ssl;
}

#define MIN_SYSERR_BUF_SZ 256

typedef char STANDARD_ERROR_BUFFER[ MIN_SYSERR_BUF_SZ ];
typedef STANDARD_ERROR_BUFFER * ERR_BUF; /*nullable and sizing prone */

int main()
{
	STANDARD_ERROR_BUFFER bbb = {0};

	ERR_BUF bbb2 = &bbb;

	int i = sizeof( *bbb2 );
	

	SSL_CTX * ctx;
	SSL * ssl;
	const char * msg = "Hello from SSL client!";
	char buf[ 1024 ];
	int bytes;

	// Initialize SSL
	SSL_library_init();

	ctx = initialize_client_context();
	ssl = connect_to_server( ctx );

	// Send message to the server
	SSL_write( ssl , msg , (int)strlen( msg ) );

	// Receive response from server
	bytes = SSL_read( ssl , buf , sizeof( buf ) - 1 );
	if ( bytes > 0 )
	{
		buf[ bytes ] = 0;  // Null terminate the message
		printf( "Received from server: %s\n" , buf );
	}
	else
	{
		ERR_print_errors_fp( stderr );
	}

	// Clean up
	SSL_shutdown( ssl );
	SSL_free( ssl );
	close( SSL_get_fd( ssl ) );
	SSL_CTX_free( ctx );

	return 0;
}
