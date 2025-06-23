#include <pthread.h> // Required for Pthreads functions
#include <stdio.h>   // Required for printf
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h> // For close()
#include <errno.h> // For errno
#include <stdarg.h>
#include <stddef.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/types.h>          /* See NOTES */
#include <time.h>

#include "json.h"

#ifndef definitions_macro_section

//#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wunused-variable"

#define BUFFER_SIZE 9000

//#define TUNNEL_COUNT 2

//#define UDP_PORT_1 1234
//#define UDP_PORT_2 1235
//
//#define TCP_PORT_1 4321
//#define TCP_SERVER_IP_1 "192.168.1.60"
//
//#define TCP_PORT_2 4322
//#define TCP_SERVER_IP_2 "192.168.1.60"

#endif

#ifndef utils_section

const char * __msg( char * msg_holder , size_t size_of_msg_holder , const char * msg , int line_number )
{
	snprintf( msg_holder , size_of_msg_holder , "%s: ln(%d)\n" , msg , line_number );
	return msg_holder;
}

const char * __snprintf( char * msg_holder , size_t size_of_msg_holder , const char * format , ... )
{
	va_list args;

	va_start( args , format );
	vsnprintf( msg_holder , size_of_msg_holder , format , args );
	va_end( args );

	return msg_holder;
}

#define _MSG(s) __msg(custom_message,sizeof(custom_message),s,__LINE__)

#define _DETAIL_ERROR( user_friendly_msg ) do { perror(_MSG(user_friendly_msg)); perror( __snprintf( custom_message , sizeof(custom_message) , "more details: %s(#%d)@ln(%d)\n" , strerror(errno), errno , __LINE__ ) ); } while(0);

#define VOID_RET ((void*)NULL)
#define MAIN_BAD_RET (1/*Indicate an error*/)

#define ERR_RET( user_friendly_msg , RET ) \
	do {\
	_DETAIL_ERROR( user_friendly_msg );\
	return RET; } while(0);

#define BAD_RETURN "bad"

void _close_socket( int * socket_id )
{
	close( *socket_id );
	*socket_id = -1;
}

const char * read_file( const char * path , char * pInBuffer /*= NULL*/ )
{
	FILE * file = fopen( path , "r" );
	if ( file == NULL )
	{
		fprintf( stderr , "Expected file \"%s\" not found" , path );
		return NULL;
	}
	fseek( file , 0 , SEEK_END );
	long len = ftell( file );
	fseek( file , 0 , SEEK_SET );
	char * buffer = pInBuffer ? pInBuffer : malloc( len + 1 );

	if ( buffer == NULL )
	{
		fprintf( stderr , "Unable to allocate memory for file" );
		fclose( file );
		return NULL;
	}

	fread( buffer , 1 , len , file );
	buffer[ len ] = '\0';

	fclose( file );
	file = NULL;

	return ( const char * )buffer;
}

#endif

#ifndef network_struct_section

struct udp_connection_data
{
	int udp_port_number;
	int udp_sockfd;
	int udp_connection_established; // udp socket established
};

struct tcp_connection_data
{
	int tcp_port_number;
	const char * tcp_ip;
	int tcp_sockfd;
	int tcp_connection_established; // tcp connection established
};

struct tunnel_data
{
	struct udp_connection_data * p_udp_data;
	struct tcp_connection_data * p_tcp_data;
	int counterpart_connection_establishment_count; // 2 means both side socket connected
};

struct tunnels_data
{
	struct tunnel_data * tunnels;
	int udp_connection_count;
	int tcp_connection_count;
	int tunnel_connection_establishment_count; // each tunnel established
};

#endif

#ifndef connection_section

