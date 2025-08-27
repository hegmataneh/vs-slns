#define Uses_accept_thresholds
#define Uses_thrd_sleep
#define Uses_errno
#define Uses_AB_tcp
#define Uses_fcntl
#define Uses_socket
#define Uses_strncpy
#define Uses_read
#define Uses_TWD
#define Uses_pthread_t
#define Uses_helper
#define Uses_statistics

//#define DIRECT_ECHO_BUF _g->stat.last_command // just before include dep
#include <Protocol_Bridge.dep>

extern G * __g;

_THREAD_FXN void_p stdout_bypass_thread( void_p pdata )
{
	static TWD twd = { 0 };
	if ( twd.threadId == 0 )
	{
		twd.threadId = pthread_self();
		twd.cal = stdout_bypass_thread; // self function address
		twd.callback_arg = pdata;
	}
	if ( pdata == NULL )
	{
		return ( void_p )&twd;
	}
	G * _g = ( G * )pdata;

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

void init_bypass_stdout( G * _g )
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
	pthread_create( &tid_stdout_bypass , NULL , stdout_bypass_thread , ( void_p )_g );
}

_STRONG_ATTR void M_showMsg( LPCSTR msg )
{
	if ( __g ) strcpy( __g->stat.last_command , msg );
}

void init( G * _g )
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

	////pthread_mutex_init( &_g->sync.mutex , NULL );
	////pthread_cond_init( &_g->sync.cond , NULL );

	cbuf_m_init( &_g->stat.round_init_set.udp_stat_5_sec_count , 5 );
	cbuf_m_init( &_g->stat.round_init_set.udp_stat_10_sec_count , 10 );
	cbuf_m_init( &_g->stat.round_init_set.udp_stat_40_sec_count , 40 );
	cbuf_m_init( &_g->stat.round_init_set.udp_stat_120_sec_count , 120 );

	cbuf_m_init( &_g->stat.round_init_set.udp_stat_5_sec_bytes , 5 );
	cbuf_m_init( &_g->stat.round_init_set.udp_stat_10_sec_bytes , 10 );
	cbuf_m_init( &_g->stat.round_init_set.udp_stat_40_sec_bytes , 40 );
	cbuf_m_init( &_g->stat.round_init_set.udp_stat_120_sec_bytes , 120 );

	cbuf_m_init( &_g->stat.round_init_set.tcp_stat_5_sec_count , 5 );
	cbuf_m_init( &_g->stat.round_init_set.tcp_stat_10_sec_count , 10 );
	cbuf_m_init( &_g->stat.round_init_set.tcp_stat_40_sec_count , 40 );
	cbuf_m_init( &_g->stat.round_init_set.tcp_stat_120_sec_count , 120 );

	cbuf_m_init( &_g->stat.round_init_set.tcp_stat_5_sec_bytes , 5 );
	cbuf_m_init( &_g->stat.round_init_set.tcp_stat_10_sec_bytes , 10 );
	cbuf_m_init( &_g->stat.round_init_set.tcp_stat_40_sec_bytes , 40 );
	cbuf_m_init( &_g->stat.round_init_set.tcp_stat_120_sec_bytes , 120 );

	distributor_init( &_g->stat.thresholds , 5 );
	distributor_subscribe( &_g->stat.thresholds , 0 , SUB_INT_DOUBLE , SUB_FXN( accept_thresholds ) , _g );
}

_THREAD_FXN void_p sync_thread( void_p pdata ) // pause app until moment other app exist
{
	//INIT_BREAKABLE_FXN();
	static TWD twd = { 0 };
	if ( twd.threadId == 0 )
	{
		twd.threadId = pthread_self();
		twd.cal = sync_thread; // self function address
		twd.callback_arg = pdata;
	}
	if ( pdata == NULL )
	{
		return ( void_p )&twd;
	}

	//G * _g = ( G * )pdata;
	////if ( _g->sync.lock_in_progress ) return NULL;

	//struct timespec now , next_round_time;
	//clock_gettime( CLOCK_REALTIME , &now );

	////pthread_mutex_lock( &_g->sync.mutex );
	//round_up_to_next_interval( &now , _g->appcfg.g_cfg->c.c.synchronization_min_wait , _g->appcfg.g_cfg->c.c.synchronization_max_roundup , &next_round_time );
	////_g->sync.lock_in_progress = 1;
	////pthread_mutex_unlock( &_g->sync.mutex );

	////next_round_time.tv_sec += 5; // bridge start later

	//format_clock_time( &next_round_time , __custom_message , sizeof( __custom_message ) );
	//_DIRECT_ECHO( "Will wake at %s" , __custom_message );

	//////pthread_mutex_lock(&_g->sync.mutex);
	////// First thread sets the global target time
	//////if (next_round_time.tv_sec == 0) {
	//////	next_round_time = target;
	//////}
	//////pthread_mutex_unlock(&_g->sync.mutex);

	//// Sleep until that global target time
	//clock_nanosleep( CLOCK_REALTIME , TIMER_ABSTIME , &next_round_time , NULL );
	//reset_nonuse_stat();

	////pthread_mutex_lock( &_g->sync.mutex );
	////_g->sync.lock_in_progress = 0;
	////_g->sync.reset_static_after_lock = 1;
	////pthread_cond_signal( &_g->sync.cond );
	//////pthread_cond_broadcast( &_g->sync.cond );
	////pthread_mutex_unlock( &_g->sync.mutex );

	////clock_gettime( CLOCK_REALTIME , &now );
	////_DIRECT_ECHO( "waked up" );
	//_DIRECT_ECHO( "" );

	return NULL;
}

