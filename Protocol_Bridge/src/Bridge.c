#define Uses_LOCK_LINE
#define Uses_pthread_mutex_timedlock_rel
#define Uses_proc_many2many_krnl_udp_store
#define Uses_sleep
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

_GLOBAL_VAR _EXTERN G * _g;

_CALLBACK_FXN _PRIVATE_FXN void post_config_init_stat_bridges( void_p src_g )
{
	INIT_BREAKABLE_FXN();
	G * _g = ( G * )src_g;

#ifdef HAS_STATISTICSS
	M_BREAK_STAT( distributor_subscribe_withOrder( &_g->distributors.init_static_table , SUB_VOID , SUB_FXN( init_bridges_statistics ) , _g , bridge_overview ) , 0 );
#endif

	BEGIN_RET
	default:
	{
		if ( d_error ) DIST_APP_FAILURE();
	}
	M_V_END_RET
}

//distributor_subscribe_withOrder( &_g->distributors.init_static_table , SUB_VOID , SUB_FXN( statistics_is_stabled_event ) , _g , statistics_is_stabled );
PRE_MAIN_INITIALIZATION( PRE_MAIN_INIT_BRIDGES )
_PRIVATE_FXN void pre_main_init_bridges_component( void )
{
	INIT_BREAKABLE_FXN();

	M_BREAK_STAT( distributor_subscribe_withOrder( &_g->distributors.bcast_post_cfg , SUB_VOID , SUB_FXN( post_config_init_stat_bridges ) , _g , post_config_order_bridges ) , 0 );

	BEGIN_RET
	default:
	{
		if ( d_error ) DIST_APP_FAILURE();
	}
	M_V_END_RET
}

_CALLBACK_FXN void try_stoping_sending_from_bridge( pass_p src_g , long v )
{
	G * _g = ( G * )src_g;
	for ( size_t idx = 0 ; idx < _g->bridges.ABs.count ; idx++ )
	{
		AB * pb = NULL;
		if ( mms_array_get_s( &_g->bridges.ABs , idx , ( void ** )&pb ) == errOK )
		{
			CIRCUIT_BREAKER long break_cuit = 0;
			for ( pb->comm.preq.stop_sending = true ; !pb->comm.preq.send_stoped && break_cuit < INFINITE_LOOP_GUARD() ; mng_basic_thread_sleep( _g , HI_PRIORITY_THREAD ) , break_cuit++ ); // order to stop. then after stop continue to clean up

			for ( CIRCUIT_BREAKER long break_cuit_1 = 0 ; break_cuit_1 < 10 ; break_cuit_1++ ) /*make a gap to insure there is no sender on tcp*/
			{
				if ( pthread_mutex_timedlock_rel( &_g->bridges.tcps_trd.mtx , 1 ) != errOK ) continue;

				for ( CIRCUIT_BREAKER long break_cuit_2 = 0 ; break_cuit_2 < 10 ; break_cuit_2++ )
				{
					if ( pthread_mutex_timedlock_rel( &_g->hdls.pkt_mgr.pm_lock , 1 ) != errOK ) continue;
					
					for ( size_t tcpidx = 0 ; tcpidx < pb->tcps_count ; tcpidx++ )
					{
						if ( pb->tcps[ tcpidx ].main_instance )
						{
							if ( pb->tcps[ tcpidx ].tcp_connection_established )
							{
								IMMORTAL_LPCSTR errString = NULL;
								_close_socket( &pb->tcps[ tcpidx ].tcp_sockfd , &errString );
								if ( errString )
								{
								#ifdef ENABLE_LOGGING
									log_write( LOG_ERROR , "%d %s" , __LINE__ , errString );
								#endif
								}
							}
						}
					}
					
					pthread_mutex_unlock( &_g->bridges.tcps_trd.mtx );
					break;
				}
				pthread_mutex_unlock( &_g->bridges.tcps_trd.mtx );
				break;
			}
		}
	}
}

_PRIVATE_FXN _CALLBACK_FXN void bridge_insure_input_bus_stoping( pass_p src_pb , long v )
{
	AB * pb = ( AB * )src_pb;
	if ( pb )
	{
		CIRCUIT_BREAKER long break_cuit = 0;
		for ( pb->comm.preq.stop_receiving = true ; !pb->comm.preq.receive_stoped && break_cuit < INFINITE_LOOP_GUARD() ; mng_basic_thread_sleep( _g , HI_PRIORITY_THREAD ) , break_cuit++ ); // order to stop. then after stop continue to clean up
	}
}

_PRIVATE_FXN _CALLBACK_FXN void cleanup_after_nomore_udp( pass_p src_pb , long v )
{
	AB * pb = ( AB * )src_pb;
	if ( pb )
	{
		cbuf_pked_destroy( &pb->comm.preq.raw_xudp_cache );
		finalize_udps_defragmentator( &pb->comm.preq.defraged_udps );

		sub_destroy( &pb->comm.preq.bcast_pcap_udp_pkt );
		sub_destroy( &pb->comm.preq.bcast_xudp_pkt );
	}
}

_CALLBACK_FXN void cleanup_bridges( pass_p src_g , long v )
{
	G * _g = ( G * )src_g;
	for ( size_t idx = 0 ; idx < _g->bridges.ABs.count ; idx++ )
	{
		AB * pb = NULL;
		if ( mms_array_get_s( &_g->bridges.ABs , idx , ( void ** )&pb ) == errOK )
		{
			cleaup_brg_cfg( &pb->cpy_cfg );

			DAC( pb->comm.acts.p_pcap_udp_counter );
			DAC( pb->comm.acts.p_krnl_udp_counter );
			DAC( pb->comm.acts.p_one2one_krnl2krnl_SF );
			DAC( pb->comm.acts.p_one2one_pcap2krnl_SF );

			if ( pb->comm.acts.p_one2many_pcap2krnl_SF )
			{
				dict_o_free( &pb->comm.acts.p_one2many_pcap2krnl_SF->dc_token_ring );
				DAC( pb->comm.acts.p_one2many_pcap2krnl_SF );
			}

			if ( pb->comm.acts.p_many2one_pcap2krnl_SF_serialize )
			{
				dict_o_free( &pb->comm.acts.p_many2one_pcap2krnl_SF_serialize->dc_token_ring );
				DAC( pb->comm.acts.p_many2one_pcap2krnl_SF_serialize );
			}

			if ( pb->comm.acts.p_one2many_krnl2krnl_SF )
			{
				dict_o_free( &pb->comm.acts.p_one2many_krnl2krnl_SF->dc_token_ring );
				DAC( pb->comm.acts.p_one2many_krnl2krnl_SF );
			}
			
			#ifdef ENABLE_THROUGHPUT_MEASURE
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
			#endif

			//DAC( pb->udps );
			//DAC( pb->tcps );
		}
	}
	mms_array_free( &_g->bridges.ABs );
}