void * thread_udp_connection_proc( void * arg )
{
	char custom_message[ 256 ];

	struct tunnels_data * pTunnels_data = ( struct tunnels_data * )arg;

	printf( _MSG( "try to connect inbound udp connection" ) );

	while ( pTunnels_data->udp_connection_count < TUNNEL_COUNT )
	{
		if ( ( pTunnels_data->tunnels_data[ pTunnels_data->udp_connection_count ].p_udp_data->udp_sockfd = socket( AF_INET , SOCK_DGRAM , 0 ) ) < 0 )
		{
			_DETAIL_ERROR( "socket creation failed" );
			pthread_exit( ( void * )BAD_RETURN );
			return NULL;
		}

		struct sockaddr_in server_addr;

		memset( &server_addr , 0 , sizeof( server_addr ) ); // Clear the structure

		server_addr.sin_family = AF_INET; // IPv4
		server_addr.sin_port = htons( ( uint16_t )pTunnels_data->tunnels_data[ pTunnels_data->udp_connection_count ].p_udp_data->udp_port_number ); // Convert port to network byte order
		//server_addr.sin_addr.s_addr = inet_addr("YOUR_IP_ADDRESS"); // Specify the IP address to bind to
		// Or use INADDR_ANY to bind to all available interfaces:
		server_addr.sin_addr.s_addr = INADDR_ANY;

		if ( bind( pTunnels_data->tunnels_data[ pTunnels_data->udp_connection_count ].p_udp_data->udp_sockfd , ( const struct sockaddr * )&server_addr , sizeof( server_addr ) ) < 0 )
		{
			_DETAIL_ERROR( "bind failed" );
			_close_socket( &pTunnels_data->tunnels_data[ pTunnels_data->udp_connection_count ].p_udp_data->udp_sockfd );

			//close( pTunnels_data->tunnels_data[pTunnels_data->udp_connection_count].p_udp_data->udp_sockfd );
			//pTunnels_data->tunnels_data[pTunnels_data->udp_connection_count].p_udp_data->udp_sockfd = -1;
			pthread_exit( ( void * )BAD_RETURN );
			return NULL;
		}

		pTunnels_data->tunnels_data[ pTunnels_data->udp_connection_count ].p_udp_data->udp_connection_established++;
		// one side connection established
		pTunnels_data->tunnels_data[ pTunnels_data->udp_connection_count ].counterpart_connection_establishment_count++;
		printf( _MSG( "inbound udp connected" ) );
		pTunnels_data->udp_connection_count++;
	}

	return NULL; // Threads can return a value, but this example returns NULL
}

int _connect_tcp( struct tcp_connection_data * pTCP )
{
	char custom_message[ 256 ];
	while ( 1 )
	{
		// try to create TCP socket
		if ( ( pTCP->tcp_sockfd = socket( AF_INET , SOCK_STREAM , 0 ) ) == -1 )
		{
			_DETAIL_ERROR( "Error creating TCP socket" );
			//pthread_exit((void *)BAD_RETURN);
			//return NULL;
			return -1;
		}

		struct sockaddr_in tcp_addr;

		// Connect to TCP server
		tcp_addr.sin_family = AF_INET;
		tcp_addr.sin_port = htons( ( uint16_t )pTCP->tcp_port_number );
		if ( inet_pton( AF_INET , pTCP->tcp_ip , &tcp_addr.sin_addr ) <= 0 )
		{
			_DETAIL_ERROR( "Invalid TCP address" );
			_close_socket( &pTCP->tcp_sockfd );
			//pthread_exit((void *)BAD_RETURN);
			//return NULL;
			return -1;
		}

		if ( connect( pTCP->tcp_sockfd , ( struct sockaddr * )&tcp_addr , sizeof( tcp_addr ) ) == -1 )
		{
			if ( errno == ECONNREFUSED || errno == ETIMEDOUT )
			{
				_close_socket( &pTCP->tcp_sockfd );
				sleep( 2 ); // sec
				continue;
			}

			_DETAIL_ERROR( "Error connecting to TCP server" );
			_close_socket( &pTCP->tcp_sockfd );
			//pthread_exit( ( void * )BAD_RETURN );
			//return NULL;
			return -1;
		}
		else
		{
			pTCP->tcp_connection_established = 1;
			printf( _MSG( "outbound tcp connected" ) );
			return 0;
		}
	}
	return -1;
}

