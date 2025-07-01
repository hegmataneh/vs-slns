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
#define Uses_INIT_BREAKABLE_FXN

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

#ifndef section_wave_config

struct wave_cfg_id
{
	char wave_name[64];
	char UDP_destination_ip[64];
	int UDP_destination_port;
	char UDP_destination_interface[128];
};

struct wave_cfg_0
{
	struct wave_cfg_id id;

	int limited_packets;
	int packet_count;
	int parallelism_count;

	int enable;
	int reset_connections;
};

struct wave_cfg_n
{
	struct wave_cfg_0 m; // first member , n - 1
};

struct wave_cfg // finalizer
{
	struct wave_cfg_n m; // be first member
	// ...
	//int checked_off; // if removed from a list after action
	//int changed;
	//int deleted;
};

#endif

#ifndef section_udp_wave

struct udp_wave
{
	//int udp_port_number;
	//int udp_sockfd;
	//int try_connect;
	//int udp_connection_established; // udp socket established

	struct wave_cfg cfg; // copy of applied cfg

	pthread_t * pThreads; // each one keep one thread
	int * socket_ids; // 
	int * thread_masks; // each int represent thrread is valid
	size_t mask_count; // thread keeper count

	int sent_counter;
};

struct udp_wave_holder
{
	struct udp_wave * pwave; // allocated
};

struct wave_holders
{
	struct udp_wave_holder * pholder; // all the active waves
	int * pValidity_masks; // waves masks
	size_t mask_count; // mask count

	//struct udp_wave_holder empty_one; // just use for condition that loaded config does not have any valid enable bridge config . it must be zero all the time
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

	// waves
	struct wave_cfg * _pprev_wave_cfg; // maybe later in some condition we need to rollback ro prev config
	size_t _prev_wave_cfg_count;
	struct wave_cfg * _pwave_cfg;
	size_t _wave_cfg_count;
	int _wave_config_changed; // act like bool . something is changed
};

struct App_Data
{
	struct App_Config cfg;
	struct wave_holders waves;
};

#endif

#ifndef section_connection_functions

void * wave_runner( void * src_pwave )
{
	struct udp_wave * pwave = ( struct udp_wave * )src_pwave;
	pthread_t tid = pthread_self();
	int socketid = -1;
	for ( int i = 0 ; i < pwave->mask_count ; i++ )
	{
		if ( pwave->thread_masks[ i ] )
		{
			if ( pwave->pThreads[ i ] == tid )
			{
				socketid = pwave->socket_ids[ i ];
				break;
			}
		}
	}
	ASSERT( socketid >= 0 );

	char buffer[ 100 ];
	memset( buffer , 0 , sizeof( buffer ) );
	sprintf( buffer , "sent %d from %d" , pwave->sent_counter , tid );

	while ( 1 )
	{
		pwave->sent_counter++;

		send( socketid , buffer , strlen( buffer ) , 0 );
	}
	return NULL;
}