_CALLBACK_FXN void pb_every_ticking_refresh( pass_p src_pb )
{
	AB * pb = ( AB * )src_pb;

#ifdef HAS_STATISTICSS
	nnc_cell_triggered( pb->stat.pb_elapse_cell );

	nnc_cell_triggered( pb->stat.pb_total_udp_get_count_cell );
	nnc_cell_triggered( pb->stat.pb_total_udp_get_byte_cell );
	nnc_cell_triggered( pb->stat.pb_total_tcp_put_count_cell );
	nnc_cell_triggered( pb->stat.pb_total_tcp_put_byte_cell );
	
#ifdef ENABLE_THROUGHPUT_MEASURE
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
#endif

#endif
}

_CALLBACK_FXN void tcp_state_changed( pass_p src_g , long conn1_dis0 )
{
	G * _g = ( G * )src_g;
	if ( conn1_dis0 )
	{
		_g->bridges.connected_tcp_out++;
	}
	else if ( _g->bridges.connected_tcp_out > 0 )
	{
		_g->bridges.connected_tcp_out--;
	}
}

_CALLBACK_FXN void init_bridges_statistics( pass_p src_g )
{
	INIT_BREAKABLE_FXN();


#ifdef HAS_STATISTICSS

	nnc_lock_for_changes( &_g->stat.nc_h );

	for ( size_t idx = 0 ; idx < _g->bridges.ABs.count ; idx++ )
	{
		AB * pb = NULL;
		if ( mms_array_get_s( &_g->bridges.ABs , idx , ( void ** )&pb ) == errOK )
		{
			M_BREAK_STAT( nnc_add_table( &_g->stat.nc_h , pb->cpy_cfg.m.m.id.short_name , &pb->ab_stat_tbl ) , 0 );
			nnc_table * ptbl = pb->ab_stat_tbl;
			// col
			M_BREAK_STAT( nnc_add_column( ptbl , "" , "" , 0 ) , 0 );
			M_BREAK_STAT( nnc_add_column( ptbl , "A" , "" , 0 ) , 0 );
			M_BREAK_STAT( nnc_add_column( ptbl , "B" , "" , 20 ) , 0 );
			M_BREAK_STAT( nnc_add_column( ptbl , "C" , "" , 0 ) , 0 );
			M_BREAK_STAT( nnc_add_column( ptbl , "D" , "" , 20 ) , 0 );
			M_BREAK_STAT( nnc_add_column( ptbl , "E" , "" , 0 ) , 0 );
			M_BREAK_STAT( nnc_add_column( ptbl , "F" , "" , 20 ) , 0 );

			int irow = -1;
			int icol = 0;

			//--->>>
			irow++; icol = 0; M_BREAK_STAT( nnc_add_empty_row( ptbl , NULL ) , 0 );
			M_BREAK_STAT( nnc_set_static_int( ptbl , ( size_t )irow , ( size_t )icol++ , irow + 1 ) , 0 );
			//---<<<
			// elapse time title
			M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "uptime" ) , 0 );
			M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pb->stat.pb_elapse_cell ) , 0 );
			pb->stat.pb_elapse_cell->storage.bt.pass_data = pb; pb->stat.pb_elapse_cell->conversion_fxn = pb_time_elapse_2_str;
			M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pb->stat.pb_elapse_cell ) , 0 );

			// fault title
			M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "fault" ) , 0 );
			M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pb->stat.pb_fault_cell ) , 0 );
			pb->stat.pb_fault_cell->storage.bt.pass_data = pb; pb->stat.pb_fault_cell->conversion_fxn = pb_fault_2_str;
			M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pb->stat.pb_fault_cell ) , 0 );


			//--->>>
			irow++; icol = 0; M_BREAK_STAT( nnc_add_empty_row( ptbl , NULL ) , 0 );
			M_BREAK_STAT( nnc_set_static_int( ptbl , ( size_t )irow , ( size_t )icol++ , irow + 1 ) , 0 );
			//---<<<
			// UDP conn title
			M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "UDP conn" ) , 0 );
			M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pb->stat.pb_UDP_conn_cell ) , 0 );
			pb->stat.pb_UDP_conn_cell->storage.bt.pass_data = pb; pb->stat.pb_UDP_conn_cell->conversion_fxn = pb_UDP_conn_2_str;
			M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pb->stat.pb_UDP_conn_cell ) , 0 );

			// TCP conn title
			M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "TCP conn" ) , 0 );
			M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pb->stat.pb_TCP_conn_cell ) , 0 );
			pb->stat.pb_TCP_conn_cell->storage.bt.pass_data = pb; pb->stat.pb_TCP_conn_cell->conversion_fxn = pb_TCP_conn_2_str;
			M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pb->stat.pb_TCP_conn_cell ) , 0 );


			//--->>>
			irow++; icol = 0; M_BREAK_STAT( nnc_add_empty_row( ptbl , NULL ) , 0 );
			M_BREAK_STAT( nnc_set_static_int( ptbl , ( size_t )irow , ( size_t )icol++ , irow + 1 ) , 0 );
			//---<<<
			// UDP retry conn title
			M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "UDP retry" ) , 0 );
			M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pb->stat.pb_UDP_retry_conn_cell ) , 0 );
			pb->stat.pb_UDP_retry_conn_cell->storage.bt.pass_data = pb; pb->stat.pb_UDP_retry_conn_cell->conversion_fxn = pb_UDP_retry_2_str;
			M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pb->stat.pb_UDP_retry_conn_cell ) , 0 );

			// TCP retry conn title
			M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "TCP retry" ) , 0 );
			M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pb->stat.pb_TCP_retry_conn_cell ) , 0 );
			pb->stat.pb_TCP_retry_conn_cell->storage.bt.pass_data = pb; pb->stat.pb_TCP_retry_conn_cell->conversion_fxn = pb_TCP_retry_2_str;
			M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pb->stat.pb_TCP_retry_conn_cell ) , 0 );


			//--->>>
			irow++; icol = 0; M_BREAK_STAT( nnc_add_empty_row( ptbl , NULL ) , 0 );
			M_BREAK_STAT( nnc_set_static_int( ptbl , ( size_t )irow , ( size_t )icol++ , irow + 1 ) , 0 );
			//---<<<
			// udp_get_count
			M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "UDP get" ) , 0 );
			M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pb->stat.pb_total_udp_get_count_cell ) , 0 );
			pb->stat.pb_total_udp_get_count_cell->storage.bt.pass_data = pb;
			pb->stat.pb_total_udp_get_count_cell->conversion_fxn = pb_UDP_get_count_2_str;
			M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pb->stat.pb_total_udp_get_count_cell ) , 0 );

			// udp_get_byte
			M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "UDP get B" ) , 0 );
			M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pb->stat.pb_total_udp_get_byte_cell ) , 0 );
			pb->stat.pb_total_udp_get_byte_cell->storage.bt.pass_data = pb; pb->stat.pb_total_udp_get_byte_cell->conversion_fxn = pb_UDP_get_byte_2_str;
			M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pb->stat.pb_total_udp_get_byte_cell ) , 0 );


			//--->>>
			irow++; icol = 0; M_BREAK_STAT( nnc_add_empty_row( ptbl , NULL ) , 0 );
			M_BREAK_STAT( nnc_set_static_int( ptbl , ( size_t )irow , ( size_t )icol++ , irow + 1 ) , 0 );
			//---<<<
			// tcp_put_count
			M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "TCP send" ) , 0 );
			M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pb->stat.pb_total_tcp_put_count_cell ) , 0 );
			pb->stat.pb_total_tcp_put_count_cell->storage.bt.pass_data = pb; pb->stat.pb_total_tcp_put_count_cell->conversion_fxn = pb_TCP_put_count_2_str;
			M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pb->stat.pb_total_tcp_put_count_cell ) , 0 );

			// tcp_put_byte
			M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "TCP send B" ) , 0 );
			M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pb->stat.pb_total_tcp_put_byte_cell ) , 0 );
			pb->stat.pb_total_tcp_put_byte_cell->storage.bt.pass_data = pb; pb->stat.pb_total_tcp_put_byte_cell->conversion_fxn = pb_TCP_put_byte_2_str;
			M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pb->stat.pb_total_tcp_put_byte_cell ) , 0 );


			#ifdef ENABLE_THROUGHPUT_MEASURE
			//--->>>
			irow++; icol = 0; M_BREAK_STAT( nnc_add_empty_row( ptbl , NULL ) , 0 );
			M_BREAK_STAT( nnc_set_static_int( ptbl , ( size_t )irow , ( size_t )icol++ , irow + 1 ) , 0 );
			//---<<<
			// 5s_udp_pps
			M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "5s_udp_pps" ) , 0 );
			M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pb->stat.pb_5s_udp_pps ) , 0 );
			pb->stat.pb_5s_udp_pps->storage.bt.pass_data = pb; pb->stat.pb_5s_udp_pps->conversion_fxn = pb_5s_udp_pps_2_str;
			M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pb->stat.pb_5s_udp_pps ) , 0 );

			// 5s_udp_bps
			M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "5s_udp_Bps" ) , 0 );
			M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pb->stat.pb_5s_udp_bps ) , 0 );
			pb->stat.pb_5s_udp_bps->storage.bt.pass_data = pb; pb->stat.pb_5s_udp_bps->conversion_fxn = pb_5s_udp_bps_2_str;
			M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pb->stat.pb_5s_udp_bps ) , 0 );

			//--->>>
			irow++; icol = 0; M_BREAK_STAT( nnc_add_empty_row( ptbl , NULL ) , 0 );
			M_BREAK_STAT( nnc_set_static_int( ptbl , ( size_t )irow , ( size_t )icol++ , irow + 1 ) , 0 );
			//---<<<
			// 10s_udp_pps
			M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "10s_udp_pps" ) , 0 );
			M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pb->stat.pb_10s_udp_pps ) , 0 );
			pb->stat.pb_10s_udp_pps->storage.bt.pass_data = pb; pb->stat.pb_10s_udp_pps->conversion_fxn = pb_10s_udp_pps_2_str;
			M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pb->stat.pb_10s_udp_pps ) , 0 );

			// 10s_udp_bps
			M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "10s_udp_Bps" ) , 0 );
			M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pb->stat.pb_10s_udp_bps ) , 0 );
			pb->stat.pb_10s_udp_bps->storage.bt.pass_data = pb; pb->stat.pb_10s_udp_bps->conversion_fxn = pb_10s_udp_bps_2_str;
			M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pb->stat.pb_10s_udp_bps ) , 0 );

			//--->>>
			irow++; icol = 0; M_BREAK_STAT( nnc_add_empty_row( ptbl , NULL ) , 0 );
			M_BREAK_STAT( nnc_set_static_int( ptbl , ( size_t )irow , ( size_t )icol++ , irow + 1 ) , 0 );
			//---<<<
			// 40s_udp_pps
			M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "40s_udp_pps" ) , 0 );
			M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pb->stat.pb_40s_udp_pps ) , 0 );
			pb->stat.pb_40s_udp_pps->storage.bt.pass_data = pb; pb->stat.pb_40s_udp_pps->conversion_fxn = pb_40s_udp_pps_2_str;
			M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pb->stat.pb_40s_udp_pps ) , 0 );

			// 40s_udp_bps
			M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "40s_udp_Bps" ) , 0 );
			M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pb->stat.pb_40s_udp_bps ) , 0 );
			pb->stat.pb_40s_udp_bps->storage.bt.pass_data = pb; pb->stat.pb_40s_udp_bps->conversion_fxn = pb_40s_udp_bps_2_str;
			M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pb->stat.pb_40s_udp_bps ) , 0 );

			//--->>>
			irow++; icol = 0; M_BREAK_STAT( nnc_add_empty_row( ptbl , NULL ) , 0 );
			M_BREAK_STAT( nnc_set_static_int( ptbl , ( size_t )irow , ( size_t )icol++ , irow + 1 ) , 0 );
			//---<<<
			//// 5s_tcp_pps
			M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "5s_tcp_pps" ) , 0 );
			M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pb->stat.pb_5s_tcp_pps ) , 0 );
			pb->stat.pb_5s_tcp_pps->storage.bt.pass_data = pb; pb->stat.pb_5s_tcp_pps->conversion_fxn = pb_5s_tcp_pps_2_str;
			M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pb->stat.pb_5s_tcp_pps ) , 0 );

			// 5s_tcp_bps
			M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "5s_tcp_Bps" ) , 0 );
			M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pb->stat.pb_5s_tcp_bps ) , 0 );
			pb->stat.pb_5s_tcp_bps->storage.bt.pass_data = pb; pb->stat.pb_5s_tcp_bps->conversion_fxn = pb_5s_tcp_bps_2_str;
			M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pb->stat.pb_5s_tcp_bps ) , 0 );

			//--->>>
			irow++; icol = 0; M_BREAK_STAT( nnc_add_empty_row( ptbl , NULL ) , 0 );
			M_BREAK_STAT( nnc_set_static_int( ptbl , ( size_t )irow , ( size_t )icol++ , irow + 1 ) , 0 );
			//---<<<
			// 10s_tcp_pps
			M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "10s_tcp_pps" ) , 0 );
			M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pb->stat.pb_10s_tcp_pps ) , 0 );
			pb->stat.pb_10s_tcp_pps->storage.bt.pass_data = pb; pb->stat.pb_10s_tcp_pps->conversion_fxn = pb_10s_tcp_pps_2_str;
			M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pb->stat.pb_10s_tcp_pps ) , 0 );

			// 10s_tcp_bps
			M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "10s_tcp_Bps" ) , 0 );
			M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pb->stat.pb_10s_tcp_bps ) , 0 );
			pb->stat.pb_10s_tcp_bps->storage.bt.pass_data = pb; pb->stat.pb_10s_tcp_bps->conversion_fxn = pb_10s_tcp_bps_2_str;
			M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pb->stat.pb_10s_tcp_bps ) , 0 );

			//--->>>
			irow++; icol = 0; M_BREAK_STAT( nnc_add_empty_row( ptbl , NULL ) , 0 );
			M_BREAK_STAT( nnc_set_static_int( ptbl , ( size_t )irow , ( size_t )icol++ , irow + 1 ) , 0 );
			//---<<<
			//// 40s_tcp_pps
			M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "40s_tcp_pps" ) , 0 );
			M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pb->stat.pb_40s_tcp_pps ) , 0 );
			pb->stat.pb_40s_tcp_pps->storage.bt.pass_data = pb; pb->stat.pb_40s_tcp_pps->conversion_fxn = pb_40s_tcp_pps_2_str;
			M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pb->stat.pb_40s_tcp_pps ) , 0 );

			// 40s_tcp_bps 
			M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "40s_tcp_Bps" ) , 0 );
			M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pb->stat.pb_40s_tcp_bps ) , 0 );
			pb->stat.pb_40s_tcp_bps->storage.bt.pass_data = pb; pb->stat.pb_40s_tcp_bps->conversion_fxn = pb_40s_tcp_bps_2_str;
			M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pb->stat.pb_40s_tcp_bps ) , 0 );
			#endif // ENABLE_THROUGHPUT_MEASURE

			M_BREAK_STAT( nnc_add_empty_row( ptbl , NULL ) , 0 );
			M_BREAK_STAT( nnc_add_empty_row( ptbl , NULL ) , 0 );
			M_BREAK_STAT( nnc_add_empty_row( ptbl , NULL ) , 0 );
			M_BREAK_STAT( nnc_add_empty_row( ptbl , NULL ) , 0 );

			irow = 0;

			irow++; icol = 5;
			M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "L1 ring miss" ) , 0 );
			M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pb->stat.pb_fst_cash_lost ) , 0 );
			pb->stat.pb_fst_cash_lost->storage.bt.pass_data = pb; pb->stat.pb_fst_cash_lost->conversion_fxn = pb_L1_ring_buff_full_2_str;
			M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pb->stat.pb_fst_cash_lost ) , 0 );

			irow++; icol = 5;
			M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "defrg bad struct" ) , 0 );
			M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pb->stat.pb_lost_ipv4_fragment ) , 0 );
			pb->stat.pb_lost_ipv4_fragment->storage.bt.pass_data = pb; pb->stat.pb_lost_ipv4_fragment->conversion_fxn = pb_defrag_ipv4_bad_structure_2_str;
			M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pb->stat.pb_lost_ipv4_fragment ) , 0 );

			irow++; icol = 5;
			M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "defrg krnl err" ) , 0 );
			M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pb->stat.pb_fst_cash_IPV4_big ) , 0 );
			pb->stat.pb_fst_cash_IPV4_big->storage.bt.pass_data = pb; pb->stat.pb_fst_cash_IPV4_big->conversion_fxn = pb_defrag_kernel_error_2_str;
			M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pb->stat.pb_fst_cash_IPV4_big ) , 0 );

			irow++; icol = 5;
			M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "defrg bad buffer" ) , 0 );
			M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pb->stat.pb_fst_cash_IPV4_mixedup ) , 0 );
			pb->stat.pb_fst_cash_IPV4_mixedup->storage.bt.pass_data = pb; pb->stat.pb_fst_cash_IPV4_mixedup->conversion_fxn = pb_defrag_bad_buffer_err_2_str;
			M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pb->stat.pb_fst_cash_IPV4_mixedup ) , 0 );

			irow++; icol = 5;
			M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "defrg unordered pkt" ) , 0 );
			M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pb->stat.pb_fst_cash_IPV4_packet_no_aggregate ) , 0 );
			pb->stat.pb_fst_cash_IPV4_packet_no_aggregate->storage.bt.pass_data = pb; pb->stat.pb_fst_cash_IPV4_packet_no_aggregate->conversion_fxn = pb_defrag_unordered_ipv4_err_2_str;
			M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pb->stat.pb_fst_cash_IPV4_packet_no_aggregate ) , 0 );

			irow++; icol = 5;
			M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "defrg corrupt" ) , 0 );
			M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pb->stat.pb_fst_cash_IPV4_packet_no_aggregate ) , 0 );
			pb->stat.pb_fst_cash_IPV4_packet_no_aggregate->storage.bt.pass_data = pb; pb->stat.pb_fst_cash_IPV4_packet_no_aggregate->conversion_fxn = pb_defrag_defragmentation_corrupted_2_str;
			M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pb->stat.pb_fst_cash_IPV4_packet_no_aggregate ) , 0 );



			M_BREAK_STAT( distributor_subscribe( &_g->distributors.throttling_refresh_stat , SUB_VOID , SUB_FXN( pb_every_ticking_refresh ) , pb ) , 1 ); // refresh cells by central ticking

		}
	}

