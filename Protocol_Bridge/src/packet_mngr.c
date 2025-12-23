#define Uses_packet_mngr_prerequisite
#define Uses_create_unique_file
#define Uses_MARK_LINE
#define Uses_log10
#define Uses_LOCK_LINE
#define Uses_pthread_mutex_timedlock_rel
#define Uses_timeval_compare
#define Uses_floorf
#define Uses_nnc_cell_content
#define Uses_timeval_diff_ms
#define Uses_STR_SAME
#define Uses_WARNING
#define Uses_packet_mngr
#define Uses_MEMSET_ZERO_O
#define Uses_errno
#define Uses_globals
#define Uses_Bridge
#define Uses_INIT_BREAKABLE_FXN

#include <Protocol_Bridge.dep>

_GLOBAL_VAR _EXTERN G * _g;

_PRIVATE_FXN _CALLBACK_FXN void cleanup_packet_mngr( pass_p src_g , long v )
{
#ifdef ENABLE_USE_DBG_TAG
	MARK_LINE();
#endif

	G * _g = ( G * )src_g;

	cleanup_pkt_mgr( &_g->hdls.pkt_mgr );

#ifdef ENABLE_USE_DBG_TAG
	MARK_LINE();
#endif
}

_PRIVATE_FXN _CALLBACK_FXN void waiting_until_no_more_unsaved_packet( pass_p src_g , long cur_order )
{
#ifdef ENABLE_USE_DBG_TAG
	MARK_LINE();
#endif

	G * _g = ( G * )src_g;
	_g->cmd.cleanup_state = cur_order;

	CIRCUIT_BREAKER long break_cuit = 0;
	time_t tbegin = time( NULL );
	for
	(	time_t tnow = tbegin ;
		!ci_sgm_is_empty( &_g->hdls.pkt_mgr.harbor_memory ) &&
		break_cuit < INFINITE_LOOP_GUARD() &&
		( ( tnow - tbegin ) < CFG().wait_at_cleanup_until_unsaved_packet_stored_sec ) ;
		mng_basic_thread_sleep( _g , HI_PRIORITY_THREAD ) , break_cuit++
	)
	{
		tnow = time( NULL );
	}

#ifdef ENABLE_USE_DBG_TAG
	MARK_LINE();
#endif
}

_CALLBACK_FXN _PRIVATE_FXN void state_pre_config_init_packet_mngr( void_p src_g )
{
	INIT_BREAKABLE_FXN();
	G * _g = ( G * )src_g;

#ifdef ENABLE_HALFFILL_SEGMENT
	M_BREAK_STAT( distributor_init( &_g->hdls.pkt_mgr.bcast_release_halffill_segment , 1 ) , 0 );
#endif
	M_BREAK_STAT( dict_fst_create( &_g->hdls.pkt_mgr.map_tcp_socket , 512 TODO ) , 0 );
#ifdef ENABLE_HALFFILL_SEGMENT
	M_BREAK_STAT( distributor_subscribe( &_g->hdls.pkt_mgr.bcast_release_halffill_segment , SUB_LONG , SUB_FXN( release_halffill_segment ) , _g ) , 0 ); // each clock try to close open segemtn
#endif

	// register here to get quit cmd
	M_BREAK_STAT( distributor_subscribe_withOrder( &_g->distributors.bcast_quit , SUB_LONG , SUB_FXN( cleanup_packet_mngr ) , _g , clean_packet_mngr ) , 0 );
	M_BREAK_STAT( distributor_subscribe_withOrder( &_g->distributors.bcast_quit , SUB_LONG , SUB_FXN( waiting_until_no_more_unsaved_packet ) , _g , wait_until_no_more_unsaved_packet ) , 0 );
	
#ifdef ENABLE_HALFFILL_SEGMENT
	M_BREAK_STAT( distributor_subscribe_withOrder( &_g->distributors.bcast_quit , SUB_LONG , SUB_FXN( release_halffill_segment ) , _g , getting_new_udp_stoped ) , 0 ); // there is no more udp so close segment
#endif

	BEGIN_RET
	default:
	{
		if ( d_error ) DIST_APP_FAILURE();
	}
	M_V_END_RET
}

_CALLBACK_FXN _PRIVATE_FXN void post_config_init_packet_mngr( void_p src_g )
{
	INIT_BREAKABLE_FXN();
	G * _g = ( G * )src_g;

	M_BREAK_STAT( cbuf_m_init( &_g->hdls.pkt_mgr.last_n_peek_total_seg_count , ( size_t )CFG().harbor_mem_flood_detection_sample_count ) , 0 );
	M_BREAK_STAT( cbuf_m_init( &_g->hdls.pkt_mgr.last_n_peek_filled_seg_count , ( size_t )CFG().harbor_mem_flood_detection_sample_count ) , 0 );

	//M_BREAK_STAT( cbuf_m_init( &_g->hdls.pkt_mgr.input_rates , CFG().harbor_mem_flood_detection_sample_count ) , 0 );
	//M_BREAK_STAT( cbuf_m_init( &_g->hdls.pkt_mgr.output_rates , CFG().harbor_mem_flood_detection_sample_count ) , 0 );

	_g->hdls.pkt_mgr.sampling_sent_packet_stride = 1;
	//_g->hdls.pkt_mgr.hysteresis_evacuate_segments_countdown = 1;
	M_BREAK_STAT( segmgr_init( &_g->hdls.pkt_mgr.harbor_memory , ( size_t )CFG().harbor_mem_segment_capacity , ( size_t )CFG().harbor_mem_segment_offsets_cnt_base , True ) , 0 );
	
#ifdef HAS_STATISTICSS
	M_BREAK_STAT( distributor_subscribe_withOrder( &_g->distributors.init_static_table , SUB_VOID , SUB_FXN( init_packetmgr_statistics ) , _g , packetmgr_overview ) , 0 );
#endif
	
	MM_BREAK_IF( pthread_mutex_init( &_g->hdls.pkt_mgr.pm_lock , NULL ) , errCreation , 0 , "mutex_init()" );

	M_BREAK_STAT( cr_in_wnd_init( &_g->hdls.pkt_mgr.longTermInputLoad , ( size_t )CFG().long_term_throughput_smoothing_samples ) , 0 );
	//M_BREAK_STAT( cr_in_wnd_init( &_g->hdls.pkt_mgr.longTermTcpOutLoad , ( size_t )CFG().long_term_throughput_smoothing_samples ) , 0 );
	M_BREAK_STAT( cr_in_wnd_init( &_g->hdls.pkt_mgr.longTermToStorageOutLoad , ( size_t )CFG().long_term_throughput_smoothing_samples ) , 0 );
	M_BREAK_STAT( cr_in_wnd_init( &_g->hdls.pkt_mgr.longTermOutLoad , ( size_t )CFG().long_term_throughput_smoothing_samples ) , 0 );
	
	inst_rate_init( &_g->hdls.pkt_mgr.instantaneousInputLoad ); // 1 sec . i use this just to handle peak load

#ifdef ENABLE_FILLED_TCP_SEGMENT_PROC
	MM_BREAK_IF( pthread_create( &_g->hdls.pkt_mgr.trd_tcp_sender , NULL , process_filled_tcp_segment_proc , ( pass_p )_g ) != PTHREAD_CREATE_OK , errCreation , 0 , "Failed to create tcp_sender thread" );
#endif

	//segmgr_init( &_g->hdls.pkt_mgr.sent_package_log , 3200000 , 100000 , True );
	M_BREAK_STAT( distributor_subscribe( &_g->hdls.prst_csh.bcast_pagestacked_pkts , SUB_DIRECT_ONE_CALL_BUFFER_SIZE , SUB_FXN( discharge_persistent_storage_data ) , _g ) , 0 );
	
#ifdef ENABLE_CLEAN_UNUSED_SEGMENT
	MM_BREAK_IF( pthread_create( &( pthread_t ){ 0 } , NULL , cleanup_unused_segment_proc , ( pass_p )_g) != PTHREAD_CREATE_OK , errCreation , 0 , "Failed to create thread");
#endif

#ifdef ENABLE_ABSOLETE_OLD_SEGMENT
	MM_BREAK_IF( pthread_create( &( pthread_t ) { 0 } , NULL , evacuate_old_segment_proc , ( pass_p )_g ) != PTHREAD_CREATE_OK , errCreation , 0 , "Failed to create thread" );
#endif

#ifdef HAS_STATISTICSS
	M_BREAK_STAT( distributor_subscribe( &_g->distributors.throttling_refresh_stat , SUB_VOID , SUB_FXN( sampling_filled_segment_count ) , _g ) , 0 );
#endif

	BEGIN_RET
	default:
	{
		if ( d_error ) DIST_APP_FAILURE();
	}
	M_V_END_RET
}

PRE_MAIN_INITIALIZATION( PRE_MAIN_INIT_PACKET_MNGR )
_PRIVATE_FXN void pre_main_init_packet_mngr_component( void )
{
	INIT_BREAKABLE_FXN();

	M_BREAK_STAT( distributor_subscribe( &_g->distributors.bcast_pre_cfg , SUB_VOID , SUB_FXN( state_pre_config_init_packet_mngr ) , _g ) , 0 );
	M_BREAK_STAT( distributor_subscribe_withOrder( &_g->distributors.bcast_post_cfg , SUB_VOID , SUB_FXN( post_config_init_packet_mngr ) , _g , post_config_order_packet_mngr ) , 0 );

	BEGIN_RET
	default:
	{
		if ( d_error ) DIST_APP_FAILURE();
	}
	M_V_END_RET
}

#ifndef statistics

#ifdef ENABLE_USE_DBG_TAG

_GLOBAL_VAR long long _mem_to_tcp = 0;
//_GLOBAL_VAR long long _mem_to_tcp_failure = 0;

_GLOBAL_VAR long long _sampled_packet = 0;
_GLOBAL_VAR long long _packet_to_file_by_mem_evacuation = 0;

