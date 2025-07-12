#ifndef section_include

#define Uses_thrd_sleep
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
#define Uses_ncurses
#define Uses_fd_set
#define Uses_fileno

#include <make_udp_packet.dep>

#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wsign-conversion"

#define VERBOSE_MODE_DEFAULT 0
#define HI_FREQUENT_LOG_INTERVAL_SEC_DEFAULT 5

#define IF_VERBOSE_MODE_CONDITION() ( _g->appcfg._general_config ? _g->appcfg._general_config->c.c.verbose_mode : VERBOSE_MODE_DEFAULT )
#define HI_FREQUENT_LOG_INTERVAL ( _g->appcfg._general_config ? _g->appcfg._general_config->c.c.hi_frequent_log_interval_sec : HI_FREQUENT_LOG_INTERVAL_SEC_DEFAULT )

#endif

#ifndef section_global_config

struct Config_ver
{
	char version[ 64 ];
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

	int verbose_mode;
	int hi_frequent_log_interval_sec;
	int refresh_variable_from_scratch;
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
	char wave_name[ 64 ];
	char UDP_destination_ip[ 64 ];
	int UDP_destination_port;
	char UDP_destination_interface[ 128 ];
};

struct wave_maintained_parameter // stays in position
{
	int limited_packets;
	int packet_count;
	int parallelism_count;

	int enable;
	int iteration_delay_milisec;
	int packet_payload_size;
};

struct wave_momentary_parameter // popup automatically
{
	int reset_connections;
};

struct wave_temp_data
{
	void * _g; // just point to the main g
	int wcfg_changed; // in passive cfg and active cfg that in alive wave, in both it means something changed
	//int wcfg_stabled; // after change happened in action thread cfg must be stablesh before doing any action
};

struct wave_cfg_0
{
	struct wave_cfg_id id; // must be uniq for each wave
	struct wave_maintained_parameter maintained;
	struct wave_momentary_parameter momentary;
	struct wave_temp_data temp_data;
};

struct wave_cfg_n
{
	struct wave_cfg_0 m; // first member , n - 1
};

struct wave_cfg // finalizer
{
	struct wave_cfg_n m; // be first member
};

#endif

#ifndef section_udp_wave

struct wave_thread // هر موج یک دسته ترد دارد که هر کدام بر مبنای کانیگ مشترکی که دارند کار واحدی انجام می دهند
{
	pthread_t trd_id;
	int base_config_change_applied;
	int do_close_thread; // command from outside to inside thread

	struct udp_wave * pwave; // point to upper wave
};

struct wave_thread_holder
{
	struct wave_thread * alc_thread;
};

struct udp_wave
{
	struct wave_cfg awcfg;  // copy of applied cfg . active wave config . یک دسته کامل از ترد ها یک کانفیگ را اعمال می کند

	int socket_id;

	struct wave_thread_holder * wv_trds; // wave threads
	int * wv_trds_masks;  // each int represent thread is valid
	size_t wv_trds_masks_count;  // thread keeper count
};

struct udp_wave_holder // every elemnt at the reallocation array must have holder because reallocate change data values but with holder just pointer addr change
{
	struct udp_wave * alc_wave; // allocated
};

struct wave_holders
{
	struct udp_wave_holder * wv_holders; // all the active waves
	int * wv_holders_masks; // waves masks
	size_t wv_holders_masks_count; // mask count
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
	struct wave_cfg * _pprev_wave_psvcfg; // maybe later in some condition we need to rollback to prev config
	size_t _prev_wave_psvcfg_count;
	struct wave_cfg * _pwave_psvcfg;
	size_t _wave_psvcfg_count;
	int _psvcfg_changed; // act like bool . something is changed
};

#define INPUT_MAX 256

struct statistics
{
	// box & window
	int scr_height , scr_width;
	WINDOW * main_win;
	WINDOW * input_win;

	// cmd
	int pipefds[ 2 ]; // used for bypass stdout
	char last_command[ INPUT_MAX ];
	char input_buffer[ INPUT_MAX ];

	// stat
	time_t start_time;
	int sender_thread_count;
	__int64u all_benchmarks_total_sent_count;
	__int64u all_benchmarks_total_sent_byte;

	__int64u all_benchmarks_total_sent_fail_count;

	__int64u syscal_err_count;
};

struct App_Data
{
	struct App_Config appcfg;
	struct wave_holders waves;
	struct statistics stat;
};

#endif

#ifndef section_connection_functions

