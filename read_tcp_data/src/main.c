#ifndef section_include

#define Uses_json
#define Uses_fd_set
#define Uses_thrd_sleep
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
#define Uses_fileno
#define Uses_setlocale

#define DIRECT_ECHO_BUF _g->stat.last_command

#include <read_tcp_data.dep>

#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wsign-conversion"

#define VERBOSE_MODE_DEFAULT 0
#define HI_FREQUENT_LOG_INTERVAL_SEC_DEFAULT 5
#define STAT_REFERESH_INTERVAL_SEC_DEFUALT 1
#define CLOSE_APP_VAR_DEFAULT 0

#define IF_VERBOSE_MODE_CONDITION() ( _g->appcfg._general_config ? _g->appcfg._general_config->c.c.verbose_mode : VERBOSE_MODE_DEFAULT )
#define HI_FREQUENT_LOG_INTERVAL ( _g->appcfg._general_config ? _g->appcfg._general_config->c.c.hi_frequent_log_interval_sec : HI_FREQUENT_LOG_INTERVAL_SEC_DEFAULT )

#define STAT_REFERESH_INTERVAL_SEC() ( _g->appcfg._general_config ? _g->appcfg._general_config->c.c.stat_referesh_interval_sec : STAT_REFERESH_INTERVAL_SEC_DEFUALT )
#define CLOSE_APP_VAR() ( _g->cmd.quit_app )

#define RETRY_UNEXPECTED_WAIT_FOR_SOCK() ( _g->appcfg._general_config ? _g->appcfg._general_config->c.c.retry_unexpected_wait_for_sock : 3 )

#define FXN_HIT_COUNT 5000
#define PC_COUNT 10 // first for hit count and last alwayz zero

int __FXN_HIT[ FXN_HIT_COUNT ][ PC_COUNT ] = { 0 }; // max size is about number of code lines
static int _pc = 1; // step of each call globally

#define SYS_ALIVE_CHECK() do {\
	int __function_line = __LINE__; _g->stat.last_line_meet = __LINE__; _g->stat.alive_check_counter = ( _g->stat.alive_check_counter + 1 ) % 10; __FXN_HIT[__function_line][0]++; static int pc = 0;/*each line hit*/ if ( pc <= PC_COUNT-1 ) __FXN_HIT[__function_line][1+pc++] = _pc++; \
	} while(0)

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
	int stat_referesh_interval_sec;

	int synchronization_min_wait;
	int synchronization_max_roundup;
	int show_line_hit;
	int retry_unexpected_wait_for_sock;
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

#ifndef section_listener_config

struct tcp_listener_cfg_id
{
	char tcp_listener_name[ 64 ];

	char TCP_listen_ip[ 64 ];
	int TCP_listen_port;
	char TCP_listen_interface[ 128 ];
};

struct tcp_listener_maintained_parameter // stays in position
{
	int enable;
};

struct tcp_listener_momentary_parameter // popup automatically
{
	int reset_connections;
};

struct tcp_listener_temp_data
{
	void * _g; // just point to the main g
	int tcfg_changed; // in passive cfg and active cfg that in alive tcp_listener, in both it means something changed
	//int wcfg_stabled; // after change happened in action thread cfg must be stablesh before doing any action
};

struct tcp_listener_cfg_0
{
	struct tcp_listener_cfg_id id; // must be uniq for each tcp_listener
	struct tcp_listener_maintained_parameter maintained;
	struct tcp_listener_momentary_parameter momentary;
	struct tcp_listener_temp_data temp_data;
};

struct tcp_listener_cfg_n
{
	struct tcp_listener_cfg_0 m; // first member , n - 1
};

struct tcp_listener_cfg // finalizer
{
	struct tcp_listener_cfg_n m; // be first member
};

#endif

#ifndef section_listeners

struct tcp_listener_thread
{
	pthread_t trd_id;
	int base_config_change_applied;
	int do_close_thread; // command from outside to inside thread

	struct tcp_listener * tl; // point to upper wave
};

struct tcp_listener_thread_holder
{
	struct tcp_listener_thread * alc_thread;
};

struct tcp_listener
{
	struct tcp_listener_cfg tlcfg;  // copy of applied cfg . active tcp_listener config . یک دسته کامل از ترد ها یک کانفیگ را اعمال می کند

	int tcp_server_listener_sockfd;
	int tcp_client_connection_sockfd;
	int tcp_connection_established; // tcp connection established

	int retry_to_connect_tcp;

	struct tcp_listener_thread_holder * tl_trds; // tcp_listener threads . in protocol listener app must be one
	int * tl_trds_masks;  // each int represent thread is valid
	size_t tl_trds_masks_count;  // thread keeper count
};

struct tcp_listener_holder // every elemnt at the reallocation array must have holder because reallocate change data values but with holder just pointer addr change
{
	struct tcp_listener * alc_tl; // allocated
};

// TODO . IPv6
struct tcp_listener_holders
{
	struct tcp_listener_holder * tl_holders; // all the active tcp_listener
	int * tl_holders_masks; // tcp_listener masks
	size_t tl_holders_masks_count; // mask count
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

	// tcp_listener 
	struct tcp_listener_cfg * _pprev_tcp_listener_psvcfg; // maybe later in some condition we need to rollback to prev config
	size_t _prev_tcp_listener_psvcfg_count;

	struct tcp_listener_cfg * _ptcp_listener_psvcfg; // passive config
	size_t _tcp_listener_psvcfg_count;

	int _psvcfg_changed; // act like bool . something is changed
};

#define INPUT_MAX 256

struct tcp_stat_1_sec
{
	time_t t_tcp_throughput;

	__int64u calc_throughput_tcp_get_count;
	__int64u calc_throughput_tcp_get_bytes;

	__int64u tcp_get_count_throughput;
	__int64u tcp_get_byte_throughput;
};

struct tcp_stat_10_sec
{
	time_t t_tcp_throughput;

	__int64u calc_throughput_tcp_get_count;
	__int64u calc_throughput_tcp_get_bytes;

	__int64u tcp_get_count_throughput;
	__int64u tcp_get_byte_throughput;
};

struct tcp_stat_40_sec
{
	time_t t_tcp_throughput;

	__int64u calc_throughput_tcp_get_count;
	__int64u calc_throughput_tcp_get_bytes;

	__int64u tcp_get_count_throughput;
	__int64u tcp_get_byte_throughput;
};

struct tcp_stat
{
	__int64u total_tcp_get_count;
	__int64u total_tcp_get_byte;

	__int64u continuously_unsuccessful_select_on_open_port_count; // that canot get find data
};

struct statistics_lock_data
{
	pthread_mutex_t lock;
};

struct BenchmarkRound
{
	// err
	int continuously_unsuccessful_receive_error;
	int total_unsuccessful_receive_error;

	__int64u syscal_err_count;

	struct tcp_stat_1_sec tcp_1_sec;
	struct tcp_stat_10_sec tcp_10_sec;
	struct tcp_stat_40_sec tcp_40_sec;

	struct tcp_stat tcp;
};

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
	int last_line_meet;
	int alive_check_counter;
	struct statistics_lock_data lock_data;
	
	int tcp_connection_count;
	int total_retry_tcp_connection_count;

	struct BenchmarkRound round;
};

struct synchronization_data
{
	//pthread_mutex_t mutex;
	//pthread_cond_t cond;
	//int lock_in_progress;
	//int reset_static_after_lock;
};

struct app_cmd
{
	int quit_app;
};

struct App_Data
{
	struct App_Config appcfg;
	struct tcp_listener_holders listeners;
	struct statistics stat;
	struct synchronization_data sync;
	struct app_cmd cmd;
};

#endif

#ifndef section_connection_functions

#define PARALLELISM_COUNT 1

