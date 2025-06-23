#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stddef.h>

#include "json.h"

#pragma GCC diagnostic ignored "-Wsign-conversion"

const char * read_file( const char * path )
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
	char * buffer = malloc( len + 1 );

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

const char * const newstr( const char * const pchar )
{
	size_t len = strlen( pchar );
	char * ar = malloc( len + 1 );
	return strcpy( ar , pchar );
}

int main()
{
	struct sConfig_ver cfg_ver = { 0 };
	struct sU2T_config U2T_config = { 0 };

	{
		const char * config_ver_file_content = read_file( "/home/mohsen/workplace/config/U2T/config_ver.txt" );
		if ( config_ver_file_content == NULL )
		{
			return -1;
		}
		result( json_element ) rs_config_ver = json_parse( config_ver_file_content );
		free( ( void * )config_ver_file_content );
		if ( catch_error( &rs_config_ver , "config_ver" ) ) return -1;
		typed( json_element ) el_config_ver = result_unwrap( json_element )( &rs_config_ver );

		result( json_element ) ver = json_object_find( el_config_ver.value.as_object , "ver" );
		if ( catch_error( &ver , "ver" ) ) return -1;
		cfg_ver.pVersion = newstr( ( char * )ver.inner.value.value.as_string );
		char * perr = NULL;
		char * pver = ( char * )ver.inner.value.value.as_string;
		cfg_ver.Major = ( int )strtol( strtok( pver , "." ) , &perr , 10 ); if ( !perr ) return -1;
		cfg_ver.Minor = ( int )strtol( strtok( NULL , "." ) , &perr, 10 ); if ( !perr ) return -1;
		cfg_ver.Build = ( int )strtol( strtok( NULL , "." ) , &perr, 10 ); if ( !perr ) return -1;
		cfg_ver.Revision_Patch = ( int )strtol( strtok( NULL , "." ) , &perr , 10 ); if ( !perr ) return -1;

		json_free( &el_config_ver );
	}
	cfg_ver.Revision_Patch++; // just to ignore warning unused

	struct sU2T_config_0 * pGeneralConfiguration = ( struct sU2T_config_0 * )&U2T_config;
	struct sU2T_Tunnel * pTunnel = NULL;
	{
		const char * U2T_config_file_content = read_file( "/home/mohsen/workplace/config/U2T/U2T_config.txt" );
		if ( U2T_config_file_content == NULL )
		{
			return -1;
		}
		result( json_element ) rs_U2T_config = json_parse( U2T_config_file_content );
		free( ( void * )U2T_config_file_content );
		if ( catch_error( &rs_U2T_config , "U2T_config" ) ) return -1;
		typed( json_element ) el_U2T_config = result_unwrap( json_element )( &rs_U2T_config );

		if ( cfg_ver.Major >= 1 )
		{
			result( json_element ) re_configurations = json_object_find( el_U2T_config.value.as_object , "configurations" );
			if ( catch_error( &re_configurations , "configurations" ) ) return -1;
			typed( json_element ) el_configurations = result_unwrap( json_element )( &re_configurations );


			//result( json_element ) re_create_date = json_object_find( el_configurations.value.as_object , "create_date" );
			//if ( catch_error( &re_create_date , "create_date" ) ) return -1;
			//typed( json_element ) el_create_date = result_unwrap( json_element )( &re_create_date );
			//pGeneralConfiguration->create_date = newstr(el_create_date.value.as_string);

			//result( json_element ) re_config_tags = json_object_find( el_configurations.value.as_object , "config_tags" );
			//if ( catch_error( &re_config_tags , "config_tags" ) ) return -1;
			//typed( json_element ) el_config_tags = result_unwrap( json_element )( &re_config_tags );
			//pGeneralConfiguration->config_tags = newstr(el_config_tags.value.as_string);


			#define CFG_ELEM_STR( name ) \
				result( json_element ) re_##name = json_object_find( el_configurations.value.as_object , #name );\
				if ( catch_error( &re_##name , #name ) ) return -1;\
				typed( json_element ) el_##name = result_unwrap( json_element )( &re_##name );\
				pGeneralConfiguration->name = newstr(el_##name.value.as_string);
			#define CFG_ELEM_I( name ) \
				result( json_element ) re_##name = json_object_find( el_configurations.value.as_object , #name );\
				if ( catch_error( &re_##name , #name ) ) return -1;\
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
			if ( catch_error( &re_tunnels , "tunnels_obj" ) ) return -1;
			typed( json_element ) el_tunnels = result_unwrap( json_element )( &re_tunnels );

			if ( el_tunnels.value.as_object->count < 1 ) return -1;

			if ( !( pTunnel = ( struct sU2T_Tunnel * )malloc( sizeof( struct sU2T_Tunnel ) * el_tunnels.value.as_object->count ) ) )
			{
				return -1; // insufficient memory
			}

			for ( int i = 0 ; i < el_tunnels.value.as_object->count ; i++ )
			{
				char tunnel_name[50] = "tunnel";
				sprintf( tunnel_name , "tunnel%d" , i + 1 );

				result( json_element ) re_tunnel = json_object_find( el_tunnels.value.as_object , tunnel_name );
				if ( catch_error( &re_tunnel , tunnel_name ) ) return -1;
				typed( json_element ) el_tunnel = result_unwrap( json_element )( &re_tunnel );

				#define CFG_ELEM_STR( name ) \
					result( json_element ) re_##name = json_object_find( el_tunnel.value.as_object , #name );\
					if ( catch_error( &re_##name , #name ) ) return -1;\
					typed( json_element ) el_##name = result_unwrap( json_element )( &re_##name );\
					((struct sU2T_Tunnel_0 *)(pTunnel + i))->name = newstr(el_##name.value.as_string);
				
				#define CFG_ELEM_I( name ) \
					result( json_element ) re_##name = json_object_find( el_tunnel.value.as_object , #name );\
					if ( catch_error( &re_##name , #name ) ) return -1;\
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

		//json_print( &U2T_config , 1 );

		json_free( &el_U2T_config );
	}

	return 0;
}
