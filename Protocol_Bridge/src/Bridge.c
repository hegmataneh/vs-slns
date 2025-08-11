#define Uses_INIT_BREAKABLE_FXN
#define Uses_TWD
#define Uses_pthread_t
#define Uses_Bridge
//#define Uses_helper

//#define DIRECT_ECHO_BUF _g->stat.last_command // just before include dep
#include <Protocol_Bridge.dep>

#ifndef bottleneck_in_input_output


_THREAD_FXN void * bottleneck_thread_proc( void * src_g )
{
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
//		return ( void * )&twd;
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
//	//	sleep( 1 );
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
//	//			if ( _g->bridges.ABhs[ i ].single_AB->udp_connection_established && _g->bridges.ABhs[ i ].single_AB->tcp_connection_established )
//	//			{
//	//				FD_SET( _g->bridges.ABhs[ i ].single_AB->udp_sockfd , &readfds );
//	//				if ( _g->bridges.ABhs[ i ].single_AB->udp_sockfd > sockfd_max )
//	//				{
//	//					sockfd_max = _g->bridges.ABhs[ i ].single_AB->udp_sockfd;
//	//				}
//	//			}
//	//		}
//	//	}
//	//	_g->bridges.under_listen_udp_sockets_group_changed = 0; // if any udp socket change then fdset must be reinitialized
//
//	//	if ( sockfd_max <= 1 )
//	//	{
//	//		sleep( 1 );
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
//	//		//	////thrd_sleep( &ts , NULL );
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
//	//		int activity = select( sockfd_max + 1 , &readfds , NULL , NULL , _g->appcfg._general_config->c.c.time_out_sec > 0 ? &timeout : NULL );
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
//	//						if ( _g->bridges.ABhs[ i ].single_AB->udp_connection_established )  // all the connected udp stoped or die so restart them
//	//						{
//	//							//if ( FD_ISSET( _g->bridges.ABhs[ i ].single_AB->udp_sockfd , &readfds ) )
//	//							{
//	//								_g->bridges.ABhs[ i ].single_AB->retry_to_connect_udp = 1;
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
//	//						if ( _g->bridges.ABhs[ i ].single_AB->udp_connection_established )  // all the connected udp stoped or die so restart them
//	//						{
//	//							//if ( FD_ISSET( _g->bridges.ABhs[ i ].single_AB->udp_sockfd , &readfds ) )
//	//							{
//	//								_g->bridges.ABhs[ i ].single_AB->retry_to_connect_udp = 1;
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
//	//				circbuf_advance( &_g->stat.round_init_set.udp_stat_5_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
//	//				circbuf_advance( &_g->stat.round_init_set.udp_stat_5_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );
//	//				
//	//				circbuf_advance( &_g->stat.round_init_set.udp_stat_10_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
//	//				circbuf_advance( &_g->stat.round_init_set.udp_stat_10_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );
//	//				
//	//				circbuf_advance( &_g->stat.round_init_set.udp_stat_40_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
//	//				circbuf_advance( &_g->stat.round_init_set.udp_stat_40_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );
//	//				
//	//				circbuf_advance( &_g->stat.round_init_set.udp_stat_120_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
//	//				circbuf_advance( &_g->stat.round_init_set.udp_stat_120_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );
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
//	//				circbuf_advance( &_g->stat.round_init_set.tcp_stat_5_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
//	//				circbuf_advance( &_g->stat.round_init_set.tcp_stat_5_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );
//
//	//				circbuf_advance( &_g->stat.round_init_set.tcp_stat_10_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
//	//				circbuf_advance( &_g->stat.round_init_set.tcp_stat_10_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );
//
//	//				circbuf_advance( &_g->stat.round_init_set.tcp_stat_40_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
//	//				circbuf_advance( &_g->stat.round_init_set.tcp_stat_40_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );
//
//	//				circbuf_advance( &_g->stat.round_init_set.tcp_stat_120_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
//	//				circbuf_advance( &_g->stat.round_init_set.tcp_stat_120_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );
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
//	//				if ( _g->bridges.ABhs[ i ].single_AB->udp_connection_established && _g->bridges.ABhs[ i ].single_AB->tcp_connection_established )
//	//				{
//	//					if ( FD_ISSET( _g->bridges.ABhs[ i ].single_AB->udp_sockfd , &readfds ) )
//	//					{
//	//						while( 1 )
//	//						{
//	//							bytes_received = recvfrom( _g->bridges.ABhs[ i ].single_AB->udp_sockfd , buffer , BUFFER_SIZE , MSG_DONTWAIT , ( struct sockaddr * )&client_addr , &client_len ); // good for udp data recieve
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
//	//							if ( ( sz = send( _g->bridges.ABhs[ i ].single_AB->tcp_sockfd , buffer , ( size_t )bytes_received , MSG_NOSIGNAL ) ) == -1 )
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
//	//											if ( _g->bridges.ABhs[ i ].single_AB->tcp_connection_established )  // all the connected udp stoped or die so restart them
//	//											{
//	//												//if ( FD_ISSET( _g->bridges.ABhs[ i ].single_AB->udp_sockfd , &readfds ) )
//	//												{
//	//													_g->bridges.ABhs[ i ].single_AB->retry_to_connect_tcp = 1;
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

return NULL;
}

