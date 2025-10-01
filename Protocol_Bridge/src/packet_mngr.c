#define Uses_packet_mngr
#define Uses_MEMSET_ZERO_O
#define Uses_errno
#define Uses_helper
#define Uses_Bridge
#define Uses_INIT_BREAKABLE_FXN

#include <Protocol_Bridge.dep>

GLOBAL_VAR extern G * _g;

_CALLBACK_FXN _PRIVATE_FXN void pre_config_init_packet_mngr( void_p src_g )
{
	G * _g = ( G * )src_g;

	distributor_init( &_g->distributors.throttling_release_halffill_segment , 1 );
	dict_fst_create( &_g->hdls.map_tcp_socket , 0 );

	distributor_subscribe( &_g->distributors.throttling_release_halffill_segment , SUB_VOID , SUB_FXN( release_halffill_segment ) , _g );
}

_CALLBACK_FXN _PRIVATE_FXN void post_config_init_packet_mngr( void_p src_g )
{
	INIT_BREAKABLE_FXN();
	G * _g = ( G * )src_g;

	segmgr_init( &_g->bufs.aggr_inp_pkt , ( size_t )_g->appcfg.g_cfg->c.c.pkt_mgr_segment_capacity , ( size_t )_g->appcfg.g_cfg->c.c.pkt_mgr_offsets_capacity , True );
	MM_BREAK_IF( pthread_create( &_g->trds.trd_tcp_sender , NULL , process_filled_tcp_segment_proc , ( pass_p )_g ) != PTHREAD_CREATE_OK , errCreation , 0 , "Failed to create tcp_sender thread" );

	BEGIN_SMPL
	M_V_END_RET
}

__attribute__( ( constructor( 105 ) ) )
static void pre_main_init_packet_mngr_component( void )
{
	distributor_subscribe( &_g->distributors.pre_configuration , SUB_VOID , SUB_FXN( pre_config_init_packet_mngr ) , _g );
	distributor_subscribe( &_g->distributors.post_config_stablished , SUB_VOID , SUB_FXN( post_config_init_packet_mngr ) , _g );
}


/// <summary>
/// from rings of each pcap or etc to global buffer
/// </summary>
_CALLBACK_FXN status operation_on_tcp_packet( pass_p data , buffer buf , int sz )
{
	INIT_BREAKABLE_FXN();

	AB_tcp * tcp = ( AB_tcp * )data;
	AB * pb = tcp->owner_pb;
	G * _g = ( G * )tcp->owner_pb->cpy_cfg.m.m.temp_data._pseudo_g;

	status ret = segmgr_append( &_g->bufs.aggr_inp_pkt , buf , sz ); // store whole pakt + hdr into global buffer
	RANJE_ACT1( ret , errArg , NULL_ACT , MACRO_E( M_BREAK_STAT( ret , 0 ) ) );

	// TODO . add into memory
	// TODO . add log if necessary
	// TODO . add record to file if memory about to full
	// TODO . some how save record to send them later to spec destination
		
	BEGIN_SMPL
	M_END_RET
}

