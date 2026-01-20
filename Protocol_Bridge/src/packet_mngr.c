//#define USE_SOURCE_IN_DDLCK /*be carefull*/

#define Uses_iSTR_SAME
#define Uses_packet_mngr_prerequisite
#define Uses_create_unique_file
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
	G * _g = ( G * )src_g;

	cleanup_pkt_mgr( &PACKET_MGR() );

}

_PRIVATE_FXN _CALLBACK_FXN void waiting_until_no_more_unsaved_packet( pass_p src_g , long cur_order )
{
	G * _g = ( G * )src_g;
	_g->cmd.cleanup_state = cur_order;

	CIRCUIT_BREAKER long break_cuit = 0;
	time_t tbegin = time( NULL );
	for
	(	time_t tnow = tbegin ;
		!ci_sgm_is_empty( &PACKET_MGR().harbor_memory ) &&
		break_cuit < INFINITE_LOOP_GUARD() &&
		( ( tnow - tbegin ) < CFG().L2_wait_until_cleaning_up_sec ) ;
		mng_basic_thread_sleep( _g , HI_PRIORITY_THREAD ) , break_cuit++
	)
	{
		tnow = time( NULL );
	}

}

_CALLBACK_FXN _PRIVATE_FXN void state_pre_config_init_packet_mngr( void_p src_g )
{
	INIT_BREAKABLE_FXN();
	G * _g = ( G * )src_g;

	M_BREAK_STAT( dict_fst_create( &PACKET_MGR().map_tcp_socket , 512 TODO ) , 0 );

	// register here to get quit cmd
	M_BREAK_STAT( distributor_subscribe_withOrder( &_g->distributors.bcast_quit , SUB_LONG , SUB_FXN( cleanup_packet_mngr ) , _g , clean_packet_mngr ) , 0 );
	M_BREAK_STAT( distributor_subscribe_withOrder( &_g->distributors.bcast_quit , SUB_LONG , SUB_FXN( waiting_until_no_more_unsaved_packet ) , _g , wait_until_no_more_unsaved_packet ) , 0 );
	
#ifdef ENABLE_RELEASE_HALFFILL_UNUSED_SEGMENT
	/*at the end of program to clean and store last packet in the buffer this callback try to do that*/
	M_BREAK_STAT( distributor_subscribe_withOrder( &_g->distributors.bcast_quit , SUB_LONG , SUB_FXN( try_release_halffill_segment ) , _g , getting_new_udp_stoped ) , 0 ); // there is no more udp so close segment
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

	//M_BREAK_STAT( cbuf_m_init( &PACKET_MGR().last_n_peek_total_seg_count , ( size_t )CFG().L2_flood_evaluator_sample_need_count ) , 0 );
	//M_BREAK_STAT( cbuf_m_init( &PACKET_MGR().last_n_peek_filled_seg_count , ( size_t )CFG().L2_flood_evaluator_sample_need_count ) , 0 );

	PACKET_MGR().sampling_sent_packet_stride = 1;
	M_BREAK_STAT( segmgr_init( &PACKET_MGR().harbor_memory , ( size_t )CFG().L2_segment_capacity , ( size_t )CFG().L2_segment_offsets_cnt_base , True ) , 0 );
	
#ifdef HAS_STATISTICSS
	M_BREAK_STAT( distributor_subscribe_withOrder( &_g->distributors.init_static_table , SUB_VOID , SUB_FXN( init_packetmgr_statistics ) , _g , packetmgr_overview ) , 0 );
#endif
	
	MM_BREAK_IF( pthread_mutex_init( &PACKET_MGR().pm_lock , NULL ) , errCreation , 0 , "mutex_init()" );

	M_BREAK_STAT( cr_in_wnd_init( &PACKET_MGR().ram_ctrl_loadOnInput , ( size_t )CFG().L2_long_term_evaluator_sample_need_count ) , 0 );
	M_BREAK_STAT( cr_in_wnd_init( &PACKET_MGR().ram_ctrl_instantaneousInputLoad , ( size_t )CFG().L2_flood_evaluator_sample_need_count ) , 0 );
	//inst_rate_init( &PACKET_MGR().ram_ctrl_instantaneousInputLoad ); // 1 sec . i use this just to handle peak load

	M_BREAK_STAT( cr_in_wnd_init( &PACKET_MGR().ram_ctrl_loadOnStorage , ( size_t )CFG().L2_long_term_evaluator_sample_need_count ) , 0 );
	M_BREAK_STAT( cr_in_wnd_init( &PACKET_MGR().ram_ctrl_loadOnOutBridge , ( size_t )CFG().L2_long_term_evaluator_sample_need_count ) , 0 );
	inst_rate_init( &PACKET_MGR().ram_ctrl_instantaneousloadOnOutBridge );
	//M_BREAK_STAT( cr_in_wnd_init( &PACKET_MGR().ram_ctrl_instantaneousloadOnStorage , ( size_t )CFG().L2_flood_evaluator_sample_need_count ) , 0 );
	
#ifdef ENABLE_FILLED_TCP_SEGMENT_PROC
	MM_BREAK_IF( pthread_create( &PACKET_MGR().trd_tcp_sender , NULL , process_filled_tcp_segment_proc , ( pass_p )_g ) != PTHREAD_CREATE_OK , errCreation , 0 , "Failed to create tcp_sender thread" );
#endif

#ifdef ENABLE_RELEASE_HALFFILL_UNUSED_SEGMENT
	/*thread for release half segment that idle too long*/
	MM_BREAK_IF( pthread_create( &( pthread_t ) { 0 } , NULL , proc_and_release_halffill_segment , ( pass_p )_g ) != PTHREAD_CREATE_OK , errCreation , 0 , "Failed to create too long idle segment thread" );
#endif

	//segmgr_init( &PACKET_MGR().sent_package_log , 3200000 , 100000 , True );

#ifdef ENABLE_PERSISTENT_CACHE
	M_BREAK_STAT( distributor_subscribe( &_g->hdls.prst_csh.bcast_pagestacked_pkts , SUB_DIRECT_ONE_CALL_BUFFER_SIZE , SUB_FXN( discharge_persistent_storage_data ) , _g ) , 0 );
#endif

#ifdef ENABLE_CLEAN_UNUSED_SEGMENT
	MM_BREAK_IF( pthread_create( &( pthread_t ){ 0 } , NULL , cleanup_unused_idle_segment_proc , ( pass_p )_g) != PTHREAD_CREATE_OK , errCreation , 0 , "Failed to create thread");
#endif

#ifdef ENABLE_STORE_OLD_FILLED_LOW_PROBABLE_SENDABLE_SEGMENT
	MM_BREAK_IF( pthread_create( &( pthread_t ) { 0 } , NULL , evacuate_long_time_sediment_segment_proc , ( pass_p )_g ) != PTHREAD_CREATE_OK , errCreation , 0 , "Failed to create thread" );
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

_GLOBAL_VAR long long _send_packet_by_tcp_from_file = 0;

_GLOBAL_VAR unsigned long long _cum_harbor_rejected = 0;
_GLOBAL_VAR unsigned long long _cum_harbor_rejected_sz = 0;

_GLOBAL_VAR long long _cum_harbor_input = 0;
_GLOBAL_VAR long long _cum_harbor_input_sz = 0;

_GLOBAL_VAR _EXTERN int _sem_in_fast_cache;
_GLOBAL_VAR long long _open_gate_cnt;
_GLOBAL_VAR long long _close_gate_cnt;

_GLOBAL_VAR long long _open_gate_no_segment_cnt;
_GLOBAL_VAR long long _open_gate_l3_order_cnt;


_GLOBAL_VAR long long _half_segment_send_directly = 0;
_GLOBAL_VAR long long _whole_segment_send_directly = 0;

_GLOBAL_VAR long long _send_by_seek = 0;
_GLOBAL_VAR long long _send_by_lookup = 0;

_GLOBAL_VAR long long _try_resolve_route_counter = 0;
_GLOBAL_VAR long long _release_half_segment = 0;

_GLOBAL_VAR _EXTERN int64 _seg_ttl_sz;
_GLOBAL_VAR _EXTERN double _remain2use;
_GLOBAL_VAR _EXTERN double _longTermInputLoad;
_GLOBAL_VAR _EXTERN double _longTermToStorageOutLoad;
_GLOBAL_VAR _EXTERN double _instantaneousInputLoad;
_GLOBAL_VAR _EXTERN double _longTermOutLoad;
_GLOBAL_VAR _EXTERN double _TTF;

//_GLOBAL_VAR _EXTERN long long _filed_packet;
_GLOBAL_VAR _EXTERN long long _evac_segment;

_GLOBAL_VAR _EXTERN long long _evac_segment_paused;

_GLOBAL_VAR long long _inner_status_error = 0;

_GLOBAL_VAR long long _remaped_packet = 0;

_GLOBAL_VAR int _release_lock_countdown = 0;

_GLOBAL_VAR int _skip_n_item = 0;

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
	if ( PACKET_MGR().latest_huge_memory_time.tv_sec )
	{
		struct tm * tm_info = localtime( &PACKET_MGR().latest_huge_memory_time.tv_sec );
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
	_FORMAT_SHRTFRM( pcell->storage.tmpbuf , sizeof( pcell->storage.tmpbuf ) , _cum_harbor_input , DOUBLE_PRECISION() , "" , "" );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}
_CALLBACK_FXN PASSED_CSTR auto_refresh_defraged_udp_sz_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	_FORMAT_SHRTFRM( pcell->storage.tmpbuf , sizeof( pcell->storage.tmpbuf ) , _cum_harbor_input_sz , DOUBLE_PRECISION() , "B" , "" );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR auto_refresh_cum_harbor_rejected_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	_FORMAT_SHRTFRM( pcell->storage.tmpbuf , sizeof( pcell->storage.tmpbuf ) , _cum_harbor_rejected , DOUBLE_PRECISION() , "" , "" );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}
_CALLBACK_FXN PASSED_CSTR auto_refresh_cum_harbor_rejected_sz_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	_FORMAT_SHRTFRM( pcell->storage.tmpbuf , sizeof( pcell->storage.tmpbuf ) , _cum_harbor_rejected_sz , DOUBLE_PRECISION() , "B" , "" );
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
	_FORMAT_SHRTFRM( buf3 , sizeof( buf3 ) , _release_lock_countdown , DOUBLE_PRECISION() , "" , "" );
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
	if ( PACKET_MGR().latest_memmap_time.tv_sec )
	{
		struct tm * tm_info = localtime( &PACKET_MGR().latest_memmap_time.tv_sec );
		strftime( pcell->storage.tmpbuf , sizeof( pcell->storage.tmpbuf ) , "%H:%M:%S" , tm_info );
	}
	else
	{
		pcell->storage.tmpbuf[ 0 ] = 0;
	}
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR auto_refresh_cum_remaped_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	ci_sgmgr_t * harbor_memory = ( ci_sgmgr_t * )pcell->storage.bt.pass_data;
	char buf3[ 64 ];
	_FORMAT_SHRTFRM( buf3 , sizeof( buf3 ) , _remaped_packet , DOUBLE_PRECISION() , "" , "" );
	sprintf( pcell->storage.tmpbuf , "%s" , buf3 );
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

