#define Uses_timeval_diff_ms
#define Uses_STR_SAME
#define Uses_WARNING
#define Uses_packet_mngr
#define Uses_MEMSET_ZERO_O
#define Uses_errno
#define Uses_helper
#define Uses_Bridge
#define Uses_INIT_BREAKABLE_FXN
//#define Uses_DGB

#include <Protocol_Bridge.dep>


//CODE_DBG_TOOLS();

GLOBAL_VAR extern G * _g;

_CALLBACK_FXN _PRIVATE_FXN void pre_config_init_packet_mngr( void_p src_g )
{
	G * _g = ( G * )src_g;

	distributor_init( &_g->hdls.pkt_mgr.throttling_release_halffill_segment , 1 );
	dict_fst_create( &_g->hdls.pkt_mgr.map_tcp_socket , 0 );
	distributor_subscribe( &_g->hdls.pkt_mgr.throttling_release_halffill_segment , SUB_VOID , SUB_FXN( release_halffill_segment ) , _g );
}

_CALLBACK_FXN _PRIVATE_FXN void post_config_init_packet_mngr( void_p src_g )
{
	INIT_BREAKABLE_FXN();
	G * _g = ( G * )src_g;

	segmgr_init( &_g->hdls.pkt_mgr.aggr_inp_pkt , ( size_t )_g->appcfg.g_cfg->c.c.pkt_mgr_segment_capacity , ( size_t )_g->appcfg.g_cfg->c.c.pkt_mgr_offsets_capacity , True );
	MM_BREAK_IF( pthread_create( &_g->hdls.pkt_mgr.trd_tcp_sender , NULL , process_filled_tcp_segment_proc , ( pass_p )_g ) != PTHREAD_CREATE_OK , errCreation , 0 , "Failed to create tcp_sender thread" );
	segmgr_init( &_g->hdls.pkt_mgr.sent_package_log , 3200000 , 100000 , True );
	M_BREAK_STAT( distributor_subscribe( &_g->hdls.prst_csh.pagestack_pakcets , SUB_DIRECT_ONE_CALL_BUFFER_SIZE , SUB_FXN( descharge_persistent_storage_data ) , _g ) , 0 );
	
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
/// most be fast . but its not necessary to be fast as defragment_pcap_data
/// </summary>
_CALLBACK_FXN status operation_on_tcp_packet( pass_p data , buffer buf , size_t sz )
{
	INIT_BREAKABLE_FXN();

	AB_tcp * tcp = ( AB_tcp * )data;
	AB * pb = tcp->owner_pb;
	G * _g = ( G * )tcp->owner_pb->cpy_cfg.m.m.temp_data._pseudo_g;

	status ret = segmgr_append( &_g->hdls.pkt_mgr.aggr_inp_pkt , buf , sz ); // store whole pakt + hdr into global buffer
	RANJE_ACT1( ret , errArg , NULL_ACT , MACRO_E( M_BREAK_STAT( ret , 0 ) ) );

	// TODO . add record to file if memory about to full
	
	BEGIN_SMPL
	M_END_RET
}

// 1 . try to send packet under tcp to destination
// TODO . multi thread entrance . be quite
_PRIVATE_FXN _CALLBACK_FXN status process_segment_itm( buffer data , size_t len , pass_p src_g )
{
	G * _g = ( G * )src_g;
	status d_error = errCanceled;
	rdy_pkt1 * pkt1 = ( rdy_pkt1 * )data;
	size_t sz_t = len - pkt1->metadata.payload_offset;
	WARNING( pkt1->metadata.version == TCP_PACKET_V1 );
	bool try_resolve_route = false;

	if ( pkt1->metadata.sent ) return errOK;

	time_t tnow = 0;
	tnow = time( NULL );
	sockfd fd = -1;
	void_p ab_tcp_p = NULL;
	AB * pb = NULL;

	// normal packet come and go. retried packet should just checked
	if ( pkt1->metadata.fault_registered ) // faulty item should not have too many attempt
	{
		if ( pkt1->metadata.cool_down_attempt && pkt1->metadata.cool_down_attempt == *( uchar * )&tnow ) // in one second we should not attempt . and this check and possiblity is rare in not too many attempt situation
		{
			return errTooManyAttempt;
		}
		pkt1->metadata.cool_down_attempt = *( uchar * )&tnow;
	}

	// fast method
	if ( dict_fst_get_faster_by_hash_id( &_g->hdls.pkt_mgr.map_tcp_socket , pkt1->metadata.tcp_name_key_hash , pkt1->metadata.tcp_name_uniq_id , &fd , &ab_tcp_p ) == errOK && fd != INVALID_FD )
	{
		d_error = tcp_send_all( fd , data + pkt1->metadata.payload_offset , sz_t , 0 , 0 ); // send is too heavy
		switch ( d_error )
		{
			case errOK:
			{
				pkt1->metadata.sent = true;
				if ( ab_tcp_p )
				{
					AB_tcp * ptcp = ( AB_tcp * )ab_tcp_p;
					pb = ptcp->owner_pb; // not safe at all . but for now is ok . TODO . fix this error prone potentially faulty part
				}
				break;
			}
			case errNoPeer:
			case errPeerClosed:
			{
				if ( ab_tcp_p )
				{
					AB_tcp * ptcp = ( AB_tcp * )ab_tcp_p;
					if ( !ptcp->tcp_is_about_to_connect ) // if another request is attempt then we should waint to complete
					{
						ptcp->tcp_is_about_to_connect = 1;
						//ptcp->retry_to_connect_tcp = 1;
						ptcp->tcp_connection_established = 0;
					}
					pb = ptcp->owner_pb; // not safe at all . but for now is ok . TODO . fix this error prone potentially faulty part
				}
				break;
			}
		}
	}
	else
	{
		try_resolve_route = true;
	}
	if ( !pkt1->metadata.sent ) // slow method
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
							d_error = tcp_send_all( _g->bridges.ABs[ iab ].single_AB->tcps[ itcp ].tcp_sockfd , data + pkt1->metadata.payload_offset , sz_t , 0 , 0 ); // send is to heavy
							switch ( d_error )
							{
								case errOK:
								{
									pkt1->metadata.sent = true;

									if ( try_resolve_route )
									{
										try_resolve_route = false;
										dict_fst_put( &_g->hdls.pkt_mgr.map_tcp_socket , pkt1->TCP_name , _g->bridges.ABs[ iab ].single_AB->tcps[ itcp ].tcp_sockfd , ( void_p )( AB_tcp * )&_g->bridges.ABs[ iab ].single_AB->tcps[ itcp ] , NULL , NULL , NULL );
									}
									break;
								}
								case errNoPeer:
								case errPeerClosed:
								{
									if ( ab_tcp_p )
									{
										if ( !_g->bridges.ABs[ iab ].single_AB->tcps[ itcp ].tcp_is_about_to_connect )
										{
											_g->bridges.ABs[ iab ].single_AB->tcps[ itcp ].tcp_is_about_to_connect = 1;
											//_g->bridges.ABs[ iab ].single_AB->tcps[ itcp ].retry_to_connect_tcp = 1;
											_g->bridges.ABs[ iab ].single_AB->tcps[ itcp ].tcp_connection_established = 0;
										}
									}
									break;
								}
							}
							break;
						}
					}
				}
				if ( pkt1->metadata.sent )
				{
					break;
				}
			}
		}
	}
	
	if ( !pkt1->metadata.sent && pkt1->metadata.retry )
	{
		pkt1->metadata.retry = false;
		pkt1->metadata.retried = true;
		return process_segment_itm( data , len , src_g ); // retry
	}

	// under here d_error could not be change because its used as succesful sending

	if ( pkt1->metadata.sent )
	{
		if ( pb )
		{
			pb->stat.round_zero_set.tcp.total_tcp_put_count++;
			pb->stat.round_zero_set.tcp.total_tcp_put_byte += sz_t;
			pb->stat.round_zero_set.tcp_1_sec.calc_throughput_tcp_put_count++;
			pb->stat.round_zero_set.tcp_1_sec.calc_throughput_tcp_put_bytes += sz_t;
			pb->stat.round_zero_set.tcp_send_data_alive_indicator++;
		}
	}
	else
	{
		// add log
		if ( !pkt1->metadata.udp_hdr.logged_2_mem )
		{
			if ( segmgr_append( &_g->hdls.pkt_mgr.sent_package_log , &pkt1->metadata.udp_hdr , sizeof( pkt1->metadata.udp_hdr ) ) == errOK )
			{
				pkt1->metadata.udp_hdr.logged_2_mem = true;
			}
		}
	}

	if ( pb )
	{
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
	}

	return d_error;
}