#endif

#ifndef seperate_thread_for_input_output

_THREAD_FXN void * income_thread_proc( void * src_g )
{
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
//		return ( void * )&twd;
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
//	//	sleep( 1 );
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
//	//			if ( _g->bridges.ABhs[ i ].single_AB->udp_connection_established /* && _g->bridges.ABhs[i].single_AB->tcp_connection_established*/ )
//	//			{
//	//				FD_SET( _g->bridges.ABhs[ i ].single_AB->udp_sockfd , &readfds );
//	//				if ( _g->bridges.ABhs[ i ].single_AB->udp_sockfd > sockfd_max )
//	//				{
//	//					sockfd_max = _g->bridges.ABhs[ i ].single_AB->udp_sockfd;
//	//				}
//	//			}
//	//		}
//	//	}
//	//	_g->bridges.under_listen_udp_sockets_group_changed = 0; // if any udp socket change then fdset must be reinitialized
//
//	//	if ( sockfd_max <= 1 )
//	//	{
//	//		sleep( 1 );
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
//	//		//	////thrd_sleep( &ts , NULL );
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
//	//						if ( _g->bridges.ABhs[ i ].single_AB->udp_connection_established )  // all the connected udp stoped or die so restart them
//	//						{
//	//							//if ( FD_ISSET( _g->bridges.ABhs[ i ].single_AB->udp_sockfd , &readfds ) )
//	//							{
//	//								_g->bridges.ABhs[ i ].single_AB->retry_to_connect_udp = 1;
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
//	//						if ( _g->bridges.ABhs[ i ].single_AB->udp_connection_established )  // all the connected udp stoped or die so restart them
//	//						{
//	//							//if ( FD_ISSET( _g->bridges.ABhs[ i ].single_AB->udp_sockfd , &readfds ) )
//	//							{
//	//								_g->bridges.ABhs[ i ].single_AB->retry_to_connect_udp = 1;
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
//	//				circbuf_advance( &_g->stat.round_init_set.udp_stat_5_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
//	//				circbuf_advance( &_g->stat.round_init_set.udp_stat_5_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );
//
//	//				circbuf_advance( &_g->stat.round_init_set.udp_stat_10_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
//	//				circbuf_advance( &_g->stat.round_init_set.udp_stat_10_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );
//
//	//				circbuf_advance( &_g->stat.round_init_set.udp_stat_40_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
//	//				circbuf_advance( &_g->stat.round_init_set.udp_stat_40_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );
//
//	//				circbuf_advance( &_g->stat.round_init_set.udp_stat_120_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
//	//				circbuf_advance( &_g->stat.round_init_set.udp_stat_120_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );
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
//	//				if ( _g->bridges.ABhs[ i ].single_AB->udp_connection_established /* && _g->bridges.ABhs[i].single_AB->tcp_connection_established*/ )
//	//				{
//	//					if ( FD_ISSET( _g->bridges.ABhs[ i ].single_AB->udp_sockfd , &readfds ) )
//	//					{
//	//						bytes_received = recvfrom( _g->bridges.ABhs[ i ].single_AB->udp_sockfd , buffer , BUFFER_SIZE , MSG_WAITALL , ( struct sockaddr * )&client_addr , &client_len ); // good for udp data recieve
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
//	//	case 3: {}
//	//	case 2: {}
//	//	case 1:
//	//	{
//	//		//_close_socket( &src_pb->tcp_sockfd );
//	//		_g->stat.round_zero_set.syscal_err_count++;
//	//	}
//	//M_V_END_RET

return NULL;
}

