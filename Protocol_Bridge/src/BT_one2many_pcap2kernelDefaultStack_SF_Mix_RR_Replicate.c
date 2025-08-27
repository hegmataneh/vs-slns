#define Uses_dict_s_i_t
#define Uses_dict_s_s_t
#define Uses_MEMSET_ZERO_O
#define Uses_token_ring_p_t
#define Uses_one2many_pcap2kernelDefaultStack_SF_Mix_RR_Replicate_pcap_udp_income_thread_proc
#define Uses_distributor_init
#define Uses_stablish_pcap_udp_connection
#define Uses_errno
#define Uses_helper
#define Uses_Bridge
#define Uses_INIT_BREAKABLE_FXN
#include <Protocol_Bridge.dep>

void mk_hlpr1( _IN AB * pb , _OUT mix_helper * hlpr )
{
	ASSERT( pb->trd.t.p_one2many_pcap2kernelDefaultStack_SF_Mix_RR_Replicate );

	mk_hlpr0( pb , ( abhelp * )hlpr );

	hlpr->income_trd_id = &pb->trd.t.p_one2many_pcap2kernelDefaultStack_SF_Mix_RR_Replicate->income_trd_id;
	hlpr->outgoing_trd_id = &pb->trd.t.p_one2many_pcap2kernelDefaultStack_SF_Mix_RR_Replicate->outgoing_trd_id;
	hlpr->cbuf = &pb->trd.t.p_one2many_pcap2kernelDefaultStack_SF_Mix_RR_Replicate->cbuf;
	hlpr->buf_pop_distr = &pb->trd.t.p_one2many_pcap2kernelDefaultStack_SF_Mix_RR_Replicate->buffer_pop_distributor;
	hlpr->dc_token_ring = &pb->trd.t.p_one2many_pcap2kernelDefaultStack_SF_Mix_RR_Replicate->dc_token_ring;
}

#ifndef get_udp

_PRIVATE_FXN _CALLBACK_FXN status buffer_push_one2many_pcap2kernelDefaultStack_SF_Mix_RR_Replicate( void_p data , buffer buf , int payload_len )
{
	AB * pb = ( AB * )data;
	return vcbuf_nb_push( &pb->trd.t.p_one2many_pcap2kernelDefaultStack_SF_Mix_RR_Replicate->cbuf , buf , payload_len );
}

_THREAD_FXN void_p one2many_pcap2kernelDefaultStack_SF_Mix_RR_Replicate_pcap_udp_income_thread_proc( void_p src_pb )
{
	INIT_BREAKABLE_FXN();
	AB * pb = ( AB * )src_pb;
	G * _g = pb->cpy_cfg.m.m.temp_data._g;

	mix_helper H;
	mk_hlpr1( pb , &H );

	ASSERT( *H.in_count == 1 );

	// TODO . implement muti input

	ASSERT( pb->udps_count == 1 );

	M_BREAK_STAT( distributor_init( H.buf_psh_distri , 1 ) , 1 );
	M_BREAK_STAT( distributor_subscribe( H.buf_psh_distri , 0 , SUB_DIRECT_ONE_CALL_BUFFER_INT ,
		SUB_FXN( buffer_push_one2many_pcap2kernelDefaultStack_SF_Mix_RR_Replicate ) , src_pb ) , 1 );
	M_BREAK_STAT( stablish_pcap_udp_connection( pb->udps ) , 1 );

	BEGIN_RET
	case 1:
	{
		_g->stat.round_zero_set.syscal_err_count++;
	}
	M_V_END_RET
	return NULL;
}

#endif

#ifndef send_tcp

status send_tcp_packet( void_p data , buffer buf , int sz )
{
	return errOK;
}