_THREAD_FXN void_p input_thread( void_p src_g )
{
	INIT_BREAKABLE_FXN();
	static TWD twd = { 0 };
	if ( twd.threadId == 0 )
	{
		twd.threadId = pthread_self();
		twd.cal = input_thread; // self function address
		twd.callback_arg = src_g;
	}
	if ( src_g == NULL )
	{
		return ( void_p )&twd;
	}

	G * _g = ( G * )src_g;
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

		if ( iSTR_SAME( _g->stat.input_buffer , "quit" ) )
		{
			_g->cmd.quit_app = 1;
			break;
		}
		else if ( iSTR_SAME( _g->stat.input_buffer , "sync" ) )
		{
			boutput_command = 0;
			pthread_t thread;
			if ( pthread_create( &thread , NULL , sync_thread , src_g ) != 0 )
			{
				_ECHO( "pthread_create" );
			}
			//break;
		}
		else if ( iSTR_SAME( _g->stat.input_buffer , "rst" ) )
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
	BEGIN_SMPL
	M_V_END_RET
	return NULL;
}

// TODO . close connection after change in config

/// <summary>
/// this function stablish every udp that need to be stablished
/// this callback call multitimes
/// </summary>
_THREAD_FXN void_p connect_udps_proc( void_p src_pb )
{
	INIT_BREAKABLE_FXN();
	////static TWD twd = { 0 };
	////if ( twd.threadId == 0 )
	////{
	////	twd.threadId = pthread_self();
	////	twd.cal = connect_udps_proc; // self function address
	////	twd.callback_arg = src_pb;
	////}
	////if ( src_pb == NULL )
	////{
	////	return ( void_p )&twd;
	////}

	AB * pb = ( AB * )src_pb;
	G * _g = ( G * )pb->cpy_cfg.m.m.temp_data._g;

	for ( int iudp = 0 ; iudp < pb->udps_count ; iudp++ )
	{
		if ( pb->udps[ iudp ].udp_connection_established )
		{
			_close_socket( &pb->udps[ iudp ].udp_sockfd );
			pb->udps[ iudp ].udp_connection_established = 0;
			_g->stat.udp_connection_count--;
		}

		MM_BREAK_IF( ( pb->udps[ iudp ].udp_sockfd = socket( AF_INET , SOCK_DGRAM , 0 ) ) == FXN_SOCKET_ERR , errGeneral , 1 , "create sock error" );

		int optval = 1;
		//socklen_t optlen = sizeof( optval );
		//MM_BREAK_IF( getsockopt( pb->udps[ iudp ].udp_sockfd , SOL_SOCKET , SO_REUSEADDR , &optval , &optlen ) < 0 , errGeneral , 1 , "SO_REUSEADDR" );
		MM_BREAK_IF( setsockopt( pb->udps[ iudp ].udp_sockfd , SOL_SOCKET , SO_REUSEADDR , &optval , sizeof( optval ) ) < 0 , errGeneral , 1 , "SO_REUSEADDR" );

		int flags = fcntl( pb->udps[ iudp ].udp_sockfd, F_GETFL );
		fcntl( pb->udps[ iudp ].udp_sockfd , F_SETFL , flags | O_NONBLOCK );

		// TODO . make it protocol independent
		struct sockaddr_in server_addr;
		memset( &server_addr , 0 , sizeof( server_addr ) );
		server_addr.sin_family = AF_INET; // IPv4
		
		int port = 0;
		M_BREAK_IF( string_to_int( pb->udps[ iudp ].__udp_cfg_pak->data.UDP_origin_ports , &port ) , errGeneral , 0 );
		ASSERT( port > 0 );
		server_addr.sin_port = htons( ( uint16_t )port ); // Convert port to network byte order
		if ( iSTR_SAME( pb->udps[ iudp ].__udp_cfg_pak->data.UDP_origin_ip , "INADDR_ANY" ) )
		{
			server_addr.sin_addr.s_addr = INADDR_ANY; // Or use INADDR_ANY to bind to all available interfaces:
		}
		else
		{
			server_addr.sin_addr.s_addr = inet_addr( pb->udps[ iudp ].__udp_cfg_pak->data.UDP_origin_ip ); // Specify the IP address to bind to
		}

		MM_BREAK_IF( bind( pb->udps[ iudp ].udp_sockfd , ( const struct sockaddr * )&server_addr , sizeof( server_addr ) ) == FXN_BIND_ERR , errGeneral , 1 , "bind sock error" );
		pb->udps[ iudp ].udp_connection_established = 1;

		_g->stat.udp_connection_count++;
		_g->stat.total_retry_udp_connection_count++;

		//_g->bridges.under_listen_udp_sockets_group_changed++; // if any udp socket change then fdset must be reinitialized
	}

	BEGIN_RET
	case 3: ;
	case 2: ;
	case 1:	_g->stat.round_zero_set.syscal_err_count++;
	M_V_END_RET
	return NULL; // Threads can return a value, but this example returns NULL
}

