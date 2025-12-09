#define Uses_MARK_LINE
#define Uses_token_ring_p_t
#define Uses_WARNING
#define Uses_iSTR_SAME
#define Uses_dict_s_i_t
#define Uses_dict_s_s_t
#define Uses_sleep
#define Uses_many_tcp_out_thread_proc
#define Uses__VERBOSE_ECHO
#define Uses_errno
#define Uses_globals
#define Uses_Bridge
#define Uses_INIT_BREAKABLE_FXN
#include <Protocol_Bridge.dep>


_CALLBACK_FXN _PRIVATE_FXN status fast_ring_2_huge_ring_7_( pass_p data , buffer buf , size_t sz )
{
	AB_tcp * tcp = ( AB_tcp * )data;
	AB * pb = tcp->owner_pb;
	G * _g = ( G * )tcp->owner_pb->cpy_cfg.m.m.temp_data._pseudo_g;

	xudp_hdr * pkt = ( xudp_hdr * )buf;
	strcpy( pkt->TCP_name , tcp->__tcp_cfg_pak->name ); // actually write on buffer

	// there in multiple tcp so each packet should have its own hash and uniq id
	dict_forcibly_get_hash_id_bykey( &_g->hdls.pkt_mgr.map_tcp_socket , pkt->TCP_name , INVALID_FD , NULL , &pkt->metadata.tcp_name_key_hash , &pkt->metadata.tcp_name_uniq_id );

	return fast_ring_2_huge_ring( data , buf , sz );
}

// spec each bridge how to act with udp packet get. and deliver it to global ring cache
_PRIVATE_FXN void init_many_tcp( AB * pb , shrt_pth_t * shrtcut )
{
	//G * _g = ( G * )pb->cpy_cfg.m.m.temp_data._g;

	// enumorate group type
	{
		dict_s_s_t dc_enum_grp_type;
		dict_init( &dc_enum_grp_type );
		for ( int iout = 0 ; iout < *shrtcut->out_count ; iout++ )
		{
			dict_put( &dc_enum_grp_type , pb->cpy_cfg.m.m.maintained.out[ iout ].data.group_type , pb->cpy_cfg.m.m.maintained.out[ iout ].data.group_type );
		}
		//size_t key_count = dict_count( &dc_enum_grp_type );
		//WARNING( key_count > 1 );

		strings pkeys = NULL;
		int keys_count = 0;
		dict_get_keys_ref( &dc_enum_grp_type , &pkeys , &keys_count );

		if ( strsistr( pkeys , keys_count , STR_RoundRobin ) >= 0 )
		{
			// TODO . i can make helper to reduce this path length 
			dict_o_init( shrtcut->dc_token_ring , true );
		}
		dict_free( &dc_enum_grp_type );
	}

	// make map of tcp grp name to index
	dict_s_i_t map_grp_idx;
	dict_s_i_init( &map_grp_idx );
	int igrpcounter = 0;
	for ( int itcp = 0 ; itcp < pb->tcps_count ; itcp++ )
	{
		//	dict_o_put( &dc_tcps , pb->tcps[ itcp ].__tcp_cfg_pak->name , pb->tcps[ itcp ].__tcp_cfg_pak );
		dict_s_i_put( &map_grp_idx , pb->tcps[ itcp ].__tcp_cfg_pak->data.group , igrpcounter++ , 0 );
	}

	// to check weather first time grp take member. so we can init grp ring
	dict_s_i_t init_grp;
	dict_s_i_init( &init_grp );

	// init pop distributor . each output distributor register here so they get arrived data
	distributor_init( shrtcut->bcast_xudp_pkt , dict_s_i_count( &map_grp_idx ) );

	//// اینجا در گروه ها می چرخد و هر مورد را به ساب اضافه می کند یعنی دریافت کننده یک دیتا
	//// در نتیجه وقتی دیتایی برای تی سی پی بود بین همه موارد توزیع می شود
	//// در راند رابین یک رینگ محافظت می کند که فقط اونی دیتا رو بگیره که توکن را داره
	for ( int itcp = 0 ; itcp < pb->tcps_count ; itcp++ )
	{
		if
		(
			iSTR_SAME( pb->tcps[ itcp ].__tcp_cfg_pak->data.group_type , STR_Replicate ) ||
			iSTR_SAME( pb->tcps[ itcp ].__tcp_cfg_pak->data.group_type , STR_ONE_OUT )
		)
		{
			int igrp = -1;
			dict_s_i_get( &map_grp_idx , pb->tcps[ itcp ].__tcp_cfg_pak->data.group , &igrp );
			distributor_subscribe_ingrp( shrtcut->bcast_xudp_pkt , (size_t)igrp , SUB_DIRECT_MULTICAST_CALL_BUFFER_SIZE ,
				SUB_FXN( fast_ring_2_huge_ring_7_ ) , pb->tcps + itcp );
		}
		else if ( iSTR_SAME( pb->tcps[ itcp ].__tcp_cfg_pak->data.group_type , STR_RoundRobin ) )
		{
			int igrp = -1; // use map to add ring to right grp
			dict_s_i_get( &map_grp_idx , pb->tcps[ itcp ].__tcp_cfg_pak->data.group , &igrp );
			int exist = 1; // try to peek then if not insert grp to dic
			dict_s_i_try_put( &init_grp , pb->tcps[ itcp ].__tcp_cfg_pak->data.group , igrp , &exist );

			if ( !exist )
			{
				// first elem spec group type
				token_ring_p_t * tring = CALLOC_ONE( tring );
				token_ring_p_init( tring );

				dict_o_put( shrtcut->dc_token_ring , igrp , tring ); // TODO . each values from dic should freed
				// TODO . release this ring finally
			}

			void_p pring = dict_o_get( shrtcut->dc_token_ring , igrp ); // get grp ring
			WARNING( pring );

			// add callback receiver of each tcp grp
			distributor_subscribe_with_ring( shrtcut->bcast_xudp_pkt ,
				( size_t )igrp , SUB_DIRECT_MULTICAST_CALL_BUFFER_SIZE , SUB_FXN( fast_ring_2_huge_ring_7_ ) , pb->tcps + itcp , pring );
		}
		else
		{
			WARNING( 0 );
		}
	}

	// TODO . call destroy or destructor of any dictionaries and collections
}


