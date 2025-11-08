#define Uses_MARK_LINE
#define Uses_ddlck
#define Uses_warn
#define Uses_sleep
#define Uses_pthread_kill
#define Uses_format_elapsed_time_with_millis
#define Uses_format_clock_time
#define Uses_iSTR_SAME
#define Uses_WARNING
#define Uses_STR_SAME
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
#define Uses_globals
#define Uses_statistics
#include <Protocol_Bridge.dep>

_GLOBAL_VAR _EXTERN G * _g;

#ifndef DEBUG_section

_GLOBAL_VAR _STRONG_ATTR void M_showMsg( LPCSTR msg )
{
	#ifdef HAS_STATISTICSS
	if ( _g ) strcpy( _g->stat.nc_h.message_text , msg );
	#endif
}

_GLOBAL_VAR void _Breaked()
{
	int i = 1;
	i++;
}

#endif

#ifndef main_initializer

_PRIVATE_FXN _CALLBACK_FXN void cleanup_globals( pass_p src_g , long v )
{
	G * _g = ( G * )src_g;
	pthread_mutex_destroy(&_g->hdls.thread_close_mtx);   // init lock

	#ifdef HAS_STATISTICSS
	sub_destroy( &_g->distributors.throttling_refresh_stat );
	#endif
	sub_destroy( &_g->distributors.bcast_pre_cfg );
	sub_destroy( &_g->distributors.bcast_post_cfg );
	sub_destroy( &_g->distributors.bcast_thread_startup );
	sub_destroy( &_g->distributors.bcast_app_lvl_failure );
	sub_destroy( &_g->distributors.bcast_pb_lvl_failure );
	sub_destroy( &_g->distributors.bcast_pb_udp_connected );
	sub_destroy( &_g->distributors.bcast_pb_udp_disconnected );
	sub_destroy( &_g->distributors.bcast_pb_tcp_connected );
	sub_destroy( &_g->distributors.bcast_pb_tcp_disconnected );

	#ifdef HAS_STATISTICSS
	sub_destroy( &_g->distributors.init_static_table );
	#endif
	sub_destroy( &_g->distributors.bcast_program_stabled );
	//sub_destroy( &_g->distributors.bcast_quit );

	array_free( &_g->hdls.registered_thread );

#ifdef ENABLE_USE_INTERNAL_C_STATISTIC
	MARK_LINE();
#endif
}

_PRIVATE_FXN _CALLBACK_FXN void cleanup_threads( pass_p src_g , long v )
{
#ifdef ENABLE_USE_INTERNAL_C_STATISTIC
	MARK_LINE();
#endif

	G * _g = ( G * )src_g;
	size_t sz = array_get_count( &_g->hdls.registered_thread );
	pthread_t * ppthread_t = NULL;
	
	bool bcontinue = true;
	while( bcontinue )
	{
		bool one_thread_still = false;
		for ( size_t idx = 0 ; idx < sz ; idx++ )
		{
			if ( array_get_s( &_g->hdls.registered_thread , idx , ( void ** )&ppthread_t ) == errOK && *ppthread_t )
			{
				one_thread_still = true;
				//break;
				//time_t tbegin = time( NULL );
				//time_t tnow = tbegin;
				//int ret = 0;
				//while( ( ret = pthread_kill( *ppthread_t , 0 ) ) && ( ( tnow - tbegin ) < 10 ) ) // signal 0: no signal sent
				//{
				//	tnow = time( NULL );
				//	mng_basic_thread_sleep( _g , HI_PRIORITY_THREAD );
				//}
			}
		}
		if ( !one_thread_still )
		{
			bcontinue = false;
		}
	}

#ifdef ENABLE_USE_INTERNAL_C_STATISTIC
	MARK_LINE();
#endif
}

_CALLBACK_FXN void thread_registration( pass_p src_g , long src_pthread_t )
{
	G * _g = ( G * )src_g;
	pthread_mutex_lock( &_g->hdls.thread_close_mtx );
	array_add( &_g->hdls.registered_thread , ( void * )&src_pthread_t );
	pthread_mutex_unlock( &_g->hdls.thread_close_mtx );
	
	#ifdef HAS_STATISTICSS
	nnc_cell_triggered( _g->stat.nc_s_req.ov_thread_cnt_cell );
	#endif
}

_CALLBACK_FXN void bad_interrupt( int sig )
{
	( void )sig;
#ifdef ENABLE_VERBOSE_FAULT
	warn( "warn: %d" , sig );
#endif
}

_CALLBACK_FXN _PRIVATE_FXN void program_is_stabled_globals( void_p src_g )
{
	INIT_BREAKABLE_FXN();

	G * _g = ( G * )src_g;
	bool bstart_tcp_thread = false;
	for ( size_t ab_idx = 0 ; ab_idx < _g->bridges.ABs.count ; ab_idx++ )
	{
		AB * pb = NULL;
		if ( mms_array_get_s( &_g->bridges.ABs , ab_idx , ( void ** )&pb ) == errOK )
		{
			if ( pb->tcps_count )
			{
				bstart_tcp_thread = true;
				break;
			}
		}
	}
	if ( bstart_tcp_thread )
	{
		pthread_mutex_lock( &_g->bridges.tcps_trd.mtx );
		if ( !_g->bridges.tcps_trd.bcreated )
		{
			d_error = pthread_create( &_g->bridges.tcps_trd.trd_tcp_connection , NULL , thread_tcp_connection_proc , _g ) != PTHREAD_CREATE_OK ? errCreation : errOK;
			if ( d_error == errOK )
			{
				_g->bridges.tcps_trd.bcreated = true;
			}
		}
		pthread_mutex_unlock( &_g->bridges.tcps_trd.mtx );
	}
	M_BREAK_STAT( d_error , 1 );

	BEGIN_RET // TODO . complete reverse on error
	case 1:
	{
		DIST_APP_FAILURE();
	}
	M_V_END_RET
}