//_GLOBAL_VAR int _regretion = 0;
_GLOBAL_VAR long long _send_packet_by_tcp_from_file = 0;
_GLOBAL_VAR long long _defraged_udp = 0;
_GLOBAL_VAR long long _defraged_udp_sz = 0;
_GLOBAL_VAR _EXTERN int _sem_in_fast_cache;
//_GLOBAL_VAR long long _L1Cache_ipv4_entrance;
_GLOBAL_VAR long long _open_gate_cnt;
_GLOBAL_VAR long long _close_gate_cnt;
_GLOBAL_VAR long long _half_segment_send_directly = 0;
_GLOBAL_VAR long long _whole_segment_send_directly = 0;

_GLOBAL_VAR long long _send_by_seek = 0;
_GLOBAL_VAR long long _send_by_lookup = 0;

_GLOBAL_VAR long long _try_resolve_route_counter = 0;
_GLOBAL_VAR long long _release_half_segment = 0;

_GLOBAL_VAR _EXTERN int64 _seg_ttl_sz;
_GLOBAL_VAR _EXTERN double _remain2use;
_GLOBAL_VAR _EXTERN double _longTermInputLoad;
//_GLOBAL_VAR _EXTERN double _longTermTcpOutLoad;
_GLOBAL_VAR _EXTERN double _longTermToStorageOutLoad;
_GLOBAL_VAR _EXTERN double _instantaneousInputLoad;
_GLOBAL_VAR _EXTERN double _longTermOutLoad;
//_GLOBAL_VAR _EXTERN double _rate_totally;
_GLOBAL_VAR _EXTERN double _TTF;

//_GLOBAL_VAR _EXTERN long long _filed_packet;
_GLOBAL_VAR _EXTERN long long _evac_segment;
_GLOBAL_VAR long long _try_evac_old_seg = 0;
_GLOBAL_VAR _EXTERN long long _evac_segment_paused;

_GLOBAL_VAR long long _inner_status_error = 0;

//_GLOBAL_VAR _EXTERN long long _hysteresis_strides_flush_packet_increament;
//_GLOBAL_VAR _EXTERN long long _hysteresis_strides_flush_packet_deacrement;

_GLOBAL_VAR _EXTERN size_t _sampling_sent_packet_stride;
_GLOBAL_VAR _EXTERN size_t _last_n_peek_total_seg;
_GLOBAL_VAR _EXTERN size_t _last_n_peek_filled_seg;

#endif

#ifdef HAS_STATISTICSS

#ifndef BufPool

_CALLBACK_FXN PASSED_CSTR auto_refresh_segment_total_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	ci_sgmgr_t * harbor_memory = ( ci_sgmgr_t * )pcell->storage.bt.pass_data;
	char buf1[ 64 ];
	_FORMAT_SHRTFRM( buf1 , sizeof( buf1 ) , harbor_memory->segment_total , DOUBLE_PRECISION() , "" , "" );
	sprintf( pcell->storage.tmpbuf , "%s" , buf1 );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR auto_refresh_filled_count_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	ci_sgmgr_t * harbor_memory = ( ci_sgmgr_t * )pcell->storage.bt.pass_data;
	char buf2[ 64 ];
	_FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , harbor_memory->filled_count , DOUBLE_PRECISION() , "" , "" );
	sprintf( pcell->storage.tmpbuf , "%s" , buf2 );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR auto_refresh_newed_segments_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	ci_sgmgr_t * harbor_memory = ( ci_sgmgr_t * )pcell->storage.bt.pass_data;
	char buf3[ 64 ];
	_FORMAT_SHRTFRM( buf3 , sizeof( buf3 ) , harbor_memory->newed_segments , DOUBLE_PRECISION() , "" , "" );
	sprintf( pcell->storage.tmpbuf , "%s" , buf3 );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR auto_refresh_released_segments_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	ci_sgmgr_t * harbor_memory = ( ci_sgmgr_t * )pcell->storage.bt.pass_data;
	char buf4[ 64 ];
	_FORMAT_SHRTFRM( buf4 , sizeof( buf4 ) , harbor_memory->released_segments , DOUBLE_PRECISION() , "" , "" );
	sprintf( pcell->storage.tmpbuf , "%s" , buf4 );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR auto_refresh_total_filled_segments_counter_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	ci_sgmgr_t * harbor_memory = ( ci_sgmgr_t * )pcell->storage.bt.pass_data;
	char buf5[ 64 ];
	_FORMAT_SHRTFRM( buf5 , sizeof( buf5 ) , harbor_memory->filled_segments_counter , DOUBLE_PRECISION() , "" , "" );
	sprintf( pcell->storage.tmpbuf , "%s" , buf5 );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR auto_refresh_harbor_memory_current_items_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	ci_sgmgr_t * harbor_memory = ( ci_sgmgr_t * )pcell->storage.bt.pass_data;
	_FORMAT_SHRTFRM( pcell->storage.tmpbuf , sizeof( pcell->storage.tmpbuf ) , harbor_memory->current_items , DOUBLE_PRECISION() , "" , "" );
	//sprintf( pcell->storage.tmpbuf , "%zu" , harbor_memory->current_items );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR auto_refresh_harbor_memory_current_bytes_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	ci_sgmgr_t * harbor_memory = ( ci_sgmgr_t * )pcell->storage.bt.pass_data;
	_FORMAT_SHRTFRM( pcell->storage.tmpbuf , sizeof( pcell->storage.tmpbuf ) , harbor_memory->current_bytes , DOUBLE_PRECISION() , "B" , "" );
	//sprintf( pcell->storage.tmpbuf , "%zu" , harbor_memory->current_bytes );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR auto_refresh_harbor_memory_tt_items_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	ci_sgmgr_t * harbor_memory = ( ci_sgmgr_t * )pcell->storage.bt.pass_data;
	_FORMAT_SHRTFRM( pcell->storage.tmpbuf , sizeof( pcell->storage.tmpbuf ) , harbor_memory->tt_items , DOUBLE_PRECISION() , "" , "" );
	//sprintf( pcell->storage.tmpbuf , "%zu" , harbor_memory->tt_items );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}
_CALLBACK_FXN PASSED_CSTR auto_refresh_harbor_memory_tt_bytes_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	ci_sgmgr_t * harbor_memory = ( ci_sgmgr_t * )pcell->storage.bt.pass_data;
	_FORMAT_SHRTFRM( pcell->storage.tmpbuf , sizeof( pcell->storage.tmpbuf ) , harbor_memory->tt_bytes , DOUBLE_PRECISION() , "B" , "" );
	//sprintf( pcell->storage.tmpbuf , "%zu" , harbor_memory->tt_bytes );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR auto_refresh_memory_time_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	G * _g = ( G * )pcell->storage.bt.pass_data;
	if ( _g->hdls.pkt_mgr.latest_huge_memory_time.tv_sec )
	{
		struct tm * tm_info = localtime( &_g->hdls.pkt_mgr.latest_huge_memory_time.tv_sec );
		strftime( pcell->storage.tmpbuf , sizeof( pcell->storage.tmpbuf ) , "%H:%M:%S" , tm_info );
	}
	else
	{
		pcell->storage.tmpbuf[ 0 ] = 0;
	}
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR auto_refresh_seg_ttl_sz_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	_FORMAT_SHRTFRM( pcell->storage.tmpbuf , sizeof( pcell->storage.tmpbuf ) , _seg_ttl_sz , DOUBLE_PRECISION() , "B" , "" );
	//sprintf( pcell->storage.tmpbuf , "seg_ttl_sz:%llu" , _seg_ttl_sz );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR auto_refresh_defraged_udp_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	//ci_sgmgr_t * harbor_memory = ( ci_sgmgr_t * )pcell->storage.bt.pass_data;
	_FORMAT_SHRTFRM( pcell->storage.tmpbuf , sizeof( pcell->storage.tmpbuf ) , _defraged_udp , DOUBLE_PRECISION() , "" , "" );
	//sprintf( pcell->storage.tmpbuf , "%lld" , _defraged_udp );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}
_CALLBACK_FXN PASSED_CSTR auto_refresh_defraged_udp_sz_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	//ci_sgmgr_t * harbor_memory = ( ci_sgmgr_t * )pcell->storage.bt.pass_data;
	_FORMAT_SHRTFRM( pcell->storage.tmpbuf , sizeof( pcell->storage.tmpbuf ) , _defraged_udp_sz , DOUBLE_PRECISION() , "B" , "" );
	//sprintf( pcell->storage.tmpbuf , "%lld" , _defraged_udp_sz );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR auto_refresh_segment_half_done_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	ci_sgmgr_t * harbor_memory = ( ci_sgmgr_t * )pcell->storage.bt.pass_data;
	char buf1[ 64 ];
	_FORMAT_SHRTFRM( buf1 , sizeof( buf1 ) , _release_half_segment , DOUBLE_PRECISION() , "" , "" );
	sprintf( pcell->storage.tmpbuf , "%s" , buf1 );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR auto_refresh_snd_ful_sgmnt_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	ci_sgmgr_t * harbor_memory = ( ci_sgmgr_t * )pcell->storage.bt.pass_data;
	char buf2[ 64 ];
	_FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , _whole_segment_send_directly , DOUBLE_PRECISION() , "" , "" );
	
	sprintf( pcell->storage.tmpbuf , "%s" , buf2 );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR auto_refresh_snd_half_sgmnt_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	ci_sgmgr_t * harbor_memory = ( ci_sgmgr_t * )pcell->storage.bt.pass_data;
	
	char buf3[ 64 ];
	_FORMAT_SHRTFRM( buf3 , sizeof( buf3 ) , _half_segment_send_directly , DOUBLE_PRECISION() , "" , "" );
	sprintf( pcell->storage.tmpbuf , "%s" , buf3 );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}