void init_many_tcp( AB * pb )
{
	G * _g = ( G * )pb->cpy_cfg.m.m.temp_data._g;

	mix_helper H;
	mk_hlpr1( pb , &H );

	// enumorate group type
	{
		dict_s_s_t dc_enum_grp_type;
		dict_init( &dc_enum_grp_type );
		for ( int iout = 0 ; iout < *H.out_count ; iout++ )
		{
			dict_put( &dc_enum_grp_type , pb->cpy_cfg.m.m.maintained.out[ iout ].data.group_type , pb->cpy_cfg.m.m.maintained.out[ iout ].data.group_type );
		}
		//size_t key_count = dict_count( &dc_enum_grp_type );
		//ASSERT( key_count > 1 );

		LPCSTR * pkeys = NULL;
		int keys_count = 0;
		dict_get_keys( &dc_enum_grp_type , &pkeys , &keys_count );

		if ( strsistr( pkeys , keys_count , STR_RoundRobin ) >= 0 )
		{
			// TODO . i can make helper to reduce this path length 
			dict_o_init( H.dc_token_ring );
		}
		dict_free( &dc_enum_grp_type );
	}

	dict_s_i_t map_grp_idx;
	dict_s_i_init( &map_grp_idx );
	int igrpcounter = 0;

	for ( int itcp = 0 ; itcp < pb->tcps_count ; itcp++ )
	{
	//	dict_o_put( &dc_tcps , pb->tcps[ itcp ].__tcp_cfg_pak->name , pb->tcps[ itcp ].__tcp_cfg_pak );
		dict_s_i_put( &map_grp_idx , pb->tcps[ itcp ].__tcp_cfg_pak->data.group , igrpcounter++ , 0 );
	}

	dict_s_i_t init_grp;
	dict_s_i_init( &init_grp );

	// init pop distributor
	distributor_init( H.buf_pop_distr , ( int )dict_s_i_count( &map_grp_idx ) );

	//// اینجا در گروه ها می چرخد و هر مورد را به ساب اضافه می کند یعنی دریافت کننده یک دیتا
	//// در نتیجه وقتی دیتایی برای تی سی پی بود بین همه موارد توزیع می شود
	//// در راند رابین یک رینگ محافظت می کند که فقط اونی دیتا رو بگیره که توکن را داره
	for ( int itcp = 0 ; itcp < pb->tcps_count ; itcp++ )
	{
		if ( iSTR_SAME( pb->tcps[ itcp ].__tcp_cfg_pak->data.group_type , STR_Replicate ) )
		{
			int igrp = -1;
			dict_s_i_get( &map_grp_idx , pb->tcps[ itcp ].__tcp_cfg_pak->data.group , &igrp );
			distributor_subscribe( H.buf_pop_distr , igrp , SUB_DIRECT_MULTICAST_CALL_BUFFER_INT , SUB_FXN( send_tcp_packet ) , pb->tcps + itcp );
		}
		else if ( iSTR_SAME( pb->tcps[ itcp ].__tcp_cfg_pak->data.group_type , STR_RoundRobin ) )
		{
			int igrp = -1;
			dict_s_i_get( &map_grp_idx , pb->tcps[ itcp ].__tcp_cfg_pak->data.group , &igrp );
			int exist = 1;
			dict_s_i_try_put( &init_grp , pb->tcps[ itcp ].__tcp_cfg_pak->data.group , igrp , &exist );

			if ( !exist )
			{
				// first elem spec group type
				token_ring_p_t * tring = MALLOC_ONE( tring );
				MEMSET_ZERO_O( tring );
				token_ring_p_init( tring );

				dict_o_put( H.dc_token_ring , igrp , tring ); // TODO . each values from dic should freed
			}

			void_p pring = dict_o_get( H.dc_token_ring , igrp );
			ASSERT( pring );

			distributor_subscribe_with_token( H.buf_pop_distr ,
				igrp , SUB_DIRECT_MULTICAST_CALL_BUFFER_INT , SUB_FXN( send_tcp_packet ) , pb->tcps + itcp , pring );
		}
		else
		{
			ASSERT( 0 );
		}
	}

	// TODO . call destroy or destructor of any dictionaries and collections


}