_CALLBACK_FXN _PRIVATE_FXN void pre_config_init_helper( void_p src_g )
{
	G * _g = ( G * )src_g;
	pthread_mutex_init(&_g->bridges.tcps_trd.mtx , NULL);
	distributor_subscribe_withOrder( &_g->distributors.bcast_program_stabled , SUB_VOID , SUB_FXN( program_is_stabled_globals ) , _g , tcp_thread_trigger );

	array_init( &_g->hdls.registered_thread , sizeof( pthread_t ) , 1 , GROW_STEP , 0 ); // keep started thread

	////pthread_mutex_init( &_g->sync.mutex , NULL );
	////pthread_cond_init( &_g->sync.cond , NULL );

	distributor_init_withLock( &_g->distributors.bcast_thread_startup , 1 ); // to trace thread activity
	distributor_subscribe( &_g->distributors.bcast_thread_startup , SUB_LONG , SUB_FXN( thread_registration ) , _g );

	distributor_subscribe_withOrder( &_g->distributors.bcast_quit , SUB_LONG , SUB_FXN( cleanup_threads ) , _g , clean_threads ); // wait for open threads

	distributor_subscribe_withOrder( &_g->distributors.bcast_quit , SUB_LONG , SUB_FXN( cleanup_globals ) , _g , clean_globals ); // place to release global variables

	#ifdef ENABLE_VERBOSE_FAULT
	signal( SIGPIPE , bad_interrupt );
	signal( SIGBUS , bad_interrupt );
	signal( SIGSEGV , bad_interrupt );
	signal( SIGFPE , bad_interrupt );
	#endif

	distributor_init( &_g->distributors.bcast_app_lvl_failure , 1 ); // error counter anywhere occured in app
	distributor_init( &_g->distributors.bcast_pb_lvl_failure , 1 ); // error counter anywhere occured in app
	distributor_subscribe( &_g->distributors.bcast_app_lvl_failure , SUB_STRING , SUB_FXN( app_err_dist ) , ( pass_p )_g );
	distributor_subscribe( &_g->distributors.bcast_pb_lvl_failure , SUB_STRING , SUB_FXN( pb_err_dist ) , NULL );

	distributor_init_withLock( &_g->distributors.bcast_pb_udp_connected , 1 );
	distributor_init_withLock( &_g->distributors.bcast_pb_udp_disconnected , 1 );
	distributor_init_withLock( &_g->distributors.bcast_pb_tcp_connected , 1 );
	distributor_init_withLock( &_g->distributors.bcast_pb_tcp_disconnected , 1 );

	distributor_subscribe( &_g->distributors.bcast_pb_udp_connected , SUB_LONG , SUB_FXN( udp_connected ) , NULL );
	distributor_subscribe( &_g->distributors.bcast_pb_udp_disconnected , SUB_LONG , SUB_FXN( udp_disconnected ) , NULL );
	distributor_subscribe( &_g->distributors.bcast_pb_tcp_connected , SUB_LONG , SUB_FXN( tcp_connected ) , NULL );
	distributor_subscribe( &_g->distributors.bcast_pb_tcp_disconnected , SUB_LONG , SUB_FXN( tcp_disconnected ) , NULL );

	mms_array_init( &_g->bridges.ABs , sizeof( AB ) , 1 , 1 , 0 );
}

PRE_MAIN_INITIALIZATION( PRE_MAIN_INIT_GLOBALS )
_PRIVATE_FXN void pre_main_init_helper_component( void )
{
	distributor_subscribe( &_g->distributors.bcast_pre_cfg , SUB_VOID , SUB_FXN( pre_config_init_helper ) , _g );
	
	distributor_init_withOrder( &_g->distributors.bcast_quit , 1 );
}

#endif

#ifndef event_handler

_CALLBACK_FXN void app_err_dist( pass_p src_g , LPCSTR msg )
{
	G * _g = ( G * )src_g;
	
	_g->stat.aggregate_stat.app_fault_count++;
	#ifdef HAS_STATISTICSS
	nnc_cell_triggered( _g->stat.nc_s_req.ov_fault_cell );
	#endif
}
_CALLBACK_FXN void pb_err_dist( pass_p src_pb , LPCSTR msg )
{
	AB * pb = ( AB * )src_pb;
	G * _g = TO_G( pb->cpy_cfg.m.m.temp_data._pseudo_g );
	pb->stat.round_zero_set.pb_fault_count++;
	distributor_publish_str( &_g->distributors.bcast_app_lvl_failure , msg , ( pass_p )_g );
	#ifdef HAS_STATISTICSS
	nnc_cell_triggered( pb->stat.pb_fault_cell ); // this is how i priotorized error view by addign this line in changes call back. instead if i added this line in some place that like pool and every for example one second call that it has lower prio .
	#endif
}

