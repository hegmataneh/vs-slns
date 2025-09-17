#define Uses_packet_mngr
#define Uses_dict_s_i_t
#define Uses_dict_s_s_t
#define Uses_MEMSET_ZERO_O
#define Uses_token_ring_p_t
#define Uses_distributor_init
#define Uses_errno
#define Uses_helper
#define Uses_Bridge
#define Uses_INIT_BREAKABLE_FXN
#define Uses_async_tcp_forward

#include <Protocol_Bridge.dep>

// 14040622 . call for replicate and round robin for now
//_PRIVATE_FXN status send_tcp_packet( pass_p data , buffer buf , int sz )
//{
//	status d_error = errOK;
//	AB_tcp * tcp = ( AB_tcp * )data;
//	AB * pb = tcp->owner_pb;
//	G * _g = ( G * )tcp->owner_pb->cpy_cfg.m.m.temp_data._g;
//
//	// TODO . add into memory
//	// TODO . add log if necessary
//	// TODO . add record to file if memory about to full
//	// TODO . some how save record to send them later to spec destination
//
//	//size_t sz_t = sz;
//	//status d_error = sendall( tcp->tcp_sockfd , buf , &sz_t ); // send is to heavy so it must send where it can
//
//	//if ( d_error == errOK && sz > 0 )
//	//{
//	//	pb->stat.round_zero_set.tcp.total_tcp_put_count++;
//	//	pb->stat.round_zero_set.tcp.total_tcp_put_byte += sz;
//	//	pb->stat.round_zero_set.tcp_1_sec.calc_throughput_tcp_put_count++;
//	//	pb->stat.round_zero_set.tcp_1_sec.calc_throughput_tcp_put_bytes += sz;
//	//	pb->stat.round_zero_set.tcp_send_data_alive_indicator++;
//	//}
//	return d_error;
//}

_PRIVATE_FXN void init_many_tcp( AB * pb , shrt_path * hlpr )
{
	//G * _g = ( G * )pb->cpy_cfg.m.m.temp_data._g;

	// enumorate group type
	{
		dict_s_s_t dc_enum_grp_type;
		dict_init( &dc_enum_grp_type );
		for ( int iout = 0 ; iout < *hlpr->out_count ; iout++ )
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
			dict_o_init( hlpr->dc_token_ring );
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
	distributor_init( hlpr->buf_pop_distr , ( int )dict_s_i_count( &map_grp_idx ) );

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
			distributor_subscribe( hlpr->buf_pop_distr , SUB_DIRECT_MULTICAST_CALL_BUFFER_INT ,
				SUB_FXN( operation_on_tcp_packet ) , pb->tcps + itcp );
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
				token_ring_p_t * tring = MALLOC_ONE( tring );
				MEMSET_ZERO_O( tring );
				token_ring_p_init( tring );

				dict_o_put( hlpr->dc_token_ring , igrp , tring ); // TODO . each values from dic should freed
				// TODO . release this ring finally
			}

			void_p pring = dict_o_get( hlpr->dc_token_ring , igrp ); // get grp ring
			WARNING( pring );

			// add callback receiver of each tcp grp
			distributor_subscribe_with_ring( hlpr->buf_pop_distr ,
				igrp , SUB_DIRECT_MULTICAST_CALL_BUFFER_INT , SUB_FXN( operation_on_tcp_packet ) , pb->tcps + itcp , pring );
		}
		else
		{
			WARNING( 0 );
		}
	}

	// TODO . call destroy or destructor of any dictionaries and collections


}