//int _connect_tcp( AB * pb )
//{
//	INIT_BREAKABLE_FXN();
//	G * _g = ( G * )pb->cpy_cfg.m.m.temp_data._g;
//
//	while ( 1 )
//	{
//
//		// try to create TCP socket
//		MM_BREAK_IF( ( pb->tcp_sockfd = socket( AF_INET , SOCK_STREAM , 0 ) ) == FXN_SOCKET_ERR , errGeneral , 0 , "create sock error" );
//
//		struct sockaddr_in tcp_addr;
//		tcp_addr.sin_family = AF_INET;
//		tcp_addr.sin_port = htons( ( uint16_t )pb->cpy_cfg.m.m.id.TCP_destination_port );
//		MM_BREAK_IF( inet_pton( AF_INET , pb->cpy_cfg.m.m.id.TCP_destination_ip , &tcp_addr.sin_addr ) <= 0 , errGeneral , 1 , "inet_pton sock error" );
//
//		if ( connect( pb->tcp_sockfd , ( struct sockaddr * )&tcp_addr , sizeof( tcp_addr ) ) == -1 )
//		{
//			if ( errno == ECONNREFUSED || errno == ETIMEDOUT )
//			{
//				_close_socket( &pb->tcp_sockfd );
//				slep( 2 ); // sec
//				continue;
//			}
//
//			MM_BREAK_IF( 1 , errGeneral , 1 , "Error connecting to TCP server" );
//		}
//		else
//		{
//			pb->tcp_connection_established = 1;
//			_g->stat.tcp_connection_count++;
//			_g->stat.total_retry_tcp_connection_count++;
//
//			//int flag = 1;
//			//setsockopt( pb->tcp_sockfd , IPPROTO_TCP , TCP_NODELAY , ( char * )&flag , sizeof( int ) );
//
//			pthread_mutex_lock( &_g->bridges.thread_base.start_working_race_cond );
//			switch ( _g->appcfg.g_cfg->c.c.atht )
//			{
//				case buttleneck:
//				case bidirection:
//				{
//					if ( pb->udp_connection_established && pb->tcp_connection_established )
//					{
//						_g->bridges.thread_base.start_working = 1;
//					}
//					break;
//				}
//				default:
//				case justIncoming:
//				{
//					if ( pb->udp_connection_established )
//					{
//						_g->bridges.thread_base.start_working = 1;
//					}
//					break;
//				}
//			}
//			pthread_mutex_unlock( &_g->bridges.thread_base.start_working_race_cond );
//
//
//			return 0;
//		}
//	}
//	
//	BEGIN_RET
//		case 3: ;
//		case 2: ;
//		case 1:
//		{
//			_close_socket( &pb->tcp_sockfd );
//			_g->stat.round_zero_set.syscal_err_count++;
//		}
//	M_V_END_RET
//	return -1;
//}

