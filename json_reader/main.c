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
	int Major; //Indicates significant changes , potentially including incompatible API changes.
	int Minor; //Denotes new functionality added in a backwards - compatible manner.
	int Build; //Represents the specific build number of the software.
	int Revision_Patch; //Represents backwards - compatible bug fixes or minor updates.
};


struct sU2T_config_0
{
	const char * d_create_date; // "2025-06-21 20:43:00"
	const char * d_modify_date;
	const char * d_config_name;
	const char * d_config_tags;
	const char * d_description;
	const char * d_log_level; // no , error , warn , verbose,
	const char * d_log_file;
	
	int d_enable;
	int d_shutdown;
	int d_watchdog_enabled;
	int d_load_prev_config;
	int d_dump_current_config;
	int d_dump_prev_config;
	int d_time_out_sec;
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
	struct sU2T_config_0 * pGeneralConfiguration = ( struct sU2T_config_0 * )&U2T_config;

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
		char * perr = NULL;
		char * pver = ( char * )ver.inner.value.value.as_string;
		cfg_ver.Major = ( int )strtol( strtok( pver , "." ) , &perr , 10 ); if ( perr != NULL ) return -1;
		cfg_ver.Minor = ( int )strtol( strtok( NULL , "." ) , &perr, 10 ); if ( perr != NULL ) return -1;
		cfg_ver.Build = ( int )strtol( strtok( NULL , "." ) , &perr, 10 ); if ( perr != NULL ) return -1;
		cfg_ver.Revision_Patch = ( int )strtol( strtok( NULL , "." ) , &perr , 10 ); if ( perr != NULL ) return -1;

		json_free( &el_config_ver );
	}
	cfg_ver.Revision_Patch++; // just to ignore warning unused

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

		{
			result( json_element ) re_configurations = json_object_find( el_U2T_config.value.as_object , "configurations" );
			if ( catch_error( &re_configurations , "configurations" ) ) return -1;
			typed( json_element ) el_configurations = result_unwrap( json_element )( &re_configurations );

			#define CFG_ELEM_STR( name ) \
				result( json_element ) re_##name = json_object_find( el_configurations.value.as_object , "d_##name" );\
				if ( catch_error( &re_##name , "d_##name" ) ) return -1;\
				typed( json_element ) el_##name = result_unwrap( json_element )( &re_##name );\
				pGeneralConfiguration->d_##name = newstr(el_##name.value.as_string);
			#define CFG_ELEM_I( name ) \
				result( json_element ) re_##name = json_object_find( el_configurations.value.as_object , "d_##name" );\
				if ( catch_error( &re_##name , "d_##name" ) ) return -1;\
				typed( json_element ) el_##name = result_unwrap( json_element )( &re_##name );\
				pGeneralConfiguration->d_##name = (int)el_##name.value.as_number.value.as_long;

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

			//printf( "%d\n" , tunnels.value.as_object->count );

			result( json_element ) re_tunnel1 = json_object_find( el_tunnels.value.as_object , "tunnel1" );
			if ( catch_error( &re_tunnel1 , "tunnel1" ) ) return -1;
			typed( json_element ) el_tunnel1 = result_unwrap( json_element )( &re_tunnel1 );

			result( json_element ) re_UDP_origin_ip = json_object_find( el_tunnel1.value.as_object , "UDP_origin_ip" );
			if ( catch_error( &re_UDP_origin_ip , "UDP_origin_ip" ) ) return -1;
			//typed( json_element ) el_UDP_origin_ip = result_unwrap( json_element )( &re_UDP_origin_ip );
			//pGeneralConfiguration->d_UDP_origin_ip = el_UDP_origin_ip.value.as_number.value.as_long;
		}

		//json_print( &U2T_config , 1 );

		json_free( &el_U2T_config );
	}

	return 0;
}