_THREAD_FXN void * outgoing_thread_proc( void * src_g )
{
//	INIT_BREAKABLE_FXN();
//	static TWD twd = { 0 };
//	if ( twd.threadId == 0 )
//	{
//		twd.threadId = pthread_self();
//		twd.cal = outgoing_thread_proc; // self function address
//		twd.callback_arg = src_pb;
//	}
//	if ( src_pb == NULL )
//	{
//		return ( void * )&twd;
//	}
//
//	////AB * pb = ( AB * )src_pb;
//	//G * _g = ( G * )src_g;
//
//	//time_t tnow = 0;
//	//char buffer[ BUFFER_SIZE ]; // Define a buffer to store received data
//	//size_t sz;
//	//ssize_t snd_ret;
//
//	//int output_tcp_socket_error_tolerance_count = 0; // restart socket after many error accur
//
//	//while( !queue_peek_available( &_g->bridges.bidirection_thread->queue ) )
//	//{
//	//	sleep(1);
//	//}
//
//	//while ( 1 )
//	//{
//
//	//	//pthread_mutex_lock( &_g->sync.mutex );
//	//	//while ( _g->sync.lock_in_progress )
//	//	//{
//	//	//	////struct timespec ts = { 0, 10L };
//	//	//	////thrd_sleep( &ts , NULL );
//	//	//	pthread_cond_wait( &_g->sync.cond , &_g->sync.mutex );
//	//	//}
//	//	//pthread_mutex_unlock( &_g->sync.mutex );
//	//	//if ( _g->sync.reset_static_after_lock )
//	//	//{
//	//	//	_g->sync.reset_static_after_lock = 0;
//	//	//	memset( &_g->stat.round , 0 , sizeof( _g->stat.round ) );
//	//	//}
//
//	//	if ( _g->bridges.thread_base.do_close_thread )
//	//	{
//	//		break;
//	//	}
//
//	//	tnow = time( NULL );
//	//	// tcp
//	//	if ( difftime( tnow , _g->stat.round_zero_set.tcp_1_sec.t_tcp_throughput ) >= 1.0 )
//	//	{
//	//		if ( _g->stat.round_zero_set.tcp_1_sec.t_tcp_throughput > 0 )
//	//		{
//	//			circbuf_advance( &_g->stat.round_init_set.tcp_stat_5_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
//	//			circbuf_advance( &_g->stat.round_init_set.tcp_stat_5_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );
//
//	//			circbuf_advance( &_g->stat.round_init_set.tcp_stat_10_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
//	//			circbuf_advance( &_g->stat.round_init_set.tcp_stat_10_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );
//
//	//			circbuf_advance( &_g->stat.round_init_set.tcp_stat_40_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
//	//			circbuf_advance( &_g->stat.round_init_set.tcp_stat_40_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );
//
//	//			circbuf_advance( &_g->stat.round_init_set.tcp_stat_120_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
//	//			circbuf_advance( &_g->stat.round_init_set.tcp_stat_120_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );
//
//	//			//_g->stat.round.tcp_1_sec.tcp_put_count_throughput = _g->stat.round.tcp_1_sec.calc_throughput_tcp_put_count;
//	//			//_g->stat.round.tcp_1_sec.tcp_put_byte_throughput = _g->stat.round.tcp_1_sec.calc_throughput_tcp_put_bytes;
//	//		}
//	//		_g->stat.round_zero_set.tcp_1_sec.t_tcp_throughput = tnow;
//	//		_g->stat.round_zero_set.tcp_1_sec.calc_throughput_tcp_put_count = 0;
//	//		_g->stat.round_zero_set.tcp_1_sec.calc_throughput_tcp_put_bytes = 0;
//	//	}
//	//	//if ( difftime( tnow , _g->stat.round.tcp_10_sec.t_tcp_throughput ) >= 10.0 )
//	//	//{
//	//	//	if ( _g->stat.round.tcp_10_sec.t_tcp_throughput > 0 )
//	//	//	{
//	//	//		_g->stat.round.tcp_10_sec.tcp_put_count_throughput = _g->stat.round.tcp_10_sec.calc_throughput_tcp_put_count;
//	//	//		_g->stat.round.tcp_10_sec.tcp_put_byte_throughput = _g->stat.round.tcp_10_sec.calc_throughput_tcp_put_bytes;
//	//	//	}
//	//	//	_g->stat.round.tcp_10_sec.t_tcp_throughput = tnow;
//	//	//	_g->stat.round.tcp_10_sec.calc_throughput_tcp_put_count = 0;
//	//	//	_g->stat.round.tcp_10_sec.calc_throughput_tcp_put_bytes = 0;
//	//	//}
//	//	//if ( difftime( tnow , _g->stat.round.tcp_40_sec.t_tcp_throughput ) >= 40.0 )
//	//	//{
//	//	//	if ( _g->stat.round.tcp_40_sec.t_tcp_throughput > 0 )
//	//	//	{
//	//	//		_g->stat.round.tcp_40_sec.tcp_put_count_throughput = _g->stat.round.tcp_40_sec.calc_throughput_tcp_put_count;
//	//	//		_g->stat.round.tcp_40_sec.tcp_put_byte_throughput = _g->stat.round.tcp_40_sec.calc_throughput_tcp_put_bytes;
//	//	//	}
//	//	//	_g->stat.round.tcp_40_sec.t_tcp_throughput = tnow;
//	//	//	_g->stat.round.tcp_40_sec.calc_throughput_tcp_put_count = 0;
//	//	//	_g->stat.round.tcp_40_sec.calc_throughput_tcp_put_bytes = 0;
//	//	//}
//
//	//	queue_pop( &_g->bridges.bidirection_thread->queue , buffer , &sz );
//	//	
//	//	for ( int i = 0 ; i < _g->bridges.ABhs_masks_count ; i++ )
//	//	{
//	//		if ( _g->bridges.ABhs_masks[ i ] && _g->bridges.ABhs[ i ].single_AB->tcp_connection_established )
//	//		{
//	//			if ( ( snd_ret = send( _g->bridges.ABhs[ i ].single_AB->tcp_sockfd , buffer , ( size_t )sz , MSG_NOSIGNAL ) ) == -1 )
//	//			{
//	//				_g->stat.round_zero_set.continuously_unsuccessful_send_error++;
//	//				_g->stat.round_zero_set.total_unsuccessful_send_error++;
//
//	//				if ( ++output_tcp_socket_error_tolerance_count > RETRY_UNEXPECTED_WAIT_FOR_SOCK() )
//	//				{
//	//					output_tcp_socket_error_tolerance_count = 0;
//	//					for ( int i = 0 ; i < _g->bridges.ABhs_masks_count ; i++ )
//	//					{
//	//						if ( _g->bridges.ABhs_masks[ i ] )
//	//						{
//	//							if ( _g->bridges.ABhs[ i ].single_AB->tcp_connection_established )  // all the connected udp stoped or die so restart them
//	//							{
//	//								//if ( FD_ISSET( _g->bridges.ABhs[ i ].single_AB->udp_sockfd , &readfds ) )
//	//								{
//	//									_g->bridges.ABhs[ i ].single_AB->retry_to_connect_tcp = 1;
//	//									break;
//	//								}
//	//							}
//	//						}
//	//					}
//	//				}
//
//	//				continue;
//	//			}
//	//			_g->stat.round_zero_set.continuously_unsuccessful_send_error = 0;
//	//			if ( snd_ret > 0 )
//	//			{
//	//				_g->stat.round_zero_set.tcp.total_tcp_put_count++;
//	//				_g->stat.round_zero_set.tcp.total_tcp_put_byte += snd_ret;
//	//				_g->stat.round_zero_set.tcp_1_sec.calc_throughput_tcp_put_count++;
//	//				_g->stat.round_zero_set.tcp_1_sec.calc_throughput_tcp_put_bytes += snd_ret;
//	//				//_g->stat.round.tcp_10_sec.calc_throughput_tcp_put_count++;
//	//				//_g->stat.round.tcp_10_sec.calc_throughput_tcp_put_bytes += snd_ret;
//	//				//_g->stat.round.tcp_40_sec.calc_throughput_tcp_put_count++;
//	//				//_g->stat.round.tcp_40_sec.calc_throughput_tcp_put_bytes += snd_ret;
//	//			}
//	//		}
//	//	}
//
//	//}
//
//	//BREAK_OK(0); // to just ignore gcc warning
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

return NULL;
}

