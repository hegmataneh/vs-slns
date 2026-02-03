#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 5000

// Initialize SSL library and create SSL context
SSL_CTX * initialize_server_context()
{
	const SSL_METHOD * method;
	SSL_CTX * ctx;

	OpenSSL_add_all_algorithms();   // Load all algorithms
	SSL_load_error_strings();       // Load error strings
	//OpenSSL_add_ssl_module();       // Add SSL module

	method = TLS_server_method();   // Use TLS method
	ctx = SSL_CTX_new( method );      // Create SSL context

	if ( !ctx )
	{
		ERR_print_errors_fp( stderr );
		exit( EXIT_FAILURE );
	}

	return ctx;
}

// Configure SSL context with certificate and private key
int configure_server_context( SSL_CTX * ctx )
{
	// Load server certificate
	if ( SSL_CTX_use_certificate_file( ctx , "server.crt" , SSL_FILETYPE_PEM ) <= 0 )
	{
		ERR_print_errors_fp( stderr );
		return 0;
	}

	// Load server private key
	if ( SSL_CTX_use_PrivateKey_file( ctx , "server.key" , SSL_FILETYPE_PEM ) <= 0 )
	{
		ERR_print_errors_fp( stderr );
		return 0;
	}

	// Verify the private key
	if ( !SSL_CTX_check_private_key( ctx ) )
	{
		fprintf( stderr , "Private key does not match the certificate public key\n" );
		return 0;
	}

	return 1;
}

// Handle SSL communication with the client
void handle_client( SSL * ssl )
{
	char buf[ 1024 ];
	int bytes;

	// Read data from client
	bytes = SSL_read( ssl , buf , sizeof( buf ) );
	if ( bytes > 0 )
	{
		buf[ bytes ] = 0;  // Null terminate the message
		printf( "Received message: %s\n" , buf );
	}
	else
	{
		ERR_print_errors_fp( stderr );
	}

	// Write response to client
	const char * msg = "Hello from SSL server!";
	SSL_write( ssl , msg , ( int )strlen( msg ) );
}

int main()
{
	SSL_CTX * ctx;
	int server_sock , client_sock;
	struct sockaddr_in addr;
	socklen_t addr_len = sizeof( addr );
	SSL * ssl;

	// Initialize SSL library
	SSL_library_init();

	ctx = initialize_server_context();
	if ( !configure_server_context( ctx ) )
	{
		fprintf( stderr , "Failed to configure SSL context\n" );
		exit( EXIT_FAILURE );
	}

	// Create TCP socket
	server_sock = socket( AF_INET , SOCK_STREAM , 0 );
	if ( server_sock < 0 )
	{
		perror( "Socket creation failed" );
		exit( EXIT_FAILURE );
	}

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons( PORT );

	// Bind socket
	if ( bind( server_sock , ( struct sockaddr * )&addr , sizeof( addr ) ) < 0 )
	{
		perror( "Bind failed" );
		close( server_sock );
		exit( EXIT_FAILURE );
	}

	// Listen for incoming connections
	if ( listen( server_sock , 1 ) < 0 )
	{
		perror( "Listen failed" );
		close( server_sock );
		exit( EXIT_FAILURE );
	}

	printf( "SSL server listening on port %d...\n" , PORT );

	// Accept incoming connections
	client_sock = accept( server_sock , ( struct sockaddr * )&addr , &addr_len );
	if ( client_sock < 0 )
	{
		perror( "Accept failed" );
		close( server_sock );
		exit( EXIT_FAILURE );
	}

	// Create SSL object and attach it to the client socket
	ssl = SSL_new( ctx );
	SSL_set_fd( ssl , client_sock );

	// Perform SSL handshake
	if ( SSL_accept( ssl ) <= 0 )
	{
		ERR_print_errors_fp( stderr );
	}
	else
	{
		printf( "SSL handshake successful\n" );

		// Handle communication with the client
		handle_client( ssl );
	}

	// Clean up
	SSL_shutdown( ssl );
	SSL_free( ssl );
	close( client_sock );
	close( server_sock );
	SSL_CTX_free( ctx );

	return 0;
}