void * wave_runner( void * src_pwave )
{
	INIT_BREAKABLE_FXN();

	struct udp_wave * pwave = ( struct udp_wave * )src_pwave;
	struct App_Data * _g = pwave->awcfg.m.m.temp_data._g;
	pthread_t tid = pthread_self();
	int socketid = -1;
	struct wave_thread * pthread = NULL;
	while ( socketid == -1 )
	{
		for ( int i = 0 ; i < pwave->wv_trds_masks_count ; i++ )
		{
			if ( pwave->wv_trds_masks[ i ] )
			{
				if ( pwave->wv_trds[ i ].alc_thread->trd_id == tid )
				{
					socketid = pwave->socket_id;
					pthread = pwave->wv_trds[ i ].alc_thread;
					break;
				}
			}
		}
	}
	if ( pthread == NULL )
	{
		_g->stat.syscal_err_count++;
		return NULL;
	}
	ASSERT( socketid >= 0 );
	//ASSERT( pbase_config_change_applied != NULL );

	_g->stat.sender_thread_count++;

	if ( IF_VERBOSE_MODE_CONDITION() )
	{
		_ECHO( "thread started" );
	}

	int config_changes = 0;
	do
	{
		if ( pthread->do_close_thread )
		{
			break;
		}

		config_changes = 0;
		struct sockaddr_in server_addr;
		memset( &server_addr , 0 , sizeof( server_addr ) );
		server_addr.sin_family = AF_INET;
		server_addr.sin_port = htons( ( uint16_t )pwave->awcfg.m.m.id.UDP_destination_port );
		if ( inet_pton( AF_INET , pwave->awcfg.m.m.id.UDP_destination_ip , &server_addr.sin_addr ) <= 0 )
		{
			_g->stat.syscal_err_count++;
			_ECHO( "inet_pton failed" );
		}

		int buf_size = pwave->awcfg.m.m.maintained.packet_payload_size;
		char * buffer = NEWBUF( char , buf_size );
		MEMSET_ZERO( buffer , char , buf_size );

		ssize_t sz = 0;

		while ( 1 )
		{
			if ( pthread->do_close_thread )
			{
				break;
			}
			if ( pthread->base_config_change_applied )
			{
				pthread->base_config_change_applied = 0;
				config_changes = 1;
				break;
			}

			//if ( socketid < 0 ) break;
			//pwave->sent_counter++;
			//memset( buffer , 0 , sizeof( buffer ) );
			//sprintf( buffer , "trd(%lu) sock(%d) sendsz (%lu) \n" , tid , socketid , sz );

			if ( IF_VERBOSE_MODE_CONDITION() )
			{
				static time_t prev_time_log = 0;
				if ( !prev_time_log ) prev_time_log = time( NULL );
				if ( difftime( time( NULL ) , prev_time_log ) > HI_FREQUENT_LOG_INTERVAL )
				{
					_ECHO( "%s" , buffer );
					prev_time_log = time( NULL );
				}
			}

			if ( ( sz = sendto( socketid , buffer , buf_size , 0 , ( struct sockaddr * )&server_addr , sizeof( server_addr ) ) ) < SENDTO_MIN_OK )
			{
				_g->stat.all_benchmarks_total_sent_fail_count++;
			}
			else
			{
				_g->stat.all_benchmarks_total_sent_count++;
				_g->stat.all_benchmarks_total_sent_byte += sz;
			}

			if ( pwave->awcfg.m.m.maintained.iteration_delay_milisec > 0 )
			{
				sleep( pwave->awcfg.m.m.maintained.iteration_delay_milisec / 1000 );
				//struct timespec ts = { 0, pwave->cfg.m.m.maintained.iteration_delay_milisec * 1000000 };
				//thrd_sleep( &ts , NULL );
			}
		} // while ( 1 )

		DAC( buffer );

	} while ( config_changes );

	_g->stat.sender_thread_count--;
	
	// delete mask
	for ( int i = 0 ; i < pthread->pwave->wv_trds_masks_count ; i++ )
	{
		if ( pthread->pwave->wv_trds_masks[ i ] && pthread->pwave->wv_trds[ i ].alc_thread->trd_id == tid )
		{
			pthread->pwave->wv_trds_masks[ i ] = 0;
			break;
		}
	}
	
	return NULL;
}

void apply_new_wave_config( struct App_Data * _g , struct udp_wave * pwave , struct wave_cfg * new_wcfg )
{
	INIT_BREAKABLE_FXN();

	int open_socket = 0;
	if ( new_wcfg->m.m.maintained.parallelism_count > 0 && pwave->wv_trds == NULL )
	{
		open_socket = 1;
	}

	// make extra space for new one
	if ( pwave->wv_trds_masks_count < new_wcfg->m.m.maintained.parallelism_count )
	{
		int old_thread_count = pwave->wv_trds_masks_count;
		int diff_count = ( new_wcfg->m.m.maintained.parallelism_count - pwave->wv_trds_masks_count );
		int new_thread_count = old_thread_count + diff_count;

		M_BREAK_IF( ( pwave->wv_trds_masks = REALLOC( pwave->wv_trds_masks , new_thread_count * sizeof( int ) ) ) == REALLOC_ERR , errMemoryLow , 0 );
		MEMSET_ZERO( pwave->wv_trds_masks + old_thread_count , int , diff_count );

		M_BREAK_IF( ( pwave->wv_trds = REALLOC( pwave->wv_trds , new_thread_count * sizeof( struct wave_thread_holder ) ) ) == REALLOC_ERR , errMemoryLow , 0 );
		MEMSET_ZERO( pwave->wv_trds + old_thread_count , struct wave_thread_holder , diff_count );

		pwave->wv_trds_masks_count = new_thread_count;
	}

	// 1. create socket first
	if ( open_socket )
	{
		MM_BREAK_IF( ( pwave->socket_id = socket( AF_INET , SOCK_DGRAM , 0 ) ) == FXN_SOCKET_ERR , errGeneral , 0 , "create sock error" );
		if ( IF_VERBOSE_MODE_CONDITION() )
		{
			_ECHO( "create socket %d" , pwave->socket_id );
		}
	}

	// create extra needed thread and socket for each of them
	int valid_thread_count = 0;
	for ( int i = 0 ; i < pwave->wv_trds_masks_count ; i++ )
	{
		if ( pwave->wv_trds_masks[ i ] ) valid_thread_count++;
	}
	// must add new thread to active wave
	if ( valid_thread_count < new_wcfg->m.m.maintained.parallelism_count )
	{
		// run new one
		int diff_new_thread_count = new_wcfg->m.m.maintained.parallelism_count - valid_thread_count;

		if ( IF_VERBOSE_MODE_CONDITION() )
		{
			_ECHO( "num new thread %d" , diff_new_thread_count );
		}
		while ( diff_new_thread_count > 0 )
		{
			for ( int i = 0 ; i < pwave->wv_trds_masks_count ; i++ )
			{
				if ( !pwave->wv_trds_masks[ i ] )
				{
					// 2. set mask
					pwave->wv_trds_masks[ i ] = 1; // order is matter

					DAC( pwave->wv_trds[ i ].alc_thread );
					M_BREAK_IF( ( pwave->wv_trds[ i ].alc_thread = NEW( struct wave_thread ) ) == NEW_ERR , errMemoryLow , 0 );
					MEMSET_ZERO( pwave->wv_trds[ i ].alc_thread , struct wave_thread , 1 );
					pwave->wv_trds[ i ].alc_thread->pwave = pwave;

					// 3. create thread also
					MM_BREAK_IF( pthread_create( &pwave->wv_trds[ i ].alc_thread->trd_id , NULL , wave_runner , ( void * )pwave ) != PTHREAD_CREATE_OK , errGeneral , 0 , "thread creation failed" );
					
					// 4. etc
					if ( IF_VERBOSE_MODE_CONDITION() )
					{
						_ECHO( "create thread %lu" , pwave->wv_trds[ i ].alc_thread->trd_id );
					}
					//_g->stat.sender_thread_count++;
					diff_new_thread_count--;
					break;
				}
			}
		}
	}

	// retire extra thread
	if ( valid_thread_count > new_wcfg->m.m.maintained.parallelism_count )
	{
		// stop extra
		int extra_count = valid_thread_count - new_wcfg->m.m.maintained.parallelism_count;
		if ( IF_VERBOSE_MODE_CONDITION() )
		{
			_ECHO( "num retire extra thread %d" , extra_count );
		}
		while ( extra_count > 0 )
		{
			for ( int i = pwave->wv_trds_masks_count - 1 ; i >= 0 ; i-- ) // az yah hazf kon
			{
				if ( pwave->wv_trds_masks[ i ] && pwave->wv_trds[ i ].alc_thread->do_close_thread == 0 )
				{
					pwave->wv_trds[ i ].alc_thread->do_close_thread = 1;

					//if ( pthread_cancel( pwave->wv_trds[ i ].trd_id ) == 0 )
					//{
						//pwave->wv_trds[ i ].alc_thread->trd_id = 0;
						//_close_socket( &pwave->wv_trds[ i ].socket_id );
						
						extra_count--;
						//_g->stat.sender_thread_count--;
					//}
					break;
				}
			}
		}
	}

	
	// when we arrive at this point we sure that somethings is changed
	memcpy( &pwave->awcfg , new_wcfg , sizeof( struct wave_cfg ) );
	//pwave->awcfg.m.m.temp_data.wcfg_stabled = 1;
	new_wcfg->m.m.temp_data.wcfg_changed = 0;
	// Set every threads that their state must change
	for ( int i = 0 ; i < pwave->wv_trds_masks_count ; i++ )
	{
		if ( pwave->wv_trds_masks[ i ] )
		{
			pwave->wv_trds[ i ].alc_thread->base_config_change_applied = 1;
		}
	}

	// TODO . complete reverse on error

	BEGIN_RET
		case 3: {}
		case 2: {}
		case 1:
		{
			_g->stat.syscal_err_count++;
		}
	M_V_END_RET
}

