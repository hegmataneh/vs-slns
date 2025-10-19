#define Uses_iSTR_SAME
#define Uses_WARNING
#define Uses_init_udps_defragmentator
#define Uses_proc_krnl_udp_capture
#define Uses_pthread_create
#define Uses_proc_many2many_pcap_krnl_SF
#define Uses_proc_one2many_pcap2krnl_SF_udp_pcap
#define Uses_proc_krnl_udp_counter
#define Uses_proc_pcap_udp_counter
#define Uses_proc_one2one_pcap2krnl_SF_udp_pcap
#define Uses_MALLOC_AR
#define Uses_INIT_BREAKABLE_FXN
#define Uses_pthread_t
#define Uses_Bridge
#define Uses_globals
#include <Protocol_Bridge.dep>

_CALLBACK_FXN void stop_sending_by_bridge( pass_p src_g , long v )
{
	G * _g = ( G * )src_g;
	for ( size_t idx = 0 ; idx < _g->bridges.ABhs_masks_count ; idx++ )
	{
		if ( _g->bridges.ABhs_masks[ idx ] )
		{
			for ( _g->bridges.ABs[ idx ].single_AB->trd.cmn.stop_sending = 1 ; !_g->bridges.ABs[ idx ].single_AB->trd.cmn.send_stoped ; sleep( 1 ) ) // order to stop. then after stop continue to clean up
			{
			}
		}
	}
	MARK_LINE();
}


_CALLBACK_FXN void bridge_stoping_input( pass_p src_pb , long v )
{
	AB * pb = ( AB * )src_pb;
	if ( pb )
	{
		for ( pb->trd.cmn.stop_receiving = true ; !pb->trd.cmn.receive_stoped ; sleep( 1 ) ) // order to stop. then after stop continue to clean up
		{
		}
	}
	MARK_LINE();
}

_CALLBACK_FXN void cleanup_sending_part_bridge( pass_p src_pb , long v )
{
	AB * pb = ( AB * )src_pb;
	if ( pb )
	{
		for ( pb->trd.cmn.stop_receiving = true ; !pb->trd.cmn.receive_stoped ; sleep(1) ) // order to stop. then after stop continue to clean up
		{ }

		cbuf_pked_destroy( &pb->trd.cmn.fast_wrt_cache );
		finalize_udps_defragmentator( &pb->trd.cmn.defraged_udps );

		sub_destroy( &pb->trd.cmn.kernel_udp_payload_ready_event );
		sub_destroy( &pb->trd.cmn.fragmented_udp_packet_on_pcap_received_event );
		sub_destroy( &pb->trd.cmn.defraged_pcap_udp_payload_event );
	}
	MARK_LINE();
}

_CALLBACK_FXN void cleanup_bridges( pass_p src_g , long v )
{
	G * _g = ( G * )src_g;
	
	for ( size_t idx = 0 ; idx < _g->bridges.ABhs_masks_count ; idx++ )
	{
		if ( _g->bridges.ABhs_masks[ idx ] )
		{
			AB * pb = _g->bridges.ABs->single_AB;
			if ( pb )
			{
				cleaup_brg_cfg( &pb->cpy_cfg );

				DAC( pb->trd.t.p_pcap_udp_counter );
				DAC( pb->trd.t.p_krnl_udp_counter );
				DAC( pb->trd.t.p_one2one_krnl2krnl_SF );
				DAC( pb->trd.t.p_one2one_pcap2krnl_SF );

				if ( pb->trd.t.p_one2many_pcap2krnl_SF )
				{
					dict_o_free( &pb->trd.t.p_one2many_pcap2krnl_SF->dc_token_ring );
					DAC( pb->trd.t.p_one2many_pcap2krnl_SF );
				}

				if ( pb->trd.t.p_many2one_pcap2krnl_SF_serialize )
				{
					dict_o_free( &pb->trd.t.p_many2one_pcap2krnl_SF_serialize->dc_token_ring );
					DAC( pb->trd.t.p_many2one_pcap2krnl_SF_serialize );
				}

				cbuf_m_free( &pb->stat.round_init_set.udp_stat_5_sec_count );
				cbuf_m_free( &pb->stat.round_init_set.udp_stat_5_sec_bytes );
				cbuf_m_free( &pb->stat.round_init_set.udp_stat_10_sec_count );
				cbuf_m_free( &pb->stat.round_init_set.udp_stat_10_sec_bytes );
				cbuf_m_free( &pb->stat.round_init_set.udp_stat_40_sec_count );
				cbuf_m_free( &pb->stat.round_init_set.udp_stat_40_sec_bytes );
				cbuf_m_free( &pb->stat.round_init_set.tcp_stat_5_sec_count );
				cbuf_m_free( &pb->stat.round_init_set.tcp_stat_5_sec_bytes );
				cbuf_m_free( &pb->stat.round_init_set.tcp_stat_10_sec_count );
				cbuf_m_free( &pb->stat.round_init_set.tcp_stat_10_sec_bytes );
				cbuf_m_free( &pb->stat.round_init_set.tcp_stat_40_sec_count );
				cbuf_m_free( &pb->stat.round_init_set.tcp_stat_40_sec_bytes );

				//DAC( pb->udps );
				//DAC( pb->tcps );
			}
			DAC( pb );
		}
	}
	DAC( _g->bridges.ABhs_masks );
	DAC( _g->bridges.ABs );
	MARK_LINE();
}