_CALLBACK_FXN PASSED_CSTR auto_refresh_gateway_open_reason_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	G * _g = ( G * )pcell->storage.bt.pass_data;
	char buf1[ 64 ];
	_FORMAT_SHRTFRM( buf1 , sizeof( buf1 ) , _open_gate_no_segment_cnt , DOUBLE_PRECISION() , "" , "" );
	char buf2[ 64 ];
	_FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , _open_gate_l3_order_cnt , DOUBLE_PRECISION() , "" , "" );
	sprintf( pcell->storage.tmpbuf , "noseg>%s l3ord>%s" , buf1 , buf2 );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

#endif

#ifndef statistics

_CALLBACK_FXN PASSED_CSTR auto_refresh_regres_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;

	sprintf( pcell->storage.tmpbuf , "%zu = ttf>%zu" , PACKET_MGR().sampling_sent_packet_stride , PACKET_MGR().TTF_sampling_sent_packet_stride );
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

_CALLBACK_FXN PASSED_CSTR auto_refresh_skip_n_item_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	_FORMAT_SHRTFRM( pcell->storage.tmpbuf , sizeof( pcell->storage.tmpbuf ) , _skip_n_item , DOUBLE_PRECISION() , "" , "" );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR auto_refresh_TTF_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	sprintf( pcell->storage.tmpbuf , "%.2f" , _TTF );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR auto_refresh_pkt_in_L1Cache_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	//ci_sgmgr_t * harbor_memory = ( ci_sgmgr_t * )pcell->storage.bt.pass_data;
	sprintf( pcell->storage.tmpbuf , "%d" , _sem_in_fast_cache );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR auto_refresh_sampling_method_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	//ci_sgmgr_t * harbor_memory = ( ci_sgmgr_t * )pcell->storage.bt.pass_data;
	switch ( PACKET_MGR().sampling_threshold_stage )
	{
		case prss_no_pressure:
		{
			sprintf( pcell->storage.tmpbuf , "" );
			break;
		}
		case prss_gentle_pressure:
		{
			sprintf( pcell->storage.tmpbuf , "gentle" );
			break;
		}
		case prss_aggressive_pressure:
		{
			sprintf( pcell->storage.tmpbuf , "aggressive" );
			break;
		}
		case prss_emergency_pressure:
		{
			sprintf( pcell->storage.tmpbuf , "emergency" );
			break;
		}
		case prss_red_zone_pressure:
		{
			sprintf( pcell->storage.tmpbuf , "red zone" );
			break;
		}
		case prss_skip_input_by_memory_fulled:
		{
			sprintf( pcell->storage.tmpbuf , "skip - mem" );
			break;
		}
		case prss_skip_input_by_hard_fulled:
		{
			sprintf( pcell->storage.tmpbuf , "skip - store" );
			break;
		}
		default:;
	}
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

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
	M_BREAK_STAT( nnc_add_column( ptbl , MEM_HUGE , "" , 10 ) , 0 );
	M_BREAK_STAT( nnc_add_column( ptbl , MEM_HUGE , "" , 10	) , 0 );
	M_BREAK_STAT( nnc_add_column( ptbl , MEM_FILE , "" , 10 ) , 0 );
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

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "cum UDP to send" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = &PACKET_MGR().harbor_memory; pcell->conversion_fxn = auto_refresh_defraged_udp_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icon_col1;

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "cum UDP to send sz" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = &PACKET_MGR().harbor_memory; pcell->conversion_fxn = auto_refresh_defraged_udp_sz_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icon_col1;

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "cum stored pkt" ) , 0); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = &PACKET_MGR().harbor_memory; pcell->conversion_fxn = auto_refresh_harbor_memory_tt_items_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icon_col1;

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "cum stored sz" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = &PACKET_MGR().harbor_memory; pcell->conversion_fxn = auto_refresh_harbor_memory_tt_bytes_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icon_col1;

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "cur stored pkt" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = &PACKET_MGR().harbor_memory; pcell->conversion_fxn = auto_refresh_harbor_memory_current_items_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icon_col1;

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "cur stored sz" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = &PACKET_MGR().harbor_memory; pcell->conversion_fxn = auto_refresh_harbor_memory_current_bytes_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icon_col1;



	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "cur Ttl segment" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = &PACKET_MGR().harbor_memory; pcell->conversion_fxn = auto_refresh_segment_total_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icon_col1;

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "cur fill segment" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = &PACKET_MGR().harbor_memory; pcell->conversion_fxn = auto_refresh_filled_count_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icon_col1;

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "cum fill segment" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = &PACKET_MGR().harbor_memory; pcell->conversion_fxn = auto_refresh_total_filled_segments_counter_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icon_col1;

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "cum new segment" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = &PACKET_MGR().harbor_memory; pcell->conversion_fxn = auto_refresh_newed_segments_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icon_col1;

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "cum del segment" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = &PACKET_MGR().harbor_memory; pcell->conversion_fxn = auto_refresh_released_segments_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icon_col1;

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "ram used" ) , 0); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = &PACKET_MGR().harbor_memory; pcell->conversion_fxn = auto_refresh_seg_ttl_sz_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icon_col1;

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "last pkt time" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = _g; pcell->conversion_fxn = auto_refresh_memory_time_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icon_col1;

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "sgmnt closed" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = &PACKET_MGR().harbor_memory; pcell->conversion_fxn = auto_refresh_segment_half_done_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icon_col1;

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "snd ful sgmnt" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = &PACKET_MGR().harbor_memory; pcell->conversion_fxn = auto_refresh_snd_ful_sgmnt_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icon_col1;

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "snd partial sgmnt" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = &PACKET_MGR().harbor_memory; pcell->conversion_fxn = auto_refresh_snd_half_sgmnt_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icon_col1;

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "cum tcp send" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = &PACKET_MGR().harbor_memory; pcell->conversion_fxn = auto_refresh_tcp_pass_cell;
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


	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "cum rejected UDP" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = _g; pcell->conversion_fxn = auto_refresh_cum_harbor_rejected_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icon_col1;

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "cum rejected UDP sz" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = _g; pcell->conversion_fxn = auto_refresh_cum_harbor_rejected_sz_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icon_col1;