void stop_wave( struct App_Data * _g , struct udp_wave * pwave )
{
	INIT_BREAKABLE_FXN();

	//if ( IF_VERBOSE_MODE_CONDITION() )
	//{
	//	_ECHO( "stop_wave" );
	//}

	pwave->awcfg.m.m.maintained.parallelism_count = 0;
	pwave->awcfg.m.m.temp_data.wcfg_changed = 1;
	apply_new_wave_config( _g , pwave , &pwave->awcfg );
	// TODO
}

void apply_wave_new_cfg_changes( struct App_Data * _g , struct wave_cfg * prev_wcfg , struct wave_cfg * new_wcfg )
{
	INIT_BREAKABLE_FXN();

	//if ( IF_VERBOSE_MODE_CONDITION() )
	//{
	//	_ECHO( "apply_wave_new_cfg_changes" );
	//}

	for ( int i = 0 ; i < _g->waves.wv_holders_masks_count ; i++ )
	{
		if ( _g->waves.wv_holders_masks[ i ] )
		{
			if ( memcmp( &_g->waves.wv_holders[ i ].alc_wave->awcfg.m.m.id , &prev_wcfg->m.m.id , sizeof( struct wave_cfg_id ) ) == 0 )
			{
				apply_new_wave_config( _g , _g->waves.wv_holders[ i ].alc_wave , new_wcfg );
			}
		}
	}
}

void remove_wave( struct App_Data * _g , struct wave_cfg * wcfg )
{
	INIT_BREAKABLE_FXN();

	//if ( IF_VERBOSE_MODE_CONDITION() )
	//{
	//	_ECHO( "remove_wave" );
	//}

	for ( int i = 0 ; i < _g->waves.wv_holders_masks_count ; i++ )
	{
		if ( _g->waves.wv_holders_masks[ i ] )
		{
			if ( memcmp( &_g->waves.wv_holders[ i ].alc_wave->awcfg.m.m.id , &wcfg->m.m.id , sizeof( struct wave_cfg_id ) ) == 0 )
			{
				stop_wave( _g , _g->waves.wv_holders[ i ].alc_wave );
				_g->waves.wv_holders_masks[ i ] = 0;
				DAC( _g->waves.wv_holders[ i ].alc_wave );
			}
		}
	}
}

#define PREALLOCAION_SIZE 10

