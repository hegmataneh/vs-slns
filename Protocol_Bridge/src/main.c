#ifndef section_include

#define Uses_fcntl
#define Uses_circbuf
#define Uses_clock_gettime
#define Uses_stricmp
#define Uses_PacketQueue
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

#include <Protocol_Bridge.dep>

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

// TODO . maybe in middle of config change bug appear ad app unexpectedly quit but that sit is very rare
#define RETRY_UNEXPECTED_WAIT_FOR_SOCK() ( _g->appcfg._general_config ? _g->appcfg._general_config->c.c.retry_unexpected_wait_for_sock : 3 )


#define NUMBER_IN_SHORT_FORM() ( _g->appcfg._general_config ? _g->appcfg._general_config->c.c.number_in_short_form : 1 )



#define FXN_HIT_COUNT 5000
#define PC_COUNT 10 // first for hit count and last alwayz zero

int __FXN_HIT[FXN_HIT_COUNT][PC_COUNT] = {0}; // max size is about number of code lines
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

enum app_thread_handler_type
{
	invalid_type = 0,
	buttleneck = 1,
	bidirection = 2,
	justIncoming = 3
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
	const char * thread_handler_type;
	enum app_thread_handler_type atht;

	int synchronization_min_wait;
	int synchronization_max_roundup;
	int show_line_hit;
	int retry_unexpected_wait_for_sock;
	int number_in_short_form;
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

#ifndef section_bridge_config

struct protocol_bridge_cfg_id
{
	char protocol_bridge_name[ 64 ];

	char UDP_origin_ip[ 64 ];
	int UDP_origin_port;
	char UDP_origin_interface[ 128 ];

	char TCP_destination_ip[ 64 ];
	int TCP_destination_port;
	char TCP_destination_interface[ 128 ];
};

struct protocol_bridge_maintained_parameter // stays in position
{
	int enable;
};

struct protocol_bridge_momentary_parameter // popup automatically
{
	int reset_connections;
};

struct protocol_bridge_temp_data
{
	void * _g; // just point to the main g
	int pcfg_changed; // in passive cfg and active cfg that in alive protocol_bridge, in both it means something changed
};

struct protocol_bridge_cfg_0
{
	struct protocol_bridge_cfg_id id; // must be uniq for each protocol_bridge
	struct protocol_bridge_maintained_parameter maintained;
	struct protocol_bridge_momentary_parameter momentary;
	struct protocol_bridge_temp_data temp_data;
};

struct protocol_bridge_cfg_n
{
	struct protocol_bridge_cfg_0 m; // first member , n - 1
};

struct protocol_bridge_cfg // finalizer
{
	struct protocol_bridge_cfg_n m; // be first member
};

#endif

#ifndef section_bridges

struct protocol_bridge_thread
{
	pthread_t trd_id;
	int base_config_change_applied;
	int do_close_thread; // command from outside to inside thread

	struct protocol_bridge * pb; // point to upper wave
};

struct protocol_bridge_thread_holder
{
	struct protocol_bridge_thread * alc_thread; // one
};

struct protocol_bridge
{
	struct protocol_bridge_cfg apcfg;  // copy of applied cfg . active protocol_bridge config . یک دسته کامل از ترد ها یک کانفیگ را اعمال می کند

	int udp_sockfd;
	int udp_connection_established; // udp socket established

	int tcp_sockfd;
	int tcp_connection_established; // tcp connection established

	int retry_to_connect_udp;
	int retry_to_connect_tcp;

	struct protocol_bridge_thread_holder * pb_trds; // protocol_bridge threads . in protocol bridge app must be one
	int * pb_trds_masks;  // each int represent thread is valid
	size_t pb_trds_masks_count;  // thread keeper count
};

struct protocol_bridge_holder // every elemnt at the reallocation array must have holder because reallocate change data values but with holder just pointer addr change
{
	struct protocol_bridge * alc_pb; // allocated
};

struct bridges_thread_base
{
	int thread_is_created;
	int do_close_thread; // command from outside to inside thread
	pthread_mutex_t creation_thread_race_cond; // prevent multi bridge create thread concurrently
	
	int start_working; // because udp port may start after thread started
	pthread_mutex_t start_working_race_cond; // prevent multi bridge create thread concurrently
};

struct bridges_bottleneck_thread // one thread for send and receive
{
	pthread_t trd_id;
};

struct bridges_bidirection_thread_zero_init_memory
{
	pthread_t income_trd_id;
	pthread_t outgoing_trd_id;
};

struct bridges_bidirection_thread // one thread for each direction
{
	struct bridges_bidirection_thread_zero_init_memory mem;

	struct PacketQueue queue;
};

struct bridges_justIncoming_thread // one thread for send and receive
{
	pthread_t trd_id;
};

// TODO . IPv6
struct protocol_bridge_holders
{
	struct protocol_bridge_holder * pb_holders; // all the active protocol_bridge
	int * pb_holders_masks; // protocol_bridge masks
	size_t pb_holders_masks_count; // mask count

	int under_listen_udp_sockets_group_changed;

	struct bridges_thread_base thread_base;
	struct bridges_bottleneck_thread * bottleneck_thread;
	struct bridges_bidirection_thread * bidirection_thread;
	struct bridges_justIncoming_thread * justIncoming_thread;
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

	// protocol_bridge 
	struct protocol_bridge_cfg * _pprev_protocol_bridge_psvcfg; // maybe later in some condition we need to rollback to prev config
	size_t _prev_protocol_bridge_psvcfg_count;

	struct protocol_bridge_cfg * _pprotocol_bridge_psvcfg; // passive config
	size_t _protocol_bridge_psvcfg_count;

	int _psvcfg_changed; // act like bool . something is changed
};

#define INPUT_MAX 256

struct udp_stat_1_sec
{
	time_t t_udp_throughput;

	__int64u calc_throughput_udp_get_count;
	__int64u calc_throughput_udp_get_bytes;

	//__int64u udp_get_count_throughput;
	//__int64u udp_get_byte_throughput;
};

struct udp_stat
{
	__int64u total_udp_get_count;
	__int64u total_udp_get_byte;

	__int64u continuously_unsuccessful_select_on_open_port_count; // that canot get find data
};

struct tcp_stat_1_sec
{
	time_t t_tcp_throughput;

	__int64u calc_throughput_tcp_put_count;
	__int64u calc_throughput_tcp_put_bytes;

	//__int64u tcp_put_count_throughput;
	//__int64u tcp_put_byte_throughput;
};

struct tcp_stat
{
	__int64u total_tcp_put_count;
	__int64u total_tcp_put_byte;
};

struct statistics_lock_data
{
	pthread_mutex_t lock;
};


struct BenchmarkRound_initable_memory // must be init with own function
{
	struct circbuf_t udp_stat_5_sec_count , udp_stat_5_sec_bytes;
	struct circbuf_t udp_stat_10_sec_count , udp_stat_10_sec_bytes;
	struct circbuf_t udp_stat_40_sec_count , udp_stat_40_sec_bytes;
	struct circbuf_t udp_stat_120_sec_count , udp_stat_120_sec_bytes;

	struct circbuf_t tcp_stat_5_sec_count , tcp_stat_5_sec_bytes;
	struct circbuf_t tcp_stat_10_sec_count , tcp_stat_10_sec_bytes;
	struct circbuf_t tcp_stat_40_sec_count , tcp_stat_40_sec_bytes;
	struct circbuf_t tcp_stat_120_sec_count , tcp_stat_120_sec_bytes;
};

struct BenchmarkRound_zero_init_memory // can be memset to zero all byte
{
	struct timeval t_begin , t_end; // begin and end of on iteration of benchmarking

	int continuously_unsuccessful_receive_error;
	int total_unsuccessful_receive_error;

	int continuously_unsuccessful_send_error;
	int total_unsuccessful_send_error;

	__int64u syscal_err_count;

	struct udp_stat_1_sec udp_1_sec;
	struct tcp_stat_1_sec tcp_1_sec;

	//struct udp_stat_10_sec stat_10_sec;
	//struct udp_stat_40_sec stat_40_sec;

	struct udp_stat udp;
	struct tcp_stat tcp;
};


//struct BenchmarkRound
//{
//	// err
//	int continuously_unsuccessful_receive_error;
//	int total_unsuccessful_receive_error;
//
//	int continuously_unsuccessful_send_error;
//	int total_unsuccessful_send_error;
//
//	__int64u syscal_err_count;
//
//	struct udp_stat_1_sec udp_1_sec;
//	//struct udp_stat_10_sec udp_10_sec;
//	//struct udp_stat_40_sec udp_40_sec;
//
//	struct udp_stat udp;
//
//	struct tcp_stat_1_sec tcp_1_sec;
//	//struct tcp_stat_10_sec tcp_10_sec;
//	//struct tcp_stat_40_sec tcp_40_sec;
//
//	struct tcp_stat tcp;
//};

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

	int udp_connection_count;
	int tcp_connection_count;
	int total_retry_udp_connection_count;
	int total_retry_tcp_connection_count;

	struct BenchmarkRound_zero_init_memory  round_zero_set;
	struct BenchmarkRound_initable_memory  round_init_set;
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
	struct protocol_bridge_holders bridges;
	struct statistics stat;
	struct synchronization_data sync;
	struct app_cmd cmd;
};

#endif

#ifndef section_connection_functions

#define PARALLELISM_COUNT 1

// TODO . close connection after change in config
void * thread_udp_connection_proc( void * src_pb )
{
	INIT_BREAKABLE_FXN();

	struct protocol_bridge * pb = ( struct protocol_bridge * )src_pb;
	struct App_Data * _g = ( struct App_Data * )pb->apcfg.m.m.temp_data._g;
	//SYS_ALIVE_CHECK();

	if ( IF_VERBOSE_MODE_CONDITION() )
	{
		_ECHO( "try to connect inbound udp connection" );
	}

	if ( pb->udp_connection_established )
	{
		SYS_ALIVE_CHECK();
		_close_socket( &pb->udp_sockfd );
		pb->udp_connection_established = 0;
		_g->stat.udp_connection_count--;
	}

	MM_BREAK_IF( ( pb->udp_sockfd = socket( AF_INET , SOCK_DGRAM , 0 ) ) == FXN_SOCKET_ERR , errGeneral , 1 , "create sock error" );

	int opt = 1;
	MM_BREAK_IF( setsockopt( pb->udp_sockfd , SOL_SOCKET , SO_REUSEADDR , &opt , sizeof( opt ) ) < 0 , errGeneral , 1 , "SO_REUSEADDR" );
	//MM_BREAK_IF( setsockopt( pb->udp_sockfd , SOL_SOCKET , 0x0200 , &opt , sizeof( opt ) ) < 0 , errGeneral , 1 , "0x0200" );

	fcntl(pb->udp_sockfd, F_SETFL, O_NONBLOCK);

	struct sockaddr_in server_addr;
	memset( &server_addr , 0 , sizeof( server_addr ) );
	server_addr.sin_family = AF_INET; // IPv4
	server_addr.sin_port = htons( ( uint16_t )pb->apcfg.m.m.id.UDP_origin_port ); // Convert port to network byte order
	server_addr.sin_addr.s_addr = inet_addr( pb->apcfg.m.m.id.UDP_origin_ip ); // Specify the IP address to bind to
	//server_addr.sin_addr.s_addr = INADDR_ANY; // Or use INADDR_ANY to bind to all available interfaces:

	MM_BREAK_IF( bind( pb->udp_sockfd , ( const struct sockaddr * )&server_addr , sizeof( server_addr ) ) == FXN_BIND_ERR , errGeneral , 1 , "bind sock error" );
	pb->udp_connection_established = 1;

	pthread_mutex_lock( &_g->bridges.thread_base.start_working_race_cond );
	switch ( _g->appcfg._general_config->c.c.atht )
	{
		case buttleneck :
		case bidirection :
		{
			if ( pb->udp_connection_established && pb->tcp_connection_established )
			{
				_g->bridges.thread_base.start_working = 1;
			}
			break;
		}
		default:
		case justIncoming :
		{
			if ( pb->udp_connection_established )
			{
				_g->bridges.thread_base.start_working = 1;
			}
			break;
		}
	}
	pthread_mutex_unlock( &_g->bridges.thread_base.start_working_race_cond );
	SYS_ALIVE_CHECK();

	_g->stat.udp_connection_count++;
	_g->stat.total_retry_udp_connection_count++;

	_g->bridges.under_listen_udp_sockets_group_changed++; // if any udp socket change then fdset must be reinitialized

	if ( IF_VERBOSE_MODE_CONDITION() )
	{
		_ECHO( "inbound udp connected" );
	}
	//SYS_ALIVE_CHECK();

	BEGIN_RET
		case 3: {}
		case 2: {}
		case 1:	_g->stat.round_zero_set.syscal_err_count++;
	M_V_END_RET
	return NULL; // Threads can return a value, but this example returns NULL
}