#endif


#ifndef Storage
	int icon_col2 = 3;
	irow = 0; icol = icon_col2;

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "cur itm cnt" ) , 0); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = _g; pcell->conversion_fxn = auto_refresh_memmap_items_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icon_col2;

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "cur itm sz" ) , 0); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = _g; pcell->conversion_fxn = auto_refresh_memmap_items_sz_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icon_col2;

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "cur files cnt" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = _g; pcell->conversion_fxn = auto_refresh_files_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icon_col2;

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "cum snd pkt" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = _g; pcell->conversion_fxn = auto_refresh_memmap_items_sent_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icon_col2;

	
	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "low bar evac" ) , 0 ); pcell = NULL;
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

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "open reason" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = _g; pcell->conversion_fxn = auto_refresh_gateway_open_reason_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icon_col2;

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "last pkt time" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = _g; pcell->conversion_fxn = auto_refresh_memmap_time_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icon_col2;

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "cum remaped" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = _g; pcell->conversion_fxn = auto_refresh_cum_remaped_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icon_col2;

#endif


#ifndef statistics
	int icol_col3 = 5;
	irow = 0; icol = icol_col3;

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "sampling method" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = &PACKET_MGR().harbor_memory; pcell->conversion_fxn = auto_refresh_sampling_method_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icol_col3;

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , MEM_FAST " cur ipv4 cnt" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = &PACKET_MGR().harbor_memory; pcell->conversion_fxn = auto_refresh_pkt_in_L1Cache_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icol_col3;

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "evac coef" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = &PACKET_MGR().harbor_memory; pcell->conversion_fxn = auto_refresh_regres_cell;
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

	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "skip" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->conversion_fxn = auto_refresh_skip_n_item_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 ); irow++; icol = icol_col3;

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
	xudp_hdr * pkt1 = ( xudp_hdr * )buf;

	if ( PACKET_MGR().sampling_threshold_stage >= prss_skip_section )
	{
		_cum_harbor_rejected++;
		_cum_harbor_rejected_sz += sz;
		BREAK( errOK , 0 );
	}

	pkt1->metadata.state = ps_on_L2_RAM;
	bool bNewSegment = false;

	status ret = segmgr_append( &PACKET_MGR().harbor_memory , buf , sz , &bNewSegment ); // store whole pakt + hdr into global buffer
	RANJE_ACT1( ret , errArg , NULL_ACT , MACRO_E( M_BREAK_STAT( ret , 0 ) ) );

	if ( bNewSegment )
	{
		MEMSET_ZERO_O( &PACKET_MGR().latest_huge_memory_time ); // try to send new packet cause time reset
	}

	if ( ret == errOK )
	{
		cr_in_wnd_add_packet( &PACKET_MGR().ram_ctrl_loadOnInput , sz ); // calculate input rate
		cr_in_wnd_add_packet( &PACKET_MGR().ram_ctrl_instantaneousInputLoad , sz );
		//inst_rate_add_packet( &PACKET_MGR().ram_ctrl_instantaneousInputLoad , sz );

		_cum_harbor_input++;
		_cum_harbor_input_sz += (long long)sz;
	}
	else
	{
		_inner_status_error++;
	}

	BEGIN_RET
	default:
	{
		if ( d_error ) DIST_APP_FAILURE();
	}
	M_END_RET
}