#endif

#ifndef thread_just_for_input

_THREAD_FXN void * justIncoming_thread_proc( void * src_g )
{
//	INIT_BREAKABLE_FXN();
//	static TWD twd = { 0 };
//	if ( twd.threadId == 0 )
//	{
//		twd.threadId = pthread_self();
//		twd.cal = justIncoming_thread_proc; // self function address
//		twd.callback_arg = src_pb;
//	}
//	if ( src_pb == NULL )
//	{
//		return ( void * )&twd;
//	}
//
//	////AB * pb = ( AB * )src_pb;
//	//G * _g = ( G * )src_g;
//
//
//	//while ( !_g->bridges.thread_base.start_working )
//	//{
//	//	if ( _g->bridges.thread_base.do_close_thread )
//	//	{
//	//		break;
//	//	}
//	//	sleep(1);
//	//}
//
//
//	//time_t tnow = 0;
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
//	//			if ( _g->bridges.ABhs[ i ].single_AB->udp_connection_established /* && _g->bridges.ABhs[i].single_AB->tcp_connection_established*/ )
//	//			{
//	//				FD_SET( _g->bridges.ABhs[ i ].single_AB->udp_sockfd , &readfds );
//	//				if ( _g->bridges.ABhs[ i ].single_AB->udp_sockfd > sockfd_max )
//	//				{
//	//					sockfd_max = _g->bridges.ABhs[ i ].single_AB->udp_sockfd;
//	//				}
//	//			}
//	//		}
//	//	}
//	//	_g->bridges.under_listen_udp_sockets_group_changed = 0; // if any udp socket change then fdset must be reinitialized
//	//	if ( sockfd_max < 0 )
//	//	{
//	//		sleep( 1 );
//	//		continue;
//	//	}
//
//	//	while ( 1 )
//	//	{
//	//		//pthread_mutex_lock( &_g->sync.mutex );
//	//		//while ( _g->sync.lock_in_progress )
//	//		//{
//	//		//	////struct timespec ts = { 0, 10L };
//	//		//	////thrd_sleep( &ts , NULL );
//	//		//	pthread_cond_wait( &_g->sync.cond , &_g->sync.mutex );
//	//		//}
//	//		//pthread_mutex_unlock( &_g->sync.mutex );
//	//		//if ( _g->sync.reset_static_after_lock )
//	//		//{
//	//		//	_g->sync.reset_static_after_lock = 0;
//	//		//	memset( &_g->stat.round , 0 , sizeof( _g->stat.round ) );
//	//		//}
//
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
//	//		//struct timeval timeout; // Set timeout (e.g., 5 seconds)
//	//		//timeout.tv_sec = ( input_udp_socket_error_tolerance_count + 1 ) * 2;
//	//		//timeout.tv_usec = 0;
//
//	//		// Wait for an activity on one of the sockets, timeout is NULL, so wait indefinitely
//	//		int activity = select( sockfd_max + 1 , &readfds , NULL , NULL , NULL/* & timeout*/ );
//
//	//		if ( ( activity < 0 ) /* && ( errno != EINTR )*/ )
//	//		{
//
//	//			int error = 0;
//	//			socklen_t errlen = sizeof( error );
//	//			getsockopt( sockfd_max , SOL_SOCKET , SO_ERROR , &error , &errlen );
//	//			if ( error != 0 )
//	//			{
//	//				_VERBOSE_ECHO( "Socket error: %d\n" , error );
//	//			}
//
//	//			if ( ++input_udp_socket_error_tolerance_count > RETRY_UNEXPECTED_WAIT_FOR_SOCK() )
//	//			{
//	//				input_udp_socket_error_tolerance_count = 0;
//	//				for ( int i = 0 ; i < _g->bridges.ABhs_masks_count ; i++ )
//	//				{
//	//					if ( _g->bridges.ABhs_masks[ i ] )
//	//					{
//	//						if ( _g->bridges.ABhs[ i ].single_AB->udp_connection_established ) // all the connected udp stoped or die so restart them
//	//						{
//	//							//if ( FD_ISSET( _g->bridges.ABhs[ i ].single_AB->udp_sockfd , &readfds ) )
//	//							{
//	//								_g->bridges.ABhs[ i ].single_AB->retry_to_connect_udp = 1;
//	//								break;
//	//							}
//	//						}
//	//					}
//	//				}
//	//			}
//
//	//			continue;
//	//		}
//	//		if ( activity == 0 ) // timed out
//	//		{
//
//	//			int error = 0;
//	//			socklen_t errlen = sizeof( error );
//	//			getsockopt( sockfd_max , SOL_SOCKET , SO_ERROR , &error , &errlen );
//	//			if ( error == 0 )
//	//			{
//	//				_g->stat.round_zero_set.udp.continuously_unsuccessful_select_on_open_port_count++;
//	//				if ( ++input_udp_socket_error_tolerance_count > RETRY_UNEXPECTED_WAIT_FOR_SOCK() )
//	//				{
//	//					input_udp_socket_error_tolerance_count = 0;
//	//					for ( int i = 0 ; i < _g->bridges.ABhs_masks_count ; i++ )
//	//					{
//	//						if ( _g->bridges.ABhs_masks[ i ] )
//	//						{
//	//							if ( _g->bridges.ABhs[ i ].single_AB->udp_connection_established ) // all the connected udp stoped or die so restart them
//	//							{
//	//								//if ( FD_ISSET( _g->bridges.ABhs[ i ].single_AB->udp_sockfd , &readfds ) )
//	//								{
//	//									_g->bridges.ABhs[ i ].single_AB->retry_to_connect_udp = 1;
//	//									break;
//	//								}
//	//							}
//	//						}
//	//					}
//	//				}
//	//				continue;
//	//			}
//	//			_VERBOSE_ECHO( "Socket error: %d\n" , error );
//	//			
//	//			continue;
//	//		}
//
//	//		_g->stat.round_zero_set.udp.continuously_unsuccessful_select_on_open_port_count = 0;
//
//	//		if ( _g->stat.round_zero_set.t_begin.tv_sec == 0 && _g->stat.round_zero_set.t_begin.tv_usec == 0 )
//	//		{
//	//			gettimeofday(&_g->stat.round_zero_set.t_begin, NULL);
//	//		}
//
//
//	//		tnow = time( NULL );
//	//		// udp
//	//		if ( difftime( tnow , _g->stat.round_zero_set.udp_1_sec.t_udp_throughput ) >= 1.0 )
//	//		{
//	//			if ( _g->stat.round_zero_set.udp_1_sec.t_udp_throughput > 0 )
//	//			{
//	//				circbuf_advance( &_g->stat.round_init_set.udp_stat_5_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
//	//				circbuf_advance( &_g->stat.round_init_set.udp_stat_5_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );
//
//	//				circbuf_advance( &_g->stat.round_init_set.udp_stat_10_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
//	//				circbuf_advance( &_g->stat.round_init_set.udp_stat_10_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );
//
//	//				circbuf_advance( &_g->stat.round_init_set.udp_stat_40_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
//	//				circbuf_advance( &_g->stat.round_init_set.udp_stat_40_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );
//
//	//				circbuf_advance( &_g->stat.round_init_set.udp_stat_120_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
//	//				circbuf_advance( &_g->stat.round_init_set.udp_stat_120_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );
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
//	//		for ( int i = 0 ; i < _g->bridges.ABhs_masks_count ; i++ )
//	//		{
//	//			if ( _g->bridges.ABhs_masks[ i ] )
//	//			{
//	//				if ( _g->bridges.ABhs[ i ].single_AB->udp_connection_established )
//	//				{
//	//					if ( FD_ISSET( _g->bridges.ABhs[ i ].single_AB->udp_sockfd , &readfds ) )
//	//					{
//	//						while( 1 )
//	//						{
//	//						
//	//							bytes_received = recvfrom( _g->bridges.ABhs[ i ].single_AB->udp_sockfd , buffer , BUFFER_SIZE , MSG_DONTWAIT , ( struct sockaddr * )&client_addr , &client_len ); // good for udp data recieve
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
//	//							gettimeofday(&_g->stat.round_zero_set.t_end, NULL);
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
//	//						}
//	//					}
//	//				}
//	//			}
//	//		}
//	//	}
//
//	//} while ( config_changes );
//	//
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
//
	return NULL;
}

