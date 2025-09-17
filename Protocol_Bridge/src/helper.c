#define Uses_packet_mngr
#define Uses__ECHO
#define Uses_INIT_BREAKABLE_FXN
#define Uses_MEMSET
#define Uses_strings_ar
#define Uses_signal
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
#include <Protocol_Bridge.dep>

extern G * _g;

_THREAD_FXN void_p stdout_bypass_thread( pass_p src_g )
{
	static TWD twd = { 0 };
	if ( twd.threadId == 0 )
	{
		twd.threadId = pthread_self();
		twd.cal = stdout_bypass_thread; // self function address
		twd.callback_arg = src_g;
	}
	if ( src_g == NULL )
	{
		return ( void_p )&twd;
	}
	G * _g = ( G * )src_g;

	fd_set readfds;
	char buffer[ DEFAULT_BUF_SIZE ];
	while ( 1 )
	{
		FD_ZERO( &readfds );
		FD_SET( _g->stat.pipefds[ 0 ] , &readfds );

		int ready = select( _g->stat.pipefds[ 0 ] + 1 , &readfds , NULL , NULL , NULL );
		if ( ready > 0 && FD_ISSET( _g->stat.pipefds[ 0 ] , &readfds ) )
		{
			int n = read( _g->stat.pipefds[ 0 ] , buffer , DEFAULT_BUF_SIZE - 1 );
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
	//MEMSET( pipefd , 0 , sizeof( pipefd ) );

	// Make pipe
	if ( pipe( _g->stat.pipefds ) == -1 )
	{
	}

	// Redirect stdout
	//fflush( stdout );
	dup2( _g->stat.pipefds[ 1 ] , fileno( stderr ) );

	pthread_t tid_stdout_bypass;
	pthread_create( &tid_stdout_bypass , NULL , stdout_bypass_thread , ( pass_p )_g );
}

_STRONG_ATTR void M_showMsg( LPCSTR msg )
{
	if ( _g ) strcpy( _g->stat.last_command , msg );
}

_CALLBACK_FXN void pb_err_dist( pass_p src_pb , LPCSTR msg )
{
	AB * pb = ( AB * )src_pb;
	G * _g = pb->cpy_cfg.m.m.temp_data._g;
	pb->stat.round_zero_set.syscal_err_count++;
	distributor_publish_str( &_g->distrbtor.ground_err_dist , msg , ( pass_p )_g );
}

_CALLBACK_FXN void ground_err_dist( pass_p src_g , LPCSTR msg )
{
	//G * _g = ( G * )src_g;
	// TODO
}

_CALLBACK_FXN void udp_connected( pass_p src_pb , int v )
{
	AB * pb = ( AB * )src_pb;
	pb->stat.round_zero_set.udp_connection_count++;
	pb->stat.round_zero_set.total_retry_udp_connection_count++;
}

_CALLBACK_FXN void udp_disconnected( pass_p src_pb , int v )
{
	AB * pb = ( AB * )src_pb;
	pb->stat.round_zero_set.udp_connection_count--;
}
_CALLBACK_FXN void tcp_connected( pass_p src_AB_tcp , sockfd fd )
{
	AB_tcp * tcp = ( AB_tcp * )src_AB_tcp;
	// first try to send by file desc if succ then pointer must be valid
	dict_fst_put( &TO_G( tcp->owner_pb->cpy_cfg.m.m.temp_data._g )->hdls.map_tcp_socket , tcp->__tcp_cfg_pak->name , fd , ( void_p )tcp , NULL , NULL , NULL );
	tcp->owner_pb->stat.round_zero_set.tcp_connection_count++;
	tcp->owner_pb->stat.round_zero_set.total_retry_tcp_connection_count++;
}
_CALLBACK_FXN void tcp_disconnected( pass_p src_pb , int v )
{
	AB * pb = ( AB * )src_pb;
	pb->stat.round_zero_set.tcp_connection_count--;
}

void quit_interrupt( int sig )
{
	if ( sig ) ( void )sig; // TODO . what happeneed here
	distributor_publish_int( &_g->distrbtor.quit_interrupt_dist , sig , NULL );
	exit( 0 );
}

void pre_config_init( G * _g )
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

	//MEMSET_ZERO_O( &_g->handles );

	////pthread_mutex_init( &_g->sync.mutex , NULL );
	////pthread_cond_init( &_g->sync.cond , NULL );

	distributor_init( &_g->distrbtor.pb_err_dist , 1 ); // error counter anywhere occured in app
	distributor_init( &_g->distrbtor.ground_err_dist , 1 ); // error counter anywhere occured in app
	distributor_subscribe( &_g->distrbtor.pb_err_dist , SUB_STRING , SUB_FXN( pb_err_dist ) , NULL );
	distributor_subscribe( &_g->distrbtor.ground_err_dist , SUB_STRING , SUB_FXN( ground_err_dist ) , ( pass_p )_g );


	distributor_init( &_g->distrbtor.pb_udp_connected_dist , 1 );
	distributor_init( &_g->distrbtor.pb_udp_disconnected_dist , 1 );
	distributor_init( &_g->distrbtor.pb_tcp_connected_dist , 1 );
	distributor_init( &_g->distrbtor.pb_tcp_disconnected_dist , 1 );

	distributor_subscribe( &_g->distrbtor.pb_udp_connected_dist , SUB_INT , SUB_FXN( udp_connected ) , NULL );
	distributor_subscribe( &_g->distrbtor.pb_udp_disconnected_dist , SUB_INT , SUB_FXN( udp_disconnected ) , NULL );
	distributor_subscribe( &_g->distrbtor.pb_tcp_connected_dist , SUB_INT , SUB_FXN( tcp_connected ) , NULL );
	distributor_subscribe( &_g->distrbtor.pb_tcp_disconnected_dist , SUB_INT , SUB_FXN( tcp_disconnected ) , NULL );

	distributor_init( &_g->distrbtor.quit_interrupt_dist , 1 );

	signal( SIGINT , quit_interrupt );
	signal( SIGTERM , quit_interrupt );
	signal( SIGPIPE , quit_interrupt );

	//SIGSEGV
	//SIGFPE

	//cbuf_m_init( &_g->stat.round_init_set.udp_stat_5_sec_count , 5 );
	//cbuf_m_init( &_g->stat.round_init_set.udp_stat_10_sec_count , 10 );
	//cbuf_m_init( &_g->stat.round_init_set.udp_stat_40_sec_count , 40 );
	//cbuf_m_init( &_g->stat.round_init_set.udp_stat_120_sec_count , 120 );

	//cbuf_m_init( &_g->stat.round_init_set.udp_stat_5_sec_bytes , 5 );
	//cbuf_m_init( &_g->stat.round_init_set.udp_stat_10_sec_bytes , 10 );
	//cbuf_m_init( &_g->stat.round_init_set.udp_stat_40_sec_bytes , 40 );
	//cbuf_m_init( &_g->stat.round_init_set.udp_stat_120_sec_bytes , 120 );

	//cbuf_m_init( &_g->stat.round_init_set.tcp_stat_5_sec_count , 5 );
	//cbuf_m_init( &_g->stat.round_init_set.tcp_stat_10_sec_count , 10 );
	//cbuf_m_init( &_g->stat.round_init_set.tcp_stat_40_sec_count , 40 );
	//cbuf_m_init( &_g->stat.round_init_set.tcp_stat_120_sec_count , 120 );

	//cbuf_m_init( &_g->stat.round_init_set.tcp_stat_5_sec_bytes , 5 );
	//cbuf_m_init( &_g->stat.round_init_set.tcp_stat_10_sec_bytes , 10 );
	//cbuf_m_init( &_g->stat.round_init_set.tcp_stat_40_sec_bytes , 40 );
	//cbuf_m_init( &_g->stat.round_init_set.tcp_stat_120_sec_bytes , 120 );

	dict_fst_create( &_g->hdls.map_tcp_socket , 0 );
}

void post_config_init( G * _g )
{
	INIT_BREAKABLE_FXN();

	segmgr_init( &_g->bufs.aggr_inp_pkt , ( size_t )_g->appcfg.g_cfg->c.c.pkt_mgr_segment_capacity , ( size_t )_g->appcfg.g_cfg->c.c.pkt_mgr_offsets_capacity , True );

	MM_BREAK_IF( pthread_create( &_g->trds.trd_tcp_sender , NULL , process_filled_tcp_segment_proc , ( pass_p )_g ) != PTHREAD_CREATE_OK , errCreation , 0 , "Failed to create tcp_sender thread" );

	BEGIN_SMPL
	M_V_END_RET
}

_THREAD_FXN void_p sync_thread( pass_p src_g ) // pause app until moment other app exist
{
	//INIT_BREAKABLE_FXN();
	static TWD twd = { 0 };
	if ( twd.threadId == 0 )
	{
		twd.threadId = pthread_self();
		twd.cal = sync_thread; // self function address
		twd.callback_arg = src_g;
	}
	if ( src_g == NULL )
	{
		return ( void_p )&twd;
	}

	//G * _g = ( G * )src_g;
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

_THREAD_FXN void_p input_thread( pass_p src_g )
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
_THREAD_FXN void_p connect_udps_proc( pass_p src_pb )
{
	INIT_BREAKABLE_FXN();

	AB * pb = ( AB * )src_pb;
	G * _g = ( G * )pb->cpy_cfg.m.m.temp_data._g;

	for ( int iudp = 0 ; iudp < pb->udps_count ; iudp++ )
	{
		if ( pb->udps[ iudp ].udp_connection_established )
		{
			_close_socket( &pb->udps[ iudp ].udp_sockfd );
			pb->udps[ iudp ].udp_connection_established = 0;
			//_g->stat.udp_connection_count--;
			// TODO
		}

		MM_BREAK_IF( ( pb->udps[ iudp ].udp_sockfd = socket( AF_INET , SOCK_DGRAM , 0 ) ) == FXN_SOCKET_ERR , errSocket , 1 , "create sock error" );

		int optval = 1;
		//socklen_t optlen = sizeof( optval );
		//MM_BREAK_IF( getsockopt( pb->udps[ iudp ].udp_sockfd , SOL_SOCKET , SO_REUSEADDR , &optval , &optlen ) < 0 , errSocket , 1 , "SO_REUSEADDR" );
		MM_BREAK_IF( setsockopt( pb->udps[ iudp ].udp_sockfd , SOL_SOCKET , SO_REUSEADDR , &optval , sizeof( optval ) ) < 0 , errSocket , 1 , "SO_REUSEADDR" );

		int flags = fcntl( pb->udps[ iudp ].udp_sockfd, F_GETFL );
		fcntl( pb->udps[ iudp ].udp_sockfd , F_SETFL , flags | O_NONBLOCK );

		// TODO . make it protocol independent
		struct sockaddr_in server_addr;
		MEMSET( &server_addr , 0 , sizeof( server_addr ) );
		server_addr.sin_family = AF_INET; // IPv4
		
		int port = 0;
		M_BREAK_IF( string_to_int( pb->udps[ iudp ].__udp_cfg_pak->data.UDP_origin_ports , &port ) , errGeneral , 0 );
		WARNING( port > 0 );
		server_addr.sin_port = htons( ( uint16_t )port ); // Convert port to network byte order
		if ( iSTR_SAME( pb->udps[ iudp ].__udp_cfg_pak->data.UDP_origin_ip , "INADDR_ANY" ) )
		{
			server_addr.sin_addr.s_addr = INADDR_ANY; // Or use INADDR_ANY to bind to all available interfaces:
		}
		else
		{
			server_addr.sin_addr.s_addr = inet_addr( pb->udps[ iudp ].__udp_cfg_pak->data.UDP_origin_ip ); // Specify the IP address to bind to
		}

		MM_BREAK_IF( bind( pb->udps[ iudp ].udp_sockfd , ( const struct sockaddr * )&server_addr , sizeof( server_addr ) ) == FXN_BIND_ERR , errSocket , 1 , "bind sock error" );
		pb->udps[ iudp ].udp_connection_established = 1;

		distributor_publish_int( &_g->distrbtor.pb_udp_connected_dist , 0 , ( pass_p )pb );

		//_g->bridges.under_listen_udp_sockets_group_changed++; // if any udp socket change then fdset must be reinitialized
	}

	BEGIN_RET
	case 1:	DIST_ERR();
	M_V_END_RET
	return NULL; // Threads can return a value, but this example returns NULL
}

status connect_one_tcp( AB_tcp * tcp )
{
	INIT_BREAKABLE_FXN();
	AB * pb = tcp->owner_pb;
	G * _g = tcp->owner_pb->cpy_cfg.m.m.temp_data._g;

	while ( 1 )
	{
		// try to create TCP socket
		MM_BREAK_IF( ( tcp->tcp_sockfd = socket( AF_INET , SOCK_STREAM , 0 ) ) == FXN_SOCKET_ERR , errSocket , 0 , "create sock error" );

		struct sockaddr_in tcp_addr;
		tcp_addr.sin_family = AF_INET;

		int port = 0;
		M_BREAK_IF( string_to_int( tcp->__tcp_cfg_pak->data.TCP_destination_ports , &port ) , errGeneral , 1 );
		WARNING( port > 0 );

		tcp_addr.sin_port = htons( ( uint16_t )port );
		MM_BREAK_IF( inet_pton( AF_INET , tcp->__tcp_cfg_pak->data.TCP_destination_ip , &tcp_addr.sin_addr ) <= 0 , errSocket , 1 , "inet_pton sock error" );

		if ( connect( tcp->tcp_sockfd , ( struct sockaddr * )&tcp_addr , sizeof( tcp_addr ) ) == -1 )
		{
			if ( errno == ECONNREFUSED || errno == ETIMEDOUT )
			{
				_close_socket( &tcp->tcp_sockfd );
				mng_basic_thread_sleep( _g , NORMAL_PRIORITY_THREAD );
				continue;
			}

			MM_BREAK_IF( 1 , errSocket , 1 , "Error connecting to TCP server" );
		}
		else
		{
			tcp->tcp_connection_established = 1;
			distributor_publish_int( &_g->distrbtor.pb_tcp_connected_dist , tcp->tcp_sockfd , ( pass_p )tcp );
			return errOK;
		}
	}

	BEGIN_RET
	case 1:
	{
		_close_socket( &tcp->tcp_sockfd );
		DIST_ERR();
	}
	M_V_END_RET
	return errSocket;
}

_THREAD_FXN void_p thread_tcp_connection_proc( pass_p src_pb )
{
	INIT_BREAKABLE_FXN();

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
_THREAD_FXN void_p watchdog_executer( pass_p src_g )
{
	INIT_BREAKABLE_FXN();

	G * _g = ( G * )src_g;

	while ( 1 )
	{
		if ( CLOSE_APP_VAR() ) break;

		for ( int imask = 0 ; imask < _g->bridges.ABhs_masks_count ; imask++ )
		{
			if ( _g->bridges.ABhs_masks[ imask ] )
			{
				if
				(
					_g->bridges.ABs[ imask ].single_AB->cpy_cfg.m.m.temp_data.delayed_validation &&
					//_g->bridges.ABs[ imask ].single_AB->cpy_cfg.m.m.maintained.enable &&
					!_g->bridges.ABs[ imask ].single_AB->trd.base.bridg_prerequisite_stabled
				)
				{
					if ( iSTR_SAME( _g->bridges.ABs[ imask ].single_AB->cpy_cfg.m.m.id.thread_handler_act , "NetStack_udp_counter" ) )
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
							WARNING( 0 ); // implement on demand
						}
					}
					else if ( iSTR_SAME( _g->bridges.ABs[ imask ].single_AB->cpy_cfg.m.m.id.thread_handler_act , "pcap_udp_counter" ) )
					{
					}
					else if
					(
						iSTR_SAME( _g->bridges.ABs[ imask ].single_AB->cpy_cfg.m.m.id.thread_handler_act , "one2one_krnl2krnl_SF" ) ||
						iSTR_SAME( _g->bridges.ABs[ imask ].single_AB->cpy_cfg.m.m.id.thread_handler_act , "one2one_pcap2NetStack_SF" )
					)
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
							WARNING( 0 ); // implement on demand
						}
					}
					else if
					(
						iSTR_SAME( _g->bridges.ABs[ imask ].single_AB->cpy_cfg.m.m.id.thread_handler_act , "one2many_pcap2NetStack_SF" ) ||
						iSTR_SAME( _g->bridges.ABs[ imask ].single_AB->cpy_cfg.m.m.id.thread_handler_act , "many2one_pcap2kernelDefaultStack_S&F_serialize" )
					)
					{
						if
						(
							iSTR_SAME( _g->bridges.ABs[ imask ].single_AB->cpy_cfg.m.m.id.out_type , "one2many" ) ||
							iSTR_SAME( _g->bridges.ABs[ imask ].single_AB->cpy_cfg.m.m.id.out_type , "many2one" )
						)
						{
							WARNING( _g->bridges.ABs[ imask ].single_AB->cpy_cfg.m.m.maintained.in_count >= 1 );
							WARNING( _g->bridges.ABs[ imask ].single_AB->cpy_cfg.m.m.maintained.out_count >= 1 );

							int prerequisite_stablished = 1;
							prerequisite_stablished &= Booleanize( _g->bridges.ABs[ imask ].single_AB->udps_count );
							prerequisite_stablished &= Booleanize( _g->bridges.ABs[ imask ].single_AB->tcps_count );
							
							for ( int iudp = 0 ; iudp < _g->bridges.ABs[ imask ].single_AB->udps_count ; iudp++ )
							{
								prerequisite_stablished &= Booleanize( _g->bridges.ABs[ imask ].single_AB->udps[ iudp ].udp_connection_established );
							}
							for ( int itcp = 0 ; itcp < _g->bridges.ABs[ imask ].single_AB->tcps_count ; itcp++ )
							{
								prerequisite_stablished &= Booleanize( _g->bridges.ABs[ imask ].single_AB->tcps[ itcp ].tcp_connection_established );
							}
							_g->bridges.ABs[ imask ].single_AB->trd.base.bridg_prerequisite_stabled = prerequisite_stablished;
						}
						else
						{
							WARNING( 0 ); // implement on demand
						}
					}
					else
					{
						WARNING( 0 ); // implement on demand
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

/// <summary>
/// single point for thread wait
/// </summary>
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

/// <summary>
/// used in collect_strings_itr to pack vary strings
/// </summary>
LPCSTR itr_interfaces( const pass_p arr , size_t i )
{
	return ( ( AB_udp * )arr )[ i ].__udp_cfg_pak->data.UDP_origin_interface;
};

/// <summary>
///	get config and get list of necessary pcap filter
/// free interface_filter , port_filter by FREE_DOUBLE_PTR
/// </summary>
_REGULAR_FXN void compile_udps_config_for_pcap_filter
(
	_IN AB * abs
	, _RET_VAL_P int * clusterd_cnt /*each actual needed cnf*/
	, _NEW_OUT_P strings * interface_filter
	, _NEW_OUT_P strings * port_filter
)
{
	INIT_BREAKABLE_FXN();

	strings_ar distinct_interface;
	{
		*clusterd_cnt = 0;
		strings_ar interface_lst;
		{
			collect_strings_itr( ( void_p )abs->udps , ( size_t )abs->udps_count , itr_interfaces , &interface_lst );
		}
		M_BREAK_STAT( strs_distinct( &interface_lst , &distinct_interface ) , 0 );
		free_string_ar( &interface_lst );
	}
	// now we have interface count
	
	*clusterd_cnt = ( int )distinct_interface.size;
	if ( *clusterd_cnt < 1 ) return;
	M_MALLOC_AR( *interface_filter , *clusterd_cnt , 0 );
	MEMSET_ZERO( *interface_filter , *clusterd_cnt );
	M_MALLOC_AR( *port_filter , *clusterd_cnt , 0 );
	MEMSET_ZERO( *port_filter , *clusterd_cnt );

	for ( int iint = 0 ; iint < *clusterd_cnt ; iint++ )
	{
		M_BREAK_IF( !( *interface_filter[ iint ] = strdup( distinct_interface.strs[ iint ] ) ) , errMemoryLow , 0 );
	}
	free_string_ar( &distinct_interface );

	// not time to combine port

	for ( int iint = 0 ; iint < *clusterd_cnt ; iint++ )
	{
		// iterate through every out section to find interface and aggregate them
		strings_ar distinct_ports;
		{
			strings_ar ports_ar;
			init_string_ar( &ports_ar );
			for ( int iout = 0 ; iout < abs->udps_count ; iout++ )
			{
				if ( iSTR_SAME( abs->udps[ iout ].__udp_cfg_pak->data.UDP_origin_interface , *interface_filter[ iint ] ) )
				{
					M_BREAK_STAT( addTo_string_ar( &ports_ar , abs->udps[ iout ].__udp_cfg_pak->data.UDP_origin_ports ) , 0 );
				}
			}
			WARNING( ports_ar.size > 0 );
		
			M_BREAK_STAT( strs_distinct( &ports_ar , &distinct_ports ) , 0 );
			free_string_ar( &ports_ar );
		}

		LPSTR * port_filter_addr = ( *( LPSTR ** )port_filter ) + iint;
		M_MALLOC_AR( *port_filter_addr , 1024 /*default filter size*/ , 0 );
		MEMSET_ZERO( *port_filter_addr , 1024 );
		LPSTR prt_flt = *port_filter_addr;

		int n = 0;
		// Start with "udp and ("
		n += sprintf( prt_flt + n , "udp and (" );
		for ( int iprt = 0 ; iprt < distinct_ports.size ; iprt++ )
		{
			if ( iprt > 0 )
			{
				n += sprintf( prt_flt + n , " or " );
			}

			// TODO . later i should check config file that no duplicate port find in it
			for ( int islk = 0 ; islk < abs->udps_count ; islk++ )
			{
				if ( iSTR_SAME( abs->udps[ islk ].__udp_cfg_pak->data.UDP_origin_interface , *interface_filter[iint] )
					&& iSTR_SAME( abs->udps[ islk ].__udp_cfg_pak->data.UDP_origin_ports , distinct_ports.strs[iprt] ) )
				{
					if ( strchr( abs->udps[ islk ].__udp_cfg_pak->data.UDP_origin_ports , '-' ) != NULL )
					{
						// it's a range
						n += sprintf( prt_flt + n , "portrange %s" , abs->udps[ islk ].__udp_cfg_pak->data.UDP_origin_ports );
					}
					else
					{
						// it's a single port
						n += sprintf( prt_flt + n , "port %s" , abs->udps[ islk ].__udp_cfg_pak->data.UDP_origin_ports );
					}
					break;
				}
			}
		}

		// close parenthesis
		n += sprintf( prt_flt + n , ")" );
	}

	BEGIN_SMPL
	M_V_END_RET
}