// 1 . try to send packet under tcp to destination
// multi thread entrance . be quite
_PRIVATE_FXN _CALLBACK_FXN status send_segment_itm( buffer data , size_t len , pass_p src_g , void * nested_callback )
{
	G * _g = ( G * )src_g;
	status err_sent = errCanceled;

	status ret_lock = errCanceled;
	PKT_MGR_LOCK_LINE( ( ret_lock = pthread_mutex_timedlock_rel( &PACKET_MGR().pm_lock , 1000 TODO ) ) ); // prevent direct send and file send do work simultanously
	if ( ret_lock == errTimeout )
	{
		return ret_lock;
	}
	
	xudp_hdr * pkt1 = ( xudp_hdr * )data;
	size_t sz_t = pkt1->metadata.payload_sz;
	WARNING( pkt1->metadata.version == TCP_XPKT_V1 );
	bool try_resolve_route = false;

	switch( pkt1->metadata.state )
	{
		case ps_sent:
		case ps_original_remain_packet_on_ram:
		case ps_remain_packet_on_pub_file:
		{
			err_sent = errOK; goto _exit_pt;
		}
		default:;
	}
	//if ( pkt1->metadata.sent || pkt1->metadata.filed /*filed one on memory does not send*/ ) { err_sent = errOK; goto _exit_pt; }
	
	if ( _g->cmd.block_sending_1 ) { err_sent = errCanceled; goto _exit_pt; }

	time_t tnow = 0;
	tnow = time( NULL );
	sockfd fd = invalid_fd;
	void_p ab_tcp_p = NULL;
	AB * pb = NULL; /*if any data sent then statistics of that pb should be updated*/
	AB_tcp * ptcp = NULL;

	switch ( pkt1->metadata.state )
	{
		case ps_replicated_one_on_pub_file:
		case ps_replicated_one_on_priv_file:
		{
			if ( pkt1->metadata.cool_down_attempt && pkt1->metadata.cool_down_attempt == *( uchar * )&tnow ) // in one second we should not attempt . and this check and possiblity is rare in not too many attempt situation
			{
				{ err_sent = errTooManyAttempt; goto _exit_pt; } // cannot send and too many attempt to send
			}
			pkt1->metadata.cool_down_attempt = *( uchar * )&tnow;

			PACKET_MGR().latest_memmap_time = pkt1->metadata.udp_hdr.tm; // after memmap packet gone it is time for huge memory packet
			break;
		}
		case ps_on_L2_RAM:
		{
			PACKET_MGR().latest_huge_memory_time = pkt1->metadata.udp_hdr.tm;
			break;
		}
		default:;
	}
	// normal packet come and go. retried packet should just checked
	//if ( pkt1->metadata.is_faulti ) // faulty item should not have too many attempt
	//{
	//	if ( pkt1->metadata.cool_down_attempt && pkt1->metadata.cool_down_attempt == *( uchar * )&tnow ) // in one second we should not attempt . and this check and possiblity is rare in not too many attempt situation
	//	{
	//		{ err_sent = errTooManyAttempt; goto _exit_pt; } // cannot send and too many attempt to send
	//	}
	//	pkt1->metadata.cool_down_attempt = *( uchar * )&tnow;
	//	PACKET_MGR().latest_memmap_time = pkt1->metadata.udp_hdr.tm; // after memmap packet gone it is time for huge memory packet
	//}
	//else // if not come from memmap . if there is huge cache packet that has oldest time compare to memmap packet then memmap packet should be send
	//{
	//	PACKET_MGR().latest_huge_memory_time = pkt1->metadata.udp_hdr.tm;
	//}
	
	// fast method
	if ( dict_fst_get_faster_by_hash_id( &PACKET_MGR().map_tcp_socket , pkt1->metadata.tcp_name_key_hash , pkt1->metadata.tcp_name_uniq_id , &fd , &ab_tcp_p ) == errOK && fd != INVALID_FD )
	{ // fast method
		
		if ( ab_tcp_p )
		{
			if ( ( ptcp = ( AB_tcp * )ab_tcp_p ) )
			{
				pb = ptcp->owner_pb;
			}

			if ( ptcp->__tcp_cfg_pak->data.send_gap_nsec > 0 ) // used when we need debounce in sending
			{
				timespec tnow_nano;
				clock_gettime( CLOCK_REALTIME , &tnow_nano );
				if ( timespec_diff_nsec( &ptcp->this->last_send_ts , &tnow_nano ) < ptcp->__tcp_cfg_pak->data.send_gap_nsec )
				{
					cr_in_wnd_add_packet( &ptcp->brdg_rate_ctrl_loadOnOutBridge , 0 );
					////cr_in_wnd_add_packet( &PACKET_MGR().ram_ctrl_loadOnOutBridge , 0 ); // no need for this . because bridge control different by ram control
					err_sent = errPortOccupied;
					goto _exit_pt;
				}
			}
			if ( ptcp->__tcp_cfg_pak->data.send_throughput_limit_Bps > 0 ) // used when we need limit in througput
			{
				cr_in_wnd_add_packet( &ptcp->brdg_rate_ctrl_loadOnOutBridge , 0 );
				if ( cr_in_wnd_get_Bps( &ptcp->brdg_rate_ctrl_loadOnOutBridge ) > ptcp->__tcp_cfg_pak->data.send_throughput_limit_Bps )
				{
					////cr_in_wnd_add_packet( &PACKET_MGR().ram_ctrl_loadOnOutBridge , 0 ); // when calced it aligned to current time automatically
					err_sent = errPortOccupied;
					goto _exit_pt;
				}
			}
		}

		IMMORTAL_LPCSTR errString = NULL;
		uchar buf[MIN_SYSERR_BUF_SZ] = {0};
		err_sent = tcp_send_all( fd , data + pkt1->metadata.payload_offset , sz_t , 0 , SEND_TIMEOUT_ms , ACK_TIMEOUT_ms , RETRY_MECHANISM , &errString , ( buffer * )&buf ); // send is too heavy
		//if ( errString != errOK )
		//{
		//	//struct in_addr addr = {0};
		//	//addr.s_addr = pkt1->metadata.udp_hdr.dstIP;  // must already be in network byte order
		//	//char str[ INET_ADDRSTRLEN ] = {0};
		//	//inet_ntop( AF_INET , &addr , str , INET_ADDRSTRLEN );
		//#ifdef ENABLE_LOGGING
		//	//log_write( LOG_ERROR , "%d %s %s %s %s" , __LINE__ , errString , buf , pkt1->TCP_name , str );
		//#endif
		//}
		
		switch ( err_sent )
		{
			case errOK:
			{
			#ifdef ENABLE_USE_DBG_TAG
				_send_by_seek++;
			#endif
				//pkt1->metadata.sent = true;
				pkt1->metadata.prev_state = pkt1->metadata.state;
				pkt1->metadata.state = ps_sent;

				if ( ptcp )
				{
					// set time
					clock_gettime( CLOCK_REALTIME , &ptcp->this->last_send_ts ); // last send time use in debuncing between sending
					ptcp->this->last_action_ts = ptcp->this->last_send_ts; // last action time on tcp connection use in rety to reconnect broken tcp

					// post action
					if ( iSTR_SAME( ptcp->__tcp_cfg_pak->data.post_action , "post enter" ) )
					{
						err_sent = tcp_send_all( fd , "\n" , 1 , 0 , SEND_TIMEOUT_ms , ACK_TIMEOUT_ms , RETRY_MECHANISM , &errString , ( buffer * )&buf ); // send is too heavy
					}

					// benchmarking send on tcp and used in output rate controlling
					cr_in_wnd_add_packet( &ptcp->brdg_rate_ctrl_loadOnOutBridge , len ); // limiter for output
				}

				switch ( pkt1->metadata.prev_state )
				{
					case ps_on_L2_RAM: // if packet come from ram then sending do lighteen ram usage
					{
						cr_in_wnd_add_packet( &PACKET_MGR().ram_ctrl_loadOnOutBridge , len );
						break;
					}
					default:;
				}
				//if ( !pkt1->metadata.is_faulti ) // just cleaning up ram packet should considered
				//{ // 
				//	//cr_in_wnd_add_packet( &PACKET_MGR().longTermTcpOutLoad , len );
				//////	cr_in_wnd_add_packet( &PACKET_MGR().ram_ctrl_loadOnOutBridge , len );
				//}
				break;
			}
			case errNoPeer:
			case errPeerClosed:
			{
				if ( ptcp )
				{
					if ( !ptcp->this->tcp_is_about_to_connect ) // if another request is attempt then we should waint to complete
					{
						ptcp->this->tcp_is_about_to_connect = 1;
						//ptcp->retry_to_connect_tcp = 1;
						ptcp->this->tcp_connection_established = 0;
					}
				}

				if ( !errString ) /*because before it logged*/
				{
				#ifdef ENABLE_LOGGING
					log_write( LOG_ERROR , "%d err:%s" , __LINE__ , internalErrorStr( err_sent , true ) );
				#endif
				}
				break;
			}
			case errGeneral:
			{
				if ( pb )
				{
					al_alive( &pb->stat.tcp_port_err_indicator , false );
				}
				_inner_status_error++;
				break;
			}
			default:
			{
				if ( pb )
				{
					al_alive( &pb->stat.tcp_port_err_indicator , false );
				}
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

		// if send couldnt happened then we should move forward benchmarking point
		if ( err_sent != errOK && ptcp )
		{
			cr_in_wnd_add_packet( &ptcp->brdg_rate_ctrl_loadOnOutBridge , 0 ); // need this
			////cr_in_wnd_add_packet( &PACKET_MGR().ram_ctrl_loadOnOutBridge , 0 ); // no need for this . just need upper line
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
	//if ( !pkt1->metadata.sent && !ab_tcp_p && _g->bridges.connected_tcp_out )
	if ( pkt1->metadata.state != ps_sent && !ab_tcp_p && _g->bridges.connected_tcp_out )
	{
		PKT_MGR_LOCK_LINE( pthread_mutex_lock( &_g->bridges.tcps_trd.mtx ) );
		{
			for ( size_t iab = 0 ; iab < _g->bridges.ABs.count ; iab++ )
			{
				if ( mms_array_get_s( &_g->bridges.ABs , iab , ( void ** )&pb ) == errOK )
				{
					for ( int itcp = 0 ; itcp < pb->tcps_count ; itcp++ )
					{
						if ( STR_SAME( pb->tcps[ itcp ].__tcp_cfg_pak->name , pkt1->TCP_name ) )
						{
							ptcp = pb->tcps[ itcp ].this; // always use this instead of owner of this because responsiblity deligate to this. see bridge

							if ( ptcp->tcp_connection_established )
							{
								if ( ptcp->__tcp_cfg_pak->data.send_gap_nsec > 0 ) // used when we need debounce in sending
								{
									timespec tnow_nano;
									clock_gettime( CLOCK_REALTIME , &tnow_nano );
									if ( timespec_diff_nsec( &ptcp->this->last_send_ts , &tnow_nano ) < ptcp->__tcp_cfg_pak->data.send_gap_nsec )
									{
										pthread_mutex_unlock( &_g->bridges.tcps_trd.mtx );
										err_sent = errPortOccupied;

										if ( ptcp )
										{
											cr_in_wnd_add_packet( &ptcp->brdg_rate_ctrl_loadOnOutBridge , 0 );
											////cr_in_wnd_add_packet( &PACKET_MGR().ram_ctrl_loadOnOutBridge , 0 );
										}
										goto _exit_pt;
									}
								}
								if ( ptcp->__tcp_cfg_pak->data.send_throughput_limit_Bps > 0 ) // used when we need limit in througput
								{
									if ( ptcp )
									{
										cr_in_wnd_add_packet( &ptcp->brdg_rate_ctrl_loadOnOutBridge , 0 );
										////cr_in_wnd_add_packet( &PACKET_MGR().ram_ctrl_loadOnOutBridge , 0 );
									}
									if ( cr_in_wnd_get_Bps( &ptcp->brdg_rate_ctrl_loadOnOutBridge ) > ptcp->__tcp_cfg_pak->data.send_throughput_limit_Bps )
									{
										pthread_mutex_unlock( &_g->bridges.tcps_trd.mtx );
										err_sent = errPortOccupied;
										goto _exit_pt;
									}
								}
							
								IMMORTAL_LPCSTR errString = NULL;
								uchar buf[MIN_SYSERR_BUF_SZ] = {0};
								err_sent = tcp_send_all( ptcp->tcp_sockfd , data + pkt1->metadata.payload_offset , sz_t , 0 , SEND_TIMEOUT_ms , ACK_TIMEOUT_ms , RETRY_MECHANISM , &errString , ( buffer * )&buf ); // send is to heavy
								if ( errString != errOK )
								{
								#ifdef ENABLE_LOGGING
									log_write( LOG_ERROR , "%d %s %s %s" , __LINE__ , errString , buf , pkt1->TCP_name );
								#endif
								}
								switch ( err_sent )
								{
									case errOK:
									{
									#ifdef ENABLE_USE_DBG_TAG
										_send_by_lookup++;
									#endif
										//pkt1->metadata.sent = true;

										pkt1->metadata.prev_state = pkt1->metadata.state;
										pkt1->metadata.state = ps_sent;

										clock_gettime( CLOCK_REALTIME , &pb->tcps[ itcp ].last_send_ts );
										pb->tcps[ itcp ].last_action_ts = pb->tcps[ itcp ].last_send_ts;

										if ( try_resolve_route )
										{
											try_resolve_route = false;
											dict_fst_put( &PACKET_MGR().map_tcp_socket , pkt1->TCP_name , ptcp->tcp_sockfd , ( void_p )ptcp , NULL , NULL , NULL );
										}

										switch ( pkt1->metadata.prev_state )
										{
											case ps_on_L2_RAM: // ram is freeing
											{
												cr_in_wnd_add_packet( &PACKET_MGR().ram_ctrl_loadOnOutBridge , len );
												break;
											}
											default:;
										}
										//if ( !pkt1->metadata.is_faulti ) // faulti item come from file so why should i conder them in calculation
										//{
										//	//cr_in_wnd_add_packet( &PACKET_MGR().longTermTcpOutLoad , len );
										//}

										cr_in_wnd_add_packet( &ptcp->brdg_rate_ctrl_loadOnOutBridge , len );

										if ( iSTR_SAME( ptcp->__tcp_cfg_pak->data.post_action , "post enter" ) )
										{
											err_sent = tcp_send_all( ptcp->tcp_sockfd , "\n" , 1 , 0 , SEND_TIMEOUT_ms , ACK_TIMEOUT_ms , RETRY_MECHANISM , &errString , ( buffer * )&buf ); // send is too heavy
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
										al_alive( &pb->stat.tcp_port_err_indicator , false );

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

								if ( err_sent != errOK && ptcp )
								{
									cr_in_wnd_add_packet( &ptcp->brdg_rate_ctrl_loadOnOutBridge , 0 );
									////cr_in_wnd_add_packet( &PACKET_MGR().ram_ctrl_loadOnOutBridge , 0 );
								}
								break;
							}
						}
					}
					//if ( pkt1->metadata.sent )
					if ( pkt1->metadata.state == ps_sent )
					{
						break;
					}
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

	//if ( pkt1->metadata.sent )
	if ( pkt1->metadata.state == ps_sent )
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
		//	if ( segmgr_append( &PACKET_MGR().sent_package_log , &pkt1->metadata.udp_hdr , sizeof( pkt1->metadata.udp_hdr ) ) == errOK )
		//	{
		//		pkt1->metadata.udp_hdr.logged_2_mem = true;
		//	}
		//}

		if ( err_sent == errCanceled && !_g->bridges.connected_tcp_out )
		{
			err_sent = errNoPeer;
		}
	}

	if ( pb ) // after it is settled that send done or no throughput markers updated
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
	pthread_mutex_unlock( &PACKET_MGR().pm_lock );
	return err_sent;
}
_PRIVATE_FXN _CALLBACK_FXN status on_ram_send_segment_itm_wrapper( buffer data , size_t len , pass_p src_g , void * ret_val )
{
	xudp_hdr * pkt1 = ( xudp_hdr * )data;
	status d_error = send_segment_itm( data , len , src_g , NULL );
	if ( d_error == errOK )
	{
		inst_rate_add_packet( &PACKET_MGR().ram_ctrl_instantaneousloadOnOutBridge , pkt1->metadata.payload_sz );
		size_t rateIn = ( size_t )cr_in_wnd_get_Bps( &PACKET_MGR().ram_ctrl_instantaneousInputLoad );
		size_t rateOut;
		double elapsed_sec;
		if ( inst_rate_loadBps( &PACKET_MGR().ram_ctrl_instantaneousloadOnOutBridge , &rateOut , &elapsed_sec ) == errOK )
		{
			if ( rateIn > rateOut && rateOut > 0 )
			{
				if ( ( ( *( size_t * )ret_val ) = MIN( ( ( size_t )ceil( rateIn / rateOut ) ) - 1 , 100/*with 2MB segment sz , pkt sz 3.4k , snd 1% of pkt*/) ) )
				{
					_skip_n_item = ( int )( *( size_t * )ret_val );
					return errSkip;
				}
			}
		}
	}
	return d_error;
}


// 2 . if sending under tcp fail try to archive them( segment )
_PRIVATE_FXN _CALLBACK_FXN status on_ram_store_itm_wrapper( buffer data , size_t len , pass_p src_g , void * nested_callback )
{
	xudp_hdr * pkt1 = ( xudp_hdr * )data;
	status d_error = ( ( seg_item_cb )nested_callback )( data , len , src_g , NULL );
	if ( d_error == errOK )
	{
		_sampled_packet++;
	}
	return d_error;
}
_PRIVATE_FXN _CALLBACK_FXN status store_segment_item( buffer data , size_t len , pass_p src_g , void * nested_callback )
{
	G * _g = ( G * )src_g;
	status d_error = errCanceled;
	xudp_hdr * pkt1 = ( xudp_hdr * )data;
	size_t sz_t = pkt1->metadata.payload_sz;

	switch ( pkt1->metadata.state ) //if ( pkt1->metadata.sent || /*pkt1->metadata.is_faulti ||*/ pkt1->metadata.filed ) { return errOK; }
	{
		default:
		{
			return errOK;
		}
		case ps_on_L2_RAM:; // white list ish
	}
	
	if ( PACKET_MGR().latest_memmap_time.tv_sec && PACKET_MGR().latest_memmap_time.tv_sec > pkt1->metadata.udp_hdr.tm.tv_sec )
	{
		// just because olds segment also released then latest_memmap_time must be latest faulti packet
	}
	else
	{
		PACKET_MGR().latest_memmap_time = pkt1->metadata.udp_hdr.tm; // wheter memmap packet is most important so first send them
	}

	//pkt1->metadata.is_faulti = 1;
	pkt1->metadata.prev_state = pkt1->metadata.state;
	pkt1->metadata.state = ps_replicated_one_on_pub_file; // store this state at file

//#ifdef ENABLE_USE_DBG_TAG
//	_mem_to_tcp_failure++;
//#endif

#ifdef ENABLE_PERSISTENT_CACHE
	// try to seperate requestor and actually archiver.
	d_error = distributor_publish_buffer_size( &_g->hdls.prst_csh.bcast_store_data , data , len , src_g ); // in this spoint packet replicated
#else
	d_error = errOK;
#endif
	
	if ( d_error == errOK )
	{
		pkt1->metadata.state = ps_original_remain_packet_on_ram; // in this spoint packet replicated
		//pkt1->metadata.filed = true; // after sure that is stored on file set filed . so retry this on memory cause stoped. and that one from file is about to sent
		cr_in_wnd_add_packet( &PACKET_MGR().ram_ctrl_loadOnStorage , len );
		//cr_in_wnd_add_packet( &PACKET_MGR().ram_ctrl_instantaneousloadOnStorage , len );
	}
	else
	{
		_inner_status_error++;
	}

	// write it down on disk
	// later we can find logged that does not sent and find the bug
	
	return d_error;
}


// emptied buffer cache( harbor_memory ) then on failure go to persistent storage cache and get from it
// this fxn do empty segment by segment
_THREAD_FXN void_p process_filled_tcp_segment_proc( pass_p src_g )
{
	G * _g = ( G * )src_g;

#ifdef ENABLE_LOG_THREADS
	__attribute__( ( cleanup( thread_goes_out_of_scope ) ) ) pthread_t this_thread = pthread_self();
	distributor_publish_x3long( &_g->distributors.bcast_thread_startup , ( long )this_thread , trdn_process_filled_tcp_segment_proc , ( long )__FUNCTION__ , _g );

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

		tm_cmp = timeval_compare( &PACKET_MGR().latest_huge_memory_time , &PACKET_MGR().latest_memmap_time );
		if
		(
			!PACKET_MGR().latest_huge_memory_time.tv_sec ||
			!PACKET_MGR().latest_memmap_time.tv_sec ||
			tm_cmp >= 0 ||
			_g->cmd.block_sending_1 ||
			PACKET_MGR().sampling_threshold_stage >= prss_skip_section
		)
		{
			pseg = segmgr_pop_filled_segment( &PACKET_MGR().harbor_memory , False , seg_trv_LIFO );
			if ( pseg ) // poped on memory packets
			{
				all_poped_send_suc = true;

				cpu_unburne.idx = ( cpu_unburne.idx + 1 ) % ( uchar )sizeof( cpu_unburne.arr );
				cpu_unburne.arr.bytes[ cpu_unburne.idx ] = 1;
				if ( _g->hdls.gateway.pagestack_gateway_open_val == gws_open )
				{
					_g->hdls.gateway.pagestack_gateway_open_val = gws_close;
#ifdef ENABLE_USE_DBG_TAG
					_close_gate_cnt++;
#endif
				}

				// try to send from mem under tcp to dst
				inst_rate_init2start( &PACKET_MGR().ram_ctrl_instantaneousloadOnOutBridge );
				inst_rate_add_packet( &PACKET_MGR().ram_ctrl_instantaneousloadOnOutBridge , 0 ); // just start timer to enable balancer calculation
				size_t skip_n = 0;
				if ( _g->cmd.block_sending_1 || ci_sgm_iter_items( pseg , on_ram_send_segment_itm_wrapper , src_g , true , PACKET_MGR().sampling_sent_packet_stride , tail_2_head , ( void_p )&skip_n ) != errOK ) // some fault detected
				{
					all_poped_send_suc = false;
					// each failed on packet sending cause its moved to memmap
					ci_sgm_iter_items( pseg , on_ram_store_itm_wrapper , src_g , true , 1 , head_2_tail , store_segment_item );
				}
				// then close segment
				if ( ( d_error = ci_sgm_mark_empty( &PACKET_MGR().harbor_memory , pseg ) ) == errEmpty ) // pop last emptied segment
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
				cpu_unburne.idx = ( cpu_unburne.idx + 1 ) % ( uchar )sizeof( cpu_unburne.arr );
				cpu_unburne.arr.bytes[ cpu_unburne.idx ] = 0;

				gateway_open_stat tmp_old = _g->hdls.gateway.pagestack_gateway_open_val;
				if ( !tmp_old )
				{
#ifdef ENABLE_USE_DBG_TAG
					_open_gate_cnt++;
#endif
					_open_gate_no_segment_cnt++;
					_g->hdls.gateway.pagestack_gateway_open_val = gws_open;
					sem_post( &_g->hdls.gateway.pagestack_gateway_open_sem );
				}
			}
		}
		else
		{
			cpu_unburne.idx = ( cpu_unburne.idx + 1 ) % ( uchar )sizeof( cpu_unburne.arr );
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
				if
				(
					PACKET_MGR().latest_huge_memory_time.tv_sec &&
					PACKET_MGR().latest_memmap_time.tv_sec &&
					tm_cmp < 0
				)
				{
					_open_gate_l3_order_cnt++;
				}

				_g->hdls.gateway.pagestack_gateway_open_val = gws_open;
				sem_post( &_g->hdls.gateway.pagestack_gateway_open_sem );
			}
		}

		if ( !cpu_unburne.arr.big_data.a && !cpu_unburne.arr.big_data.b ) // enough time for packet arrive
		{
			mng_basic_thread_sleep( _g , HI_PRIORITY_THREAD ); // make balance between cpu burst and network bandwidth time
		}
	} while ( 1 );
	if ( _g->hdls.gateway.pagestack_gateway_open_val == gws_open )
	{
		_g->hdls.gateway.pagestack_gateway_open_val = gws_close;
	}

	return NULL;
}

#ifdef ENABLE_PERSISTENT_CACHE

_PRIVATE_FXN _CALLBACK_FXN status remap_storage_data( pass_p src_g , buffer buf , size_t sz ) // call from thread discharge_persistent
{
	G * _g = ( G * )src_g;
	status d_error = errCanceled;
	xudp_hdr * pkt1 = ( xudp_hdr * )buf;

	//if ( pkt1->metadata.is_remapped )
	if ( pkt1->metadata.state == ps_replicated_one_on_priv_file )
	{
		return errDoneAlready;
	}

	//pkt1->metadata.is_remapped = true; // first it should set to mapped but if failed then remove this flag
	pkt1->metadata.prev_state = pkt1->metadata.state;
	pkt1->metadata.state = ps_replicated_one_on_priv_file;
	d_error = distributor_publish_buffer_size_data( &_g->hdls.prst_csh.bcast_reroute_nopeer_pkt , buf , sz , ( long )pkt1->TCP_name , src_g ); // in this point packet replicated

	if ( d_error == errOK )
	{
		pkt1->metadata.state = ps_remain_packet_on_pub_file; // copy of packet move into priv_file then remain one is still exist in pub_file . but it poped from stack
		_remaped_packet++;
		return d_error;
	}
	//pkt1->metadata.is_remapped = false;
	pkt1->metadata.state = pkt1->metadata.prev_state; // if it does not move to prib_file then rollback state
	return d_error;
}

/* try send in file data */
_CALLBACK_FXN status discharge_persistent_storage_data( pass_p src_g , buffer buf , size_t sz )
{
	G * _g = ( G * )src_g;
	xudp_hdr * pkt1 = ( xudp_hdr * )buf;

	// there is condition that need just skip in memory packets. so output port most be unoccopied just to send in mem packets
	if ( PACKET_MGR().sampling_threshold_stage >= prss_skip_section )
	{
		return errPortOccupied;
	}

	// when no data in memmap then time reset to zero and actual send data if wait then release and do his work
	if ( buf == NULL && !sz )
	{
		if ( PACKET_MGR().latest_memmap_time.tv_sec )
		{
			MEMSET_ZERO_O( &PACKET_MGR().latest_memmap_time ); // store failed packet reset value
		}
		return errOK;
	}


	pkt1->metadata.prev_state = pkt1->metadata.state; // be here to insist that it come from pub_file
	//pkt1->state = pub_file2sending; // just in file packets arrive here so for next state just sent is probable
	status d_error = send_segment_itm( buf , sz , src_g , NULL );
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

#endif

#ifdef ENABLE_CLEAN_UNUSED_SEGMENT

/*cleanup long time idle segment that keep memory just occupied and unused*/
_THREAD_FXN void_p cleanup_unused_idle_segment_proc( pass_p src_g )
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

		size_t sampling_sent_packet_stride = MAX( PACKET_MGR().sampling_sent_packet_stride , 1 );

		segmgr_cleanup_idle( &PACKET_MGR().harbor_memory , ( time_t )MAX( ( size_t )CFG().L2_unused_segment_holdon_time_sec / sampling_sent_packet_stride , 1 ) ); // if there is no work to do clean unused segment

		mng_basic_thread_sleep( _g , NORMAL_PRIORITY_THREAD );
	} while ( 1 );

	return NULL;
}

#endif

#ifdef ENABLE_STORE_OLD_FILLED_LOW_PROBABLE_SENDABLE_SEGMENT

/* each packet come here to be sent */
_PRIVATE_FXN _CALLBACK_FXN status store_old_segment_item( buffer data , size_t len , pass_p src_g , void * nested_callback )
{
	G * _g = ( G * )src_g;
	xudp_hdr * pkt1 = ( xudp_hdr * )data;

	struct timeval tnow;
	gettimeofday( &tnow , NULL );
	double df_sec = timeval_diff_ms( &pkt1->metadata.udp_hdr.tm , &tnow ) / 1000;
	if ( df_sec < CFG().L2_old_udp_holdon_timeout_sec ) // tolerate on new segment persistent
	{
		_evac_segment_paused++;
		return errNoCountinue;
	}
	status d_error = store_segment_item( data , len , src_g , NULL );
	if ( d_error == errOK )
	{
		_packet_to_file_by_mem_evacuation++;
	}
	return d_error;
}
// try to move in memory packet into file just to adapt with memory overfill condition
_THREAD_FXN void_p evacuate_long_time_sediment_segment_proc( pass_p src_g )
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

	time_t last_assign = {0};

	do
	{
		if ( pthis_thread_alive_time ) *pthis_thread_alive_time = time( NULL );
		if ( GRACEFULLY_END_THREAD() ) break;

		mng_basic_thread_sleep( _g , NORMAL_PRIORITY_THREAD );

		// rule1. goal is release much memory to remain mem could be release in less than spec time

		cr_in_wnd_add_packet( &PACKET_MGR().ram_ctrl_loadOnStorage , 0 );
		//cr_in_wnd_add_packet( &PACKET_MGR().ram_ctrl_loadOnOutBridge , 0 );
		cr_in_wnd_add_packet( &PACKET_MGR().ram_ctrl_loadOnInput , 0 );
		//cr_in_wnd_add_packet( &PACKET_MGR().ram_ctrl_instantaneousloadOnStorage , 0 );
		cr_in_wnd_add_packet( &PACKET_MGR().ram_ctrl_instantaneousInputLoad , 0 );

		//if ( time( NULL ) - t1 > 5 )
		//{
		//	_release_lock_countdown = 0;
		//}

		if ( ci_sgm_get_oldest_maintained_segment( &PACKET_MGR().harbor_memory , NULL ) != errOK ) continue;

		size_t loadOnStorage = ( size_t )cr_in_wnd_get_Bps( &PACKET_MGR().ram_ctrl_loadOnStorage );
		//size_t loadOnOutBridge = ( size_t )cr_in_wnd_get_Bps( &PACKET_MGR().ram_ctrl_loadOnOutBridge );
		size_t loadOnInput = ( size_t )cr_in_wnd_get_Bps( &PACKET_MGR().ram_ctrl_loadOnInput );
		size_t instantaneousInputLoad = ( size_t )cr_in_wnd_get_Bps( &PACKET_MGR().ram_ctrl_instantaneousInputLoad );
		//size_t instantaneousloadOnStorage = ( size_t )cr_in_wnd_get_Bps( &PACKET_MGR().ram_ctrl_instantaneousloadOnStorage );

		double coef = CFG().instantaneous_input_load_coefficient;
		size_t input_load = ( ( size_t )( coef * instantaneousInputLoad ) ) + ( ( size_t )( ( 1 - coef ) * loadOnInput ) ); // partial part from instantaneous and complement from long term

		if ( !PACKET_MGR().harbor_memory.current_items ) continue;

		size_t avg_inp_pkt_sz = PACKET_MGR().harbor_memory.current_bytes / PACKET_MGR().harbor_memory.current_items;
		size_t pkt_fit_in_L1cache = ( size_t )( CFG().L1_cache_sz_byte * 0.95/*overhead space*/ );
		if ( input_load >= pkt_fit_in_L1cache ) continue; // if load is too much then no time for extra lock to clear memory
		size_t inp_load_latency = input_load * ( size_t )CFG().L1_2_L2_byte_copy_latency_nsec;
		if ( inp_load_latency >= 1000000000L ) continue; // load bar can occopied whole second
		size_t one_sec_free_time_nsec = 1000000000L - inp_load_latency;
		if ( !CFG().L1_2_L2_byte_copy_latency_nsec ) continue;
		if ( !avg_inp_pkt_sz ) continue;
		
		size_t release_lock_countdown = one_sec_free_time_nsec / ( size_t )CFG().L1_2_L2_byte_copy_latency_nsec / avg_inp_pkt_sz;
		if ( release_lock_countdown <= _release_lock_countdown )
		{
			_release_lock_countdown = release_lock_countdown;
		}
		else
		{
			if ( time( NULL ) - last_assign > 5 )
			{
				_release_lock_countdown = ( int )MIN( MIN( ( size_t )( _release_lock_countdown * 0.1 ) , release_lock_countdown ) , 1000 );
				last_assign = time( NULL );
			}
		}
		//if ( _release_lock_countdown <= 0 ) continue; // still try free if nothing arrive
		segmgr_try_process_filled_segment( &PACKET_MGR().harbor_memory , store_old_segment_item , src_g , seg_trv_FIFO_nolock , _release_lock_countdown );
		
	} while ( 1 );

	return NULL;
}

#endif

#ifdef ENABLE_RELEASE_HALFFILL_UNUSED_SEGMENT

// check last packet of segment then if packet idle for long time then close segment
_PRIVATE_FXN _CALLBACK_FXN bool release_halffill_segment_condition( const buffer buf , size_t sz )
{
	if ( _g->cmd.cleanup_state <= getting_new_udp_stoped ) return true;
	xudp_hdr * pkt1 = ( xudp_hdr * )buf;
	WARNING( pkt1->metadata.version == TCP_XPKT_V1 );

	struct timeval tnow;
	gettimeofday( &tnow , NULL );
	double df_sec = timeval_diff_ms( &pkt1->metadata.udp_hdr.tm , &tnow ) / 1000;
	return df_sec > CFG().L2_idle_active_segment_expiration_timeout_sec;
}
// check if condition is true then set halffill segemtn as fill
_CALLBACK_FXN void try_release_halffill_segment( pass_p src_g , long v )
{
	G * _g = ( G * )src_g;
	if ( ci_sgm_peek_decide_active( &PACKET_MGR().harbor_memory , release_halffill_segment_condition ) )
	{
		_release_half_segment++;
		//MEMSET_ZERO_O( &PACKET_MGR().latest_huge_memory_time ); // try to send new packet cause time reset
	}
}
_THREAD_FXN void_p proc_and_release_halffill_segment( pass_p src_g )
{
	G * _g = ( G * )src_g;
	status d_error;

#ifdef ENABLE_LOG_THREADS
	__attribute__( ( cleanup( thread_goes_out_of_scope ) ) ) pthread_t this_thread = pthread_self();
	distributor_publish_x3long( &_g->distributors.bcast_thread_startup , ( long )this_thread , trdn_try_to_release_halffill_segment , ( long )__FUNCTION__ , _g );

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

		try_release_halffill_segment( src_g , NP );

		mng_basic_thread_sleep_sec( _g , CFG().L2_idle_active_iteration_sec );

	} while ( 1 );

	return NULL;
}

#endif

#ifndef calc_sampling

int64 _seg_ttl_sz = 0;
double _remain2use = 0;
double _longTermInputLoad = 0;
double _longTermToStorageOutLoad = 0;
double _instantaneousInputLoad = 0;
double _longTermOutLoad = 0;
double _TTF = 0;

// every one second
_CALLBACK_FXN void sampling_filled_segment_count( pass_p src_g )
{
	G * _g = ( G * )src_g;
	
	cr_in_wnd_add_packet( &PACKET_MGR().ram_ctrl_loadOnInput , 0 );
	cr_in_wnd_add_packet( &PACKET_MGR().ram_ctrl_instantaneousInputLoad , 0 );

	cr_in_wnd_add_packet( &PACKET_MGR().ram_ctrl_loadOnStorage , 0 );
	cr_in_wnd_add_packet( &PACKET_MGR().ram_ctrl_loadOnOutBridge , 0 );

	double ram_ctrl_loadOnInput = cr_in_wnd_get_Bps( &PACKET_MGR().ram_ctrl_loadOnInput );
	double ram_ctrl_instantaneousInputLoad = cr_in_wnd_get_Bps( &PACKET_MGR().ram_ctrl_instantaneousInputLoad );

	double ram_ctrl_loadOnStorage = cr_in_wnd_get_Bps( &PACKET_MGR().ram_ctrl_loadOnStorage );
	double ram_ctrl_loadOnOutBridge = cr_in_wnd_get_Bps( &PACKET_MGR().ram_ctrl_loadOnOutBridge );

	double coef = CFG().instantaneous_input_load_coefficient;
	double input_rate = ( coef * ram_ctrl_instantaneousInputLoad ) + ( ( 1 - coef ) * ram_ctrl_loadOnInput ); // partial part from instantaneous and complement from long term
	double rate_velocity = input_rate - ( ram_ctrl_loadOnOutBridge + ram_ctrl_loadOnStorage );
	
	PACKET_MGR().sampling_threshold_stage = prss_no_pressure;
	
	_longTermInputLoad = ram_ctrl_loadOnInput;
	_instantaneousInputLoad = ram_ctrl_instantaneousInputLoad;
	_longTermToStorageOutLoad = ram_ctrl_loadOnStorage;
	_longTermOutLoad = ram_ctrl_loadOnOutBridge;

#ifndef TTF // mechanism on last duration
	PACKET_MGR().TTF_sampling_sent_packet_stride = 1;
	int64 seg_ttl_sz = ( int64 )( PACKET_MGR().harbor_memory.segment_total * PACKET_MGR().harbor_memory.default_seg_capacity );
	double remain2use = ( double )( MAX( CFG().L2_allowed_ram_allocation - seg_ttl_sz , 0 ) );
	_seg_ttl_sz = seg_ttl_sz;
	_remain2use = remain2use;
	
	double TTF = remain2use / MAX( rate_velocity , 0.1 ); /*time to fill*/
	_TTF = TTF;
	if ( TTF > CFG().TTF_no_backpressure_threshold_sec )
	{
		PACKET_MGR().TTF_sampling_sent_packet_stride = 1;
		PACKET_MGR().sampling_threshold_stage = prss_no_pressure;
	}
	else if ( TTF > CFG().TTF_gentle_backpressure_threshold_sec )
	{
		PACKET_MGR().TTF_sampling_sent_packet_stride = (size_t)CFG().TTF_gentle_backpressure_stride;
		PACKET_MGR().sampling_threshold_stage = prss_gentle_pressure;
	}
	else if ( TTF > CFG().TTF_aggressive_backpressure_threshold_sec )
	{
		PACKET_MGR().TTF_sampling_sent_packet_stride = (size_t)CFG().TTF_aggressive_backpressure_stride;
		PACKET_MGR().sampling_threshold_stage = prss_aggressive_pressure;
	}
	else if ( TTF > CFG().TTF_emergency_drop_backpressure_threshold_sec )
	{
		PACKET_MGR().TTF_sampling_sent_packet_stride = (size_t)CFG().TTF_emergency_drop_backpressure_stride;
		PACKET_MGR().sampling_threshold_stage = prss_emergency_pressure;
	}
	else if ( TTF > CFG().TTF_skip_input_threshold_sec )
	{
		PACKET_MGR().TTF_sampling_sent_packet_stride = (size_t)CFG().TTF_red_zone_stride;
		PACKET_MGR().sampling_threshold_stage = prss_red_zone_pressure;
	}
	else
	{
		PACKET_MGR().TTF_sampling_sent_packet_stride = (size_t)CFG().TTF_red_zone_stride;
		PACKET_MGR().sampling_threshold_stage = prss_skip_input_by_memory_fulled;
	}
#endif

#ifndef hard_is_full
	size_t page_occupied_MB = pg_stk_get_page_occupied_MB( &_g->hdls.prst_csh.page_stack );
	if ( page_occupied_MB > CFG().max_saved_file_size_threshold_MB )
	{
		PACKET_MGR().TTF_sampling_sent_packet_stride = 1;
		PACKET_MGR().sampling_threshold_stage = prss_skip_input_by_hard_fulled;
	}
#endif

//#ifndef hard_full
//	// warning: be aware of difference between each port rate and total rate
//	PACKET_MGR().passable_sampling_rate_stride = 1;
//	if ( PACKET_MGR().harbor_memory.current_items > 0 )
//	{
//		double avg_pkt_size = PACKET_MGR().harbor_memory.current_bytes / PACKET_MGR().harbor_memory.current_items;
//		if ( avg_pkt_size > 0 )
//		{
//			double input_pkt_rate = input_rate / avg_pkt_size;
//			double tcp_out_pkt_rate = ram_ctrl_loadOnOutBridge / avg_pkt_size;
//			PACKET_MGR().passable_sampling_rate_stride = (size_t)MAX( input_pkt_rate / MAX( tcp_out_pkt_rate , 1/*all input go to hard*/) , 1/*stride not below 1*/);
//		}
//	}
//#endif

	switch ( PACKET_MGR().sampling_threshold_stage )
	{
		case prss_skip_input_by_hard_fulled:
		{
			PACKET_MGR().sampling_sent_packet_stride = PACKET_MGR().TTF_sampling_sent_packet_stride;
			break;
		}
		default:
		{
			PACKET_MGR().sampling_sent_packet_stride = MAX( PACKET_MGR().TTF_sampling_sent_packet_stride , 1 );
			break;
		}
	}
}
#endif


void cleanup_pkt_mgr( pkt_mgr_t * pktmgr )
{
	//dict_fst_destroy( &pktmgr->map_tcp_socket ); it cause shutdown haulted
	segmgr_destroy( &pktmgr->harbor_memory );
	pthread_mutex_destroy( &pktmgr->pm_lock );
	cr_in_wnd_free( &pktmgr->ram_ctrl_loadOnInput );
	cr_in_wnd_free( &pktmgr->ram_ctrl_loadOnStorage );
	cr_in_wnd_free( &pktmgr->ram_ctrl_loadOnOutBridge );
	cr_in_wnd_free( &pktmgr->ram_ctrl_instantaneousInputLoad );

	//inst_rate_destroy( &pktmgr->ram_ctrl_instantaneousInputLoad );
}
