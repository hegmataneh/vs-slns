#ifndef section_include

#define Uses_json
#define Uses_close
#define Uses_socket
#define Uses_pthread_t
#define Uses_rand
#define Uses_errno
#define Uses_strerror
#define Uses_va_list
#define Uses_printf
#define Uses_sockaddr_in
#define Uses_ssize_t
#define Uses_Remote_vs_prj
#define Uses_NEWSTR

#include <make_udp_packet.dep>

#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wsign-conversion"

#endif

#ifndef section_global_config

struct Config_ver
{
	char version[64];
	int Major; //Indicates significant changes , potentially including incompatible API changes.
	int Minor; //Denotes new functionality added in a backwards - compatible manner.
	int Build; //Represents the specific build number of the software.
	int Revision_Patch; //Represents backwards - compatible bug fixes or minor updates.
};

// general config

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

#endif

#ifndef section_udp_config

struct output_udp_cfg_id_part
{
	char UDP_destination_ip[64];
	int UDP_destination_port;
	char UDP_destination_interface[128];
};

struct output_udp_CFG_0
{
	struct output_udp_cfg_id_part id;

	int limit_packets;
	int packet_count;
	int parallelism_count;

	int enable;
	int reset_connections;
};

struct output_udp_CFG_n
{
	struct output_udp_CFG_0 m; // first member , n - 1
};

struct output_udp_CFG // finalizer
{
	struct output_udp_CFG_n m; // be first member
	// ...
	//int checked_off; // if removed from a list after action
	//int changed;
	//int deleted;
};

#endif

#ifndef section_udp_connections

struct UDP_connection_data
{
	int udp_port_number;
	int udp_sockfd;
	int udp_connection_established; // udp socket established

	struct output_udp_CFG * pCFG;
};

struct UDP_connection_holder
{
	struct UDP_connection_data udp_data;
};

struct UDPs_holder
{
	struct UDP_connection_holder * udps; // all the active udps
	size_t udps_count;

	struct UDP_connection_holder empty_one; // just use for condition that loaded config does not have any valid enable bridge config . it must be zero all the time
};

#endif

#ifndef section_app_data

struct App_Config // global config
{
	struct Config_ver ___temp_ver; // not usable just to prevent reallocation
	struct Config_ver * _ver; // app version
	int _version_changed; // act like bool . this var indicate just load new config

	// general
	struct Global_Config * _prev_general_config;
	struct Global_Config * _general_config;
	int _general_config_changed;

	// udps
	struct output_udp_CFG * _prev_cfg_udps; // maybe later in some condition we need to rollback ro prev config
	size_t _prev_cfg_udps_count;
	struct output_udp_CFG * _cfg_udps;
	size_t _cfg_udps_count;
	int _udp_config_changed; // act like bool . something is changed
};

struct App_Data
{
	struct App_Config cfg;
	struct UDPs_holder udps;
};

#endif

#ifndef section_connection_functions

// Thread function to send UDP packets
//void *send_udp_thread(void *arg) {
//
//    char custom_message[256];
//
//    struct ThreadArgs *args = (struct ThreadArgs *)arg;
//    char buffer[BUFFER_SIZE];
//
//    while (1) {
//        printf("Enter text to send (or 'exit'): ");
//        if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) {
//            _DETAIL_ERROR("fgets failed");
//            break;
//        }
//
//        // Remove trailing newline character if present
//        buffer[strcspn(buffer, "\n")] = 0;
//
//        if (strcmp(buffer, "exit") == 0) {
//            printf("Exiting sender thread.\n");
//            break;
//        }
//        ssize_t bytes_sent = 0;
//        int dest = rand() % 3;
//        switch ( dest )
//        {
//            case 0:
//            {
//                bytes_sent += sendto(args->sockfds[0] , buffer , strlen(buffer) , 0 ,
//                                     (struct sockaddr *)&args->dest_addrs[0] ,
//                                     sizeof(args->dest_addrs[0]));
//                break;
//            }
//            case 1:
//            {
//                bytes_sent = sendto(args->sockfds[1] , buffer , strlen(buffer) , 0 ,
//                                     (struct sockaddr *)&args->dest_addrs[1] ,
//                                     sizeof(args->dest_addrs[1]));
//                break;
//            }
//            case 2:
//            {
//                bytes_sent = sendto(args->sockfds[0] , buffer , strlen(buffer) , 0 ,
//                                     (struct sockaddr *)&args->dest_addrs[0] ,
//                                     sizeof(args->dest_addrs[0]));
//                bytes_sent += sendto(args->sockfds[1] , buffer , strlen(buffer) , 0 ,
//                                     (struct sockaddr *)&args->dest_addrs[1] ,
//                                     sizeof(args->dest_addrs[1]));
//                break;
//            }
//        }
//        if ( bytes_sent < 0 )
//        {
//            _DETAIL_ERROR("sendto failed");
//            break;
//        }
//        printf("Sent %zd bytes: '%s' to terminal %d\n", bytes_sent, buffer, dest);
//    }
//    pthread_exit(NULL);
//}