_CALLBACK_FXN PASSED_CSTR auto_refresh_tcp_pass_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	char buf1[ 64 ];
	_FORMAT_SHRTFRM( buf1 , sizeof( buf1 ) , _mem_to_tcp , DOUBLE_PRECISION() , "" , "" );
	sprintf( pcell->storage.tmpbuf , "%s" , buf1 );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR auto_refresh_send_by_seek_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	//G * _g = ( G * )pcell->storage.bt.pass_data;
	char buf1[ 64 ];
	_FORMAT_SHRTFRM( buf1 , sizeof( buf1 ) , _send_by_seek , DOUBLE_PRECISION() , "" , "" );
	sprintf( pcell->storage.tmpbuf , "%s" , buf1 );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR auto_refresh_send_by_lookup_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	//G * _g = ( G * )pcell->storage.bt.pass_data;
	char buf2[ 64 ];
	_FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , _send_by_lookup , DOUBLE_PRECISION() , "" , "" );
	sprintf( pcell->storage.tmpbuf , "%s" , buf2 );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR auto_refresh_try_resolve_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	G * _g = ( G * )pcell->storage.bt.pass_data;
	_FORMAT_SHRTFRM( pcell->storage.tmpbuf , sizeof( pcell->storage.tmpbuf ) , _try_resolve_route_counter , DOUBLE_PRECISION() , "" , "" );
	//sprintf( pcell->storage.tmpbuf , "try:%lld" , _try_resolve_route_counter );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR auto_refresh_sampled_packet_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;

	char buf2[ 64 ];
	_FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , _sampled_packet , DOUBLE_PRECISION() , "" , "" );

	sprintf( pcell->storage.tmpbuf , "%s" , buf2 );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR auto_refresh_packet_to_file_by_mem_evacuation_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;

	char buf3[ 64 ];
	_FORMAT_SHRTFRM( buf3 , sizeof( buf3 ) , _packet_to_file_by_mem_evacuation , DOUBLE_PRECISION() , "" , "" );

	sprintf( pcell->storage.tmpbuf , "%s" , buf3 );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

#endif

#ifndef Storage

_CALLBACK_FXN PASSED_CSTR auto_refresh_memmap_items_sent_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	//G * _g = ( G * )pcell->storage.bt.pass_data;
	_FORMAT_SHRTFRM( pcell->storage.tmpbuf , sizeof( pcell->storage.tmpbuf ) , _send_packet_by_tcp_from_file , DOUBLE_PRECISION() , "" , "" );
	//sprintf( pcell->storage.tmpbuf , "%lld" , _sucFromFile );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR auto_refresh_memmap_items_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	G * _g = ( G * )pcell->storage.bt.pass_data;
	_FORMAT_SHRTFRM( pcell->storage.tmpbuf , sizeof( pcell->storage.tmpbuf ) , _g->hdls.prst_csh.page_stack.item_stored , DOUBLE_PRECISION() , "" , "" );
	//sprintf( pcell->storage.tmpbuf , "%zu" , _g->hdls.prst_csh.page_stack.item_stored );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR auto_refresh_memmap_items_sz_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	G * _g = ( G * )pcell->storage.bt.pass_data;
	_FORMAT_SHRTFRM( pcell->storage.tmpbuf , sizeof( pcell->storage.tmpbuf ) , _g->hdls.prst_csh.page_stack.item_stored_byte , DOUBLE_PRECISION() , "B" , "" );
	//sprintf( pcell->storage.tmpbuf , "%zu" , _g->hdls.prst_csh.page_stack.item_stored_byte );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR auto_refresh_files_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	G * _g = ( G * )pcell->storage.bt.pass_data;
	sprintf( pcell->storage.tmpbuf , "%zu" , _g->hdls.prst_csh.page_stack.files.count );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR auto_refresh_evac_seg_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	ci_sgmgr_t * harbor_memory = ( ci_sgmgr_t * )pcell->storage.bt.pass_data;
	char buf2[ 64 ];
	_FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , _evac_segment , DOUBLE_PRECISION() , "" , "" );
	sprintf( pcell->storage.tmpbuf , "%s" , buf2 );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR auto_refresh_try_evac_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	ci_sgmgr_t * harbor_memory = ( ci_sgmgr_t * )pcell->storage.bt.pass_data;
	char buf3[ 64 ];
	_FORMAT_SHRTFRM( buf3 , sizeof( buf3 ) , _try_evac_old_seg , DOUBLE_PRECISION() , "" , "" );
	sprintf( pcell->storage.tmpbuf , "%s" , buf3 );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR auto_refresh_evac_paused_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	ci_sgmgr_t * harbor_memory = ( ci_sgmgr_t * )pcell->storage.bt.pass_data;
	char buf3[ 64 ];
	_FORMAT_SHRTFRM( buf3 , sizeof( buf3 ) , _evac_segment_paused , DOUBLE_PRECISION() , "" , "" );
	sprintf( pcell->storage.tmpbuf , "%s" , buf3 );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR auto_refresh_memmap_time_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	G * _g = ( G * )pcell->storage.bt.pass_data;
	if ( _g->hdls.pkt_mgr.latest_memmap_time.tv_sec )
	{
		struct tm * tm_info = localtime( &_g->hdls.pkt_mgr.latest_memmap_time.tv_sec );
		strftime( pcell->storage.tmpbuf , sizeof( pcell->storage.tmpbuf ) , "%H:%M:%S" , tm_info );
	}
	else
	{
		pcell->storage.tmpbuf[ 0 ] = 0;
	}
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR auto_refresh_gateway_open_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	G * _g = ( G * )pcell->storage.bt.pass_data;
	char buf1[ 64 ];
	_FORMAT_SHRTFRM( buf1 , sizeof( buf1 ) , _open_gate_cnt , DOUBLE_PRECISION() , "" , "" );
	char buf2[ 64 ];
	_FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , _close_gate_cnt , DOUBLE_PRECISION() , "" , "" );
	sprintf( pcell->storage.tmpbuf , "%s +%s -%s" , _g->hdls.gateway.pagestack_gateway_open_val ? "o" : "c" , buf1 , buf2 );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

#endif



#ifndef statistics

_CALLBACK_FXN PASSED_CSTR auto_refresh_regres_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;

	sprintf( pcell->storage.tmpbuf , "%zu=max(smpl>%zu,fill>%zu,segs>%zu)" , _g->hdls.pkt_mgr.sampling_sent_packet_stride , _sampling_sent_packet_stride ,
		_last_n_peek_filled_seg , _last_n_peek_total_seg );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR auto_refresh_remain2use_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	_FORMAT_SHRTFRM( pcell->storage.tmpbuf , sizeof( pcell->storage.tmpbuf ) , _remain2use , DOUBLE_PRECISION() , "B" , "" );
	//sprintf( pcell->storage.tmpbuf , "remain2use:%.2f" , _remain2use );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}
_CALLBACK_FXN PASSED_CSTR auto_refresh_longTermInputLoad_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	_FORMAT_SHRTFRM( pcell->storage.tmpbuf , sizeof( pcell->storage.tmpbuf ) , _longTermInputLoad , DOUBLE_PRECISION() , "Bps" , "" );
	//sprintf( pcell->storage.tmpbuf , "longTermInputLoad:%.2f" , _longTermInputLoad );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR auto_refresh_longTermToStorageOutLoad_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	_FORMAT_SHRTFRM( pcell->storage.tmpbuf , sizeof( pcell->storage.tmpbuf ) , _longTermToStorageOutLoad , DOUBLE_PRECISION() , "Bps" , "" );
	//sprintf( pcell->storage.tmpbuf , "longTermFaultyOutLoad:%.2f" , _longTermFaultyOutLoad );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}
_CALLBACK_FXN PASSED_CSTR auto_refresh_instantaneousInputLoad_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	_FORMAT_SHRTFRM( pcell->storage.tmpbuf , sizeof( pcell->storage.tmpbuf ) , _instantaneousInputLoad , DOUBLE_PRECISION() , "Bps" , "" );
	//sprintf( pcell->storage.tmpbuf , "instantaneousInputLoad:%.2f" , _instantaneousInputLoad );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR auto_refresh_longTermOutLoad_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	_FORMAT_SHRTFRM( pcell->storage.tmpbuf , sizeof( pcell->storage.tmpbuf ) , _longTermOutLoad , DOUBLE_PRECISION() , "Bps" , "" );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR auto_refresh_TTF_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	sprintf( pcell->storage.tmpbuf , "%.2f" , _TTF );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

//_CALLBACK_FXN PASSED_CSTR auto_refresh_hysteresis_increament_cell( pass_p src_pcell )
//{
//	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
//	//G * _g = ( G * )pcell->storage.bt.pass_data;
//	sprintf( pcell->storage.tmpbuf , "%lld" , _hysteresis_strides_flush_packet_increament );
//	return ( PASSED_CSTR )pcell->storage.tmpbuf;
//}
//_CALLBACK_FXN PASSED_CSTR auto_refresh_hysteresis_decrement_cell( pass_p src_pcell )
//{
//	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
//	//G * _g = ( G * )pcell->storage.bt.pass_data;
//	sprintf( pcell->storage.tmpbuf , "%lld" , _hysteresis_strides_flush_packet_deacrement );
//	return ( PASSED_CSTR )pcell->storage.tmpbuf;
//}

_CALLBACK_FXN PASSED_CSTR auto_refresh_pkt_in_L1Cache_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	//ci_sgmgr_t * harbor_memory = ( ci_sgmgr_t * )pcell->storage.bt.pass_data;
	sprintf( pcell->storage.tmpbuf , "%d" , _sem_in_fast_cache );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

//_CALLBACK_FXN PASSED_CSTR auto_refresh_L1Cache_ipv4_entrance_cell( pass_p src_pcell )
//{
//	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
//	//ci_sgmgr_t * harbor_memory = ( ci_sgmgr_t * )pcell->storage.bt.pass_data;
//	_FORMAT_SHRTFRM( pcell->storage.tmpbuf , sizeof( pcell->storage.tmpbuf ) , _L1Cache_ipv4_entrance , DOUBLE_PRECISION() , "" , "" );
//	return ( PASSED_CSTR )pcell->storage.tmpbuf;
//}