int _connect_tcp( struct protocol_bridge * pb )
{
	INIT_BREAKABLE_FXN();
	struct App_Data * _g = ( struct App_Data * )pb->apcfg.m.m.temp_data._g;

	while ( 1 )
	{
		SYS_ALIVE_CHECK();

		// try to create TCP socket
		MM_BREAK_IF( ( pb->tcp_sockfd = socket( AF_INET , SOCK_STREAM , 0 ) ) == FXN_SOCKET_ERR , errGeneral , 0 , "create sock error" );

		struct sockaddr_in tcp_addr;
		tcp_addr.sin_family = AF_INET;
		tcp_addr.sin_port = htons( ( uint16_t )pb->apcfg.m.m.id.TCP_destination_port );
		MM_BREAK_IF( inet_pton( AF_INET , pb->apcfg.m.m.id.TCP_destination_ip , &tcp_addr.sin_addr ) <= 0 , errGeneral , 1 , "inet_pton sock error" );

		if ( connect( pb->tcp_sockfd , ( struct sockaddr * )&tcp_addr , sizeof( tcp_addr ) ) == -1 )
		{
			if ( errno == ECONNREFUSED || errno == ETIMEDOUT )
			{
				_close_socket( &pb->tcp_sockfd );
				sleep( 2 ); // sec
				continue;
			}

			MM_BREAK_IF( 1 , errGeneral , 1 , "Error connecting to TCP server" );
		}
		else
		{
			pb->tcp_connection_established = 1;
			_g->stat.tcp_connection_count++;
			_g->stat.total_retry_tcp_connection_count++;

			pthread_mutex_lock( &_g->bridges.thread_base.start_working_race_cond );
			switch ( _g->appcfg._general_config->c.c.atht )
			{
				case buttleneck:
				case bidirection:
				{
					if ( pb->udp_connection_established && pb->tcp_connection_established )
					{
						_g->bridges.thread_base.start_working = 1;
					}
					break;
				}
				default:
				case justIncoming:
				{
					if ( pb->udp_connection_established )
					{
						_g->bridges.thread_base.start_working = 1;
					}
					break;
				}
			}
			pthread_mutex_unlock( &_g->bridges.thread_base.start_working_race_cond );
			SYS_ALIVE_CHECK();

			if ( IF_VERBOSE_MODE_CONDITION() )
			{
				_ECHO( "outbound tcp connected" );
			}
			SYS_ALIVE_CHECK();
			return 0;
		}
	}
	
	BEGIN_RET
		case 3: {}
		case 2: {}
		case 1:
		{
			_close_socket( &pb->tcp_sockfd );
			_g->stat.round_zero_set.syscal_err_count++;
		}
	M_V_END_RET
	return -1;
}

void * thread_tcp_connection_proc( void * src_pb )
{
	INIT_BREAKABLE_FXN();

	struct protocol_bridge * pb = ( struct protocol_bridge * )src_pb;
	struct App_Data * _g = ( struct App_Data * )pb->apcfg.m.m.temp_data._g;

	if ( IF_VERBOSE_MODE_CONDITION() )
	{
		_ECHO( "try to connect outbound tcp connection" );
	}

	if ( pb->tcp_connection_established )
	{
		SYS_ALIVE_CHECK();
		_close_socket( &pb->tcp_sockfd );
		pb->tcp_connection_established = 0;
		_g->stat.tcp_connection_count--;
	}

	if ( _connect_tcp( pb ) == 0 )
	{

	}
	BREAK_OK(0); // to just ignore gcc warning

	BEGIN_RET
		case 3: {}
		case 2: {}
		case 1: {}
	M_V_END_RET
	return NULL; // Threads can return a value, but this example returns NULL
}

#endif

#ifndef section_connection_functions

#define BUFFER_SIZE MAX_PACKET_SIZE

#ifndef bottleneck_in_input_output