status connect_one_tcp( AB_tcp * tcp )
{
	INIT_BREAKABLE_FXN();
	G * _g = tcp->owner_pb->cpy_cfg.m.m.temp_data._g;

	while ( 1 )
	{
		// try to create TCP socket
		MM_BREAK_IF( ( tcp->tcp_sockfd = socket( AF_INET , SOCK_STREAM , 0 ) ) == FXN_SOCKET_ERR , errGeneral , 0 , "create sock error" );

		struct sockaddr_in tcp_addr;
		tcp_addr.sin_family = AF_INET;

		int port = 0;
		M_BREAK_IF( string_to_int( tcp->__tcp_cfg_pak->data.TCP_destination_ports , &port ) , errGeneral , 1 );
		ASSERT( port > 0 );

		tcp_addr.sin_port = htons( ( uint16_t )port );
		MM_BREAK_IF( inet_pton( AF_INET , tcp->__tcp_cfg_pak->data.TCP_destination_ip , &tcp_addr.sin_addr ) <= 0 , errGeneral , 1 , "inet_pton sock error" );

		if ( connect( tcp->tcp_sockfd , ( struct sockaddr * )&tcp_addr , sizeof( tcp_addr ) ) == -1 )
		{
			if ( errno == ECONNREFUSED || errno == ETIMEDOUT )
			{
				_close_socket( &tcp->tcp_sockfd );
				mng_basic_thread_sleep( _g , NORMAL_PRIORITY_THREAD );
				continue;
			}

			MM_BREAK_IF( 1 , errGeneral , 1 , "Error connecting to TCP server" );
		}
		else
		{
			tcp->tcp_connection_established = 1;
			_g->stat.tcp_connection_count++;
			_g->stat.total_retry_tcp_connection_count++;

			//int flag = 1;
			//setsockopt( pb->tcp_sockfd , IPPROTO_TCP , TCP_NODELAY , ( char * )&flag , sizeof( int ) );

			return errOK;
		}
	}

	BEGIN_RET
	case 3: ;
	case 2: ;
	case 1:
	{
		_close_socket( &tcp->tcp_sockfd );
		_g->stat.round_zero_set.syscal_err_count++;
	}
	M_V_END_RET
	return errGeneral;
}

_THREAD_FXN void_p thread_tcp_connection_proc( void_p src_pb )
{
	INIT_BREAKABLE_FXN();
	////static TWD twd = { 0 };
	////if ( twd.threadId == 0 )
	////{
	////	twd.threadId = pthread_self();
	////	twd.cal = thread_tcp_connection_proc; // self function address
	////	twd.callback_arg = src_pb;
	////}
	////if ( src_pb == NULL )
	////{
	////	return ( void_p )&twd;
	////}

	AB * pb = ( AB * )src_pb;
	//G * _g = ( G * )pb->cpy_cfg.m.m.temp_data._g;

	while( 1 )
	{
		int all_tcp_connected = 0;
		for ( int itcp = 0 ; itcp < pb->tcps_count ; itcp++ )
		{
			if ( pb->tcps[ itcp ].tcp_connection_established )
			{
				all_tcp_connected++;
				continue;
			}

			connect_one_tcp( &pb->tcps[ itcp ] );
		}
		if ( all_tcp_connected == pb->tcps_count )
		{
			break;
		}
	}

	BREAK_OK(0); // to just ignore gcc warning

	BEGIN_SMPL
	M_V_END_RET
	return NULL; // Threads can return a value, but this example returns NULL
}