#endif



#define MEM_FAST "L1Cache"
#define MEM_HUGE "BuferPool"
#define MEM_FILE "Storage"

_CALLBACK_FXN void init_packetmgr_statistics( pass_p src_g )
{
	INIT_BREAKABLE_FXN();
	G * _g = ( G * )src_g;

	nnc_lock_for_changes( &_g->stat.nc_h );

	nnc_table * ptbl = NULL;
	M_BREAK_STAT( nnc_add_table( &_g->stat.nc_h , "Mem Overview" , &ptbl ) , 0 );

	// col
	M_BREAK_STAT( nnc_add_column( ptbl , ""  , "" , 0 ) , 0 );
	M_BREAK_STAT( nnc_add_column( ptbl , "" , "" , 10 ) , 0 );
	M_BREAK_STAT( nnc_add_column( ptbl , MEM_HUGE , "" , 10	) , 0 );
	M_BREAK_STAT( nnc_add_column( ptbl , "" , "" , 10 ) , 0 );
	M_BREAK_STAT( nnc_add_column( ptbl , MEM_FILE , "" , 10 ) , 0 );
	M_BREAK_STAT( nnc_add_column( ptbl , "" , "" , 10 ) , 0 );
	M_BREAK_STAT( nnc_add_column( ptbl , "" , "" , 10 ) , 0 );

	int irow = -1;
	int icol = 0;
	nnc_cell_content * pcell = NULL;

	for ( int idx_row = 0 ; idx_row < 25 ; idx_row++ )
	{
		irow++; icol = 0; M_BREAK_STAT( nnc_add_empty_row( ptbl , NULL ) , 0 );
		M_BREAK_STAT( nnc_set_static_int( ptbl , ( size_t )irow , ( size_t )icol++ , irow + 1 ) , 0 );
	}

	#ifndef BufPool
	int icon_col1 = 1;
	irow=0;icol=icon_col1;

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "cum Defraged UDP" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = &_g->hdls.pkt_mgr.harbor_memory; pcell->conversion_fxn = auto_refresh_defraged_udp_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icon_col1;

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "cum Defraged sz" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = &_g->hdls.pkt_mgr.harbor_memory; pcell->conversion_fxn = auto_refresh_defraged_udp_sz_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icon_col1;

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , MEM_HUGE " cum stored pkt" ) , 0); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = &_g->hdls.pkt_mgr.harbor_memory; pcell->conversion_fxn = auto_refresh_harbor_memory_tt_items_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icon_col1;

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , MEM_HUGE " cum stored sz" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = &_g->hdls.pkt_mgr.harbor_memory; pcell->conversion_fxn = auto_refresh_harbor_memory_tt_bytes_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icon_col1;

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , MEM_HUGE " cur stored pkt" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = &_g->hdls.pkt_mgr.harbor_memory; pcell->conversion_fxn = auto_refresh_harbor_memory_current_items_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icon_col1;

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , MEM_HUGE " cur stored sz" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = &_g->hdls.pkt_mgr.harbor_memory; pcell->conversion_fxn = auto_refresh_harbor_memory_current_bytes_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icon_col1;



	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , MEM_HUGE " cur Ttl segment" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = &_g->hdls.pkt_mgr.harbor_memory; pcell->conversion_fxn = auto_refresh_segment_total_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icon_col1;

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , MEM_HUGE " cur fill segment" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = &_g->hdls.pkt_mgr.harbor_memory; pcell->conversion_fxn = auto_refresh_filled_count_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icon_col1;

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , MEM_HUGE " cum fill segment" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = &_g->hdls.pkt_mgr.harbor_memory; pcell->conversion_fxn = auto_refresh_total_filled_segments_counter_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icon_col1;

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , MEM_HUGE " cum new segment" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = &_g->hdls.pkt_mgr.harbor_memory; pcell->conversion_fxn = auto_refresh_newed_segments_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icon_col1;

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , MEM_HUGE " cum del segment" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = &_g->hdls.pkt_mgr.harbor_memory; pcell->conversion_fxn = auto_refresh_released_segments_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icon_col1;

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , MEM_HUGE " ram used" ) , 0); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = &_g->hdls.pkt_mgr.harbor_memory; pcell->conversion_fxn = auto_refresh_seg_ttl_sz_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icon_col1;

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , MEM_HUGE " last pkt time" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = _g; pcell->conversion_fxn = auto_refresh_memory_time_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icon_col1;

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "sgmnt closed" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = &_g->hdls.pkt_mgr.harbor_memory; pcell->conversion_fxn = auto_refresh_segment_half_done_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icon_col1;

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "snd ful sgmnt" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = &_g->hdls.pkt_mgr.harbor_memory; pcell->conversion_fxn = auto_refresh_snd_ful_sgmnt_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icon_col1;

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "snd partial sgmnt" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = &_g->hdls.pkt_mgr.harbor_memory; pcell->conversion_fxn = auto_refresh_snd_half_sgmnt_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icon_col1;

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "cum tcp send" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = &_g->hdls.pkt_mgr.harbor_memory; pcell->conversion_fxn = auto_refresh_tcp_pass_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icon_col1;

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "snd quick" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = _g; pcell->conversion_fxn = auto_refresh_send_by_seek_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icon_col1;

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "snd slow" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = _g; pcell->conversion_fxn = auto_refresh_send_by_lookup_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icon_col1;

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "tcp sock lookup" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = _g; pcell->conversion_fxn = auto_refresh_try_resolve_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icon_col1;

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "selective stored pkt" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = _g; pcell->conversion_fxn = auto_refresh_sampled_packet_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icon_col1;

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "evacuated pkt" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = _g; pcell->conversion_fxn = auto_refresh_packet_to_file_by_mem_evacuation_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icon_col1;

	#endif


	#ifndef Storage
	int icon_col2 = 3;
	irow = 0; icol = icon_col2;

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , MEM_FILE " cur itm cnt" ) , 0); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = _g; pcell->conversion_fxn = auto_refresh_memmap_items_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icon_col2;

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , MEM_FILE " cur itm sz" ) , 0); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = _g; pcell->conversion_fxn = auto_refresh_memmap_items_sz_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icon_col2;

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , MEM_FILE " cur files cnt" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = _g; pcell->conversion_fxn = auto_refresh_files_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icon_col2;

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , MEM_FILE " cum snd pkt" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = _g; pcell->conversion_fxn = auto_refresh_memmap_items_sent_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icon_col2;

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "try evacuat" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->conversion_fxn = auto_refresh_try_evac_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icon_col2;

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "cum evac seg" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->conversion_fxn = auto_refresh_evac_seg_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icon_col2;

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "evac paused" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->conversion_fxn = auto_refresh_evac_paused_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icon_col2;

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "gateway mode" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = _g; pcell->conversion_fxn = auto_refresh_gateway_open_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icon_col2;

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , MEM_FILE " last pkt time" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = _g; pcell->conversion_fxn = auto_refresh_memmap_time_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icon_col2;

	#endif



	#ifndef statistics
	int icol_col3 = 5;
	irow = 0; icol = icol_col3;

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , MEM_FAST " cur ipv4 cnt" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = &_g->hdls.pkt_mgr.harbor_memory; pcell->conversion_fxn = auto_refresh_pkt_in_L1Cache_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icol_col3;

	//M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , MEM_FAST " ttl ipv4 cnt" ) , 0 ); pcell = NULL;
	//M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	//pcell->storage.bt.pass_data = &_g->hdls.pkt_mgr.harbor_memory; pcell->conversion_fxn = auto_refresh_L1Cache_ipv4_entrance_cell;
	//M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icol_col3;

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "evac coef" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = &_g->hdls.pkt_mgr.harbor_memory; pcell->conversion_fxn = auto_refresh_regres_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icol_col3;

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "remain ram" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->conversion_fxn = auto_refresh_remain2use_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icol_col3;

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "time to fill" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->conversion_fxn = auto_refresh_TTF_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icol_col3;

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "inLoad" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->conversion_fxn = auto_refresh_longTermInputLoad_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icol_col3;

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "instantLoad" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->conversion_fxn = auto_refresh_instantaneousInputLoad_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icol_col3;

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "Load2Storage" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->conversion_fxn = auto_refresh_longTermToStorageOutLoad_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icol_col3;

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "TotalOutLoad" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->conversion_fxn = auto_refresh_longTermOutLoad_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icol_col3;

	//M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "hysteresis +" ) , 0 ); pcell = NULL;
	//M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	//pcell->conversion_fxn = auto_refresh_hysteresis_increament_cell;
	//M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icol_col3;

	//M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "hysteresis -" ) , 0 ); pcell = NULL;
	//M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	//pcell->conversion_fxn = auto_refresh_hysteresis_decrement_cell;
	//M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icol_col3;

	#endif

	// --------------------

	M_BREAK_STAT( nnc_register_into_page_auto_refresh( ptbl , &_g->distributors.throttling_refresh_stat ) , 0 );

	BEGIN_RET
	default:
	{
		if ( d_error ) DIST_APP_FAILURE();
	}
	M_V_END_RET

	nnc_release_lock( &_g->stat.nc_h );
}

#endif // HAS_STATISTICSS
#endif // statistics