int _connect_tcp( struct tcp_listener * src_tl )
{
	INIT_BREAKABLE_FXN();

	struct App_Data * _g = ( struct App_Data * )src_tl->tlcfg.m.m.temp_data._g;

	while ( 1 )
	{
		SYS_ALIVE_CHECK();
		if ( src_tl->tl_trds->alc_thread->do_close_thread )
		{
			break;
		}

		MM_BREAK_IF( ( src_tl->tcp_server_listener_sockfd = socket( AF_INET , SOCK_STREAM , 0 ) ) == FXN_SOCKET_ERR , errGeneral , 1 , "create sock error" );

		int opt = 1;
		if (setsockopt(src_tl->tcp_server_listener_sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
			_VERBOSE_ECHO( "setsockopt error" );
			_close_socket( &src_tl->tcp_server_listener_sockfd );
			sleep(1);
			continue;
		}
		//int opt = 1;
		if (setsockopt(src_tl->tcp_server_listener_sockfd, SOL_SOCKET, 15, &opt, sizeof(opt)) < 0) {
			_VERBOSE_ECHO( "setsockopt error" );
			_close_socket( &src_tl->tcp_server_listener_sockfd );
			sleep(1);
			continue;
		}

		struct sockaddr_in server_addr;
		socklen_t addrlen = sizeof( server_addr );
		memset( &server_addr , 0 , sizeof( server_addr ) );
		server_addr.sin_family = AF_INET;
		if ( strcmp( src_tl->tlcfg.m.m.id.TCP_listen_ip , "INADDR_ANY" ) == 0 )
		{
			server_addr.sin_addr.s_addr = INADDR_ANY;
		}
		else
		{
			server_addr.sin_addr.s_addr = inet_addr( src_tl->tlcfg.m.m.id.TCP_listen_ip ); // Specify the IP address to bind to
		}
		server_addr.sin_port = htons( src_tl->tlcfg.m.m.id.TCP_listen_port );

		if ( bind( src_tl->tcp_server_listener_sockfd , ( const struct sockaddr * )&server_addr , sizeof( server_addr ) ) == FXN_BIND_ERR )
		{
			_VERBOSE_ECHO( "bind error" );
			_close_socket( &src_tl->tcp_server_listener_sockfd );
			sleep(1);
			continue;
		}

		if ( listen( src_tl->tcp_server_listener_sockfd , 1 ) < 0 )
		{
			_VERBOSE_ECHO( "listen error" );
			_close_socket( &src_tl->tcp_server_listener_sockfd );
			sleep(1);
			continue;
		}

		if ( ( src_tl->tcp_client_connection_sockfd = accept( src_tl->tcp_server_listener_sockfd , ( struct sockaddr * )&server_addr , ( socklen_t * )&addrlen ) ) < 0 )
		{
			_VERBOSE_ECHO( "accept error" );
			_close_socket( &src_tl->tcp_server_listener_sockfd );
			sleep(1);
			continue;
		}
		src_tl->tcp_connection_established = 1;

		_g->stat.tcp_connection_count++;
		_g->stat.total_retry_tcp_connection_count++;
		SYS_ALIVE_CHECK();

		return 1;
	}

	BEGIN_RET
		case 3: {}
		case 2: {}
		case 1: {}
	M_V_END_RET
	return -1;
}

void * thread_tcp_connection_proc( void * src_tl )
{
	INIT_BREAKABLE_FXN();

	struct tcp_listener * tl = ( struct tcp_listener * )src_tl;
	struct App_Data * _g = ( struct App_Data * )tl->tlcfg.m.m.temp_data._g;

	if ( IF_VERBOSE_MODE_CONDITION() )
	{
		_ECHO( "try to connect outbound tcp connection" );
	}

	if ( tl->tcp_connection_established )
	{
		SYS_ALIVE_CHECK();
		_close_socket( &tl->tcp_client_connection_sockfd );
		_close_socket( &tl->tcp_server_listener_sockfd );
		tl->tcp_connection_established = 0;
		_g->stat.tcp_connection_count--;
	}

	if ( _connect_tcp( tl ) == 0 )
	{

	}
	BREAK_OK( 0 ); // to just ignore gcc warning

	BEGIN_RET
		case 3: {}
		case 2: {}
		case 1: {}
	M_V_END_RET
	return NULL; // Threads can return a value, but this example returns NULL
}

#define BUFFER_SIZE MAX_PACKET_SIZE

void * tcp_listener_runner( void * src_tl )
{
	INIT_BREAKABLE_FXN();
	struct tcp_listener * tl = ( struct tcp_listener * )src_tl;
	struct App_Data * _g = tl->tlcfg.m.m.temp_data._g;
	SYS_ALIVE_CHECK();

	pthread_t tid = pthread_self();
	time_t tnow = 0;
	struct tcp_listener_thread * pthread = NULL;
	while ( pthread == NULL )
	{
		for ( int i = 0 ; i < tl->tl_trds_masks_count ; i++ )
		{
			if ( tl->tl_trds_masks[ i ] )
			{
				if ( tl->tl_trds[ i ].alc_thread->trd_id == tid )
				{
					pthread = tl->tl_trds[ i ].alc_thread;
					break;
				}
			}
		}
	}
	if ( pthread == NULL )
	{
		_g->stat.round.syscal_err_count++;
		return NULL;
	}

	//_g->stat.sender_thread_count++;

	if ( IF_VERBOSE_MODE_CONDITION() )
	{
		_ECHO( "thread started" );
	}

	char buffer[ BUFFER_SIZE ];
	ssize_t bytes_received;
	fd_set readfds; // Set of socket descriptors

	int socket_error_tolerance_count = 0; // restart socket after many error accur

	int config_changes = 0;
	int reconnect_tcp = 1;

	pthread->base_config_change_applied = 0;
	do
	{
		SYS_ALIVE_CHECK();
		if ( pthread->do_close_thread )
		{
			break;
		}

		config_changes = 0;

		if ( reconnect_tcp )
		{
			pthread_t trd_tcp_connection;
			MM_BREAK_IF( pthread_create( &trd_tcp_connection , NULL , thread_tcp_connection_proc , tl ) != PTHREAD_CREATE_OK , errGeneral , 0 , "thread creation failed" );
		}
		reconnect_tcp = 0;

		while ( 1 )
		{
			SYS_ALIVE_CHECK();
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
			if ( !tl->tcp_connection_established )
			{
				sleep( 1 );
				continue;
			}
			if ( tl->retry_to_connect_tcp )
			{
				tl->retry_to_connect_tcp = 0;
				reconnect_tcp = 1;
				break;
			}

			// Clear the socket set
			FD_ZERO( &readfds );
			FD_SET( tl->tcp_client_connection_sockfd , &readfds );

			tnow = time( NULL );

			// tcp
			if ( difftime( tnow , _g->stat.round.tcp_1_sec.t_tcp_throughput ) >= 1.0 )
			{
				if ( _g->stat.round.tcp_1_sec.t_tcp_throughput > 0 )
				{
					_g->stat.round.tcp_1_sec.tcp_get_count_throughput = _g->stat.round.tcp_1_sec.calc_throughput_tcp_get_count;
					_g->stat.round.tcp_1_sec.tcp_get_byte_throughput = _g->stat.round.tcp_1_sec.calc_throughput_tcp_get_bytes;
				}
				_g->stat.round.tcp_1_sec.t_tcp_throughput = tnow;
				_g->stat.round.tcp_1_sec.calc_throughput_tcp_get_count = 0;
				_g->stat.round.tcp_1_sec.calc_throughput_tcp_get_bytes = 0;
			}
			if ( difftime( tnow , _g->stat.round.tcp_10_sec.t_tcp_throughput ) >= 10.0 )
			{
				if ( _g->stat.round.tcp_10_sec.t_tcp_throughput > 0 )
				{
					_g->stat.round.tcp_10_sec.tcp_get_count_throughput = _g->stat.round.tcp_10_sec.calc_throughput_tcp_get_count;
					_g->stat.round.tcp_10_sec.tcp_get_byte_throughput = _g->stat.round.tcp_10_sec.calc_throughput_tcp_get_bytes;
				}
				_g->stat.round.tcp_10_sec.t_tcp_throughput = tnow;
				_g->stat.round.tcp_10_sec.calc_throughput_tcp_get_count = 0;
				_g->stat.round.tcp_10_sec.calc_throughput_tcp_get_bytes = 0;
			}
			if ( difftime( tnow , _g->stat.round.tcp_40_sec.t_tcp_throughput ) >= 40.0 )
			{
				if ( _g->stat.round.tcp_40_sec.t_tcp_throughput > 0 )
				{
					_g->stat.round.tcp_40_sec.tcp_get_count_throughput = _g->stat.round.tcp_40_sec.calc_throughput_tcp_get_count;
					_g->stat.round.tcp_40_sec.tcp_get_byte_throughput = _g->stat.round.tcp_40_sec.calc_throughput_tcp_get_bytes;
				}
				_g->stat.round.tcp_40_sec.t_tcp_throughput = tnow;
				_g->stat.round.tcp_40_sec.calc_throughput_tcp_get_count = 0;
				_g->stat.round.tcp_40_sec.calc_throughput_tcp_get_bytes = 0;
			}

			struct timeval timeout; //// Set timeout (e.g., 5 seconds)
			timeout.tv_sec = ( socket_error_tolerance_count + 1 ) * 2;
			timeout.tv_usec = 0;
			SYS_ALIVE_CHECK();
			int activity = select( tl->tcp_client_connection_sockfd + 1 , &readfds , NULL , NULL , &timeout );
			SYS_ALIVE_CHECK();

			if ( ( activity < 0 ) /* && ( errno != EINTR )*/ )
			{
				int error = 0;
				socklen_t errlen = sizeof( error );
				getsockopt( tl->tcp_client_connection_sockfd , SOL_SOCKET , SO_ERROR , &error , &errlen );
				if ( error != 0 )
				{
					_VERBOSE_ECHO( "Socket error: %d\n" , error );
				}

				if ( ++socket_error_tolerance_count > RETRY_UNEXPECTED_WAIT_FOR_SOCK() )
				{
					socket_error_tolerance_count = 0;
					for ( int i = 0 ; i < _g->listeners.tl_holders_masks_count ; i++ )
					{
						if ( _g->listeners.tl_holders_masks[ i ] )
						{
							if ( _g->listeners.tl_holders[ i ].alc_tl->tcp_connection_established ) // all the connected udp stoped or die so restart them
							{
								//if ( FD_ISSET( _g->bridges.pb_holders[ i ].alc_pb->udp_sockfd , &readfds ) )
								{
									_g->listeners.tl_holders[ i ].alc_tl->retry_to_connect_tcp = 1;
									break;
								}
							}
						}
					}
				}

				continue;
			}
			if ( activity == 0 )
			{
				int error = 0;
				socklen_t errlen = sizeof( error );
				getsockopt( tl->tcp_client_connection_sockfd , SOL_SOCKET , SO_ERROR , &error , &errlen );
				if ( error != 0 )
				{
					_VERBOSE_ECHO( "Socket error: %d\n" , error );
				}

				if ( ++socket_error_tolerance_count > RETRY_UNEXPECTED_WAIT_FOR_SOCK() )
				{
					socket_error_tolerance_count = 0;
					for ( int i = 0 ; i < _g->listeners.tl_holders_masks_count ; i++ )
					{
						if ( _g->listeners.tl_holders_masks[ i ] )
						{
							if ( _g->listeners.tl_holders[ i ].alc_tl->tcp_connection_established ) // all the connected udp stoped or die so restart them
							{
								//if ( FD_ISSET( _g->bridges.pb_holders[ i ].alc_pb->udp_sockfd , &readfds ) )
								{
									_g->listeners.tl_holders[ i ].alc_tl->retry_to_connect_tcp = 1;
									break;
								}
							}
						}
					}
				}

				continue;
			}

			_g->stat.round.tcp.continuously_unsuccessful_select_on_open_port_count = 0;

			if ( FD_ISSET( tl->tcp_client_connection_sockfd , &readfds ) )
			{
				bytes_received = recv( tl->tcp_client_connection_sockfd , buffer , BUFFER_SIZE - 1 , 0 );
				if ( bytes_received <= 0 )
				{
					_g->stat.round.continuously_unsuccessful_receive_error++;
					_g->stat.round.total_unsuccessful_receive_error++;

					if ( ++socket_error_tolerance_count > RETRY_UNEXPECTED_WAIT_FOR_SOCK() )
					{
						socket_error_tolerance_count = 0;
						for ( int i = 0 ; i < _g->listeners.tl_holders_masks_count ; i++ )
						{
							if ( _g->listeners.tl_holders_masks[ i ] )
							{
								if ( _g->listeners.tl_holders[ i ].alc_tl->tcp_connection_established ) // all the connected udp stoped or die so restart them
								{
									//if ( FD_ISSET( _g->bridges.pb_holders[ i ].alc_pb->udp_sockfd , &readfds ) )
									{
										_g->listeners.tl_holders[ i ].alc_tl->retry_to_connect_tcp = 1;
										break;
									}
								}
							}
						}
						continue;
					}
				}
				_g->stat.round.continuously_unsuccessful_receive_error = 0;
				if ( bytes_received > 0 )
				{
					_g->stat.round.tcp.total_tcp_get_count++;
					_g->stat.round.tcp.total_tcp_get_byte += bytes_received;
					_g->stat.round.tcp_1_sec.calc_throughput_tcp_get_count++;
					_g->stat.round.tcp_1_sec.calc_throughput_tcp_get_bytes += bytes_received;
					_g->stat.round.tcp_10_sec.calc_throughput_tcp_get_count++;
					_g->stat.round.tcp_10_sec.calc_throughput_tcp_get_bytes += bytes_received;
					_g->stat.round.tcp_40_sec.calc_throughput_tcp_get_count++;
					_g->stat.round.tcp_40_sec.calc_throughput_tcp_get_bytes += bytes_received;
				}
			}

		} // loop while ( 1 )

		//DAC( buffer );

	} while ( config_changes || reconnect_tcp );

	//_g->stat.sender_thread_count--;

	//// delete mask
	for ( int i = 0 ; i < pthread->tl->tl_trds_masks_count ; i++ )
	{
		if ( pthread->tl->tl_trds_masks[ i ] && pthread->tl->tl_trds[ i ].alc_thread->trd_id == tid )
		{
			pthread->tl->tl_trds_masks[ i ] = 0;
			break;
		}
	}

	SYS_ALIVE_CHECK();

	BEGIN_RET
		case 3: {}
		case 2: {}
		case 1:
		{
			//_close_socket( &src_tl->tcp_sockfd );
			_g->stat.round.syscal_err_count++;
		}
	M_V_END_RET
	return NULL;
}

void apply_new_tcp_listener_config( struct App_Data * _g , struct tcp_listener * tl , struct tcp_listener_cfg * new_tlcfg )
{
	INIT_BREAKABLE_FXN();

	if ( !new_tlcfg->m.m.maintained.enable )
	{
		for ( int i = 0 ; i < tl->tl_trds_masks_count ; i++ )
		{
			if ( tl->tl_trds_masks[ i ] )
			{
				tl->tl_trds->alc_thread->do_close_thread = 1;
			}
		}
		return;
	}

	// make extra space for new one
	if ( tl->tl_trds_masks_count < PARALLELISM_COUNT )
	{
		int old_thread_count = tl->tl_trds_masks_count;
		int diff_count = ( PARALLELISM_COUNT - tl->tl_trds_masks_count );
		int new_thread_count = old_thread_count + diff_count;

		M_BREAK_IF( ( tl->tl_trds_masks = REALLOC( tl->tl_trds_masks , new_thread_count * sizeof( int ) ) ) == REALLOC_ERR , errMemoryLow , 0 );
		MEMSET_ZERO( tl->tl_trds_masks + old_thread_count , int , diff_count );

		M_BREAK_IF( ( tl->tl_trds = REALLOC( tl->tl_trds , new_thread_count * sizeof( struct tcp_listener_thread_holder ) ) ) == REALLOC_ERR , errMemoryLow , 0 );
		MEMSET_ZERO( tl->tl_trds + old_thread_count , struct tcp_listener_thread_holder , diff_count );

		tl->tl_trds_masks_count = new_thread_count;
	}

	// create extra needed thread and socket for each of them
	int valid_thread_count = 0;
	for ( int i = 0 ; i < tl->tl_trds_masks_count ; i++ )
	{
		if ( tl->tl_trds_masks[ i ] ) valid_thread_count++;
	}
	// must add new thread to active wave
	if ( valid_thread_count < PARALLELISM_COUNT )
	{
		// run new one
		int diff_new_thread_count = PARALLELISM_COUNT - valid_thread_count;

		if ( IF_VERBOSE_MODE_CONDITION() )
		{
			_ECHO( "num new thread %d" , diff_new_thread_count );
		}
		while ( diff_new_thread_count > 0 )
		{
			for ( int i = 0 ; i < tl->tl_trds_masks_count ; i++ )
			{
				if ( !tl->tl_trds_masks[ i ] )
				{
					// 2. set mask
					tl->tl_trds_masks[ i ] = 1; // order is matter

					DAC( tl->tl_trds[ i ].alc_thread );
					M_BREAK_IF( ( tl->tl_trds[ i ].alc_thread = NEW( struct tcp_listener_thread ) ) == NEW_ERR , errMemoryLow , 0 );
					MEMSET_ZERO( tl->tl_trds[ i ].alc_thread , struct tcp_listener_thread , 1 );
					tl->tl_trds[ i ].alc_thread->tl = tl;

					// 3. create thread also
					MM_BREAK_IF( pthread_create( &tl->tl_trds[ i ].alc_thread->trd_id , NULL , tcp_listener_runner , ( void * )tl ) != PTHREAD_CREATE_OK , errGeneral , 0 , "thread creation failed" );

					// 4. etc
					if ( IF_VERBOSE_MODE_CONDITION() )
					{
						_ECHO( "create thread %lu" , tl->tl_trds[ i ].alc_thread->trd_id );
					}
					//_g->stat.sender_thread_count++;
					diff_new_thread_count--;
					break;
				}
			}
		}
	}

	// retire extra thread
	if ( valid_thread_count > PARALLELISM_COUNT )
	{
		// stop extra
		int extra_count = valid_thread_count - PARALLELISM_COUNT;
		if ( IF_VERBOSE_MODE_CONDITION() )
		{
			_ECHO( "num retire extra thread %d" , extra_count );
		}
		while ( extra_count > 0 )
		{
			for ( int i = tl->tl_trds_masks_count - 1 ; i >= 0 ; i-- ) // az yah hazf kon
			{
				if ( tl->tl_trds_masks[ i ] && tl->tl_trds[ i ].alc_thread->do_close_thread == 0 )
				{
					tl->tl_trds[ i ].alc_thread->do_close_thread = 1;

					//if ( pthread_cancel( pwave->tl_trds[ i ].trd_id ) == 0 )
					//{
					//	pwave->tl_trds[ i ].alc_thread->trd_id = 0;
					//	_close_socket( &pwave->tl_trds[ i ].socket_id );

					extra_count--;
					//	_g->stat.sender_thread_count--;
					//}
					break;
				}
			}
		}
	}

	// when we arrive at this point we sure that somethings is changed
	memcpy( &tl->tlcfg , new_tlcfg , sizeof( struct tcp_listener_cfg ) );
	//pwave->awcfg.m.m.temp_data.wcfg_stabled = 1;
	new_tlcfg->m.m.temp_data.tcfg_changed = 0;
	// Set every threads that their state must change
	for ( int i = 0 ; i < tl->tl_trds_masks_count ; i++ )
	{
		if ( tl->tl_trds_masks[ i ] )
		{
			tl->tl_trds[ i ].alc_thread->base_config_change_applied = 1;
		}
	}

	// TODO . complete reverse on error

	BEGIN_RET
		case 3: {}
		case 2: {}
		case 1:
		{
			_g->stat.round.syscal_err_count++;
		}
	M_V_END_RET
}

void stop_tcp_listener( struct App_Data * _g , struct tcp_listener * tl )
{
	INIT_BREAKABLE_FXN();

	//if ( IF_VERBOSE_MODE_CONDITION() )
	//{
	//	_ECHO( "stop_wave" );
	//}

	tl->tlcfg.m.m.maintained.enable = 0;
	tl->tlcfg.m.m.temp_data.tcfg_changed = 1;
	apply_new_tcp_listener_config( _g , tl , &tl->tlcfg );
	// TODO
}

void apply_tcp_listener_new_cfg_changes( struct App_Data * _g , struct tcp_listener_cfg * prev_tlcfg , struct tcp_listener_cfg * new_tlcfg )
{
	INIT_BREAKABLE_FXN();

	//if ( IF_VERBOSE_MODE_CONDITION() )
	//{
	//	_ECHO( "apply_wave_new_cfg_changes" );
	//}

	for ( int i = 0 ; i < _g->listeners.tl_holders_masks_count ; i++ )
	{
		if ( _g->listeners.tl_holders_masks[ i ] )
		{
			if ( memcmp( &_g->listeners.tl_holders[ i ].alc_tl->tlcfg.m.m.id , &prev_tlcfg->m.m.id , sizeof( struct tcp_listener_cfg_id ) ) == 0 )
			{
				apply_new_tcp_listener_config( _g , _g->listeners.tl_holders[ i ].alc_tl , new_tlcfg );
			}
		}
	}
}

void remove_tcp_listener( struct App_Data * _g , struct tcp_listener_cfg * tlcfg )
{
	INIT_BREAKABLE_FXN();

	//if ( IF_VERBOSE_MODE_CONDITION() )
	//{
	//	_ECHO( "remove_wave" );
	//}

	for ( int i = 0 ; i < _g->listeners.tl_holders_masks_count ; i++ )
	{
		if ( _g->listeners.tl_holders_masks[ i ] )
		{
			if ( memcmp( &_g->listeners.tl_holders[ i ].alc_tl->tlcfg.m.m.id , &tlcfg->m.m.id , sizeof( struct tcp_listener_cfg_id ) ) == 0 )
			{
				stop_tcp_listener( _g , _g->listeners.tl_holders[ i ].alc_tl );
				_g->listeners.tl_holders_masks[ i ] = 0;
				DAC( _g->listeners.tl_holders[ i ].alc_tl );
			}
		}
	}
}

#define PREALLOCAION_SIZE 10

void add_new_tcp_listener( struct App_Data * _g , struct tcp_listener_cfg * new_tlcfg )
{
	INIT_BREAKABLE_FXN();

	//if ( IF_VERBOSE_MODE_CONDITION() )
	//{
	//	_ECHO( "add_new_wave" );
	//}

	int new_tlcfg_placement_index = -1;
	while ( new_tlcfg_placement_index < 0 ) // try to find one place for new wave
	{
		for ( int i = 0 ; i < _g->listeners.tl_holders_masks_count ; i++ )
		{
			if ( !_g->listeners.tl_holders_masks[ i ] )
			{
				new_tlcfg_placement_index = i;
				break;
			}
		}
		if ( new_tlcfg_placement_index < 0 )
		{
			int old_tl_holders_masks_count = _g->listeners.tl_holders_masks_count;
			int new_tl_holders_masks_count = old_tl_holders_masks_count + PREALLOCAION_SIZE;

			M_BREAK_IF( ( _g->listeners.tl_holders_masks = REALLOC( _g->listeners.tl_holders_masks , new_tl_holders_masks_count * sizeof( int ) ) ) == REALLOC_ERR , errMemoryLow , 2 );
			MEMSET_ZERO( _g->listeners.tl_holders_masks + old_tl_holders_masks_count , int , PREALLOCAION_SIZE );

			M_BREAK_IF( ( _g->listeners.tl_holders = REALLOC( _g->listeners.tl_holders , new_tl_holders_masks_count * sizeof( struct tcp_listener_holder ) ) ) == REALLOC_ERR , errMemoryLow , 1 );
			MEMSET_ZERO( _g->listeners.tl_holders + old_tl_holders_masks_count , struct tcp_listener_holder , PREALLOCAION_SIZE );

			_g->listeners.tl_holders_masks_count = new_tl_holders_masks_count;
		}
	}

	ASSERT( _g->listeners.tl_holders[ new_tlcfg_placement_index ].alc_tl == NULL );
	M_BREAK_IF( ( _g->listeners.tl_holders[ new_tlcfg_placement_index ].alc_tl = NEW( struct tcp_listener ) ) == NEW_ERR , errMemoryLow , 0 );
	MEMSET_ZERO( _g->listeners.tl_holders[ new_tlcfg_placement_index ].alc_tl , struct tcp_listener , 1 );
	_g->listeners.tl_holders_masks[ new_tlcfg_placement_index ] = 1;
	memcpy( &_g->listeners.tl_holders[ new_tlcfg_placement_index ].alc_tl->tlcfg , new_tlcfg , sizeof( struct tcp_listener_cfg ) );

	apply_tcp_listener_new_cfg_changes( _g , new_tlcfg , new_tlcfg );

	BEGIN_RET
		case 3: DAC( _g->listeners.tl_holders );
		case 2: DAC( _g->listeners.tl_holders_masks );
		case 1: _g->stat.round.syscal_err_count++;
	M_V_END_RET
} // TODO . return value

#endif

#ifndef section_load_config

#define CONFIG_ROOT_PATH "/home/my_projects/home-config/tcp_listener"

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

		if ( CLOSE_APP_VAR() ) break;

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
		case 1: _g->stat.round.syscal_err_count++;
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
		if ( CLOSE_APP_VAR() ) break;
		sleep( 1 );
	}

	typed( json_element ) el_tcp_listener_config;

	while ( 1 )
	{
		cur_time = time( NULL );
		if ( CLOSE_APP_VAR() ) break;
		if ( _g->appcfg._general_config == NULL || _g->appcfg._version_changed /* || difftime(cur_time , prev_time) > 15 * 60*/ )
		{
			prev_time = prev_time; // to ignore warning
			prev_time = cur_time;

			struct Global_Config temp_config = { 0 };
			struct Global_Config_0 * pGeneralConfiguration = ( struct Global_Config_0 * )&temp_config;
			struct tcp_listener_cfg * ptcp_listeners = NULL;
			size_t tcp_listeners_count = 0;
			{
				const char * tcp_listener_config_file_content = read_file( CONFIG_ROOT_PATH "/tcp_listener_config.txt" , NULL );
				MM_BREAK_IF( !tcp_listener_config_file_content , errGeneral , 0 , "cannot open config file" );

				result( json_element ) rs_tcp_listener_config = json_parse( tcp_listener_config_file_content );
				free( ( void * )tcp_listener_config_file_content );
				MM_BREAK_IF( catch_error( &rs_tcp_listener_config , "tcp_listener_config" ) , errGeneral , 0 , "cannot parse config file" );
				el_tcp_listener_config = result_unwrap( json_element )( &rs_tcp_listener_config );

				/*configurations*/
				if ( _g->appcfg._ver->Major >= 1 ) // first version of config file structure
				{
					result( json_element ) re_configurations = json_object_find( el_tcp_listener_config.value.as_object , "configurations" );
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
					CFG_ELEM_I( stat_referesh_interval_sec );

					CFG_ELEM_I( synchronization_min_wait );
					CFG_ELEM_I( synchronization_max_roundup );
					CFG_ELEM_I( show_line_hit );
					CFG_ELEM_I( retry_unexpected_wait_for_sock );
					

#undef CFG_ELEM_I
#undef CFG_ELEM_STR
				}

				/*tcp_listeners*/
				{
					result( json_element ) re_tcp_listeners = json_object_find( el_tcp_listener_config.value.as_object , "TCP_listeners" );
					MM_BREAK_IF( catch_error( &re_tcp_listeners , "tcp_listeners" ) , errGeneral , 0 , "tcp_listeners" );
					typed( json_element ) el_tcp_listeners = result_unwrap( json_element )( &re_tcp_listeners );

					MM_BREAK_IF( ( tcp_listeners_count = el_tcp_listeners.value.as_object->count ) < 1 , errGeneral , 0 , "tcp_listeners must be not zero" );

					M_BREAK_IF( !( ptcp_listeners = NEWBUF( struct tcp_listener_cfg , el_tcp_listeners.value.as_object->count ) ) , errMemoryLow , 0 );
					MEMSET_ZERO( ptcp_listeners , struct tcp_listener_cfg , el_tcp_listeners.value.as_object->count );


					for ( int i = 0 ; i < el_tcp_listeners.value.as_object->count ; i++ )
					{
						( ( struct tcp_listener_cfg_0 * )( ptcp_listeners + i ) )->temp_data._g = ( void * )_g;

						char output_tcp_listener_name[ 32 ];
						memset( output_tcp_listener_name , 0 , sizeof( output_tcp_listener_name ) );
						sprintf( output_tcp_listener_name , "listener%d" , i + 1 );

						result( json_element ) re_output_tcp_listener = json_object_find( el_tcp_listeners.value.as_object , output_tcp_listener_name );
						M_BREAK_IF( catch_error( &re_output_tcp_listener , output_tcp_listener_name ) , errGeneral , 0 );
						typed( json_element ) el_output_tcp_listener = result_unwrap( json_element )( &re_output_tcp_listener );

#define CFG_ELEM_STR( name ) \
							result( json_element ) re_##name = json_object_find( el_output_tcp_listener.value.as_object , #name );\
							M_BREAK_IF( catch_error( &re_##name , #name ) , errGeneral , 0 );\
							typed( json_element ) el_##name = result_unwrap( json_element )( &re_##name );\
							strcpy(((struct tcp_listener_cfg_0 *)(ptcp_listeners + i))->name , el_##name.value.as_string );

#define CFG_ID_ELEM_STR( name ) \
							result( json_element ) re_##name = json_object_find( el_output_tcp_listener.value.as_object , #name );\
							M_BREAK_IF( catch_error( &re_##name , #name ) , errGeneral , 0 );\
							typed( json_element ) el_##name = result_unwrap( json_element )( &re_##name );\
							strcpy(((struct tcp_listener_cfg_0 *)(ptcp_listeners + i))->id.name , el_##name.value.as_string );

#define CFG_ELEM_I_maintained( name ) \
							result( json_element ) re_##name = json_object_find( el_output_tcp_listener.value.as_object , #name );\
							M_BREAK_IF( catch_error( &re_##name , #name ) , errGeneral , 0 );\
							typed( json_element ) el_##name = result_unwrap( json_element )( &re_##name );\
							((struct tcp_listener_cfg_0 *)(ptcp_listeners + i))->maintained.name = (int)el_##name.value.as_number.value.as_long;

#define CFG_ELEM_I_momentary( name ) \
							result( json_element ) re_##name = json_object_find( el_output_tcp_listener.value.as_object , #name );\
							M_BREAK_IF( catch_error( &re_##name , #name ) , errGeneral , 0 );\
							typed( json_element ) el_##name = result_unwrap( json_element )( &re_##name );\
							((struct tcp_listener_cfg_0 *)(ptcp_listeners + i))->momentary.name = (int)el_##name.value.as_number.value.as_long;

#define CFG_ID_ELEM_I( name ) \
							result( json_element ) re_##name = json_object_find( el_output_tcp_listener.value.as_object , #name );\
							M_BREAK_IF( catch_error( &re_##name , #name ) , errGeneral , 0 );\
							typed( json_element ) el_##name = result_unwrap( json_element )( &re_##name );\
							((struct tcp_listener_cfg_0 *)(ptcp_listeners + i))->id.name = (int)el_##name.value.as_number.value.as_long;

						strcpy( ( ( struct tcp_listener_cfg_0 * )( ptcp_listeners + i ) )->id.tcp_listener_name , output_tcp_listener_name );

						CFG_ID_ELEM_STR( TCP_listen_ip );
						CFG_ID_ELEM_I( TCP_listen_port );
						CFG_ID_ELEM_STR( TCP_listen_interface );

						CFG_ELEM_I_maintained( enable );
						CFG_ELEM_I_momentary( reset_connections );



#undef CFG_ID_ELEM_I
#undef CFG_ELEM_I
#undef CFG_ID_ELEM_STR
#undef CFG_ELEM_STR
					}

					json_free( &el_tcp_listeners );
				}
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
					_g->appcfg._general_config_changed |= !( _g->appcfg._general_config->c.c.stat_referesh_interval_sec == _g->appcfg._prev_general_config->c.c.stat_referesh_interval_sec );

					_g->appcfg._general_config_changed |= !( _g->appcfg._general_config->c.c.synchronization_min_wait == _g->appcfg._prev_general_config->c.c.synchronization_min_wait );
					_g->appcfg._general_config_changed |= !( _g->appcfg._general_config->c.c.synchronization_max_roundup == _g->appcfg._prev_general_config->c.c.synchronization_max_roundup );
					_g->appcfg._general_config_changed |= !( _g->appcfg._general_config->c.c.show_line_hit == _g->appcfg._prev_general_config->c.c.show_line_hit );

					_g->appcfg._general_config_changed |= !( _g->appcfg._general_config->c.c.retry_unexpected_wait_for_sock == _g->appcfg._prev_general_config->c.c.retry_unexpected_wait_for_sock );
					
				}
			}

			if ( _g->appcfg._general_config_changed )
			{
				if ( IF_VERBOSE_MODE_CONDITION() )
				{
					_ECHO( "general config changed" );
				}
			}

			if ( _g->appcfg._ptcp_listener_psvcfg )
			{
				DAC( _g->appcfg._pprev_tcp_listener_psvcfg );
				_g->appcfg._prev_tcp_listener_psvcfg_count = 0;
				_g->appcfg._pprev_tcp_listener_psvcfg = _g->appcfg._ptcp_listener_psvcfg;
				_g->appcfg._prev_tcp_listener_psvcfg_count = _g->appcfg._tcp_listener_psvcfg_count;
			}
			_g->appcfg._ptcp_listener_psvcfg = ptcp_listeners;
			_g->appcfg._tcp_listener_psvcfg_count = tcp_listeners_count;
			ptcp_listeners = NULL; // to not delete intentionally
			tcp_listeners_count = 0;

			if ( _g->appcfg._pprev_tcp_listener_psvcfg == NULL && _g->appcfg._ptcp_listener_psvcfg )
			{
				// all new ones
				_g->appcfg._psvcfg_changed = 1; // ham koli set mishavad change rokh dad
				for ( int i = 0 ; i < _g->appcfg._tcp_listener_psvcfg_count ; i++ )
				{
					_g->appcfg._ptcp_listener_psvcfg[ i ].m.m.temp_data.tcfg_changed = 1; // ham joz e set mishavad
				}
			}
			else if ( _g->appcfg._pprev_tcp_listener_psvcfg && _g->appcfg._ptcp_listener_psvcfg )
			{
				// from old perspective
				for ( int i = 0 ; i < _g->appcfg._prev_tcp_listener_psvcfg_count ; i++ )
				{
					int prev_exist = 0;
					for ( int j = 0 ; j < _g->appcfg._tcp_listener_psvcfg_count ; j++ )
					{
						if ( memcmp( &_g->appcfg._pprev_tcp_listener_psvcfg[ i ].m.m.id , &_g->appcfg._ptcp_listener_psvcfg[ j ].m.m.id , sizeof( struct tcp_listener_cfg_id ) ) == 0 )
						{
							prev_exist = 1;
							if ( memcmp( &_g->appcfg._pprev_tcp_listener_psvcfg[ i ].m.m.maintained , &_g->appcfg._ptcp_listener_psvcfg[ j ].m.m.maintained , sizeof( struct tcp_listener_maintained_parameter ) ) )
							{
								_g->appcfg._psvcfg_changed = 1;
								_g->appcfg._ptcp_listener_psvcfg[ j ].m.m.temp_data.tcfg_changed = 1;
							}
							break;
						}
					}
					if ( !prev_exist )
					{
						_g->appcfg._psvcfg_changed = 1;
						_g->appcfg._pprev_tcp_listener_psvcfg[ i ].m.m.temp_data.tcfg_changed = 1;
						break;
					}
				}
				// from new perspective
				for ( int j = 0 ; j < _g->appcfg._tcp_listener_psvcfg_count ; j++ )
				{
					int new_exist = 0;
					for ( int i = 0 ; i < _g->appcfg._prev_tcp_listener_psvcfg_count ; i++ )
					{
						if ( memcmp( &_g->appcfg._pprev_tcp_listener_psvcfg[ i ].m.m.id , &_g->appcfg._ptcp_listener_psvcfg[ j ].m.m.id , sizeof( struct tcp_listener_cfg_id ) ) == 0 )
						{
							new_exist = 1;
							if ( memcmp( &_g->appcfg._pprev_tcp_listener_psvcfg[ i ].m.m.maintained , &_g->appcfg._ptcp_listener_psvcfg[ j ].m.m.maintained , sizeof( struct tcp_listener_maintained_parameter ) ) )
							{
								_g->appcfg._psvcfg_changed = 1;
								_g->appcfg._ptcp_listener_psvcfg[ j ].m.m.temp_data.tcfg_changed = 1;
							}
							break;
						}
					}
					if ( !new_exist )
					{
						_g->appcfg._psvcfg_changed = 1;
						_g->appcfg._ptcp_listener_psvcfg[ j ].m.m.temp_data.tcfg_changed = 1;
						break;
					}
				}
			}

			_g->appcfg._version_changed = 0;

			if ( _g->appcfg._psvcfg_changed && IF_VERBOSE_MODE_CONDITION() )
			{
				_ECHO( "tcp_listener config changed" );
			}
		}
		sleep( 2 );
	}

	BEGIN_RET
		case 1: {}
	M_V_END_RET
	return NULL;
}

// TODO . aware of concurrency in config read and act on it
void * tcp_listener_manager( void * app_data )
{
	INIT_BREAKABLE_FXN();
	struct App_Data * _g = ( struct App_Data * )app_data;

	while ( !_g->appcfg._tcp_listener_psvcfg_count ) // load after any config loaded
	{
		if ( CLOSE_APP_VAR() ) break;
		sleep( 1 );
	}

	while ( 1 )
	{
		if ( CLOSE_APP_VAR() )
		{
			for ( int j = 0 ; j < _g->appcfg._tcp_listener_psvcfg_count ; j++ )
			{
				_g->appcfg._ptcp_listener_psvcfg[ j ].m.m.maintained.enable = 0;
				apply_tcp_listener_new_cfg_changes( _g , &_g->appcfg._ptcp_listener_psvcfg[ j ] , &_g->appcfg._ptcp_listener_psvcfg[ j ] );
			}
			break;
		}
		if ( _g->appcfg._general_config_changed )
		{
			_g->appcfg._general_config_changed = 0; // for now . TODO later
		}

		if ( _g->appcfg._psvcfg_changed )
		{
			for ( int i = 0 ; i < _g->appcfg._prev_tcp_listener_psvcfg_count ; i++ )
			{
				int exist = 0;
				for ( int j = 0 ; j < _g->appcfg._tcp_listener_psvcfg_count ; j++ )
				{
					if ( exist ) break;
					if ( memcmp( &_g->appcfg._pprev_tcp_listener_psvcfg[ i ].m.m.id , &_g->appcfg._ptcp_listener_psvcfg[ j ].m.m.id , sizeof( struct tcp_listener_cfg_id ) ) == 0 && _g->appcfg._ptcp_listener_psvcfg[ j ].m.m.temp_data.tcfg_changed )
					{
						// existed cfg changed
						apply_tcp_listener_new_cfg_changes( _g , &_g->appcfg._pprev_tcp_listener_psvcfg[ i ] , &_g->appcfg._ptcp_listener_psvcfg[ j ] );
						exist = 1;
					}
				}
				if ( !exist )
				{
					// remove removed one
					remove_tcp_listener( _g , &_g->appcfg._pprev_tcp_listener_psvcfg[ i ] );
				}
			}
			for ( int i = 0 ; i < _g->appcfg._tcp_listener_psvcfg_count ; i++ )
			{
				int exist = 0;
				for ( int j = 0 ; j < _g->appcfg._prev_tcp_listener_psvcfg_count ; j++ )
				{
					if ( memcmp( &_g->appcfg._ptcp_listener_psvcfg[ i ].m.m.id , &_g->appcfg._pprev_tcp_listener_psvcfg[ j ].m.m.id , sizeof( struct tcp_listener_cfg_id ) ) == 0 )
					{
						exist = 1;
						break;
					}
				}
				if ( !exist )
				{
					// start new cfg
					add_new_tcp_listener( _g , &_g->appcfg._ptcp_listener_psvcfg[ i ] );
				}
			}
			_g->appcfg._psvcfg_changed = 0; // changes applied
		}
		sleep( 5 );
	}
	return NULL;
}

#endif

#ifndef section_staff_thread

void * sync_thread( void * pdata ) // pause app until moment other app exist
{
	INIT_BREAKABLE_FXN();
	struct App_Data * _g = ( struct App_Data * )pdata;
	//if ( _g->sync.lock_in_progress ) return NULL;

	struct timespec now , next_round_time;
	clock_gettime( CLOCK_REALTIME , &now );

	//pthread_mutex_lock( &_g->sync.mutex );
	round_up_to_next_interval( &now , _g->appcfg._general_config->c.c.synchronization_min_wait , _g->appcfg._general_config->c.c.synchronization_max_roundup , &next_round_time );
	//_g->sync.lock_in_progress = 1;
	//pthread_mutex_unlock( &_g->sync.mutex );

	//next_round_time.tv_sec += 5; // bridge start later

	format_clock_time( &next_round_time , __custom_message , sizeof( __custom_message ) );
	_DIRECT_ECHO( "Will wake at %s" , __custom_message );

	////pthread_mutex_lock(&_g->sync.mutex);
	//// First thread sets the global target time
	////if (next_round_time.tv_sec == 0) {
	////	next_round_time = target;
	////}
	////pthread_mutex_unlock(&_g->sync.mutex);

	// Sleep until that global target time
	clock_nanosleep( CLOCK_REALTIME , TIMER_ABSTIME , &next_round_time , NULL );
	memset( &_g->stat.round , 0 , sizeof( _g->stat.round ) );

	//pthread_mutex_lock( &_g->sync.mutex );
	//_g->sync.lock_in_progress = 0;
	//_g->sync.reset_static_after_lock = 1;
	//pthread_cond_signal( &_g->sync.cond );
	////pthread_cond_broadcast( &_g->sync.cond );
	//pthread_mutex_unlock( &_g->sync.mutex );

	//clock_gettime( CLOCK_REALTIME , &now );
	//_DIRECT_ECHO( "waked up" );
	_DIRECT_ECHO( "" );

	return NULL;
}

void * input_thread( void * pdata )
{
	INIT_BREAKABLE_FXN();
	struct App_Data * _g = ( struct App_Data * )pdata;
	while ( 1 )
	{
		pthread_mutex_lock( &_g->stat.lock_data.lock );

		werase( _g->stat.input_win );
		box( _g->stat.input_win , 0 , 0 );

		pthread_mutex_unlock( &_g->stat.lock_data.lock );

		// Enable echo and get input
		echo();
		curs_set( 1 );
		wmove( _g->stat.input_win , 1 , 1 );
		wprintw( _g->stat.input_win , "cmd(quit,sync): " );
		wrefresh( _g->stat.input_win );
		wgetnstr( _g->stat.input_win , _g->stat.input_buffer , INPUT_MAX - 1 );
		noecho();
		curs_set( 0 );

		pthread_mutex_lock( &_g->stat.lock_data.lock );
		bool boutput_command = 1;

		if ( stricmp( _g->stat.input_buffer , "quit" ) == 0 )
		{
			_g->cmd.quit_app = 1;
			break;
		}
		else if ( stricmp( _g->stat.input_buffer , "sync" ) == 0 )
		{
			boutput_command = 0;
			pthread_t thread;
			if ( pthread_create( &thread , NULL , sync_thread , pdata ) != 0 )
			{
				_ECHO( "pthread_create" );
			}
			//break;
		}


		if ( boutput_command )
		{
			strncpy( _g->stat.last_command , _g->stat.input_buffer , INPUT_MAX );
			_g->stat.last_command[ INPUT_MAX - 1 ] = EOS;
		}

		pthread_mutex_unlock( &_g->stat.lock_data.lock );
	}
	return NULL;
}

#endif

#ifndef section_stat

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
	char * header_border = "+----------+--------------------------------------------------------------------------------+";

	int cell_w = strlen( header_border ) / 2;
	int start_x = 2;
	int y = 1;

	// Top border
	mvwprintw( MAIN_WIN , y++ , start_x , header_border );

	char buf[ 640 ];
	char buf2[ 64 ];
	struct timespec now;
	clock_gettime( CLOCK_REALTIME , &now );
	format_clock_time( &now , buf , sizeof( buf ) );
	snprintf( buf2 , sizeof( buf2 ) , "TL Metric-%s" , buf );

	// Header
	wattron( MAIN_WIN , COLOR_PAIR( 1 ) );
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , buf2 );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , "Value" );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );
	wattroff( MAIN_WIN , COLOR_PAIR( 1 ) );

	// Header border
	mvwprintw( MAIN_WIN , y++ , start_x , header_border );

	// Data rows
	wattron( MAIN_WIN , COLOR_PAIR( 2 ) );

	setlocale( LC_NUMERIC , "en_US.UTF-8" );

	//
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "ver." );
	snprintf( buf , sizeof( buf ) , "%s" , _g->appcfg._ver->version );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );

	//
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "alive" );
	snprintf( buf , sizeof( buf ) , "%d%.*s" , MAIN_STAT().last_line_meet , MAIN_STAT().alive_check_counter , "-+-+-+-+-+-+-+-+" );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );

	///////////

	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "inp failure" );
	snprintf( buf , sizeof( buf ) , "v%d Σv%d" , MAIN_STAT().round.continuously_unsuccessful_receive_error , MAIN_STAT().round.total_unsuccessful_receive_error );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );

	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "syscal_err" );
	snprintf( buf , sizeof( buf ) , "%s" , format_pps( buf2 , sizeof( buf2 ) , MAIN_STAT().round.syscal_err_count , 2 , "" ) );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );

	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "TCP conn" );
	snprintf( buf , sizeof( buf ) , "%d Σ%d" , MAIN_STAT().tcp_connection_count , MAIN_STAT().total_retry_tcp_connection_count );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );

	if ( _g->appcfg._general_config && _g->appcfg._general_config->c.c.show_line_hit )
	{
		mvwprintw( MAIN_WIN , y++ , start_x , header_border );
		for ( int i = 0 ; i < FXN_HIT_COUNT ; i++ )
		{
			if ( __FXN_HIT[ i ][ 0 ] > 0 )
			{
				mvwprintw( MAIN_WIN , y , start_x , "|" );
				snprintf( buf , sizeof( buf ) , "%d" , i ); // line
				print_cell( MAIN_WIN , y , start_x + 1 , cell_w , buf ); // line
				snprintf( buf , sizeof( buf ) , "%d " , __FXN_HIT[ i ][ 0 ] ); // hit count

				for ( int k = 1 ; k < PC_COUNT ; k++ )
				{
					if ( __FXN_HIT[ i ][ k ] == 0 )
					{
						break;
					}
					snprintf( buf2 , sizeof( buf2 ) , ",%d" , __FXN_HIT[ i ][ k ] );
					strcat( buf , buf2 );
				}

				mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
				print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
				mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );
			}
		}

		mvwprintw( MAIN_WIN , y++ , start_x , header_border );
	}

	mvwprintw( MAIN_WIN , y++ , start_x , header_border );


	//
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "tcp get" );
	snprintf( buf , sizeof( buf ) , "%s" , format_pps( buf2 , sizeof( buf2 ) , MAIN_STAT().round.tcp.total_tcp_get_count , 2 , "" ) );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );
	//
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "tcp get byte" );
	snprintf( buf , sizeof( buf ) , "%s" , format_pps( buf2 , sizeof( buf2 ) , MAIN_STAT().round.tcp.total_tcp_get_byte , 2 , "B" ) );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );

	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "contnu unsuces slct tcp" );
	snprintf( buf , sizeof( buf ) , "%s" , format_pps( buf2 , sizeof( buf2 ) , MAIN_STAT().round.tcp.continuously_unsuccessful_select_on_open_port_count , 2 , "" ) );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );

	// 1 sec
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "1s tcp pps" );
	snprintf( buf , sizeof( buf ) , "%s" , format_pps( buf2 , sizeof( buf2 ) , MAIN_STAT().round.tcp_1_sec.tcp_get_count_throughput , 4 , "" ) );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );
	//
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "1s tcp bps" );
	snprintf( buf , sizeof( buf ) , "%s" , format_pps( buf2 , sizeof( buf2 ) , MAIN_STAT().round.tcp_1_sec.tcp_get_byte_throughput , 4 , "B" ) );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );

	// 10 sec
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "10s tcp pps" );
	snprintf( buf , sizeof( buf ) , "%s" , format_pps( buf2 , sizeof( buf2 ) , MAIN_STAT().round.tcp_10_sec.tcp_get_count_throughput / 10 , 4 , "" ) );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );
	//
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "10s tcp bps" );
	snprintf( buf , sizeof( buf ) , "%s" , format_pps( buf2 , sizeof( buf2 ) , MAIN_STAT().round.tcp_10_sec.tcp_get_byte_throughput / 10 , 4 , "B" ) );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );

	// 40 sec
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "40s tcp pps" );
	snprintf( buf , sizeof( buf ) , "%s" , format_pps( buf2 , sizeof( buf2 ) , MAIN_STAT().round.tcp_40_sec.tcp_get_count_throughput / 40 , 4 , "" ) );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );
	//
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "40s tcp bps" );
	snprintf( buf , sizeof( buf ) , "%s" , format_pps( buf2 , sizeof( buf2 ) , MAIN_STAT().round.tcp_40_sec.tcp_get_byte_throughput / 40 , 4 , "B" ) );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );

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
		//if ( CLOSE_APP_VAR() ) break; // keep track changes until app is down

		pthread_mutex_lock( &_g->stat.lock_data.lock );

		werase( _g->stat.main_win );
		box( _g->stat.main_win , 0 , 0 );
		draw_table( _g );
		wrefresh( _g->stat.main_win );

		pthread_mutex_unlock( &_g->stat.lock_data.lock );

		sleep( STAT_REFERESH_INTERVAL_SEC() );
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

