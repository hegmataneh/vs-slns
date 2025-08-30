#define Uses_errno
#define Uses_helper
#define Uses_Bridge
#define Uses_INIT_BREAKABLE_FXN
#include <Protocol_Bridge.dep>

/// <summary>
/// this fxn get uninitialized active bridge single udp cfg and open and initialized it
/// </summary>
/// <param name="src_AB_udp"></param>
/// <returns></returns>
_THREAD_FXN void_p kernel_default_stack_udp_counter_thread_proc( void_p src_pb )
{
	INIT_BREAKABLE_FXN();
	//static TWD twd = { 0 }; // static not allowed on shared fxn
	//if ( twd.threadId == 0 )
	//{
	//	twd.threadId = pthread_self();
	//	twd.cal = udp_counter_thread_proc; // self function address
	//	twd.callback_arg = src_pb;
	//}
	//if ( src_pb == NULL )
	//{
	//	return ( void_p )&twd;
	//}

	//AB_udp * pAB_udp = ( AB_udp * )src_AB_udp;
	AB * pb = ( AB * )src_pb;
	G * _g = pb->cpy_cfg.m.m.temp_data._g;

	while ( !pb->trd.base.bridg_prerequisite_stabled )
	{
		if ( pb->trd.base.do_close_thread )
		{
			break;
		}
		mng_basic_thread_sleep( _g , HI_PRIORITY_THREAD );
	}

	time_t tnow = 0;

	int input_udp_socket_error_tolerance_count = 0; // restart socket after many error accur

	int config_changes = 0;
	do
	{
		if ( pb->trd.base.do_close_thread )
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

		sockfd sockfd_max = -1; // for select compulsion
		for ( int iudp = 0 ; iudp < pb->udps_count ; iudp++ )
		{
			if ( pb->udps[ iudp ].udp_connection_established )
			{
				FD_SET( pb->udps[ iudp ].udp_sockfd , &readfds );
				if ( pb->udps[ iudp ].udp_sockfd > sockfd_max )
				{
					sockfd_max = pb->udps[ iudp ].udp_sockfd;
				}
			}
		}
		//pb->under_listen_udp_sockets_group_changed = 0; // if any udp socket change then fdset must be reinitialized
		if ( sockfd_max < 0 )
		{
			mng_basic_thread_sleep( _g , NORMAL_PRIORITY_THREAD );
			continue;
		}

		while ( 1 )
		{
			//pthread_mutex_lock( &_g->sync.mutex );
			//while ( _g->sync.lock_in_progress )
			//{
			//	////struct timespec ts = { 0, 10L };
			//	////thrd_slep( &ts , NULL );
			//	pthread_cond_wait( &_g->sync.cond , &_g->sync.mutex );
			//}
			//pthread_mutex_unlock( &_g->sync.mutex );
			//if ( _g->sync.reset_static_after_lock )
			//{
			//	_g->sync.reset_static_after_lock = 0;
			//	memset( &_g->stat.round , 0 , sizeof( _g->stat.round ) );
			//}


			if ( pb->trd.base.do_close_thread )
			{
				break;
			}
			//if ( pb->under_listen_udp_sockets_group_changed )
			//{
			//	config_changes = 1;
			//	break;
			//}

			//struct timeval timeout; // Set timeout (e.g., 5 seconds)
			//timeout.tv_sec = ( input_udp_socket_error_tolerance_count + 1 ) * 2;
			//timeout.tv_usec = 0;

			// Wait for an activity on one of the sockets, timeout is NULL, so wait indefinitely
			int activity = select( sockfd_max + 1 , &readfds , NULL , NULL , NULL/* & timeout*/ );

			if ( ( activity < 0 ) /* && ( errno != EINTR )*/ )
			{

				int error = 0;
				socklen_t errlen = sizeof( error );
				getsockopt( sockfd_max , SOL_SOCKET , SO_ERROR , &error , &errlen );
				if ( error != 0 )
				{
					_VERBOSE_ECHO( "Socket error: %d\n" , error );
				}

				//if ( ++input_udp_socket_error_tolerance_count > RETRY_UNEXPECTED_WAIT_FOR_SOCK() )
				//{
				//	input_udp_socket_error_tolerance_count = 0;
				//	for ( int i = 0 ; i < pb->ABhs_masks_count ; i++ )
				//	{
				//		if ( pb->ABhs_masks[ i ] )
				//		{
				//			if ( pb->ABs[ i ].single_AB->udp_connection_established ) // all the connected udp stoped or die so restart them
				//			{
				//				//if ( FD_ISSET( pb->ABs[ i ].single_AB->udp_sockfd , &readfds ) )
				//				{
				//					pb->ABs[ i ].single_AB->retry_to_connect_udp = 1;
				//					break;
				//				}
				//			}
				//		}
				//	}
				//}

				continue;
			}
			if ( activity == 0 ) // timed out
			{

				int error = 0;
				socklen_t errlen = sizeof( error );
				getsockopt( sockfd_max , SOL_SOCKET , SO_ERROR , &error , &errlen );
				if ( error == 0 )
				{
					_g->stat.round_zero_set.udp.continuously_unsuccessful_select_on_open_port_count++;
					//if ( ++input_udp_socket_error_tolerance_count > RETRY_UNEXPECTED_WAIT_FOR_SOCK() )
					//{
					//	input_udp_socket_error_tolerance_count = 0;
					//	for ( int i = 0 ; i < pb->ABhs_masks_count ; i++ )
					//	{
					//		if ( pb->ABhs_masks[ i ] )
					//		{
					//			if ( pb->ABs[ i ].single_AB->udp_connection_established ) // all the connected udp stoped or die so restart them
					//			{
					//				//if ( FD_ISSET( pb->ABs[ i ].single_AB->udp_sockfd , &readfds ) )
					//				{
					//					pb->ABs[ i ].single_AB->retry_to_connect_udp = 1;
					//					break;
					//				}
					//			}
					//		}
					//	}
					//}
					continue;
				}
				_VERBOSE_ECHO( "Socket error: %d\n" , error );

				continue;
			}

			_g->stat.round_zero_set.udp.continuously_unsuccessful_select_on_open_port_count = 0;

			if ( _g->stat.round_zero_set.t_begin.tv_sec == 0 && _g->stat.round_zero_set.t_begin.tv_usec == 0 )
			{
				gettimeofday( &_g->stat.round_zero_set.t_begin , NULL );
			}


			tnow = time( NULL );
			// udp
			if ( difftime( tnow , _g->stat.round_zero_set.udp_1_sec.t_udp_throughput ) >= 1.0 )
			{
				if ( _g->stat.round_zero_set.udp_1_sec.t_udp_throughput > 0 )
				{
					cbuf_m_advance( &_g->stat.round_init_set.udp_stat_5_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
					cbuf_m_advance( &_g->stat.round_init_set.udp_stat_5_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );

					cbuf_m_advance( &_g->stat.round_init_set.udp_stat_10_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
					cbuf_m_advance( &_g->stat.round_init_set.udp_stat_10_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );

					cbuf_m_advance( &_g->stat.round_init_set.udp_stat_40_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
					cbuf_m_advance( &_g->stat.round_init_set.udp_stat_40_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );

					cbuf_m_advance( &_g->stat.round_init_set.udp_stat_120_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
					cbuf_m_advance( &_g->stat.round_init_set.udp_stat_120_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );
				}
				_g->stat.round_zero_set.udp_1_sec.t_udp_throughput = tnow;
				_g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count = 0;
				_g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes = 0;
			}

			for ( int iudp = 0 ; iudp < pb->udps_count ; iudp++ )
			{
				if ( pb->udps[ iudp ].udp_connection_established )
				{
					if ( FD_ISSET( pb->udps[ iudp ].udp_sockfd , &readfds ) )
					{
						while ( 1 ) // drain it
						{

							bytes_received = recvfrom( pb->udps[ iudp ].udp_sockfd , buffer , BUFFER_SIZE , MSG_DONTWAIT , ( struct sockaddr * )&client_addr , &client_len ); // good for udp data recieve
							if ( bytes_received < 0 )
							{
								if ( errno == EAGAIN || errno == EWOULDBLOCK )
								{
									// No more packets available
									break;
								}
								else
								{
									_g->stat.round_zero_set.continuously_unsuccessful_receive_error++;
									_g->stat.round_zero_set.total_unsuccessful_receive_error++;
									break;
								}
							}

							if ( bytes_received <= 0 )
							{
								_g->stat.round_zero_set.continuously_unsuccessful_receive_error++;
								_g->stat.round_zero_set.total_unsuccessful_receive_error++;
								continue;
							}
							_g->stat.round_zero_set.continuously_unsuccessful_receive_error = 0;
							//buffer[ bytes_received ] = '\0'; // Null-terminate the received data

							gettimeofday( &_g->stat.round_zero_set.t_end , NULL );

							_g->stat.round_zero_set.udp.total_udp_get_count++;
							_g->stat.round_zero_set.udp.total_udp_get_byte += bytes_received;
							_g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count++;
							_g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes += bytes_received;
							//_g->stat.round.udp_10_sec.calc_throughput_udp_get_count++;
							//_g->stat.round.udp_10_sec.calc_throughput_udp_get_bytes += bytes_received;
							//_g->stat.round.udp_40_sec.calc_throughput_udp_get_count++;
							//_g->stat.round.udp_40_sec.calc_throughput_udp_get_bytes += bytes_received;

							_g->stat.udp_get_data_alive_indicator++;

						}
					}
				}
			}
		}

	} while ( config_changes );

	BREAK_OK( 0 ); // to just ignore gcc warning

	BEGIN_RET
	case 3: ;
	case 2: ;
	case 1:
	{
		//_close_socket( &src_pb->tcp_sockfd );
		_g->stat.round_zero_set.syscal_err_count++;
	}
	M_V_END_RET

	return NULL;
}