void * bottleneck_thread_proc( void * src_g )
{
	INIT_BREAKABLE_FXN();

	//struct protocol_bridge * pb = ( struct protocol_bridge * )src_pb;
	struct App_Data * _g = ( struct App_Data * )src_g;

	time_t tnow = 0;

	while ( !_g->bridges.thread_base.start_working )
	{
		if ( _g->bridges.thread_base.do_close_thread )
		{
			break;
		}
		sleep( 1 );
	}

	int input_udp_socket_error_tolerance_count = 0; // restart socket after many error accur
	int output_tcp_socket_error_tolerance_count = 0; // restart socket after many error accur

	int config_changes = 0;
	do
	{
		SYS_ALIVE_CHECK();
		if ( _g->bridges.thread_base.do_close_thread )
		{
			break;
		}

		config_changes = 0;

		char buffer[ BUFFER_SIZE ]; // Define a buffer to store received data
		struct sockaddr_in client_addr;
		socklen_t client_len = sizeof( client_addr );
		ssize_t bytes_received;

		fd_set readfds; // Set of socket descriptors
		FD_ZERO( &readfds );

		ssize_t sz;
		int num_valid_masks = 0;

		int sockfd_max = -1; // for select compulsion
		for ( int i = 0 ; i < _g->bridges.pb_holders_masks_count ; i++ )
		{
			if ( _g->bridges.pb_holders_masks[ i ] )
			{
				if ( _g->bridges.pb_holders[ i ].alc_pb->udp_connection_established && _g->bridges.pb_holders[ i ].alc_pb->tcp_connection_established )
				{
					FD_SET( _g->bridges.pb_holders[ i ].alc_pb->udp_sockfd , &readfds );
					if ( _g->bridges.pb_holders[ i ].alc_pb->udp_sockfd > sockfd_max )
					{
						sockfd_max = _g->bridges.pb_holders[ i ].alc_pb->udp_sockfd;
					}
				}
			}
		}
		_g->bridges.under_listen_udp_sockets_group_changed = 0; // if any udp socket change then fdset must be reinitialized

		if ( sockfd_max <= 1 )
		{
			sleep( 1 );
			continue;
		}

		while ( 1 )
		{
			SYS_ALIVE_CHECK();

			//pthread_mutex_lock( &_g->sync.mutex );
			//while ( _g->sync.lock_in_progress )
			//{
			//	////struct timespec ts = { 0, 10L };
			//	////thrd_sleep( &ts , NULL );
			//	pthread_cond_wait( &_g->sync.cond , &_g->sync.mutex );
			//}
			//pthread_mutex_unlock( &_g->sync.mutex );
			//if ( _g->sync.reset_static_after_lock )
			//{
			//	_g->sync.reset_static_after_lock = 0;
			//	memset( &_g->stat.round , 0 , sizeof( _g->stat.round ) );
			//}

			if ( _g->bridges.thread_base.do_close_thread )
			{
				break;
			}
			if ( _g->bridges.under_listen_udp_sockets_group_changed )
			{
				config_changes = 1;
				break;
			}

			SYS_ALIVE_CHECK();

			struct timeval timeout; // Set timeout (e.g., 5 seconds)
			timeout.tv_sec = 5;
			timeout.tv_usec = 0;

			// Wait for an activity on one of the sockets, timeout is NULL, so wait indefinitely
			int activity = select( sockfd_max + 1 , &readfds , NULL , NULL , &timeout );

			if ( ( activity < 0 ) /* && ( errno != EINTR )*/ )
			{
				//SYS_ALIVE_CHECK();

				_g->stat.round_zero_set.udp.continuously_unsuccessful_select_on_open_port_count++;

				int error = 0;
				socklen_t errlen = sizeof( error );
				getsockopt( sockfd_max , SOL_SOCKET , SO_ERROR , &error , &errlen );
				if ( error != 0 )
				{
					_ECHO( "Socket error: %d\n" , error );
				}

				if ( ++input_udp_socket_error_tolerance_count > RETRY_UNEXPECTED_WAIT_FOR_SOCK() )
				{
					input_udp_socket_error_tolerance_count = 0;
					for ( int i = 0 ; i < _g->bridges.pb_holders_masks_count ; i++ )
					{
						if ( _g->bridges.pb_holders_masks[ i ] )
						{
							if ( _g->bridges.pb_holders[ i ].alc_pb->udp_connection_established )  // all the connected udp stoped or die so restart them
							{
								//if ( FD_ISSET( _g->bridges.pb_holders[ i ].alc_pb->udp_sockfd , &readfds ) )
								{
									_g->bridges.pb_holders[ i ].alc_pb->retry_to_connect_udp = 1;
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
				//SYS_ALIVE_CHECK();

				_g->stat.round_zero_set.udp.continuously_unsuccessful_select_on_open_port_count++;

				int error = 0;
				socklen_t errlen = sizeof( error );
				getsockopt( sockfd_max , SOL_SOCKET , SO_ERROR , &error , &errlen );
				if ( error != 0 )
				{
					_ECHO( "Socket error: %d\n" , error );
				}

				if ( ++input_udp_socket_error_tolerance_count > RETRY_UNEXPECTED_WAIT_FOR_SOCK() )
				{
					input_udp_socket_error_tolerance_count = 0;
					for ( int i = 0 ; i < _g->bridges.pb_holders_masks_count ; i++ )
					{
						if ( _g->bridges.pb_holders_masks[ i ] )
						{
							if ( _g->bridges.pb_holders[ i ].alc_pb->udp_connection_established )  // all the connected udp stoped or die so restart them
							{
								//if ( FD_ISSET( _g->bridges.pb_holders[ i ].alc_pb->udp_sockfd , &readfds ) )
								{
									_g->bridges.pb_holders[ i ].alc_pb->retry_to_connect_udp = 1;
									break;
								}
							}
						}
					}
				}

				continue;
			}

			_g->stat.round_zero_set.udp.continuously_unsuccessful_select_on_open_port_count = 0;

			//SYS_ALIVE_CHECK();

			tnow = time( NULL );
			// udp
			if ( difftime( tnow , _g->stat.round_zero_set.udp_1_sec.t_udp_throughput ) >= 1.0 )
			{
				if ( _g->stat.round_zero_set.udp_1_sec.t_udp_throughput > 0 )
				{
					// add it here

					circbuf_advance( &_g->stat.round_init_set.udp_stat_5_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
					circbuf_advance( &_g->stat.round_init_set.udp_stat_5_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );
					
					circbuf_advance( &_g->stat.round_init_set.udp_stat_10_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
					circbuf_advance( &_g->stat.round_init_set.udp_stat_10_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );
					
					circbuf_advance( &_g->stat.round_init_set.udp_stat_40_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
					circbuf_advance( &_g->stat.round_init_set.udp_stat_40_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );
					
					circbuf_advance( &_g->stat.round_init_set.udp_stat_120_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
					circbuf_advance( &_g->stat.round_init_set.udp_stat_120_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );

					//_g->stat.round.udp_1_sec.udp_get_count_throughput = _g->stat.round.udp_1_sec.calc_throughput_udp_get_count;
					//_g->stat.round.udp_1_sec.udp_get_byte_throughput = _g->stat.round.udp_1_sec.calc_throughput_udp_get_bytes;
				}
				_g->stat.round_zero_set.udp_1_sec.t_udp_throughput = tnow;
				_g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count = 0;
				_g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes = 0;
			}
			//if ( difftime( tnow , _g->stat.round.udp_10_sec.t_udp_throughput ) >= 10.0 )
			//{
			//	if ( _g->stat.round.udp_10_sec.t_udp_throughput > 0 )
			//	{
			//		_g->stat.round.udp_10_sec.udp_get_count_throughput = _g->stat.round.udp_10_sec.calc_throughput_udp_get_count;
			//		_g->stat.round.udp_10_sec.udp_get_byte_throughput = _g->stat.round.udp_10_sec.calc_throughput_udp_get_bytes;
			//	}
			//	_g->stat.round.udp_10_sec.t_udp_throughput = tnow;
			//	_g->stat.round.udp_10_sec.calc_throughput_udp_get_count = 0;
			//	_g->stat.round.udp_10_sec.calc_throughput_udp_get_bytes = 0;
			//}
			//if ( difftime( tnow , _g->stat.round.udp_40_sec.t_udp_throughput ) >= 40.0 )
			//{
			//	if ( _g->stat.round.udp_40_sec.t_udp_throughput > 0 )
			//	{
			//		_g->stat.round.udp_40_sec.udp_get_count_throughput = _g->stat.round.udp_40_sec.calc_throughput_udp_get_count;
			//		_g->stat.round.udp_40_sec.udp_get_byte_throughput = _g->stat.round.udp_40_sec.calc_throughput_udp_get_bytes;
			//	}
			//	_g->stat.round.udp_40_sec.t_udp_throughput = tnow;
			//	_g->stat.round.udp_40_sec.calc_throughput_udp_get_count = 0;
			//	_g->stat.round.udp_40_sec.calc_throughput_udp_get_bytes = 0;
			//}


			// tcp
			if ( difftime( tnow , _g->stat.round_zero_set.tcp_1_sec.t_tcp_throughput ) >= 1.0 )
			{
				if ( _g->stat.round_zero_set.tcp_1_sec.t_tcp_throughput > 0 )
				{
					circbuf_advance( &_g->stat.round_init_set.tcp_stat_5_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
					circbuf_advance( &_g->stat.round_init_set.tcp_stat_5_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );

					circbuf_advance( &_g->stat.round_init_set.tcp_stat_10_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
					circbuf_advance( &_g->stat.round_init_set.tcp_stat_10_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );

					circbuf_advance( &_g->stat.round_init_set.tcp_stat_40_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
					circbuf_advance( &_g->stat.round_init_set.tcp_stat_40_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );

					circbuf_advance( &_g->stat.round_init_set.tcp_stat_120_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
					circbuf_advance( &_g->stat.round_init_set.tcp_stat_120_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );

					//_g->stat.round_zero_set.tcp_1_sec.tcp_put_count_throughput = _g->stat.round_zero_set.tcp_1_sec.calc_throughput_tcp_put_count;
					//_g->stat.round_zero_set.tcp_1_sec.tcp_put_byte_throughput = _g->stat.round_zero_set.tcp_1_sec.calc_throughput_tcp_put_bytes;
				}
				_g->stat.round_zero_set.tcp_1_sec.t_tcp_throughput = tnow;
				_g->stat.round_zero_set.tcp_1_sec.calc_throughput_tcp_put_count = 0;
				_g->stat.round_zero_set.tcp_1_sec.calc_throughput_tcp_put_bytes = 0;
			}
			//if ( difftime( tnow , _g->stat.round.tcp_10_sec.t_tcp_throughput ) >= 10.0 )
			//{
			//	if ( _g->stat.round.tcp_10_sec.t_tcp_throughput > 0 )
			//	{
			//		_g->stat.round.tcp_10_sec.tcp_put_count_throughput = _g->stat.round.tcp_10_sec.calc_throughput_tcp_put_count;
			//		_g->stat.round.tcp_10_sec.tcp_put_byte_throughput = _g->stat.round.tcp_10_sec.calc_throughput_tcp_put_bytes;
			//	}
			//	_g->stat.round.tcp_10_sec.t_tcp_throughput = tnow;
			//	_g->stat.round.tcp_10_sec.calc_throughput_tcp_put_count = 0;
			//	_g->stat.round.tcp_10_sec.calc_throughput_tcp_put_bytes = 0;
			//}
			//if ( difftime( tnow , _g->stat.round.tcp_40_sec.t_tcp_throughput ) >= 40.0 )
			//{
			//	if ( _g->stat.round.tcp_40_sec.t_tcp_throughput > 0 )
			//	{
			//		_g->stat.round.tcp_40_sec.tcp_put_count_throughput = _g->stat.round.tcp_40_sec.calc_throughput_tcp_put_count;
			//		_g->stat.round.tcp_40_sec.tcp_put_byte_throughput = _g->stat.round.tcp_40_sec.calc_throughput_tcp_put_bytes;
			//	}
			//	_g->stat.round.tcp_40_sec.t_tcp_throughput = tnow;
			//	_g->stat.round.tcp_40_sec.calc_throughput_tcp_put_count = 0;
			//	_g->stat.round.tcp_40_sec.calc_throughput_tcp_put_bytes = 0;
			//}

			for ( int i = 0 ; i < _g->bridges.pb_holders_masks_count ; i++ )
			{
				if ( _g->bridges.pb_holders_masks[ i ] )
				{
					if ( _g->bridges.pb_holders[ i ].alc_pb->udp_connection_established && _g->bridges.pb_holders[ i ].alc_pb->tcp_connection_established )
					{
						if ( FD_ISSET( _g->bridges.pb_holders[ i ].alc_pb->udp_sockfd , &readfds ) )
						{
							//SYS_ALIVE_CHECK();
							bytes_received = recvfrom( _g->bridges.pb_holders[ i ].alc_pb->udp_sockfd , buffer , BUFFER_SIZE , MSG_WAITALL , ( struct sockaddr * )&client_addr , &client_len ); // good for udp data recieve
							if ( bytes_received <= 0 )
							{
								_g->stat.round_zero_set.continuously_unsuccessful_receive_error++;
								_g->stat.round_zero_set.total_unsuccessful_receive_error++;
								continue;
							}
							_g->stat.round_zero_set.continuously_unsuccessful_receive_error = 0;
							//buffer[ bytes_received ] = '\0'; // Null-terminate the received data

							_g->stat.round_zero_set.udp.total_udp_get_count++;
							_g->stat.round_zero_set.udp.total_udp_get_byte += bytes_received;
							_g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count++;
							_g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes += bytes_received;
							//_g->stat.round.udp_10_sec.calc_throughput_udp_get_count++;
							//_g->stat.round.udp_10_sec.calc_throughput_udp_get_bytes += bytes_received;
							//_g->stat.round.udp_40_sec.calc_throughput_udp_get_count++;
							//_g->stat.round.udp_40_sec.calc_throughput_udp_get_bytes += bytes_received;

							// Send data over TCP
							if ( ( sz = send( _g->bridges.pb_holders[ i ].alc_pb->tcp_sockfd , buffer , ( size_t )bytes_received , MSG_NOSIGNAL ) ) == -1 )
							{
								_g->stat.round_zero_set.continuously_unsuccessful_send_error++;
								_g->stat.round_zero_set.total_unsuccessful_send_error++;

								if ( ++output_tcp_socket_error_tolerance_count > RETRY_UNEXPECTED_WAIT_FOR_SOCK() )
								{
									output_tcp_socket_error_tolerance_count = 0;
									for ( int i = 0 ; i < _g->bridges.pb_holders_masks_count ; i++ )
									{
										if ( _g->bridges.pb_holders_masks[ i ] )
										{
											if ( _g->bridges.pb_holders[ i ].alc_pb->tcp_connection_established )  // all the connected udp stoped or die so restart them
											{
												//if ( FD_ISSET( _g->bridges.pb_holders[ i ].alc_pb->udp_sockfd , &readfds ) )
												{
													_g->bridges.pb_holders[ i ].alc_pb->retry_to_connect_tcp = 1;
													break;
												}
											}
										}
									}
								}

								continue;
							}
							_g->stat.round_zero_set.continuously_unsuccessful_send_error = 0;

							_g->stat.round_zero_set.tcp.total_tcp_put_count++;
							_g->stat.round_zero_set.tcp.total_tcp_put_byte += sz;
							_g->stat.round_zero_set.tcp_1_sec.calc_throughput_tcp_put_count++;
							_g->stat.round_zero_set.tcp_1_sec.calc_throughput_tcp_put_bytes += sz;
							//_g->stat.round.tcp_10_sec.calc_throughput_tcp_put_count++;
							//_g->stat.round.tcp_10_sec.calc_throughput_tcp_put_bytes += sz;
							//_g->stat.round.tcp_40_sec.calc_throughput_tcp_put_count++;
							//_g->stat.round.tcp_40_sec.calc_throughput_tcp_put_bytes += sz;

							//SYS_ALIVE_CHECK();

						}
					}
				}
			}
			//SYS_ALIVE_CHECK();
		}

	} while ( config_changes );
	//SYS_ALIVE_CHECK();
	BREAK_OK( 0 ); // to just ignore gcc warning

	BEGIN_RET
		case 3: {}
		case 2: {}
		case 1:
		{
			//_close_socket( &src_pb->tcp_sockfd );
			_g->stat.round_zero_set.syscal_err_count++;
		}
	M_V_END_RET

	return NULL;
}

#endif

#ifndef seperate_thread_for_input_output

void * income_thread_proc( void * src_g )
{
	INIT_BREAKABLE_FXN();

	//struct protocol_bridge * pb = ( struct protocol_bridge * )src_pb;
	struct App_Data * _g = ( struct App_Data * )src_g;

	time_t tnow = 0;

	while ( !_g->bridges.thread_base.start_working )
	{
		if ( _g->bridges.thread_base.do_close_thread )
		{
			break;
		}
		sleep( 1 );
	}

	int input_udp_socket_error_tolerance_count = 0; // restart socket after many error accur

	int config_changes = 0;
	do
	{
		SYS_ALIVE_CHECK();
		if ( _g->bridges.thread_base.do_close_thread )
		{
			break;
		}

		config_changes = 0;

		char buffer[ BUFFER_SIZE ]; // Define a buffer to store received data
		struct sockaddr_in client_addr;
		socklen_t client_len = sizeof( client_addr );
		ssize_t bytes_received;

		fd_set readfds; // Set of socket descriptors
		FD_ZERO( &readfds );

		ssize_t sz;

		int sockfd_max = -1; // for select compulsion
		for ( int i = 0 ; i < _g->bridges.pb_holders_masks_count ; i++ )
		{
			if ( _g->bridges.pb_holders_masks[ i ] )
			{
				if ( _g->bridges.pb_holders[ i ].alc_pb->udp_connection_established /* && _g->bridges.pb_holders[i].alc_pb->tcp_connection_established*/ )
				{
					FD_SET( _g->bridges.pb_holders[ i ].alc_pb->udp_sockfd , &readfds );
					if ( _g->bridges.pb_holders[ i ].alc_pb->udp_sockfd > sockfd_max )
					{
						sockfd_max = _g->bridges.pb_holders[ i ].alc_pb->udp_sockfd;
					}
				}
			}
		}
		_g->bridges.under_listen_udp_sockets_group_changed = 0; // if any udp socket change then fdset must be reinitialized

		if ( sockfd_max <= 1 )
		{
			sleep( 1 );
			continue;
		}

		while ( 1 )
		{
			SYS_ALIVE_CHECK();

			//pthread_mutex_lock( &_g->sync.mutex );
			//while ( _g->sync.lock_in_progress )
			//{
			//	////struct timespec ts = { 0, 10L };
			//	////thrd_sleep( &ts , NULL );
			//	pthread_cond_wait( &_g->sync.cond , &_g->sync.mutex );
			//}
			//pthread_mutex_unlock( &_g->sync.mutex );
			//if ( _g->sync.reset_static_after_lock )
			//{
			//	_g->sync.reset_static_after_lock = 0;
			//	memset( &_g->stat.round , 0 , sizeof( _g->stat.round ) );
			//}

			if ( _g->bridges.thread_base.do_close_thread )
			{
				break;
			}
			if ( _g->bridges.under_listen_udp_sockets_group_changed )
			{
				config_changes = 1;
				break;
			}

			struct timeval timeout; // Set timeout (e.g., 5 seconds)
			timeout.tv_sec = 5;
			timeout.tv_usec = 0;

			// Wait for an activity on one of the sockets, timeout is NULL, so wait indefinitely
			int activity = select( sockfd_max + 1 , &readfds , NULL , NULL , &timeout );

			if ( ( activity < 0 ) /* && ( errno != EINTR )*/ )
			{
				//SYS_ALIVE_CHECK();

				_g->stat.round_zero_set.udp.continuously_unsuccessful_select_on_open_port_count++;

				int error = 0;
				socklen_t errlen = sizeof( error );
				getsockopt( sockfd_max , SOL_SOCKET , SO_ERROR , &error , &errlen );
				if ( error != 0 )
				{
					_ECHO( "Socket error: %d\n" , error );
				}

				if ( ++input_udp_socket_error_tolerance_count > RETRY_UNEXPECTED_WAIT_FOR_SOCK() )
				{
					input_udp_socket_error_tolerance_count = 0;
					for ( int i = 0 ; i < _g->bridges.pb_holders_masks_count ; i++ )
					{
						if ( _g->bridges.pb_holders_masks[ i ] )
						{
							if ( _g->bridges.pb_holders[ i ].alc_pb->udp_connection_established )  // all the connected udp stoped or die so restart them
							{
								//if ( FD_ISSET( _g->bridges.pb_holders[ i ].alc_pb->udp_sockfd , &readfds ) )
								{
									_g->bridges.pb_holders[ i ].alc_pb->retry_to_connect_udp = 1;
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
				//SYS_ALIVE_CHECK();

				_g->stat.round_zero_set.udp.continuously_unsuccessful_select_on_open_port_count++;

				int error = 0;
				socklen_t errlen = sizeof( error );
				getsockopt( sockfd_max , SOL_SOCKET , SO_ERROR , &error , &errlen );
				if ( error != 0 )
				{
					_ECHO( "Socket error: %d\n" , error );
				}

				if ( ++input_udp_socket_error_tolerance_count > RETRY_UNEXPECTED_WAIT_FOR_SOCK() )
				{
					input_udp_socket_error_tolerance_count = 0;
					for ( int i = 0 ; i < _g->bridges.pb_holders_masks_count ; i++ )
					{
						if ( _g->bridges.pb_holders_masks[ i ] )
						{
							if ( _g->bridges.pb_holders[ i ].alc_pb->udp_connection_established )  // all the connected udp stoped or die so restart them
							{
								//if ( FD_ISSET( _g->bridges.pb_holders[ i ].alc_pb->udp_sockfd , &readfds ) )
								{
									_g->bridges.pb_holders[ i ].alc_pb->retry_to_connect_udp = 1;
									break;
								}
							}
						}
					}
				}

				continue;
			}

			_g->stat.round_zero_set.udp.continuously_unsuccessful_select_on_open_port_count = 0;

			SYS_ALIVE_CHECK();

			tnow = time( NULL );
			// udp
			if ( difftime( tnow , _g->stat.round_zero_set.udp_1_sec.t_udp_throughput ) >= 1.0 )
			{
				if ( _g->stat.round_zero_set.udp_1_sec.t_udp_throughput > 0 )
				{
					circbuf_advance( &_g->stat.round_init_set.udp_stat_5_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
					circbuf_advance( &_g->stat.round_init_set.udp_stat_5_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );

					circbuf_advance( &_g->stat.round_init_set.udp_stat_10_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
					circbuf_advance( &_g->stat.round_init_set.udp_stat_10_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );

					circbuf_advance( &_g->stat.round_init_set.udp_stat_40_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
					circbuf_advance( &_g->stat.round_init_set.udp_stat_40_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );

					circbuf_advance( &_g->stat.round_init_set.udp_stat_120_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
					circbuf_advance( &_g->stat.round_init_set.udp_stat_120_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );
					//_g->stat.round.udp_1_sec.udp_get_count_throughput = _g->stat.round.udp_1_sec.calc_throughput_udp_get_count;
					//_g->stat.round.udp_1_sec.udp_get_byte_throughput = _g->stat.round.udp_1_sec.calc_throughput_udp_get_bytes;
				}
				_g->stat.round_zero_set.udp_1_sec.t_udp_throughput = tnow;
				_g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count = 0;
				_g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes = 0;
			}
			//if ( difftime( tnow , _g->stat.round.udp_10_sec.t_udp_throughput ) >= 10.0 )
			//{
			//	if ( _g->stat.round.udp_10_sec.t_udp_throughput > 0 )
			//	{
			//		_g->stat.round.udp_10_sec.udp_get_count_throughput = _g->stat.round.udp_10_sec.calc_throughput_udp_get_count;
			//		_g->stat.round.udp_10_sec.udp_get_byte_throughput = _g->stat.round.udp_10_sec.calc_throughput_udp_get_bytes;
			//	}
			//	_g->stat.round.udp_10_sec.t_udp_throughput = tnow;
			//	_g->stat.round.udp_10_sec.calc_throughput_udp_get_count = 0;
			//	_g->stat.round.udp_10_sec.calc_throughput_udp_get_bytes = 0;
			//}
			//if ( difftime( tnow , _g->stat.round.udp_40_sec.t_udp_throughput ) >= 40.0 )
			//{
			//	if ( _g->stat.round.udp_40_sec.t_udp_throughput > 0 )
			//	{
			//		_g->stat.round.udp_40_sec.udp_get_count_throughput = _g->stat.round.udp_40_sec.calc_throughput_udp_get_count;
			//		_g->stat.round.udp_40_sec.udp_get_byte_throughput = _g->stat.round.udp_40_sec.calc_throughput_udp_get_bytes;
			//	}
			//	_g->stat.round.udp_40_sec.t_udp_throughput = tnow;
			//	_g->stat.round.udp_40_sec.calc_throughput_udp_get_count = 0;
			//	_g->stat.round.udp_40_sec.calc_throughput_udp_get_bytes = 0;
			//}

			for ( int i = 0 ; i < _g->bridges.pb_holders_masks_count ; i++ )
			{
				if ( _g->bridges.pb_holders_masks[ i ] )
				{
					if ( _g->bridges.pb_holders[ i ].alc_pb->udp_connection_established /* && _g->bridges.pb_holders[i].alc_pb->tcp_connection_established*/ )
					{
						if ( FD_ISSET( _g->bridges.pb_holders[ i ].alc_pb->udp_sockfd , &readfds ) )
						{
							SYS_ALIVE_CHECK();
							bytes_received = recvfrom( _g->bridges.pb_holders[ i ].alc_pb->udp_sockfd , buffer , BUFFER_SIZE , MSG_WAITALL , ( struct sockaddr * )&client_addr , &client_len ); // good for udp data recieve
							if ( bytes_received <= 0 )
							{
								_g->stat.round_zero_set.continuously_unsuccessful_receive_error++;
								_g->stat.round_zero_set.total_unsuccessful_receive_error++;
								continue;
							}
							_g->stat.round_zero_set.continuously_unsuccessful_receive_error = 0;
							//buffer[ bytes_received ] = '\0'; // Null-terminate the received data

							_g->stat.round_zero_set.udp.total_udp_get_count++;
							_g->stat.round_zero_set.udp.total_udp_get_byte += bytes_received;
							_g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count++;
							_g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes += bytes_received;
							//_g->stat.round.udp_10_sec.calc_throughput_udp_get_count++;
							//_g->stat.round.udp_10_sec.calc_throughput_udp_get_bytes += bytes_received;
							//_g->stat.round.udp_40_sec.calc_throughput_udp_get_count++;
							//_g->stat.round.udp_40_sec.calc_throughput_udp_get_bytes += bytes_received;

							queue_push( &_g->bridges.bidirection_thread->queue , buffer , bytes_received );
							SYS_ALIVE_CHECK();
						}
					}
				}
			}
			SYS_ALIVE_CHECK();
		}

	} while ( config_changes );
	SYS_ALIVE_CHECK();
	BREAK_OK( 0 ); // to just ignore gcc warning

	BEGIN_RET
		case 3: {}
		case 2: {}
		case 1:
		{
			//_close_socket( &src_pb->tcp_sockfd );
			_g->stat.round_zero_set.syscal_err_count++;
		}
	M_V_END_RET

	return NULL;
}

void * outgoing_thread_proc( void * src_g )
{
	INIT_BREAKABLE_FXN();

	//struct protocol_bridge * pb = ( struct protocol_bridge * )src_pb;
	struct App_Data * _g = ( struct App_Data * )src_g;

	time_t tnow = 0;
	char buffer[ BUFFER_SIZE ]; // Define a buffer to store received data
	size_t sz;
	ssize_t snd_ret;

	int output_tcp_socket_error_tolerance_count = 0; // restart socket after many error accur

	while( !queue_peek_available( &_g->bridges.bidirection_thread->queue ) )
	{
		sleep(1);
	}

	while ( 1 )
	{
		SYS_ALIVE_CHECK();

		//pthread_mutex_lock( &_g->sync.mutex );
		//while ( _g->sync.lock_in_progress )
		//{
		//	////struct timespec ts = { 0, 10L };
		//	////thrd_sleep( &ts , NULL );
		//	pthread_cond_wait( &_g->sync.cond , &_g->sync.mutex );
		//}
		//pthread_mutex_unlock( &_g->sync.mutex );
		//if ( _g->sync.reset_static_after_lock )
		//{
		//	_g->sync.reset_static_after_lock = 0;
		//	memset( &_g->stat.round , 0 , sizeof( _g->stat.round ) );
		//}

		if ( _g->bridges.thread_base.do_close_thread )
		{
			break;
		}

		tnow = time( NULL );
		// tcp
		if ( difftime( tnow , _g->stat.round_zero_set.tcp_1_sec.t_tcp_throughput ) >= 1.0 )
		{
			if ( _g->stat.round_zero_set.tcp_1_sec.t_tcp_throughput > 0 )
			{
				circbuf_advance( &_g->stat.round_init_set.tcp_stat_5_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
				circbuf_advance( &_g->stat.round_init_set.tcp_stat_5_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );

				circbuf_advance( &_g->stat.round_init_set.tcp_stat_10_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
				circbuf_advance( &_g->stat.round_init_set.tcp_stat_10_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );

				circbuf_advance( &_g->stat.round_init_set.tcp_stat_40_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
				circbuf_advance( &_g->stat.round_init_set.tcp_stat_40_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );

				circbuf_advance( &_g->stat.round_init_set.tcp_stat_120_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
				circbuf_advance( &_g->stat.round_init_set.tcp_stat_120_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );

				//_g->stat.round.tcp_1_sec.tcp_put_count_throughput = _g->stat.round.tcp_1_sec.calc_throughput_tcp_put_count;
				//_g->stat.round.tcp_1_sec.tcp_put_byte_throughput = _g->stat.round.tcp_1_sec.calc_throughput_tcp_put_bytes;
			}
			_g->stat.round_zero_set.tcp_1_sec.t_tcp_throughput = tnow;
			_g->stat.round_zero_set.tcp_1_sec.calc_throughput_tcp_put_count = 0;
			_g->stat.round_zero_set.tcp_1_sec.calc_throughput_tcp_put_bytes = 0;
		}
		//if ( difftime( tnow , _g->stat.round.tcp_10_sec.t_tcp_throughput ) >= 10.0 )
		//{
		//	if ( _g->stat.round.tcp_10_sec.t_tcp_throughput > 0 )
		//	{
		//		_g->stat.round.tcp_10_sec.tcp_put_count_throughput = _g->stat.round.tcp_10_sec.calc_throughput_tcp_put_count;
		//		_g->stat.round.tcp_10_sec.tcp_put_byte_throughput = _g->stat.round.tcp_10_sec.calc_throughput_tcp_put_bytes;
		//	}
		//	_g->stat.round.tcp_10_sec.t_tcp_throughput = tnow;
		//	_g->stat.round.tcp_10_sec.calc_throughput_tcp_put_count = 0;
		//	_g->stat.round.tcp_10_sec.calc_throughput_tcp_put_bytes = 0;
		//}
		//if ( difftime( tnow , _g->stat.round.tcp_40_sec.t_tcp_throughput ) >= 40.0 )
		//{
		//	if ( _g->stat.round.tcp_40_sec.t_tcp_throughput > 0 )
		//	{
		//		_g->stat.round.tcp_40_sec.tcp_put_count_throughput = _g->stat.round.tcp_40_sec.calc_throughput_tcp_put_count;
		//		_g->stat.round.tcp_40_sec.tcp_put_byte_throughput = _g->stat.round.tcp_40_sec.calc_throughput_tcp_put_bytes;
		//	}
		//	_g->stat.round.tcp_40_sec.t_tcp_throughput = tnow;
		//	_g->stat.round.tcp_40_sec.calc_throughput_tcp_put_count = 0;
		//	_g->stat.round.tcp_40_sec.calc_throughput_tcp_put_bytes = 0;
		//}

		queue_pop( &_g->bridges.bidirection_thread->queue , buffer , &sz );
		
		for ( int i = 0 ; i < _g->bridges.pb_holders_masks_count ; i++ )
		{
			if ( _g->bridges.pb_holders_masks[ i ] && _g->bridges.pb_holders[ i ].alc_pb->tcp_connection_established )
			{
				if ( ( snd_ret = send( _g->bridges.pb_holders[ i ].alc_pb->tcp_sockfd , buffer , ( size_t )sz , MSG_NOSIGNAL ) ) == -1 )
				{
					_g->stat.round_zero_set.continuously_unsuccessful_send_error++;
					_g->stat.round_zero_set.total_unsuccessful_send_error++;

					if ( ++output_tcp_socket_error_tolerance_count > RETRY_UNEXPECTED_WAIT_FOR_SOCK() )
					{
						output_tcp_socket_error_tolerance_count = 0;
						for ( int i = 0 ; i < _g->bridges.pb_holders_masks_count ; i++ )
						{
							if ( _g->bridges.pb_holders_masks[ i ] )
							{
								if ( _g->bridges.pb_holders[ i ].alc_pb->tcp_connection_established )  // all the connected udp stoped or die so restart them
								{
									//if ( FD_ISSET( _g->bridges.pb_holders[ i ].alc_pb->udp_sockfd , &readfds ) )
									{
										_g->bridges.pb_holders[ i ].alc_pb->retry_to_connect_tcp = 1;
										break;
									}
								}
							}
						}
					}

					continue;
				}
				_g->stat.round_zero_set.continuously_unsuccessful_send_error = 0;
				if ( snd_ret > 0 )
				{
					_g->stat.round_zero_set.tcp.total_tcp_put_count++;
					_g->stat.round_zero_set.tcp.total_tcp_put_byte += snd_ret;
					_g->stat.round_zero_set.tcp_1_sec.calc_throughput_tcp_put_count++;
					_g->stat.round_zero_set.tcp_1_sec.calc_throughput_tcp_put_bytes += snd_ret;
					//_g->stat.round.tcp_10_sec.calc_throughput_tcp_put_count++;
					//_g->stat.round.tcp_10_sec.calc_throughput_tcp_put_bytes += snd_ret;
					//_g->stat.round.tcp_40_sec.calc_throughput_tcp_put_count++;
					//_g->stat.round.tcp_40_sec.calc_throughput_tcp_put_bytes += snd_ret;
				}
			}
		}

		SYS_ALIVE_CHECK();
	}

	SYS_ALIVE_CHECK();
	BREAK_OK(0); // to just ignore gcc warning

	BEGIN_RET
		case 3: {}
		case 2: {}
		case 1:
		{
			//_close_socket( &src_pb->tcp_sockfd );
			_g->stat.round_zero_set.syscal_err_count++;
		}
	M_V_END_RET

	return NULL;
}

#endif

#ifndef thread_just_for_input

void * justIncoming_thread_proc( void * src_g )
{
	INIT_BREAKABLE_FXN();

	//struct protocol_bridge * pb = ( struct protocol_bridge * )src_pb;
	struct App_Data * _g = ( struct App_Data * )src_g;

	//SYS_ALIVE_CHECK();

	while ( !_g->bridges.thread_base.start_working )
	{
		if ( _g->bridges.thread_base.do_close_thread )
		{
			break;
		}
		sleep(1);
	}

	SYS_ALIVE_CHECK();

	time_t tnow = 0;

	int input_udp_socket_error_tolerance_count = 0; // restart socket after many error accur

	int config_changes = 0;
	do
	{
		//SYS_ALIVE_CHECK();
		if ( _g->bridges.thread_base.do_close_thread )
		{
			break;
		}
		//SYS_ALIVE_CHECK();
		config_changes = 0;

		char buffer[ BUFFER_SIZE ]; // Define a buffer to store received data
		struct sockaddr_in client_addr;
		socklen_t client_len = sizeof( client_addr );
		ssize_t bytes_received;

		fd_set readfds; // Set of socket descriptors
		FD_ZERO( &readfds );

		ssize_t sz;

		int sockfd_max = -1; // for select compulsion
		for ( int i = 0 ; i < _g->bridges.pb_holders_masks_count ; i++ )
		{
			if ( _g->bridges.pb_holders_masks[ i ] )
			{
				//SYS_ALIVE_CHECK();
				if ( _g->bridges.pb_holders[ i ].alc_pb->udp_connection_established /* && _g->bridges.pb_holders[i].alc_pb->tcp_connection_established*/ )
				{
					SYS_ALIVE_CHECK();
					FD_SET( _g->bridges.pb_holders[ i ].alc_pb->udp_sockfd , &readfds );
					//SYS_ALIVE_CHECK();
					if ( _g->bridges.pb_holders[ i ].alc_pb->udp_sockfd > sockfd_max )
					{
						//SYS_ALIVE_CHECK();
						sockfd_max = _g->bridges.pb_holders[ i ].alc_pb->udp_sockfd;
					}
				}
			}
		}
		_g->bridges.under_listen_udp_sockets_group_changed = 0; // if any udp socket change then fdset must be reinitialized
		//SYS_ALIVE_CHECK();
		if ( sockfd_max < 0 )
		{
			sleep( 1 );
			continue;
		}

		while ( 1 )
		{
			//SYS_ALIVE_CHECK();

			//pthread_mutex_lock( &_g->sync.mutex );
			//while ( _g->sync.lock_in_progress )
			//{
			//	////struct timespec ts = { 0, 10L };
			//	////thrd_sleep( &ts , NULL );
			//	pthread_cond_wait( &_g->sync.cond , &_g->sync.mutex );
			//}
			//pthread_mutex_unlock( &_g->sync.mutex );
			//if ( _g->sync.reset_static_after_lock )
			//{
			//	_g->sync.reset_static_after_lock = 0;
			//	memset( &_g->stat.round , 0 , sizeof( _g->stat.round ) );
			//}

			//SYS_ALIVE_CHECK();

			if ( _g->bridges.thread_base.do_close_thread )
			{
				break;
			}
			if ( _g->bridges.under_listen_udp_sockets_group_changed )
			{
				config_changes = 1;
				break;
			}

			//struct timeval timeout; // Set timeout (e.g., 5 seconds)
			//timeout.tv_sec = ( input_udp_socket_error_tolerance_count + 1 ) * 2;
			//timeout.tv_usec = 0;

			// Wait for an activity on one of the sockets, timeout is NULL, so wait indefinitely
			int activity = select( sockfd_max + 1 , &readfds , NULL , NULL , NULL/* & timeout*/ );

			if ( ( activity < 0 ) /* && ( errno != EINTR )*/ )
			{
				//SYS_ALIVE_CHECK();

				int error = 0;
				socklen_t errlen = sizeof( error );
				getsockopt( sockfd_max , SOL_SOCKET , SO_ERROR , &error , &errlen );
				if ( error != 0 )
				{
					_VERBOSE_ECHO( "Socket error: %d\n" , error );
				}

				if ( ++input_udp_socket_error_tolerance_count > RETRY_UNEXPECTED_WAIT_FOR_SOCK() )
				{
					input_udp_socket_error_tolerance_count = 0;
					for ( int i = 0 ; i < _g->bridges.pb_holders_masks_count ; i++ )
					{
						if ( _g->bridges.pb_holders_masks[ i ] )
						{
							if ( _g->bridges.pb_holders[ i ].alc_pb->udp_connection_established ) // all the connected udp stoped or die so restart them
							{
								//if ( FD_ISSET( _g->bridges.pb_holders[ i ].alc_pb->udp_sockfd , &readfds ) )
								{
									_g->bridges.pb_holders[ i ].alc_pb->retry_to_connect_udp = 1;
									break;
								}
							}
						}
					}
				}

				continue;
			}
			if ( activity == 0 ) // timed out
			{
				//SYS_ALIVE_CHECK();

				int error = 0;
				socklen_t errlen = sizeof( error );
				getsockopt( sockfd_max , SOL_SOCKET , SO_ERROR , &error , &errlen );
				if ( error == 0 )
				{
					_g->stat.round_zero_set.udp.continuously_unsuccessful_select_on_open_port_count++;
					if ( ++input_udp_socket_error_tolerance_count > RETRY_UNEXPECTED_WAIT_FOR_SOCK() )
					{
						input_udp_socket_error_tolerance_count = 0;
						for ( int i = 0 ; i < _g->bridges.pb_holders_masks_count ; i++ )
						{
							if ( _g->bridges.pb_holders_masks[ i ] )
							{
								if ( _g->bridges.pb_holders[ i ].alc_pb->udp_connection_established ) // all the connected udp stoped or die so restart them
								{
									//if ( FD_ISSET( _g->bridges.pb_holders[ i ].alc_pb->udp_sockfd , &readfds ) )
									{
										_g->bridges.pb_holders[ i ].alc_pb->retry_to_connect_udp = 1;
										break;
									}
								}
							}
						}
					}
					continue;
				}
				_VERBOSE_ECHO( "Socket error: %d\n" , error );
				
				continue;
			}

			_g->stat.round_zero_set.udp.continuously_unsuccessful_select_on_open_port_count = 0;

			if ( _g->stat.round_zero_set.t_begin.tv_sec == 0 && _g->stat.round_zero_set.t_begin.tv_usec == 0 )
			{
				gettimeofday(&_g->stat.round_zero_set.t_begin, NULL);
			}

			//SYS_ALIVE_CHECK();

			tnow = time( NULL );
			// udp
			if ( difftime( tnow , _g->stat.round_zero_set.udp_1_sec.t_udp_throughput ) >= 1.0 )
			{
				if ( _g->stat.round_zero_set.udp_1_sec.t_udp_throughput > 0 )
				{
					circbuf_advance( &_g->stat.round_init_set.udp_stat_5_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
					circbuf_advance( &_g->stat.round_init_set.udp_stat_5_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );

					circbuf_advance( &_g->stat.round_init_set.udp_stat_10_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
					circbuf_advance( &_g->stat.round_init_set.udp_stat_10_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );

					circbuf_advance( &_g->stat.round_init_set.udp_stat_40_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
					circbuf_advance( &_g->stat.round_init_set.udp_stat_40_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );

					circbuf_advance( &_g->stat.round_init_set.udp_stat_120_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
					circbuf_advance( &_g->stat.round_init_set.udp_stat_120_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );

					//_g->stat.round.udp_1_sec.udp_get_count_throughput = _g->stat.round.udp_1_sec.calc_throughput_udp_get_count;
					//_g->stat.round.udp_1_sec.udp_get_byte_throughput = _g->stat.round.udp_1_sec.calc_throughput_udp_get_bytes;
				}
				_g->stat.round_zero_set.udp_1_sec.t_udp_throughput = tnow;
				_g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count = 0;
				_g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes = 0;
			}
			//if ( difftime( tnow , _g->stat.round.udp_10_sec.t_udp_throughput ) >= 10.0 )
			//{
			//	if ( _g->stat.round.udp_10_sec.t_udp_throughput > 0 )
			//	{
			//		_g->stat.round.udp_10_sec.udp_get_count_throughput = _g->stat.round.udp_10_sec.calc_throughput_udp_get_count;
			//		_g->stat.round.udp_10_sec.udp_get_byte_throughput = _g->stat.round.udp_10_sec.calc_throughput_udp_get_bytes;
			//	}
			//	_g->stat.round.udp_10_sec.t_udp_throughput = tnow;
			//	_g->stat.round.udp_10_sec.calc_throughput_udp_get_count = 0;
			//	_g->stat.round.udp_10_sec.calc_throughput_udp_get_bytes = 0;
			//}
			//if ( difftime( tnow , _g->stat.round.udp_40_sec.t_udp_throughput ) >= 40.0 )
			//{
			//	if ( _g->stat.round.udp_40_sec.t_udp_throughput > 0 )
			//	{
			//		_g->stat.round.udp_40_sec.udp_get_count_throughput = _g->stat.round.udp_40_sec.calc_throughput_udp_get_count;
			//		_g->stat.round.udp_40_sec.udp_get_byte_throughput = _g->stat.round.udp_40_sec.calc_throughput_udp_get_bytes;
			//	}
			//	_g->stat.round.udp_40_sec.t_udp_throughput = tnow;
			//	_g->stat.round.udp_40_sec.calc_throughput_udp_get_count = 0;
			//	_g->stat.round.udp_40_sec.calc_throughput_udp_get_bytes = 0;
			//}


			for ( int i = 0 ; i < _g->bridges.pb_holders_masks_count ; i++ )
			{
				if ( _g->bridges.pb_holders_masks[ i ] )
				{
					if ( _g->bridges.pb_holders[ i ].alc_pb->udp_connection_established )
					{
						if ( FD_ISSET( _g->bridges.pb_holders[ i ].alc_pb->udp_sockfd , &readfds ) )
						{
							//SYS_ALIVE_CHECK();
							bytes_received = recvfrom( _g->bridges.pb_holders[ i ].alc_pb->udp_sockfd , buffer , BUFFER_SIZE , MSG_WAITALL , ( struct sockaddr * )&client_addr , &client_len ); // good for udp data recieve
							if ( bytes_received < 0 )
							{
								_g->stat.round_zero_set.continuously_unsuccessful_receive_error++;
								_g->stat.round_zero_set.total_unsuccessful_receive_error++;
								continue;
							}
							_g->stat.round_zero_set.continuously_unsuccessful_receive_error = 0;
							//buffer[ bytes_received ] = '\0'; // Null-terminate the received data

							gettimeofday(&_g->stat.round_zero_set.t_end, NULL);

							_g->stat.round_zero_set.udp.total_udp_get_count++;
							_g->stat.round_zero_set.udp.total_udp_get_byte += bytes_received;
							_g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count++;
							_g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes += bytes_received;
							//_g->stat.round.udp_10_sec.calc_throughput_udp_get_count++;
							//_g->stat.round.udp_10_sec.calc_throughput_udp_get_bytes += bytes_received;
							//_g->stat.round.udp_40_sec.calc_throughput_udp_get_count++;
							//_g->stat.round.udp_40_sec.calc_throughput_udp_get_bytes += bytes_received;

							//SYS_ALIVE_CHECK();
						}
					}
				}
			}
			//SYS_ALIVE_CHECK();
		}

	} while ( config_changes );
	
	//SYS_ALIVE_CHECK();
	BREAK_OK( 0 ); // to just ignore gcc warning

	BEGIN_RET
		case 3: {}
		case 2: {}
		case 1:
		{
			//_close_socket( &src_pb->tcp_sockfd );
			_g->stat.round_zero_set.syscal_err_count++;
		}
	M_V_END_RET

	return NULL;
}

#endif

void * protocol_bridge_runner( void * src_pb )
{
	INIT_BREAKABLE_FXN();

	struct protocol_bridge * pb = ( struct protocol_bridge * )src_pb;
	struct App_Data * _g = pb->apcfg.m.m.temp_data._g;
	pthread_t tid = pthread_self();
	//time_t tnow = 0;
	struct protocol_bridge_thread * pthread = NULL;
	while ( pthread == NULL )
	{
		for ( int i = 0 ; i < pb->pb_trds_masks_count ; i++ )
		{
			if ( pb->pb_trds_masks[ i ] )
			{
				if ( pb->pb_trds[ i ].alc_thread->trd_id == tid )
				{
					pthread = pb->pb_trds[ i ].alc_thread;
					break;
				}
			}
		}
	}
	//SYS_ALIVE_CHECK();
	if ( pthread == NULL )
	{
		_g->stat.round_zero_set.syscal_err_count++;
		return NULL;
	}

	if ( IF_VERBOSE_MODE_CONDITION() )
	{
		_ECHO( "thread started" );
	}

	if ( _g->appcfg._general_config->c.c.atht == buttleneck )
	{
		if ( _g->bridges.bottleneck_thread != NULL )
		{
			pthread_mutex_lock( &_g->bridges.thread_base.creation_thread_race_cond );
			if ( !_g->bridges.thread_base.thread_is_created )
			{
				MM_BREAK_IF( pthread_create( &_g->bridges.bottleneck_thread->trd_id , NULL , bottleneck_thread_proc , _g ) != PTHREAD_CREATE_OK , errGeneral , 0 , "thread creation failed" );
				_g->bridges.thread_base.thread_is_created = 1;
			}
			pthread_mutex_unlock( &_g->bridges.thread_base.creation_thread_race_cond );
		}
	}

	if ( _g->appcfg._general_config->c.c.atht == bidirection )
	{
		if ( _g->bridges.bidirection_thread != NULL )
		{
			pthread_mutex_lock( &_g->bridges.thread_base.creation_thread_race_cond );
			if ( !_g->bridges.thread_base.thread_is_created )
			{
				MM_BREAK_IF( pthread_create( &_g->bridges.bidirection_thread->mem.income_trd_id , NULL , income_thread_proc , _g ) != PTHREAD_CREATE_OK , errGeneral , 0 , "thread creation failed" );
				MM_BREAK_IF( pthread_create( &_g->bridges.bidirection_thread->mem.outgoing_trd_id , NULL , outgoing_thread_proc , _g ) != PTHREAD_CREATE_OK , errGeneral , 0 , "thread creation failed" );

				_g->bridges.thread_base.thread_is_created = 1;
			}
			pthread_mutex_unlock( &_g->bridges.thread_base.creation_thread_race_cond );
		}
	}

	if ( _g->appcfg._general_config->c.c.atht == justIncoming )
	{
		if ( _g->bridges.justIncoming_thread != NULL )
		{
			pthread_mutex_lock( &_g->bridges.thread_base.creation_thread_race_cond );
			if ( !_g->bridges.thread_base.thread_is_created )
			{
				MM_BREAK_IF( pthread_create( &_g->bridges.justIncoming_thread->trd_id , NULL , justIncoming_thread_proc , _g ) != PTHREAD_CREATE_OK , errGeneral , 0 , "thread creation failed" );
				_g->bridges.thread_base.thread_is_created = 1;
			}
			pthread_mutex_unlock( &_g->bridges.thread_base.creation_thread_race_cond );
		}
	}

	int try_to_connect_udp_port = 1; // for the first time
	int try_to_connect_tcp_port = 1; // for the first time

	pthread->base_config_change_applied = 0;
	do
	{
		//SYS_ALIVE_CHECK();
		if ( pthread->do_close_thread )
		{
			break;
		}

		if ( pthread->base_config_change_applied ) // config change cause reconnect
		{
			pthread->base_config_change_applied = 0;
			try_to_connect_udp_port = 1;
			try_to_connect_tcp_port = 1;
		}
		if ( pb->retry_to_connect_udp ) // retry from socket err cause reconnect
		{
			pb->retry_to_connect_udp = 0;
			try_to_connect_udp_port = 1;
		}
		if ( pb->retry_to_connect_tcp ) // retry from socket err cause reconnect
		{
			pb->retry_to_connect_tcp = 0;
			try_to_connect_tcp_port = 1;
		}
		if ( !try_to_connect_udp_port && !try_to_connect_tcp_port )
		{
			sleep(2);
			continue;
		}
		int tmp_try_to_connect_udp_port = try_to_connect_udp_port;
		int tmp_try_to_connect_tcp_port = try_to_connect_tcp_port;
		try_to_connect_udp_port = 0;
		try_to_connect_tcp_port = 0;

		if ( _g->appcfg._general_config->c.c.atht == buttleneck || _g->appcfg._general_config->c.c.atht == bidirection )
		{
			// first close then reconnect
			if ( tmp_try_to_connect_udp_port )
			{
				pthread_t trd_udp_connection;
				MM_BREAK_IF( pthread_create( &trd_udp_connection , NULL , thread_udp_connection_proc , pb ) != PTHREAD_CREATE_OK , errGeneral , 0 , "thread creation failed" );
			}
			if ( tmp_try_to_connect_tcp_port )
			{
				pthread_t trd_tcp_connection;
				MM_BREAK_IF( pthread_create( &trd_tcp_connection , NULL , thread_tcp_connection_proc , pb ) != PTHREAD_CREATE_OK , errGeneral , 0 , "thread creation failed" );
			}
		}
		else if ( _g->appcfg._general_config->c.c.atht == justIncoming )
		{
			if ( tmp_try_to_connect_udp_port )
			{
				pthread_t trd_udp_connection;
				MM_BREAK_IF( pthread_create( &trd_udp_connection , NULL , thread_udp_connection_proc , pb ) != PTHREAD_CREATE_OK , errGeneral , 0 , "thread creation failed" );
			}
		}
		
		//SYS_ALIVE_CHECK();

	} while ( 1 );


	//// delete mask
	for ( int i = 0 ; i < pthread->pb->pb_trds_masks_count ; i++ )
	{
		if ( pthread->pb->pb_trds_masks[ i ] && pthread->pb->pb_trds[ i ].alc_thread->trd_id == tid )
		{
			pthread->pb->pb_trds_masks[ i ] = 0;
			break;
		}
	}

	//SYS_ALIVE_CHECK();
	
	BEGIN_RET
		case 3: {}
		case 2: {}
		case 1:
		{
			//_close_socket( &src_pb->tcp_sockfd );
			_g->stat.round_zero_set.syscal_err_count++;
		}
	M_V_END_RET

	return NULL;
}

#endif

#ifndef apply_config

void apply_new_protocol_bridge_config( struct App_Data * _g , struct protocol_bridge * pb , struct protocol_bridge_cfg * new_pcfg )
{
	INIT_BREAKABLE_FXN();

	if ( !new_pcfg->m.m.maintained.enable )
	{
		for ( int i = 0 ; i < pb->pb_trds_masks_count ; i++ )
		{
			if ( pb->pb_trds_masks[ i ] )
			{
				pb->pb_trds->alc_thread->do_close_thread = 1;
			}
		}
		return;
	}

	// make extra space for new one
	if ( pb->pb_trds_masks_count < PARALLELISM_COUNT )
	{
		int old_thread_count = pb->pb_trds_masks_count;
		int diff_count = ( PARALLELISM_COUNT - pb->pb_trds_masks_count );
		int new_thread_count = old_thread_count + diff_count;

		M_BREAK_IF( ( pb->pb_trds_masks = REALLOC( pb->pb_trds_masks , new_thread_count * sizeof( int ) ) ) == REALLOC_ERR , errMemoryLow , 0 );
		MEMSET_ZERO( pb->pb_trds_masks + old_thread_count , int , diff_count );

		M_BREAK_IF( ( pb->pb_trds = REALLOC( pb->pb_trds , new_thread_count * sizeof( struct protocol_bridge_thread_holder ) ) ) == REALLOC_ERR , errMemoryLow , 0 );
		MEMSET_ZERO( pb->pb_trds + old_thread_count , struct protocol_bridge_thread_holder , diff_count );

		pb->pb_trds_masks_count = new_thread_count;
	}

	// create extra needed thread and socket for each of them
	int valid_thread_count = 0;
	for ( int i = 0 ; i < pb->pb_trds_masks_count ; i++ )
	{
		if ( pb->pb_trds_masks[ i ] ) valid_thread_count++;
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
			for ( int i = 0 ; i < pb->pb_trds_masks_count ; i++ )
			{
				if ( !pb->pb_trds_masks[ i ] )
				{
					// 2. set mask
					pb->pb_trds_masks[ i ] = 1; // order is matter

					DAC( pb->pb_trds[ i ].alc_thread );
					M_BREAK_IF( ( pb->pb_trds[ i ].alc_thread = NEW( struct protocol_bridge_thread ) ) == NEW_ERR , errMemoryLow , 0 );
					MEMSET_ZERO( pb->pb_trds[ i ].alc_thread , struct protocol_bridge_thread , 1 );
					pb->pb_trds[ i ].alc_thread->pb = pb;

					// 3. create thread also
					MM_BREAK_IF( pthread_create( &pb->pb_trds[ i ].alc_thread->trd_id , NULL , protocol_bridge_runner , ( void * )pb ) != PTHREAD_CREATE_OK , errGeneral , 0 , "thread creation failed" );
					
					// 4. etc
					if ( IF_VERBOSE_MODE_CONDITION() )
					{
						_ECHO( "create thread %lu" , pb->pb_trds[ i ].alc_thread->trd_id );
					}
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
			for ( int i = pb->pb_trds_masks_count - 1 ; i >= 0 ; i-- ) // az yah hazf kon
			{
				if ( pb->pb_trds_masks[ i ] && pb->pb_trds[ i ].alc_thread->do_close_thread == 0 )
				{
					pb->pb_trds[ i ].alc_thread->do_close_thread = 1;

					//if ( pthread_cancel( pwave->pb_trds[ i ].trd_id ) == 0 )
					//{
					//	pwave->pb_trds[ i ].alc_thread->trd_id = 0;
					//	_close_socket( &pwave->pb_trds[ i ].socket_id );
						
						extra_count--;
					//}
					break;
				}
			}
		}
	}

	
	// when we arrive at this point we sure that somethings is changed
	memcpy( &pb->apcfg , new_pcfg , sizeof( struct protocol_bridge_cfg ) );
	new_pcfg->m.m.temp_data.pcfg_changed = 0;
	// Set every threads that their state must change
	for ( int i = 0 ; i < pb->pb_trds_masks_count ; i++ )
	{
		if ( pb->pb_trds_masks[ i ] )
		{
			pb->pb_trds[ i ].alc_thread->base_config_change_applied = 1;
		}
	}

	// TODO . complete reverse on error

	BEGIN_RET
		case 3: {}
		case 2: {}
		case 1:
		{
			_g->stat.round_zero_set.syscal_err_count++;
		}
	M_V_END_RET
}

void stop_protocol_bridge( struct App_Data * _g , struct protocol_bridge * pb )
{
	//INIT_BREAKABLE_FXN();

	//if ( IF_VERBOSE_MODE_CONDITION() )
	//{
	//	_ECHO( "stop_wave" );
	//}

	pb->apcfg.m.m.maintained.enable = 0;
	pb->apcfg.m.m.temp_data.pcfg_changed = 1;
	apply_new_protocol_bridge_config( _g , pb , &pb->apcfg );
	// TODO
}

void apply_protocol_bridge_new_cfg_changes( struct App_Data * _g , struct protocol_bridge_cfg * prev_pcfg , struct protocol_bridge_cfg * new_pcfg )
{
	//INIT_BREAKABLE_FXN();

	//if ( IF_VERBOSE_MODE_CONDITION() )
	//{
	//	_ECHO( "apply_wave_new_cfg_changes" );
	//}

	for ( int i = 0 ; i < _g->bridges.pb_holders_masks_count ; i++ )
	{
		if ( _g->bridges.pb_holders_masks[ i ] )
		{
			if ( memcmp( &_g->bridges.pb_holders[ i ].alc_pb->apcfg.m.m.id , &prev_pcfg->m.m.id , sizeof( struct protocol_bridge_cfg_id ) ) == 0 )
			{
				apply_new_protocol_bridge_config( _g , _g->bridges.pb_holders[ i ].alc_pb , new_pcfg );
			}
		}
	}
}

void remove_protocol_bridge( struct App_Data * _g , struct protocol_bridge_cfg * pcfg )
{
	//INIT_BREAKABLE_FXN();

	//if ( IF_VERBOSE_MODE_CONDITION() )
	//{
	//	_ECHO( "remove_wave" );
	//}

	for ( int i = 0 ; i < _g->bridges.pb_holders_masks_count ; i++ )
	{
		if ( _g->bridges.pb_holders_masks[ i ] )
		{
			if ( memcmp( &_g->bridges.pb_holders[ i ].alc_pb->apcfg.m.m.id , &pcfg->m.m.id , sizeof( struct protocol_bridge_cfg_id ) ) == 0 )
			{
				stop_protocol_bridge( _g , _g->bridges.pb_holders[ i ].alc_pb );
				_g->bridges.pb_holders_masks[ i ] = 0;
				SYS_ALIVE_CHECK();
				DAC( _g->bridges.pb_holders[ i ].alc_pb );
			}
		}
	}
}

#define PREALLOCAION_SIZE 10

void add_new_protocol_bridge( struct App_Data * _g , struct protocol_bridge_cfg * new_pcfg )
{
	INIT_BREAKABLE_FXN();

	//if ( IF_VERBOSE_MODE_CONDITION() )
	//{
	//	_ECHO( "add_new_wave" );
	//}

	int new_pcfg_placement_index = -1;
	while ( new_pcfg_placement_index < 0 ) // try to find one place for new wave
	{
		for ( int i = 0 ; i < _g->bridges.pb_holders_masks_count ; i++ )
		{
			if ( !_g->bridges.pb_holders_masks[ i ] )
			{
				new_pcfg_placement_index = i;
				break;
			}
		}
		if ( new_pcfg_placement_index < 0 )
		{
			int old_pb_holders_masks_count = _g->bridges.pb_holders_masks_count;
			int new_pb_holders_masks_count = old_pb_holders_masks_count + PREALLOCAION_SIZE;

			M_BREAK_IF( ( _g->bridges.pb_holders_masks = REALLOC( _g->bridges.pb_holders_masks , new_pb_holders_masks_count * sizeof( int ) ) ) == REALLOC_ERR , errMemoryLow , 2 );
			MEMSET_ZERO( _g->bridges.pb_holders_masks + old_pb_holders_masks_count , int , PREALLOCAION_SIZE );

			M_BREAK_IF( ( _g->bridges.pb_holders = REALLOC( _g->bridges.pb_holders , new_pb_holders_masks_count * sizeof( struct protocol_bridge_holder ) ) ) == REALLOC_ERR , errMemoryLow , 1 );
			MEMSET_ZERO( _g->bridges.pb_holders + old_pb_holders_masks_count , struct protocol_bridge_holder , PREALLOCAION_SIZE );

			_g->bridges.pb_holders_masks_count = new_pb_holders_masks_count;
		}
	}

	ASSERT( _g->bridges.pb_holders[ new_pcfg_placement_index ].alc_pb == NULL );
	M_BREAK_IF( ( _g->bridges.pb_holders[ new_pcfg_placement_index ].alc_pb = NEW( struct protocol_bridge ) ) == NEW_ERR , errMemoryLow , 0 );
	MEMSET_ZERO( _g->bridges.pb_holders[ new_pcfg_placement_index ].alc_pb , struct protocol_bridge , 1 );
	////SYS_ALIVE_CHECK();
	_g->bridges.pb_holders_masks[ new_pcfg_placement_index ] = 1;
	memcpy( &_g->bridges.pb_holders[ new_pcfg_placement_index ].alc_pb->apcfg , new_pcfg , sizeof( struct protocol_bridge_cfg ) );

	apply_protocol_bridge_new_cfg_changes( _g , new_pcfg , new_pcfg );

	BEGIN_RET
		case 3: DAC( _g->bridges.pb_holders );
		case 2: DAC( _g->bridges.pb_holders_masks );
		case 1: _g->stat.round_zero_set.syscal_err_count++;
	M_V_END_RET
} // TODO . return value

#endif

#ifndef section_load_config

#define CONFIG_ROOT_PATH "/home/my_projects/home-config/protocol_Bridge"

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
		case 1: _g->stat.round_zero_set.syscal_err_count++;
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

	typed( json_element ) el_Protocol_Bridge_config;

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
			struct protocol_bridge_cfg * pProtocol_Bridges = NULL;
			size_t Protocol_Bridges_count = 0;
			{
				const char * Protocol_Bridge_config_file_content = read_file( CONFIG_ROOT_PATH "/Protocol_Bridge_config.txt" , NULL );
				MM_BREAK_IF( !Protocol_Bridge_config_file_content , errGeneral , 0 , "cannot open config file" );
				
				result( json_element ) rs_Protocol_Bridge_config = json_parse( Protocol_Bridge_config_file_content );
				free( ( void * )Protocol_Bridge_config_file_content );
				MM_BREAK_IF( catch_error( &rs_Protocol_Bridge_config , "Protocol_Bridge_config" ) , errGeneral , 0 , "cannot parse config file" );
				el_Protocol_Bridge_config = result_unwrap( json_element )( &rs_Protocol_Bridge_config );

				/*configurations*/
				if ( _g->appcfg._ver->Major >= 1 ) // first version of config file structure
				{
					result( json_element ) re_configurations = json_object_find( el_Protocol_Bridge_config.value.as_object , "configurations" );
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
					CFG_ELEM_STR( thread_handler_type );
					
					CFG_ELEM_I( synchronization_min_wait );
					CFG_ELEM_I( synchronization_max_roundup );
					CFG_ELEM_I( show_line_hit );
					CFG_ELEM_I( retry_unexpected_wait_for_sock );
					CFG_ELEM_I( number_in_short_form );
					
					
					
					
#undef CFG_ELEM_I
#undef CFG_ELEM_STR
				}

				/*Protocol_Bridges*/
				{
					result( json_element ) re_Protocol_Bridges = json_object_find( el_Protocol_Bridge_config.value.as_object , "Protocol_Bridges" );
					MM_BREAK_IF( catch_error( &re_Protocol_Bridges , "Protocol_Bridges" ) , errGeneral , 0 , "Protocol_Bridges" );
					typed( json_element ) el_Protocol_Bridges = result_unwrap( json_element )( &re_Protocol_Bridges );

					MM_BREAK_IF( ( Protocol_Bridges_count = el_Protocol_Bridges.value.as_object->count ) < 1 , errGeneral , 0 , "Protocol_Bridges must be not zero" );

					M_BREAK_IF( !( pProtocol_Bridges = NEWBUF( struct protocol_bridge_cfg , el_Protocol_Bridges.value.as_object->count ) ) , errMemoryLow , 0 );
					MEMSET_ZERO( pProtocol_Bridges , struct protocol_bridge_cfg , el_Protocol_Bridges.value.as_object->count );
					

					for ( int i = 0 ; i < el_Protocol_Bridges.value.as_object->count ; i++ )
					{
						((struct protocol_bridge_cfg_0 *)(pProtocol_Bridges + i))->temp_data._g = ( void * )_g;

						char output_Protocol_Bridge_name[ 32 ];
						memset( output_Protocol_Bridge_name , 0 , sizeof( output_Protocol_Bridge_name ) );
						sprintf( output_Protocol_Bridge_name , "bridge%d" , i + 1 );

						result( json_element ) re_output_Protocol_Bridge = json_object_find( el_Protocol_Bridges.value.as_object , output_Protocol_Bridge_name );
						M_BREAK_IF( catch_error( &re_output_Protocol_Bridge , output_Protocol_Bridge_name ) , errGeneral , 0 );
						typed( json_element ) el_output_Protocol_Bridge = result_unwrap( json_element )( &re_output_Protocol_Bridge );

						#define CFG_ELEM_STR( name ) \
							result( json_element ) re_##name = json_object_find( el_output_Protocol_Bridge.value.as_object , #name );\
							M_BREAK_IF( catch_error( &re_##name , #name ) , errGeneral , 0 );\
							typed( json_element ) el_##name = result_unwrap( json_element )( &re_##name );\
							strcpy(((struct protocol_bridge_cfg_0 *)(pProtocol_Bridges + i))->name , el_##name.value.as_string );

						#define CFG_ID_ELEM_STR( name ) \
							result( json_element ) re_##name = json_object_find( el_output_Protocol_Bridge.value.as_object , #name );\
							M_BREAK_IF( catch_error( &re_##name , #name ) , errGeneral , 0 );\
							typed( json_element ) el_##name = result_unwrap( json_element )( &re_##name );\
							strcpy(((struct protocol_bridge_cfg_0 *)(pProtocol_Bridges + i))->id.name , el_##name.value.as_string );

						#define CFG_ELEM_I_maintained( name ) \
							result( json_element ) re_##name = json_object_find( el_output_Protocol_Bridge.value.as_object , #name );\
							M_BREAK_IF( catch_error( &re_##name , #name ) , errGeneral , 0 );\
							typed( json_element ) el_##name = result_unwrap( json_element )( &re_##name );\
							((struct protocol_bridge_cfg_0 *)(pProtocol_Bridges + i))->maintained.name = (int)el_##name.value.as_number.value.as_long;

						#define CFG_ELEM_I_momentary( name ) \
							result( json_element ) re_##name = json_object_find( el_output_Protocol_Bridge.value.as_object , #name );\
							M_BREAK_IF( catch_error( &re_##name , #name ) , errGeneral , 0 );\
							typed( json_element ) el_##name = result_unwrap( json_element )( &re_##name );\
							((struct protocol_bridge_cfg_0 *)(pProtocol_Bridges + i))->momentary.name = (int)el_##name.value.as_number.value.as_long;

						#define CFG_ID_ELEM_I( name ) \
							result( json_element ) re_##name = json_object_find( el_output_Protocol_Bridge.value.as_object , #name );\
							M_BREAK_IF( catch_error( &re_##name , #name ) , errGeneral , 0 );\
							typed( json_element ) el_##name = result_unwrap( json_element )( &re_##name );\
							((struct protocol_bridge_cfg_0 *)(pProtocol_Bridges + i))->id.name = (int)el_##name.value.as_number.value.as_long;

						strcpy( ( ( struct protocol_bridge_cfg_0 * )( pProtocol_Bridges + i ) )->id.protocol_bridge_name , output_Protocol_Bridge_name );
						CFG_ID_ELEM_STR( UDP_origin_ip );
						CFG_ID_ELEM_I( UDP_origin_port );
						CFG_ID_ELEM_STR( UDP_origin_interface );

						CFG_ID_ELEM_STR( TCP_destination_ip );
						CFG_ID_ELEM_I( TCP_destination_port );
						CFG_ID_ELEM_STR( TCP_destination_interface );

						CFG_ELEM_I_maintained( enable );
						CFG_ELEM_I_momentary( reset_connections );
						


						#undef CFG_ID_ELEM_I
						#undef CFG_ELEM_I
						#undef CFG_ID_ELEM_STR
						#undef CFG_ELEM_STR
					}

					json_free( &el_Protocol_Bridges );
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
					_g->appcfg._general_config_changed |= !!strcmp( _g->appcfg._general_config->c.c.thread_handler_type , _g->appcfg._prev_general_config->c.c.thread_handler_type );

					_g->appcfg._general_config_changed |= !( _g->appcfg._general_config->c.c.synchronization_min_wait == _g->appcfg._prev_general_config->c.c.synchronization_min_wait );
					_g->appcfg._general_config_changed |= !( _g->appcfg._general_config->c.c.synchronization_max_roundup == _g->appcfg._prev_general_config->c.c.synchronization_max_roundup );
					_g->appcfg._general_config_changed |= !( _g->appcfg._general_config->c.c.show_line_hit == _g->appcfg._prev_general_config->c.c.show_line_hit );
					_g->appcfg._general_config_changed |= !( _g->appcfg._general_config->c.c.retry_unexpected_wait_for_sock == _g->appcfg._prev_general_config->c.c.retry_unexpected_wait_for_sock );
					_g->appcfg._general_config_changed |= !( _g->appcfg._general_config->c.c.number_in_short_form == _g->appcfg._prev_general_config->c.c.number_in_short_form );
					
					
					
				}
			}
			if ( strcmp( _g->appcfg._general_config->c.c.thread_handler_type , "buttleneck" ) == 0 )
			{
				_g->appcfg._general_config->c.c.atht = buttleneck;
			}
			else if ( strcmp( _g->appcfg._general_config->c.c.thread_handler_type , "bidirection" ) == 0 )
			{
				_g->appcfg._general_config->c.c.atht = bidirection;
			}
			else if ( strcmp( _g->appcfg._general_config->c.c.thread_handler_type , "justIncoming" ) == 0 )
			{
				_g->appcfg._general_config->c.c.atht = justIncoming;
			}

			if ( _g->appcfg._general_config_changed )
			{
				if ( IF_VERBOSE_MODE_CONDITION() )
				{
					_ECHO( "general config changed" );
				}
			}

			if ( _g->appcfg._pprotocol_bridge_psvcfg )
			{
				DAC( _g->appcfg._pprev_protocol_bridge_psvcfg );
				_g->appcfg._prev_protocol_bridge_psvcfg_count = 0;
				_g->appcfg._pprev_protocol_bridge_psvcfg = _g->appcfg._pprotocol_bridge_psvcfg;
				_g->appcfg._prev_protocol_bridge_psvcfg_count = _g->appcfg._protocol_bridge_psvcfg_count;
			}
			_g->appcfg._pprotocol_bridge_psvcfg = pProtocol_Bridges;
			_g->appcfg._protocol_bridge_psvcfg_count = Protocol_Bridges_count;
			pProtocol_Bridges = NULL; // to not delete intentionally
			Protocol_Bridges_count = 0;

			if ( _g->appcfg._pprev_protocol_bridge_psvcfg == NULL && _g->appcfg._pprotocol_bridge_psvcfg )
			{
				// all new ones
				_g->appcfg._psvcfg_changed = 1; // ham koli set mishavad change rokh dad
				for ( int i = 0 ; i < _g->appcfg._protocol_bridge_psvcfg_count ; i++ )
				{
					_g->appcfg._pprotocol_bridge_psvcfg[ i ].m.m.temp_data.pcfg_changed = 1; // ham joz e set mishavad
				}
			}
			else if ( _g->appcfg._pprev_protocol_bridge_psvcfg && _g->appcfg._pprotocol_bridge_psvcfg )
			{
				// from old perspective
				for ( int i = 0 ; i < _g->appcfg._prev_protocol_bridge_psvcfg_count ; i++ )
				{
					int prev_exist = 0;
					for ( int j = 0 ; j < _g->appcfg._protocol_bridge_psvcfg_count ; j++ )
					{
						if ( memcmp( &_g->appcfg._pprev_protocol_bridge_psvcfg[ i ].m.m.id , &_g->appcfg._pprotocol_bridge_psvcfg[ j ].m.m.id , sizeof( struct protocol_bridge_cfg_id ) ) == 0 )
						{
							prev_exist = 1;
							if ( memcmp( &_g->appcfg._pprev_protocol_bridge_psvcfg[ i ].m.m.maintained , &_g->appcfg._pprotocol_bridge_psvcfg[ j ].m.m.maintained , sizeof( struct protocol_bridge_maintained_parameter ) ) )
							{
								_g->appcfg._psvcfg_changed = 1;
								_g->appcfg._pprotocol_bridge_psvcfg[ j ].m.m.temp_data.pcfg_changed = 1;
							}
							break;
						}
					}
					if ( !prev_exist )
					{
						_g->appcfg._psvcfg_changed = 1;
						_g->appcfg._pprev_protocol_bridge_psvcfg[ i ].m.m.temp_data.pcfg_changed = 1;
						break;
					}
				}
				// from new perspective
				for ( int j = 0 ; j < _g->appcfg._protocol_bridge_psvcfg_count ; j++ )
				{
					int new_exist = 0;
					for ( int i = 0 ; i < _g->appcfg._prev_protocol_bridge_psvcfg_count ; i++ )
					{
						if ( memcmp( &_g->appcfg._pprev_protocol_bridge_psvcfg[ i ].m.m.id , &_g->appcfg._pprotocol_bridge_psvcfg[ j ].m.m.id , sizeof( struct protocol_bridge_cfg_id ) ) == 0 )
						{
							new_exist = 1;
							if ( memcmp( &_g->appcfg._pprev_protocol_bridge_psvcfg[ i ].m.m.maintained , &_g->appcfg._pprotocol_bridge_psvcfg[ j ].m.m.maintained , sizeof( struct protocol_bridge_maintained_parameter ) ) )
							{
								_g->appcfg._psvcfg_changed = 1;
								_g->appcfg._pprotocol_bridge_psvcfg[ j ].m.m.temp_data.pcfg_changed = 1;
							}
							break;
						}
					}
					if ( !new_exist )
					{
						_g->appcfg._psvcfg_changed = 1;
						_g->appcfg._pprotocol_bridge_psvcfg[ j ].m.m.temp_data.pcfg_changed = 1;
						break;
					}
				}
			}

			_g->appcfg._version_changed = 0;

			if ( _g->appcfg._psvcfg_changed && IF_VERBOSE_MODE_CONDITION() )
			{
				_ECHO( "protocol_bridge config changed" );
			}
		}
		sleep( 2 );
	}

	BEGIN_RET
		case 1: { break; }
	M_V_END_RET

	return NULL;
}

// TODO . aware of concurrency in config read and act on it
void * protocol_bridge_manager( void * app_data )
{
	INIT_BREAKABLE_FXN();
	struct App_Data * _g = ( struct App_Data * )app_data;

	while ( !_g->appcfg._protocol_bridge_psvcfg_count ) // load after any config loaded
	{
		if ( CLOSE_APP_VAR() ) break;
		sleep( 1 );
	}

	// TODO . change behavior by fresh config
	if ( _g->appcfg._general_config->c.c.atht == buttleneck )
	{
		_g->bridges.bottleneck_thread = NEW( struct bridges_bottleneck_thread );
		MEMSET_ZERO( _g->bridges.bottleneck_thread , struct bridges_bottleneck_thread , 1 );
		pthread_mutex_init( &_g->bridges.thread_base.creation_thread_race_cond , NULL );
		pthread_mutex_init( &_g->bridges.thread_base.start_working_race_cond , NULL );
	}
	else if ( _g->appcfg._general_config->c.c.atht == bidirection )
	{
		_g->bridges.bidirection_thread = NEW( struct bridges_bidirection_thread );
		memset( &_g->bridges.bidirection_thread->mem , 0 , sizeof( struct bridges_bidirection_thread_zero_init_memory ) );
		queue_init( &_g->bridges.bidirection_thread->queue );
		pthread_mutex_init( &_g->bridges.thread_base.start_working_race_cond , NULL );
	}
	else if ( _g->appcfg._general_config->c.c.atht == justIncoming )
	{
		_g->bridges.justIncoming_thread = NEW( struct bridges_justIncoming_thread );
		MEMSET_ZERO( _g->bridges.justIncoming_thread , struct bridges_justIncoming_thread , 1 );
		pthread_mutex_init( &_g->bridges.thread_base.creation_thread_race_cond , NULL );
		pthread_mutex_init( &_g->bridges.thread_base.start_working_race_cond , NULL );
	}

	while ( 1 )
	{
		if ( CLOSE_APP_VAR() )
		{
			for ( int j = 0 ; j < _g->appcfg._protocol_bridge_psvcfg_count ; j++ )
			{
				_g->appcfg._pprotocol_bridge_psvcfg[ j ].m.m.maintained.enable = 0;
				apply_protocol_bridge_new_cfg_changes( _g , &_g->appcfg._pprotocol_bridge_psvcfg[ j ] , &_g->appcfg._pprotocol_bridge_psvcfg[ j ] );
			}
			break;
		}
		if ( _g->appcfg._general_config_changed )
		{
			_g->appcfg._general_config_changed = 0; // for now . TODO later
		}

		if ( _g->appcfg._psvcfg_changed )
		{
			for ( int i = 0 ; i < _g->appcfg._prev_protocol_bridge_psvcfg_count ; i++ )
			{
				int exist = 0;
				for ( int j = 0 ; j < _g->appcfg._protocol_bridge_psvcfg_count ; j++ )
				{
					if ( exist ) break;
					if ( memcmp( &_g->appcfg._pprev_protocol_bridge_psvcfg[ i ].m.m.id , &_g->appcfg._pprotocol_bridge_psvcfg[ j ].m.m.id , sizeof( struct protocol_bridge_cfg_id ) ) == 0 && _g->appcfg._pprotocol_bridge_psvcfg[ j ].m.m.temp_data.pcfg_changed )
					{
						// existed cfg changed
						apply_protocol_bridge_new_cfg_changes( _g , &_g->appcfg._pprev_protocol_bridge_psvcfg[ i ] , &_g->appcfg._pprotocol_bridge_psvcfg[ j ] );
						exist = 1;
					}
				}
				if ( !exist )
				{
					// remove removed one
					remove_protocol_bridge( _g , &_g->appcfg._pprev_protocol_bridge_psvcfg[ i ] );
				}
			}
			for ( int i = 0 ; i < _g->appcfg._protocol_bridge_psvcfg_count ; i++ )
			{
				int exist = 0;
				for ( int j = 0 ; j < _g->appcfg._prev_protocol_bridge_psvcfg_count ; j++ )
				{
					if ( memcmp( &_g->appcfg._pprotocol_bridge_psvcfg[ i ].m.m.id , &_g->appcfg._pprev_protocol_bridge_psvcfg[ j ].m.m.id , sizeof( struct protocol_bridge_cfg_id ) ) == 0 )
					{
						exist = 1;
						break;
					}
				}
				if ( !exist )
				{
					// start new cfg
					add_new_protocol_bridge( _g , &_g->appcfg._pprotocol_bridge_psvcfg[ i ] );
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

struct App_Data * __g;

void reset_nonuse_stat()
{
	struct App_Data * _g = __g;
	memset( &_g->stat.round_zero_set , 0 , sizeof( _g->stat.round_zero_set ) );

	circbuf_reset( &_g->stat.round_init_set.udp_stat_5_sec_count );
	circbuf_reset( &_g->stat.round_init_set.udp_stat_10_sec_count );
	circbuf_reset( &_g->stat.round_init_set.udp_stat_40_sec_count );
	circbuf_reset( &_g->stat.round_init_set.udp_stat_120_sec_count );

	circbuf_reset( &_g->stat.round_init_set.udp_stat_5_sec_bytes );
	circbuf_reset( &_g->stat.round_init_set.udp_stat_10_sec_bytes );
	circbuf_reset( &_g->stat.round_init_set.udp_stat_40_sec_bytes );
	circbuf_reset( &_g->stat.round_init_set.udp_stat_120_sec_bytes );

	circbuf_reset( &_g->stat.round_init_set.tcp_stat_5_sec_count );
	circbuf_reset( &_g->stat.round_init_set.tcp_stat_10_sec_count );
	circbuf_reset( &_g->stat.round_init_set.tcp_stat_40_sec_count );
	circbuf_reset( &_g->stat.round_init_set.tcp_stat_120_sec_count );

	circbuf_reset( &_g->stat.round_init_set.tcp_stat_5_sec_bytes );
	circbuf_reset( &_g->stat.round_init_set.tcp_stat_10_sec_bytes );
	circbuf_reset( &_g->stat.round_init_set.tcp_stat_40_sec_bytes );
	circbuf_reset( &_g->stat.round_init_set.tcp_stat_120_sec_bytes );
}

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
	reset_nonuse_stat();

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
		wprintw( _g->stat.input_win , "cmd(quit,sync,rst): " );
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
		else if ( stricmp( _g->stat.input_buffer , "rst" ) == 0 )
		{
			boutput_command = 0;
			reset_nonuse_stat();
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
	snprintf( buf2 , sizeof( buf2 ) , "PB Metric-%s" , buf );

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
	
	setlocale(LC_NUMERIC, "en_US.UTF-8");

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

	//////////////
	
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "UDP conn" );
	snprintf( buf , sizeof( buf ) , "%d Σ%d" , MAIN_STAT().udp_connection_count , MAIN_STAT().total_retry_udp_connection_count );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );

	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "TCP conn" );
	snprintf( buf , sizeof( buf ) , "%d Σ%d" , MAIN_STAT().tcp_connection_count , MAIN_STAT().total_retry_tcp_connection_count );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );

	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "inp failure" );
	snprintf( buf , sizeof( buf ) , "v%d Σv%d" , MAIN_STAT().round_zero_set.continuously_unsuccessful_receive_error , MAIN_STAT().round_zero_set.total_unsuccessful_receive_error );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );

	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "out failure" );
	snprintf( buf , sizeof( buf ) , "^%d Σ^%d" , MAIN_STAT().round_zero_set.continuously_unsuccessful_send_error , MAIN_STAT().round_zero_set.total_unsuccessful_send_error );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );

	mvwprintw( MAIN_WIN , y++ , start_x , header_border );

	if ( _g->appcfg._general_config && _g->appcfg._general_config->c.c.show_line_hit )
	{
		for ( int i = 0 ; i < FXN_HIT_COUNT ; i++ )
		{
			if ( __FXN_HIT[ i ][0] > 0 )
			{
				mvwprintw( MAIN_WIN , y , start_x , "|" );
				snprintf( buf , sizeof( buf ) , "%d" , i ); // line
				print_cell( MAIN_WIN , y , start_x + 1 , cell_w , buf ); // line
				snprintf( buf , sizeof( buf ) , "%d " , __FXN_HIT[ i ][0] ); // hit count

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

	///////////

	#define _FORMAT_SHRTFRM( baaf , NPP , val , decimal_precision , unit ) ( NUMBER_IN_SHORT_FORM() ? \
		format_pps( baaf , sizeof(baaf) , val , decimal_precision , unit ) :\
		__snprintf( baaf , sizeof(baaf) , "%llu" , val ) )

	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "syscal_err" );
	snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof(buf2) , MAIN_STAT().round_zero_set.syscal_err_count , 2 , "" ) );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );

	if ( _g->appcfg._general_config && _g->appcfg._general_config->c.c.atht == bidirection && _g->bridges.bidirection_thread )
	{
		mvwprintw( MAIN_WIN , y , start_x , "|" );
		print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "qu cnt " );
		snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , ( ubigint )_g->bridges.bidirection_thread->queue.count , 2 , "" ) );
		mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
		print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
		mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );
	}

	mvwprintw( MAIN_WIN , y++ , start_x , header_border );

	format_elapsed_time_with_millis( _g->stat.round_zero_set.t_begin , _g->stat.round_zero_set.t_end , buf2 , sizeof( buf2 ) );
	//
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "itr duration" );
	snprintf( buf , sizeof( buf ) , "%s" , buf2 );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );

	//
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "udp get" );
	snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , MAIN_STAT().round_zero_set.udp.total_udp_get_count , 2 , "" ) );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );
	//
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "udp get byte" );
	snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , MAIN_STAT().round_zero_set.udp.total_udp_get_byte , 2 , "B" ) );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );

	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "contnu unsuces slct udp" );
	snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , MAIN_STAT().round_zero_set.udp.continuously_unsuccessful_select_on_open_port_count , 2 , "" ) );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );

	#ifndef time_frame

	// 5 sec
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "5s udp pps" );
	snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , ( ubigint )circbuf_mean_all( &MAIN_STAT().round_init_set.udp_stat_5_sec_count ) , 4 , "" ) );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );
	//
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "5s udp bps" );
	snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , ( ubigint )circbuf_mean_all( &MAIN_STAT().round_init_set.udp_stat_5_sec_bytes ) , 4 , "B" ) );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );

	// 10 sec
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "10s udp pps" );
	snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , ( ubigint )circbuf_mean_all( &MAIN_STAT().round_init_set.udp_stat_10_sec_count ) , 4 , "" ) );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );
	//
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "10s udp bps" );
	snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , ( ubigint )circbuf_mean_all( &MAIN_STAT().round_init_set.udp_stat_10_sec_bytes ) , 4 , "B" ) );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );

	// 40 sec
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "40s udp pps" );
	snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , ( ubigint )circbuf_mean_all( &MAIN_STAT().round_init_set.udp_stat_40_sec_count ) , 4 , "" ) );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );
	//
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "40s udp bps" );
	snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , ( ubigint )circbuf_mean_all( &MAIN_STAT().round_init_set.udp_stat_40_sec_bytes ) , 4 , "B" ) );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );

	// 120 sec
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "120s udp pps" );
	snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , ( ubigint )circbuf_mean_all( &MAIN_STAT().round_init_set.udp_stat_120_sec_count ) , 4 , "" ) );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );
	//
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "120s udp bps" );
	snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , ( ubigint )circbuf_mean_all( &MAIN_STAT().round_init_set.udp_stat_120_sec_bytes ) , 4 , "B" ) );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );

	mvwprintw( MAIN_WIN , y++ , start_x , header_border );

	#endif


	#ifndef tcp

	//
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "tcp put" );
	snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , MAIN_STAT().round_zero_set.tcp.total_tcp_put_count , 2 , "" ) );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );
	//
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "tcp put byte" );
	snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , MAIN_STAT().round_zero_set.tcp.total_tcp_put_byte , 2 , "B" ) );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );


	// 5 sec
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "5s tcp pps" );
	snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , ( ubigint )circbuf_mean_all( &MAIN_STAT().round_init_set.tcp_stat_5_sec_count ) , 4 , "" ) );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );
	//
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "5s tcp bps" );
	snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , ( ubigint )circbuf_mean_all( &MAIN_STAT().round_init_set.tcp_stat_5_sec_bytes ) , 4 , "B" ) );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );


	//// 10 sec
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "10s tcp pps" );
	snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , ( ubigint )circbuf_mean_all( &MAIN_STAT().round_init_set.tcp_stat_10_sec_count ) , 4 , "" ) );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );
	//
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "10s tcp bps" );
	snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , ( ubigint )circbuf_mean_all( &MAIN_STAT().round_init_set.tcp_stat_10_sec_bytes ) , 4 , "B" ) );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );

	//// 40 sec
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "40s tcp pps" );
	snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , ( ubigint )circbuf_mean_all( &MAIN_STAT().round_init_set.tcp_stat_40_sec_count ) , 4 , "" ) );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );
	//
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "40s tcp bps" );
	snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , ( ubigint )circbuf_mean_all( &MAIN_STAT().round_init_set.tcp_stat_40_sec_bytes ) , 4 , "B" ) );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );

	//// 120 sec
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "120s tcp pps" );
	snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , ( ubigint )circbuf_mean_all( &MAIN_STAT().round_init_set.tcp_stat_120_sec_count ) , 4 , "" ) );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );
	//
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "120s tcp bps" );
	snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , ( ubigint )circbuf_mean_all( &MAIN_STAT().round_init_set.tcp_stat_120_sec_bytes ) , 4 , "B" ) );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );

	#endif


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
	if ( pipe( _g->stat.pipefds ) == -1 ) {}

	// Redirect stdout
	//fflush( stdout );
	dup2( _g->stat.pipefds[ 1 ] , fileno( stderr ) );

	pthread_t tid_stdout_bypass;
	pthread_create( &tid_stdout_bypass , NULL , stdout_bypass_thread , ( void * )_g );
}

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

	circbuf_init( &_g->stat.round_init_set.udp_stat_5_sec_count , 5 );
	circbuf_init( &_g->stat.round_init_set.udp_stat_10_sec_count , 10 );
	circbuf_init( &_g->stat.round_init_set.udp_stat_40_sec_count , 40 );
	circbuf_init( &_g->stat.round_init_set.udp_stat_120_sec_count , 120 );

	circbuf_init( &_g->stat.round_init_set.udp_stat_5_sec_bytes , 5 );
	circbuf_init( &_g->stat.round_init_set.udp_stat_10_sec_bytes , 10 );
	circbuf_init( &_g->stat.round_init_set.udp_stat_40_sec_bytes , 40 );
	circbuf_init( &_g->stat.round_init_set.udp_stat_120_sec_bytes , 120 );

	circbuf_init( &_g->stat.round_init_set.tcp_stat_5_sec_count , 5 );
	circbuf_init( &_g->stat.round_init_set.tcp_stat_10_sec_count , 10 );
	circbuf_init( &_g->stat.round_init_set.tcp_stat_40_sec_count , 40 );
	circbuf_init( &_g->stat.round_init_set.tcp_stat_120_sec_count , 120 );
										   
	circbuf_init( &_g->stat.round_init_set.tcp_stat_5_sec_bytes , 5 );
	circbuf_init( &_g->stat.round_init_set.tcp_stat_10_sec_bytes , 10 );
	circbuf_init( &_g->stat.round_init_set.tcp_stat_40_sec_bytes , 40 );
	circbuf_init( &_g->stat.round_init_set.tcp_stat_120_sec_bytes , 120 );
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

	pthread_t trd_protocol_bridge_manager;
	MM_BREAK_IF( pthread_create( &trd_protocol_bridge_manager , NULL , protocol_bridge_manager , ( void * )_g ) != PTHREAD_CREATE_OK , errGeneral , 0 , "Failed to create thread" );

	M_BREAK_IF( pthread_join( trd_protocol_bridge_manager , NULL ) != PTHREAD_JOIN_OK , errGeneral , 0 );

	SYS_ALIVE_CHECK();

	return 0;
	BEGIN_RET
		case 2: {}
		case 1: {}
		case 0: __g->stat.round_zero_set.syscal_err_count++;
	M_V_END_RET
	return 1;
}

#endif