void apply_new_wave_config( struct App_Data * _g , struct udp_wave * pwave , struct wave_cfg * new_cfg )
{
	INIT_BREAKABLE_FXN();

	if ( pwave->mask_count < new_cfg->m.m.parallelism_count )
	{
		int old_mask_count = pwave->mask_count;
		int diff_count = ( new_cfg->m.m.parallelism_count - pwave->mask_count );
		int new_mask_count = old_mask_count + diff_count;

		M_BREAK_IF( !( pwave->pThreads = REALLOC( pwave->pThreads , new_mask_count * sizeof( pthread_t ) ) ) , errMemoryLow , 0 );
		MEMSET_ZERO( pwave->pThreads + old_mask_count , pthread_t , diff_count );

		M_BREAK_IF( !( pwave->socket_ids = REALLOC( pwave->socket_ids , new_mask_count * sizeof( int ) ) ) , errMemoryLow , 1 );
		MEMSET_ZERO( pwave->socket_ids + old_mask_count , int , diff_count );

		M_BREAK_IF( !( pwave->thread_masks = REALLOC( pwave->thread_masks , new_mask_count * sizeof( int ) ) ) , errMemoryLow , 2 );
		MEMSET_ZERO( pwave->thread_masks + old_mask_count , int , diff_count );

		pwave->mask_count = new_mask_count;
	}

	int valid_thread_count = 0;
	for ( int i = 0 ; i < pwave->mask_count ; i++ )
	{
		if ( pwave->thread_masks[ i ] ) valid_thread_count++;
	}
	if ( valid_thread_count < new_cfg->m.m.parallelism_count )
	{
		// run new one
		int diff_new_thread_count = new_cfg->m.m.parallelism_count - valid_thread_count;
		while ( diff_new_thread_count > 0 )
		{
			for ( int i = 0 ; i < pwave->mask_count ; i++ )
			{
				if ( !pwave->thread_masks[ i ] )
				{
					pthread_t trd;
					M_BREAK_IF( ( pthread_create( &trd , NULL , wave_runner , ( void * )pwave ) ) != 0 , errGeneral , 2 );
					pwave->pThreads[ i ] = trd;
					pwave->thread_masks[ i ] = 1;

					M_BREAK_IF( ( pwave->socket_ids[ i ] = socket( AF_INET , SOCK_DGRAM , 0 ) ) < 0 , errGeneral , 2 );
					struct sockaddr_in server_addr;
					memset( &server_addr , 0 , sizeof( server_addr ) );
					server_addr.sin_family = AF_INET; // IPv4
					server_addr.sin_port = htons( ( uint16_t )pwave->cfg.m.m.id.UDP_destination_port ); // Convert port to network byte order
					//server_addr.sin_addr.s_addr = inet_addr("YOUR_IP_ADDRESS"); // Specify the IP address to bind to
					server_addr.sin_addr.s_addr = INADDR_ANY; // Or use INADDR_ANY to bind to all available interfaces:
					M_BREAK_IF( bind( pwave->socket_ids[ i ] , ( const struct sockaddr * )&server_addr , sizeof( server_addr ) ) < 0 , errGeneral , 2 );

					diff_new_thread_count--;
					break;
				}
			}
		}
	}

	if ( valid_thread_count > new_cfg->m.m.parallelism_count )
	{
		// stop extra
		int extra_count = valid_thread_count - new_cfg->m.m.parallelism_count;
		while ( extra_count > 0 )
		{
			for ( int i = pwave->mask_count - 1 ; i >= 0 ; i-- ) // az yah hazf kon
			{
				if ( pwave->thread_masks[ i ] )
				{
					_close_socket( pwave->socket_ids + i );
					if ( pthread_cancel( pwave->pThreads[ i ] ) == 0 )
					{
						pwave->thread_masks[ i ] = 0;
						extra_count--;
					}
					break;
				}
			}
		}
	}

	memcpy( &pwave->cfg , new_cfg , sizeof( struct wave_cfg ) );

	// TODO . complete reverse on error

	BEGIN_RET
		case 1: DAC( pwave->pThreads );
	V_END_RET
}

void stop_wave( struct App_Data * _g , struct udp_wave * pwave )
{
	INIT_BREAKABLE_FXN();

	pwave->cfg.m.m.parallelism_count = 0;
	apply_new_wave_config( _g , pwave , &pwave->cfg );
	// TODO
}

void apply_new_wave_changes( struct App_Data * _g , struct wave_cfg * prev_cfg , struct wave_cfg * new_cfg )
{
	INIT_BREAKABLE_FXN();

	for ( int i = 0 ; i < _g->waves.mask_count ; i++ )
	{
		if ( _g->waves.pValidity_masks[ i ] )
		{
			if ( memcmp( &_g->waves.pholder[ i ].pwave->cfg.m.m.id , &prev_cfg->m.m.id , sizeof( struct wave_cfg_id ) ) == 0 )
			{
				apply_new_wave_config( _g , _g->waves.pholder[ i ].pwave , new_cfg );
			}
		}
	}
}

void remove_wave( struct App_Data * _g , struct wave_cfg * cfg )
{
	INIT_BREAKABLE_FXN();
	for ( int i = 0 ; i < _g->waves.mask_count ; i++ )
	{
		if ( _g->waves.pValidity_masks[ i ] )
		{
			if ( memcmp( &_g->waves.pholder[ i ].pwave->cfg.m.m.id , &cfg->m.m.id , sizeof( struct wave_cfg_id ) ) == 0 )
			{
				stop_wave( _g , _g->waves.pholder[ i ].pwave );
				_g->waves.pValidity_masks[ i ] = 0;
				DAC( _g->waves.pholder[ i ].pwave );
			}
		}
	}
}

#define PREALLOCAION_SIZE 10