void * thread_tcp_connection_proc( void * arg )
{
	char custom_message[ 256 ];

	struct tunnels_data * pTunnels_data = ( struct tunnels_data * )arg;

	printf( _MSG( "try to connect outbound tcp connection" ) );

	while ( pTunnels_data->tcp_connection_count < TUNNEL_COUNT )
	{
		if ( _connect_tcp( pTunnels_data->tunnels_data[ pTunnels_data->tcp_connection_count ].p_tcp_data ) == 0 )
		{
			if
				(
					pTunnels_data->tunnels_data[ pTunnels_data->tcp_connection_count ].p_udp_data->udp_connection_established ||
					pTunnels_data->tunnels_data[ pTunnels_data->tcp_connection_count ].p_tcp_data->tcp_connection_established
					)
			{
				// both side connection established
				pTunnels_data->tunnels_data[ pTunnels_data->tcp_connection_count ].counterpart_connection_establishment_count++;
				pTunnels_data->tunnel_connection_establishment_count++;
			}
			pTunnels_data->tcp_connection_count++;

			continue;
		}
		//pthread_exit((void *)BAD_RETURN);
		//return NULL;
	}

	return NULL; // Threads can return a value, but this example returns NULL
}

void * thread_tunnel_proc( void * arg )
{
	char custom_message[ 256 ];

	struct tunnels_data * pTunnels_data = ( struct tunnels_data * )arg;

	while ( !pTunnels_data->tunnel_connection_establishment_count )
	{
		sleep( 1 );
	}

	//printf( "udp to tcp tunnel stablished\n" );

	char buffer[ BUFFER_SIZE ]; // Define a buffer to store received data
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof( client_addr );
	ssize_t bytes_received;

	fd_set readfds; // Set of socket descriptors

	while ( 1 )
	{
		// Clear the socket set
		FD_ZERO( &readfds );

		int sockfd_max = -1; // for select compulsion
		for ( int i = 0 ; i < pTunnels_data->tunnel_connection_establishment_count ; i++ )
		{
			if ( pTunnels_data->tunnels_data[ i ].counterpart_connection_establishment_count )
			{
				// Add socket to set
				FD_SET( pTunnels_data->tunnels_data[ i ].p_udp_data->udp_sockfd , &readfds );
				if ( pTunnels_data->tunnels_data[ i ].p_udp_data->udp_sockfd > sockfd_max )
				{
					sockfd_max = pTunnels_data->tunnels_data[ i ].p_udp_data->udp_sockfd;
				}
			}
		}

		if ( sockfd_max <= 1 )
		{
			sleep( 1 );
			continue;
		}

		struct timeval timeout; // Set timeout (e.g., 5 seconds)
		timeout.tv_sec = 5;
		timeout.tv_usec = 0;

		// Wait for an activity on one of the sockets, timeout is NULL, so wait indefinitely
		int activity = select( sockfd_max + 1 , &readfds , NULL , NULL , &timeout );

		if ( ( activity < 0 ) && ( errno != EINTR ) )
		{
			_DETAIL_ERROR( "select error" );
			continue;
		}
		if ( activity == 0 )
		{
			//_DETAIL_ERROR("timed out");
			continue;
		}

		for ( int i = 0 ; i < pTunnels_data->tunnel_connection_establishment_count ; i++ )
		{
			if ( pTunnels_data->tunnels_data[ i ].p_udp_data->udp_connection_established )
			{
				if ( FD_ISSET( pTunnels_data->tunnels_data[ i ].p_udp_data->udp_sockfd , &readfds ) )
				{
					// good for udp data recieve
					bytes_received = recvfrom( pTunnels_data->tunnels_data[ i ].p_udp_data->udp_sockfd , buffer , BUFFER_SIZE , MSG_WAITALL ,
											   ( struct sockaddr * )&client_addr , &client_len );
					if ( bytes_received < 0 )
					{
						_DETAIL_ERROR( "Error receiving UDP packet" );
						continue;
					}
					buffer[ bytes_received ] = '\0'; // Null-terminate the received data

					// Send data over TCP
					if ( send( pTunnels_data->tunnels_data[ i ].p_tcp_data->tcp_sockfd , buffer , ( size_t )bytes_received , 0 ) == -1 )
					{
						_DETAIL_ERROR( "Error sending data over TCP" );
						continue;
					}

					printf( "Received from client: %s\n" , buffer );
					printf( "Forwarded %zd bytes from UDP to TCP.\n" , bytes_received );
				}
			}
			else
			{
				printf( "\n" );
			}
		}
	}
}