/// <summary>
/// from rings of each pcap or etc to global buffer
/// most be fast . but its not necessary to be fast as defragment_pcap_data
/// </summary>
_CALLBACK_FXN status fast_ring_2_huge_ring( pass_p data , buffer buf , size_t sz )
{
	INIT_BREAKABLE_FXN();

	AB_tcp * tcp = ( AB_tcp * )data;
	AB * pb = tcp->owner_pb;
	G * _g = ( G * )tcp->owner_pb->cpy_cfg.m.m.temp_data._pseudo_g;

	bool bNewSegment = false;
	status ret = segmgr_append( &_g->hdls.pkt_mgr.harbor_memory , buf , sz , &bNewSegment ); // store whole pakt + hdr into global buffer
	RANJE_ACT1( ret , errArg , NULL_ACT , MACRO_E( M_BREAK_STAT( ret , 0 ) ) );

	if ( bNewSegment )
	{
		MEMSET_ZERO_O( &_g->hdls.pkt_mgr.latest_huge_memory_time ); // try to send new packet cause time reset
	}

#ifdef ENABLE_USE_DBG_TAG
	if ( ret == errOK )
	{
		cr_in_wnd_add_packet( &_g->hdls.pkt_mgr.longTermInputLoad , sz ); // calculate input rate
		inst_rate_add_packet( &_g->hdls.pkt_mgr.instantaneousInputLoad , sz );

		_defraged_udp++;
		_defraged_udp_sz += (long long)sz;
	}
	else
	{
		_inner_status_error++;
	}
#endif

	BEGIN_RET
	default:
	{
		if ( d_error ) DIST_APP_FAILURE();
	}
	M_END_RET
}