void add_new_wave( struct App_Data * _g , struct wave_cfg * new_wcfg )
{
	INIT_BREAKABLE_FXN();

	//if ( IF_VERBOSE_MODE_CONDITION() )
	//{
	//	_ECHO( "add_new_wave" );
	//}

	int new_wcfg_placement_index = -1;
	while ( new_wcfg_placement_index < 0 ) // try to find one place for new wave
	{
		for ( int i = 0 ; i < _g->waves.wv_holders_masks_count ; i++ )
		{
			if ( !_g->waves.wv_holders_masks[ i ] )
			{
				new_wcfg_placement_index = i;
				break;
			}
		}
		if ( new_wcfg_placement_index < 0 )
		{
			int old_wv_holders_masks_count = _g->waves.wv_holders_masks_count;
			int new_wv_holders_masks_count = old_wv_holders_masks_count + PREALLOCAION_SIZE;

			M_BREAK_IF( ( _g->waves.wv_holders_masks = REALLOC( _g->waves.wv_holders_masks , new_wv_holders_masks_count * sizeof( int ) ) ) == REALLOC_ERR , errMemoryLow , 2 );
			MEMSET_ZERO( _g->waves.wv_holders_masks + old_wv_holders_masks_count , int , PREALLOCAION_SIZE );

			M_BREAK_IF( ( _g->waves.wv_holders = REALLOC( _g->waves.wv_holders , new_wv_holders_masks_count * sizeof( struct udp_wave_holder ) ) ) == REALLOC_ERR , errMemoryLow , 1 );
			MEMSET_ZERO( _g->waves.wv_holders + old_wv_holders_masks_count , struct udp_wave_holder , PREALLOCAION_SIZE );

			_g->waves.wv_holders_masks_count = new_wv_holders_masks_count;
		}
	}

	ASSERT( _g->waves.wv_holders[ new_wcfg_placement_index ].alc_wave == NULL );
	M_BREAK_IF( ( _g->waves.wv_holders[ new_wcfg_placement_index ].alc_wave = NEW( struct udp_wave ) ) == NEW_ERR , errMemoryLow , 0 );
	MEMSET_ZERO( _g->waves.wv_holders[ new_wcfg_placement_index ].alc_wave , struct udp_wave , 1 );
	_g->waves.wv_holders_masks[ new_wcfg_placement_index ] = 1;
	memcpy( &_g->waves.wv_holders[ new_wcfg_placement_index ].alc_wave->awcfg , new_wcfg , sizeof( struct wave_cfg ) );

	apply_wave_new_cfg_changes( _g , new_wcfg , new_wcfg );

	BEGIN_RET
		case 3: DAC( _g->waves.wv_holders );
		case 2: DAC( _g->waves.wv_holders_masks );
		case 1: _g->stat.syscal_err_count++;
	M_V_END_RET
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
	time_t prev_time = { 0 } , cur_time = { 0 };
	struct Config_ver temp_ver = { 0 };
	while ( 1 )
	{
		cur_time = time( NULL );

		if ( _g->appcfg._ver == NULL || difftime( cur_time , prev_time ) > 5 )
		{
			prev_time = cur_time;

			memset( buf , 0 , sizeof( buf ) );
			const char * config_ver_file_content = read_file( CONFIG_ROOT_PATH "/config_ver.txt" , ( char * )buf );
			MM_BREAK_IF( !config_ver_file_content , errGeneral , 0 , "cannot open and read version file" );

			result( json_element ) rs_config_ver = json_parse( config_ver_file_content );
			//free( ( void * )config_ver_file_content );
			MM_BREAK_IF( catch_error( &rs_config_ver , "config_ver" ) , errGeneral , 0 , "error in json_parse version file" );
			typed( json_element ) el_config_ver = result_unwrap( json_element )( &rs_config_ver );

			result( json_element ) ver = json_object_find( el_config_ver.value.as_object , "ver" );
			MM_BREAK_IF( catch_error( &ver , "ver" ) , errGeneral , 0 , "ver not found" );

			memset( &temp_ver , 0 , sizeof( temp_ver ) );

			strcpy( temp_ver.version , ( char * )ver.inner.value.value.as_string );
			char * perr = NULL;
			char * pver = ( char * )ver.inner.value.value.as_string;
			temp_ver.Major = ( int )strtol( strtok( pver , "." ) , &perr , 10 );			MM_BREAK_IF( !perr , errGeneral , 0 , "ver Major wrong" );
			temp_ver.Minor = ( int )strtol( strtok( NULL , "." ) , &perr , 10 );			MM_BREAK_IF( !perr , errGeneral , 0 , "ver Minor wrong" );
			temp_ver.Build = ( int )strtol( strtok( NULL , "." ) , &perr , 10 );			MM_BREAK_IF( !perr , errGeneral , 0 , "ver Build wrong" );
			temp_ver.Revision_Patch = ( int )strtol( strtok( NULL , "." ) , &perr , 10 );	MM_BREAK_IF( !perr , errGeneral , 0 , "ver Revision_Patch wrong" );
			json_free( &el_config_ver ); // string is user so must be free at the end

			if ( _g->appcfg._ver == NULL || strcmp( temp_ver.version , _g->appcfg._ver->version ) )
			{
				_g->appcfg._ver = ( struct Config_ver * )memcpy( &_g->appcfg.___temp_ver , &temp_ver , sizeof( temp_ver ) );
				_g->appcfg._version_changed = 1;
				if ( IF_VERBOSE_MODE_CONDITION() )
				{
					_ECHO( "version changed %s" , _g->appcfg._ver->version );
				}
			}
		}
		sleep( 1 );
	}
	BEGIN_RET
		case 3: {}
		case 2: {}
		case 1: _g->stat.syscal_err_count++;
	M_V_END_RET
	return VOID_RET;
}