_CALLBACK_FXN void udp_connected( pass_p src_AB , long src_AB_udp )
{
	AB * pb = ( AB * )src_AB;
	AB_udp * udp = ( AB_udp * )src_AB_udp;
	
	pb->stat.round_zero_set.udp_connection_count++;
	pb->stat.round_zero_set.total_retry_udp_connection_count++;

	//distributor_publish_long( &pudp->bcast_change_state , 1 , udp ); // transfer state per case

	TO_G( pb->cpy_cfg.m.m.temp_data._pseudo_g )->stat.aggregate_stat.total_udp_connection_count++;
	TO_G( pb->cpy_cfg.m.m.temp_data._pseudo_g )->stat.aggregate_stat.total_retry_udp_connection_count++;
	
	#ifdef HAS_STATISTICSS
	nnc_cell_triggered( _g->stat.nc_s_req.ov_UDP_conn_cell );
	nnc_cell_triggered( _g->stat.nc_s_req.ov_UDP_retry_conn_cell );
	nnc_cell_triggered( pb->stat.pb_UDP_conn_cell );
	nnc_cell_triggered( pb->stat.pb_UDP_retry_conn_cell );
	#endif
}
_CALLBACK_FXN void udp_disconnected( pass_p src_AB , long src_AB_udp )
{
	AB_udp * pudp = ( AB_udp * )src_AB_udp;
	AB * pb = pudp->owner_pb;

	pb->stat.round_zero_set.udp_connection_count--;

	//distributor_publish_long( &pudp->bcast_change_state , 0 , src_AB_udp ); // transfer state per case

	TO_G( pb->cpy_cfg.m.m.temp_data._pseudo_g )->stat.aggregate_stat.total_udp_connection_count--;

	#ifdef HAS_STATISTICSS
	nnc_cell_triggered( _g->stat.nc_s_req.ov_UDP_conn_cell );
	nnc_cell_triggered( pb->stat.pb_UDP_conn_cell );
	#endif
}

// upper obeserver and distributor
_CALLBACK_FXN void tcp_connected( pass_p src_AB_tcp , long file_desc )
{
	AB_tcp * tcp = ( AB_tcp * )src_AB_tcp;
	AB * pb = tcp->owner_pb;
	
	// first try to send by file desc if succ then pointer must be valid
	dict_fst_put( &TO_G( tcp->owner_pb->cpy_cfg.m.m.temp_data._pseudo_g )->hdls.pkt_mgr.map_tcp_socket , tcp->__tcp_cfg_pak->name , ( int )file_desc , ( void_p )tcp , NULL , NULL , NULL );
	tcp->owner_pb->stat.round_zero_set.tcp_connection_count++;
	tcp->owner_pb->stat.round_zero_set.total_retry_tcp_connection_count++;

	if ( tcp->main_instance )
	{
		distributor_publish_long( &tcp->this->bcast_change_state , 1 , src_AB_tcp ); // transfer state per case
	}

	if ( tcp->main_instance )
	{
		TO_G( tcp->owner_pb->cpy_cfg.m.m.temp_data._pseudo_g )->stat.aggregate_stat.total_tcp_connection_count++;
		TO_G( tcp->owner_pb->cpy_cfg.m.m.temp_data._pseudo_g )->stat.aggregate_stat.total_retry_tcp_connection_count++;
	}

	#ifdef HAS_STATISTICSS
	if ( tcp->main_instance )
	{
		nnc_cell_triggered( _g->stat.nc_s_req.ov_TCP_conn_cell );
		nnc_cell_triggered( _g->stat.nc_s_req.ov_TCP_retry_conn_cell );
	}
	nnc_cell_triggered( pb->stat.pb_TCP_conn_cell );
	nnc_cell_triggered( pb->stat.pb_TCP_retry_conn_cell );
	#endif
}
_CALLBACK_FXN void tcp_disconnected( pass_p src_AB_tcp , long v )
{
	AB_tcp * tcp = ( AB_tcp * )src_AB_tcp;
	AB * pb = tcp->owner_pb;

	pb->stat.round_zero_set.tcp_connection_count--;

	distributor_publish_long( &tcp->this->bcast_change_state , 0 , src_AB_tcp ); // transfer state per case

	TO_G( pb->cpy_cfg.m.m.temp_data._pseudo_g )->stat.aggregate_stat.total_tcp_connection_count--;

	#ifdef HAS_STATISTICSS
	nnc_cell_triggered( _g->stat.nc_s_req.ov_TCP_conn_cell );
	nnc_cell_triggered( pb->stat.pb_TCP_conn_cell );
	#endif
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
	G * _g = ( G * )pb->cpy_cfg.m.m.temp_data._pseudo_g;
	
	distributor_publish_long( &_g->distributors.bcast_thread_startup , (long)pthread_self() , _g );
	__attribute__( ( cleanup( thread_goes_out_of_scope ) ) ) pthread_t trd_id = pthread_self();
#ifdef ENABLE_USE_INTERNAL_C_STATISTIC
	MARK_START_THREAD();
#endif

	for ( int iudp = 0 ; iudp < pb->udps_count ; iudp++ )
	{
		if ( pb->udps[ iudp ].udp_connection_established )
		{
			_close_socket( &pb->udps[ iudp ].udp_sockfd );
			pb->udps[ iudp ].udp_connection_established = 0;
			//_g->stat.udp_connection_count--;
			// TODO
		}

		NM_BREAK_IF( ( pb->udps[ iudp ].udp_sockfd = socket( AF_INET , SOCK_DGRAM , 0 ) ) == FXN_SOCKET_ERR , errSocket , 1 , "create sock error" );

		int optval = 1;
		//socklen_t optlen = sizeof( optval );
		//MM_BREAK_IF( getsockopt( pb->udps[ iudp ].udp_sockfd , SOL_SOCKET , SO_REUSEADDR , &optval , &optlen ) < 0 , errSocket , 1 , "SO_REUSEADDR" );
		NM_BREAK_IF( setsockopt( pb->udps[ iudp ].udp_sockfd , SOL_SOCKET , SO_REUSEADDR , &optval , sizeof( optval ) ) < 0 , errSocket , 1 , "SO_REUSEADDR" );

		int flags = fcntl( pb->udps[ iudp ].udp_sockfd , F_GETFL );
		fcntl( pb->udps[ iudp ].udp_sockfd , F_SETFL , flags | O_NONBLOCK );

		// TODO . make it protocol independent
		struct sockaddr_in server_addr;
		MEMSET( &server_addr , 0 , sizeof( server_addr ) );
		server_addr.sin_family = AF_INET; // IPv4

		int port = 0;
		N_BREAK_IF( string_to_int( pb->udps[ iudp ].__udp_cfg_pak->data.core.UDP_origin_ports , &port ) , errGeneral , 0 );
		WARNING( port > 0 );
		server_addr.sin_port = htons( ( uint16_t )port ); // Convert port to network byte order
		if ( iSTR_SAME( pb->udps[ iudp ].__udp_cfg_pak->data.core.UDP_origin_ip , "INADDR_ANY" ) )
		{
			server_addr.sin_addr.s_addr = INADDR_ANY; // Or use INADDR_ANY to bind to all available interfaces:
		}
		else
		{
			server_addr.sin_addr.s_addr = inet_addr( pb->udps[ iudp ].__udp_cfg_pak->data.core.UDP_origin_ip ); // Specify the IP address to bind to
		}
		
		N_BREAK_IF( bind( pb->udps[ iudp ].udp_sockfd , ( const struct sockaddr * )&server_addr , sizeof( server_addr ) ) == FXN_BIND_ERR , errSocket , 1 );
		pb->udps[ iudp ].udp_connection_established = 1;

		distributor_publish_long( &_g->distributors.bcast_pb_udp_connected , (long)&( pb->udps[ iudp ] ) , pb );

		//_g->bridges.under_listen_udp_sockets_group_changed++; // if any udp socket change then fdset must be reinitialized
	}

	BEGIN_RET
	case 1:	DIST_BRIDGE_FAILURE();
	M_V_END_RET
	return NULL; // Threads can return a value, but this example returns NULL
}