#endif

#ifndef config_section

struct sConfig_ver
{
	const char * pVersion;
	int Major; //Indicates significant changes , potentially including incompatible API changes.
	int Minor; //Denotes new functionality added in a backwards - compatible manner.
	int Build; //Represents the specific build number of the software.
	int Revision_Patch; //Represents backwards - compatible bug fixes or minor updates.
};

struct sU2T_config_0
{
	const char * create_date; // "2025-06-21 20:43:00"
	const char * modify_date;
	const char * config_name;
	const char * config_tags;
	const char * description;
	const char * log_level; // no , error , warn , verbose,
	const char * log_file;
	
	int enable;
	int shutdown;
	int watchdog_enabled;
	int load_prev_config;
	int dump_current_config;
	int dump_prev_config;
	int time_out_sec;
};

struct sU2T_config_n
{
	struct sU2T_config_0 U2T_config_0; // first member , n - 1
};

struct sU2T_config // finalizer
{
	struct sU2T_config_n U2T_config; // first member
	// ...
};

struct sU2T_Tunnel_0
{
	const char * UDP_origin_ip;
	int UDP_origin_port;
	const char * TCP_termination_ip;
	int TCP_termination_port;
	int enable;
	int reset_connections;
};

struct sU2T_Tunnel_n
{
	struct sU2T_Tunnel_0 U2T_tunnel_0; // first member , n - 1
};

struct sU2T_Tunnel // finalizer
{
	struct sU2T_Tunnel_n U2T_tunnel; // first member
	// ...
};

struct GVar
{
	struct sConfig_ver * _ver;
	int _version_changed;

	struct sU2T_config * _prev_config;
	struct sU2T_config * _config;
	int _config_changed;

	struct sU2T_Tunnel * _prev_tunnels;
	int _prev_tunnel_count;
	struct sU2T_Tunnel * _tunnels;
	int _tunnel_count;
};

#endif

