//#define Uses_dict_t
#define Uses_one2many_pcap2kernelDefaultStack_SF_Mix_RR_Replicate_pcap_udp_income_thread_proc
#define Uses_MALLOC_AR
#define Uses_BT_one2one_pcap2kernelDefaultStack_SF
#define Uses_kernel_default_stack_udp_counter_thread_proc
#define Uses_pcap_udp_counter_thread_proc
#define Uses_INIT_BREAKABLE_FXN
//#define Uses_TWD
#define Uses_pthread_t
#define Uses_Bridge
#define Uses_helper

//#define DIRECT_ECHO_BUF _g->stat.last_command // just before include dep
#include <Protocol_Bridge.dep>

//_THREAD_FXN void_p bottleneck_thread_proc( void_p src_pb )
//{
//	INIT_BREAKABLE_FXN();
//	static TWD twd = { 0 };
//	if ( twd.threadId == 0 )
//	{
//		twd.threadId = pthread_self();
//		twd.cal = bottleneck_thread_proc; // self function address
//		twd.callback_arg = src_pb;
//	}
//	if ( src_pb == NULL )
//	{
//		return ( void_p )&twd;
//	}
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
//_THREAD_FXN void_p income_thread_proc( void_p src_pb )
//{
//	INIT_BREAKABLE_FXN();
//	static TWD twd = { 0 };
//	if ( twd.threadId == 0 )
//	{
//		twd.threadId = pthread_self();
//		twd.cal = income_thread_proc; // self function address
//		twd.callback_arg = src_pb;
//	}
//	if ( src_pb == NULL )
//	{
//		return ( void_p )&twd;
//	}
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
//////_THREAD_FXN void_p protocol_bridge_runner( void_p src_pb )
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

/// <summary>
/// init active udp tcp structure by the bridge config
/// </summary>
/// <param name="_g"></param>
/// <param name="pb"></param>
void init_ActiveBridge( G * _g , AB * pb )
{
	INIT_BREAKABLE_FXN();

	if ( pb->cpy_cfg.m.m.maintained.in_count > 0 )
	{
		M_BREAK_IF( !( pb->udps = MALLOC_AR( pb->udps , pb->cpy_cfg.m.m.maintained.in_count ) ) , errMemoryLow , 1 );
		MEMSET_ZERO( pb->udps , pb->cpy_cfg.m.m.maintained.in_count );
		pb->udps_count = pb->cpy_cfg.m.m.maintained.in_count;

		for ( int i = 0 ; i < pb->udps_count ; i++ )
		{
			pb->udps[ i ].owner_pb = pb;
			pb->udps[ i ].__udp_cfg = &pb->cpy_cfg.m.m.maintained.in[ i ].data;
		}
	}
	if ( pb->cpy_cfg.m.m.maintained.out_count > 0 )
	{
		M_BREAK_IF( !( pb->tcps = MALLOC_AR( pb->tcps , pb->cpy_cfg.m.m.maintained.out_count ) ) , errMemoryLow , 1 );
		MEMSET_ZERO( pb->tcps , pb->cpy_cfg.m.m.maintained.out_count );
		pb->tcps_count = pb->cpy_cfg.m.m.maintained.out_count;

		for ( int i = 0 ; i < pb->tcps_count ; i++ )
		{
			pb->tcps[ i ].owner_pb = pb;
			pb->tcps[ i ].__tcp_cfg = &pb->cpy_cfg.m.m.maintained.out[ i ].data;
		}
	}

	BEGIN_RET // TODO . complete reverse on error
	case 3: ;
	case 2: ;
	case 1:
	{
		_g->stat.round_zero_set.syscal_err_count++;
	}
	M_V_END_RET
}

vcbuf_nb * ppp;

void mk_hlpr0( _IN AB * pb , _OUT abhelp * hlpr )
{
	ASSERT( pb && hlpr );
	hlpr->pab = pb;
	hlpr->in_count = &pb->cpy_cfg.m.m.maintained.in_count;
	hlpr->out_count = &pb->cpy_cfg.m.m.maintained.out_count;
	hlpr->thread_is_created = &pb->trd.base.thread_is_created;
	hlpr->do_close_thread = &pb->trd.base.do_close_thread;
	hlpr->creation_thread_race_cond = &pb->trd.base.creation_thread_race_cond;
	hlpr->bridg_prerequisite_stabled = &pb->trd.base.bridg_prerequisite_stabled;
	hlpr->buf_psh_distri = &pb->trd.base.buffer_push_distributor;
}

