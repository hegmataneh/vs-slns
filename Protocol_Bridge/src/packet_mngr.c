#define Uses_packet_mngr
#define Uses_MEMSET_ZERO_O
#define Uses_errno
#define Uses_helper
#define Uses_Bridge
#define Uses_INIT_BREAKABLE_FXN

#include <Protocol_Bridge.dep>

/// <summary>
/// from each of the rings to global buffer
/// </summary>
_CALLBACK_FXN status operation_on_tcp_packet( pass_p data , buffer buf , int sz )
{
	INIT_BREAKABLE_FXN();

	AB_tcp * tcp = ( AB_tcp * )data;
	AB * pb = tcp->owner_pb;
	G * _g = ( G * )tcp->owner_pb->cpy_cfg.m.m.temp_data._g;

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
	WARNING( pkt1->flags.version == TCP_PACKET_VERSION );

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