_PRIVATE_FXN status process_segment_itm( buffer data , size_t len , pass_p src_g )
{
	G * _g = ( G * )src_g;
	status d_error = errCanceled;
	rdy_pkt1 * pkt1 = ( rdy_pkt1 * )data;
	size_t sz_t = len - pkt1->flags.payload_offset;
	WARNING( pkt1->flags.version == TCP_PACKET_V1 );

	time_t tnow = 0;
	tnow = time( NULL );
	sockfd fd = -1;
	void_p ab_tcp_p = NULL;
	AB * pb = NULL;
	// fast method
	if ( dict_fst_get_faster_by_hash_id( &_g->hdls.map_tcp_socket , pkt1->flags.tcp_name_key_hash , pkt1->flags.tcp_name_uniq_id , &fd , &ab_tcp_p ) == errOK )
	{
		d_error = sendall( fd , data + pkt1->flags.payload_offset , &sz_t ); // send is to heavy
		if ( d_error == errOK )
		{
			pkt1->flags.sent = true;
			if ( ab_tcp_p )
			{
				AB_tcp * ptcp = ( AB_tcp * )ab_tcp_p;
				pb = ptcp->owner_pb; // not safe at all . but for now is ok . TODO . fix this error prone potentially faulty part
			}
		}
	}
	if ( !pkt1->flags.sent ) // slow method
	{
		for ( int iab = 0 ; iab < _g->bridges.ABhs_masks_count ; iab++ )
		{
			if ( _g->bridges.ABhs_masks[ iab ] )
			{
				for ( int itcp = 0 ; itcp < _g->bridges.ABs[ iab ].single_AB->tcps_count ; itcp++ )
				{
					if ( STR_SAME( _g->bridges.ABs[ iab ].single_AB->tcps[ itcp ].__tcp_cfg_pak->name , pkt1->TCP_name ) )
					{
						pb = _g->bridges.ABs[ iab ].single_AB;
						if ( _g->bridges.ABs[ iab ].single_AB->tcps[ itcp ].tcp_connection_established )
						{
							d_error = sendall( _g->bridges.ABs[ iab ].single_AB->tcps[ itcp ].tcp_sockfd , data + pkt1->flags.payload_offset , &sz_t ); // send is to heavy
							if ( d_error == errOK )
							{
								pkt1->flags.sent = true;
							}
							break;
						}
					}
				}
				if ( pkt1->flags.sent )
				{
					break;
				}
			}
		}
	}
	if ( pkt1->flags.sent && pb )
	{
		pb->stat.round_zero_set.tcp.total_tcp_put_count++;
		pb->stat.round_zero_set.tcp.total_tcp_put_byte += sz_t;
		pb->stat.round_zero_set.tcp_1_sec.calc_throughput_tcp_put_count++;
		pb->stat.round_zero_set.tcp_1_sec.calc_throughput_tcp_put_bytes += sz_t;
		pb->stat.round_zero_set.tcp_send_data_alive_indicator++;
	}

	if ( difftime( tnow , pb->stat.round_zero_set.tcp_1_sec.t_tcp_throughput ) >= 1.0 )
	{
		if ( pb->stat.round_zero_set.tcp_1_sec.t_tcp_throughput > 0 )
		{
			cbuf_m_advance( &pb->stat.round_init_set.tcp_stat_5_sec_count , pb->stat.round_zero_set.tcp_1_sec.calc_throughput_tcp_put_count );
			cbuf_m_advance( &pb->stat.round_init_set.tcp_stat_5_sec_bytes , pb->stat.round_zero_set.tcp_1_sec.calc_throughput_tcp_put_bytes );

			cbuf_m_advance( &pb->stat.round_init_set.tcp_stat_10_sec_count , pb->stat.round_zero_set.tcp_1_sec.calc_throughput_tcp_put_count );
			cbuf_m_advance( &pb->stat.round_init_set.tcp_stat_10_sec_bytes , pb->stat.round_zero_set.tcp_1_sec.calc_throughput_tcp_put_bytes );

			cbuf_m_advance( &pb->stat.round_init_set.tcp_stat_40_sec_count , pb->stat.round_zero_set.tcp_1_sec.calc_throughput_tcp_put_count );
			cbuf_m_advance( &pb->stat.round_init_set.tcp_stat_40_sec_bytes , pb->stat.round_zero_set.tcp_1_sec.calc_throughput_tcp_put_bytes );
		}
		pb->stat.round_zero_set.tcp_1_sec.t_tcp_throughput = tnow;
		pb->stat.round_zero_set.tcp_1_sec.calc_throughput_tcp_put_count = 0;
		pb->stat.round_zero_set.tcp_1_sec.calc_throughput_tcp_put_bytes = 0;
	}

	return d_error;
}

_THREAD_FXN void_p process_filled_tcp_segment_proc( pass_p src_g )
{
	G * _g = ( G * )src_g;
	ci_sgm_t * pseg = NULL;
	
	do
	{
		if ( CLOSE_APP_VAR() ) break;

		pseg = segmgr_pop_filled_segment( &_g->bufs.aggr_inp_pkt , True , seg_trv_LIFO );
		if ( pseg )
		{
			ci_sgm_iter_items( pseg , process_segment_itm , src_g );
			ci_sgm_mark_empty( &_g->bufs.aggr_inp_pkt , pseg );
		}
	}
	while( 1 );
	return NULL;
}

_PRIVATE_FXN _CALLBACK_FXN bool peek_decide_active_sgm( const buffer buf , size_t sz )
{
	rdy_pkt1 * pkt1 = ( rdy_pkt1 * )buf;
	WARNING( pkt1->flags.version == TCP_PACKET_V1 );

	struct timespec tnow;
	clock_gettime( CLOCK_MONOTONIC_COARSE , &tnow );
	return timespec_diff_ms( &pkt1->flags.rec_t , &tnow ) > _g->appcfg.g_cfg->c.c.pkt_mgr_maximum_keep_unfinished_segment_sec;
}

_CALLBACK_FXN void release_halffill_segment( pass_p src_g )
{
	G * _g = ( G * )src_g;
	ci_sgm_peek_decide_active( &_g->bufs.aggr_inp_pkt , peek_decide_active_sgm );
}
