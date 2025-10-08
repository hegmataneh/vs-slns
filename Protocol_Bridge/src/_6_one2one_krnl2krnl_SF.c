#define Uses_many_tcp_out_thread_proc
#define Uses__VERBOSE_ECHO
#define Uses_errno
#define Uses_helper
#define Uses_Bridge
#define Uses_INIT_BREAKABLE_FXN
#include <Protocol_Bridge.dep>

_PRIVATE_FXN _CALLBACK_FXN status buffer_push_one2one_krnl2krnl_SF( pass_p data , buffer buf , size_t payload_len )
{
	AB * pb = ( AB * )data;
	return cbuf_pked_push( &pb->trd.cmn.ring_buf , buf , payload_len , payload_len , NULL );
}

/// <summary>
/// this fxn get uninitialized active bridge single udp cfg and open and initialized it
/// </summary>
_THREAD_FXN void_p proc_one2one_krnl_udp_store( void_p src_pb )
{
	INIT_BREAKABLE_FXN();

	//AB_udp * pAB_udp = ( AB_udp * )src_AB_udp;
	AB * pb = ( AB * )src_pb;
	G * _g = pb->cpy_cfg.m.m.temp_data._pseudo_g;

	while ( !pb->trd.cmn.bridg_prerequisite_stabled )
	{
		if ( pb->trd.cmn.do_close_thread )
		{
			break;
		}
		mng_basic_thread_sleep( _g , HI_PRIORITY_THREAD );
	}

	M_BREAK_STAT( distributor_init( &pb->trd.cmn.payload_push , 1 ) , 1 );
	M_BREAK_STAT( distributor_subscribe( &pb->trd.cmn.payload_push , SUB_DIRECT_ONE_CALL_BUFFER_SIZE ,
		SUB_FXN( buffer_push_one2one_krnl2krnl_SF ) , src_pb ) , 1 );

	time_t tnow = 0;

	int input_udp_socket_error_tolerance_count = 0; // restart socket after many error accur

	int config_changes = 0;
	do
	{
		if ( pb->trd.cmn.do_close_thread )
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
			//	MEMSET( &_g->stat.round , 0 , sizeof( _g->stat.round ) );
			//}


			if ( pb->trd.cmn.do_close_thread )
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
					pb->stat.round_zero_set.udp.continuously_unsuccessful_select_on_open_port_count++;
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

			pb->stat.round_zero_set.udp.continuously_unsuccessful_select_on_open_port_count = 0;

			if ( pb->stat.round_zero_set.t_begin.tv_sec == 0 && pb->stat.round_zero_set.t_begin.tv_usec == 0 )
			{
				gettimeofday( &pb->stat.round_zero_set.t_begin , NULL );
				gettimeofday( &pb->stat.round_zero_set.t_end , NULL );
			}

			tnow = time( NULL );
			// udp
			if ( difftime( tnow , pb->stat.round_zero_set.udp_1_sec.t_udp_throughput ) >= 1.0 )
			{
				if ( pb->stat.round_zero_set.udp_1_sec.t_udp_throughput > 0 )
				{
					cbuf_m_advance( &pb->stat.round_init_set.udp_stat_5_sec_count , pb->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
					cbuf_m_advance( &pb->stat.round_init_set.udp_stat_5_sec_bytes , pb->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );

					cbuf_m_advance( &pb->stat.round_init_set.udp_stat_10_sec_count , pb->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
					cbuf_m_advance( &pb->stat.round_init_set.udp_stat_10_sec_bytes , pb->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );

					cbuf_m_advance( &pb->stat.round_init_set.udp_stat_40_sec_count , pb->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
					cbuf_m_advance( &pb->stat.round_init_set.udp_stat_40_sec_bytes , pb->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );
				}
				pb->stat.round_zero_set.udp_1_sec.t_udp_throughput = tnow;
				pb->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count = 0;
				pb->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes = 0;
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
								if ( errno == EAGAIN )
								{
									// No more packets available
									break;
								}
								else if ( errno == EWOULDBLOCK )
								{
									continue;
								}
								else
								{
									pb->stat.round_zero_set.continuously_unsuccessful_receive_error++;
									pb->stat.round_zero_set.total_unsuccessful_receive_error++;
									break;
								}
							}

							if ( bytes_received <= 0 )
							{
								pb->stat.round_zero_set.continuously_unsuccessful_receive_error++;
								pb->stat.round_zero_set.total_unsuccessful_receive_error++;
								continue;
							}
							pb->stat.round_zero_set.continuously_unsuccessful_receive_error = 0;
							//buffer[ bytes_received ] = '\0'; // Null-terminate the received data

							if ( distributor_publish_buffer_size( &pb->trd.cmn.payload_push , buffer , bytes_received , NULL ) != errOK ) continue; // dist udp packet

							gettimeofday( &pb->stat.round_zero_set.t_end , NULL );

							pb->stat.round_zero_set.udp.total_udp_get_count++;
							pb->stat.round_zero_set.udp.total_udp_get_byte += bytes_received;
							pb->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count++;
							pb->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes += bytes_received;
							pb->stat.round_zero_set.udp_get_data_alive_indicator++;
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
		DIST_BRIDGE_FAILURE();
	}
	M_V_END_RET

	return NULL;
}

_THREAD_FXN void_p proc_one2one_krnl_tcp_forward( pass_p src_pb )
{
	INIT_BREAKABLE_FXN();

	AB * pb = ( AB * )src_pb;
	//G * _g = ( G * )pb->cpy_cfg.m.m.temp_data._g;

	shrt_path pth;
	mk_shrt_path( pb , &pth );
	pth.ring_buf = &pb->trd.cmn.ring_buf;
	pth.poped_payload = &pb->trd.cmn.poped_payload_from_rbuf;

	many_tcp_out_thread_proc( pb , &pth );

	return NULL;
}