_CALLBACK_FXN void pb_every_ticking_refresh( pass_p src_pb )
{
	AB * pb = ( AB * )src_pb;

	nnc_cell_triggered( pb->stat.pb_elapse_cell );

	nnc_cell_triggered( pb->stat.pb_total_udp_get_count_cell );
	nnc_cell_triggered( pb->stat.pb_total_udp_get_byte_cell );
	nnc_cell_triggered( pb->stat.pb_total_tcp_put_count_cell );
	nnc_cell_triggered( pb->stat.pb_total_tcp_put_byte_cell );
	nnc_cell_triggered( pb->stat.pb_5s_udp_pps );
	nnc_cell_triggered( pb->stat.pb_5s_udp_bps );
	nnc_cell_triggered( pb->stat.pb_10s_udp_pps );
	nnc_cell_triggered( pb->stat.pb_10s_udp_bps );
	nnc_cell_triggered( pb->stat.pb_40s_udp_pps );
	nnc_cell_triggered( pb->stat.pb_40s_udp_bps );
	nnc_cell_triggered( pb->stat.pb_5s_tcp_pps );
	nnc_cell_triggered( pb->stat.pb_5s_tcp_bps );
	nnc_cell_triggered( pb->stat.pb_10s_tcp_pps );
	nnc_cell_triggered( pb->stat.pb_10s_tcp_bps );

	nnc_cell_triggered( pb->stat.pb_40s_tcp_pps );
	nnc_cell_triggered( pb->stat.pb_40s_tcp_bps );
}