// 2 . if sending under tcp fail try to archive them( segment )
_PRIVATE_FXN _CALLBACK_FXN status process_faulty_itm( buffer data , size_t len , pass_p src_g )
{
	G * _g = ( G * )src_g;
	status d_error = errCanceled;
	rdy_pkt1 * pkt1 = ( rdy_pkt1 * )data;
	size_t sz_t = len - pkt1->metadata.payload_offset;

	if ( pkt1->metadata.sent || pkt1->metadata.fault_registered ) return errOK;
	
	pkt1->metadata.fault_registered = 1;

	// TODO . do something with faulty item
	// try to seperate requestor and actually archiver. so i drop it on ground and coursed stinky person grab it
	distributor_publish_buffer_size( &_g->hdls.prst_csh.store_data , data , len , src_g );

	// write it down on disk
	// later we can find logged that does not sent and find the bug
	
	return d_error;
}

_CALLBACK_FXN status descharge_persistent_storage_data( pass_p src_g , buffer buf , size_t sz )
{
	G * _g = ( G * )src_g;
	return process_segment_itm( buf , sz , src_g );
}

// emptied buffer cache( aggr_inp_pkt ) then on failure go to persistent storage cache and get from it
// this fxn do empty segment by segment
_THREAD_FXN void_p process_filled_tcp_segment_proc( pass_p src_g )
{
	G * _g = ( G * )src_g;
	ci_sgm_t * pseg = NULL;
	
	struct
	{
		union
		{
			struct
			{
				long a , b;
			} big_data;
			char bytes[16];
		} arr;
		uchar idx;
	} cpu_unburne = {0}; // suppress long time useless fetch rate

	do
	{
		if ( CLOSE_APP_VAR() ) break;

		//SYS_ALIVE_CHECK();

		pseg = segmgr_pop_filled_segment( &_g->hdls.pkt_mgr.aggr_inp_pkt , False , seg_trv_LIFO );
		if ( pseg ) // poped on memory packets
		{
		//SYS_ALIVE_CHECK();

			cpu_unburne.idx = ( cpu_unburne.idx + 1 ) % sizeof( cpu_unburne.arr );
			cpu_unburne.arr.bytes[ cpu_unburne.idx ] = 1;
			distributor_publish_int( &_g->hdls.prst_csh.pagestack_gateway_status , 0 , NULL ); // stop persistent storage discharge

			// try to send from mem under tcp to dst
			if ( ci_sgm_iter_items( pseg , process_segment_itm , src_g , true ) != errOK ) // some fault detected
			{
				// if sending filled segment fail try to archive them
				ci_sgm_iter_items( pseg , process_faulty_itm , src_g , true );
			}
			// then close segment
			ci_sgm_mark_empty( &_g->hdls.pkt_mgr.aggr_inp_pkt , pseg ); // pop last emptied segment

			//SYS_ALIVE_CHECK();
		}
		else // there is no packet in memory so fetch persisted packets
		{
			cpu_unburne.idx = ( cpu_unburne.idx + 1 ) % sizeof( cpu_unburne.arr );
			cpu_unburne.arr.bytes[ cpu_unburne.idx ] = 0;
			distributor_publish_int( &_g->hdls.prst_csh.pagestack_gateway_status , 1 , NULL ); // resume persistent storage discharge

			//SYS_ALIVE_CHECK();
		}
		
		//SYS_ALIVE_CHECK();

		if ( !cpu_unburne.arr.big_data.a && !cpu_unburne.arr.big_data.b ) // enough time for packet arrive
		{
			mng_basic_thread_sleep( _g , HI_PRIORITY_THREAD );
		}
	}
	while( 1 );
	distributor_publish_int( &_g->hdls.prst_csh.pagestack_gateway_status , 0 , NULL ); // stop persistent storage discharge
	return NULL;
}

// check first packet of segment then if packet pending too long then close segment
_PRIVATE_FXN _CALLBACK_FXN bool peek_decide_active_sgm( const buffer buf , size_t sz )
{
	rdy_pkt1 * pkt1 = ( rdy_pkt1 * )buf;
	WARNING( pkt1->metadata.version == TCP_PACKET_V1 );

	struct timeval tnow;
	gettimeofday( &tnow , NULL );
	return timeval_diff_ms( &pkt1->metadata.udp_hdr.tm , &tnow ) > _g->appcfg.g_cfg->c.c.pkt_mgr_maximum_keep_unfinished_segment_sec;
}

_CALLBACK_FXN void release_halffill_segment( pass_p src_g )
{
	G * _g = ( G * )src_g;
	ci_sgm_peek_decide_active( &_g->hdls.pkt_mgr.aggr_inp_pkt , peek_decide_active_sgm );
}
