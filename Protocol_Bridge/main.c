#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>
#include <stddef.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/types.h>
#include <time.h>
#include "json.h"
#include "IntellisenseAssist.h" // should be at the end of any include to correct Intellisense bug

#ifndef definitions_section

//#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wunused-variable"

#define BUFFER_SIZE 9000

#define EOS '\0'

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

#define FUNCTION_SCOPE_INIT() char __custom_message[ 256 ] = "";
static char __custom_message[ 256 ] = ""; // if forgot to define __custom_message then global is choosed
#define _MSG(s) __msg(__custom_message,sizeof(__custom_message),s,__LINE__)

#define _DETAIL_ERROR( user_friendly_msg ) do { perror(_MSG(user_friendly_msg)); perror( __snprintf( __custom_message , sizeof(__custom_message) , "more details: %s(#%d)@ln(%d)\n" , strerror(errno), errno , __LINE__ ) ); } while(0);

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
	char * buffer = pInBuffer ? pInBuffer : malloc( (size_t)(len + 1) );

	if ( buffer == NULL )
	{
		fprintf( stderr , "Unable to allocate memory for file" );
		fclose( file );
		return NULL;
	}

	fread( buffer , 1 , (size_t)len , file );
	buffer[ len ] = EOS;

	fclose( file );
	file = NULL;

	return ( const char * )buffer;
}

const char * const newstr( const char * const pchar )
{
	size_t len = strlen( pchar );
	char * ar = malloc( len + 1 );
	return strcpy( ar , pchar );
}

#endif

#ifndef struct_section

struct UDP_connection_data
{
	int udp_port_number;
	int udp_sockfd;
	int udp_connection_established; // udp socket established
};

// TODO . IPv6
struct TCP_connection_data
{
	int tcp_port_number;
	char tcp_ip[64];
	int tcp_sockfd;
	int tcp_connection_established; // tcp connection established
};

struct Bridge_holder
{
	struct UDP_connection_data udp_data;
	struct TCP_connection_data tcp_data;
	size_t counterpart_connection_establishment_count; // 2 means both side socket connected
};

struct Bridges_holder
{
	struct Bridge_holder * pbs;
	size_t pbs_count;

	size_t pb_connection_establishment_count; // each protocol_bridge established
};

struct Config_ver
{
	char version[64];
	int Major; //Indicates significant changes , potentially including incompatible API changes.
	int Minor; //Denotes new functionality added in a backwards - compatible manner.
	int Build; //Represents the specific build number of the software.
	int Revision_Patch; //Represents backwards - compatible bug fixes or minor updates.
};

struct Global_Config_0
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

struct Global_Config_n
{
	struct Global_Config_0 c; // first member , n - 1
};

struct Global_Config // finalizer
{
	struct Global_Config_n c; // first member
	// ...
};

struct Protocol_Bridge_CFG_0
{
	char UDP_origin_ip[64];
	int UDP_origin_port;
	char TCP_destination_ip[64];
	int TCP_destination_port;
	int enable;
	int reset_connections;

	int checked_off; // if removed from a list after action
};

struct Protocol_Bridge_CFG_n
{
	struct Protocol_Bridge_CFG_0 c; // first member , n - 1
};

struct Protocol_Bridge_CFG // finalizer
{
	struct Protocol_Bridge_CFG_n c; // first member
	// ...
};

struct App_Config // global config
{
	struct Config_ver ___temp_ver; // not usable just to prevent reallocation
	struct Config_ver * _ver; // app version
	int _version_changed;

	struct Global_Config * _prev_general_config;
	struct Global_Config * _general_config;
	int _general_config_changed;

	struct Protocol_Bridge_CFG * _prev_cfg_pbs;
	size_t _prev_cfg_pbs_count;
	struct Protocol_Bridge_CFG * _cfg_pbs;
	size_t _cfg_pbs_count;
};

struct App_Data
{
	struct App_Config cfg;
	struct Bridges_holder bridges;
};

#endif

#ifndef connection_section