_THREAD_FXN void_p one2many_pcap2kernelDefaultStack_SF_Mix_RR_Replicate_many_tcp_out_thread_proc( void_p src_pb )
{
	INIT_BREAKABLE_FXN();
	//	static TWD twd = { 0 };
	//	if ( twd.threadId == 0 )
	//	{
	//		twd.threadId = pthread_self();
	//		twd.cal = outgoing_thread_proc; // self function address
	//		twd.callback_arg = src_pb;
	//	}
	//	if ( src_pb == NULL )
	//	{
	//		return ( void_p )&twd;
	//	}
	//
	AB * pb = ( AB * )src_pb;
	G * _g = ( G * )pb->cpy_cfg.m.m.temp_data._g;

	mix_helper H;
	mk_hlpr1( pb , &H );

	time_t tnow = 0;
	char buffer[ BUFFER_SIZE ]; // Define a buffer to store received data
	size_t sz;
	//ssize_t snd_ret;

	init_many_tcp( pb );

	int output_tcp_socket_error_tolerance_count = 0; // restart socket after many error accur

	while ( !pb->trd.base.bridg_prerequisite_stabled )
	{
		if ( CLOSE_APP_VAR() ) break;
		mng_basic_thread_sleep( _g , HI_PRIORITY_THREAD );
	}

	ASSERT( pb->tcps_count >= 1 );
	AB_tcp * tcp = pb->tcps; // caution . in this type of bridge udp conn must be just one

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
		//	//	memset( &_g->stat.round , 0 , sizeof( _g->stat.round ) );
		//	//}

		if ( pb->trd.base.do_close_thread )
		{
			break;
		}

		tnow = time( NULL );
		// tcp
		if ( difftime( tnow , _g->stat.round_zero_set.tcp_1_sec.t_tcp_throughput ) >= 1.0 )
		{
			if ( _g->stat.round_zero_set.tcp_1_sec.t_tcp_throughput > 0 )
			{
				cbuf_m_advance( &_g->stat.round_init_set.tcp_stat_5_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
				cbuf_m_advance( &_g->stat.round_init_set.tcp_stat_5_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );

				cbuf_m_advance( &_g->stat.round_init_set.tcp_stat_10_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
				cbuf_m_advance( &_g->stat.round_init_set.tcp_stat_10_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );

				cbuf_m_advance( &_g->stat.round_init_set.tcp_stat_40_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
				cbuf_m_advance( &_g->stat.round_init_set.tcp_stat_40_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );

				cbuf_m_advance( &_g->stat.round_init_set.tcp_stat_120_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
				cbuf_m_advance( &_g->stat.round_init_set.tcp_stat_120_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );

				//_g->stat.round.tcp_1_sec.tcp_put_count_throughput = _g->stat.round.tcp_1_sec.calc_throughput_tcp_put_count;
				//_g->stat.round.tcp_1_sec.tcp_put_byte_throughput = _g->stat.round.tcp_1_sec.calc_throughput_tcp_put_bytes;
			}
			_g->stat.round_zero_set.tcp_1_sec.t_tcp_throughput = tnow;
			_g->stat.round_zero_set.tcp_1_sec.calc_throughput_tcp_put_count = 0;
			_g->stat.round_zero_set.tcp_1_sec.calc_throughput_tcp_put_bytes = 0;
		}

		while ( vcbuf_nb_pop( &pb->trd.t.p_one2many_pcap2kernelDefaultStack_SF_Mix_RR_Replicate->cbuf , buffer , &sz , 60/*timeout*/ ) == errOK )
		{
			// TODO . if connection lost i should do something here. but i dont know what should i do for now

			distributor_publish_buffer_int( H.buf_pop_distr , buffer , sz );

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
			_g->stat.round_zero_set.continuously_unsuccessful_send_error = 0;
			if ( sz > 0 )
			{
				_g->stat.round_zero_set.tcp.total_tcp_put_count++;
				_g->stat.round_zero_set.tcp.total_tcp_put_byte += sz;
				_g->stat.round_zero_set.tcp_1_sec.calc_throughput_tcp_put_count++;
				_g->stat.round_zero_set.tcp_1_sec.calc_throughput_tcp_put_bytes += sz;
				//_g->stat.round.tcp_10_sec.calc_throughput_tcp_put_count++;
				//_g->stat.round.tcp_10_sec.calc_throughput_tcp_put_bytes += snd_ret;
				//_g->stat.round.tcp_40_sec.calc_throughput_tcp_put_count++;
				//_g->stat.round.tcp_40_sec.calc_throughput_tcp_put_bytes += snd_ret;

				_g->stat.tcp_send_data_alive_indicator++;
			}

			/*keep tcp arrival timing*/
			{
				tcp->pk_tm.prev_access = tcp->pk_tm.last_access;
				tcp->pk_tm.last_access = time( NULL );
				if ( tcp->pk_tm.prev_access > 0 )
				{
					tcp->pk_tm.curr_packet_delay = difftime( tcp->pk_tm.last_access , tcp->pk_tm.prev_access );
					if ( tcp->pk_tm.curr_packet_delay > tcp->pk_tm.max_packet_delay )
					{
						tcp->pk_tm.max_packet_delay = tcp->pk_tm.curr_packet_delay;
						distributor_publish_int_double( &_g->stat.thresholds , MAX_TCP_PACKET_DELAY , tcp->pk_tm.max_packet_delay );
					}
				}
			}
			/*~keep tcp arrival timing*/

		}

	}

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

#endif