// TODO . exit gracefully by auto mechanism
void * version_checker( void * global_var )
{
	char custom_message[ 256 ];
	struct GVar * _g = ( struct GVar * )global_var;
	char buf[ 50 ] = { 0 };
	struct time_t prev_time = {0} , cur_time = { 0 };
	struct sConfig_ver temp_ver = { 0 };
	while ( 1 )
	{
		cur_time = time( NULL );

		if ( _g->_ver == NULL || difftime( cur_time , prev_time ) > 5 )
		{
			prev_time = cur_time;

			memset( buf , 0 , sizeof(buf) );
			const char * config_ver_file_content = read_file( "/home/mohsen/workplace/config/U2T/config_ver.txt" , ( char * )buf );
			if ( config_ver_file_content == NULL ) ERR_RET( "cannot open and read version file" , VOID_RET );

			result( json_element ) rs_config_ver = json_parse( config_ver_file_content );
			//free( ( void * )config_ver_file_content );
			if ( catch_error( &rs_config_ver , "config_ver" ) ) ERR_RET( "error in json_parse version file" , VOID_RET );
			typed( json_element ) el_config_ver = result_unwrap( json_element )( &rs_config_ver );

			result( json_element ) ver = json_object_find( el_config_ver.value.as_object , "ver" );
			if ( catch_error( &ver , "ver" ) ) ERR_RET( "ver not found" , VOID_RET );

			memset( &temp_ver , 0 , sozeof( temp_ver ) );

			temp_ver.pVersion = ( char * )ver.inner.value.value.as_string;
			char * perr = NULL;
			char * pver = ( char * )ver.inner.value.value.as_string;
			temp_ver.Major = ( int )strtol( strtok( pver , "." ) , &perr , 10 ); if ( !perr ) ERR_RET( "ver Major wrong" , VOID_RET );
			temp_ver.Minor = ( int )strtol( strtok( NULL , "." ) , &perr , 10 ); if ( !perr ) ERR_RET( "ver Minor wrong" , VOID_RET );
			temp_ver.Build = ( int )strtol( strtok( NULL , "." ) , &perr , 10 ); if ( !perr ) ERR_RET( "ver Build wrong" , VOID_RET );
			temp_ver.Revision_Patch = ( int )strtol( strtok( NULL , "." ) , &perr , 10 ); if ( !perr ) ERR_RET( "ver Revision_Patch wrong" , VOID_RET );

			if ( _g->_ver == NULL || strcmp( temp_ver.pVersion , _g->_ver->pVersion ) )
			{
				if ( _g->_ver == NULL )
				{
					_g->_ver = malloc( sizeof( struct sConfig_ver ) );
					memset( _g->_ver , 0 , sizeof( struct sConfig_ver ) );
				}

				memcpy( _g->_ver , &temp_ver , sizeof( temp_ver ) );
				_g->_ver->pVersion = newstr( ( char * )ver.inner.value.value.as_string );
				_g->_version_changed = 1;
			}

			json_free( &el_config_ver ); // string is user so must be free at the end
		}
		sleep( 1 );
	}
	return NULL;
}