// 1 . try to send packet under tcp to destination
// multi thread entrance . be quite
_PRIVATE_FXN _CALLBACK_FXN status process_segment_itm( buffer data , size_t len , pass_p src_g , void * nested_callback )
{
	G * _g = ( G * )src_g;
	status err_sent = errCanceled;

	status ret_lock = errCanceled;
	PKT_MGR_LOCK_LINE( ( ret_lock = pthread_mutex_timedlock_rel( &_g->hdls.pkt_mgr.pm_lock , 1000 TODO ) ) );
	if ( ret_lock == errTimeout )
	{
		return ret_lock;
	}
	
	xudp_hdr * pkt1 = ( xudp_hdr * )data;
	size_t sz_t = pkt1->metadata.payload_sz;
	WARNING( pkt1->metadata.version == TCP_XPKT_V1 );
	bool try_resolve_route = false;

	if ( pkt1->metadata.sent || pkt1->metadata.filed /*filed one on memory does not send*/ ) { err_sent = errOK; goto _exit_pt; }
	if ( _g->cmd.block_sending_1 ) { err_sent = errCanceled; goto _exit_pt; }

	time_t tnow = 0;
	tnow = time( NULL );
	sockfd fd = invalid_fd;
	void_p ab_tcp_p = NULL;
	AB * pb = NULL; /*if any data sent then statistics of that pb should be updated*/

	// normal packet come and go. retried packet should just checked
	if ( pkt1->metadata.is_faulti ) // faulty item should not have too many attempt
	{
		if ( pkt1->metadata.cool_down_attempt && pkt1->metadata.cool_down_attempt == *( uchar * )&tnow ) // in one second we should not attempt . and this check and possiblity is rare in not too many attempt situation
		{
			{ err_sent = errTooManyAttempt; goto _exit_pt; } // cannot send and too many attempt to send
		}
		pkt1->metadata.cool_down_attempt = *( uchar * )&tnow;

		_g->hdls.pkt_mgr.latest_memmap_time = pkt1->metadata.udp_hdr.tm; // after memmap packet gone it is time for huge memory packet
	}
	else // if not come from memmap . if there is huge cache packet that has oldest time compare to memmap packet then memmap packet should be send
	{
		_g->hdls.pkt_mgr.latest_huge_memory_time = pkt1->metadata.udp_hdr.tm;
	}
	
	// fast method
	if ( dict_fst_get_faster_by_hash_id( &_g->hdls.pkt_mgr.map_tcp_socket , pkt1->metadata.tcp_name_key_hash , pkt1->metadata.tcp_name_uniq_id , &fd , &ab_tcp_p ) == errOK && fd != INVALID_FD )
	{ // fast method
		if ( ab_tcp_p )
		{
			AB_tcp * ptcp = ( AB_tcp * )ab_tcp_p;
		}

		IMMORTAL_LPCSTR errString = NULL;
		uchar buf[MIN_SYSERR_BUF_SZ] = {0};
		err_sent = tcp_send_all( fd , data + pkt1->metadata.payload_offset , sz_t , 0 , SEND_TIMEOUT_ms , ACK_TIMEOUT_ms , RETRY_MECHANISM , &errString , ( buffer * )&buf ); // send is too heavy
		if ( errString != errOK )
		{
			//struct in_addr addr = {0};
			//addr.s_addr = pkt1->metadata.udp_hdr.dstIP;  // must already be in network byte order
			//char str[ INET_ADDRSTRLEN ] = {0};
			//inet_ntop( AF_INET , &addr , str , INET_ADDRSTRLEN );

		#ifdef ENABLE_LOGGING
			//log_write( LOG_ERROR , "%d %s %s %s %s" , __LINE__ , errString , buf , pkt1->TCP_name , str );
		#endif
		}
		else
		{
			err_sent = tcp_send_all( fd , "\n" , 1 , 0 , SEND_TIMEOUT_ms , ACK_TIMEOUT_ms , RETRY_MECHANISM , &errString , ( buffer * )&buf); // send is too heavy
		}
		switch ( err_sent )
		{
			case errOK:
			{
			#ifdef ENABLE_USE_DBG_TAG
				_send_by_seek++;
			#endif
				pkt1->metadata.sent = true;
				if ( ab_tcp_p )
				{
					AB_tcp * ptcp = ( AB_tcp * )ab_tcp_p;
					ptcp->this->last_access = time( NULL );
					pb = ptcp->owner_pb;
				}

				if ( !pkt1->metadata.is_faulti )
				{
					//cr_in_wnd_add_packet( &_g->hdls.pkt_mgr.longTermTcpOutLoad , len );
					cr_in_wnd_add_packet( &_g->hdls.pkt_mgr.longTermOutLoad , len );
				}
				break;
			}
			case errNoPeer:
			case errPeerClosed:
			{
				if ( ab_tcp_p )
				{
					AB_tcp * ptcp = ( AB_tcp * )ab_tcp_p;
					if ( !ptcp->this->tcp_is_about_to_connect ) // if another request is attempt then we should waint to complete
					{
						ptcp->this->tcp_is_about_to_connect = 1;
						//ptcp->retry_to_connect_tcp = 1;
						ptcp->this->tcp_connection_established = 0;
					}
					pb = ptcp->owner_pb;
				}

				if ( !errString ) /*because before it logged*/
				{
				#ifdef ENABLE_LOGGING
					log_write( LOG_ERROR , "%d err:%s" , __LINE__ , internalErrorStr( err_sent , true ) );
				#endif
				}
				break;
			}
			default:
			{
				_inner_status_error++;
				if ( !errString ) /*because before it logged*/
				{
				#ifdef ENABLE_LOGGING
					log_write( LOG_ERROR , "%d err:%s" , __LINE__ , internalErrorStr( err_sent , true ) );
				#endif
				}
				break;
			}
		}
	}
	else
	{
		try_resolve_route = true;
	#ifdef ENABLE_USE_DBG_TAG
		_try_resolve_route_counter++;
	#endif
	}
	
	// slow method
	if ( !pkt1->metadata.sent && !ab_tcp_p && _g->bridges.connected_tcp_out )
	{
		PKT_MGR_LOCK_LINE( pthread_mutex_lock( &_g->bridges.tcps_trd.mtx ) );
		for ( size_t iab = 0 ; iab < _g->bridges.ABs.count ; iab++ )
		{
			if ( mms_array_get_s( &_g->bridges.ABs , iab , ( void ** )&pb ) == errOK )
			{
				for ( int itcp = 0 ; itcp < pb->tcps_count ; itcp++ )
				{
					if ( STR_SAME( pb->tcps[ itcp ].__tcp_cfg_pak->name , pkt1->TCP_name ) )
					{
						if ( pb->tcps[ itcp ].this->tcp_connection_established )
						{
							IMMORTAL_LPCSTR errString = NULL;
							uchar buf[MIN_SYSERR_BUF_SZ] = {0};
							err_sent = tcp_send_all( pb->tcps[ itcp ].this->tcp_sockfd , data + pkt1->metadata.payload_offset , sz_t , 0 , SEND_TIMEOUT_ms , ACK_TIMEOUT_ms , RETRY_MECHANISM , &errString , ( buffer * )&buf ); // send is to heavy
							if ( errString != errOK )
							{
							#ifdef ENABLE_LOGGING
								log_write( LOG_ERROR , "%d %s %s %s" , __LINE__ , errString , buf , pkt1->TCP_name );
							#endif
							}
							else
							{
								err_sent = tcp_send_all( pb->tcps[ itcp ].this->tcp_sockfd , "\n" , 1 , 0 , SEND_TIMEOUT_ms , ACK_TIMEOUT_ms , RETRY_MECHANISM , &errString , ( buffer * )&buf ); // send is too heavy
							}
							switch ( err_sent )
							{
								case errOK:
								{
								#ifdef ENABLE_USE_DBG_TAG
									_send_by_lookup++;
								#endif
									pkt1->metadata.sent = true;
									pb->tcps[ itcp ].last_access = time( NULL );
									if ( try_resolve_route )
									{
										try_resolve_route = false;
										dict_fst_put( &_g->hdls.pkt_mgr.map_tcp_socket , pkt1->TCP_name , pb->tcps[ itcp ].this->tcp_sockfd , ( void_p )pb->tcps[ itcp ].this , NULL , NULL , NULL );
									}

									if ( !pkt1->metadata.is_faulti ) // faulti item come from file so why should i conder them in calculation
									{
										//cr_in_wnd_add_packet( &_g->hdls.pkt_mgr.longTermTcpOutLoad , len );
										cr_in_wnd_add_packet( &_g->hdls.pkt_mgr.longTermOutLoad , len );
									}
									break;
								}
								case errNoPeer:
								case errPeerClosed:
								{
									if ( !pb->tcps[ itcp ].tcp_is_about_to_connect )
									{
										pb->tcps[ itcp ].tcp_is_about_to_connect = 1;
										//pb->tcps[ itcp ].retry_to_connect_tcp = 1;
										pb->tcps[ itcp ].tcp_connection_established = 0;
									}
									if ( !errString ) /*because before it logged*/
									{
									#ifdef ENABLE_LOGGING
										log_write( LOG_ERROR , "%d err:%s" , __LINE__ , internalErrorStr( err_sent , true ) );
									#endif
									}
									break;
								}
								default:
								{
									_inner_status_error++;
									if ( !errString ) /*because before it logged*/
									{
									#ifdef ENABLE_LOGGING
										log_write( LOG_ERROR , "%d err:%s" , __LINE__ , internalErrorStr( err_sent , true ) );
									#endif
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
		pthread_mutex_unlock( &_g->bridges.tcps_trd.mtx );
	}
	
	//if ( !pkt1->metadata.sent && pkt1->metadata.retry )
	//{
	//	pkt1->metadata.retry = false;
	//	pkt1->metadata.retried = true;
	//	err_sent = process_segment_itm( data , len , src_g ); // retry
	//	goto _exit_pt;
	//}

	// under here err_sent could not be change because its used as succesful sending

	if ( pkt1->metadata.sent )
	{
		#ifdef ENABLE_USE_DBG_TAG
			_mem_to_tcp++;
		#endif

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
		//if ( !pkt1->metadata.udp_hdr.logged_2_mem )
		//{
		//	if ( segmgr_append( &_g->hdls.pkt_mgr.sent_package_log , &pkt1->metadata.udp_hdr , sizeof( pkt1->metadata.udp_hdr ) ) == errOK )
		//	{
		//		pkt1->metadata.udp_hdr.logged_2_mem = true;
		//	}
		//}

		if ( err_sent == errCanceled && !_g->bridges.connected_tcp_out )
		{
			err_sent = errNoPeer;
		}
	}

	if ( pb )
	{
		if ( difftime( tnow , pb->stat.round_zero_set.tcp_1_sec.t_tcp_throughput ) >= 1.0 )
		{
			#ifdef ENABLE_THROUGHPUT_MEASURE
			if ( pb->stat.round_zero_set.tcp_1_sec.t_tcp_throughput > 0 )
			{
				cbuf_m_advance( &pb->stat.round_init_set.tcp_stat_5_sec_count , pb->stat.round_zero_set.tcp_1_sec.calc_throughput_tcp_put_count );
				cbuf_m_advance( &pb->stat.round_init_set.tcp_stat_5_sec_bytes , pb->stat.round_zero_set.tcp_1_sec.calc_throughput_tcp_put_bytes );

				cbuf_m_advance( &pb->stat.round_init_set.tcp_stat_10_sec_count , pb->stat.round_zero_set.tcp_1_sec.calc_throughput_tcp_put_count );
				cbuf_m_advance( &pb->stat.round_init_set.tcp_stat_10_sec_bytes , pb->stat.round_zero_set.tcp_1_sec.calc_throughput_tcp_put_bytes );

				cbuf_m_advance( &pb->stat.round_init_set.tcp_stat_40_sec_count , pb->stat.round_zero_set.tcp_1_sec.calc_throughput_tcp_put_count );
				cbuf_m_advance( &pb->stat.round_init_set.tcp_stat_40_sec_bytes , pb->stat.round_zero_set.tcp_1_sec.calc_throughput_tcp_put_bytes );
			}
			#endif
			pb->stat.round_zero_set.tcp_1_sec.t_tcp_throughput = tnow;
			pb->stat.round_zero_set.tcp_1_sec.calc_throughput_tcp_put_count = 0;
			pb->stat.round_zero_set.tcp_1_sec.calc_throughput_tcp_put_bytes = 0;
		}
	}

	//d_error = errOK;
_exit_pt:
	pthread_mutex_unlock( &_g->hdls.pkt_mgr.pm_lock );
	return err_sent;
}

// 2 . if sending under tcp fail try to archive them( segment )
_PRIVATE_FXN _CALLBACK_FXN status process_faulty_itm_wrapper( buffer data , size_t len , pass_p src_g , void * nested_callback )
{
	status d_error = ( ( seg_item_cb )nested_callback )( data , len , src_g , NULL );
	if ( d_error == errOK )
	{
		_sampled_packet++;
	}
	return d_error;
}
_PRIVATE_FXN _CALLBACK_FXN status process_faulty_itm( buffer data , size_t len , pass_p src_g , void * nested_callback )
{
	G * _g = ( G * )src_g;
	status d_error = errCanceled;
	xudp_hdr * pkt1 = ( xudp_hdr * )data;
	size_t sz_t = pkt1->metadata.payload_sz;

	if ( pkt1->metadata.sent || /*pkt1->metadata.is_faulti ||*/ pkt1->metadata.filed )
	{
		return errOK;
	}
	
	if ( _g->hdls.pkt_mgr.latest_memmap_time.tv_sec && _g->hdls.pkt_mgr.latest_memmap_time.tv_sec > pkt1->metadata.udp_hdr.tm.tv_sec )
	{
		// just because olds segment also released then latest_memmap_time must be latest faulti packet
	}
	else
	{
		_g->hdls.pkt_mgr.latest_memmap_time = pkt1->metadata.udp_hdr.tm; // wheter memmap packet is most important so first send them
	}

	pkt1->metadata.is_faulti = 1;

//#ifdef ENABLE_USE_DBG_TAG
//	_mem_to_tcp_failure++;
//#endif

#ifdef ENABLE_PERSISTENT_CACHE
	// try to seperate requestor and actually archiver. so i drop it on ground and coursed stinky person grab it
	d_error = distributor_publish_buffer_size( &_g->hdls.prst_csh.bcast_store_data , data , len , src_g );
#endif
	
	if ( d_error == errOK )
	{
		pkt1->metadata.filed = true; // after sure that is stored on file set filed . so retry this on memory cause stoped. and that one from file is about to sent
		cr_in_wnd_add_packet( &_g->hdls.pkt_mgr.longTermToStorageOutLoad , len );
		cr_in_wnd_add_packet( &_g->hdls.pkt_mgr.longTermOutLoad , len );
	}
	else
	{
		_inner_status_error++;
	}

	// write it down on disk
	// later we can find logged that does not sent and find the bug
	
	return d_error;
}

_GLOBAL_VAR _EXTERN long long _evac_segment_paused;

/*
each packet come here to be sent
*/
_PRIVATE_FXN _CALLBACK_FXN status filed_itm( buffer data , size_t len , pass_p src_g , void * nested_callback )
{
	G * _g = ( G * )src_g;
	xudp_hdr * pkt1 = ( xudp_hdr * )data;

	struct timeval tnow;
	gettimeofday( &tnow , NULL );
	double df_sec = timeval_diff_ms( &pkt1->metadata.udp_hdr.tm , &tnow ) / 1000;
	if ( df_sec < CFG().in_memory_udp_hold_time_sec ) // tolerate on new segment persitent
	{
		_evac_segment_paused++;
		return errNoCountinue;
	}
	status d_error = process_faulty_itm( data , len , src_g , NULL );
	if ( d_error == errOK )
	{
		_packet_to_file_by_mem_evacuation++;
	}
	return d_error;
}

_CALLBACK_FXN status remap_storage_data( pass_p src_g , buffer buf , size_t sz ) // call from thread discharge_persistent
{
	G * _g = ( G * )src_g;
	status d_error = errCanceled;
	xudp_hdr * pkt1 = ( xudp_hdr * )buf;

	if ( pkt1->metadata.is_remapped )
	{
		return errDoneAlready;
	}

#ifdef ENABLE_PERSISTENT_CACHE
	pkt1->metadata.is_remapped = true; // first it should set to mapped but if failed then remove this flag
	d_error = distributor_publish_buffer_size_data( &_g->hdls.prst_csh.bcast_reroute_nopeer_pkt , buf , sz , ( long )pkt1->TCP_name , src_g );
#endif

	if ( d_error == errOK )
	{
	}
	else
	{
		pkt1->metadata.is_remapped = false;
	}

	return d_error;
}

/*
try send in file data
*/
_CALLBACK_FXN status discharge_persistent_storage_data( pass_p src_g , buffer buf , size_t sz )
{
	G * _g = ( G * )src_g;
	if ( buf == NULL && !sz ) // when no data in memmap then time reset to zero and actual send data if wait then release and do his work
	{
		if ( _g->hdls.pkt_mgr.latest_memmap_time.tv_sec )
		{
			MEMSET_ZERO_O( &_g->hdls.pkt_mgr.latest_memmap_time ); // store failed packet reset value
		}
		return errOK;
	}
	status d_error = process_segment_itm( buf , sz , src_g , NULL );

	switch( d_error )
	{
		case errOK:
		{
		#ifdef ENABLE_USE_DBG_TAG
			_send_packet_by_tcp_from_file++;
		#endif
			break;
		}
		case errPeerClosed: // second try to send from memmap stack
		case errNoPeer:
		{
		#ifdef ENABLE_REMAP_UNSEDABLE_PACKET
			// if it mapped then no more map happened
			switch ( remap_storage_data( src_g , buf , sz ) )
			{
				case errOK:
				{
					return errMapped;
				}
				case errDoneAlready:
				{
					return errButContinue;
				}
			}
		#endif
		}
	}

	// donot check error because tcp loss make error
	return d_error;
}

// emptied buffer cache( harbor_memory ) then on failure go to persistent storage cache and get from it
// this fxn do empty segment by segment
_THREAD_FXN void_p process_filled_tcp_segment_proc( pass_p src_g )
{
	G * _g = ( G * )src_g;
	
#ifdef ENABLE_LOG_THREADS
	__attribute__( ( cleanup( thread_goes_out_of_scope ) ) ) pthread_t this_thread = pthread_self();
	distributor_publish_x3long( &_g->distributors.bcast_thread_startup , (long)this_thread , trdn_process_filled_tcp_segment_proc , (long)__FUNCTION__ , _g );
	
	/*retrieve track alive indicator*/
	THREAD_LOCK_LINE( pthread_mutex_lock( &_g->stat.nc_s_req.thread_list_mtx ) );
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

	ci_sgm_t * pseg = NULL;
	int tm_cmp;
	status d_error;
	bool all_poped_send_suc;
	
	struct
	{
		union
		{
			struct
			{
				long a , b;
			} big_data;
			char bytes[ 16 ];
		} arr;
		uchar idx;
	} cpu_unburne = { 0 }; // suppress long time useless fetch rate

	do
	{
		if ( pthis_thread_alive_time ) *pthis_thread_alive_time = time( NULL );
		if ( GRACEFULLY_END_NOLOSS_THREAD() ) break;

		tm_cmp = timeval_compare( &_g->hdls.pkt_mgr.latest_huge_memory_time , &_g->hdls.pkt_mgr.latest_memmap_time );
		if
		(
			!_g->hdls.pkt_mgr.latest_huge_memory_time.tv_sec ||
			!_g->hdls.pkt_mgr.latest_memmap_time.tv_sec ||
			tm_cmp >= 0 ||
			_g->cmd.block_sending_1
		)
		{
			pseg = segmgr_pop_filled_segment( &_g->hdls.pkt_mgr.harbor_memory , False , seg_trv_LIFO );
			if ( pseg ) // poped on memory packets
			{
				all_poped_send_suc = true;

				cpu_unburne.idx = ( cpu_unburne.idx + 1 ) % (uchar)sizeof( cpu_unburne.arr );
				cpu_unburne.arr.bytes[ cpu_unburne.idx ] = 1;
				if ( _g->hdls.gateway.pagestack_gateway_open_val == gws_open )
				{
					_g->hdls.gateway.pagestack_gateway_open_val = gws_close;
				#ifdef ENABLE_USE_DBG_TAG
					_close_gate_cnt++;
				#endif
				}

				// try to send from mem under tcp to dst
				if ( _g->cmd.block_sending_1 || ci_sgm_iter_items( pseg , process_segment_itm , src_g , true , _g->hdls.pkt_mgr.sampling_sent_packet_stride , tail_2_head , NULL ) != errOK ) // some fault detected
				{
					all_poped_send_suc = false;
					// if sending filled segment fail try to archive them
					ci_sgm_iter_items( pseg , process_faulty_itm_wrapper , src_g , true , 1 , head_2_tail , process_faulty_itm );
				}
				// then close segment
				if ( ( d_error = ci_sgm_mark_empty( &_g->hdls.pkt_mgr.harbor_memory , pseg ) ) == errEmpty ) // pop last emptied segment
				{
					if ( _g->cmd.cleanup_state == cleaned_segments_state ) // when no more data exist and app about to exit
					{
						break;
					}
				}
				if ( d_error == errOK && _g->cmd.block_sending_1 )
				{
					continue;
				}
				if ( all_poped_send_suc )
				{
				#ifdef ENABLE_USE_DBG_TAG
					_whole_segment_send_directly++;
				#endif
				}
				else
				{
				#ifdef ENABLE_USE_DBG_TAG
					_half_segment_send_directly++;
				#endif
				}
			}
			else // there is no packet in memory so fetch persisted packets
			{
				cpu_unburne.idx = ( cpu_unburne.idx + 1 ) % (uchar)sizeof( cpu_unburne.arr );
				cpu_unburne.arr.bytes[ cpu_unburne.idx ] = 0;
				
				gateway_open_stat tmp_old = _g->hdls.gateway.pagestack_gateway_open_val;
				if ( !tmp_old )
				{
				#ifdef ENABLE_USE_DBG_TAG
					_open_gate_cnt++;
				#endif
					_g->hdls.gateway.pagestack_gateway_open_val = gws_open;
					sem_post( &_g->hdls.gateway.pagestack_gateway_open_sem );
				}
			}
		}
		else
		{
			cpu_unburne.idx = ( cpu_unburne.idx + 1 ) % (uchar)sizeof( cpu_unburne.arr );
			cpu_unburne.arr.bytes[ cpu_unburne.idx ] = 0;

			//if ( cpu_unburne.arr.big_data.a || cpu_unburne.arr.big_data.b )
			//{
			//	MEMSET_ZERO_O( &cpu_unburne );
			//}
			gateway_open_stat tmp_old = _g->hdls.gateway.pagestack_gateway_open_val;
			if ( !tmp_old )
			{
			#ifdef ENABLE_USE_DBG_TAG
				_open_gate_cnt++;
			#endif
				_g->hdls.gateway.pagestack_gateway_open_val = gws_open;
				sem_post( &_g->hdls.gateway.pagestack_gateway_open_sem );
			}
		}
		
		if ( !cpu_unburne.arr.big_data.a && !cpu_unburne.arr.big_data.b ) // enough time for packet arrive
		{
			mng_basic_thread_sleep( _g , HI_PRIORITY_THREAD ); // make balance between cpu burst and network bandwidth time
		}
	}
	while( 1 );
	if ( _g->hdls.gateway.pagestack_gateway_open_val == gws_open )
	{
		_g->hdls.gateway.pagestack_gateway_open_val = gws_close;
	}

#ifdef ENABLE_USE_DBG_TAG
	MARK_LINE();
#endif

	return NULL;
}

_THREAD_FXN void_p cleanup_unused_segment_proc( pass_p src_g )
{
	G * _g = ( G * )src_g;
	
#ifdef ENABLE_LOG_THREADS
	__attribute__( ( cleanup( thread_goes_out_of_scope ) ) ) pthread_t this_thread = pthread_self();
	distributor_publish_x3long( &_g->distributors.bcast_thread_startup , (long)this_thread , trdn_cleanup_unused_segment_proc , (long)__FUNCTION__ , _g );
	
	/*retrieve track alive indicator*/
	THREAD_LOCK_LINE( pthread_mutex_lock( &_g->stat.nc_s_req.thread_list_mtx ) );
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

	do
	{
		if ( pthis_thread_alive_time ) *pthis_thread_alive_time = time( NULL );
		if ( GRACEFULLY_END_THREAD() ) break;

		size_t sampling_sent_packet_stride = MAX( _g->hdls.pkt_mgr.sampling_sent_packet_stride , 1 );

		segmgr_cleanup_idle( &_g->hdls.pkt_mgr.harbor_memory , ( time_t )MAX( ( size_t )CFG().unused_memory_block_hold_time_sec / sampling_sent_packet_stride , 1 ) ); // if there is no work to do clean unused segment

		mng_basic_thread_sleep( _g , NORMAL_PRIORITY_THREAD );
	} while ( 1 );

#ifdef ENABLE_USE_DBG_TAG
	MARK_LINE();
#endif

	return NULL;
}

#ifndef evacuate_old_filled_segment_to_hard
_THREAD_FXN void_p evacuate_old_segment_proc( pass_p src_g )
{
	G * _g = ( G * )src_g;

#ifdef ENABLE_LOG_THREADS
	__attribute__( ( cleanup( thread_goes_out_of_scope ) ) ) pthread_t this_thread = pthread_self();
	distributor_publish_x3long( &_g->distributors.bcast_thread_startup , ( long )this_thread , trdn_evacuate_old_segment_proc , ( long )__FUNCTION__ , _g );

	/*retrieve track alive indicator*/
	THREAD_LOCK_LINE( pthread_mutex_lock( &_g->stat.nc_s_req.thread_list_mtx ) );
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

	do
	{
		if ( pthis_thread_alive_time ) *pthis_thread_alive_time = time( NULL );
		if ( GRACEFULLY_END_THREAD() ) break;

		_try_evac_old_seg++;
		segmgr_try_process_filled_segment( &_g->hdls.pkt_mgr.harbor_memory , filed_itm , src_g , seg_trv_FIFO_nolock , _g->hdls.pkt_mgr.sampling_sent_packet_stride );

		mng_basic_thread_sleep( _g , NORMAL_PRIORITY_THREAD );
	} while ( 1 );

#ifdef ENABLE_USE_DBG_TAG
	MARK_LINE();
#endif

	return NULL;
}
#endif


#ifndef release_active_unused_segment
#ifdef ENABLE_HALFFILL_SEGMENT
// check last packet of segment then if packet idle for long time then close segment
_PRIVATE_FXN _CALLBACK_FXN bool release_halffill_segment_condition( const buffer buf , size_t sz )
{
	if ( _g->cmd.cleanup_state <= getting_new_udp_stoped ) return true;
	xudp_hdr * pkt1 = ( xudp_hdr * )buf;
	WARNING( pkt1->metadata.version == TCP_XPKT_V1 );

	struct timeval tnow;
	gettimeofday( &tnow , NULL );
	double df_sec = timeval_diff_ms( &pkt1->metadata.udp_hdr.tm , &tnow ) / 1000;
	return df_sec > CFG().idle_active_harbor_mem_segment_timeout_sec;
}

// check if condition is true then set halffill segemtn as fill
_CALLBACK_FXN void release_halffill_segment( pass_p src_g , long v )
{
	G * _g = ( G * )src_g;
	if ( ci_sgm_peek_decide_active( &_g->hdls.pkt_mgr.harbor_memory , release_halffill_segment_condition ) )
	{
		_release_half_segment++;
		//MEMSET_ZERO_O( &_g->hdls.pkt_mgr.latest_huge_memory_time ); // try to send new packet cause time reset
	}
}
#endif
#endif

#ifndef calc_sampling

int64 _seg_ttl_sz = 0;
double _remain2use = 0;
double _longTermInputLoad = 0;
//double _longTermTcpOutLoad = 0;
double _longTermToStorageOutLoad = 0;
double _instantaneousInputLoad = 0;
double _longTermOutLoad = 0;
//double _rate_totally = 0;
double _TTF = 0;

//_GLOBAL_VAR long long _hysteresis_strides_flush_packet_increament = 0;
//_GLOBAL_VAR long long _hysteresis_strides_flush_packet_deacrement = 0;
_GLOBAL_VAR size_t _sampling_sent_packet_stride = 1;
_GLOBAL_VAR size_t _last_n_peek_total_seg = 1;
_GLOBAL_VAR size_t _last_n_peek_filled_seg = 1;

// every one second
_CALLBACK_FXN void sampling_filled_segment_count( pass_p src_g )
{
	G * _g = ( G * )src_g;
	
#ifndef TTF
	int64 seg_ttl_sz = ( int64 )( _g->hdls.pkt_mgr.harbor_memory.segment_total * _g->hdls.pkt_mgr.harbor_memory.default_seg_capacity );
	double remain2use = ( double )( CFG().harbor_mem_max_allowed_allocation - seg_ttl_sz );
	
	cr_in_wnd_add_packet( &_g->hdls.pkt_mgr.longTermInputLoad , 0 );
	cr_in_wnd_add_packet( &_g->hdls.pkt_mgr.longTermToStorageOutLoad , 0 );
	cr_in_wnd_add_packet( &_g->hdls.pkt_mgr.longTermOutLoad , 0 );

	double longTermInputLoad = cr_in_wnd_get_bps( &_g->hdls.pkt_mgr.longTermInputLoad );
	//double longTermTcpOutLoad = cr_in_wnd_get_bps( &_g->hdls.pkt_mgr.longTermTcpOutLoad );
	double longTermToStorageOutLoad = cr_in_wnd_get_bps( &_g->hdls.pkt_mgr.longTermToStorageOutLoad );
	double instantaneousInputLoad = ( double )inst_rate_get_last( &_g->hdls.pkt_mgr.instantaneousInputLoad , (size_t)CFG().instant_load_influence_window_time_sec );
	double longTermOutLoad = cr_in_wnd_get_bps( &_g->hdls.pkt_mgr.longTermOutLoad );

	double coef = CFG().instantaneous_input_load_coefficient;
	double input_rate = ( coef * instantaneousInputLoad ) + ( ( 1 - coef ) * longTermInputLoad ); // partial part from instantaneous and complement from long term
	double rate_totally = MAX( input_rate - longTermOutLoad , 0 );
	_sampling_sent_packet_stride = 1;

	_seg_ttl_sz = seg_ttl_sz;
	_remain2use = remain2use;
	_longTermInputLoad = longTermInputLoad;
	//_longTermTcpOutLoad = longTermTcpOutLoad;
	_longTermToStorageOutLoad = longTermToStorageOutLoad;
	_instantaneousInputLoad = instantaneousInputLoad;
	_longTermOutLoad = longTermOutLoad;
	//_rate_totally = rate_totally;

	if ( rate_totally > 0 )
	{
		double TTF = remain2use / rate_totally; /*time to fill*/
		_TTF = TTF;
		if ( TTF > CFG().TTF_no_backpressure_threshold_sec )
		{
			_sampling_sent_packet_stride = 1;
		}
		else if ( TTF > CFG().TTF_gentle_backpressure_threshold_sec )
		{
			_sampling_sent_packet_stride = (size_t)CFG().TTF_gentle_backpressure_stride;
		}
		else if ( TTF > CFG().TTF_aggressive_backpressure_threshold_sec )
		{
			_sampling_sent_packet_stride = (size_t)CFG().TTF_aggressive_backpressure_stride;
		}
		else if ( TTF > CFG().TTF_emergency_drop_backpressure_threshold_sec )
		{
			_sampling_sent_packet_stride = (size_t)CFG().TTF_emergency_drop_backpressure_stride;
		}
		else
		{
			//double BW_needed = _g->hdls.pkt_mgr.harbor_memory.current_bytes / TTF;
			//fill_condition_stride = ( int )ceil( BW_needed / longTermTcpOutLoad ); /**/
			_sampling_sent_packet_stride = (size_t)CFG().TTF_red_zone_stride;
		}
	}

	// 1. fill_condition_stride
#endif
	
//#ifndef aaaaaaaaaaaaaa
//
//	double corr = cr_in_wnd_calc_pearson_correlation( &_g->hdls.pkt_mgr.longTermInputLoad , &_g->hdls.pkt_mgr.longTermOutLoad );
//	
//	size_t old_hysteresis_evacuate_segments_countdown = _g->hdls.pkt_mgr.hysteresis_evacuate_segments_countdown;
//	if ( longTermOutLoad > longTermInputLoad * 1.5  )
//	{
//		_g->hdls.pkt_mgr.hysteresis_evacuate_segments_countdown = 1; 
//		if ( old_hysteresis_evacuate_segments_countdown > 1 ) _hysteresis_strides_flush_packet_deacrement++;
//	}
//	else if ( longTermOutLoad > longTermInputLoad * 1.3  )
//	{
//		_g->hdls.pkt_mgr.hysteresis_evacuate_segments_countdown /= 2 ;
//		if ( old_hysteresis_evacuate_segments_countdown > 1 ) _hysteresis_strides_flush_packet_deacrement++;
//	}
//	else if ( longTermOutLoad > longTermInputLoad * 0.99  )
//	{
//		_g->hdls.pkt_mgr.hysteresis_evacuate_segments_countdown--;
//		if ( old_hysteresis_evacuate_segments_countdown > 1 ) _hysteresis_strides_flush_packet_deacrement++;
//	}
//	else
//	{
//		if ( corr < 0.5 )
//		{
//			if ( longTermOutLoad < longTermInputLoad * 0.1  )
//			{
//				_g->hdls.pkt_mgr.hysteresis_evacuate_segments_countdown += 5 ;
//				_hysteresis_strides_flush_packet_increament++;
//			}
//			else if ( longTermOutLoad < longTermInputLoad * 0.5  )
//			{
//				_g->hdls.pkt_mgr.hysteresis_evacuate_segments_countdown += 2 ;
//				_hysteresis_strides_flush_packet_increament++;
//			}
//			else if ( longTermOutLoad < longTermInputLoad * 0.8  )
//			{
//				_g->hdls.pkt_mgr.hysteresis_evacuate_segments_countdown++;
//				_hysteresis_strides_flush_packet_increament++;
//			}
//		}
//	}
//
//	if ( _g->hdls.pkt_mgr.hysteresis_evacuate_segments_countdown < 1 )
//	{
//		_g->hdls.pkt_mgr.hysteresis_evacuate_segments_countdown = 1;
//	}
//	else if ( _g->hdls.pkt_mgr.hysteresis_evacuate_segments_countdown > ( size_t )CFG().TTF_red_zone_stride  )
//	{
//		_g->hdls.pkt_mgr.hysteresis_evacuate_segments_countdown = ( size_t )CFG().TTF_red_zone_stride;
//	}
//
//#endif

#ifndef total_segments_slope
	cbuf_m_advance( &_g->hdls.pkt_mgr.last_n_peek_total_seg_count , _g->hdls.pkt_mgr.harbor_memory.segment_total );
	_last_n_peek_total_seg = ( size_t )MAX(cbuf_m_regression_slope_all(&_g->hdls.pkt_mgr.last_n_peek_total_seg_count) + 1 , 1);
#endif
	
#ifndef filled_segments_slope
	cbuf_m_advance( &_g->hdls.pkt_mgr.last_n_peek_filled_seg_count , _g->hdls.pkt_mgr.harbor_memory.filled_count );
	_last_n_peek_filled_seg = ( size_t )MAX( cbuf_m_regression_slope_all( &_g->hdls.pkt_mgr.last_n_peek_filled_seg_count ) + 1 , 1 );
	
#endif

//#ifndef input_over_output
//	cbuf_m_advance( &_g->hdls.pkt_mgr.input_rates , ( uint64 )input_rate );
//	cbuf_m_advance( &_g->hdls.pkt_mgr.output_rates , ( uint64 )( longTermTcpOutLoad + longTermFaultyOutLoad ) );
//
//	float m_input_rate = cbuf_m_mean_all( &_g->hdls.pkt_mgr.input_rates );
//	float m_output_rate = cbuf_m_mean_all( &_g->hdls.pkt_mgr.output_rates );
//
//	if ( m_input_rate > m_output_rate )
//	{
//		/*3.*/_g->hdls.pkt_mgr.hysteresis_strides_flush_packet++;
//	}
//	else
//	{
//		_g->hdls.pkt_mgr.hysteresis_strides_flush_packet--;
//		if ( _g->hdls.pkt_mgr.hysteresis_strides_flush_packet < 1 ) _g->hdls.pkt_mgr.hysteresis_strides_flush_packet = 1;
//	}
//#endif

	_g->hdls.pkt_mgr.sampling_sent_packet_stride =
		MAX( MAX( MAX( _last_n_peek_filled_seg , _sampling_sent_packet_stride ) , _last_n_peek_total_seg ) , 1 );
}
#endif


void cleanup_pkt_mgr( pkt_mgr_t * pktmgr )
{
#ifdef ENABLE_USE_DBG_TAG
	DBG_PT();
#endif
#ifdef ENABLE_HALFFILL_SEGMENT
	sub_destroy( &pktmgr->bcast_release_halffill_segment );
#endif
#ifdef ENABLE_USE_DBG_TAG
	DBG_PT();
#endif
	//dict_fst_destroy( &pktmgr->map_tcp_socket ); it cause shutdown haulted
	segmgr_destroy( &pktmgr->harbor_memory );
#ifdef ENABLE_USE_DBG_TAG
	DBG_PT();
#endif
	pthread_mutex_destroy( &pktmgr->pm_lock );

	cr_in_wnd_free( &pktmgr->longTermInputLoad );
	//cr_in_wnd_free( &pktmgr->longTermTcpOutLoad );
	//cr_in_wnd_free( &pktmgr->longTermFaultyOutLoad );

	inst_rate_destroy( &pktmgr->instantaneousInputLoad );
}