#endif // HAS_STATISTICSS

	BEGIN_RET
	M_V_END_RET

	nnc_release_lock( &_g->stat.nc_h );
}


/// <summary>
/// init active udp tcp structure by the bridge config
/// </summary>
_PRIVATE_FXN void init_ActiveBridge( G * _g , AB * pb )
{
	INIT_BREAKABLE_FXN();

	// UDP
	if ( pb->cpy_cfg.m.m.maintained.in_count > 0 )
	{
		M_BREAK_IF( !( pb->udps = CALLOC_AR( pb->udps , pb->cpy_cfg.m.m.maintained.in_count ) ) , errMemoryLow , 1 );
		pb->udps_count = pb->cpy_cfg.m.m.maintained.in_count; // caution . count is always set after main ptr initialized

		for ( int iudp = 0 ; iudp < pb->udps_count ; iudp++ )
		{
			pb->udps[ iudp ].udp_sockfd = invalid_fd;
			pb->udps[ iudp ].owner_pb = pb;
			pb->udps[ iudp ].__udp_cfg_pak = &pb->cpy_cfg.m.m.maintained.in[ iudp ];
			M_BREAK_STAT( distributor_init( &pb->udps[ iudp ].bcast_change_state , 1 ) , 0 );
		}
	}

	// TCP
	BP_LOCK_LINE( pthread_mutex_lock( &_g->bridges.tcps_trd.mtx ) );
	if ( pb->cpy_cfg.m.m.maintained.out_count > 0 )
	{
		if ( ( pb->tcps = CALLOC_AR( pb->tcps , pb->cpy_cfg.m.m.maintained.out_count ) ) )
		{
			pb->tcps_count = pb->cpy_cfg.m.m.maintained.out_count;

			for ( int itcp_piv = 0 ; itcp_piv < pb->tcps_count ; itcp_piv++ )
			{
				for ( size_t ab_idx = 0 ; ab_idx < _g->bridges.ABs.count ; ab_idx++ )
				{
					AB * pab = NULL;
					if ( mms_array_get_s( &_g->bridges.ABs , ab_idx , ( void ** )&pab ) == errOK )
					{
						if ( pab != pb )
						{
							for ( int itcp_sec = 0 ; itcp_sec < pab->tcps_count ; itcp_sec++ )
							{
								if ( pab->tcps[ itcp_sec ].__tcp_cfg_pak )
								{
									if ( tcp_core_config_equlity( pab->tcps[ itcp_sec ].__tcp_cfg_pak , &pb->cpy_cfg.m.m.maintained.out[ itcp_piv ] ) )
									{
										pb->tcps[ itcp_piv ].this = &pab->tcps[ itcp_sec ];
										pb->tcps[ itcp_piv ].main_instance = false;
										break;
									}
								}
							}
						}
					}
					if ( pb->tcps[ itcp_piv ].this )
					{
						break;
					}
				}

				if ( !pb->tcps[ itcp_piv ].this )
				{
					pb->tcps[ itcp_piv ].this = &pb->tcps[ itcp_piv ];
					pb->tcps[ itcp_piv ].main_instance = true;
					pb->tcps[ itcp_piv ].tcp_sockfd = invalid_fd;
					M_BREAK_STAT( distributor_init( &pb->tcps[ itcp_piv ].bcast_change_state , 1 ) , 0 );
					subscriber_t * psubscriber = NULL;
					if ( distributor_subscribe_out( &pb->tcps[ itcp_piv ].bcast_change_state , SUB_LONG , SUB_FXN( tcp_state_changed ) , _g , &psubscriber ) == errOK )
					{
						psubscriber->data_order = ord_consumer;
					}
				}
				if ( pb->tcps[ itcp_piv ].this ) // it should be not null here
				{
					pb->tcps[ itcp_piv ].owner_pb = pb;
					pb->tcps[ itcp_piv ].__tcp_cfg_pak = &pb->cpy_cfg.m.m.maintained.out[ itcp_piv ];
				}
			}
		}
	}
	pthread_mutex_unlock( &_g->bridges.tcps_trd.mtx );

	// TODO . if Ab goes away then unregister quit intrupt
	M_BREAK_STAT( distributor_subscribe_withOrder( &_g->distributors.bcast_quit , SUB_LONG , SUB_FXN( bridge_insure_input_bus_stoping ) , pb , bridge_insure_input_bus_stoped ) , 0 ); // in several level bridge make cleanup
	M_BREAK_STAT( distributor_subscribe_withOrder( &_g->distributors.bcast_quit , SUB_LONG , SUB_FXN( cleanup_after_nomore_udp ) , pb , getting_new_udp_stoped ) , 0 ); // in several level bridge make cleanup
	M_BREAK_STAT( distributor_subscribe_withOrder( &_g->distributors.bcast_quit , SUB_LONG , SUB_FXN( cleanup_bridges ) , _g , clean_globals_shared_var ) , 0 ); // in several level bridge make cleanup
	
#ifdef HAS_STATISTICSS
#ifdef ENABLE_THROUGHPUT_MEASURE
	M_BREAK_STAT( cbuf_m_init( &pb->stat.round_init_set.udp_stat_5_sec_count , 5 ) , 0 );
	M_BREAK_STAT( cbuf_m_init( &pb->stat.round_init_set.udp_stat_10_sec_count , 10 ) , 0 );
	M_BREAK_STAT( cbuf_m_init( &pb->stat.round_init_set.udp_stat_40_sec_count , 40 ) , 0 );

	M_BREAK_STAT( cbuf_m_init( &pb->stat.round_init_set.udp_stat_5_sec_bytes , 5 ) , 0 );
	M_BREAK_STAT( cbuf_m_init( &pb->stat.round_init_set.udp_stat_10_sec_bytes , 10 ) , 0 );
	M_BREAK_STAT( cbuf_m_init( &pb->stat.round_init_set.udp_stat_40_sec_bytes , 40 ) , 0 );

	M_BREAK_STAT( cbuf_m_init( &pb->stat.round_init_set.tcp_stat_5_sec_count , 5 ) , 0 );
	M_BREAK_STAT( cbuf_m_init( &pb->stat.round_init_set.tcp_stat_10_sec_count , 10 ) , 0 );
	M_BREAK_STAT( cbuf_m_init( &pb->stat.round_init_set.tcp_stat_40_sec_count , 40 ) , 0 );

	M_BREAK_STAT( cbuf_m_init( &pb->stat.round_init_set.tcp_stat_5_sec_bytes , 5 ) , 0 );
	M_BREAK_STAT( cbuf_m_init( &pb->stat.round_init_set.tcp_stat_10_sec_bytes , 10 ) , 0 );
	M_BREAK_STAT( cbuf_m_init( &pb->stat.round_init_set.tcp_stat_40_sec_bytes , 40 ) , 0 );
#endif
#endif

	M_BREAK_STAT( init_udps_defragmentator( &pb->comm.preq.defraged_udps ) , 0 ); // defragmentor

	BEGIN_RET
	case 1:
	{
		DIST_BRIDGE_FAILURE();
	}
	M_V_END_RET
	nnc_release_lock( &_g->stat.nc_h );
}

