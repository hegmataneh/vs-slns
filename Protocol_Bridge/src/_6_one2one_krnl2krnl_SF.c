#define Uses_MARK_LINE
#define Uses_sleep
#define Uses_many_tcp_out_thread_proc
#define Uses__VERBOSE_ECHO
#define Uses_errno
#define Uses_globals
#define Uses_Bridge
#define Uses_INIT_BREAKABLE_FXN
#include <Protocol_Bridge.dep>

/// <summary>
/// this fxn get uninitialized active bridge single udp cfg and open and initialized it
/// </summary>
_THREAD_FXN void_p proc_one2one_krnl_udp_store( void_p src_pb )
{
	INIT_BREAKABLE_FXN();

	sleep(5);

	//AB_udp * pAB_udp = ( AB_udp * )src_AB_udp;
	AB * pb = ( AB * )src_pb;
	G * _g = pb->cpy_cfg.m.m.temp_data._pseudo_g;
	
	distributor_publish_long( &_g->distributors.bcast_thread_startup , (long)pthread_self() , _g );
	__attribute__( ( cleanup( thread_goes_out_of_scope ) ) ) pthread_t trd_id = pthread_self();
#ifdef ENABLE_USE_INTERNAL_C_STATISTIC
	MARK_START_THREAD();
#endif

	// init pop distributor . each output distributor register here so they get arrived data
	// it is multicast because of round robin and replication
	distributor_init( &pb->comm.preq.bcast_xudp_pkt , 1 );
	distributor_subscribe( &pb->comm.preq.bcast_xudp_pkt , SUB_DIRECT_MULTICAST_CALL_BUFFER_SIZE ,
		SUB_FXN( fast_ring_2_huge_ring ) , &pb->tcps[ 0 ] );


	char buffer[ BUFFER_SIZE ] = {0}; // Define a buffer to store received data

	xudp_hdr * pkt = ( xudp_hdr * )buffer; // plain cup for packet
	pkt->metadata.version = TCP_XPKT_V1;
	pkt->metadata.sent = false;
	pkt->metadata.retry = false; // since sending latest packet is prioritized so just try send them once unless rare condition 
	pkt->metadata.retried = false;
	
	// TODO . correct multi tcp policy
	strcpy( pkt->TCP_name , pb->tcps[ 0 ].__tcp_cfg_pak->name ); // actually write on buffer
	pkt->metadata.TCP_name_size = (uint8_t)strlen( pb->tcps[ 0 ].__tcp_cfg_pak->name );
	pkt->metadata.payload_offset = (uint8_t)sizeof( pkt->metadata ) + pkt->metadata.TCP_name_size + (uint8_t)sizeof( EOS )/*to read hdr name faster*/;
	//int local_tcp_header_data_length = sizeof( pkt->flags ) + pkt->flags.TCP_name_size + sizeof( EOS );

	//while ( !pb->comm.preq.bridg_prerequisite_stabled )
	//{
	//	if ( pb->comm.preq.do_close_thread )
	//	{
	//		break;
	//	}
	//	mng_basic_thread_sleep( _g , HI_PRIORITY_THREAD );
	//}

	time_t tnow = 0;

	int input_udp_socket_error_tolerance_count = 0; // restart socket after many error accur

	int config_changes = 0;
	do
	{
		if ( pb->comm.preq.stop_receiving )
		{
			break;
		}
		config_changes = 0;

		struct sockaddr_in client_addr;
		socklen_t client_len = sizeof( client_addr );
		ssize_t bytes_received;

		fd_set readfds; // Set of socket descriptors
		FD_ZERO( &readfds );

		//ssize_t sz;

		sockfd sockfd_max = invalid_fd; // for select compulsion
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
			#ifdef ENABLE_COMMUNICATION

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

			if ( pb->comm.preq.stop_receiving )
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
			int activity = select( sockfd_max + 1 , &readfds , NULL , NULL , NULL/* & timeout*/ ); TODO 

			if ( ( activity < 0 ) /* && ( errno != EINTR )*/ )
			{

				int error = 0;
				socklen_t errlen = sizeof( error );
				getsockopt( sockfd_max , SOL_SOCKET , SO_ERROR , &error , &errlen );
				if ( error != 0 )
				{
					#ifdef ENABLE_VERBOSE_FAULT
					_VERBOSE_ECHO( "Socket error: %d\n" , error );
					#endif
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
				#ifdef ENABLE_VERBOSE_FAULT
				_VERBOSE_ECHO( "Socket error: %d\n" , error );
				#endif

				continue;
			}

			#ifdef ENABLE_GATHER_STATIC
			pb->stat.round_zero_set.udp.continuously_unsuccessful_select_on_open_port_count = 0;
			#endif

			#ifndef statistic
			#ifdef ENABLE_GATHER_STATIC
			if ( pb->stat.round_zero_set.t_begin.tv_sec == 0 && pb->stat.round_zero_set.t_begin.tv_usec == 0 )
			{
				gettimeofday( &pb->stat.round_zero_set.t_begin , NULL );
				gettimeofday( &pb->stat.round_zero_set.t_end , NULL );
			}

			tnow = time( NULL );
			// udp
			if ( difftime( tnow , pb->stat.round_zero_set.udp_1_sec.t_udp_throughput ) >= 1.0 )
			{
				#ifdef ENABLE_THROUGHPUT_MEASURE
				if ( pb->stat.round_zero_set.udp_1_sec.t_udp_throughput > 0 )
				{
					cbuf_m_advance( &pb->stat.round_init_set.udp_stat_5_sec_count , pb->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
					cbuf_m_advance( &pb->stat.round_init_set.udp_stat_5_sec_bytes , pb->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );

					cbuf_m_advance( &pb->stat.round_init_set.udp_stat_10_sec_count , pb->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
					cbuf_m_advance( &pb->stat.round_init_set.udp_stat_10_sec_bytes , pb->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );

					cbuf_m_advance( &pb->stat.round_init_set.udp_stat_40_sec_count , pb->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
					cbuf_m_advance( &pb->stat.round_init_set.udp_stat_40_sec_bytes , pb->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );
				}
				#endif
				pb->stat.round_zero_set.udp_1_sec.t_udp_throughput = tnow;
				pb->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count = 0;
				pb->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes = 0;
			}
			#endif
			#endif

			for ( int iudp = 0 ; iudp < pb->udps_count ; iudp++ )
			{
				if ( pb->udps[ iudp ].udp_connection_established )
				{
					if ( FD_ISSET( pb->udps[ iudp ].udp_sockfd , &readfds ) )
					{
						while ( 1 ) // drain it
						{

							bytes_received = recvfrom( pb->udps[ iudp ].udp_sockfd , buffer + pkt->metadata.payload_offset /*hdr + pkt*/ , BUFFER_SIZE , MSG_DONTWAIT , ( struct sockaddr * )&client_addr , &client_len ); // good for udp data recieve

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
									#ifdef ENABLE_GATHER_STATIC
									pb->stat.round_zero_set.continuously_unsuccessful_receive_error++;
									pb->stat.round_zero_set.total_unsuccessful_receive_error++;
									#endif
									break;
								}
							}

							if ( bytes_received <= 0 )
							{
								#ifdef ENABLE_GATHER_STATIC
								pb->stat.round_zero_set.continuously_unsuccessful_receive_error++;
								pb->stat.round_zero_set.total_unsuccessful_receive_error++;
								#endif
								continue;
							}
							#ifdef ENABLE_GATHER_STATIC
							pb->stat.round_zero_set.continuously_unsuccessful_receive_error = 0;
							#endif
							//buffer[ bytes_received ] = '\0'; // Null-terminate the received data

							// TODO . we should determind that each udp send to which tcp

							#ifdef SEND_DIRECTLY_ARRIVE_UDP
								tcp_send_all( pb->tcps[ iudp ].tcp_sockfd , buffer + pkt->metadata.payload_offset , (int)bytes_received , 0 , 0 );
							#else
								if ( distributor_publish_buffer_size( &pb->comm.preq.bcast_xudp_pkt , buffer , (size_t)( bytes_received + pkt->metadata.payload_offset ) , SUBSCRIBER_PROVIDED ) != errOK ) // 14040622 . do replicate or roundrobin
									continue;
							#endif

							#ifndef statistics
							#ifdef ENABLE_GATHER_STATIC
							gettimeofday( &pb->stat.round_zero_set.t_end , NULL );

							pb->stat.round_zero_set.udp.total_udp_get_count++;
							pb->stat.round_zero_set.udp.total_udp_get_byte += (__int64u)bytes_received;
							pb->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count++;
							pb->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes += (__int64u)bytes_received;
							pb->stat.round_zero_set.udp_get_data_alive_indicator++;
							#endif
							#endif
						}
					}
				}
			}
		
			#else // ENABLE_COMMUNICATION
			sleep(1);
			#endif // ENABLE_COMMUNICATION

		}

	} while ( config_changes );

	BREAK_OK( 0 ); // to just ignore gcc warning

	BEGIN_RET
	case 3: ;
	case 2: ;
	case 1:
	{
		////_close_socket( &src_pb->tcp_sockfd );
		//DIST_BRIDGE_FAILURE();
	}
	M_V_END_RET

	pb->comm.preq.receive_stoped = true;
	

	return NULL;
}
