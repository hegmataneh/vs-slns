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

// spec each bridge how to act with udp packet get
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

// read udp ring buffer and sent them into general buffer as fast as possible
_REGULAR_FXN void_p many_tcp_out_thread_proc( AB * pb , shrt_path * hlpr )
{
	INIT_BREAKABLE_FXN();

	G * _g = ( G * )pb->cpy_cfg.m.m.temp_data._pseudo_g;

	char buffer[ BUFFER_SIZE + 128 /*tcp version + grp name[64]*/ ]; // Define a buffer to store received data
	MEMSET_ZERO_O( buffer );
	pass_p pdata = NULL;
	size_t sz;

	init_many_tcp( pb , hlpr );

	// try to find AB of udp getter
	BREAK_STAT( distributor_get_data( hlpr->buf_pop_distr , &pdata ) , 0 );
	AB_tcp * tcp = ( AB_tcp * )pdata;
	
	rdy_pkt1 * pkt = ( rdy_pkt1 * )buffer; // plain cup for packet
	pkt->flags.version = TCP_PACKET_V1;
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

	// to be insure we used correct tcp output
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

		// from ring pcap to stack general
		while ( vcbuf_nb_pop( hlpr->cbuf , buffer + pkt->flags.payload_offset /*hdr + pkt*/ , &sz , 60/*timeout*/ ) == errOK )
		{
			clock_gettime( CLOCK_MONOTONIC_COARSE , &pkt->flags.rec_t );

			if ( pb->trd.base.do_close_thread )
			{
				break;
			}

			// TODO . if connection lost i should do something here. but i dont know what should i do for now

			// TODO . result must be seperated from each other to make right statistic

			// CAUTION . in this broadcast it must store packet and return as soon as possible

			if ( distributor_publish_buffer_int( hlpr->buf_pop_distr , buffer , sz + pkt->flags.payload_offset , NULL ) != errOK ) // 14040622 . do replicate or roundrobin
				continue;

			pb->stat.round_zero_set.continuously_unsuccessful_send_error = 0;

		}

	}

	BREAK_OK( 0 ); // to just ignore gcc warning

	BEGIN_RET
	case 1:
	{
		//_close_socket( &src_pb->tcp_sockfd );
		DIST_BRIDGE_FAILURE();
	}
	M_V_END_RET
	return NULL;
}