#endif

_THREAD_FXN void * protocol_bridge_runner( void * src_pb )
{
//	INIT_BREAKABLE_FXN();
//	static TWD twd = { 0 };
//	if ( twd.threadId == 0 )
//	{
//		twd.threadId = pthread_self();
//		twd.cal = protocol_bridge_runner; // self function address
//		twd.callback_arg = src_pb;
//	}
//	if ( src_pb == NULL )
//	{
//		return ( void * )&twd;
//	}
//
//	//AB * pb = ( AB * )src_pb;
//	//G * _g = pb->ccfg.m.m.temp_data._g;
//	//pthread_t tid = pthread_self();
//	////time_t tnow = 0;
//	//struct protocol_bridge_thread * pthread = NULL;
//	//while ( pthread == NULL )
//	//{
//	//	for ( int i = 0 ; i < pb->pb_trds_masks_count ; i++ )
//	//	{
//	//		if ( pb->pb_trds_masks[ i ] )
//	//		{
//	//			if ( pb->pb_trds[ i ].alc_thread->trd_id == tid )
//	//			{
//	//				pthread = pb->pb_trds[ i ].alc_thread;
//	//				break;
//	//			}
//	//		}
//	//	}
//	//}
//	//if ( pthread == NULL )
//	//{
//	//	_g->stat.round_zero_set.syscal_err_count++;
//	//	return NULL;
//	//}
//
//	//if ( _g->appcfg._general_config->c.c.atht == buttleneck )
//	//{
//	//	if ( _g->bridges.bottleneck_thread != NULL )
//	//	{
//	//		pthread_mutex_lock( &_g->bridges.thread_base.creation_thread_race_cond );
//	//		if ( !_g->bridges.thread_base.thread_is_created )
//	//		{
//	//			MM_BREAK_IF( pthread_create( &_g->bridges.bottleneck_thread->trd_id , NULL , bottleneck_thread_proc , _g ) != PTHREAD_CREATE_OK , errGeneral , 0 , "thread creation failed" );
//	//			_g->bridges.thread_base.thread_is_created = 1;
//	//		}
//	//		pthread_mutex_unlock( &_g->bridges.thread_base.creation_thread_race_cond );
//	//	}
//	//}
//
//	//if ( _g->appcfg._general_config->c.c.atht == bidirection )
//	//{
//	//	if ( _g->bridges.bidirection_thread != NULL )
//	//	{
//	//		pthread_mutex_lock( &_g->bridges.thread_base.creation_thread_race_cond );
//	//		if ( !_g->bridges.thread_base.thread_is_created )
//	//		{
//	//			MM_BREAK_IF( pthread_create( &_g->bridges.bidirection_thread->mem.income_trd_id , NULL , income_thread_proc , _g ) != PTHREAD_CREATE_OK , errGeneral , 0 , "thread creation failed" );
//	//			MM_BREAK_IF( pthread_create( &_g->bridges.bidirection_thread->mem.outgoing_trd_id , NULL , outgoing_thread_proc , _g ) != PTHREAD_CREATE_OK , errGeneral , 0 , "thread creation failed" );
//
//	//			_g->bridges.thread_base.thread_is_created = 1;
//	//		}
//	//		pthread_mutex_unlock( &_g->bridges.thread_base.creation_thread_race_cond );
//	//	}
//	//}
//
//	//if ( _g->appcfg._general_config->c.c.atht == justIncoming )
//	//{
//	//	if ( _g->bridges.justIncoming_thread != NULL )
//	//	{
//	//		pthread_mutex_lock( &_g->bridges.thread_base.creation_thread_race_cond );
//	//		if ( !_g->bridges.thread_base.thread_is_created )
//	//		{
//	//			MM_BREAK_IF( pthread_create( &_g->bridges.justIncoming_thread->trd_id , NULL , justIncoming_thread_proc , _g ) != PTHREAD_CREATE_OK , errGeneral , 0 , "thread creation failed" );
//	//			_g->bridges.thread_base.thread_is_created = 1;
//	//		}
//	//		pthread_mutex_unlock( &_g->bridges.thread_base.creation_thread_race_cond );
//	//	}
//	//}
//
//	//int try_to_connect_udp_port = 1; // for the first time
//	//int try_to_connect_tcp_port = 1; // for the first time
//
//	//pthread->base_config_change_applied = 0;
//	//do
//	//{
//	//	if ( pthread->do_close_thread )
//	//	{
//	//		break;
//	//	}
//
//	//	if ( pthread->base_config_change_applied ) // config change cause reconnect
//	//	{
//	//		pthread->base_config_change_applied = 0;
//	//		try_to_connect_udp_port = 1;
//	//		try_to_connect_tcp_port = 1;
//	//	}
//	//	if ( pb->retry_to_connect_udp ) // retry from socket err cause reconnect
//	//	{
//	//		pb->retry_to_connect_udp = 0;
//	//		try_to_connect_udp_port = 1;
//	//	}
//	//	if ( pb->retry_to_connect_tcp ) // retry from socket err cause reconnect
//	//	{
//	//		pb->retry_to_connect_tcp = 0;
//	//		try_to_connect_tcp_port = 1;
//	//	}
//	//	if ( !try_to_connect_udp_port && !try_to_connect_tcp_port )
//	//	{
//	//		sleep(2);
//	//		continue;
//	//	}
//	//	int tmp_try_to_connect_udp_port = try_to_connect_udp_port;
//	//	int tmp_try_to_connect_tcp_port = try_to_connect_tcp_port;
//	//	try_to_connect_udp_port = 0;
//	//	try_to_connect_tcp_port = 0;
//
//	//	if ( _g->appcfg._general_config->c.c.atht == buttleneck || _g->appcfg._general_config->c.c.atht == bidirection )
//	//	{
//	//		// first close then reconnect
//	//		if ( tmp_try_to_connect_udp_port )
//	//		{
//	//			pthread_t trd_udp_connection;
//	//			MM_BREAK_IF( pthread_create( &trd_udp_connection , NULL , thread_udp_connection_proc , pb ) != PTHREAD_CREATE_OK , errGeneral , 0 , "thread creation failed" );
//	//		}
//	//		if ( tmp_try_to_connect_tcp_port )
//	//		{
//	//			pthread_t trd_tcp_connection;
//	//			MM_BREAK_IF( pthread_create( &trd_tcp_connection , NULL , thread_tcp_connection_proc , pb ) != PTHREAD_CREATE_OK , errGeneral , 0 , "thread creation failed" );
//	//		}
//	//	}
//	//	else if ( _g->appcfg._general_config->c.c.atht == justIncoming )
//	//	{
//	//		if ( tmp_try_to_connect_udp_port )
//	//		{
//	//			pthread_t trd_udp_connection;
//	//			MM_BREAK_IF( pthread_create( &trd_udp_connection , NULL , thread_udp_connection_proc , pb ) != PTHREAD_CREATE_OK , errGeneral , 0 , "thread creation failed" );
//	//		}
//	//	}
//	//	
//
//	//} while ( 1 );
//
//
//	////// delete mask
//	//for ( int i = 0 ; i < pthread->pb->pb_trds_masks_count ; i++ )
//	//{
//	//	if ( pthread->pb->pb_trds_masks[ i ] && pthread->pb->pb_trds[ i ].alc_thread->trd_id == tid )
//	//	{
//	//		pthread->pb->pb_trds_masks[ i ] = 0;
//	//		break;
//	//	}
//	//}
//
//	//
//	//BEGIN_RET
//	//	case 3: {}
//	//	case 2: {}
//	//	case 1:
//	//	{
//	//		//_close_socket( &src_pb->tcp_sockfd );
//	//		_g->stat.round_zero_set.syscal_err_count++;
//	//	}
//	//M_V_END_RET
//
	return NULL;
}