void * thread_udp_connection_proc( void * app_data )
{
	FUNCTION_SCOPE_INIT();
	struct App_Data * _g = ( struct App_Data * )app_data;

	printf( _MSG( "try to connect inbound udp connection" ) );

	while ( 1 )
	{
		int any_udp_to_connect = 0;
		for ( int i = 0 ; i < _g->bridges.pbs_count ; i++ )
		{
			if ( !_g->bridges.pbs[ i ].udp_data.udp_connection_established )
			{
				any_udp_to_connect++;
				break;
			}
		}
		if ( !any_udp_to_connect ) break;

		for ( int i = 0 ; i < _g->bridges.pbs_count ; i++ )
		{
			if ( !_g->bridges.pbs[ i ].udp_data.udp_connection_established )
			{
				if ( ( _g->bridges.pbs[ i ].udp_data.udp_sockfd = socket( AF_INET , SOCK_DGRAM , 0 ) ) < 0 )
				{
					_DETAIL_ERROR( "socket creation failed" );
					pthread_exit( ( void * )BAD_RETURN );
					return NULL;
				}

				struct sockaddr_in server_addr;
				memset( &server_addr , 0 , sizeof( server_addr ) );
				server_addr.sin_family = AF_INET; // IPv4
				server_addr.sin_port = htons( ( uint16_t )_g->bridges.pbs[ i ].udp_data.udp_port_number ); // Convert port to network byte order
				//server_addr.sin_addr.s_addr = inet_addr("YOUR_IP_ADDRESS"); // Specify the IP address to bind to
				server_addr.sin_addr.s_addr = INADDR_ANY; // Or use INADDR_ANY to bind to all available interfaces:

				if ( bind( _g->bridges.pbs[ i ].udp_data.udp_sockfd , ( const struct sockaddr * )&server_addr , sizeof( server_addr ) ) < 0 )
				{
					_DETAIL_ERROR( "bind failed" );
					_close_socket( &_g->bridges.pbs[ i ].udp_data.udp_sockfd );
					pthread_exit( ( void * )BAD_RETURN );
					return NULL;
				}

				_g->bridges.pbs[ i ].udp_data.udp_connection_established++;
				_g->bridges.pbs[ i ].counterpart_connection_establishment_count++; // one side connection established
				printf( _MSG( "inbound udp connected" ) );
			}
		}
		sleep( 1 );
	}

	return NULL; // Threads can return a value, but this example returns NULL
}