/// <summary>
/// init active udp tcp structure by the bridge config
/// </summary>
_PRIVATE_FXN void init_ActiveBridge( G * _g , AB * pb )
{
	INIT_BREAKABLE_FXN();

	if ( pb->cpy_cfg.m.m.maintained.in_count > 0 )
	{
		M_BREAK_IF( !( pb->udps = MALLOC_AR( pb->udps , pb->cpy_cfg.m.m.maintained.in_count ) ) , errMemoryLow , 1 );
		MEMSET_ZERO( pb->udps , pb->cpy_cfg.m.m.maintained.in_count );
		pb->udps_count = pb->cpy_cfg.m.m.maintained.in_count; // caution . count is always set after main ptr initialized

		for ( int iudp = 0 ; iudp < pb->udps_count ; iudp++ )
		{
			pb->udps[ iudp ].owner_pb = pb;
			pb->udps[ iudp ].__udp_cfg_pak = pb->cpy_cfg.m.m.maintained.in + iudp;
			//distributor_init( &pb->udps[ iudp ].change_state_dist , 1 );
		}
	}
	if ( pb->cpy_cfg.m.m.maintained.out_count > 0 )
	{
		M_BREAK_IF( !( pb->tcps = MALLOC_AR( pb->tcps , pb->cpy_cfg.m.m.maintained.out_count ) ) , errMemoryLow , 1 );
		MEMSET_ZERO( pb->tcps , pb->cpy_cfg.m.m.maintained.out_count );
		pb->tcps_count = pb->cpy_cfg.m.m.maintained.out_count;

		for ( int itcp = 0 ; itcp < pb->tcps_count ; itcp++ )
		{
			pb->tcps[ itcp ].owner_pb = pb;
			pb->tcps[ itcp ].__tcp_cfg_pak = pb->cpy_cfg.m.m.maintained.out + itcp;
			distributor_init( &pb->tcps[ itcp ].change_state_dist , 1 );
		}
	}

	// TODO . if Ab goes away then unregister quit intrupt
	distributor_subscribe_withOrder( &_g->distributors.quit_interrupt_dist , SUB_LONG , SUB_FXN( bridge_stoping_input ) , pb , bridge_stop_input );

	distributor_subscribe_withOrder( &_g->distributors.quit_interrupt_dist , SUB_LONG , SUB_FXN( cleanup_sending_part_bridge ) , pb , clean_bridge_send_part );

	distributor_subscribe_withOrder( &_g->distributors.quit_interrupt_dist , SUB_LONG , SUB_FXN( cleanup_bridges ) , _g , clean_globals_shared_var );
	
	

	#ifndef notcurses_Section

	cbuf_m_init( &pb->stat.round_init_set.udp_stat_5_sec_count , 5 );
	cbuf_m_init( &pb->stat.round_init_set.udp_stat_10_sec_count , 10 );
	cbuf_m_init( &pb->stat.round_init_set.udp_stat_40_sec_count , 40 );

	cbuf_m_init( &pb->stat.round_init_set.udp_stat_5_sec_bytes , 5 );
	cbuf_m_init( &pb->stat.round_init_set.udp_stat_10_sec_bytes , 10 );
	cbuf_m_init( &pb->stat.round_init_set.udp_stat_40_sec_bytes , 40 );

	cbuf_m_init( &pb->stat.round_init_set.tcp_stat_5_sec_count , 5 );
	cbuf_m_init( &pb->stat.round_init_set.tcp_stat_10_sec_count , 10 );
	cbuf_m_init( &pb->stat.round_init_set.tcp_stat_40_sec_count , 40 );

	cbuf_m_init( &pb->stat.round_init_set.tcp_stat_5_sec_bytes , 5 );
	cbuf_m_init( &pb->stat.round_init_set.tcp_stat_10_sec_bytes , 10 );
	cbuf_m_init( &pb->stat.round_init_set.tcp_stat_40_sec_bytes , 40 );


	M_BREAK_STAT( nnc_add_table( &_g->stat.nc_h , pb->cpy_cfg.m.m.id.short_name , &pb->ab_stat_tbl ) , 0 );
	nnc_table * ptbl = pb->ab_stat_tbl;
	// col
	M_BREAK_STAT( nnc_add_column( ptbl , "" , "" , 0 ) , 0 );
	M_BREAK_STAT( nnc_add_column( ptbl , "" , "" , 20 ) , 0 );
	M_BREAK_STAT( nnc_add_column( ptbl , "" , "" , 0 ) , 0 );
	M_BREAK_STAT( nnc_add_column( ptbl , "" , "" , 20 ) , 0 );

	int irow = -1;

	irow++;
	M_BREAK_STAT( nnc_add_empty_row( ptbl , NULL ) , 0 );
	// elapse time title
	M_BREAK_STAT( nnc_set_static_text( ptbl , irow , 0 , "elapse" ) , 0 );
	// elapse time cell
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pb->stat.pb_elapse_cell ) , 0 );
	pb->stat.pb_elapse_cell->storage.bt.pass_data = pb;
	pb->stat.pb_elapse_cell->conversion_fxn = pb_time_elapse_2_str;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , irow , 1 , pb->stat.pb_elapse_cell ) , 0 );

	// fault title
	M_BREAK_STAT( nnc_set_static_text( ptbl , irow , 2 , "fault" ) , 0 );
	// elapse time cell
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pb->stat.pb_fault_cell ) , 0 );
	pb->stat.pb_fault_cell->storage.bt.pass_data = pb;
	pb->stat.pb_fault_cell->conversion_fxn = pb_fault_2_str;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , irow , 3 , pb->stat.pb_fault_cell ) , 0 );

	irow++;
	M_BREAK_STAT( nnc_add_empty_row( ptbl , NULL ) , 0 );
	// UDP conn title
	M_BREAK_STAT( nnc_set_static_text( ptbl , irow , 0 , "UDP conn" ) , 0 );
	// UDP conn cell
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pb->stat.pb_UDP_conn_cell ) , 0 );
	pb->stat.pb_UDP_conn_cell->storage.bt.pass_data = pb;
	pb->stat.pb_UDP_conn_cell->conversion_fxn = pb_UDP_conn_2_str;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , irow , 1 , pb->stat.pb_UDP_conn_cell ) , 0 );
	//// TCP conn title
	M_BREAK_STAT( nnc_set_static_text( ptbl , irow , 2 , "TCP conn" ) , 0 );
	// TCP conn cell
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pb->stat.pb_TCP_conn_cell ) , 0 );
	pb->stat.pb_TCP_conn_cell->storage.bt.pass_data = pb;
	pb->stat.pb_TCP_conn_cell->conversion_fxn = pb_TCP_conn_2_str;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , irow , 3 , pb->stat.pb_TCP_conn_cell ) , 0 );

	irow++;
	M_BREAK_STAT( nnc_add_empty_row( ptbl , NULL ) , 0 );
	//// UDP retry conn title
	M_BREAK_STAT( nnc_set_static_text( ptbl , irow , 0 , "UDP retry" ) , 0 );
	// UDP conn cell
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pb->stat.pb_UDP_retry_conn_cell ) , 0 );
	pb->stat.pb_UDP_retry_conn_cell->storage.bt.pass_data = pb;
	pb->stat.pb_UDP_retry_conn_cell->conversion_fxn = pb_UDP_retry_2_str;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , irow , 1 , pb->stat.pb_UDP_retry_conn_cell ) , 0 );
	//// TCP retry conn title
	M_BREAK_STAT( nnc_set_static_text( ptbl , irow , 2 , "TCP retry" ) , 0 );
	// TCP conn cell
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pb->stat.pb_TCP_retry_conn_cell ) , 0 );
	pb->stat.pb_TCP_retry_conn_cell->storage.bt.pass_data = pb;
	pb->stat.pb_TCP_retry_conn_cell->conversion_fxn = pb_TCP_retry_2_str;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , irow , 3 , pb->stat.pb_TCP_retry_conn_cell ) , 0 );

	irow++;
	M_BREAK_STAT( nnc_add_empty_row( ptbl , NULL ) , 0 );
	//// udp_get_count
	M_BREAK_STAT( nnc_set_static_text( ptbl , irow , 0 , "UDP get" ) , 0 );
	// udp_get_count
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pb->stat.pb_total_udp_get_count_cell ) , 0 );
	pb->stat.pb_total_udp_get_count_cell->storage.bt.pass_data = pb;
	pb->stat.pb_total_udp_get_count_cell->conversion_fxn = pb_UDP_get_count_2_str;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , irow , 1 , pb->stat.pb_total_udp_get_count_cell ) , 0 );
	//// udp_get_byte
	M_BREAK_STAT( nnc_set_static_text( ptbl , irow , 2 , "UDP get" ) , 0 );
	// udp_get_byte
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pb->stat.pb_total_udp_get_byte_cell ) , 0 );
	pb->stat.pb_total_udp_get_byte_cell->storage.bt.pass_data = pb;
	pb->stat.pb_total_udp_get_byte_cell->conversion_fxn = pb_UDP_get_byte_2_str;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , irow , 3 , pb->stat.pb_total_udp_get_byte_cell ) , 0 );

	irow++;
	M_BREAK_STAT( nnc_add_empty_row( ptbl , NULL ) , 0 );
	//// tcp_put_count
	M_BREAK_STAT( nnc_set_static_text( ptbl , irow , 0 , "TCP sent" ) , 0 );
	// tcp_put_count
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pb->stat.pb_total_tcp_put_count_cell ) , 0 );
	pb->stat.pb_total_tcp_put_count_cell->storage.bt.pass_data = pb;
	pb->stat.pb_total_tcp_put_count_cell->conversion_fxn = pb_TCP_put_count_2_str;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , irow , 1 , pb->stat.pb_total_tcp_put_count_cell ) , 0 );
	//// tcp_put_byte
	M_BREAK_STAT( nnc_set_static_text( ptbl , irow , 2 , "TCP sent" ) , 0 );
	// tcp_put_byte
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pb->stat.pb_total_tcp_put_byte_cell ) , 0 );
	pb->stat.pb_total_tcp_put_byte_cell->storage.bt.pass_data = pb;
	pb->stat.pb_total_tcp_put_byte_cell->conversion_fxn = pb_TCP_put_byte_2_str;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , irow , 3 , pb->stat.pb_total_tcp_put_byte_cell ) , 0 );

	irow++;
	M_BREAK_STAT( nnc_add_empty_row( ptbl , NULL ) , 0 );
	//// 5s_udp_pps
	M_BREAK_STAT( nnc_set_static_text( ptbl , irow , 0 , "5s_udp_pps" ) , 0 );
	// 5s_udp_pps
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pb->stat.pb_5s_udp_pps ) , 0 );
	pb->stat.pb_5s_udp_pps->storage.bt.pass_data = pb;
	pb->stat.pb_5s_udp_pps->conversion_fxn = pb_5s_udp_pps_2_str;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , irow , 1 , pb->stat.pb_5s_udp_pps ) , 0 );
	//// 5s_udp_bps
	M_BREAK_STAT( nnc_set_static_text( ptbl , irow , 2 , "5s_udp_bps" ) , 0 );
	// 5s_udp_bps
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pb->stat.pb_5s_udp_bps ) , 0 );
	pb->stat.pb_5s_udp_bps->storage.bt.pass_data = pb;
	pb->stat.pb_5s_udp_bps->conversion_fxn = pb_5s_udp_bps_2_str;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , irow , 3 , pb->stat.pb_5s_udp_bps ) , 0 );

	irow++;
	M_BREAK_STAT( nnc_add_empty_row( ptbl , NULL ) , 0 );
	//// 10s_udp_pps
	M_BREAK_STAT( nnc_set_static_text( ptbl , irow , 0 , "10s_udp_pps" ) , 0 );
	// 10s_udp_pps
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pb->stat.pb_10s_udp_pps ) , 0 );
	pb->stat.pb_10s_udp_pps->storage.bt.pass_data = pb;
	pb->stat.pb_10s_udp_pps->conversion_fxn = pb_10s_udp_pps_2_str;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , irow , 1 , pb->stat.pb_10s_udp_pps ) , 0 );
	//// 10s_udp_bps
	M_BREAK_STAT( nnc_set_static_text( ptbl , irow , 2 , "10s_udp_bps" ) , 0 );
	// 10s_udp_bps
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pb->stat.pb_10s_udp_bps ) , 0 );
	pb->stat.pb_10s_udp_bps->storage.bt.pass_data = pb;
	pb->stat.pb_10s_udp_bps->conversion_fxn = pb_10s_udp_bps_2_str;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , irow , 3 , pb->stat.pb_10s_udp_bps ) , 0 );

	irow++;
	M_BREAK_STAT( nnc_add_empty_row( ptbl , NULL ) , 0 );
	//// 40s_udp_pps
	M_BREAK_STAT( nnc_set_static_text( ptbl , irow , 0 , "40s_udp_pps" ) , 0 );
	// 40s_udp_pps
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pb->stat.pb_40s_udp_pps ) , 0 );
	pb->stat.pb_40s_udp_pps->storage.bt.pass_data = pb;
	pb->stat.pb_40s_udp_pps->conversion_fxn = pb_40s_udp_pps_2_str;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , irow , 1 , pb->stat.pb_40s_udp_pps ) , 0 );
	//// 40s_udp_bps
	M_BREAK_STAT( nnc_set_static_text( ptbl , irow , 2 , "40s_udp_bps" ) , 0 );
	// 40s_udp_bps
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pb->stat.pb_40s_udp_bps ) , 0 );
	pb->stat.pb_40s_udp_bps->storage.bt.pass_data = pb;
	pb->stat.pb_40s_udp_bps->conversion_fxn = pb_40s_udp_bps_2_str;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , irow , 3 , pb->stat.pb_40s_udp_bps ) , 0 );

	irow++;
	M_BREAK_STAT( nnc_add_empty_row( ptbl , NULL ) , 0 );
	//// 5s_tcp_pps
	M_BREAK_STAT( nnc_set_static_text( ptbl , irow , 0 , "5s_tcp_pps" ) , 0 );
	// 5s_tcp_pps
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pb->stat.pb_5s_tcp_pps ) , 0 );
	pb->stat.pb_5s_tcp_pps->storage.bt.pass_data = pb;
	pb->stat.pb_5s_tcp_pps->conversion_fxn = pb_5s_tcp_pps_2_str;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , irow , 1 , pb->stat.pb_5s_tcp_pps ) , 0 );
	//// 5s_tcp_bps
	M_BREAK_STAT( nnc_set_static_text( ptbl , irow , 2 , "5s_tcp_bps" ) , 0 );
	// 5s_tcp_bps
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pb->stat.pb_5s_tcp_bps ) , 0 );
	pb->stat.pb_5s_tcp_bps->storage.bt.pass_data = pb;
	pb->stat.pb_5s_tcp_bps->conversion_fxn = pb_5s_tcp_bps_2_str;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , irow , 3 , pb->stat.pb_5s_tcp_bps ) , 0 );

	irow++;
	M_BREAK_STAT( nnc_add_empty_row( ptbl , NULL ) , 0 );
	//// 10s_tcp_pps
	M_BREAK_STAT( nnc_set_static_text( ptbl , irow , 0 , "10s_tcp_pps" ) , 0 );
	// 10s_tcp_pps
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pb->stat.pb_10s_tcp_pps ) , 0 );
	pb->stat.pb_10s_tcp_pps->storage.bt.pass_data = pb;
	pb->stat.pb_10s_tcp_pps->conversion_fxn = pb_10s_tcp_pps_2_str;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , irow , 1 , pb->stat.pb_10s_tcp_pps ) , 0 );
	//// 10s_tcp_bps
	M_BREAK_STAT( nnc_set_static_text( ptbl , irow , 2 , "10s_tcp_bps" ) , 0 );
	// 10s_tcp_bps
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pb->stat.pb_10s_tcp_bps ) , 0 );
	pb->stat.pb_10s_tcp_bps->storage.bt.pass_data = pb;
	pb->stat.pb_10s_tcp_bps->conversion_fxn = pb_10s_tcp_bps_2_str;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , irow , 3 , pb->stat.pb_10s_tcp_bps ) , 0 );

	irow++;
	M_BREAK_STAT( nnc_add_empty_row( ptbl , NULL ) , 0 );
	//// 40s_tcp_pps
	M_BREAK_STAT( nnc_set_static_text( ptbl , irow , 0 , "40s_tcp_pps" ) , 0 );
	// 40s_tcp_pps
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pb->stat.pb_40s_tcp_pps ) , 0 );
	pb->stat.pb_40s_tcp_pps->storage.bt.pass_data = pb;
	pb->stat.pb_40s_tcp_pps->conversion_fxn = pb_40s_tcp_pps_2_str;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , irow , 1 , pb->stat.pb_40s_tcp_pps ) , 0 );
	//// 40s_tcp_bps 
	M_BREAK_STAT( nnc_set_static_text( ptbl , irow , 2 , "40s_tcp_bps" ) , 0 );
	// 40s_tcp_bps
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pb->stat.pb_40s_tcp_bps ) , 0 );
	pb->stat.pb_40s_tcp_bps->storage.bt.pass_data = pb;
	pb->stat.pb_40s_tcp_bps->conversion_fxn = pb_40s_tcp_bps_2_str;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , irow , 3 , pb->stat.pb_40s_tcp_bps ) , 0 );


	M_BREAK_STAT( distributor_subscribe( &_g->distributors.throttling_refresh_stat , SUB_VOID , SUB_FXN( pb_every_ticking_refresh ) , pb ) , 1 ); // refresh cells by central ticking
	init_udps_defragmentator( &pb->trd.cmn.defraged_udps ); // defragmentor

	#endif


	BEGIN_RET // TODO . complete reverse on error
	case 1:
	{
		DIST_BRIDGE_FAILURE();
	}
	M_V_END_RET
}

