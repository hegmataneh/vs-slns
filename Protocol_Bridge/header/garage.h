#pragma once



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




//#define MAX_UDP_PACKET_DELAY 1
//#define MAX_TCP_PACKET_DELAY 2

//#define FXN_HIT_COUNT 5000
//#define PC_COUNT 10 // first for hit count and last alwayz zero

//int __FXN_HIT[FXN_HIT_COUNT][PC_COUNT] = {0}; // max size is about number of code lines
//static int _pc = 1; // step of each call globally
//#define SYS_ALIVE_CHECK() do {\
//	int __function_line = __LINE__; _g->stat.last_line_meet = __LINE__; _g->stat.alive_check_counter = ( _g->stat.alive_check_counter + 1 ) % 10; __FXN_HIT[__function_line][0]++; static int pc = 0;/*each line hit*/ if ( pc <= PC_COUNT-1 ) __FXN_HIT[__function_line][1+pc++] = _pc++; \
//	} while(0)



//else if ( iSTR_SAME( pb->cpy_cfg.m.m.id.thread_handler_act , BOTTLENECK ) )
//{
//	if ( !_g->bridges.bottleneck_thread )
//	{
//		_g->bridges.bottleneck_thread = NEW( struct bridges_bottleneck_thread );
//		MEMSET_ZERO( _g->bridges.bottleneck_thread , struct bridges_bottleneck_thread , 1 );
//		pthread_mutex_init( &_g->bridges.trd.creation_thread_race_cond , NULL );
//		pthread_mutex_init( &_g->bridges.trd.start_working_race_cond , NULL );
//		pthread_mutex_lock( &_g->bridges.trd.base.creation_thread_race_cond );
//		if ( !_g->bridges.trd.base.thread_is_created )
//		{
//			MM_BREAK_IF( pthread_create( &_g->bridges.bidirection_thread->mem.income_trd_id , NULL , income_thread_proc , _g ) != PTHREAD_CREATE_OK , errGeneral , 0 , "thread creation failed" );
//			MM_BREAK_IF( pthread_create( &_g->bridges.bidirection_thread->mem.outgoing_trd_id , NULL , outgoing_thread_proc , _g ) != PTHREAD_CREATE_OK , errGeneral , 0 , "thread creation failed" );
//			_g->bridges.trd.base.thread_is_created = 1;
//		}
//		pthread_mutex_unlock( &_g->bridges.trd.base.creation_thread_race_cond );
//	}
//}
//else if ( iSTR_SAME( pb->cpy_cfg.m.m.id.thread_handler_act , BIDIRECTION ) )
//{
//	if ( !_g->bridges.bidirection_thread )
//	{
//		_g->bridges.bidirection_thread = NEW( struct bridges_bidirection_thread );
//		memset( &_g->bridges.bidirection_thread->mem , 0 , sizeof( struct bridges_bidirection_thread_zero_init_memory ) );
//		queue_init( &_g->bridges.bidirection_thread->queue );
//		pthread_mutex_init( &_g->bridges.trd.start_working_race_cond , NULL );
//		pthread_mutex_lock( &_g->bridges.trd.base.creation_thread_race_cond );
//		if ( !_g->bridges.trd.base.thread_is_created )
//		{
//			MM_BREAK_IF( pthread_create( &_g->bridges.bottleneck_thread->trd_id , NULL , bottleneck_thread_proc , _g ) != PTHREAD_CREATE_OK , errGeneral , 0 , "thread creation failed" );
//			_g->bridges.trd.base.thread_is_created = 1;
//		}
//		pthread_mutex_unlock( &_g->bridges.trd.base.creation_thread_race_cond );
//	}
//}
//if ( _g->appcfg.g_cfg->c.c.atht == buttleneck || _g->appcfg.g_cfg->c.c.atht == bidirection )
//{
//	// first close then reconnect
//	if ( tmp_try_to_connect_udp_port )
//	{
//		pthread_t trd_udp_connection;
//		MM_BREAK_IF( pthread_create( &trd_udp_connection , NULL , connect_udps_proc , pb ) != PTHREAD_CREATE_OK , errGeneral , 0 , "thread creation failed" );
//	}
//	if ( tmp_try_to_connect_tcp_port )
//	{
//		pthread_t trd_tcp_connection;
//		MM_BREAK_IF( pthread_create( &trd_tcp_connection , NULL , thread_tcp_connection_proc , pb ) != PTHREAD_CREATE_OK , errGeneral , 0 , "thread creation failed" );
//	}
//}
//_THREAD_FXN void_p bottleneck_thread_proc( pass_p src_pb )
//{
//	INIT_BREAKABLE_FXN();
//
//	////AB *pb = ( AB * )src_pb;
//	//G * _g = ( G * )src_g;
//
//	//time_t tnow = 0;
//
//	//while ( !_g->bridges.thread_base.start_working )
//	//{
//	//	if ( _g->bridges.thread_base.do_close_thread )
//	//	{
//	//		break;
//	//	}
//	//	mng_basic_thread_slep();
//	//}
//
//	//int input_udp_socket_error_tolerance_count = 0; // restart socket after many error accur
//	//int output_tcp_socket_error_tolerance_count = 0; // restart socket after many error accur
//
//	//int config_changes = 0;
//	//do
//	//{
//	//	if ( _g->bridges.thread_base.do_close_thread )
//	//	{
//	//		break;
//	//	}
//
//	//	config_changes = 0;
//
//	//	char buffer[ BUFFER_SIZE ]; // Define a buffer to store received data
//	//	struct sockaddr_in client_addr;
//	//	socklen_t client_len = sizeof( client_addr );
//	//	ssize_t bytes_received;
//
//	//	fd_set readfds; // Set of socket descriptors
//	//	FD_ZERO( &readfds );
//
//	//	ssize_t sz;
//	//	int num_valid_masks = 0;
//
//	//	int sockfd_max = -1; // for select compulsion
//	//	for ( int i = 0 ; i < _g->bridges.ABhs_masks_count ; i++ )
//	//	{
//	//		if ( _g->bridges.ABhs_masks[ i ] )
//	//		{
//	//			if ( _g->bridges.ABs[ i ].single_AB->udp_connection_established && _g->bridges.ABs[ i ].single_AB->tcp_connection_established )
//	//			{
//	//				FD_SET( _g->bridges.ABs[ i ].single_AB->udp_sockfd , &readfds );
//	//				if ( _g->bridges.ABs[ i ].single_AB->udp_sockfd > sockfd_max )
//	//				{
//	//					sockfd_max = _g->bridges.ABs[ i ].single_AB->udp_sockfd;
//	//				}
//	//			}
//	//		}
//	//	}
//	//	_g->bridges.under_listen_udp_sockets_group_changed = 0; // if any udp socket change then fdset must be reinitialized
//
//	//	if ( sockfd_max <= 1 )
//	//	{
//	//		slep( 1 );
//	//		continue;
//	//	}
//
//	//	while ( 1 )
//	//	{
//
//	//		//pthread_mutex_lock( &_g->sync.mutex );
//	//		//while ( _g->sync.lock_in_progress )
//	//		//{
//	//		//	////struct timespec ts = { 0, 10L };
//	//		//	////thrd_slep( &ts , NULL );
//	//		//	pthread_cond_wait( &_g->sync.cond , &_g->sync.mutex );
//	//		//}
//	//		//pthread_mutex_unlock( &_g->sync.mutex );
//	//		//if ( _g->sync.reset_static_after_lock )
//	//		//{
//	//		//	_g->sync.reset_static_after_lock = 0;
//	//		//	memset( &_g->stat.round , 0 , sizeof( _g->stat.round ) );
//	//		//}
//
//	//		if ( _g->bridges.thread_base.do_close_thread )
//	//		{
//	//			break;
//	//		}
//	//		if ( _g->bridges.under_listen_udp_sockets_group_changed )
//	//		{
//	//			config_changes = 1;
//	//			break;
//	//		}
//
//	//		struct timeval timeout; // Set timeout (e.g., 5 seconds)
//	//		timeout.tv_sec = 5;
//	//		timeout.tv_usec = 0;
//
//	//		// Wait for an activity on one of the sockets, timeout is NULL, so wait indefinitely
//	//		int activity = select( sockfd_max + 1 , &readfds , NULL , NULL , _g->appcfg.g_cfg->c.c.time_out_sec > 0 ? &timeout : NULL );
//
//	//		if ( ( activity < 0 ) /* && ( errno != EINTR )*/ )
//	//		{
//
//	//			_g->stat.round_zero_set.udp.continuously_unsuccessful_select_on_open_port_count++;
//
//	//			int error = 0;
//	//			socklen_t errlen = sizeof( error );
//	//			getsockopt( sockfd_max , SOL_SOCKET , SO_ERROR , &error , &errlen );
//	//			if ( error != 0 )
//	//			{
//	//				_ECHO( "Socket error: %d\n" , error );
//	//			}
//
//	//			if ( ++input_udp_socket_error_tolerance_count > RETRY_UNEXPECTED_WAIT_FOR_SOCK() )
//	//			{
//	//				input_udp_socket_error_tolerance_count = 0;
//	//				for ( int i = 0 ; i < _g->bridges.ABhs_masks_count ; i++ )
//	//				{
//	//					if ( _g->bridges.ABhs_masks[ i ] )
//	//					{
//	//						if ( _g->bridges.ABs[ i ].single_AB->udp_connection_established )  // all the connected udp stoped or die so restart them
//	//						{
//	//							//if ( FD_ISSET( _g->bridges.ABs[ i ].single_AB->udp_sockfd , &readfds ) )
//	//							{
//	//								_g->bridges.ABs[ i ].single_AB->retry_to_connect_udp = 1;
//	//								break;
//	//							}
//	//						}
//	//					}
//	//				}
//	//			}
//
//	//			continue;
//	//		}
//	//		if ( activity == 0 )
//	//		{
//
//	//			_g->stat.round_zero_set.udp.continuously_unsuccessful_select_on_open_port_count++;
//
//	//			int error = 0;
//	//			socklen_t errlen = sizeof( error );
//	//			getsockopt( sockfd_max , SOL_SOCKET , SO_ERROR , &error , &errlen );
//	//			if ( error != 0 )
//	//			{
//	//				_ECHO( "Socket error: %d\n" , error );
//	//			}
//
//	//			if ( ++input_udp_socket_error_tolerance_count > RETRY_UNEXPECTED_WAIT_FOR_SOCK() )
//	//			{
//	//				input_udp_socket_error_tolerance_count = 0;
//	//				for ( int i = 0 ; i < _g->bridges.ABhs_masks_count ; i++ )
//	//				{
//	//					if ( _g->bridges.ABhs_masks[ i ] )
//	//					{
//	//						if ( _g->bridges.ABs[ i ].single_AB->udp_connection_established )  // all the connected udp stoped or die so restart them
//	//						{
//	//							//if ( FD_ISSET( _g->bridges.ABs[ i ].single_AB->udp_sockfd , &readfds ) )
//	//							{
//	//								_g->bridges.ABs[ i ].single_AB->retry_to_connect_udp = 1;
//	//								break;
//	//							}
//	//						}
//	//					}
//	//				}
//	//			}
//
//	//			continue;
//	//		}
//
//	//		_g->stat.round_zero_set.udp.continuously_unsuccessful_select_on_open_port_count = 0;
//
//	//		if ( _g->stat.round_zero_set.t_begin.tv_sec == 0 && _g->stat.round_zero_set.t_begin.tv_usec == 0 )
//	//		{
//	//			gettimeofday( &_g->stat.round_zero_set.t_begin , NULL );
//	//		}
//
//	//		tnow = time( NULL );
//	//		// udp
//	//		if ( difftime( tnow , _g->stat.round_zero_set.udp_1_sec.t_udp_throughput ) >= 1.0 )
//	//		{
//	//			if ( _g->stat.round_zero_set.udp_1_sec.t_udp_throughput > 0 )
//	//			{
//	//				// add it here
//
//	//				cbuf_m_advance( &_g->stat.round_init_set.udp_stat_5_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
//	//				cbuf_m_advance( &_g->stat.round_init_set.udp_stat_5_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );
//	//				
//	//				cbuf_m_advance( &_g->stat.round_init_set.udp_stat_10_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
//	//				cbuf_m_advance( &_g->stat.round_init_set.udp_stat_10_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );
//	//				
//	//				cbuf_m_advance( &_g->stat.round_init_set.udp_stat_40_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
//	//				cbuf_m_advance( &_g->stat.round_init_set.udp_stat_40_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );
//	//				
//	//				cbuf_m_advance( &_g->stat.round_init_set.udp_stat_120_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
//	//				cbuf_m_advance( &_g->stat.round_init_set.udp_stat_120_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );
//
//	//				//_g->stat.round.udp_1_sec.udp_get_count_throughput = _g->stat.round.udp_1_sec.calc_throughput_udp_get_count;
//	//				//_g->stat.round.udp_1_sec.udp_get_byte_throughput = _g->stat.round.udp_1_sec.calc_throughput_udp_get_bytes;
//	//			}
//	//			_g->stat.round_zero_set.udp_1_sec.t_udp_throughput = tnow;
//	//			_g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count = 0;
//	//			_g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes = 0;
//	//		}
//	//		//if ( difftime( tnow , _g->stat.round.udp_10_sec.t_udp_throughput ) >= 10.0 )
//	//		//{
//	//		//	if ( _g->stat.round.udp_10_sec.t_udp_throughput > 0 )
//	//		//	{
//	//		//		_g->stat.round.udp_10_sec.udp_get_count_throughput = _g->stat.round.udp_10_sec.calc_throughput_udp_get_count;
//	//		//		_g->stat.round.udp_10_sec.udp_get_byte_throughput = _g->stat.round.udp_10_sec.calc_throughput_udp_get_bytes;
//	//		//	}
//	//		//	_g->stat.round.udp_10_sec.t_udp_throughput = tnow;
//	//		//	_g->stat.round.udp_10_sec.calc_throughput_udp_get_count = 0;
//	//		//	_g->stat.round.udp_10_sec.calc_throughput_udp_get_bytes = 0;
//	//		//}
//	//		//if ( difftime( tnow , _g->stat.round.udp_40_sec.t_udp_throughput ) >= 40.0 )
//	//		//{
//	//		//	if ( _g->stat.round.udp_40_sec.t_udp_throughput > 0 )
//	//		//	{
//	//		//		_g->stat.round.udp_40_sec.udp_get_count_throughput = _g->stat.round.udp_40_sec.calc_throughput_udp_get_count;
//	//		//		_g->stat.round.udp_40_sec.udp_get_byte_throughput = _g->stat.round.udp_40_sec.calc_throughput_udp_get_bytes;
//	//		//	}
//	//		//	_g->stat.round.udp_40_sec.t_udp_throughput = tnow;
//	//		//	_g->stat.round.udp_40_sec.calc_throughput_udp_get_count = 0;
//	//		//	_g->stat.round.udp_40_sec.calc_throughput_udp_get_bytes = 0;
//	//		//}
//
//
//	//		// tcp
//	//		if ( difftime( tnow , _g->stat.round_zero_set.tcp_1_sec.t_tcp_throughput ) >= 1.0 )
//	//		{
//	//			if ( _g->stat.round_zero_set.tcp_1_sec.t_tcp_throughput > 0 )
//	//			{
//	//				cbuf_m_advance( &_g->stat.round_init_set.tcp_stat_5_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
//	//				cbuf_m_advance( &_g->stat.round_init_set.tcp_stat_5_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );
//
//	//				cbuf_m_advance( &_g->stat.round_init_set.tcp_stat_10_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
//	//				cbuf_m_advance( &_g->stat.round_init_set.tcp_stat_10_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );
//
//	//				cbuf_m_advance( &_g->stat.round_init_set.tcp_stat_40_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
//	//				cbuf_m_advance( &_g->stat.round_init_set.tcp_stat_40_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );
//
//	//				cbuf_m_advance( &_g->stat.round_init_set.tcp_stat_120_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
//	//				cbuf_m_advance( &_g->stat.round_init_set.tcp_stat_120_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );
//
//	//				//_g->stat.round_zero_set.tcp_1_sec.tcp_put_count_throughput = _g->stat.round_zero_set.tcp_1_sec.calc_throughput_tcp_put_count;
//	//				//_g->stat.round_zero_set.tcp_1_sec.tcp_put_byte_throughput = _g->stat.round_zero_set.tcp_1_sec.calc_throughput_tcp_put_bytes;
//	//			}
//	//			_g->stat.round_zero_set.tcp_1_sec.t_tcp_throughput = tnow;
//	//			_g->stat.round_zero_set.tcp_1_sec.calc_throughput_tcp_put_count = 0;
//	//			_g->stat.round_zero_set.tcp_1_sec.calc_throughput_tcp_put_bytes = 0;
//	//		}
//	//		//if ( difftime( tnow , _g->stat.round.tcp_10_sec.t_tcp_throughput ) >= 10.0 )
//	//		//{
//	//		//	if ( _g->stat.round.tcp_10_sec.t_tcp_throughput > 0 )
//	//		//	{
//	//		//		_g->stat.round.tcp_10_sec.tcp_put_count_throughput = _g->stat.round.tcp_10_sec.calc_throughput_tcp_put_count;
//	//		//		_g->stat.round.tcp_10_sec.tcp_put_byte_throughput = _g->stat.round.tcp_10_sec.calc_throughput_tcp_put_bytes;
//	//		//	}
//	//		//	_g->stat.round.tcp_10_sec.t_tcp_throughput = tnow;
//	//		//	_g->stat.round.tcp_10_sec.calc_throughput_tcp_put_count = 0;
//	//		//	_g->stat.round.tcp_10_sec.calc_throughput_tcp_put_bytes = 0;
//	//		//}
//	//		//if ( difftime( tnow , _g->stat.round.tcp_40_sec.t_tcp_throughput ) >= 40.0 )
//	//		//{
//	//		//	if ( _g->stat.round.tcp_40_sec.t_tcp_throughput > 0 )
//	//		//	{
//	//		//		_g->stat.round.tcp_40_sec.tcp_put_count_throughput = _g->stat.round.tcp_40_sec.calc_throughput_tcp_put_count;
//	//		//		_g->stat.round.tcp_40_sec.tcp_put_byte_throughput = _g->stat.round.tcp_40_sec.calc_throughput_tcp_put_bytes;
//	//		//	}
//	//		//	_g->stat.round.tcp_40_sec.t_tcp_throughput = tnow;
//	//		//	_g->stat.round.tcp_40_sec.calc_throughput_tcp_put_count = 0;
//	//		//	_g->stat.round.tcp_40_sec.calc_throughput_tcp_put_bytes = 0;
//	//		//}
//
//	//		for ( int i = 0 ; i < _g->bridges.ABhs_masks_count ; i++ )
//	//		{
//	//			if ( _g->bridges.ABhs_masks[ i ] )
//	//			{
//	//				if ( _g->bridges.ABs[ i ].single_AB->udp_connection_established && _g->bridges.ABs[ i ].single_AB->tcp_connection_established )
//	//				{
//	//					if ( FD_ISSET( _g->bridges.ABs[ i ].single_AB->udp_sockfd , &readfds ) )
//	//					{
//	//						while( 1 )
//	//						{
//	//							bytes_received = recvfrom( _g->bridges.ABs[ i ].single_AB->udp_sockfd , buffer , BUFFER_SIZE , MSG_DONTWAIT , ( struct sockaddr * )&client_addr , &client_len ); // good for udp data recieve
//	//							if ( bytes_received < 0 )
//	//							{
//	//								if ( errno == EAGAIN || errno == EWOULDBLOCK )
//	//								{
//	//									// No more packets available
//	//									break;
//	//								}
//	//								else
//	//								{
//	//									_g->stat.round_zero_set.continuously_unsuccessful_receive_error++;
//	//									_g->stat.round_zero_set.total_unsuccessful_receive_error++;
//	//									break;
//	//								}
//	//							}
//	//							
//	//							if ( bytes_received <= 0 )
//	//							{
//	//								_g->stat.round_zero_set.continuously_unsuccessful_receive_error++;
//	//								_g->stat.round_zero_set.total_unsuccessful_receive_error++;
//	//								continue;
//	//							}
//	//							_g->stat.round_zero_set.continuously_unsuccessful_receive_error = 0;
//	//							//buffer[ bytes_received ] = '\0'; // Null-terminate the received data
//
//	//							_g->stat.round_zero_set.udp.total_udp_get_count++;
//	//							_g->stat.round_zero_set.udp.total_udp_get_byte += bytes_received;
//	//							_g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count++;
//	//							_g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes += bytes_received;
//	//							//_g->stat.round.udp_10_sec.calc_throughput_udp_get_count++;
//	//							//_g->stat.round.udp_10_sec.calc_throughput_udp_get_bytes += bytes_received;
//	//							//_g->stat.round.udp_40_sec.calc_throughput_udp_get_count++;
//	//							//_g->stat.round.udp_40_sec.calc_throughput_udp_get_bytes += bytes_received;
//
//	//							// Send data over TCP
//	//							if ( ( sz = send( _g->bridges.ABs[ i ].single_AB->tcp_sockfd , buffer , ( size_t )bytes_received , MSG_NOSIGNAL ) ) == -1 )
//	//							{
//	//								_g->stat.round_zero_set.continuously_unsuccessful_send_error++;
//	//								_g->stat.round_zero_set.total_unsuccessful_send_error++;
//
//	//								if ( ++output_tcp_socket_error_tolerance_count > RETRY_UNEXPECTED_WAIT_FOR_SOCK() )
//	//								{
//	//									output_tcp_socket_error_tolerance_count = 0;
//	//									for ( int i = 0 ; i < _g->bridges.ABhs_masks_count ; i++ )
//	//									{
//	//										if ( _g->bridges.ABhs_masks[ i ] )
//	//										{
//	//											if ( _g->bridges.ABs[ i ].single_AB->tcp_connection_established )  // all the connected udp stoped or die so restart them
//	//											{
//	//												//if ( FD_ISSET( _g->bridges.ABs[ i ].single_AB->udp_sockfd , &readfds ) )
//	//												{
//	//													_g->bridges.ABs[ i ].single_AB->retry_to_connect_tcp = 1;
//	//													break;
//	//												}
//	//											}
//	//										}
//	//									}
//	//								}
//
//	//								continue;
//	//							}
//	//							_g->stat.round_zero_set.continuously_unsuccessful_send_error = 0;
//
//	//							gettimeofday(&_g->stat.round_zero_set.t_end, NULL);
//
//	//							_g->stat.round_zero_set.tcp.total_tcp_put_count++;
//	//							_g->stat.round_zero_set.tcp.total_tcp_put_byte += sz;
//	//							_g->stat.round_zero_set.tcp_1_sec.calc_throughput_tcp_put_count++;
//	//							_g->stat.round_zero_set.tcp_1_sec.calc_throughput_tcp_put_bytes += sz;
//	//							//_g->stat.round.tcp_10_sec.calc_throughput_tcp_put_count++;
//	//							//_g->stat.round.tcp_10_sec.calc_throughput_tcp_put_bytes += sz;
//	//							//_g->stat.round.tcp_40_sec.calc_throughput_tcp_put_count++;
//	//							//_g->stat.round.tcp_40_sec.calc_throughput_tcp_put_bytes += sz;
//
//	//						}
//
//	//					}
//	//				}
//	//			}
//	//		}
//	//	}
//
//	//} while ( config_changes );
//	//BREAK_OK( 0 ); // to just ignore gcc warning
//
//	//BEGIN_RET
//	//	case 3: {}
//	//	case 2: {}
//	//	case 1:
//	//	{
//	//		//_close_socket( &src_pb->tcp_sockfd );
//	//		_g->stat.round_zero_set.syscal_err_count++;
//	//	}
//	//M_V_END_RET
//return NULL;
//}
//
//_THREAD_FXN void_p income_thread_proc( pass_p src_pb )
//{
//	INIT_BREAKABLE_FXN();
//
//	////AB * pb = ( AB * )src_pb;
//	//G * _g = ( G * )src_g;
//
//	//time_t tnow = 0;
//
//	//while ( !_g->bridges.thread_base.start_working )
//	//{
//	//	if ( _g->bridges.thread_base.do_close_thread )
//	//	{
//	//		break;
//	//	}
//	//	slep( 1 );
//	//}
//
//	//int input_udp_socket_error_tolerance_count = 0; // restart socket after many error accur
//
//	//int config_changes = 0;
//	//do
//	//{
//	//	if ( _g->bridges.thread_base.do_close_thread )
//	//	{
//	//		break;
//	//	}
//
//	//	config_changes = 0;
//
//	//	char buffer[ BUFFER_SIZE ]; // Define a buffer to store received data
//	//	struct sockaddr_in client_addr;
//	//	socklen_t client_len = sizeof( client_addr );
//	//	ssize_t bytes_received;
//
//	//	fd_set readfds; // Set of socket descriptors
//	//	FD_ZERO( &readfds );
//
//	//	ssize_t sz;
//
//	//	int sockfd_max = -1; // for select compulsion
//	//	for ( int i = 0 ; i < _g->bridges.ABhs_masks_count ; i++ )
//	//	{
//	//		if ( _g->bridges.ABhs_masks[ i ] )
//	//		{
//	//			if ( _g->bridges.ABs[ i ].single_AB->udp_connection_established /* && _g->bridges.ABs[i].single_AB->tcp_connection_established*/ )
//	//			{
//	//				FD_SET( _g->bridges.ABs[ i ].single_AB->udp_sockfd , &readfds );
//	//				if ( _g->bridges.ABs[ i ].single_AB->udp_sockfd > sockfd_max )
//	//				{
//	//					sockfd_max = _g->bridges.ABs[ i ].single_AB->udp_sockfd;
//	//				}
//	//			}
//	//		}
//	//	}
//	//	_g->bridges.under_listen_udp_sockets_group_changed = 0; // if any udp socket change then fdset must be reinitialized
//
//	//	if ( sockfd_max <= 1 )
//	//	{
//	//		slep( 1 );
//	//		continue;
//	//	}
//
//	//	while ( 1 )
//	//	{
//
//	//		//pthread_mutex_lock( &_g->sync.mutex );
//	//		//while ( _g->sync.lock_in_progress )
//	//		//{
//	//		//	////struct timespec ts = { 0, 10L };
//	//		//	////thrd_slep( &ts , NULL );
//	//		//	pthread_cond_wait( &_g->sync.cond , &_g->sync.mutex );
//	//		//}
//	//		//pthread_mutex_unlock( &_g->sync.mutex );
//	//		//if ( _g->sync.reset_static_after_lock )
//	//		//{
//	//		//	_g->sync.reset_static_after_lock = 0;
//	//		//	memset( &_g->stat.round , 0 , sizeof( _g->stat.round ) );
//	//		//}
//
//	//		if ( _g->bridges.thread_base.do_close_thread )
//	//		{
//	//			break;
//	//		}
//	//		if ( _g->bridges.under_listen_udp_sockets_group_changed )
//	//		{
//	//			config_changes = 1;
//	//			break;
//	//		}
//
//	//		struct timeval timeout; // Set timeout (e.g., 5 seconds)
//	//		timeout.tv_sec = 5;
//	//		timeout.tv_usec = 0;
//
//	//		// Wait for an activity on one of the sockets, timeout is NULL, so wait indefinitely
//	//		int activity = select( sockfd_max + 1 , &readfds , NULL , NULL , &timeout );
//
//	//		if ( ( activity < 0 ) /* && ( errno != EINTR )*/ )
//	//		{
//
//	//			_g->stat.round_zero_set.udp.continuously_unsuccessful_select_on_open_port_count++;
//
//	//			int error = 0;
//	//			socklen_t errlen = sizeof( error );
//	//			getsockopt( sockfd_max , SOL_SOCKET , SO_ERROR , &error , &errlen );
//	//			if ( error != 0 )
//	//			{
//	//				_ECHO( "Socket error: %d\n" , error );
//	//			}
//
//	//			if ( ++input_udp_socket_error_tolerance_count > RETRY_UNEXPECTED_WAIT_FOR_SOCK() )
//	//			{
//	//				input_udp_socket_error_tolerance_count = 0;
//	//				for ( int i = 0 ; i < _g->bridges.ABhs_masks_count ; i++ )
//	//				{
//	//					if ( _g->bridges.ABhs_masks[ i ] )
//	//					{
//	//						if ( _g->bridges.ABs[ i ].single_AB->udp_connection_established )  // all the connected udp stoped or die so restart them
//	//						{
//	//							//if ( FD_ISSET( _g->bridges.ABs[ i ].single_AB->udp_sockfd , &readfds ) )
//	//							{
//	//								_g->bridges.ABs[ i ].single_AB->retry_to_connect_udp = 1;
//	//								break;
//	//							}
//	//						}
//	//					}
//	//				}
//	//			}
//
//	//			continue;
//	//		}
//	//		if ( activity == 0 )
//	//		{
//
//	//			_g->stat.round_zero_set.udp.continuously_unsuccessful_select_on_open_port_count++;
//
//	//			int error = 0;
//	//			socklen_t errlen = sizeof( error );
//	//			getsockopt( sockfd_max , SOL_SOCKET , SO_ERROR , &error , &errlen );
//	//			if ( error != 0 )
//	//			{
//	//				_ECHO( "Socket error: %d\n" , error );
//	//			}
//
//	//			if ( ++input_udp_socket_error_tolerance_count > RETRY_UNEXPECTED_WAIT_FOR_SOCK() )
//	//			{
//	//				input_udp_socket_error_tolerance_count = 0;
//	//				for ( int i = 0 ; i < _g->bridges.ABhs_masks_count ; i++ )
//	//				{
//	//					if ( _g->bridges.ABhs_masks[ i ] )
//	//					{
//	//						if ( _g->bridges.ABs[ i ].single_AB->udp_connection_established )  // all the connected udp stoped or die so restart them
//	//						{
//	//							//if ( FD_ISSET( _g->bridges.ABs[ i ].single_AB->udp_sockfd , &readfds ) )
//	//							{
//	//								_g->bridges.ABs[ i ].single_AB->retry_to_connect_udp = 1;
//	//								break;
//	//							}
//	//						}
//	//					}
//	//				}
//	//			}
//
//	//			continue;
//	//		}
//
//	//		_g->stat.round_zero_set.udp.continuously_unsuccessful_select_on_open_port_count = 0;
//
//
//	//		tnow = time( NULL );
//	//		// udp
//	//		if ( difftime( tnow , _g->stat.round_zero_set.udp_1_sec.t_udp_throughput ) >= 1.0 )
//	//		{
//	//			if ( _g->stat.round_zero_set.udp_1_sec.t_udp_throughput > 0 )
//	//			{
//	//				cbuf_m_advance( &_g->stat.round_init_set.udp_stat_5_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
//	//				cbuf_m_advance( &_g->stat.round_init_set.udp_stat_5_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );
//
//	//				cbuf_m_advance( &_g->stat.round_init_set.udp_stat_10_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
//	//				cbuf_m_advance( &_g->stat.round_init_set.udp_stat_10_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );
//
//	//				cbuf_m_advance( &_g->stat.round_init_set.udp_stat_40_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
//	//				cbuf_m_advance( &_g->stat.round_init_set.udp_stat_40_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );
//
//	//				cbuf_m_advance( &_g->stat.round_init_set.udp_stat_120_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
//	//				cbuf_m_advance( &_g->stat.round_init_set.udp_stat_120_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );
//	//				//_g->stat.round.udp_1_sec.udp_get_count_throughput = _g->stat.round.udp_1_sec.calc_throughput_udp_get_count;
//	//				//_g->stat.round.udp_1_sec.udp_get_byte_throughput = _g->stat.round.udp_1_sec.calc_throughput_udp_get_bytes;
//	//			}
//	//			_g->stat.round_zero_set.udp_1_sec.t_udp_throughput = tnow;
//	//			_g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count = 0;
//	//			_g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes = 0;
//	//		}
//	//		//if ( difftime( tnow , _g->stat.round.udp_10_sec.t_udp_throughput ) >= 10.0 )
//	//		//{
//	//		//	if ( _g->stat.round.udp_10_sec.t_udp_throughput > 0 )
//	//		//	{
//	//		//		_g->stat.round.udp_10_sec.udp_get_count_throughput = _g->stat.round.udp_10_sec.calc_throughput_udp_get_count;
//	//		//		_g->stat.round.udp_10_sec.udp_get_byte_throughput = _g->stat.round.udp_10_sec.calc_throughput_udp_get_bytes;
//	//		//	}
//	//		//	_g->stat.round.udp_10_sec.t_udp_throughput = tnow;
//	//		//	_g->stat.round.udp_10_sec.calc_throughput_udp_get_count = 0;
//	//		//	_g->stat.round.udp_10_sec.calc_throughput_udp_get_bytes = 0;
//	//		//}
//	//		//if ( difftime( tnow , _g->stat.round.udp_40_sec.t_udp_throughput ) >= 40.0 )
//	//		//{
//	//		//	if ( _g->stat.round.udp_40_sec.t_udp_throughput > 0 )
//	//		//	{
//	//		//		_g->stat.round.udp_40_sec.udp_get_count_throughput = _g->stat.round.udp_40_sec.calc_throughput_udp_get_count;
//	//		//		_g->stat.round.udp_40_sec.udp_get_byte_throughput = _g->stat.round.udp_40_sec.calc_throughput_udp_get_bytes;
//	//		//	}
//	//		//	_g->stat.round.udp_40_sec.t_udp_throughput = tnow;
//	//		//	_g->stat.round.udp_40_sec.calc_throughput_udp_get_count = 0;
//	//		//	_g->stat.round.udp_40_sec.calc_throughput_udp_get_bytes = 0;
//	//		//}
//
//	//		for ( int i = 0 ; i < _g->bridges.ABhs_masks_count ; i++ )
//	//		{
//	//			if ( _g->bridges.ABhs_masks[ i ] )
//	//			{
//	//				if ( _g->bridges.ABs[ i ].single_AB->udp_connection_established /* && _g->bridges.ABs[i].single_AB->tcp_connection_established*/ )
//	//				{
//	//					if ( FD_ISSET( _g->bridges.ABs[ i ].single_AB->udp_sockfd , &readfds ) )
//	//					{
//	//						bytes_received = recvfrom( _g->bridges.ABs[ i ].single_AB->udp_sockfd , buffer , BUFFER_SIZE , MSG_WAITALL , ( struct sockaddr * )&client_addr , &client_len ); // good for udp data recieve
//	//						if ( bytes_received <= 0 )
//	//						{
//	//							_g->stat.round_zero_set.continuously_unsuccessful_receive_error++;
//	//							_g->stat.round_zero_set.total_unsuccessful_receive_error++;
//	//							continue;
//	//						}
//	//						_g->stat.round_zero_set.continuously_unsuccessful_receive_error = 0;
//	//						//buffer[ bytes_received ] = '\0'; // Null-terminate the received data
//
//	//						_g->stat.round_zero_set.udp.total_udp_get_count++;
//	//						_g->stat.round_zero_set.udp.total_udp_get_byte += bytes_received;
//	//						_g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count++;
//	//						_g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes += bytes_received;
//	//						//_g->stat.round.udp_10_sec.calc_throughput_udp_get_count++;
//	//						//_g->stat.round.udp_10_sec.calc_throughput_udp_get_bytes += bytes_received;
//	//						//_g->stat.round.udp_40_sec.calc_throughput_udp_get_count++;
//	//						//_g->stat.round.udp_40_sec.calc_throughput_udp_get_bytes += bytes_received;
//
//	//						queue_push( &_g->bridges.bidirection_thread->queue , buffer , bytes_received );
//	//					}
//	//				}
//	//			}
//	//		}
//	//	}
//
//	//} while ( config_changes );
//	//BREAK_OK( 0 ); // to just ignore gcc warning
//
//	//BEGIN_RET
//	//	case 3: ;
//	//	case 2: ;
//	//	case 1:
//	//	{
//	//		//_close_socket( &src_pb->tcp_sockfd );
//	//		_g->stat.round_zero_set.syscal_err_count++;
//	//	}
//	//M_V_END_RET
//return NULL;
////}
//
//
//
//
//////_THREAD_FXN void_p protocol_bridge_runner( pass_p src_pb )
////{
////	//int try_to_connect_udp_port = 1; // for the first time
////	//int try_to_connect_tcp_port = 1; // for the first time
////
////	//pthread->base_config_change_applied = 0;
////	//do
////	//{
////	//	if ( pthread->do_close_thread )
////	//	{
////	//		break;
////	//	}
////
////	//	if ( pthread->base_config_change_applied ) // config change cause reconnect
////	//	{
////	//		pthread->base_config_change_applied = 0;
////	//		try_to_connect_udp_port = 1;
////	//		try_to_connect_tcp_port = 1;
////	//	}
////	//	if ( pb->retry_to_connect_udp ) // retry from socket err cause reconnect
////	//	{
////	//		pb->retry_to_connect_udp = 0;
////	//		try_to_connect_udp_port = 1;
////	//	}
////	//	if ( pb->retry_to_connect_tcp ) // retry from socket err cause reconnect
////	//	{
////	//		pb->retry_to_connect_tcp = 0;
////	//		try_to_connect_tcp_port = 1;
////	//	}
////	//	if ( !try_to_connect_udp_port && !try_to_connect_tcp_port )
////	//	{
////	//		slep(2);
////	//		continue;
////	//	}
////	//	int tmp_try_to_connect_udp_port = try_to_connect_udp_port;
////	//	int tmp_try_to_connect_tcp_port = try_to_connect_tcp_port;
////	//	try_to_connect_udp_port = 0;
////	//	try_to_connect_tcp_port = 0;
////
////	//	if ( _g->appcfg.g_cfg->c.c.atht == buttleneck || _g->appcfg.g_cfg->c.c.atht == bidirection )
////	//	{
////	//		// first close then reconnect
////	//		if ( tmp_try_to_connect_udp_port )
////	//		{
////	//			pthread_t trd_udp_connection;
////	//			MM_BREAK_IF( pthread_create( &trd_udp_connection , NULL , connect_udps_proc , pb ) != PTHREAD_CREATE_OK , errGeneral , 0 , "thread creation failed" );
////	//		}
////	//		if ( tmp_try_to_connect_tcp_port )
////	//		{
////	//			pthread_t trd_tcp_connection;
////	//			MM_BREAK_IF( pthread_create( &trd_tcp_connection , NULL , thread_tcp_connection_proc , pb ) != PTHREAD_CREATE_OK , errGeneral , 0 , "thread creation failed" );
////	//		}
////	//	}
////	//	else if ( _g->appcfg.g_cfg->c.c.atht == justIncoming )
////	//	{
////	//		if ( tmp_try_to_connect_udp_port )
////	//		{
////	//			pthread_t trd_udp_connection;
////	//			MM_BREAK_IF( pthread_create( &trd_udp_connection , NULL , connect_udps_proc , pb ) != PTHREAD_CREATE_OK , errGeneral , 0 , "thread creation failed" );
////	//		}
////	//	}
////	//	
////
////	//} while ( 1 );
////
////
////	////// delete mask
////	//for ( int i = 0 ; i < pthread->pb->pb_trds_masks_count ; i++ )
////	//{
////	//	if ( pthread->pb->pb_trds_masks[ i ] && pthread->pb->pb_trds[ i ].alc_thread->trd_id == tid )
////	//	{
////	//		pthread->pb->pb_trds_masks[ i ] = 0;
////	//		break;
////	//	}
////	//}
////
////	//
////	//BEGIN_RET
////	//	case 3: ;
////	//	case 2: ;
////	//	case 1:
////	//	{
////	//		//_close_socket( &src_pb->tcp_sockfd );
////	//		_g->stat.round_zero_set.syscal_err_count++;
////	//	}
////	//M_V_END_RET
////
////	return NULL;
////}