void apply_new_protocol_bridge_config( G * _g , AB * pb , Bcfg * new_ccfg )
{
	INIT_BREAKABLE_FXN();

	//if ( !new_ccfg->m.m.maintained.enable )
	//{
	//	for ( int i = 0 ; i < pb->pb_trds_masks_count ; i++ )
	//	{
	//		if ( pb->pb_trds_masks[ i ] )
	//		{
	//			pb->pb_trds->alc_thread->do_close_thread = 1;
	//		}
	//	}
	//	return;
	//}

	// when we arrive at this point we sure that somethings is changed
	new_ccfg->m.m.temp_data.pcfg_changed = 0; // say to config that change applied to bridge
	
	// each thread action switched here

	if ( iSTR_SAME( pb->cpy_cfg.m.m.id.thread_handler_act , "pcap_udp_counter" ) )
	{
		if ( !pb->trd.t.p_pcap_udp_counter_thread )
		{
			init_ActiveBridge( _g , pb );

			M_BREAK_IF( !( pb->trd.t.p_pcap_udp_counter_thread = MALLOC_ONE( pb->trd.t.p_pcap_udp_counter_thread ) ) , errMemoryLow , 1 );
			MEMSET_ZERO_O( pb->trd.t.p_pcap_udp_counter_thread );
			pthread_mutex_init( &pb->trd.base.creation_thread_race_cond , NULL );
			//pthread_mutex_init( &pb->trd.base.do_all_prerequisite_stablished_race_cond , NULL );

			pthread_mutex_lock( &pb->trd.base.creation_thread_race_cond );
			if ( !pb->trd.base.thread_is_created )
			{
				MM_BREAK_IF( pthread_create( &pb->trd.t.p_pcap_udp_counter_thread->trd_id , NULL , pcap_udp_counter_thread_proc , pb ) != PTHREAD_CREATE_OK , errGeneral , 0 , "thread creation failed" );
				pb->trd.base.thread_is_created = 1;
			}
			pthread_mutex_unlock( &pb->trd.base.creation_thread_race_cond );
		}
	}

	else if ( iSTR_SAME( pb->cpy_cfg.m.m.id.thread_handler_act , "kernel_default_stack_udp_counter" ) )
	{
		if ( !pb->trd.t.p_udp_counter_thread )
		{
			init_ActiveBridge( _g , pb );

			M_BREAK_IF( !( pb->trd.t.p_udp_counter_thread = MALLOC_ONE( pb->trd.t.p_udp_counter_thread ) ) , errMemoryLow , 1 );
			MEMSET_ZERO_O( pb->trd.t.p_udp_counter_thread );
			pthread_mutex_init( &pb->trd.base.creation_thread_race_cond , NULL );
			//pthread_mutex_init( &pb->trd.base.do_all_prerequisite_stablished_race_cond , NULL );

			pthread_mutex_lock( &pb->trd.base.creation_thread_race_cond );
			if ( !pb->trd.base.thread_is_created )
			{
				MM_BREAK_IF( pthread_create( &pb->trd.t.p_udp_counter_thread->trd_id , NULL , kernel_default_stack_udp_counter_thread_proc , pb ) != PTHREAD_CREATE_OK , errGeneral , 0 , "thread creation failed" );
				pb->trd.base.thread_is_created = 1;
			}
			pthread_mutex_unlock( &pb->trd.base.creation_thread_race_cond );

			pthread_t trd_udp_connection;
			MM_BREAK_IF( pthread_create( &trd_udp_connection , NULL , connect_udps_proc , pb ) != PTHREAD_CREATE_OK , errGeneral , 0 , "thread creation failed" );
		}
	}

	else if ( iSTR_SAME( pb->cpy_cfg.m.m.id.thread_handler_act , "one2one_pcap2kernelDefaultStack_S&F" ) )
	{
		if ( !pb->trd.t.p_one2one_pcap2kernelDefaultStack_SF_thread )
		{
			init_ActiveBridge( _g , pb );

			M_BREAK_IF( !( pb->trd.t.p_one2one_pcap2kernelDefaultStack_SF_thread = MALLOC_ONE( pb->trd.t.p_one2one_pcap2kernelDefaultStack_SF_thread ) ) , errMemoryLow , 1 );
			MEMSET_ZERO_O( pb->trd.t.p_one2one_pcap2kernelDefaultStack_SF_thread );
			pthread_mutex_init( &pb->trd.base.creation_thread_race_cond , NULL );
			//pthread_mutex_init( &pb->trd.base.do_all_prerequisite_stablished_race_cond , NULL );

			M_BREAK_STAT( vcbuf_nb_init( &pb->trd.t.p_one2one_pcap2kernelDefaultStack_SF_thread->cbuf , 100000 , /*1470 -> + hdr = 1512*/1512 ) , 1 );

			ppp = &pb->trd.t.p_one2one_pcap2kernelDefaultStack_SF_thread->cbuf; // TEMP

			pthread_mutex_lock( &pb->trd.base.creation_thread_race_cond );
			if ( !pb->trd.base.thread_is_created )
			{
				MM_BREAK_IF( pthread_create( &pb->trd.t.p_one2one_pcap2kernelDefaultStack_SF_thread->income_trd_id , NULL ,
					one2one_pcap2kernelDefaultStack_SF_pcap_udp_income_thread_proc , pb ) != PTHREAD_CREATE_OK , errGeneral , 0 , "thread creation failed" );
				MM_BREAK_IF( pthread_create( &pb->trd.t.p_one2one_pcap2kernelDefaultStack_SF_thread->outgoing_trd_id , NULL ,
					one2one_pcap2kernelDefaultStack_SF_one_tcp_out_thread_proc , pb ) != PTHREAD_CREATE_OK , errGeneral , 0 , "thread creation failed" );
				pb->trd.base.thread_is_created = 1;
			}
			pthread_mutex_unlock( &pb->trd.base.creation_thread_race_cond );

			pthread_t trd_tcp_connection;
			MM_BREAK_IF( pthread_create( &trd_tcp_connection , NULL , thread_tcp_connection_proc , pb ) != PTHREAD_CREATE_OK , errGeneral , 0 , "thread creation failed" );
		}
	}

	else if ( iSTR_SAME( pb->cpy_cfg.m.m.id.thread_handler_act , "one2many_pcap2kernelDefaultStack_S&F_Mix_RR_Replicate" ) )
	{
		if ( !pb->trd.t.p_one2many_pcap2kernelDefaultStack_SF_Mix_RR_Replicate )
		{
			init_ActiveBridge( _g , pb );

			M_BREAK_IF( !( pb->trd.t.p_one2many_pcap2kernelDefaultStack_SF_Mix_RR_Replicate = MALLOC_ONE( pb->trd.t.p_one2many_pcap2kernelDefaultStack_SF_Mix_RR_Replicate ) ) , errMemoryLow , 1 );
			MEMSET_ZERO_O( pb->trd.t.p_one2many_pcap2kernelDefaultStack_SF_Mix_RR_Replicate );
			pthread_mutex_init( &pb->trd.base.creation_thread_race_cond , NULL );
			//pthread_mutex_init( &pb->trd.base.do_all_prerequisite_stablished_race_cond , NULL );

			M_BREAK_STAT( vcbuf_nb_init( &pb->trd.t.p_one2many_pcap2kernelDefaultStack_SF_Mix_RR_Replicate->cbuf , 100000 , /*1470 -> + hdr = 1512*/1512 ) , 1 );

			ppp = &pb->trd.t.p_one2many_pcap2kernelDefaultStack_SF_Mix_RR_Replicate->cbuf; // TEMP . remove later

			pthread_mutex_lock( &pb->trd.base.creation_thread_race_cond );
			if ( !pb->trd.base.thread_is_created )
			{
				MM_BREAK_IF( pthread_create( &pb->trd.t.p_one2many_pcap2kernelDefaultStack_SF_Mix_RR_Replicate->income_trd_id , NULL ,
					one2many_pcap2kernelDefaultStack_SF_Mix_RR_Replicate_pcap_udp_income_thread_proc , pb ) != PTHREAD_CREATE_OK , errGeneral , 0 , "thread creation failed" );
				MM_BREAK_IF( pthread_create( &pb->trd.t.p_one2many_pcap2kernelDefaultStack_SF_Mix_RR_Replicate->outgoing_trd_id , NULL ,
					one2many_pcap2kernelDefaultStack_SF_Mix_RR_Replicate_many_tcp_out_thread_proc , pb ) != PTHREAD_CREATE_OK , errGeneral , 0 , "thread creation failed" );
				pb->trd.base.thread_is_created = 1;
			}
			pthread_mutex_unlock( &pb->trd.base.creation_thread_race_cond );

			pthread_t trd_tcp_connection;
			MM_BREAK_IF( pthread_create( &trd_tcp_connection , NULL , thread_tcp_connection_proc , pb ) != PTHREAD_CREATE_OK , errGeneral , 0 , "thread creation failed" );
		}
	}

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
	

	BEGIN_RET // TODO . complete reverse on error
	case 3: ;
	case 2: ;
	case 1:
	{
		_g->stat.round_zero_set.syscal_err_count++;
	}
	M_V_END_RET
}