void mk_shrt_path( _IN AB * pb , _RET_VAL_P shrt_path * hlpr )
{
	MEMSET_ZERO_O( hlpr );

	WARNING( pb && hlpr );
	hlpr->pab = pb;
	hlpr->in_count = &pb->cpy_cfg.m.m.maintained.in_count;
	hlpr->out_count = &pb->cpy_cfg.m.m.maintained.out_count;
	hlpr->thread_is_created = &pb->trd.cmn.thread_is_created;
	//hlpr->do_close_thread = &pb->trd.cmn.do_close_thread;
	//hlpr->creation_thread_race_cond = &pb->trd.cmn.creation_thread_race_cond;
	//hlpr->bridg_prerequisite_stabled = &pb->trd.cmn.bridg_prerequisite_stabled;
	
}

_REGULAR_FXN void apply_new_protocol_bridge_config( G * _g , AB * pb , brg_cfg_t * new_ccfg )
{
	INIT_BREAKABLE_FXN();

	if ( !new_ccfg->m.m.maintained.enable ) return; // think more about this option maybe at better place should place it

	//if ( !new_ccfg->m.m.maintained.enable )
	//{
	//	for ( int i = 0 ; i < pb->pb_trds_masks_count ; i++ )
	//	{
	//		if ( pb->pb_trds_masks[ i ] )
	//		{
	//			pb->pb_trds->alc_thread->do_close_thread = 1;
	//		}
	//	}
	//	return;
	//}

	// when we arrive at this point we sure that somethings is changed
	new_ccfg->m.m.temp_data.pcfg_changed = 0; // say to config that change applied to bridge
	
	// each thread action switched here

	if ( iSTR_SAME( pb->cpy_cfg.m.m.id.thread_handler_act , "pcap_udp_counter" ) )
	{
		if ( !pb->trd.t.p_pcap_udp_counter )
		{
			init_ActiveBridge( _g , pb );

			M_BREAK_IF( !( pb->trd.t.p_pcap_udp_counter = MALLOC_ONE( pb->trd.t.p_pcap_udp_counter ) ) , errMemoryLow , 1 );
			MEMSET_ZERO_O( pb->trd.t.p_pcap_udp_counter );
			//pthread_mutex_init( &pb->trd.cmn.creation_thread_race_cond , NULL );
			//pthread_mutex_init( &pb->trd.cmn.do_all_prerequisite_stablished_race_cond , NULL );
			//pthread_mutex_lock( &pb->trd.cmn.creation_thread_race_cond );
			if ( !pb->trd.cmn.thread_is_created )
			{
				MM_BREAK_IF( pthread_create( &pb->trd.t.p_pcap_udp_counter->trd_id , NULL ,
					proc_pcap_udp_counter , pb ) != PTHREAD_CREATE_OK , errCreation , 0 , "thread creation failed" );
				pb->trd.cmn.thread_is_created = 1;
			}
			//pthread_mutex_unlock( &pb->trd.cmn.creation_thread_race_cond );
		}
	}

	else if ( iSTR_SAME( pb->cpy_cfg.m.m.id.thread_handler_act , "krnl_udp_counter" ) )
	{
		if ( !pb->trd.t.p_krnl_udp_counter )
		{
			init_ActiveBridge( _g , pb );

			M_BREAK_IF( !( pb->trd.t.p_krnl_udp_counter = MALLOC_ONE( pb->trd.t.p_krnl_udp_counter ) ) , errMemoryLow , 1 );
			MEMSET_ZERO_O( pb->trd.t.p_krnl_udp_counter );
			//pthread_mutex_init( &pb->trd.cmn.creation_thread_race_cond , NULL );
			//pthread_mutex_init( &pb->trd.cmn.do_all_prerequisite_stablished_race_cond , NULL );
			//pthread_mutex_lock( &pb->trd.cmn.creation_thread_race_cond );
			if ( !pb->trd.cmn.thread_is_created )
			{
				MM_BREAK_IF( pthread_create( &pb->trd.t.p_krnl_udp_counter->trd_id , NULL ,
					proc_krnl_udp_counter , pb ) != PTHREAD_CREATE_OK , errCreation , 0 , "thread creation failed" );
				pb->trd.cmn.thread_is_created = 1;
			}
			//pthread_mutex_unlock( &pb->trd.cmn.creation_thread_race_cond );

			pthread_t trd_udp_connection;
			MM_BREAK_IF( pthread_create( &trd_udp_connection , NULL , connect_udps_proc , pb ) != PTHREAD_CREATE_OK , errCreation , 0 , "thread creation failed" );
		}
	}

	else if ( iSTR_SAME( pb->cpy_cfg.m.m.id.thread_handler_act , "one2one_krnl2krnl_SF" ) )
	{
		if ( !pb->trd.t.p_one2one_krnl2krnl_SF )
		{
			init_ActiveBridge( _g , pb );

			M_BREAK_IF( !( pb->trd.t.p_one2one_krnl2krnl_SF = CALLOC_ONE( pb->trd.t.p_one2one_krnl2krnl_SF ) ) , errMemoryLow , 1 );
			//pthread_mutex_init( &pb->trd.cmn.creation_thread_race_cond , NULL );
			//pthread_mutex_init( &pb->trd.cmn.do_all_prerequisite_stablished_race_cond , NULL );
			//pthread_mutex_lock( &pb->trd.cmn.creation_thread_race_cond );

			// TODO . this size come from config and each packet size and release as soon as possible to prevent lost
			M_BREAK_STAT( cbuf_pked_init( &pb->trd.cmn.fast_wrt_cache , 1073741824 , &_g->cmd.burst_waiting_2 ) , 1 );

			if ( !pb->trd.cmn.thread_is_created )
			{
				MM_BREAK_IF( pthread_create( &pb->trd.t.p_one2one_krnl2krnl_SF->income_trd_id , NULL ,
					proc_one2one_krnl_udp_store , pb ) != PTHREAD_CREATE_OK , errCreation , 0 , "thread creation failed" );
				MM_BREAK_IF( pthread_create( &pb->trd.t.p_one2one_krnl2krnl_SF->outgoing_trd_id , NULL ,
					proc_one2one_krnl_tcp_forward , pb ) != PTHREAD_CREATE_OK , errCreation , 0 , "thread creation failed" );
				pb->trd.cmn.thread_is_created = 1;
			}
			//pthread_mutex_unlock( &pb->trd.cmn.creation_thread_race_cond );

			pthread_t trd_udp_connection;
			MM_BREAK_IF( pthread_create( &trd_udp_connection , NULL , connect_udps_proc , pb ) != PTHREAD_CREATE_OK , errCreation , 0 , "thread creation failed" );
			pthread_t trd_tcp_connection;
			MM_BREAK_IF( pthread_create( &trd_tcp_connection , NULL , thread_tcp_connection_proc , pb ) != PTHREAD_CREATE_OK , errCreation , 0 , "thread creation failed" );
		}
	}

	else if ( iSTR_SAME( pb->cpy_cfg.m.m.id.thread_handler_act , "one2one_pcap2krnl_SF" ) )
	{
		if ( !pb->trd.t.p_one2one_pcap2krnl_SF )
		{
			init_ActiveBridge( _g , pb );

			M_BREAK_IF( !( pb->trd.t.p_one2one_pcap2krnl_SF = MALLOC_ONE( pb->trd.t.p_one2one_pcap2krnl_SF ) ) , errMemoryLow , 1 );
			MEMSET_ZERO_O( pb->trd.t.p_one2one_pcap2krnl_SF );
			//pthread_mutex_init( &pb->trd.cmn.creation_thread_race_cond , NULL );
			//pthread_mutex_init( &pb->trd.cmn.do_all_prerequisite_stablished_race_cond , NULL );

			// TODO . this size come from config and each packet size and release as soon as possible to prevent lost
			M_BREAK_STAT( cbuf_pked_init( &pb->trd.cmn.fast_wrt_cache , 1073741824 , &_g->cmd.burst_waiting_2 ) , 1 );

			//pthread_mutex_lock( &pb->trd.cmn.creation_thread_race_cond );
			if ( !pb->trd.cmn.thread_is_created )
			{
				MM_BREAK_IF( pthread_create( &pb->trd.t.p_one2one_pcap2krnl_SF->income_trd_id , NULL ,
					proc_one2one_pcap2krnl_SF_udp_pcap , pb ) != PTHREAD_CREATE_OK , errCreation , 0 , "thread creation failed" );
				MM_BREAK_IF( pthread_create( &pb->trd.t.p_one2one_pcap2krnl_SF->outgoing_trd_id , NULL ,
					proc_one2one_pcap2krnl_SF_tcp_out , pb ) != PTHREAD_CREATE_OK , errCreation , 0 , "thread creation failed" );
				pb->trd.cmn.thread_is_created = 1;
			}
			//pthread_mutex_unlock( &pb->trd.cmn.creation_thread_race_cond );

			pthread_t trd_tcp_connection;
			MM_BREAK_IF( pthread_create( &trd_tcp_connection , NULL , thread_tcp_connection_proc , pb ) != PTHREAD_CREATE_OK , errCreation , 0 , "thread creation failed" );
		}
	}

	else if ( iSTR_SAME( pb->cpy_cfg.m.m.id.thread_handler_act , "one2many_pcap2krnl_SF" ) )
	{
		if ( !pb->trd.t.p_one2many_pcap2krnl_SF )
		{
			init_ActiveBridge( _g , pb );

			M_BREAK_IF( !( pb->trd.t.p_one2many_pcap2krnl_SF = MALLOC_ONE( pb->trd.t.p_one2many_pcap2krnl_SF ) ) , errMemoryLow , 1 );
			MEMSET_ZERO_O( pb->trd.t.p_one2many_pcap2krnl_SF );
			//pthread_mutex_init( &pb->trd.cmn.creation_thread_race_cond , NULL );
			//pthread_mutex_init( &pb->trd.cmn.do_all_prerequisite_stablished_race_cond , NULL );
			
			M_BREAK_STAT( cbuf_pked_init( &pb->trd.cmn.fast_wrt_cache , 1073741824 , &_g->cmd.burst_waiting_2 ) , 1 );

			//pthread_mutex_lock( &pb->trd.cmn.creation_thread_race_cond );
			if ( !pb->trd.cmn.thread_is_created )
			{
				MM_BREAK_IF( pthread_create( &pb->trd.t.p_one2many_pcap2krnl_SF->income_trd_id , NULL ,
					proc_one2many_pcap2krnl_SF_udp_pcap , pb ) != PTHREAD_CREATE_OK , errCreation , 0 , "thread creation failed" );
				MM_BREAK_IF( pthread_create( &pb->trd.t.p_one2many_pcap2krnl_SF->outgoing_trd_id , NULL ,
					proc_one2many_tcp_out , pb ) != PTHREAD_CREATE_OK , errCreation , 0 , "thread creation failed" );
				pb->trd.cmn.thread_is_created = 1;
			}
			//pthread_mutex_unlock( &pb->trd.cmn.creation_thread_race_cond );

			pthread_t trd_tcp_connection;
			MM_BREAK_IF( pthread_create( &trd_tcp_connection , NULL , thread_tcp_connection_proc , pb ) != PTHREAD_CREATE_OK , errCreation , 0 , "thread creation failed" );
		}
	}

	else if ( iSTR_SAME( pb->cpy_cfg.m.m.id.thread_handler_act , "many2one_pcap2krnl_S&F_serialize" ) )
	{
		if ( !pb->trd.t.p_many2one_pcap2krnl_SF_serialize )
		{
			init_ActiveBridge( _g , pb );
			M_BREAK_IF( !( pb->trd.t.p_many2one_pcap2krnl_SF_serialize = MALLOC_ONE( pb->trd.t.p_many2one_pcap2krnl_SF_serialize ) ) , errMemoryLow , 1 );
			MEMSET_ZERO_O( pb->trd.t.p_many2one_pcap2krnl_SF_serialize );
	//		//pthread_mutex_init( &pb->trd.cmn.creation_thread_race_cond , NULL );
	//		//pthread_mutex_init( &pb->trd.cmn.do_all_prerequisite_stablished_race_cond , NULL );
	//		// TODO . buff size came from config
			M_BREAK_STAT( cbuf_pked_init( &pb->trd.cmn.fast_wrt_cache , 1073741824 , &_g->cmd.burst_waiting_2 ) , 1 );
	//		//pthread_mutex_lock( &pb->trd.cmn.creation_thread_race_cond );
			if ( !pb->trd.cmn.thread_is_created )
			{
				MM_BREAK_IF( pthread_create( &pb->trd.t.p_many2one_pcap2krnl_SF_serialize->income_trd_id , NULL ,
					proc_many2many_pcap_krnl_SF , pb ) != PTHREAD_CREATE_OK , errCreation , 0 , "thread creation failed" );
				//MM_BREAK_IF( pthread_create( &pb->trd.t.p_many2one_pcap2krnl_SF_serialize->outgoing_trd_id , NULL ,
				//	 , pb ) != PTHREAD_CREATE_OK , errCreation , 0 , "thread creation failed" );
				pb->trd.cmn.thread_is_created = 1;
			}
	//		//pthread_mutex_unlock( &pb->trd.cmn.creation_thread_race_cond );
			pthread_t trd_tcp_connection;
			MM_BREAK_IF( pthread_create( &trd_tcp_connection , NULL , thread_tcp_connection_proc , pb ) != PTHREAD_CREATE_OK , errCreation , 0 , "thread creation failed" );
		}
	}


	BEGIN_RET // TODO . complete reverse on error
	case 1:
	{
		DIST_BRIDGE_FAILURE();
	}
	M_V_END_RET
}