/// <summary>
/// this fxn get uninitialized active bridge single udp cfg and open and initialized it
/// </summary>
_THREAD_FXN void_p proc_many2many_krnl_udp_store( void_p src_pb )
{
	INIT_BREAKABLE_FXN();

	sleep( 5 );

	//AB_udp * pAB_udp = ( AB_udp * )src_AB_udp;
	AB * pb = ( AB * )src_pb;
	G * _g = pb->cpy_cfg.m.m.temp_data._pseudo_g;

#ifdef ENABLE_LOG_THREADS
	__attribute__( ( cleanup( thread_goes_out_of_scope ) ) ) pthread_t this_thread = pthread_self();
	distributor_publish_x3long( &_g->distributors.bcast_thread_startup , ( long )this_thread , trdn_proc_many2many_krnl_udp_store , (long)__FUNCTION__ , _g );
	
	/*retrieve track alive indicator*/
	pthread_mutex_lock( &_g->stat.nc_s_req.thread_list_mtx );
	time_t * pthis_thread_alive_time = NULL;
	for ( size_t idx = 0 ; idx < _g->stat.nc_s_req.thread_list.count ; idx++ )
	{
		thread_alive_indicator * pthread_ind = NULL;
		if ( mms_array_get_s( &_g->stat.nc_s_req.thread_list , idx , ( void ** )&pthread_ind ) == errOK && pthread_ind->thread_id == this_thread )
		{
			pthis_thread_alive_time = &pthread_ind->alive_time;
		}
	}
	pthread_mutex_unlock( &_g->stat.nc_s_req.thread_list_mtx );
#ifdef ENABLE_USE_DBG_TAG
	MARK_START_THREAD();
#endif
#endif

	shrt_pth_t shrtcut;
	mk_shrt_path( pb , &shrtcut );
	shrtcut.dc_token_ring = &pb->comm.acts.p_one2many_krnl2krnl_SF->dc_token_ring;
	//shrtcut.raw_xudp_cache = &pb->comm.preq.raw_xudp_cache;
	shrtcut.bcast_xudp_pkt = &pb->comm.preq.bcast_xudp_pkt;

#ifdef ENABLE_USE_DBG_TAG
	MARK_LINE();
#endif

	init_many_tcp( pb , &shrtcut ); // here make broadcaster then when i send to brodcast itself manage round robin and replicate

#ifdef ENABLE_USE_DBG_TAG
	MARK_LINE();
#endif

	char bufferr[ BUFFER_SIZE ] = { 0 }; // Define a buffer to store received data

	xudp_hdr * pkt = ( xudp_hdr * )bufferr; // plain cup for packet
	pkt->metadata.version = TCP_XPKT_V1;
	pkt->metadata.sent = false;
	pkt->metadata.retry = false; // since sending latest packet is prioritized so just try send them once unless rare condition 
	pkt->metadata.retried = false;

	pkt->metadata.TCP_name_size = ( uint8_t )sizeof( pb->tcps[ 0 ].__tcp_cfg_pak->name );
	pkt->metadata.payload_offset = ( uint8_t )sizeof( pkt->metadata ) + pkt->metadata.TCP_name_size + ( uint8_t )sizeof( EOS )/*to read hdr name faster*/;
	//int local_tcp_header_data_length = sizeof( pkt->flags ) + pkt->flags.TCP_name_size + sizeof( EOS );

	// TODO . add M_BREAK_STAT( dict_forcibly_get_hash_id_bykey( &_g->hdls.pkt_mgr.map_tcp_socket , pkt->TCP_name , INVALID_FD , NULL , &pkt->metadata.tcp_name_key_hash , &pkt->metadata.tcp_name_uniq_id ) , 0 );

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
		if ( pthis_thread_alive_time ) *pthis_thread_alive_time = time( NULL );
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
			if ( pthis_thread_alive_time ) *pthis_thread_alive_time = time( NULL );
			#ifdef ENABLE_COMMUNICATION

			if ( pb->comm.preq.stop_receiving )
			{
				break;
			}

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
							if ( pthis_thread_alive_time ) *pthis_thread_alive_time = time( NULL );
							bytes_received = recvfrom( pb->udps[ iudp ].udp_sockfd , bufferr + pkt->metadata.payload_offset /*hdr + pkt*/ , BUFFER_SIZE , MSG_DONTWAIT , ( struct sockaddr * )&client_addr , &client_len ); // good for udp data recieve

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

							pkt->metadata.payload_sz = (size_t)bytes_received;

							#ifdef SEND_DIRECTLY_ARRIVE_UDP
								IMMORTAL_LPCSTR errString = NULL;
								uchar buf[ MIN_SYSERR_BUF_SZ ] = { 0 };
								tcp_send_all( pb->tcps[ iudp ].tcp_sockfd , bufferr + pkt->metadata.payload_offset , pkt->metadata.payload_sz , 0 , SEND_TIMEOUT_ms , ACK_TIMEOUT_ms , RETRY_MECHANISM , &errString , ( buffer * )&buf );
							#else
								if ( distributor_publish_buffer_size( &pb->comm.preq.bcast_xudp_pkt , bufferr , (size_t)( bytes_received + pkt->metadata.payload_offset ) , SUBSCRIBER_PROVIDED ) != errOK ) // 14040622 . do replicate or roundrobin
									continue;
							#endif

							#ifndef statistics
							#ifdef ENABLE_GATHER_STATIC
							gettimeofday( &pb->stat.round_zero_set.t_end , NULL );

							pb->stat.round_zero_set.udp.total_udp_get_count++;
							pb->stat.round_zero_set.udp.total_udp_get_byte += ( __int64u )bytes_received;
							pb->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count++;
							pb->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes += ( __int64u )bytes_received;
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
	pb->comm.preq.receive_stoped = true; // this is important that do not be anyway that stoped does not set
	return NULL;
}