#ifndef section_err

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
#pragma GCC diagnostic ignored "-Wstringop-truncation"
			strncpy( _g->stat.last_command , buffer , sizeof( _g->stat.last_command ) - 1 );
#pragma GCC diagnostic pop
		}
	}
	return NULL;
}

void init_bypass_stdout( struct App_Data * _g )
{
	//int pipefd[ 2 ];
	//memset( pipefd , 0 , sizeof( pipefd ) );

	// Make pipe
	if ( pipe( _g->stat.pipefds ) == -1 )
	{
	}

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

void init( struct App_Data * _g )
{
	//INIT_BREAKABLE_FXN();

	// Initialize curses
	initscr();
	start_color();
	cbreak();
	noecho();
	curs_set( 1 );

	init_pair( 1 , COLOR_WHITE , COLOR_BLUE );   // Header
	init_pair( 2 , COLOR_GREEN , COLOR_BLACK );  // Data
	init_pair( 3 , COLOR_YELLOW , COLOR_BLACK ); // Last Command

	pthread_mutex_init( &_g->stat.lock_data.lock , NULL );

	// Initial window creation
	init_windows( _g );
	init_bypass_stdout( _g );

	//pthread_mutex_init( &_g->sync.mutex , NULL );
	//pthread_cond_init( &_g->sync.cond , NULL );
}

int main()
{
	INIT_BREAKABLE_FXN();
	struct App_Data g = { 0 };
	struct App_Data * _g = &g;
	__g = _g;

	init( _g );

	pthread_t tid_stats , tid_input;
	pthread_create( &tid_stats , NULL , stats_thread , ( void * )_g );
	pthread_create( &tid_input , NULL , input_thread , ( void * )_g );

	pthread_t trd_version_checker;
	MM_BREAK_IF( pthread_create( &trd_version_checker , NULL , version_checker , ( void * )_g ) != PTHREAD_CREATE_OK , errGeneral , 0 , "Failed to create thread" );

	pthread_t trd_config_loader;
	MM_BREAK_IF( pthread_create( &trd_config_loader , NULL , config_loader , ( void * )_g ) != PTHREAD_CREATE_OK , errGeneral , 0 , "Failed to create thread" );

	pthread_t trd_tcp_listener_manager;
	MM_BREAK_IF( pthread_create( &trd_tcp_listener_manager , NULL , tcp_listener_manager , ( void * )_g ) != PTHREAD_CREATE_OK , errGeneral , 0 , "Failed to create thread" );

	M_BREAK_IF( pthread_join( trd_tcp_listener_manager , NULL ) != PTHREAD_JOIN_OK , errGeneral , 0 );

	return 0;
	BEGIN_RET
		case 2: {}
		case 1: {}
		case 0:
		{
			__g->stat.round.syscal_err_count++;
		}
	M_V_END_RET
	return 1;
}

#endif