_REGULAR_FXN void_p many_tcp_out_thread_proc( AB * pb , shrt_path * hlpr )
{
	INIT_BREAKABLE_FXN();

	G * _g = ( G * )pb->cpy_cfg.m.m.temp_data._g;

	time_t tnow = 0;
	char buffer[ BUFFER_SIZE + 128 /*tcp version + grp name[64]*/ ]; // Define a buffer to store received data
	MEMSET_ZERO_O( buffer );
	pass_p pdata = NULL;
	size_t sz;
	//ssize_t snd_ret;

	init_many_tcp( pb , hlpr );

	BREAK_STAT( distributor_get_data( hlpr->buf_pop_distr , &pdata ) , 0 );
	AB_tcp * tcp = ( AB_tcp * )pdata;
	
	rdy_pkt1 * pkt = ( rdy_pkt1 * )buffer;
	pkt->flags.version = TCP_PACKET_VERSION;
	pkt->flags.sent = false;
	strcpy( pkt->TCP_name , tcp->__tcp_cfg_pak->name );
	pkt->flags.TCP_name_size = strlen( pkt->TCP_name );
	pkt->flags.payload_offset = sizeof( pkt->flags ) + pkt->flags.TCP_name_size + sizeof( EOS )/*to read hdr name faster*/;
	//int local_tcp_header_data_length = sizeof( pkt->flags ) + pkt->flags.TCP_name_size + sizeof( EOS );

	int output_tcp_socket_error_tolerance_count = 0; // restart socket after many error accur

	while ( !pb->trd.base.bridg_prerequisite_stabled )
	{
		if ( CLOSE_APP_VAR() ) break;
		mng_basic_thread_sleep( _g , HI_PRIORITY_THREAD );
	}

	M_BREAK_STAT( dict_fst_get_hash_id_bykey( &_g->hdls.map_tcp_socket , pkt->TCP_name , &pkt->flags.tcp_name_key_hash , &pkt->flags.tcp_name_uniq_id ) , 0 );

	//WARNING( pb->tcps_count >= 1 );
	//AB_tcp * tcp = pb->tcps; // caution . in this type of bridge udp conn must be just one

	while ( 1 )
	{
		//	//pthread_mutex_lock( &_g->sync.mutex );
		//	//while ( _g->sync.lock_in_progress )
		//	//{
		//	//	////struct timespec ts = { 0, 10L };
		//	//	////thrd_slep( &ts , NULL );
		//	//	pthread_cond_wait( &_g->sync.cond , &_g->sync.mutex );
		//	//}
		//	//pthread_mutex_unlock( &_g->sync.mutex );
		//	//if ( _g->sync.reset_static_after_lock )
		//	//{
		//	//	_g->sync.reset_static_after_lock = 0;
		//	//	MEMSET( &_g->stat.round , 0 , sizeof( _g->stat.round ) );
		//	//}

		if ( pb->trd.base.do_close_thread )
		{
			break;
		}

		tnow = time( NULL );
		// tcp
		if ( difftime( tnow , pb->stat.round_zero_set.tcp_1_sec.t_tcp_throughput ) >= 1.0 )
		{
			if ( pb->stat.round_zero_set.tcp_1_sec.t_tcp_throughput > 0 )
			{
				cbuf_m_advance( &pb->stat.round_init_set.tcp_stat_5_sec_count , pb->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
				cbuf_m_advance( &pb->stat.round_init_set.tcp_stat_5_sec_bytes , pb->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );

				cbuf_m_advance( &pb->stat.round_init_set.tcp_stat_10_sec_count , pb->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
				cbuf_m_advance( &pb->stat.round_init_set.tcp_stat_10_sec_bytes , pb->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );

				cbuf_m_advance( &pb->stat.round_init_set.tcp_stat_40_sec_count , pb->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
				cbuf_m_advance( &pb->stat.round_init_set.tcp_stat_40_sec_bytes , pb->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );

				cbuf_m_advance( &pb->stat.round_init_set.tcp_stat_120_sec_count , pb->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
				cbuf_m_advance( &pb->stat.round_init_set.tcp_stat_120_sec_bytes , pb->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );

				//_g->stat.round.tcp_1_sec.tcp_put_count_throughput = _g->stat.round.tcp_1_sec.calc_throughput_tcp_put_count;
				//_g->stat.round.tcp_1_sec.tcp_put_byte_throughput = _g->stat.round.tcp_1_sec.calc_throughput_tcp_put_bytes;
			}
			pb->stat.round_zero_set.tcp_1_sec.t_tcp_throughput = tnow;
			pb->stat.round_zero_set.tcp_1_sec.calc_throughput_tcp_put_count = 0;
			pb->stat.round_zero_set.tcp_1_sec.calc_throughput_tcp_put_bytes = 0;
		}

		

		while ( vcbuf_nb_pop( hlpr->cbuf , buffer + pkt->flags.payload_offset /*hdr + pkt*/ , &sz , 60/*timeout*/ ) == errOK )
		{
			// TODO . if connection lost i should do something here. but i dont know what should i do for now

			// TODO . result must be seperated from each other to make right statistic

			// CAUTION . in this broadcast it must store packet and return as soon as possible

			if ( distributor_publish_buffer_int( hlpr->buf_pop_distr , buffer , sz + pkt->flags.payload_offset , NULL ) != errOK ) // 14040622 . do replicate or roundrobin
				continue;

			//if ( sendall( pb->tcps->tcp_sockfd , buffer , &sz ) != errOK )
			//{
			//	_g->stat.round_zero_set.continuously_unsuccessful_send_error++;
			//	_g->stat.round_zero_set.total_unsuccessful_send_error++;

			//	if ( ++output_tcp_socket_error_tolerance_count > RETRY_UNEXPECTED_WAIT_FOR_SOCK() )
			//	{
			//		output_tcp_socket_error_tolerance_count = 0;
			//		if ( pb->tcps_count && pb->tcps->tcp_connection_established )
			//		{
			//			if ( peerTcpClosed( pb->tcps->tcp_sockfd ) )
			//			{
			//				pb->tcps->retry_to_connect_tcp = 1;
			//			}
			//		}
			//	}
			//	continue;
			//}
			pb->stat.round_zero_set.continuously_unsuccessful_send_error = 0;
			//if ( sz > 0 )
			//{
			//	_g->stat.round_zero_set.tcp.total_tcp_put_count++;
			//	_g->stat.round_zero_set.tcp.total_tcp_put_byte += sz;
			//	_g->stat.round_zero_set.tcp_1_sec.calc_throughput_tcp_put_count++;
			//	_g->stat.round_zero_set.tcp_1_sec.calc_throughput_tcp_put_bytes += sz;
			//	//_g->stat.round.tcp_10_sec.calc_throughput_tcp_put_count++;
			//	//_g->stat.round.tcp_10_sec.calc_throughput_tcp_put_bytes += snd_ret;
			//	//_g->stat.round.tcp_40_sec.calc_throughput_tcp_put_count++;
			//	//_g->stat.round.tcp_40_sec.calc_throughput_tcp_put_bytes += snd_ret;

			//	_g->stat.tcp_send_data_alive_indicator++;
			//}

		}

	}

	BREAK_OK( 0 ); // to just ignore gcc warning

	BEGIN_RET
	case 1:
	{
		//_close_socket( &src_pb->tcp_sockfd );
		DIST_ERR();
	}
	M_V_END_RET
	return NULL;
}