// TODO . fix memory leak
// TODO . echo acceptible config one time to inform user
void * config_loader( void * app_data )
{
	INIT_BREAKABLE_FXN();

	struct App_Data * _g = ( struct App_Data * )app_data;
	time_t prev_time , cur_time;

	while ( !_g->appcfg._version_changed ) // load after version loaded
	{
		sleep( 1 );
	}

	typed( json_element ) el_UDP_generator_config;

	while ( 1 )
	{
		cur_time = time( NULL );

		if ( _g->appcfg._general_config == NULL || _g->appcfg._version_changed /* || difftime(cur_time , prev_time) > 15 * 60*/ )
		{
			prev_time = prev_time; // to ignore warning
			prev_time = cur_time;

			struct Global_Config temp_config = { 0 };
			struct Global_Config_0 * pGeneralConfiguration = ( struct Global_Config_0 * )&temp_config;
			struct wave_cfg * pWaves = NULL;
			size_t waves_count = 0;
			{
				const char * UDP_generator_config_file_content = read_file( CONFIG_ROOT_PATH "/UDP_generator_config.txt" , NULL );
				MM_BREAK_IF( !UDP_generator_config_file_content , errGeneral , 0 , "cannot open config file" );
				
				result( json_element ) rs_UDP_generator_config = json_parse( UDP_generator_config_file_content );
				free( ( void * )UDP_generator_config_file_content );
				MM_BREAK_IF( catch_error( &rs_UDP_generator_config , "UDP_generator_config" ) , errGeneral , 0 , "cannot parse config file" );
				el_UDP_generator_config = result_unwrap( json_element )( &rs_UDP_generator_config );

				/*configurations*/
				if ( _g->appcfg._ver->Major >= 1 ) // first version of config file structure
				{
					result( json_element ) re_configurations = json_object_find( el_UDP_generator_config.value.as_object , "configurations" );
					MM_BREAK_IF( catch_error( &re_configurations , "configurations" ) , errGeneral , 0 , "configurations" );
					typed( json_element ) el_configurations = result_unwrap( json_element )( &re_configurations );

#define CFG_ELEM_STR( name ) \
						result( json_element ) re_##name = json_object_find( el_configurations.value.as_object , #name );\
						M_BREAK_IF( catch_error( &re_##name , #name ) , errGeneral , 0 );\
						typed( json_element ) el_##name = result_unwrap( json_element )( &re_##name );\
						NEWSTR( pGeneralConfiguration->name , el_##name.value.as_string , 0 );
#define CFG_ELEM_I( name ) \
						result( json_element ) re_##name = json_object_find( el_configurations.value.as_object , #name );\
						M_BREAK_IF( catch_error( &re_##name , #name ) , errGeneral , 0 );\
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
					CFG_ELEM_I( verbose_mode );
					CFG_ELEM_I( hi_frequent_log_interval_sec );
					CFG_ELEM_I( refresh_variable_from_scratch );
					

#undef CFG_ELEM_I
#undef CFG_ELEM_STR
				}

				/*waves*/
				{
					result( json_element ) re_waves = json_object_find( el_UDP_generator_config.value.as_object , "waves" );
					MM_BREAK_IF( catch_error( &re_waves , "waves" ) , errGeneral , 0 , "waves" );
					typed( json_element ) el_waves = result_unwrap( json_element )( &re_waves );

					MM_BREAK_IF( ( waves_count = el_waves.value.as_object->count ) < 1 , errGeneral , 0 , "waves must be not zero" );

					M_BREAK_IF( !( pWaves = NEWBUF( struct wave_cfg , el_waves.value.as_object->count ) ) , errMemoryLow , 0 );
					MEMSET_ZERO( pWaves , struct wave_cfg , el_waves.value.as_object->count );
					pWaves->m.m.temp_data._g = ( void * )_g;

					for ( int i = 0 ; i < el_waves.value.as_object->count ; i++ )
					{
						char output_wave_name[ 32 ];
						memset( output_wave_name , 0 , sizeof( output_wave_name ) );
						sprintf( output_wave_name , "wave%d" , i + 1 );

						result( json_element ) re_output_wave = json_object_find( el_waves.value.as_object , output_wave_name );
						M_BREAK_IF( catch_error( &re_output_wave , output_wave_name ) , errGeneral , 0 );
						typed( json_element ) el_output_wave = result_unwrap( json_element )( &re_output_wave );

#define CFG_ELEM_STR( name ) \
							result( json_element ) re_##name = json_object_find( el_output_wave.value.as_object , #name );\
							M_BREAK_IF( catch_error( &re_##name , #name ) , errGeneral , 0 );\
							typed( json_element ) el_##name = result_unwrap( json_element )( &re_##name );\
							strcpy(((struct wave_cfg_0 *)(pWaves + i))->name , el_##name.value.as_string );

#define CFG_ID_ELEM_STR( name ) \
							result( json_element ) re_##name = json_object_find( el_output_wave.value.as_object , #name );\
							M_BREAK_IF( catch_error( &re_##name , #name ) , errGeneral , 0 );\
							typed( json_element ) el_##name = result_unwrap( json_element )( &re_##name );\
							strcpy(((struct wave_cfg_0 *)(pWaves + i))->id.name , el_##name.value.as_string );

#define CFG_ELEM_I_maintained( name ) \
							result( json_element ) re_##name = json_object_find( el_output_wave.value.as_object , #name );\
							M_BREAK_IF( catch_error( &re_##name , #name ) , errGeneral , 0 );\
							typed( json_element ) el_##name = result_unwrap( json_element )( &re_##name );\
							((struct wave_cfg_0 *)(pWaves + i))->maintained.name = (int)el_##name.value.as_number.value.as_long;

#define CFG_ELEM_I_momentary( name ) \
							result( json_element ) re_##name = json_object_find( el_output_wave.value.as_object , #name );\
							M_BREAK_IF( catch_error( &re_##name , #name ) , errGeneral , 0 );\
							typed( json_element ) el_##name = result_unwrap( json_element )( &re_##name );\
							((struct wave_cfg_0 *)(pWaves + i))->momentary.name = (int)el_##name.value.as_number.value.as_long;

#define CFG_ID_ELEM_I( name ) \
							result( json_element ) re_##name = json_object_find( el_output_wave.value.as_object , #name );\
							M_BREAK_IF( catch_error( &re_##name , #name ) , errGeneral , 0 );\
							typed( json_element ) el_##name = result_unwrap( json_element )( &re_##name );\
							((struct wave_cfg_0 *)(pWaves + i))->id.name = (int)el_##name.value.as_number.value.as_long;

						strcpy( ( ( struct wave_cfg_0 * )( pWaves + i ) )->id.wave_name , output_wave_name );

						CFG_ID_ELEM_STR( UDP_destination_ip );
						CFG_ID_ELEM_I( UDP_destination_port );
						CFG_ID_ELEM_STR( UDP_destination_interface );
						CFG_ELEM_I_maintained( limited_packets );
						CFG_ELEM_I_maintained( packet_count );
						CFG_ELEM_I_maintained( parallelism_count );
						CFG_ELEM_I_maintained( iteration_delay_milisec );
						CFG_ELEM_I_maintained( packet_payload_size );
						CFG_ELEM_I_maintained( enable );
						CFG_ELEM_I_momentary( reset_connections );

#undef CFG_ID_ELEM_I
#undef CFG_ELEM_I
#undef CFG_ID_ELEM_STR
#undef CFG_ELEM_STR
					}
				}

				json_free( &el_UDP_generator_config );
			}

			int initial_general_config = 0;
			if ( _g->appcfg._general_config == NULL ) // TODO . make assignemnt atomic
			{
				M_BREAK_IF( !( _g->appcfg._general_config = malloc( sizeof( struct Global_Config ) ) ) , errMemoryLow , 0 );

				memset( _g->appcfg._general_config , 0 , sizeof( struct Global_Config ) );
				memcpy( _g->appcfg._general_config , &temp_config , sizeof( temp_config ) );
				memset( &temp_config , 0 , sizeof( temp_config ) ); // copy to global variable and then zero to not free strings
				initial_general_config = 1;
				_g->appcfg._general_config_changed = 1;
			}
			if ( !initial_general_config )
			{
				DAC( _g->appcfg._prev_general_config );

				_g->appcfg._prev_general_config = _g->appcfg._general_config;
				_g->appcfg._general_config = NULL;

				M_BREAK_IF( !( _g->appcfg._general_config = malloc( sizeof( struct Global_Config ) ) ) , errMemoryLow , 0 );
				memset( _g->appcfg._general_config , 0 , sizeof( struct Global_Config ) );
				memcpy( _g->appcfg._general_config , &temp_config , sizeof( temp_config ) );
				memset( &temp_config , 0 , sizeof( temp_config ) ); // copy to global variable and then zero to not free strings

				if ( _g->appcfg._prev_general_config != NULL && _g->appcfg._general_config != NULL )
				{
					_g->appcfg._general_config_changed |= !!strcmp( _g->appcfg._general_config->c.c.create_date , _g->appcfg._prev_general_config->c.c.create_date );
					_g->appcfg._general_config_changed |= !!strcmp( _g->appcfg._general_config->c.c.modify_date , _g->appcfg._prev_general_config->c.c.modify_date );
					_g->appcfg._general_config_changed |= !!strcmp( _g->appcfg._general_config->c.c.config_name , _g->appcfg._prev_general_config->c.c.config_name );
					_g->appcfg._general_config_changed |= !!strcmp( _g->appcfg._general_config->c.c.config_tags , _g->appcfg._prev_general_config->c.c.config_tags );
					_g->appcfg._general_config_changed |= !!strcmp( _g->appcfg._general_config->c.c.description , _g->appcfg._prev_general_config->c.c.description );
					_g->appcfg._general_config_changed |= !!strcmp( _g->appcfg._general_config->c.c.log_level , _g->appcfg._prev_general_config->c.c.log_level );
					_g->appcfg._general_config_changed |= !!strcmp( _g->appcfg._general_config->c.c.log_file , _g->appcfg._prev_general_config->c.c.log_file );

					_g->appcfg._general_config_changed |= !( _g->appcfg._general_config->c.c.enable == _g->appcfg._prev_general_config->c.c.enable );
					_g->appcfg._general_config_changed |= !( _g->appcfg._general_config->c.c.shutdown == _g->appcfg._prev_general_config->c.c.shutdown );
					_g->appcfg._general_config_changed |= !( _g->appcfg._general_config->c.c.watchdog_enabled == _g->appcfg._prev_general_config->c.c.watchdog_enabled );
					_g->appcfg._general_config_changed |= !( _g->appcfg._general_config->c.c.load_prev_config == _g->appcfg._prev_general_config->c.c.load_prev_config );
					_g->appcfg._general_config_changed |= !( _g->appcfg._general_config->c.c.dump_current_config == _g->appcfg._prev_general_config->c.c.dump_current_config );
					_g->appcfg._general_config_changed |= !( _g->appcfg._general_config->c.c.dump_prev_config == _g->appcfg._prev_general_config->c.c.dump_prev_config );
					_g->appcfg._general_config_changed |= !( _g->appcfg._general_config->c.c.time_out_sec == _g->appcfg._prev_general_config->c.c.time_out_sec );

					_g->appcfg._general_config_changed |= !( _g->appcfg._general_config->c.c.verbose_mode == _g->appcfg._prev_general_config->c.c.verbose_mode );
					_g->appcfg._general_config_changed |= !( _g->appcfg._general_config->c.c.hi_frequent_log_interval_sec == _g->appcfg._prev_general_config->c.c.hi_frequent_log_interval_sec );
					_g->appcfg._general_config_changed |= !( _g->appcfg._general_config->c.c.refresh_variable_from_scratch == _g->appcfg._prev_general_config->c.c.refresh_variable_from_scratch );
				}
			}

			if ( _g->appcfg._general_config_changed )
			{
				if ( IF_VERBOSE_MODE_CONDITION() )
				{
					_ECHO( "general config changed" );
				}
			}

			if ( _g->appcfg._pwave_psvcfg )
			{
				DAC( _g->appcfg._pprev_wave_psvcfg );
				_g->appcfg._prev_wave_psvcfg_count = 0;
				_g->appcfg._pprev_wave_psvcfg = _g->appcfg._pwave_psvcfg;
				_g->appcfg._prev_wave_psvcfg_count = _g->appcfg._wave_psvcfg_count;
			}
			_g->appcfg._pwave_psvcfg = pWaves;
			_g->appcfg._wave_psvcfg_count = waves_count;
			pWaves = NULL; // to not delete intentionally
			waves_count = 0;

			if ( _g->appcfg._pprev_wave_psvcfg == NULL && _g->appcfg._pwave_psvcfg )
			{
				// all new ones
				_g->appcfg._psvcfg_changed = 1; // ham koli set mishavad change rokh dad
				for ( int i = 0 ; i < _g->appcfg._wave_psvcfg_count ; i++ )
				{
					_g->appcfg._pwave_psvcfg[ i ].m.m.temp_data.wcfg_changed = 1; // ham joz e set mishavad
				}
			}
			else if ( _g->appcfg._pprev_wave_psvcfg && _g->appcfg._pwave_psvcfg )
			{
				// from old perspective
				for ( int i = 0 ; i < _g->appcfg._prev_wave_psvcfg_count ; i++ )
				{
					int prev_exist = 0;
					for ( int j = 0 ; j < _g->appcfg._wave_psvcfg_count ; j++ )
					{
						if ( memcmp( &_g->appcfg._pprev_wave_psvcfg[ i ].m.m.id , &_g->appcfg._pwave_psvcfg[ j ].m.m.id , sizeof( struct wave_cfg_id ) ) == 0 )
						{
							prev_exist = 1;
							if ( memcmp( &_g->appcfg._pprev_wave_psvcfg[ i ].m.m.maintained , &_g->appcfg._pwave_psvcfg[ j ].m.m.maintained , sizeof( struct wave_maintained_parameter ) ) )
							{
								_g->appcfg._psvcfg_changed = 1;
								_g->appcfg._pwave_psvcfg[ j ].m.m.temp_data.wcfg_changed = 1;
							}
							break;
						}
					}
					if ( !prev_exist )
					{
						_g->appcfg._psvcfg_changed = 1;
						_g->appcfg._pprev_wave_psvcfg[ i ].m.m.temp_data.wcfg_changed = 1;
						break;
					}
				}
				// from new perspective
				for ( int j = 0 ; j < _g->appcfg._wave_psvcfg_count ; j++ )
				{
					int new_exist = 0;
					for ( int i = 0 ; i < _g->appcfg._prev_wave_psvcfg_count ; i++ )
					{
						if ( memcmp( &_g->appcfg._pprev_wave_psvcfg[ i ].m.m.id , &_g->appcfg._pwave_psvcfg[ j ].m.m.id , sizeof( struct wave_cfg_id ) ) == 0 )
						{
							new_exist = 1;
							if ( memcmp( &_g->appcfg._pprev_wave_psvcfg[ i ].m.m.maintained , &_g->appcfg._pwave_psvcfg[ j ].m.m.maintained , sizeof( struct wave_maintained_parameter ) ) )
							{
								_g->appcfg._psvcfg_changed = 1;
								_g->appcfg._pwave_psvcfg[ j ].m.m.temp_data.wcfg_changed = 1;
							}
							break;
						}
					}
					if ( !new_exist )
					{
						_g->appcfg._psvcfg_changed = 1;
						_g->appcfg._pwave_psvcfg[ j ].m.m.temp_data.wcfg_changed = 1;
						break;
					}
				}
			}

			_g->appcfg._version_changed = 0;

			if ( _g->appcfg._psvcfg_changed && IF_VERBOSE_MODE_CONDITION() )
			{
				_ECHO( "wave config changed" );
			}
		}
		sleep( 2 );
	}

	BEGIN_RET
		case 1:
		{
			json_free( &el_UDP_generator_config );
			break;
		}
	M_V_END_RET

	return NULL;
}

// TODO . aware of concurrency in config read and act on it
void * waves_manager( void * app_data )
{
	INIT_BREAKABLE_FXN();
	struct App_Data * _g = ( struct App_Data * )app_data;

	pthread_t trd_udp_connection , trd_tcp_connection;
	pthread_t trd_protocol_bridge;

	while ( !_g->appcfg._wave_psvcfg_count ) // load after any config loaded
	{
		sleep( 1 );
	}

	while ( 1 )
	{
		if ( _g->appcfg._general_config_changed )
		{
			_g->appcfg._general_config_changed = 0; // for now . TODO later
		}

		if ( _g->appcfg._psvcfg_changed )
		{
			for ( int i = 0 ; i < _g->appcfg._prev_wave_psvcfg_count ; i++ )
			{
				int exist = 0;
				for ( int j = 0 ; j < _g->appcfg._wave_psvcfg_count ; j++ )
				{
					if ( exist ) break;
					if ( memcmp( &_g->appcfg._pprev_wave_psvcfg[ i ].m.m.id , &_g->appcfg._pwave_psvcfg[ j ].m.m.id , sizeof( struct wave_cfg_id ) ) == 0 && _g->appcfg._pwave_psvcfg[ j ].m.m.temp_data.wcfg_changed )
					{
						// existed cfg changed
						apply_wave_new_cfg_changes( _g , &_g->appcfg._pprev_wave_psvcfg[ i ] , &_g->appcfg._pwave_psvcfg[ j ] );
						exist = 1;
					}
				}
				if ( !exist )
				{
					// remove removed one
					remove_wave( _g , &_g->appcfg._pprev_wave_psvcfg[ i ] );
				}
			}
			for ( int i = 0 ; i < _g->appcfg._wave_psvcfg_count ; i++ )
			{
				int exist = 0;
				for ( int j = 0 ; j < _g->appcfg._prev_wave_psvcfg_count ; j++ )
				{
					if ( memcmp( &_g->appcfg._pwave_psvcfg[ i ].m.m.id , &_g->appcfg._pprev_wave_psvcfg[ j ].m.m.id , sizeof( struct wave_cfg_id ) ) == 0 )
					{
						exist = 1;
						break;
					}
				}
				if ( !exist )
				{
					// start new cfg
					add_new_wave( _g , &_g->appcfg._pwave_psvcfg[ i ] );
				}
			}
			_g->appcfg._psvcfg_changed = 0; // changes applied
		}
		sleep( 5 );
	}
	return NULL;
}

#endif

#ifndef section_stat

pthread_mutex_t data_lock;

// Centered cell printing
void print_cell( WINDOW * win , int y , int x , int width , const char * text )
{
	size_t len = strlen( text );
	int pad = ( width - ( int )len ) / 2;
	if ( pad < 0 ) pad = 0;
	mvwprintw( win , y , x + pad , "%s" , text );
}

#define MAIN_STAT()  _g->stat
#define MAIN_WIN  MAIN_STAT().main_win

// Drawing the full table
void draw_table( struct App_Data * _g )
{
	char * header_border = "+----------+--------------------+";

	int cell_w = strlen( header_border ) / 2;
	int start_x = 2;
	int y = 1;

	// Top border
	mvwprintw( MAIN_WIN , y++ , start_x , header_border );

	// Header
	wattron( MAIN_WIN , COLOR_PAIR( 1 ) );
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "Metric" );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , "Value" );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );
	wattroff( MAIN_WIN , COLOR_PAIR( 1 ) );

	// Header border
	mvwprintw( MAIN_WIN , y++ , start_x , header_border );

	// Data rows
	wattron( MAIN_WIN , COLOR_PAIR( 2 ) );
	char buf[ 32 ];
		
	//
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "sender_thread_count" );
	snprintf( buf , sizeof( buf ) , "%d" , MAIN_STAT().sender_thread_count );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );

	//
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "total_sent_count" );
	snprintf( buf , sizeof( buf ) , "%llu" , MAIN_STAT().all_benchmarks_total_sent_count );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );

	//
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "sent_fail_count" );
	snprintf( buf , sizeof( buf ) , "%llu" , MAIN_STAT().all_benchmarks_total_sent_fail_count );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );
	
	//
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "total_sent_byte" );
	snprintf( buf , sizeof( buf ) , "%llu" , MAIN_STAT().all_benchmarks_total_sent_byte );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );

	//
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "failure_count" );
	snprintf( buf , sizeof( buf ) , "%llu" , MAIN_STAT().syscal_err_count );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );
	

	//// Memory
	//mvwprintw( MAIN_WIN , y , start_x , "|" );
	//print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "MemoryMB" );
	//snprintf( buf , sizeof( buf ) , "%.2f" , pdata->mem_usage );
	//mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	//print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	//mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );

	//// IO
	//mvwprintw( MAIN_WIN , y , start_x , "|" );
	//print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "001 IO KB/s" );
	//snprintf( buf , sizeof( buf ) , "%.2f" , pdata->io_rate );
	//mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	//print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	//mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );

	wattroff( MAIN_WIN , COLOR_PAIR( 2 ) );

	// Mid border
	mvwprintw( MAIN_WIN , y++ , start_x , header_border );

	// Last Command Row
	wattron( MAIN_WIN , COLOR_PAIR( 3 ) );
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "Last Cmd" );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , MAIN_STAT().last_command );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );
	wattroff( MAIN_WIN , COLOR_PAIR( 3 ) );

	// Bottom border
	mvwprintw( MAIN_WIN , y++ , start_x , header_border );
}

#undef MAIN_WIN

// Stats update thread
void * stats_thread( void * pdata )
{
	struct App_Data * _g = ( struct App_Data * )pdata;
	while ( 1 )
	{
		pthread_mutex_lock( &data_lock );

		werase( _g->stat.main_win );
		box( _g->stat.main_win , 0 , 0 );
		draw_table( _g );
		wrefresh( _g->stat.main_win );

		pthread_mutex_unlock( &data_lock );

		sleep( 1 );
	}
	return NULL;
}

// Input thread
void * input_thread( void * pdata )
{
	struct App_Data * _g = ( struct App_Data * )pdata;
	while ( 1 )
	{
		pthread_mutex_lock( &data_lock );

		werase( _g->stat.input_win );
		box( _g->stat.input_win , 0 , 0 );

		pthread_mutex_unlock( &data_lock );

		// Enable echo and get input
		echo();
		curs_set( 1 );
		wmove( _g->stat.input_win , 1 , 1 );
		wprintw( _g->stat.input_win , "cmd(quit to exit): " );
		wrefresh( _g->stat.input_win );
		wgetnstr( _g->stat.input_win , _g->stat.input_buffer , INPUT_MAX - 1 );
		noecho();
		curs_set( 0 );

		pthread_mutex_lock( &data_lock );

		strncpy( _g->stat.last_command , _g->stat.input_buffer , INPUT_MAX - 1 );
		_g->stat.last_command[ INPUT_MAX - 1 ] = EOS;

		pthread_mutex_unlock( &data_lock );
	}
	return NULL;
}

void init_windows( struct App_Data * _g )
{
	//int maxy, maxx;
	getmaxyx( stdscr , _g->stat.scr_height , _g->stat.scr_width );

	// Calculate window sizes (60% for cells, 40% for input)
	int stats_height = _g->stat.scr_height - 3;
	int input_height = 3;

	// Create or replace windows
	if ( _g->stat.main_win ) delwin( _g->stat.main_win );
	if ( _g->stat.input_win ) delwin( _g->stat.input_win );

	_g->stat.main_win = newwin( stats_height , _g->stat.scr_width , 0 , 0 );
	_g->stat.input_win = newwin( input_height , _g->stat.scr_width , stats_height , 0 );

	// Enable scrolling and keypad for input window
	scrollok( _g->stat.main_win , TRUE );
	keypad( _g->stat.input_win , TRUE );

	// Set box borders
	box( _g->stat.main_win , 0 , 0 );
	box( _g->stat.input_win , 0 , 0 );

	// Refresh windows
	wrefresh( _g->stat.main_win );
	wrefresh( _g->stat.input_win );
}

#endif

#ifndef section_stderr

#define BUF_SIZE 2048

void * stdout_bypass_thread( void * pdata )
{
	struct App_Data * _g = ( struct App_Data * )pdata;
	fd_set readfds;
	char buffer[ BUF_SIZE ];
	while ( 1 )
	{
		FD_ZERO( &readfds );
		FD_SET( _g->stat.pipefds[ 0 ] , &readfds );

		int ready = select( _g->stat.pipefds[ 0 ] + 1 , &readfds , NULL , NULL , NULL );
		if ( ready > 0 && FD_ISSET( _g->stat.pipefds[ 0 ] , &readfds ) )
		{
			int n = read( _g->stat.pipefds[ 0 ] , buffer , BUF_SIZE - 1 );
			buffer[ n ] = EOS;
			strncpy( _g->stat.last_command , buffer , sizeof( _g->stat.last_command ) - 1 );
		}
	}
	return NULL;
}

void init_bypass_stdout( struct App_Data * _g )
{
	//int pipefd[ 2 ];
	//memset( pipefd , 0 , sizeof( pipefd ) );

	// Make pipe
	pipe( _g->stat.pipefds );

	// Redirect stdout
	//fflush( stdout );
	dup2( _g->stat.pipefds[ 1 ] , fileno( stderr ) );

	pthread_t tid_stdout_bypass;
	pthread_create( &tid_stdout_bypass , NULL , stdout_bypass_thread , ( void * )_g );
}

struct App_Data * __g;

void M_showMsg( const char * msg ) 
{
	if ( __g ) strcpy( __g->stat.last_command , msg );
}

#endif

#ifndef section_main

int main()
{
	INIT_BREAKABLE_FXN();
	struct App_Data _g = { 0 };
	__g = &_g;

	// Initialize curses
	initscr();
	start_color();
	cbreak();
	noecho();
	curs_set( 1 );

	init_pair( 1 , COLOR_WHITE , COLOR_BLUE );   // Header
	init_pair( 2 , COLOR_GREEN , COLOR_BLACK );  // Data
	init_pair( 3 , COLOR_YELLOW , COLOR_BLACK ); // Last Command

	pthread_mutex_init( &data_lock , NULL );

	// Initial window creation
	init_windows( &_g );

	init_bypass_stdout( &_g );

	pthread_t tid_stats , tid_input;
	pthread_create( &tid_stats , NULL , stats_thread , ( void * )&_g );
	pthread_create( &tid_input , NULL , input_thread , ( void * )&_g );

	pthread_t trd_version_checker;
	MM_BREAK_IF( pthread_create( &trd_version_checker , NULL , version_checker , ( void * )&_g ) != PTHREAD_CREATE_OK , errGeneral , 0 , "Failed to create thread" );

	pthread_t trd_config_loader;
	MM_BREAK_IF( pthread_create( &trd_config_loader , NULL , config_loader , ( void * )&_g ) != PTHREAD_CREATE_OK , errGeneral , 0 , "Failed to create thread" );

	pthread_t trd_waves_manager;
	MM_BREAK_IF( pthread_create( &trd_waves_manager , NULL , waves_manager , ( void * )&_g ) != PTHREAD_CREATE_OK , errGeneral , 0 , "Failed to create thread" );

	M_BREAK_IF( pthread_join( trd_waves_manager , NULL ) != PTHREAD_JOIN_OK , errGeneral , 0 );

	return 0;
	BEGIN_RET
		case 2: {}
		case 1: {}
		case 0:
		{
			__g->stat.syscal_err_count++;
		}
	M_V_END_RET
	return 1;
}

#endif