_PRIVATE_FXN status connect_one_tcp( AB_tcp * tcp )
{
	INIT_BREAKABLE_FXN();
	AB * pb = tcp->owner_pb;
	G * _g = tcp->owner_pb->cpy_cfg.m.m.temp_data._pseudo_g;

	while ( 1 )
	{
		_close_socket( &tcp->tcp_sockfd );

		// try to create TCP socket
		NM_BREAK_IF( ( tcp->tcp_sockfd = socket( AF_INET , SOCK_STREAM , 0 ) ) == FXN_SOCKET_ERR , errSocket , 0 , "create sock error" );

		//struct sockaddr_in tcp_addr;
		//tcp_addr.sin_family = AF_INET;

		int port = 0;
		N_BREAK_IF( string_to_int( tcp->__tcp_cfg_pak->data.core.TCP_destination_ports , &port ) , errGeneral , 1 );
		WARNING( port > 0 );

		//tcp_addr.sin_port = htons( ( uint16_t )port );
		//NM_BREAK_IF( inet_pton( AF_INET , tcp->__tcp_cfg_pak->data.core.TCP_destination_ip , &tcp_addr.sin_addr ) <= 0 , errSocket , 1 , "inet_pton sock error" );

		//if ( connect( tcp->tcp_sockfd , ( struct sockaddr * )&tcp_addr , sizeof( tcp_addr ) ) == -1 )
		if ( ( tcp->tcp_sockfd = connect_with_timeout( tcp->__tcp_cfg_pak->data.core.TCP_destination_ip , port , BAD_NETWORK_HANDSHAKE_TIMEOUT TODO ) ) == -1 )
		{
			if ( errno == ECONNREFUSED || errno == ETIMEDOUT )
			{
				_close_socket( &tcp->tcp_sockfd );
				//mng_basic_thread_sleep( _g , NORMAL_PRIORITY_THREAD );
				//continue;
			}
			
			N_BREAK_IF( 1 , errSocket , 1 );
		}
		else
		{
			fcntl( tcp->tcp_sockfd , F_SETFL , O_NONBLOCK ); // to prevent tcp stack handles lingering to closed peer connection
			//enable_keepalive_chaotic( tcp->tcp_sockfd ); // to keep alive and try to probe peer in semi normal sitribution time and does not have line up in connction

			tcp->tcp_connection_established = 1;
			tcp->tcp_is_about_to_connect = 0;
			distributor_publish_long( &_g->distributors.bcast_pb_tcp_connected , tcp->tcp_sockfd , ( pass_p )tcp );
			return errOK;
		}
	}

	BEGIN_RET
	case 1:
	{
		_close_socket( &tcp->tcp_sockfd );
		//DIST_BRIDGE_FAILURE();
	}
	M_V_END_RET
	return errSocket;
}