void apply_new_udp_changes( struct App_Data * _g , struct output_udp_CFG * prev_cfg , struct output_udp_CFG * new_cfg )
{

}

void remove_udp( struct App_Data * _g , struct output_udp_CFG * cfg )
{

}

void add_new_udp_changes( struct App_Data * _g , struct output_udp_CFG * cfg )
{

}

#endif

#ifndef section_load_config

#define CONFIG_ROOT_PATH "/home/my_projects/home-config/UDP_generator"

// TODO . exit gracefully by auto mechanism
// TODO . think about race condition
void * version_checker( void * app_data )
{
	INIT_BREAKABLE_FXN();

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
			const char * config_ver_file_content = read_file( CONFIG_ROOT_PATH "/config_ver.txt" , ( char * )buf );
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
	INIT_BREAKABLE_FXN();

	struct App_Data * _g = ( struct App_Data * )app_data;
	time_t prev_time , cur_time;

	while ( !_g->cfg._version_changed ) // load after version loaded
	{
		sleep( 1 );
	}

	typed( json_element ) el_UDP_generator_config;

	while ( 1 )
	{
		cur_time = time( NULL );

		if ( _g->cfg._general_config == NULL || _g->cfg._version_changed /* || difftime(cur_time , prev_time) > 15 * 60*/ )
		{
			prev_time = prev_time; // to ignore warning
			prev_time = cur_time;

			struct Global_Config temp_config = { 0 };
			struct Global_Config_0 * pGeneralConfiguration = ( struct Global_Config_0 * )&temp_config;
			struct output_udp_CFG * pUDPs = NULL;
			size_t udps_count = 0;
			{
				const char * UDP_generator_config_file_content = read_file( CONFIG_ROOT_PATH "/UDP_generator_config.txt" , NULL );
				if ( UDP_generator_config_file_content == NULL )
				{
					ERR_RET( "cannot open config file" , VOID_RET );
				}
				result( json_element ) rs_UDP_generator_config = json_parse( UDP_generator_config_file_content );
				free( ( void * )UDP_generator_config_file_content );
				if ( catch_error( &rs_UDP_generator_config , "UDP_generator_config" ) ) ERR_RET( "cannot parse config file" , VOID_RET );
				el_UDP_generator_config = result_unwrap( json_element )( &rs_UDP_generator_config );

				/*configurations*/
				if ( _g->cfg._ver->Major >= 1 ) // first version of config file structure
				{
					result( json_element ) re_configurations = json_object_find( el_UDP_generator_config.value.as_object , "configurations" );
					if ( catch_error( &re_configurations , "configurations" ) ) ERR_RET( "err" , VOID_RET );
					typed( json_element ) el_configurations = result_unwrap( json_element )( &re_configurations );

					#define CFG_ELEM_STR( name ) \
						result( json_element ) re_##name = json_object_find( el_configurations.value.as_object , #name );\
						if ( catch_error( &re_##name , #name ) ) ERR_RET( "err" , VOID_RET );\
						typed( json_element ) el_##name = result_unwrap( json_element )( &re_##name );\
						NEWSTR( pGeneralConfiguration->name , el_##name.value.as_string , 0 );
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

				/*output_udps*/
				{
					result( json_element ) re_output_udps = json_object_find( el_UDP_generator_config.value.as_object , "output_udps" );
					if ( catch_error( &re_output_udps , "output_udps" ) ) ERR_RET( "err" , VOID_RET );
					typed( json_element ) el_output_udps = result_unwrap( json_element )( &re_output_udps );

					if ( ( udps_count = el_output_udps.value.as_object->count ) < 1 ) ERR_RET( "err" , VOID_RET );

					if ( !( pUDPs = ( struct output_udp_CFG * )malloc( sizeof( struct output_udp_CFG ) * el_output_udps.value.as_object->count ) ) )
					{
						ERR_RET( "insufficient memory" , VOID_RET );
					}

					for ( int i = 0 ; i < el_output_udps.value.as_object->count ; i++ )
					{
						char output_udp_name[32];
						sprintf( output_udp_name , "UDP%d" , i + 1 );

						result( json_element ) re_output_udp = json_object_find( el_output_udps.value.as_object , output_udp_name );
						if ( catch_error( &re_output_udp , output_udp_name ) ) ERR_RET( "err" , VOID_RET );
						typed( json_element ) el_output_udp = result_unwrap( json_element )( &re_output_udp );

						#define CFG_ELEM_STR( name ) \
							result( json_element ) re_##name = json_object_find( el_output_udp.value.as_object , #name );\
							if ( catch_error( &re_##name , #name ) ) ERR_RET( "err" , VOID_RET );\
							typed( json_element ) el_##name = result_unwrap( json_element )( &re_##name );\
							strcpy(((struct output_udp_CFG_0 *)(pUDPs + i))->name , el_##name.value.as_string );
						
						#define CFG_ID_ELEM_STR( name ) \
							result( json_element ) re_##name = json_object_find( el_output_udp.value.as_object , #name );\
							if ( catch_error( &re_##name , #name ) ) ERR_RET( "err" , VOID_RET );\
							typed( json_element ) el_##name = result_unwrap( json_element )( &re_##name );\
							strcpy(((struct output_udp_CFG_0 *)(pUDPs + i))->id.name , el_##name.value.as_string );
						
						#define CFG_ELEM_I( name ) \
							result( json_element ) re_##name = json_object_find( el_output_udp.value.as_object , #name );\
							if ( catch_error( &re_##name , #name ) ) ERR_RET( "err" , VOID_RET );\
							typed( json_element ) el_##name = result_unwrap( json_element )( &re_##name );\
							((struct output_udp_CFG_0 *)(pUDPs + i))->name = (int)el_##name.value.as_number.value.as_long;

						#define CFG_ID_ELEM_I( name ) \
							result( json_element ) re_##name = json_object_find( el_output_udp.value.as_object , #name );\
							if ( catch_error( &re_##name , #name ) ) ERR_RET( "err" , VOID_RET );\
							typed( json_element ) el_##name = result_unwrap( json_element )( &re_##name );\
							((struct output_udp_CFG_0 *)(pUDPs + i))->id.name = (int)el_##name.value.as_number.value.as_long;

						CFG_ID_ELEM_STR( UDP_destination_ip );
						CFG_ID_ELEM_I( UDP_destination_port );
						CFG_ID_ELEM_STR( UDP_destination_interface );
						CFG_ELEM_I( limit_packets );
						CFG_ELEM_I( packet_count );
						CFG_ELEM_I( parallelism_count );
						CFG_ELEM_I( enable );
						CFG_ELEM_I( reset_connections );

						#undef CFG_ID_ELEM_I
						#undef CFG_ELEM_I
						#undef CFG_ID_ELEM_STR
						#undef CFG_ELEM_STR
					}
				}

				json_free( &el_UDP_generator_config );
			}
		
			int initial_general_config = 0;
			if ( _g->cfg._general_config == NULL ) // TODO . make assignemnt atomic
			{
				if ( !( _g->cfg._general_config = malloc( sizeof( struct Global_Config ) ) ) )
				{
					ERR_RET( "insufficient memory" , VOID_RET );
				}
				memset( _g->cfg._general_config , 0 , sizeof( struct Global_Config ) );
				memcpy( _g->cfg._general_config , &temp_config , sizeof( temp_config ) );
				memset( &temp_config , 0 , sizeof( temp_config ) ); // copy to global variable and then zero to not free strings
				initial_general_config = 1;
				_g->cfg._general_config_changed = 1;
			}
			if ( !initial_general_config )
			{
				//DAC(_g->cfg._prev_general_config );
				//do
				//{
				//	if ( _g->cfg._prev_general_config )
				//	{
				//		DEL( _g->cfg._prev_general_config ); _g->cfg._prev_general_config = NULL;
				//	}
				//} while ( 0 );

				DAC(_g->cfg._prev_general_config);

				_g->cfg._prev_general_config = _g->cfg._general_config;
				_g->cfg._general_config = NULL;

				if ( !( _g->cfg._general_config = malloc( sizeof( struct Global_Config ) ) ) )
				{
					ERR_RET( "insufficient memory" , VOID_RET );
				}
				memset( _g->cfg._general_config , 0 , sizeof( struct Global_Config ) );
				memcpy( _g->cfg._general_config , &temp_config , sizeof( temp_config ) );
				memset( &temp_config , 0 , sizeof( temp_config ) ); // copy to global variable and then zero to not free strings

				if ( _g->cfg._prev_general_config != NULL && _g->cfg._general_config != NULL )
				{
					_g->cfg._general_config_changed |= !!strcmp( _g->cfg._general_config->c.c.create_date , _g->cfg._prev_general_config->c.c.create_date );
					_g->cfg._general_config_changed |= !!strcmp( _g->cfg._general_config->c.c.modify_date , _g->cfg._prev_general_config->c.c.modify_date );
					_g->cfg._general_config_changed |= !!strcmp( _g->cfg._general_config->c.c.config_name , _g->cfg._prev_general_config->c.c.config_name );
					_g->cfg._general_config_changed |= !!strcmp( _g->cfg._general_config->c.c.config_tags , _g->cfg._prev_general_config->c.c.config_tags );
					_g->cfg._general_config_changed |= !!strcmp( _g->cfg._general_config->c.c.description , _g->cfg._prev_general_config->c.c.description );
					_g->cfg._general_config_changed |= !!strcmp( _g->cfg._general_config->c.c.log_level , _g->cfg._prev_general_config->c.c.log_level );
					_g->cfg._general_config_changed |= !!strcmp( _g->cfg._general_config->c.c.log_file , _g->cfg._prev_general_config->c.c.log_file );

					_g->cfg._general_config_changed |= !( _g->cfg._general_config->c.c.enable == _g->cfg._prev_general_config->c.c.enable );
					_g->cfg._general_config_changed |= !( _g->cfg._general_config->c.c.shutdown == _g->cfg._prev_general_config->c.c.shutdown );
					_g->cfg._general_config_changed |= !( _g->cfg._general_config->c.c.watchdog_enabled == _g->cfg._prev_general_config->c.c.watchdog_enabled );
					_g->cfg._general_config_changed |= !( _g->cfg._general_config->c.c.load_prev_config == _g->cfg._prev_general_config->c.c.load_prev_config );
					_g->cfg._general_config_changed |= !( _g->cfg._general_config->c.c.dump_current_config == _g->cfg._prev_general_config->c.c.dump_current_config );
					_g->cfg._general_config_changed |= !( _g->cfg._general_config->c.c.dump_prev_config == _g->cfg._prev_general_config->c.c.dump_prev_config );
					_g->cfg._general_config_changed |= !( _g->cfg._general_config->c.c.time_out_sec == _g->cfg._prev_general_config->c.c.time_out_sec );
				}
			}
			
			if ( !_g->cfg._cfg_udps )
			{
				_g->cfg._prev_cfg_udps = _g->cfg._cfg_udps;
				_g->cfg._prev_cfg_udps_count = _g->cfg._cfg_udps_count;
			}
			_g->cfg._cfg_udps = pUDPs;
			_g->cfg._cfg_udps_count = udps_count;
			_g->cfg._version_changed = 0;
			_g->cfg._udp_config_changed = 1;
			pUDPs = NULL; // to not delete intentionally
			udps_count = 0;
		}
		sleep( 2 );
	}

	BEGIN_RET
	case 0:
	{
		json_free( &el_UDP_generator_config );
		break;
	}
	V_END_RET

	return NULL;
}

// TODO . aware of concurrency in config read and act on it
void * udp_generator_manager( void * app_data )
{
	FUNCTION_SCOPE_INIT();
	struct App_Data * _g = ( struct App_Data * )app_data;

	pthread_t trd_udp_connection , trd_tcp_connection;
	pthread_t trd_protocol_bridge;

	while ( !_g->cfg._cfg_udps_count ) // load after any config loaded
	{
		sleep( 1 );
	}

	while ( 1 )
	{
		if ( _g->cfg._udp_config_changed )
		{
			for ( int i = 0 ; i < _g->cfg._prev_cfg_udps_count ; i++ )
			{
				int exist = 0;
				for ( int j = 0 ; j < _g->cfg._cfg_udps_count ; j++ )
				{
					if ( exist ) break;
					if ( memcmp( &_g->cfg._prev_cfg_udps[ i ].m.m.id , &_g->cfg._cfg_udps[ j ].m.m.id , sizeof( struct output_udp_cfg_id_part ) ) == 0 )
					{
						// existed cfg changed
						apply_new_udp_changes( _g , &_g->cfg._prev_cfg_udps[ i ] , &_g->cfg._cfg_udps[ j ] );
						exist = 1;
					}
				}
				if ( !exist )
				{
					// remove removed one
					remove_udp( _g , &_g->cfg._prev_cfg_udps[ i ] );
				}
			}
			for ( int i = 0 ; i < _g->cfg._cfg_udps_count ; i++ )
			{
				int exist = 0;
				for ( int j = 0 ; j < _g->cfg._prev_cfg_udps_count ; j++ )
				{
					if ( memcmp( &_g->cfg._cfg_udps[ i ].m.m.id , &_g->cfg._prev_cfg_udps[ j ].m.m.id , sizeof(struct output_udp_cfg_id_part)) == 0 )
					{
						exist = 1;
						break;
					}
				}
				if ( !exist )
				{
					// start new cfg
					add_new_udp_changes( _g , &_g->cfg._cfg_udps[ i ] );
				}
			}



			//size_t enable_cfg_count = 0;
			//for ( int i = 0 ; i < _g->cfg._cfg_pbs_count ; i++ )
			//{
			//	if ( _g->cfg._cfg_pbs[ i ].c.c.enable )
			//	{
			//		enable_cfg_count++;
			//	}
			//}
			//if ( !enable_cfg_count )
			//{
			//	_g->bridges.pbs = &_g->bridges.empty_one;
			//	_g->bridges.pbs_count = 0;
			//	continue;
			//}

			//if ( !( _g->bridges.pbs = ( struct Bridge_holder * )malloc( sizeof( struct Bridge_holder ) * enable_cfg_count ) ) )
			//{
			//	ERR_RET( "insufficient memory" , VOID_RET );
			//}
			//memset( _g->bridges.pbs , 0 , sizeof( struct Bridge_holder ) * enable_cfg_count );
			//_g->bridges.pbs_count = enable_cfg_count;
			//int index_active_bridge = -1;
			//for ( int i = 0 ; i < _g->cfg._cfg_pbs_count ; i++ ) // check all cfg
			//{
			//	if ( _g->cfg._cfg_pbs[ i ].c.c.enable )
			//	{
			//		_g->bridges.pbs[ ++index_active_bridge ].udp_data.udp_port_number = _g->cfg._cfg_pbs[ i ].c.c.UDP_origin_port;
			//		strcpy( _g->bridges.pbs[ index_active_bridge ].tcp_data.tcp_ip , _g->cfg._cfg_pbs[ i ].c.c.TCP_destination_ip );
			//		_g->bridges.pbs[ index_active_bridge ].tcp_data.tcp_port_number = _g->cfg._cfg_pbs[ i ].c.c.TCP_destination_port;
			//	}
			//}

			//if ( pthread_create( &trd_udp_connection , NULL , thread_udp_connection_proc , app_data ) != 0 )
			//{
			//	_DETAIL_ERROR( "Failed to create thread" );
			//	return NULL; // Indicate an error
			//}


			_g->cfg._udp_config_changed = 0; // changes applied
		}
		sleep( 5 );
	}
	return NULL;
}

#endif

#ifndef section_main

int main()
{
    INIT_BREAKABLE_FXN();
	struct App_Data _g = { 0 };

	pthread_t trd_version_checker;
	if ( pthread_create( &trd_version_checker , NULL , version_checker , ( void * )&_g ) != 0 ) ERR_RET( "Failed to create thread" , MAIN_BAD_RET );

	pthread_t trd_config_loader;
	if ( pthread_create( &trd_config_loader , NULL , config_loader , ( void * )&_g ) != 0 ) ERR_RET( "Failed to create thread" , MAIN_BAD_RET );

	pthread_t trd_udp_generator_manager;
	if ( pthread_create( &trd_udp_generator_manager , NULL , udp_generator_manager , ( void * )&_g ) != 0 ) ERR_RET( "Failed to create thread" , MAIN_BAD_RET );

	if ( pthread_join( trd_udp_generator_manager , NULL ) != 0 )
	{
		_DETAIL_ERROR( "Failed to join thread" );
		return 1; // Indicate an error
	}

    return 0;
}

#endif