void mk_shrt_path( _IN AB * pb , _RET_VAL_P shrt_pth_t * shrtcut )
{
	MEMSET_ZERO_O( shrtcut );

	WARNING( pb && shrtcut );
	shrtcut->pab = pb;
	shrtcut->in_count = &pb->cpy_cfg.m.m.maintained.in_count;
	shrtcut->out_count = &pb->cpy_cfg.m.m.maintained.out_count;
	shrtcut->thread_is_created = &pb->comm.preq.thread_is_created;
	//shrtcut->do_close_thread = &pb->comm.preq.do_close_thread;
	//shrtcut->bridg_prerequisite_stabled = &pb->comm.preq.bridg_prerequisite_stabled;
}

//_PRIVATE_FXN status create_tcp_connectios( AB * pb )
//{
//	status d_error = errOK;
//	G * _g = TO_G( pb->cpy_cfg.m.m.temp_data._pseudo_g );
//	pthread_mutex_lock( &_g->bridges.tcps_trd.mtx );
//	if ( !_g->bridges.tcps_trd.bcreated )
//	{
//		d_error = pthread_create( &_g->bridges.tcps_trd.trd_tcp_connection , NULL , thread_tcp_connection_proc , pb ) != PTHREAD_CREATE_OK ? errCreation : errOK;
//		_g->bridges.tcps_trd.bcreated = true;
//	}
//	pthread_mutex_unlock( &_g->bridges.tcps_trd.mtx );
//	return d_error;
//}

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

	#ifndef pkt_counter

	if ( iSTR_SAME( pb->cpy_cfg.m.m.id.thread_handler_act , "pcap_udp_counter" ) )
	{
		//if ( !pb->comm.acts.p_pcap_udp_counter )
		//{
		//	init_ActiveBridge( _g , pb );

		//	M_BREAK_IF( !( pb->comm.acts.p_pcap_udp_counter = MALLOC_ONE( pb->comm.acts.p_pcap_udp_counter ) ) , errMemoryLow , 1 );
		//	MEMSET_ZERO_O( pb->comm.acts.p_pcap_udp_counter );

	#ifdef ENABLE_BRIDGE_THREAD_CREATION
		//	if ( !pb->comm.preq.thread_is_created )
		//	{
		//		MM_BREAK_IF( pthread_create( &pb->comm.acts.p_pcap_udp_counter->trd_id , NULL ,
		//			proc_pcap_udp_counter , pb ) != PTHREAD_CREATE_OK , errCreation , 0 , "thread creation failed" );
		//		pb->comm.preq.thread_is_created = 1;
		//	}
	#endif
		
		//}
	}

	else if ( iSTR_SAME( pb->cpy_cfg.m.m.id.thread_handler_act , "krnl_udp_counter" ) )
	{
		if ( !pb->comm.acts.p_krnl_udp_counter )
		{
			//init_ActiveBridge( _g , pb );

			//M_BREAK_IF( !( pb->comm.acts.p_krnl_udp_counter = MALLOC_ONE( pb->comm.acts.p_krnl_udp_counter ) ) , errMemoryLow , 1 );
			//MEMSET_ZERO_O( pb->comm.acts.p_krnl_udp_counter );

		#ifdef ENABLE_BRIDGE_THREAD_CREATION
			//if ( !pb->comm.preq.thread_is_created )
			//{
			//	MM_BREAK_IF( pthread_create( &pb->comm.acts.p_krnl_udp_counter->trd_id , NULL ,
			//		proc_krnl_udp_counter , pb ) != PTHREAD_CREATE_OK , errCreation , 0 , "thread creation failed" );
			//	pb->comm.preq.thread_is_created = 1;
			//}
		#endif

			//pthread_t trd_udp_connection;
			//MM_BREAK_IF( pthread_create( &trd_udp_connection , NULL , connect_udps_proc , pb ) != PTHREAD_CREATE_OK , errCreation , 0 , "thread creation failed" );
		}
	}

	#endif

	#ifndef one2one

	else if ( iSTR_SAME( pb->cpy_cfg.m.m.id.thread_handler_act , "one2one_krnl2krnl_SF" ) )
	{
		if ( !pb->comm.acts.p_one2one_krnl2krnl_SF )
		{
			init_ActiveBridge( _g , pb );

			M_BREAK_IF( !( pb->comm.acts.p_one2one_krnl2krnl_SF = CALLOC_ONE( pb->comm.acts.p_one2one_krnl2krnl_SF ) ) , errMemoryLow , 1 );

			// each packet most release as soon as possible to prevent lost
			M_BREAK_STAT( cbuf_pked_init( &pb->comm.preq.raw_xudp_cache , ( size_t )CFG().raw_udp_cache_sz_byte , &_g->cmd.burst_waiting_2 ) , 1 );

		#ifdef ENABLE_BRIDGE_THREAD_CREATION
			if ( !pb->comm.preq.thread_is_created )
			{
				MM_BREAK_IF( pthread_create( &pb->comm.acts.p_one2one_krnl2krnl_SF->income_trd_id , NULL ,
					proc_one2one_krnl_udp_store , pb ) != PTHREAD_CREATE_OK , errCreation , 0 , "thread creation failed" );
				////MM_BREAK_IF( pthread_create( &pb->comm.acts.p_one2one_krnl2krnl_SF->outgoing_trd_id , NULL ,
				////	proc_one2one_krnl_tcp_forward , pb ) != PTHREAD_CREATE_OK , errCreation , 0 , "thread creation failed" );
				pb->comm.preq.thread_is_created = 1;
			}
		#endif

			pthread_t trd_udp_connection;
			MM_BREAK_IF( pthread_create( &trd_udp_connection , NULL , connect_udps_proc , pb ) != PTHREAD_CREATE_OK , errCreation , 0 , "thread creation failed" );
			
			//MM_BREAK_STAT( create_tcp_connectios( pb ) , 0 , "thread creation failed" );
		}
	}

	else if ( iSTR_SAME( pb->cpy_cfg.m.m.id.thread_handler_act , "one2one_pcap2krnl_SF" ) )
	{
		if ( !pb->comm.acts.p_one2one_pcap2krnl_SF )
		{
			init_ActiveBridge( _g , pb );

			M_BREAK_IF( !( pb->comm.acts.p_one2one_pcap2krnl_SF = MALLOC_ONE( pb->comm.acts.p_one2one_pcap2krnl_SF ) ) , errMemoryLow , 1 );
			MEMSET_ZERO_O( pb->comm.acts.p_one2one_pcap2krnl_SF );

			// each packet most release as soon as possible to prevent lost
			M_BREAK_STAT( cbuf_pked_init( &pb->comm.preq.raw_xudp_cache , ( size_t )CFG().raw_udp_cache_sz_byte , &_g->cmd.burst_waiting_2 ) , 1 );

		#ifdef ENABLE_BRIDGE_THREAD_CREATION
			if ( !pb->comm.preq.thread_is_created )
			{
				MM_BREAK_IF( pthread_create( &pb->comm.acts.p_one2one_pcap2krnl_SF->income_trd_id , NULL ,
					proc_one2one_pcap2krnl_SF_udp_pcap , pb ) != PTHREAD_CREATE_OK , errCreation , 0 , "thread creation failed" );
				#ifdef ENABLE_ON_PCAP_TCP_OUT
					MM_BREAK_IF( pthread_create( &pb->comm.acts.p_one2one_pcap2krnl_SF->outgoing_trd_id , NULL ,
						proc_one2one_pcap2krnl_SF_tcp_out , pb ) != PTHREAD_CREATE_OK , errCreation , 0 , "thread creation failed" );
				#endif
				pb->comm.preq.thread_is_created = 1;
			}
		#endif

			//MM_BREAK_STAT( create_tcp_connectios( pb ) , 0 , "thread creation failed" );
		}
	}

	#endif

	#ifndef one2many

	else if ( iSTR_SAME( pb->cpy_cfg.m.m.id.thread_handler_act , "one2many_krnl2krnl_SF" ) )
	{
		if ( !pb->comm.acts.p_one2many_krnl2krnl_SF )
		{
			init_ActiveBridge( _g , pb );

			M_BREAK_IF( !( pb->comm.acts.p_one2many_krnl2krnl_SF = CALLOC_ONE( pb->comm.acts.p_one2many_krnl2krnl_SF ) ) , errMemoryLow , 1 );

			// each packet most release as soon as possible to prevent lost
			M_BREAK_STAT( cbuf_pked_init( &pb->comm.preq.raw_xudp_cache , ( size_t )CFG().raw_udp_cache_sz_byte , &_g->cmd.burst_waiting_2 ) , 1 );

		#ifdef ENABLE_BRIDGE_THREAD_CREATION
			if ( !pb->comm.preq.thread_is_created )
			{
				MM_BREAK_IF( pthread_create( &pb->comm.acts.p_one2many_krnl2krnl_SF->income_trd_id , NULL ,
					proc_many2many_krnl_udp_store , pb ) != PTHREAD_CREATE_OK , errCreation , 0 , "thread creation failed" );
			////	MM_BREAK_IF( pthread_create( &pb->comm.acts.p_one2many_pcap2krnl_SF->outgoing_trd_id , NULL ,
			////		proc_one2many_tcp_out , pb ) != PTHREAD_CREATE_OK , errCreation , 0 , "thread creation failed" );
				pb->comm.preq.thread_is_created = 1;
			}
		#endif

			pthread_t trd_udp_connection;
			MM_BREAK_IF( pthread_create( &trd_udp_connection , NULL , connect_udps_proc , pb ) != PTHREAD_CREATE_OK , errCreation , 0 , "thread creation failed" );
			//MM_BREAK_STAT( create_tcp_connectios( pb ) , 0 , "thread creation failed" );
		}
	}

	else if ( iSTR_SAME( pb->cpy_cfg.m.m.id.thread_handler_act , "one2many_pcap2krnl_SF" ) )
	{
		if ( !pb->comm.acts.p_one2many_pcap2krnl_SF )
		{
			init_ActiveBridge( _g , pb );

			M_BREAK_IF( !( pb->comm.acts.p_one2many_pcap2krnl_SF = MALLOC_ONE( pb->comm.acts.p_one2many_pcap2krnl_SF ) ) , errMemoryLow , 1 );
			MEMSET_ZERO_O( pb->comm.acts.p_one2many_pcap2krnl_SF );

			// each packet most release as soon as possible to prevent lost
			M_BREAK_STAT( cbuf_pked_init( &pb->comm.preq.raw_xudp_cache , ( size_t )CFG().raw_udp_cache_sz_byte , &_g->cmd.burst_waiting_2 ) , 1 );

		#ifdef ENABLE_BRIDGE_THREAD_CREATION
			if ( !pb->comm.preq.thread_is_created )
			{
				MM_BREAK_IF( pthread_create( &pb->comm.acts.p_one2many_pcap2krnl_SF->income_trd_id , NULL ,
					proc_one2many_pcap2krnl_SF_udp_pcap , pb ) != PTHREAD_CREATE_OK , errCreation , 0 , "thread creation failed" );
				#ifdef ENABLE_ON_PCAP_TCP_OUT
					MM_BREAK_IF( pthread_create( &pb->comm.acts.p_one2many_pcap2krnl_SF->outgoing_trd_id , NULL ,
						proc_one2many_tcp_out , pb ) != PTHREAD_CREATE_OK , errCreation , 0 , "thread creation failed" );
				#endif
				pb->comm.preq.thread_is_created = 1;
			}
		#endif

			//MM_BREAK_STAT( create_tcp_connectios( pb ) , 0 , "thread creation failed" );
		}
	}

	//else if ( iSTR_SAME( pb->cpy_cfg.m.m.id.thread_handler_act , "many2one_pcap2krnl_S&F_serialize" ) )
	//{
	//	if ( !pb->comm.acts.p_many2one_pcap2krnl_SF_serialize )
	//	{
	//		//init_ActiveBridge( _g , pb );
	//		//M_BREAK_IF( !( pb->comm.acts.p_many2one_pcap2krnl_SF_serialize = MALLOC_ONE( pb->comm.acts.p_many2one_pcap2krnl_SF_serialize ) ) , errMemoryLow , 1 );
	//		//MEMSET_ZERO_O( pb->comm.acts.p_many2one_pcap2krnl_SF_serialize );
	//		
	//		// each packet most release as soon as possible to prevent lost
	//		//M_BREAK_STAT( cbuf_pked_init( &pb->comm.preq.raw_xudp_cache , ( size_t )CFG().raw_udp_cache_sz_byte , &_g->cmd.burst_waiting_2 ) , 1 );
	//		
	//	#ifdef ENABLE_BRIDGE_THREAD_CREATION
	//		//if ( !pb->comm.preq.thread_is_created )
	//		//{
	//		//	MM_BREAK_IF( pthread_create( &pb->comm.acts.p_many2one_pcap2krnl_SF_serialize->income_trd_id , NULL ,
	//		//		proc_many2many_pcap_krnl_SF , pb ) != PTHREAD_CREATE_OK , errCreation , 0 , "thread creation failed" );
	//		//	////MM_BREAK_IF( pthread_create( &pb->comm.acts.p_many2one_pcap2krnl_SF_serialize->outgoing_trd_id , NULL ,
	//		//	////	 , pb ) != PTHREAD_CREATE_OK , errCreation , 0 , "thread creation failed" );
	//		//	pb->comm.preq.thread_is_created = 1;
	//		//}
	//	#endif
	//		
	//		//MM_BREAK_STAT( create_tcp_connectios( pb ) , 0 , "thread creation failed" );
	//	}
	//}

	#endif

	//#ifndef many2many

	//else if ( iSTR_SAME( pb->cpy_cfg.m.m.id.thread_handler_act , "many2many_pcap2krnl_SF" ) )
	//{
	//	//if ( !pb->comm.acts.p_one2many_pcap2krnl_SF )
	//	//{
	//	//	init_ActiveBridge( _g , pb );

	//	//	M_BREAK_IF( !( pb->comm.acts.p_one2many_pcap2krnl_SF = MALLOC_ONE( pb->comm.acts.p_one2many_pcap2krnl_SF ) ) , errMemoryLow , 1 );
	//	//	MEMSET_ZERO_O( pb->comm.acts.p_one2many_pcap2krnl_SF );

	//	// each packet most release as soon as possible to prevent lost
	//	//	M_BREAK_STAT( cbuf_pked_init( &pb->comm.preq.raw_xudp_cache , ( size_t )CFG().raw_udp_cache_sz_byte , &_g->cmd.burst_waiting_2 ) , 1 );

	//#ifdef ENABLE_BRIDGE_THREAD_CREATION
	//	//	if ( !pb->comm.preq.thread_is_created )
	//	//	{
	//	//		MM_BREAK_IF( pthread_create( &pb->comm.acts.p_one2many_pcap2krnl_SF->income_trd_id , NULL ,
	//	//			proc_one2many_pcap2krnl_SF_udp_pcap , pb ) != PTHREAD_CREATE_OK , errCreation , 0 , "thread creation failed" );
	//	//		MM_BREAK_IF( pthread_create( &pb->comm.acts.p_one2many_pcap2krnl_SF->outgoing_trd_id , NULL ,
	//	//			proc_one2many_tcp_out , pb ) != PTHREAD_CREATE_OK , errCreation , 0 , "thread creation failed" );
	//	//		pb->comm.preq.thread_is_created = 1;
	//	//	}
	//#endif

	//	//	MM_BREAK_STAT( create_tcp_connectios( pb ) , 0 , "thread creation failed" );
	//	//}
	//}

	//#endif

	BEGIN_RET
	case 1:
	{
		DIST_BRIDGE_FAILURE();
	}
	M_V_END_RET
}