/// <summary>
/// this thread must be as simple as possible and without exception and any error because it is resposible for checking other and making them up
/// </summary>
/// <param name="src_g"></param>
/// <returns></returns>
_THREAD_FXN void_p watchdog_executer( void_p src_g )
{
	INIT_BREAKABLE_FXN();
	////static TWD twd = { 0 };
	////if ( twd.threadId == 0 )
	////{
	////	twd.threadId = pthread_self();
	////	twd.cal = watchdog_executer; // self function address
	////	twd.callback_arg = src_g;
	////}
	////if ( src_g == NULL )
	////{
	////	return ( void_p )&twd;
	////}

	G * _g = ( G * )src_g;

	while ( 1 )
	{
		if ( CLOSE_APP_VAR() ) break;

		for ( int imask = 0 ; imask < _g->bridges.ABhs_masks_count ; imask++ )
		{
			if ( _g->bridges.ABhs_masks[ imask ] )
			{
				if ( !_g->bridges.ABs[ imask ].single_AB->trd.base.bridg_prerequisite_stabled )
				{
					if ( iSTR_SAME( _g->bridges.ABs[ imask ].single_AB->cpy_cfg.m.m.id.thread_handler_act , "kernel_default_stack_udp_counter" ) )
					{
						if ( iSTR_SAME( _g->bridges.ABs[ imask ].single_AB->cpy_cfg.m.m.id.out_type , "one" ) )
						{
							if ( _g->bridges.ABs[ imask ].single_AB->udps_count > 0 && _g->bridges.ABs[ imask ].single_AB->udps->udp_connection_established )
							{
								_g->bridges.ABs[ imask ].single_AB->trd.base.bridg_prerequisite_stabled = 1;
							}
						}
						else
						{
							ASSERT( 0 ); // implement on demand
						}
					}
					else if ( iSTR_SAME( _g->bridges.ABs[ imask ].single_AB->cpy_cfg.m.m.id.thread_handler_act , "pcap_udp_counter" ) )
					{
					}
					else if ( iSTR_SAME( _g->bridges.ABs[ imask ].single_AB->cpy_cfg.m.m.id.thread_handler_act , "one2one_pcap2kernelDefaultStack_S&F" ) )
					{
						if ( iSTR_SAME( _g->bridges.ABs[ imask ].single_AB->cpy_cfg.m.m.id.out_type , "one2one" ) )
						{
							if
							(
								_g->bridges.ABs[ imask ].single_AB->udps_count > 0 && _g->bridges.ABs[ imask ].single_AB->udps->udp_connection_established &&
								_g->bridges.ABs[ imask ].single_AB->tcps_count > 0 && _g->bridges.ABs[ imask ].single_AB->tcps->tcp_connection_established
							)
							{
								_g->bridges.ABs[ imask ].single_AB->trd.base.bridg_prerequisite_stabled = 1;
							}
						}
						else
						{
							ASSERT( 0 ); // implement on demand
						}
					}
					else if ( iSTR_SAME( _g->bridges.ABs[ imask ].single_AB->cpy_cfg.m.m.id.thread_handler_act , "one2many_pcap2kernelDefaultStack_S&F_Mix_RR_Replicate" ) )
					{
						if ( iSTR_SAME( _g->bridges.ABs[ imask ].single_AB->cpy_cfg.m.m.id.out_type , "one2many" ) )
						{
							ASSERT( _g->bridges.ABs[ imask ].single_AB->cpy_cfg.m.m.maintained.in_count == 1 );
							ASSERT( _g->bridges.ABs[ imask ].single_AB->cpy_cfg.m.m.maintained.out_count > 1 );

							int prerequisite_stablished = 1;
							prerequisite_stablished &= Booleanize( _g->bridges.ABs[ imask ].single_AB->udps_count );
							prerequisite_stablished &= Booleanize( _g->bridges.ABs[ imask ].single_AB->tcps_count );
							if ( _g->bridges.ABs[ imask ].single_AB->udps_count )
							{
								prerequisite_stablished &= Booleanize( _g->bridges.ABs[ imask ].single_AB->udps->udp_connection_established );
							}
							for ( int itcp = 0 ; itcp < _g->bridges.ABs[ imask ].single_AB->tcps_count ; itcp++ )
							{
								prerequisite_stablished &= Booleanize( _g->bridges.ABs[ imask ].single_AB->tcps[ itcp ].tcp_connection_established );
							}
							_g->bridges.ABs[ imask ].single_AB->trd.base.bridg_prerequisite_stabled = prerequisite_stablished;
						}
						else
						{
							ASSERT( 0 ); // implement on demand
						}
					}
					else
					{
						ASSERT( 0 ); // implement on demand
					}
				}
			}
		}

		mng_basic_thread_sleep( _g , NORMAL_PRIORITY_THREAD );
	}

	BEGIN_SMPL
	M_V_END_RET
	return NULL; // Threads can return a value, but this time returns NULL
}

void mng_basic_thread_sleep( G * _g , int priority )
{
	struct timespec ts;
	MEMSET_ZERO( &ts , 1 );
	switch ( priority )
	{
		case LOW_PRIORITY_THREAD:
		{
			ts.tv_nsec = LOW_THREAD_DEFAULT_DELAY_NANOSEC();
			break;
		}
		case NORMAL_PRIORITY_THREAD:
		{
			ts.tv_nsec = NORMAL_THREAD_DEFAULT_DELAY_NANOSEC();
			break;
		}
		default:
		case HI_PRIORITY_THREAD:
		{
			ts.tv_nsec = HI_THREAD_DEFAULT_DELAY_NANOSEC();
			break;
		}
	}
	ts.tv_sec = ts.tv_nsec / 1000000000L;   // seconds
	ts.tv_nsec = ts.tv_nsec % 1000000000L;  // nanoseconds
	thrd_sleep( &ts , NULL );
}