_THREAD_FXN void_p thread_tcp_connection_proc( pass_p src_g )
{
	INIT_BREAKABLE_FXN();
	G * _g = ( G * )src_g;
	
	distributor_publish_long( &_g->distributors.bcast_thread_startup , (long)pthread_self() , _g );
	__attribute__( ( cleanup( thread_goes_out_of_scope ) ) ) pthread_t trd_id = pthread_self();
#ifdef ENABLE_USE_INTERNAL_C_STATISTIC
	MARK_START_THREAD();
#endif

	while ( 1 )
	{
		if ( GRACEFULLY_END_THREAD() ) break;

		pthread_mutex_lock( &_g->bridges.tcps_trd.mtx );
		for ( size_t ab_idx = 0 ; ab_idx < _g->bridges.ABs.count ; ab_idx++ )
		{
			AB * pb = NULL;
			if ( mms_array_get_s( &_g->bridges.ABs , ab_idx , ( void ** )&pb ) == errOK )
			{
				for ( int itcp = 0 ; itcp < pb->tcps_count ; itcp++ )
				{
					if ( pb->tcps[ itcp ].main_instance )
					{
						if ( pb->tcps[ itcp ].tcp_connection_established )
						{
							if
							(
								!pb->tcps[ itcp ].tcp_is_about_to_connect &&
								( time( NULL ) - pb->tcps[ itcp ].last_access ) > 60 TODO
							)
							{
								pb->tcps[ itcp ].last_access = time( NULL );
								if ( is_socket_connected_peek( pb->tcps[ itcp ].tcp_sockfd , 0 ) <= 0 )
								{
									pb->tcps[ itcp ].tcp_is_about_to_connect = 1;
									pb->tcps[ itcp ].tcp_connection_established = 0;
									itcp = 0;
									continue;
								}
							}
							continue;
						}
						if ( !pb->tcps[ itcp ].tcp_connection_established )
						{
							connect_one_tcp( &pb->tcps[ itcp ] );
						}
					}
					//else // !main_instance
					//{
					//	if ( pb->tcps[ itcp ].this->tcp_connection_established )
					//	{
					//		distributor_publish_long( &_g->distributors.bcast_pb_tcp_connected , pb->tcps[ itcp ].this->tcp_sockfd , ( pass_p )( &pb->tcps[ itcp ] ) );
					//	}
					//}
				}
			}
		}
		pthread_mutex_unlock( &_g->bridges.tcps_trd.mtx );

		mng_basic_thread_sleep( _g , NORMAL_PRIORITY_THREAD );
	}

	BREAK_OK( 0 ); // to just ignore gcc warning

	BEGIN_SMPL
		M_V_END_RET
		return NULL; // Threads can return a value, but this example returns NULL
}

#endif

/// <summary>
/// this thread must be as simple as possible and without exception and any error because it is resposible for checking other and making them up
/// </summary>
_THREAD_FXN void_p watchdog_executer( pass_p src_g )
{
	INIT_BREAKABLE_FXN();

	G * _g = ( G * )src_g;

	while ( !_g->cmd.quit_app_4 )
	{
		if ( _g->cmd.quit_app_4 ) break; // most be closed when every thing closed

		#ifdef ENABLE_LOCK_TRACING
			for ( size_t tt1 = 0 ; tt1 < DD_MAX_FILE ; tt1++ )
			{
				for ( size_t tt2 = 0 ; tt2 < DD_LINE_COUNT ; tt2++ )
				{
					if ( __lck_hit[ tt1 ][ tt2 ].state != 0 )
					{
						int u = 2 + 2;
					}
				}
			}
		#endif

		mng_basic_thread_sleep( _g , NORMAL_PRIORITY_THREAD );
	}

	BEGIN_SMPL
	M_V_END_RET
	return NULL; // Threads can return a value, but this time returns NULL
}

#ifndef usual_utils