int _connect_tcp( struct TCP_connection_data * pTCP )
{
	FUNCTION_SCOPE_INIT();
	while ( 1 )
	{
		// try to create TCP socket
		if ( ( pTCP->tcp_sockfd = socket( AF_INET , SOCK_STREAM , 0 ) ) == -1 )
		{
			_DETAIL_ERROR( "Error creating TCP socket" );
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

void * thread_tcp_connection_proc( void * app_data )
{
	FUNCTION_SCOPE_INIT();
	struct App_Data * _g = ( struct App_Data * )app_data;

	printf( _MSG( "try to connect outbound tcp connection" ) );

	while ( 1 )
	{
		int any_tcp_to_connect = 0;
		for ( int i = 0 ; i < _g->bridges.pbs_count ; i++ )
		{
			if ( !_g->bridges.pbs[ i ].tcp_data.tcp_connection_established )
			{
				any_tcp_to_connect++;
				break;
			}
		}
		if ( !any_tcp_to_connect ) break;

		for ( int i = 0 ; i < _g->bridges.pbs_count ; i++ )
		{
			if ( !_g->bridges.pbs[ i ].tcp_data.tcp_connection_established )
			{
				if ( _connect_tcp( &( _g->bridges.pbs[ i ].tcp_data ) ) == 0 )
				{
					if
					(
						_g->bridges.pbs[ i ].udp_data.udp_connection_established ||
						_g->bridges.pbs[ i ].tcp_data.tcp_connection_established
					)
					{
						// both side connection established
						_g->bridges.pbs[ i ].counterpart_connection_establishment_count++;
						_g->bridges.pb_connection_establishment_count++;
					}

					continue;
				}
			}
		}
	}

	return NULL; // Threads can return a value, but this example returns NULL
}

void * thread_protocol_bridge_proc( void * app_data )
{
	FUNCTION_SCOPE_INIT();
	struct App_Data * _g = ( struct App_Data * )app_data;

	while ( !_g->bridges.pb_connection_establishment_count )
	{
		sleep( 1 );
	}

	//printf( "udp to tcp protocol_bridge stablished\n" );

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
		for ( int i = 0 ; i < _g->bridges.pb_connection_establishment_count ; i++ )
		{
			if ( _g->bridges.pbs[ i ].counterpart_connection_establishment_count )
			{
				// Add socket to set
				FD_SET( _g->bridges.pbs[ i ].udp_data.udp_sockfd , &readfds );
				if ( _g->bridges.pbs[ i ].udp_data.udp_sockfd > sockfd_max )
				{
					sockfd_max = _g->bridges.pbs[ i ].udp_data.udp_sockfd;
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

		for ( int i = 0 ; i < _g->bridges.pb_connection_establishment_count ; i++ )
		{
			if ( _g->bridges.pbs[ i ].udp_data.udp_connection_established )
			{
				if ( FD_ISSET( _g->bridges.pbs[ i ].udp_data.udp_sockfd , &readfds ) )
				{
					// good for udp data recieve
					bytes_received = recvfrom( _g->bridges.pbs[ i ].udp_data.udp_sockfd , buffer , BUFFER_SIZE , MSG_WAITALL ,
											   ( struct sockaddr * )&client_addr , &client_len );
					if ( bytes_received < 0 )
					{
						_DETAIL_ERROR( "Error receiving UDP packet" );
						continue;
					}
					buffer[ bytes_received ] = '\0'; // Null-terminate the received data

					// Send data over TCP
					if ( send( _g->bridges.pbs[ i ].tcp_data.tcp_sockfd , buffer , ( size_t )bytes_received , 0 ) == -1 )
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

// TODO . exit gracefully by auto mechanism
// TODO . think about race condition
void * version_checker( void * app_data )
{
	FUNCTION_SCOPE_INIT();
	struct App_Data * _g = ( struct App_Data * )app_data;
	char buf[ 50 ] = { 0 };
	time_t prev_time = {0} , cur_time = { 0 };
	struct Config_ver temp_ver = { 0 };
	while ( 1 )
	{
		cur_time = time( NULL );

		if ( _g->cfg._ver == NULL || difftime( cur_time , prev_time ) > 5 )
		{
			prev_time = cur_time;

			memset( buf , 0 , sizeof(buf) );
			const char * config_ver_file_content = read_file( "/home/mohsen/workplace/config/Protocol_Bridge/config_ver.txt" , ( char * )buf );
			if ( config_ver_file_content == NULL ) ERR_RET( "cannot open and read version file" , VOID_RET );

			result( json_element ) rs_config_ver = json_parse( config_ver_file_content );
			//free( ( void * )config_ver_file_content );
			if ( catch_error( &rs_config_ver , "config_ver" ) ) ERR_RET( "error in json_parse version file" , VOID_RET );
			typed( json_element ) el_config_ver = result_unwrap( json_element )( &rs_config_ver );

			result( json_element ) ver = json_object_find( el_config_ver.value.as_object , "ver" );
			if ( catch_error( &ver , "ver" ) ) ERR_RET( "ver not found" , VOID_RET );

			memset( &temp_ver , 0 , sizeof( temp_ver ) );

			strcpy( temp_ver.version , ( char * )ver.inner.value.value.as_string );
			char * perr = NULL;
			char * pver = ( char * )ver.inner.value.value.as_string;
			temp_ver.Major = ( int )strtol( strtok( pver , "." ) , &perr , 10 ); if ( !perr ) ERR_RET( "ver Major wrong" , VOID_RET );
			temp_ver.Minor = ( int )strtol( strtok( NULL , "." ) , &perr , 10 ); if ( !perr ) ERR_RET( "ver Minor wrong" , VOID_RET );
			temp_ver.Build = ( int )strtol( strtok( NULL , "." ) , &perr , 10 ); if ( !perr ) ERR_RET( "ver Build wrong" , VOID_RET );
			temp_ver.Revision_Patch = ( int )strtol( strtok( NULL , "." ) , &perr , 10 ); if ( !perr ) ERR_RET( "ver Revision_Patch wrong" , VOID_RET );
			json_free( &el_config_ver ); // string is user so must be free at the end

			if ( _g->cfg._ver == NULL || strcmp( temp_ver.version , _g->cfg._ver->version ) )
			{
				_g->cfg._ver = ( struct Config_ver * )memcpy( &_g->cfg.___temp_ver , &temp_ver , sizeof( temp_ver ) );
				_g->cfg._version_changed = 1;
			}
		}
		sleep( 1 );
	}
	return NULL;
}

// TODO . fix memory leak
// TODO . echo acceptible config one time to inform user
void * config_loader( void * app_data )
{
	FUNCTION_SCOPE_INIT();
	struct App_Data * _g = ( struct App_Data * )app_data;
	time_t prev_time , cur_time;

	while ( !_g->cfg._version_changed ) // load after version loaded
	{
		sleep( 1 );
	}

	while ( 1 )
	{
		cur_time = time( NULL );

		if ( _g->cfg._general_config == NULL || _g->cfg._version_changed || difftime(cur_time , prev_time) > 15 * 60 )
		{
			prev_time = cur_time;

			struct Global_Config temp_config = { 0 };
			struct Global_Config_0 * pGeneralConfiguration = ( struct Global_Config_0 * )&temp_config;
			struct Protocol_Bridge_CFG * pPbs = NULL;
			size_t pbs_count = 0;
			{
				const char * Protocol_Bridge_config_file_content = read_file( "/home/mohsen/workplace/config/Protocol_Bridge/Protocol_Bridge_config.txt" , NULL );
				if ( Protocol_Bridge_config_file_content == NULL )
				{
					ERR_RET( "cannot open config file" , VOID_RET );
				}
				result( json_element ) rs_Protocol_Bridge_config = json_parse( Protocol_Bridge_config_file_content );
				free( ( void * )Protocol_Bridge_config_file_content );
				if ( catch_error( &rs_Protocol_Bridge_config , "Protocol_Bridge_config" ) ) ERR_RET( "cannot parse config file" , VOID_RET );
				typed( json_element ) el_Protocol_Bridge_config = result_unwrap( json_element )( &rs_Protocol_Bridge_config );

				/*configurations*/
				if ( _g->cfg._ver->Major >= 1 ) // first version of config file structure
				{
					result( json_element ) re_configurations = json_object_find( el_Protocol_Bridge_config.value.as_object , "configurations" );
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

				/*Protocol_Bridges*/
				{
					result( json_element ) re_protocol_bridges = json_object_find( el_Protocol_Bridge_config.value.as_object , "Protocol_Bridges" );
					if ( catch_error( &re_protocol_bridges , "Protocol_Bridges" ) ) ERR_RET( "err" , VOID_RET );
					typed( json_element ) el_protocol_bridges = result_unwrap( json_element )( &re_protocol_bridges );

					if ( ( pbs_count = el_protocol_bridges.value.as_object->count ) < 1 ) ERR_RET( "err" , VOID_RET );

					if ( !( pPbs = ( struct Protocol_Bridge_CFG * )malloc( sizeof( struct Protocol_Bridge_CFG ) * el_protocol_bridges.value.as_object->count ) ) )
					{
						ERR_RET( "insufficient memory" , VOID_RET );
					}

					for ( int i = 0 ; i < el_protocol_bridges.value.as_object->count ; i++ )
					{
						char protocol_bridges_name[32];
						sprintf( protocol_bridges_name , "bridge%d" , i + 1 );

						result( json_element ) re_protocol_bridge = json_object_find( el_protocol_bridges.value.as_object , protocol_bridges_name );
						if ( catch_error( &re_protocol_bridge , protocol_bridges_name ) ) ERR_RET( "err" , VOID_RET );
						typed( json_element ) el_protocol_bridge = result_unwrap( json_element )( &re_protocol_bridge );

						#define CFG_ELEM_STR( name ) \
							result( json_element ) re_##name = json_object_find( el_protocol_bridge.value.as_object , #name );\
							if ( catch_error( &re_##name , #name ) ) ERR_RET( "err" , VOID_RET );\
							typed( json_element ) el_##name = result_unwrap( json_element )( &re_##name );\
							strcpy(((struct Protocol_Bridge_CFG_0 *)(pPbs + i))->name , el_##name.value.as_string );
				
						#define CFG_ELEM_I( name ) \
							result( json_element ) re_##name = json_object_find( el_protocol_bridge.value.as_object , #name );\
							if ( catch_error( &re_##name , #name ) ) ERR_RET( "err" , VOID_RET );\
							typed( json_element ) el_##name = result_unwrap( json_element )( &re_##name );\
							((struct Protocol_Bridge_CFG_0 *)(pPbs + i))->name = (int)el_##name.value.as_number.value.as_long;

						CFG_ELEM_STR( UDP_origin_ip );
						CFG_ELEM_I( UDP_origin_port );
						CFG_ELEM_STR( TCP_destination_ip );
						CFG_ELEM_I( TCP_destination_port );
						CFG_ELEM_I( enable );
						CFG_ELEM_I( reset_connections );

						#undef CFG_ELEM_I
						#undef CFG_ELEM_STR
					}
				}

				json_free( &el_Protocol_Bridge_config );
			}
		
			if ( _g->cfg._general_config == NULL ) // TODO . make assignemnt atomic
			{
				if ( !( _g->cfg._general_config = malloc( sizeof( struct Global_Config ) ) ) )
				{
					ERR_RET( "insufficient memory" , VOID_RET );
				}
				memset( _g->cfg._general_config , 0 , sizeof( struct Global_Config ) );
				memcpy( _g->cfg._general_config , &temp_config , sizeof( temp_config ) );
				memset( &temp_config , 0 , sizeof( temp_config ) ); // copy to global variable and then zero to not free strings
			}

			if ( !_g->cfg._cfg_pbs )
			{
				_g->cfg._prev_cfg_pbs = _g->cfg._cfg_pbs;
				_g->cfg._prev_cfg_pbs_count = _g->cfg._cfg_pbs_count;
			}
			_g->cfg._cfg_pbs = pPbs;
			_g->cfg._cfg_pbs_count = pbs_count;
			pPbs = NULL; // to not delete intentionally
			pbs_count = 0;
		}
		sleep( 2 );
	}
	return NULL;
}

void * protocol_bridge_manager( void * app_data )
{
	FUNCTION_SCOPE_INIT();
	struct App_Data * _g = ( struct App_Data * )app_data;

	pthread_t trd_udp_connection , trd_tcp_connection;
	pthread_t trd_protocol_bridge;

	while ( !_g->cfg._cfg_pbs_count ) // load after any config loaded
	{
		sleep( 1 );
	}

	while ( 1 )
	{
		if ( !_g->cfg._prev_cfg_pbs && !_g->bridges.pbs_count && _g->cfg._cfg_pbs_count )
		{
			if ( !( _g->bridges.pbs = ( struct Bridge_holder * )malloc( sizeof( struct Bridge_holder ) * _g->cfg._cfg_pbs_count ) ) )
			{
				ERR_RET( "insufficient memory" , VOID_RET );
			}
			memset( _g->bridges.pbs , 0 , sizeof( struct Bridge_holder ) * _g->cfg._cfg_pbs_count );
			_g->bridges.pbs_count = _g->cfg._cfg_pbs_count;
			for ( int i = 0 ; i < _g->bridges.pbs_count ; i++ )
			{
				_g->bridges.pbs[ i ].udp_data.udp_port_number = _g->cfg._cfg_pbs[ i ].c.c.UDP_origin_port;
				strcpy( _g->bridges.pbs[ i ].tcp_data.tcp_ip , _g->cfg._cfg_pbs[ i ].c.c.TCP_destination_ip );
				_g->bridges.pbs[ i ].tcp_data.tcp_port_number = _g->cfg._cfg_pbs[ i ].c.c.TCP_destination_port;
			}

			if ( pthread_create( &trd_udp_connection , NULL , thread_udp_connection_proc , app_data ) != 0 )
			{
				_DETAIL_ERROR( "Failed to create thread" );
				return NULL; // Indicate an error
			}

			if ( pthread_create( &trd_tcp_connection , NULL , thread_tcp_connection_proc , app_data ) != 0 )
			{
				_DETAIL_ERROR( "Failed to create thread" );
				return NULL; // Indicate an error
			}

			if ( pthread_create( &trd_protocol_bridge , NULL , thread_protocol_bridge_proc , app_data ) != 0 )
			{
				_DETAIL_ERROR( "Failed to create thread" );
				return NULL; // Indicate an error
			}
		}
		else if ( _g->cfg._prev_cfg_pbs_count != _g->cfg._cfg_pbs_count )
		{
			// TODO . release unused protocol Bridge
		}
		sleep( 5 );
	}
	return NULL;
}

int main()
{
	FUNCTION_SCOPE_INIT();
	struct App_Data _g = { 0 };

	pthread_t trd_version_checker;
	if ( pthread_create( &trd_version_checker , NULL , version_checker , ( void * )&_g ) != 0 ) ERR_RET( "Failed to create thread" , MAIN_BAD_RET );

	pthread_t trd_config_loader;
	if ( pthread_create( &trd_config_loader , NULL , config_loader , ( void * )&_g ) != 0 ) ERR_RET( "Failed to create thread" , MAIN_BAD_RET );

	pthread_t trd_protocol_bridge_manager;
	if ( pthread_create( &trd_protocol_bridge_manager , NULL , protocol_bridge_manager , ( void * )&_g ) != 0 ) ERR_RET( "Failed to create thread" , MAIN_BAD_RET );

	if ( pthread_join( trd_protocol_bridge_manager , NULL ) != 0 )
	{
		_DETAIL_ERROR( "Failed to join thread" );
		return 1; // Indicate an error
	}

	printf( _MSG( "Main thread finished waiting for the new thread." ) );

	return 0;
}