void add_new_wave( struct App_Data * _g , struct wave_cfg * new_cfg )
{
	INIT_BREAKABLE_FXN();

	if ( !_g->waves.mask_count )
	{
		M_BREAK_IF( !( _g->waves.pValidity_masks = NEWBUF( int , PREALLOCAION_SIZE ) ) , errMemoryLow , 0 );
		MEMSET_ZERO( _g->waves.pValidity_masks , int , PREALLOCAION_SIZE );

		M_BREAK_IF( !( _g->waves.pholder = NEWBUF( struct udp_wave_holder , PREALLOCAION_SIZE ) ) , errMemoryLow , 1 );
		MEMSET_ZERO( _g->waves.pholder , struct udp_wave_holder , PREALLOCAION_SIZE );

		_g->waves.mask_count = PREALLOCAION_SIZE;
	}

	int new_cfg_placement_index = -1;
	while ( new_cfg_placement_index < 0 )
	{
		for ( int i = 0 ; i < _g->waves.mask_count ; i++ )
		{
			if ( !_g->waves.pValidity_masks[ i ] )
			{
				new_cfg_placement_index = i;
				break;
			}
		}
		if ( new_cfg_placement_index < 0 )
		{
			int old_mask_count = _g->waves.mask_count;
			int new_mask_count = old_mask_count + PREALLOCAION_SIZE;

			M_BREAK_IF( !( _g->waves.pValidity_masks = REALLOC( _g->waves.pValidity_masks , new_mask_count * sizeof( int ) ) ) , errMemoryLow , 2 );
			MEMSET_ZERO( _g->waves.pValidity_masks + old_mask_count , int , PREALLOCAION_SIZE );

			M_BREAK_IF( !( _g->waves.pholder = REALLOC( _g->waves.pholder , new_mask_count * sizeof( struct udp_wave_holder ) ) ) , errMemoryLow , 1 );
			MEMSET_ZERO( _g->waves.pholder + old_mask_count , struct udp_wave_holder , PREALLOCAION_SIZE );

			_g->waves.mask_count = new_mask_count;
		}
	}

	ASSERT( _g->waves.pholder[ new_cfg_placement_index ].pwave );
	M_BREAK_IF( !( _g->waves.pholder[ new_cfg_placement_index ].pwave = NEW( struct udp_wave ) ) , errMemoryLow , 0 );
	MEMSET_ZERO( _g->waves.pholder[ new_cfg_placement_index ].pwave , struct udp_wave , 1 );
	_g->waves.pValidity_masks[ new_cfg_placement_index ] = 1;
	memcpy( &_g->waves.pholder[ new_cfg_placement_index ].pwave->cfg , new_cfg , sizeof( struct wave_cfg ) );
	//_g->waves.pholder[ new_cfg_placement_index ].pwave->try_connect = 1;

	apply_new_wave_changes( _g , new_cfg , new_cfg );

	BEGIN_RET
		case 2: DAC( _g->waves.pholder );
		case 1: DAC( _g->waves.pValidity_masks );
	V_END_RET
} // TODO . return value

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
			struct wave_cfg * pWaves = NULL;
			size_t waves_count = 0;
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

				/*waves*/
				{
					result( json_element ) re_waves = json_object_find( el_UDP_generator_config.value.as_object , "waves" );
					if ( catch_error( &re_waves , "waves" ) ) ERR_RET( "err" , VOID_RET );
					typed( json_element ) el_waves = result_unwrap( json_element )( &re_waves );

					if ( ( waves_count = el_waves.value.as_object->count ) < 1 ) ERR_RET( "err" , VOID_RET );

					if ( !( pWaves = ( struct wave_cfg * )malloc( sizeof( struct wave_cfg ) * el_waves.value.as_object->count ) ) )
					{
						ERR_RET( "insufficient memory" , VOID_RET );
					}

					for ( int i = 0 ; i < el_waves.value.as_object->count ; i++ )
					{
						char output_wave_name[32];
						memset( output_wave_name , 0 , sizeof( output_wave_name ) );
						sprintf( output_wave_name , "wave%d" , i + 1 );

						result( json_element ) re_output_wave = json_object_find( el_waves.value.as_object , output_wave_name );
						if ( catch_error( &re_output_wave , output_wave_name ) ) ERR_RET( "err" , VOID_RET );
						typed( json_element ) el_output_wave = result_unwrap( json_element )( &re_output_wave );

						#define CFG_ELEM_STR( name ) \
							result( json_element ) re_##name = json_object_find( el_output_wave.value.as_object , #name );\
							if ( catch_error( &re_##name , #name ) ) ERR_RET( "err" , VOID_RET );\
							typed( json_element ) el_##name = result_unwrap( json_element )( &re_##name );\
							strcpy(((struct wave_cfg_0 *)(pWaves + i))->name , el_##name.value.as_string );
						
						#define CFG_ID_ELEM_STR( name ) \
							result( json_element ) re_##name = json_object_find( el_output_wave.value.as_object , #name );\
							if ( catch_error( &re_##name , #name ) ) ERR_RET( "err" , VOID_RET );\
							typed( json_element ) el_##name = result_unwrap( json_element )( &re_##name );\
							strcpy(((struct wave_cfg_0 *)(pWaves + i))->id.name , el_##name.value.as_string );
						
						#define CFG_ELEM_I( name ) \
							result( json_element ) re_##name = json_object_find( el_output_wave.value.as_object , #name );\
							if ( catch_error( &re_##name , #name ) ) ERR_RET( "err" , VOID_RET );\
							typed( json_element ) el_##name = result_unwrap( json_element )( &re_##name );\
							((struct wave_cfg_0 *)(pWaves + i))->name = (int)el_##name.value.as_number.value.as_long;

						#define CFG_ID_ELEM_I( name ) \
							result( json_element ) re_##name = json_object_find( el_output_wave.value.as_object , #name );\
							if ( catch_error( &re_##name , #name ) ) ERR_RET( "err" , VOID_RET );\
							typed( json_element ) el_##name = result_unwrap( json_element )( &re_##name );\
							((struct wave_cfg_0 *)(pWaves + i))->id.name = (int)el_##name.value.as_number.value.as_long;

						strcpy(((struct wave_cfg_0 *)(pWaves + i))->id.wave_name , output_wave_name );

						CFG_ID_ELEM_STR( UDP_destination_ip );
						CFG_ID_ELEM_I( UDP_destination_port );
						CFG_ID_ELEM_STR( UDP_destination_interface );
						CFG_ELEM_I( limited_packets );
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
			
			if ( !_g->cfg._pwave_cfg )
			{
				_g->cfg._pprev_wave_cfg = _g->cfg._pwave_cfg;
				_g->cfg._prev_wave_cfg_count = _g->cfg._wave_cfg_count;
			}
			_g->cfg._pwave_cfg = pWaves;
			_g->cfg._wave_cfg_count = waves_count;
			_g->cfg._version_changed = 0;
			_g->cfg._wave_config_changed = 1;
			pWaves = NULL; // to not delete intentionally
			waves_count = 0;
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
	INIT_BREAKABLE_FXN();
	struct App_Data * _g = ( struct App_Data * )app_data;

	pthread_t trd_udp_connection , trd_tcp_connection;
	pthread_t trd_protocol_bridge;

	while ( !_g->cfg._wave_cfg_count ) // load after any config loaded
	{
		sleep( 1 );
	}

	while ( 1 )
	{
		if ( _g->cfg._wave_config_changed )
		{
			for ( int i = 0 ; i < _g->cfg._prev_wave_cfg_count ; i++ )
			{
				int exist = 0;
				for ( int j = 0 ; j < _g->cfg._wave_cfg_count ; j++ )
				{
					if ( exist ) break;
					if ( memcmp( &_g->cfg._pprev_wave_cfg[ i ].m.m.id , &_g->cfg._pwave_cfg[ j ].m.m.id , sizeof( struct wave_cfg_id ) ) == 0 )
					{
						// existed cfg changed
						apply_new_wave_changes( _g , &_g->cfg._pprev_wave_cfg[ i ] , &_g->cfg._pwave_cfg[ j ] );
						exist = 1;
					}
				}
				if ( !exist )
				{
					// remove removed one
					remove_wave( _g , &_g->cfg._pprev_wave_cfg[ i ] );
				}
			}
			for ( int i = 0 ; i < _g->cfg._wave_cfg_count ; i++ )
			{
				int exist = 0;
				for ( int j = 0 ; j < _g->cfg._prev_wave_cfg_count ; j++ )
				{
					if ( memcmp( &_g->cfg._pwave_cfg[ i ].m.m.id , &_g->cfg._pprev_wave_cfg[ j ].m.m.id , sizeof(struct wave_cfg_id)) == 0 )
					{
						exist = 1;
						break;
					}
				}
				if ( !exist )
				{
					// start new cfg
					add_new_wave( _g , &_g->cfg._pwave_cfg[ i ] );
				}
			}
			_g->cfg._wave_config_changed = 0; // changes applied
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