/// <summary>
/// single point for thread wait
/// </summary>
void mng_basic_thread_sleep( G * _g , int priority ) TODO 
{
	struct timespec ts;
	MEMSET_ZERO( &ts , 1 );
	switch ( priority )
	{
		case VLOW_PRIORITY_THREAD:
		{
			ts.tv_nsec = VLOW_THREAD_DEFAULT_DELAY_NANOSEC();
			break;
		}
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
_PRIVATE_FXN _CALLBACK_FXN LPCSTR itr_interfaces( const pass_p arr , size_t i )
{
	return ( ( AB_udp * )arr )[ i ].__udp_cfg_pak->data.core.UDP_origin_interface;
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
		M_BREAK_IF( !( *interface_filter[ iint ] = STRDUP( distinct_interface.strs[ iint ] ) ) , errMemoryLow , 0 );
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
				if ( iSTR_SAME( abs->udps[ iout ].__udp_cfg_pak->data.core.UDP_origin_interface , *interface_filter[ iint ] ) )
				{
					M_BREAK_STAT( addTo_string_ar( &ports_ar , abs->udps[ iout ].__udp_cfg_pak->data.core.UDP_origin_ports ) , 0 );
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

		// TODO . implement multiple source and dest ip

		if ( distinct_ports.size == 1 && STR_SAME( distinct_ports.strs[ 0 ] , "-" ) )
		{
			n += sprintf( prt_flt + n , "src host %s and dst host %s" , abs->udps[ 0 ].__udp_cfg_pak->data.core.UDP_origin_ip , abs->udps[ 0 ].__udp_cfg_pak->data.core.UDP_destination_ip );
		}
		else
		{
			for ( int iprt = 0 ; iprt < distinct_ports.size ; iprt++ )
			{
				if ( iprt > 0 )
				{
					n += sprintf( prt_flt + n , " or " );
				}

				// TODO . later i should check config file that no duplicate port find in it
				for ( int islk = 0 ; islk < abs->udps_count ; islk++ )
				{
					if ( iSTR_SAME( abs->udps[ islk ].__udp_cfg_pak->data.core.UDP_origin_interface , *interface_filter[iint] )
						&& iSTR_SAME( abs->udps[ islk ].__udp_cfg_pak->data.core.UDP_origin_ports , distinct_ports.strs[iprt] ) )
					{
						if ( strchr( abs->udps[ islk ].__udp_cfg_pak->data.core.UDP_origin_ports , '-' ) != NULL )
						{
							// it's a range
							n += sprintf( prt_flt + n , "dst portrange %s" , abs->udps[ islk ].__udp_cfg_pak->data.core.UDP_origin_ports );
						}
						else
						{
							// it's a single port
							n += sprintf( prt_flt + n , "dst port %s" , abs->udps[ islk ].__udp_cfg_pak->data.core.UDP_origin_ports );
						}
						break;
					}
				}
			}
		}

		// close parenthesis
		n += sprintf( prt_flt + n , ")" );
	}

	BEGIN_SMPL
	M_V_END_RET
}

_CALLBACK_FXN void thread_goes_out_of_scope( void * ptr )
{
	pthread_mutex_lock( &_g->hdls.thread_close_mtx );
	pthread_t * ptrd = ( pthread_t * )ptr;
	size_t sz = array_get_count( &_g->hdls.registered_thread );
	pthread_t * ppthread_t = NULL;
	for ( size_t idx = 0 ; idx < sz ; idx++ )
	{
		if ( array_get_s( &_g->hdls.registered_thread , idx , ( void ** )&ppthread_t ) == errOK && *ppthread_t == *ptrd )
		{
			*ppthread_t = 0;
		}
	}
	pthread_mutex_unlock( &_g->hdls.thread_close_mtx );
}

#endif

#ifndef input_thread

_THREAD_FXN void_p stdout_bypass_thread( pass_p src_g )
{
	G * _g = ( G * )src_g;
	
	distributor_publish_long( &_g->distributors.bcast_thread_startup , (long)pthread_self() , _g );
	__attribute__( ( cleanup( thread_goes_out_of_scope ) ) ) pthread_t trd_id = pthread_self();
#ifdef ENABLE_USE_INTERNAL_C_STATISTIC
	MARK_START_THREAD();
#endif

	fd_set readfds;
	char buffer[ DEFAULT_BUF_SIZE ];
	while ( 1 )
	{
		if ( GRACEFULLY_END_THREAD() ) break;

		FD_ZERO( &readfds );
		FD_SET( _g->stat.pipefds[ 0 ] , &readfds );

		struct timeval timeout = {0}; // Set timeout (e.g., 5 seconds)
		timeout.tv_sec = 1;
		int ready = select( _g->stat.pipefds[ 0 ] + 1 , &readfds , NULL , NULL , &timeout );
		if ( ready <= 0 )
		{
			sleep(1);
			continue;
		}
		if ( ready > 0 && FD_ISSET( _g->stat.pipefds[ 0 ] , &readfds ) )
		{
			int n = (int)read( _g->stat.pipefds[ 0 ] , buffer , DEFAULT_BUF_SIZE - 1 );
			buffer[ n ] = EOS;
#pragma GCC diagnostic ignored "-Wstringop-truncation"
			#ifdef HAS_STATISTICSS
			strncpy( _g->stat.nc_h.message_text , buffer , sizeof( _g->stat.nc_h.message_text ) - 1 );
			#endif
#pragma GCC diagnostic pop
		}
	}
	return NULL;
}

void init_bypass_stdout( G * _g )
{
	int pipefd[ 2 ];
	MEMSET( pipefd , 0 , sizeof( pipefd ) );

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

_THREAD_FXN void_p sync_thread( pass_p src_g ) // pause app until moment other app exist
{
	//INIT_BREAKABLE_FXN();

	//G * _g = ( G * )src_g;
	// distributor_publish_long( &_g->distributors.bcast_thread_startup , (long)pthread_self() , _g );
	//__attribute__( ( cleanup( thread_goes_out_of_scope ) ) ) pthread_t trd_id = pthread_self();
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
	//INIT_BREAKABLE_FXN();
	//G * _g = ( G * )src_g;
	//distributor_publish_long( &_g->distributors.bcast_thread_startup , (long)pthread_self() , _g );
	//__attribute__( ( cleanup( thread_goes_out_of_scope ) ) ) pthread_t trd_id = pthread_self();

	while ( 1 )
	{
		//pthread_mutex_lock( &_g->stat.lock_data.lock );

		//werase( _g->stat.input_win );
		//box( _g->stat.input_win , 0 , 0 );

		//pthread_mutex_unlock( &_g->stat.lock_data.lock );

		//// Enable echo and get input
		//echo();
		//curs_set( 1 );
		//wmove( _g->stat.input_win , 1 , 1 );
		//wprintw( _g->stat.input_win , "cmd(quit,sync,rst): " );
		//wrefresh( _g->stat.input_win );
		//wgetnstr( _g->stat.input_win , _g->stat.input_buffer , INPUT_MAX - 1 );
		//noecho();
		//curs_set( 0 );

		//pthread_mutex_lock( &_g->stat.lock_data.lock );
		//Boolean boutput_command = True;

		//if ( iSTR_SAME( _g->stat.input_buffer , "quit" ) )
		//{
		//	_g->cmd.quit_app_4 = 1;
		//	break;
		//}
		//else if ( iSTR_SAME( _g->stat.input_buffer , "sync" ) )
		//{
		//	boutput_command = False;
		//	pthread_t thread;
		//	if ( pthread_create( &thread , NULL , sync_thread , src_g ) != 0 )
		//	{
		//		_ECHO( "pthread_create" );
		//	}
		//	//break;
		//}
		//else if ( iSTR_SAME( _g->stat.input_buffer , "rst" ) )
		//{
		//	boutput_command = False;
		//	reset_nonuse_stat();
		//	//break;
		//}
		//
		//if ( boutput_command )
		//{
		//	strncpy( _g->stat.last_command , _g->stat.input_buffer , INPUT_MAX );
		//	_g->stat.last_command[ INPUT_MAX - 1 ] = EOS;
		//}

		//pthread_mutex_unlock( &_g->stat.lock_data.lock );
	}
	//BEGIN_SMPL
	//M_V_END_RET
	return NULL;
}

#endif

#ifndef update_cell_section
#ifdef HAS_STATISTICSS

_CALLBACK_FXN PASSED_CSTR ov_cell_time_2_str( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	//G * _g = ( G * )pcell->storage.bt.pass_data;
	struct timespec now;
	clock_gettime( CLOCK_REALTIME , &now );
	format_clock_time( &now , pcell->storage.tmpbuf , sizeof( pcell->storage.tmpbuf ) );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR ov_cell_version_2_str( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	G * _g = ( G * )pcell->storage.bt.pass_data;
	sprintf( pcell->storage.tmpbuf , "%s" , _g->appcfg.ver->version );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR ov_UDP_conn_2_str( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	G * _g = ( G * )pcell->storage.bt.pass_data;
	sprintf( pcell->storage.tmpbuf , "%d" , _g->stat.aggregate_stat.total_udp_connection_count );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR ov_TCP_conn_2_str( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	G * _g = ( G * )pcell->storage.bt.pass_data;
	sprintf( pcell->storage.tmpbuf , "%d" , _g->stat.aggregate_stat.total_tcp_connection_count );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR ov_UDP_retry_2_str( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	G * _g = ( G * )pcell->storage.bt.pass_data;
	sprintf( pcell->storage.tmpbuf , "%d" , _g->stat.aggregate_stat.total_retry_udp_connection_count );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR ov_TCP_retry_2_str( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	G * _g = ( G * )pcell->storage.bt.pass_data;
	sprintf( pcell->storage.tmpbuf , "%d" , _g->stat.aggregate_stat.total_retry_tcp_connection_count );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR ov_fault_2_str( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	G * _g = ( G * )pcell->storage.bt.pass_data;
	_FORMAT_SHRTFRM( pcell->storage.tmpbuf , sizeof(pcell->storage.tmpbuf) , _g->stat.aggregate_stat.app_fault_count , 2 , "" );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR ov_time_elapse_2_str( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	struct timeval t_end;
	gettimeofday( &t_end , NULL );
	G * _g = ( G * )pcell->storage.bt.pass_data;
	format_elapsed_time_with_millis( _g->stat.aggregate_stat.t_begin , t_end , pcell->storage.tmpbuf , sizeof( pcell->storage.tmpbuf ) , 1 );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR ov_thread_cnt_2_str( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	G * _g = ( G * )pcell->storage.bt.pass_data;
	sprintf( pcell->storage.tmpbuf , "%d" , _g->hdls.registered_thread.count );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR pb_time_elapse_2_str( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	AB * pb = ( AB * )pcell->storage.bt.pass_data;
	format_elapsed_time_with_millis( pb->stat.round_zero_set.t_begin , pb->stat.round_zero_set.t_end , pcell->storage.tmpbuf , sizeof( pcell->storage.tmpbuf ) , 1 );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR pb_fault_2_str( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	AB * pb = ( AB * )pcell->storage.bt.pass_data;
	_FORMAT_SHRTFRM( pcell->storage.tmpbuf , sizeof( pcell->storage.tmpbuf ) , pb->stat.round_zero_set.pb_fault_count , 2 , "" );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR pb_fst_cash_lost_2_str( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	AB * pb = ( AB * )pcell->storage.bt.pass_data;
	_FORMAT_SHRTFRM( pcell->storage.tmpbuf , sizeof( pcell->storage.tmpbuf ) , ( __int64u )pb->comm.preq.raw_xudp_cache.err_full , 2 , "" );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR pb_UDP_conn_2_str( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	AB * pb = ( AB * )pcell->storage.bt.pass_data;
	sprintf( pcell->storage.tmpbuf , "%d" , pb->stat.round_zero_set.udp_connection_count );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR pb_TCP_conn_2_str( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	AB * pb = ( AB * )pcell->storage.bt.pass_data;
	sprintf( pcell->storage.tmpbuf , "%d" , pb->stat.round_zero_set.tcp_connection_count );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR pb_UDP_retry_2_str( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	AB * pb = ( AB * )pcell->storage.bt.pass_data;
	sprintf( pcell->storage.tmpbuf , "%d" , pb->stat.round_zero_set.total_retry_udp_connection_count );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR pb_TCP_retry_2_str( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	AB * pb = ( AB * )pcell->storage.bt.pass_data;
	sprintf( pcell->storage.tmpbuf , "%d" , pb->stat.round_zero_set.total_retry_tcp_connection_count );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR pb_UDP_get_count_2_str( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	AB * pb = ( AB * )pcell->storage.bt.pass_data;
	_FORMAT_SHRTFRM( pcell->storage.tmpbuf , sizeof( pcell->storage.tmpbuf ) , pb->stat.round_zero_set.udp.total_udp_get_count , 2 , "" );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR pb_UDP_get_byte_2_str( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	AB * pb = ( AB * )pcell->storage.bt.pass_data;
	_FORMAT_SHRTFRM( pcell->storage.tmpbuf , sizeof( pcell->storage.tmpbuf ) , pb->stat.round_zero_set.udp.total_udp_get_byte , 2 , "B" );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR pb_TCP_put_count_2_str( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	AB * pb = ( AB * )pcell->storage.bt.pass_data;
	_FORMAT_SHRTFRM( pcell->storage.tmpbuf , sizeof( pcell->storage.tmpbuf ) , pb->stat.round_zero_set.tcp.total_tcp_put_count , 2 , "" );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR pb_TCP_put_byte_2_str( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	AB * pb = ( AB * )pcell->storage.bt.pass_data;
	_FORMAT_SHRTFRM( pcell->storage.tmpbuf , sizeof( pcell->storage.tmpbuf ) , pb->stat.round_zero_set.tcp.total_tcp_put_byte , 2 , "B" );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

#ifdef ENABLE_THROUGHPUT_MEASURE
_CALLBACK_FXN PASSED_CSTR pb_5s_udp_pps_2_str( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	AB * pb = ( AB * )pcell->storage.bt.pass_data;
	_FORMAT_SHRTFRM( pcell->storage.tmpbuf , sizeof( pcell->storage.tmpbuf ) , ( ubigint )cbuf_m_mean_all( &pb->stat.round_init_set.udp_stat_5_sec_count ) , 4 , "" );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR pb_5s_udp_bps_2_str( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	AB * pb = ( AB * )pcell->storage.bt.pass_data;
	_FORMAT_SHRTFRM( pcell->storage.tmpbuf , sizeof( pcell->storage.tmpbuf ) , ( ubigint )cbuf_m_mean_all( &pb->stat.round_init_set.udp_stat_5_sec_bytes ) , 4 , "B" );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR pb_10s_udp_pps_2_str( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	AB * pb = ( AB * )pcell->storage.bt.pass_data;
	_FORMAT_SHRTFRM( pcell->storage.tmpbuf , sizeof( pcell->storage.tmpbuf ) , ( ubigint )cbuf_m_mean_all( &pb->stat.round_init_set.udp_stat_10_sec_count ) , 4 , "" );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR pb_10s_udp_bps_2_str( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	AB * pb = ( AB * )pcell->storage.bt.pass_data;
	_FORMAT_SHRTFRM( pcell->storage.tmpbuf , sizeof( pcell->storage.tmpbuf ) , ( ubigint )cbuf_m_mean_all( &pb->stat.round_init_set.udp_stat_10_sec_bytes ) , 4 , "B" );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR pb_40s_udp_pps_2_str( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	AB * pb = ( AB * )pcell->storage.bt.pass_data;
	_FORMAT_SHRTFRM( pcell->storage.tmpbuf , sizeof( pcell->storage.tmpbuf ) , ( ubigint )cbuf_m_mean_all( &pb->stat.round_init_set.udp_stat_40_sec_count ) , 4 , "" );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR pb_40s_udp_bps_2_str( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	AB * pb = ( AB * )pcell->storage.bt.pass_data;
	_FORMAT_SHRTFRM( pcell->storage.tmpbuf , sizeof( pcell->storage.tmpbuf ) , ( ubigint )cbuf_m_mean_all( &pb->stat.round_init_set.udp_stat_40_sec_bytes ) , 4 , "B" );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR pb_5s_tcp_pps_2_str( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	AB * pb = ( AB * )pcell->storage.bt.pass_data;
	_FORMAT_SHRTFRM( pcell->storage.tmpbuf , sizeof( pcell->storage.tmpbuf ) , ( ubigint )cbuf_m_mean_all( &pb->stat.round_init_set.tcp_stat_5_sec_count ) , 4 , "" );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR pb_5s_tcp_bps_2_str( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	AB * pb = ( AB * )pcell->storage.bt.pass_data;
	_FORMAT_SHRTFRM( pcell->storage.tmpbuf , sizeof( pcell->storage.tmpbuf ) , ( ubigint )cbuf_m_mean_all( &pb->stat.round_init_set.tcp_stat_5_sec_bytes ) , 4 , "B" );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR pb_10s_tcp_pps_2_str( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	AB * pb = ( AB * )pcell->storage.bt.pass_data;
	_FORMAT_SHRTFRM( pcell->storage.tmpbuf , sizeof( pcell->storage.tmpbuf ) , ( ubigint )cbuf_m_mean_all( &pb->stat.round_init_set.tcp_stat_10_sec_count ) , 4 , "" );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR pb_10s_tcp_bps_2_str( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	AB * pb = ( AB * )pcell->storage.bt.pass_data;
	_FORMAT_SHRTFRM( pcell->storage.tmpbuf , sizeof( pcell->storage.tmpbuf ) , ( ubigint )cbuf_m_mean_all( &pb->stat.round_init_set.tcp_stat_10_sec_bytes ) , 4 , "B" );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR pb_40s_tcp_pps_2_str( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	AB * pb = ( AB * )pcell->storage.bt.pass_data;
	_FORMAT_SHRTFRM( pcell->storage.tmpbuf , sizeof( pcell->storage.tmpbuf ) , ( ubigint )cbuf_m_mean_all( &pb->stat.round_init_set.tcp_stat_40_sec_count ) , 4 , "" );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR pb_40s_tcp_bps_2_str( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	AB * pb = ( AB * )pcell->storage.bt.pass_data;
	_FORMAT_SHRTFRM( pcell->storage.tmpbuf , sizeof( pcell->storage.tmpbuf ) , ( ubigint )cbuf_m_mean_all( &pb->stat.round_init_set.tcp_stat_40_sec_bytes ) , 4 , "B" );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}
#endif
#endif
#endif