// TODO . fix memory leak
// TODO . echo acceptible config one time to inform user
void * config_loader( void * global_var )
{
	char custom_message[ 256 ];
	struct GVar * _g = ( struct GVar * )global_var;
	struct time_t prev_time = {0} , cur_time = { 0 };

	while ( !_g->_version_changed ) // load after version loaded
	{
		sleep( 1 );
	}

	while ( 1 )
	{
		cur_time = time( NULL );

		if ( _g->_config == NULL || _g->_version_changed /* || difftime(cur_time , prev_time) > 60*/ )
		{
			prev_time = cur_time;

			struct sU2T_config U2T_config = { 0 };
			struct sU2T_config_0 * pGeneralConfiguration = ( struct sU2T_config_0 * )&U2T_config;
			struct sU2T_Tunnel * pTunnel = NULL;
			int tunnel_count = 0;
			{
				const char * U2T_config_file_content = read_file( "/home/mohsen/workplace/config/U2T/U2T_config.txt" , NULL );
				if ( U2T_config_file_content == NULL )
				{
					ERR_RET( "cannot open config file" , VOID_RET );
				}
				result( json_element ) rs_U2T_config = json_parse( U2T_config_file_content );
				free( ( void * )U2T_config_file_content );
				if ( catch_error( &rs_U2T_config , "U2T_config" ) ) ERR_RET( "cannot parse config file" , VOID_RET );
				typed( json_element ) el_U2T_config = result_unwrap( json_element )( &rs_U2T_config );

				if ( _g->_ver->Major >= 1 ) // first version of config file structure
				{
					result( json_element ) re_configurations = json_object_find( el_U2T_config.value.as_object , "configurations" );
					if ( catch_error( &re_configurations , "configurations" ) ) ERR_RET( "err" , VOID_RET );
					typed( json_element ) el_configurations = result_unwrap( json_element )( &re_configurations );

					#define CFG_ELEM_STR( name ) \
						result( json_element ) re_##name = json_object_find( el_configurations.value.as_object , #name );\
						if ( catch_error( &re_##name , #name ) ) ERR_RET( "err" , VOID_RET );\
						typed( json_element ) el_##name = result_unwrap( json_element )( &re_##name );\
						pGeneralConfiguration->name = newstr(el_##name.value.as_string);
					#define CFG_ELEM_I( name ) \
						result( json_element ) re_##name = json_object_find( el_configurations.value.as_object , #name );\
						if ( catch_error( &re_##name , #name ) ) ERR_RET( "err" , VOID_RET );\
						typed( json_element ) el_##name = result_unwrap( json_element )( &re_##name );\
						pGeneralConfiguration->name = (int)el_##name.value.as_number.value.as_long;

					CFG_ELEM_STR( create_date );
					CFG_ELEM_STR( modify_date );
					CFG_ELEM_STR( config_name );
					CFG_ELEM_STR( config_tags );
					CFG_ELEM_STR( description );
					CFG_ELEM_STR( log_level );
					CFG_ELEM_STR( log_file );
					CFG_ELEM_I( enable );
					CFG_ELEM_I( shutdown );
					CFG_ELEM_I( watchdog_enabled );
					CFG_ELEM_I( load_prev_config );
					CFG_ELEM_I( dump_current_config );
					CFG_ELEM_I( dump_prev_config );
					CFG_ELEM_I( time_out_sec );

					#undef CFG_ELEM_I
					#undef CFG_ELEM_STR
				}

				{
					result( json_element ) re_tunnels = json_object_find( el_U2T_config.value.as_object , "tunnels" );
					if ( catch_error( &re_tunnels , "tunnels_obj" ) ) ERR_RET( "err" , VOID_RET );
					typed( json_element ) el_tunnels = result_unwrap( json_element )( &re_tunnels );

					if ( ( tunnel_count = el_tunnels.value.as_object->count ) < 1 ) ERR_RET( "err" , VOID_RET );

					if ( !( pTunnel = ( struct sU2T_Tunnel * )malloc( sizeof( struct sU2T_Tunnel ) * el_tunnels.value.as_object->count ) ) )
					{
						ERR_RET( "insufficient memory" , VOID_RET );
					}

					for ( int i = 0 ; i < el_tunnels.value.as_object->count ; i++ )
					{
						char tunnel_name[32] = "tunnel";
						sprintf( tunnel_name , "tunnel%d" , i + 1 );

						result( json_element ) re_tunnel = json_object_find( el_tunnels.value.as_object , tunnel_name );
						if ( catch_error( &re_tunnel , tunnel_name ) ) ERR_RET( "err" , VOID_RET );
						typed( json_element ) el_tunnel = result_unwrap( json_element )( &re_tunnel );

						#define CFG_ELEM_STR( name ) \
							result( json_element ) re_##name = json_object_find( el_tunnel.value.as_object , #name );\
							if ( catch_error( &re_##name , #name ) ) ERR_RET( "err" , VOID_RET );\
							typed( json_element ) el_##name = result_unwrap( json_element )( &re_##name );\
							((struct sU2T_Tunnel_0 *)(pTunnel + i))->name = newstr(el_##name.value.as_string);
				
						#define CFG_ELEM_I( name ) \
							result( json_element ) re_##name = json_object_find( el_tunnel.value.as_object , #name );\
							if ( catch_error( &re_##name , #name ) ) ERR_RET( "err" , VOID_RET );\
							typed( json_element ) el_##name = result_unwrap( json_element )( &re_##name );\
							((struct sU2T_Tunnel_0 *)(pTunnel + i))->name = (int)el_##name.value.as_number.value.as_long;

						CFG_ELEM_STR( UDP_origin_ip );
						CFG_ELEM_I( UDP_origin_port );
						CFG_ELEM_STR( TCP_termination_ip );
						CFG_ELEM_I( TCP_termination_port );
						CFG_ELEM_I( enable );
						CFG_ELEM_I( reset_connections );

						#undef CFG_ELEM_I
						#undef CFG_ELEM_STR
					}
				}

				json_free( &el_U2T_config );
			}
		
			if ( _g->_config == NULL ) // TODO . make assignemnt atomic
			{
				if ( !( _g->_config = malloc( sizeof( struct sU2T_config ) ) ) )
				{
					ERR_RET( "insufficient memory" , VOID_RET );
				}
				memset( _g->_config , 0 , sizeof( struct sU2T_config ) ) );
				memcpy( _g->_config , &U2T_config , sizeof( U2T_config ) );
				memset( &U2T_config , 0 , sizeof( U2T_config ) ); // copy to global variable and then zero to not free strings
			}

			if ( !_g->_tunnels )
			{
				_g->_prev_tunnels = _g->_tunnels;
				_g->_prev_tunnel_count = _g->_tunnel_count;
			}
			_g->_tunnels = pTunnel;
			_g->_tunnel_count = tunnel_count;
			pTunnel = NULL; // to not delete intentionally
			tunnel_count = 0;
		}
	}
	return NULL;
}

int main()
{
	char custom_message[ 256 ];
	struct GVar _g = { 0 };

	pthread_t trd_version_checker , trd_config_loader;

	if ( pthread_create( &trd_version_checker , NULL , version_checker , ( void * )&_g ) != 0 ) ERR_RET( "Failed to create thread" , MAIN_BAD_RET );

	if ( pthread_create( &trd_config_loader , NULL , config_loader , ( void * )&_g ) != 0 ) ERR_RET( "Failed to create thread" , MAIN_BAD_RET );

	pthread_t thread_udp_connection , thread_tcp_connection;
	pthread_t thread_tunnel;

	struct udp_connection_data udp_connection_datas[ TUNNEL_COUNT ];
	memset( udp_connection_datas , 0 , sizeof( udp_connection_datas ) );
	udp_connection_datas[ 0 ].udp_port_number = UDP_PORT_1;
	udp_connection_datas[ 1 ].udp_port_number = UDP_PORT_2;

	struct tcp_connection_data tcp_connection_datas[ TUNNEL_COUNT ];
	memset( tcp_connection_datas , 0 , sizeof( tcp_connection_datas ) );
	tcp_connection_datas[ 0 ].tcp_ip = TCP_SERVER_IP_1;
	tcp_connection_datas[ 0 ].tcp_port_number = TCP_PORT_1;
	tcp_connection_datas[ 1 ].tcp_ip = TCP_SERVER_IP_2;
	tcp_connection_datas[ 1 ].tcp_port_number = TCP_PORT_2;

	struct tunnels_data tunnels;
	memset( &tunnels , 0 , sizeof( tunnels ) );
	tunnels.tunnels_data[ 0 ].p_udp_data = &udp_connection_datas[ 0 ];
	tunnels.tunnels_data[ 0 ].p_tcp_data = &tcp_connection_datas[ 0 ];

	tunnels.tunnels_data[ 1 ].p_udp_data = &udp_connection_datas[ 1 ];
	tunnels.tunnels_data[ 1 ].p_tcp_data = &tcp_connection_datas[ 1 ];

	if ( pthread_create( &thread_udp_connection , NULL , thread_udp_connection_proc , ( void * )&tunnels ) != 0 )
	{
		_DETAIL_ERROR( "Failed to create thread" );
		return 1; // Indicate an error
	}

	if ( pthread_create( &thread_tcp_connection , NULL , thread_tcp_connection_proc , ( void * )&tunnels ) != 0 )
	{
		_DETAIL_ERROR( "Failed to create thread" );
		return 1; // Indicate an error
	}

	if ( pthread_create( &thread_tunnel , NULL , thread_tunnel_proc , ( void * )&tunnels ) != 0 )
	{
		_DETAIL_ERROR( "Failed to create thread" );
		return 1; // Indicate an error
	}

	if ( pthread_join( thread_tunnel , NULL ) != 0 )
	{
		_DETAIL_ERROR( "Failed to join thread" );
		return 1; // Indicate an error
	}

	//close(udp1_connection_data.sockfd);
	//close(tcp1_connection_data.sockfd);

	printf( _MSG( "Main thread finished waiting for the new thread." ) );

	return 0;
}

