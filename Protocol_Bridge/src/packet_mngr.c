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

	time_t tbegin = time( NULL );
	CIRCUIT_BREAKER long break_cuit = 0;time_t tnow = tbegin;
	while ( !ci_sgm_is_empty( &_g->hdls.pkt_mgr.huge_fst_cache ) && ( ( tnow - tbegin ) < 60 ) ) TODO 
	{
		tnow = time( NULL );
	}

#ifdef ENABLE_USE_DBG_TAG
	MARK_LINE();
#endif
}

_CALLBACK_FXN _PRIVATE_FXN void pre_config_init_packet_mngr( void_p src_g )
{
	INIT_BREAKABLE_FXN();
	G * _g = ( G * )src_g;

#ifdef ENABLE_HALFFILL_SEGMENT
	M_BREAK_STAT( distributor_init( &_g->hdls.pkt_mgr.bcast_release_halffill_segment , 1 ) , 0 );
#endif
	M_BREAK_STAT( dict_fst_create( &_g->hdls.pkt_mgr.map_tcp_socket , 512 ) , 0 );
#ifdef ENABLE_HALFFILL_SEGMENT
	M_BREAK_STAT( distributor_subscribe( &_g->hdls.pkt_mgr.bcast_release_halffill_segment , SUB_LONG , SUB_FXN( release_halffill_segment ) , _g ) , 0 ); // each clock try to close open segemtn
#endif

	// register here to get quit cmd
	M_BREAK_STAT( distributor_subscribe_withOrder( &_g->distributors.bcast_quit , SUB_LONG , SUB_FXN( cleanup_packet_mngr ) , _g , clean_packet_mngr ) , 0 );
	M_BREAK_STAT( distributor_subscribe_withOrder( &_g->distributors.bcast_quit , SUB_LONG , SUB_FXN( waiting_until_no_more_unsaved_packet ) , _g , wait_until_no_more_unsaved_packet ) , 0 );
	
#ifdef ENABLE_HALFFILL_SEGMENT
	M_BREAK_STAT( distributor_subscribe_withOrder( &_g->distributors.bcast_quit , SUB_LONG , SUB_FXN( release_halffill_segment ) , _g , getting_new_udp_stoped ) , 0 ); // there is no more udp so close segment
#endif

	M_BREAK_STAT( cbuf_m_init( &_g->hdls.pkt_mgr.last_30_sec_seg_count , 30 ) , 0 );

	BEGIN_SMPL
	M_V_END_RET
}

_CALLBACK_FXN _PRIVATE_FXN void post_config_init_packet_mngr( void_p src_g )
{
	INIT_BREAKABLE_FXN();
	G * _g = ( G * )src_g;

	M_BREAK_STAT( segmgr_init( &_g->hdls.pkt_mgr.huge_fst_cache , ( size_t )CFG().pkt_mgr_segment_capacity , ( size_t )CFG().pkt_mgr_offsets_capacity , True ) , 0 );
	
#ifdef HAS_STATISTICSS
	M_BREAK_STAT( distributor_subscribe_withOrder( &_g->distributors.init_static_table , SUB_VOID , SUB_FXN( init_packetmgr_statistics ) , _g , packetmgr_overview ) , 0 );
#endif
	
	MM_BREAK_IF( pthread_mutex_init( &_g->hdls.pkt_mgr.pm_lock , NULL ) , errCreation , 0 , "mutex_init()" );

	M_BREAK_STAT( cwnd_rate_init( &_g->hdls.pkt_mgr.longTermInputLoad , 10 /*sec*/ TODO ) , 0);
	M_BREAK_STAT( cwnd_rate_init( &_g->hdls.pkt_mgr.longTermTcpOutLoad , 10 /*sec*/ TODO ) , 0);
	M_BREAK_STAT( cwnd_rate_init( &_g->hdls.pkt_mgr.longTermFaultyOutLoad , 10 /*sec*/ TODO ) , 0);

	inst_rate_init( &_g->hdls.pkt_mgr.instantaneousInputLoad ); // 1 sec . i use this just to handle peak load

#ifdef ENABLE_FILLED_TCP_SEGMENT_PROC
	MM_BREAK_IF( pthread_create( &_g->hdls.pkt_mgr.trd_tcp_sender , NULL , process_filled_tcp_segment_proc , ( pass_p )_g ) != PTHREAD_CREATE_OK , errCreation , 0 , "Failed to create tcp_sender thread" );
#endif

	//segmgr_init( &_g->hdls.pkt_mgr.sent_package_log , 3200000 , 100000 , True );
	M_BREAK_STAT( distributor_subscribe( &_g->hdls.prst_csh.bcast_pagestacked_pkts , SUB_DIRECT_ONE_CALL_BUFFER_SIZE , SUB_FXN( discharge_persistent_storage_data ) , _g ) , 0 );
	
#ifdef ENABLE_CLEAN_UNUSED_SEGMENT
	MM_BREAK_IF( pthread_create( &_g->hdls.pkt_mgr.trd_clean_unused_segment , NULL , cleanup_unused_segment_proc , ( pass_p )_g ) != PTHREAD_CREATE_OK , errCreation , 0 , "Failed to create tcp_sender thread" );
#endif

#ifdef HAS_STATISTICSS
	M_BREAK_STAT( distributor_subscribe( &_g->distributors.throttling_refresh_stat , SUB_VOID , SUB_FXN( sampling_filled_segment_count ) , _g ) , 0 );
#endif

	BEGIN_SMPL
	M_V_END_RET
}

PRE_MAIN_INITIALIZATION( PRE_MAIN_INIT_PACKET_MNGR )
_PRIVATE_FXN void pre_main_init_packet_mngr_component( void )
{
	INIT_BREAKABLE_FXN();

	M_BREAK_STAT( distributor_subscribe( &_g->distributors.bcast_pre_cfg , SUB_VOID , SUB_FXN( pre_config_init_packet_mngr ) , _g ) , 0 );
	M_BREAK_STAT( distributor_subscribe_withOrder( &_g->distributors.bcast_post_cfg , SUB_VOID , SUB_FXN( post_config_init_packet_mngr ) , _g , post_config_order_packet_mngr ) , 0 );

	BEGIN_SMPL
	M_V_END_RET
}

#ifndef statistics

#ifdef ENABLE_USE_DBG_TAG
	_GLOBAL_VAR long long _mem_to_tcp = 0;
	_GLOBAL_VAR long long _mem_to_tcp_failure = 0;
	_GLOBAL_VAR int _regretion = 0;
	_GLOBAL_VAR long long _sucFromFile = 0;
	_GLOBAL_VAR long long _defraged_udp = 0;
	_GLOBAL_VAR long long _defraged_udp_sz = 0;
	_GLOBAL_VAR _EXTERN int _sem_in_fast_cache;
	_GLOBAL_VAR long long _open_gate_cnt;
	_GLOBAL_VAR long long _close_gate_cnt;
	_GLOBAL_VAR long long _half_segment_send_directly = 0;
	_GLOBAL_VAR long long _whole_segment_send_directly = 0;

	_GLOBAL_VAR long long _send_by_fast_dic = 0;
	_GLOBAL_VAR long long _send_by_slow_seek = 0;

	_GLOBAL_VAR long long _try_resolve_route_counter = 0;


	_GLOBAL_VAR _EXTERN int64 _seg_ttl_sz;
	_GLOBAL_VAR _EXTERN double _remain2use;
	_GLOBAL_VAR _EXTERN double _longTermInputLoad;
	_GLOBAL_VAR _EXTERN double _longTermTcpOutLoad;
	_GLOBAL_VAR _EXTERN double _longTermFaultyOutLoad;
	_GLOBAL_VAR _EXTERN double _instantaneousInputLoad;
	_GLOBAL_VAR _EXTERN double _rate_totally;
	_GLOBAL_VAR _EXTERN double _TTF;
	_GLOBAL_VAR _EXTERN int _fill_condition_stride;
	_GLOBAL_VAR _EXTERN int _which_cond;

#endif

#ifdef HAS_STATISTICSS

_CALLBACK_FXN PASSED_CSTR auto_refresh_segment_total_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	ci_sgmgr_t * huge_fst_cache = ( ci_sgmgr_t * )pcell->storage.bt.pass_data;
	sprintf( pcell->storage.tmpbuf , "A:%zu F:%zu + %zu - %zu" , huge_fst_cache->segment_total , huge_fst_cache->filled_count , huge_fst_cache->newed_segments , huge_fst_cache->released_segments );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

#ifdef ENABLE_USE_DBG_TAG
_CALLBACK_FXN PASSED_CSTR auto_refresh_segment_send_partially_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	ci_sgmgr_t * huge_fst_cache = ( ci_sgmgr_t * )pcell->storage.bt.pass_data;
	sprintf( pcell->storage.tmpbuf , "w:%zu h:%zu" , _whole_segment_send_directly , _half_segment_send_directly );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}
#endif

_CALLBACK_FXN PASSED_CSTR auto_refresh_m2_cur_items_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	ci_sgmgr_t * huge_fst_cache = ( ci_sgmgr_t * )pcell->storage.bt.pass_data;
	sprintf( pcell->storage.tmpbuf , "%zu" , huge_fst_cache->current_items );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR auto_refresh_m2_cur_bytes_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	ci_sgmgr_t * huge_fst_cache = ( ci_sgmgr_t * )pcell->storage.bt.pass_data;
	sprintf( pcell->storage.tmpbuf , "%zu" , huge_fst_cache->current_bytes );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR auto_refresh_m2_tt_items_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	ci_sgmgr_t * huge_fst_cache = ( ci_sgmgr_t * )pcell->storage.bt.pass_data;
	sprintf( pcell->storage.tmpbuf , "%zu" , huge_fst_cache->tt_items );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}
_CALLBACK_FXN PASSED_CSTR auto_refresh_m2_tt_bytes_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	ci_sgmgr_t * huge_fst_cache = ( ci_sgmgr_t * )pcell->storage.bt.pass_data;
	sprintf( pcell->storage.tmpbuf , "%zu" , huge_fst_cache->tt_bytes );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

#ifdef ENABLE_USE_DBG_TAG
_CALLBACK_FXN PASSED_CSTR auto_refresh_suc_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	//ci_sgmgr_t * huge_fst_cache = ( ci_sgmgr_t * )pcell->storage.bt.pass_data;
	sprintf( pcell->storage.tmpbuf , "+ %lld  ! %lld" , _mem_to_tcp , _mem_to_tcp_failure);
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR auto_refresh_regres_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	//ci_sgmgr_t * huge_fst_cache = ( ci_sgmgr_t * )pcell->storage.bt.pass_data;
	sprintf( pcell->storage.tmpbuf , "%d" , _regretion );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}
_CALLBACK_FXN PASSED_CSTR auto_refresh_memmap_items_sent_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	//G * _g = ( G * )pcell->storage.bt.pass_data;
	sprintf( pcell->storage.tmpbuf , "%lld" , _sucFromFile );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}
_CALLBACK_FXN PASSED_CSTR auto_refresh_defraged_udp_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	//ci_sgmgr_t * huge_fst_cache = ( ci_sgmgr_t * )pcell->storage.bt.pass_data;
	sprintf( pcell->storage.tmpbuf , "%lld" , _defraged_udp );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR auto_refresh_defraged_udp_sz_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	//ci_sgmgr_t * huge_fst_cache = ( ci_sgmgr_t * )pcell->storage.bt.pass_data;
	sprintf( pcell->storage.tmpbuf , "%lld" , _defraged_udp_sz );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}
_CALLBACK_FXN PASSED_CSTR auto_refresh_pkt_in_fst_cch_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	//ci_sgmgr_t * huge_fst_cache = ( ci_sgmgr_t * )pcell->storage.bt.pass_data;
	sprintf( pcell->storage.tmpbuf , "%d" , _sem_in_fast_cache );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

#endif

_CALLBACK_FXN PASSED_CSTR auto_refresh_memory_time_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	G * _g = ( G * )pcell->storage.bt.pass_data;
	if ( _g->hdls.pkt_mgr.latest_huge_memory_time.tv_sec )
	{
		struct tm * tm_info = localtime( &_g->hdls.pkt_mgr.latest_huge_memory_time.tv_sec );
		strftime( pcell->storage.tmpbuf , sizeof(pcell->storage.tmpbuf) , "%H:%M:%S" , tm_info );
	}
	else
	{
		pcell->storage.tmpbuf[0] = 0;
	}
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
		pcell->storage.tmpbuf[0] = 0;
	}
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

#ifdef ENABLE_USE_DBG_TAG
_CALLBACK_FXN PASSED_CSTR auto_refresh_send_kind_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	//G * _g = ( G * )pcell->storage.bt.pass_data;
	sprintf( pcell->storage.tmpbuf , "Fst:%lld Slw:%lld" , _send_by_fast_dic , _send_by_slow_seek );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}
#endif

#ifdef ENABLE_USE_DBG_TAG
_CALLBACK_FXN PASSED_CSTR auto_refresh_gateway_open_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	G * _g = ( G * )pcell->storage.bt.pass_data;
	sprintf( pcell->storage.tmpbuf , "%s + %lld - %lld" , _g->hdls.gateway.pagestack_gateway_open_val ? "o" : "c" , _open_gate_cnt , _close_gate_cnt);
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}
#endif

_CALLBACK_FXN PASSED_CSTR auto_refresh_files_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	G * _g = ( G * )pcell->storage.bt.pass_data;
	sprintf( pcell->storage.tmpbuf , "%zu" , _g->hdls.prst_csh.page_stack.files.count );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR auto_refresh_memmap_items_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	G * _g = ( G * )pcell->storage.bt.pass_data;
	sprintf( pcell->storage.tmpbuf , "%zu" , _g->hdls.prst_csh.page_stack.item_stored );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR auto_refresh_memmap_items_sz_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	G * _g = ( G * )pcell->storage.bt.pass_data;
	sprintf( pcell->storage.tmpbuf , "%zu" , _g->hdls.prst_csh.page_stack.item_stored_byte );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

#ifdef ENABLE_USE_DBG_TAG
_CALLBACK_FXN PASSED_CSTR auto_refresh_try_resolve_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	G * _g = ( G * )pcell->storage.bt.pass_data;
	sprintf( pcell->storage.tmpbuf , "try:%lld" , _try_resolve_route_counter );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}
#endif

_CALLBACK_FXN PASSED_CSTR auto_refresh_seg_ttl_sz_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	sprintf( pcell->storage.tmpbuf , "seg_ttl_sz:%llu" , _seg_ttl_sz );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}
_CALLBACK_FXN PASSED_CSTR auto_refresh_remain2use_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	sprintf( pcell->storage.tmpbuf , "remain2use:%.2f" , _remain2use );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}
_CALLBACK_FXN PASSED_CSTR auto_refresh_longTermInputLoad_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	sprintf( pcell->storage.tmpbuf , "longTermInputLoad:%.2f" , _longTermInputLoad );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}
_CALLBACK_FXN PASSED_CSTR auto_refresh_longTermTcpOutLoad_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	sprintf( pcell->storage.tmpbuf , "longTermTcpOutLoad:%.2f" , _longTermTcpOutLoad );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}
_CALLBACK_FXN PASSED_CSTR auto_refresh_longTermFaultyOutLoad_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	sprintf( pcell->storage.tmpbuf , "longTermFaultyOutLoad:%.2f" , _longTermFaultyOutLoad );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}
_CALLBACK_FXN PASSED_CSTR auto_refresh_instantaneousInputLoad_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	sprintf( pcell->storage.tmpbuf , "instantaneousInputLoad:%.2f" , _instantaneousInputLoad );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}
_CALLBACK_FXN PASSED_CSTR auto_refresh_rate_totally_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	sprintf( pcell->storage.tmpbuf , "rate_totally:%.2f" , _rate_totally );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}
_CALLBACK_FXN PASSED_CSTR auto_refresh_TTF_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	sprintf( pcell->storage.tmpbuf , "TTF:%.2f" , _TTF );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}
_CALLBACK_FXN PASSED_CSTR auto_refresh_fill_condition_stride_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	sprintf( pcell->storage.tmpbuf , "fill_condition_stride:%d" , _fill_condition_stride );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}
_CALLBACK_FXN PASSED_CSTR auto_refresh_which_cond_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	G * _g = ( G * )pcell->storage.bt.pass_data;
	sprintf( pcell->storage.tmpbuf , "which_cond:%d" , _which_cond );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

#define MEM_FAST "Mm1"
#define MEM_HUGE "Mm2"
#define MEM_FILE "Mm3"

_CALLBACK_FXN void init_packetmgr_statistics( pass_p src_g )
{
	INIT_BREAKABLE_FXN();
	G * _g = ( G * )src_g;

	nnc_lock_for_changes( &_g->stat.nc_h );

	nnc_table * ptbl = NULL;
	M_BREAK_STAT( nnc_add_table( &_g->stat.nc_h , "huge_fst_cache" , &ptbl ) , 0 );

	// col
	M_BREAK_STAT( nnc_add_column( ptbl , ""  , "" , 0 ) , 0 );
	M_BREAK_STAT( nnc_add_column( ptbl , "A" , "" , 0 ) , 0 );
	M_BREAK_STAT( nnc_add_column( ptbl , "B" , "" , 20 ) , 0 );
	M_BREAK_STAT( nnc_add_column( ptbl , "C" , "" , 0 ) , 0 );
	M_BREAK_STAT( nnc_add_column( ptbl , "D" , "" , 20 ) , 0 );
	M_BREAK_STAT( nnc_add_column( ptbl , "E" , "" , 0 ) , 0 );
	M_BREAK_STAT( nnc_add_column( ptbl , "F" , "" , 20 ) , 0 );

	int irow = -1;
	int icol = 0;
	nnc_cell_content * pcell = NULL;


	//--------------------
	irow++; icol = 0; M_BREAK_STAT( nnc_add_empty_row( ptbl , NULL ) , 0 );
	M_BREAK_STAT( nnc_set_static_int( ptbl , (size_t)irow , ( size_t )icol++ , irow + 1 ) , 0 );
	//--------------------
	// segment_total
	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , MEM_HUGE " segments" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = &_g->hdls.pkt_mgr.huge_fst_cache; pcell->conversion_fxn = auto_refresh_segment_total_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 );

#ifdef ENABLE_USE_DBG_TAG
	// segment_send_stat
	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , MEM_HUGE " partially" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = &_g->hdls.pkt_mgr.huge_fst_cache; pcell->conversion_fxn = auto_refresh_segment_send_partially_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 );
#endif
	
#ifdef ENABLE_USE_DBG_TAG
	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , MEM_FAST "    pkts" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = &_g->hdls.pkt_mgr.huge_fst_cache; pcell->conversion_fxn = auto_refresh_pkt_in_fst_cch_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 );
#endif


	//--------------------
	irow++; icol = 0; M_BREAK_STAT( nnc_add_empty_row( ptbl , NULL ) , 0 );
	M_BREAK_STAT( nnc_set_static_int( ptbl , (size_t)irow , ( size_t )icol++ , irow + 1 ) , 0 );
	//--------------------
#ifdef ENABLE_USE_DBG_TAG
	// _regretion
	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "slope" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = &_g->hdls.pkt_mgr.huge_fst_cache; pcell->conversion_fxn = auto_refresh_regres_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 );
#endif

#ifdef ENABLE_USE_DBG_TAG
	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , MEM_HUGE " defraged" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = &_g->hdls.pkt_mgr.huge_fst_cache; pcell->conversion_fxn = auto_refresh_defraged_udp_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 );
#endif

#ifdef ENABLE_USE_DBG_TAG
	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , MEM_HUGE " defraged B" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = &_g->hdls.pkt_mgr.huge_fst_cache; pcell->conversion_fxn = auto_refresh_defraged_udp_sz_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 );
#endif

	//--------------------
	irow++; icol = 0; M_BREAK_STAT( nnc_add_empty_row( ptbl , NULL ) , 0 );
	M_BREAK_STAT( nnc_set_static_int( ptbl , ( size_t )irow , ( size_t )icol++ , irow + 1 ) , 0 );
	//--------------------
	
#ifdef ENABLE_USE_DBG_TAG
	// gateway_open
	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "gateway" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = _g; pcell->conversion_fxn = auto_refresh_gateway_open_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 );
#endif

	// current_items
	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , MEM_HUGE "    pkts" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = &_g->hdls.pkt_mgr.huge_fst_cache; pcell->conversion_fxn = auto_refresh_m2_cur_items_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 );

	// current_bytes
	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , MEM_HUGE "     pkts B" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = &_g->hdls.pkt_mgr.huge_fst_cache; pcell->conversion_fxn = auto_refresh_m2_cur_bytes_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 );

	
	//--------------------
	irow++; icol = 0; M_BREAK_STAT( nnc_add_empty_row( ptbl , NULL ) , 0 );
	M_BREAK_STAT( nnc_set_static_int( ptbl , ( size_t )irow , ( size_t )icol++ , irow + 1 ) , 0 );
	//--------------------
#ifdef ENABLE_USE_DBG_TAG
	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , MEM_HUGE "->tcp" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = &_g->hdls.pkt_mgr.huge_fst_cache; pcell->conversion_fxn = auto_refresh_suc_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 );
#endif
	
	// tt_m2_items
	icol = 3;
	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , MEM_HUGE " tt pkts" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = &_g->hdls.pkt_mgr.huge_fst_cache; pcell->conversion_fxn = auto_refresh_m2_tt_items_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 );

	// tt_m2_bytes
	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , MEM_HUGE "  tt pkts B" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = &_g->hdls.pkt_mgr.huge_fst_cache; pcell->conversion_fxn = auto_refresh_m2_tt_bytes_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 );


	//--------------------
	irow++; icol = 0; M_BREAK_STAT( nnc_add_empty_row( ptbl , NULL ) , 0 );
	M_BREAK_STAT( nnc_set_static_int( ptbl , (size_t)irow , ( size_t )icol++ , irow + 1 ) , 0 );
	//--------------------
	// memory_time
	M_BREAK_STAT( nnc_set_static_text( ptbl , (size_t)irow , (size_t)icol++ , MEM_HUGE " time" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = _g; pcell->conversion_fxn = auto_refresh_memory_time_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , (size_t)irow , (size_t)icol++ , pcell ) , 0 );

	// memmap_items
	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , MEM_FILE "    pkts" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = _g; pcell->conversion_fxn = auto_refresh_memmap_items_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 );

	// memmap_items
	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , MEM_FILE "     pkts B" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = _g; pcell->conversion_fxn = auto_refresh_memmap_items_sz_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 );

	//--------------------
	irow++; icol = 0; M_BREAK_STAT( nnc_add_empty_row( ptbl , NULL ) , 0 );
	M_BREAK_STAT( nnc_set_static_int( ptbl , ( size_t )irow , ( size_t )icol++ , irow + 1 ) , 0 );
	//--------------------
	// memmap_time
	M_BREAK_STAT( nnc_set_static_text( ptbl , (size_t)irow , (size_t)icol++ , MEM_FILE " time" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = _g; pcell->conversion_fxn = auto_refresh_memmap_time_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , (size_t)irow , (size_t)icol++ , pcell ) , 0 );

#ifdef ENABLE_USE_DBG_TAG
	// sent memmap items
	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , MEM_FILE " snd T" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = _g; pcell->conversion_fxn = auto_refresh_send_kind_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 );
#endif

#ifdef ENABLE_USE_DBG_TAG
	// sent memmap items
	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , MEM_FILE " pkt_snt" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = _g; pcell->conversion_fxn = auto_refresh_memmap_items_sent_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 );
#endif


	//--------------------
	irow++; icol = 0; M_BREAK_STAT( nnc_add_empty_row( ptbl , NULL ) , 0 );
	M_BREAK_STAT( nnc_set_static_int( ptbl , (size_t)irow , ( size_t )icol++ , irow + 1 ) , 0 );
	//--------------------
	// files
	M_BREAK_STAT( nnc_set_static_text( ptbl , (size_t)irow , (size_t)icol++ , MEM_FILE " pages" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = _g; pcell->conversion_fxn = auto_refresh_files_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , (size_t)irow , (size_t)icol++ , pcell ) , 0 );


	//--------------------
	irow++; icol = 0; M_BREAK_STAT( nnc_add_empty_row( ptbl , NULL ) , 0 );
	M_BREAK_STAT( nnc_set_static_int( ptbl , ( size_t )irow , ( size_t )icol++ , irow + 1 ) , 0 );
	//--------------------
	#ifdef ENABLE_USE_DBG_TAG
	// files
	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , " resolve" ) , 0 ); pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = _g; pcell->conversion_fxn = auto_refresh_try_resolve_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 );
	#endif
	//_try_resolve_route_counter


	//--------------------
	irow++; icol = 0; M_BREAK_STAT( nnc_add_empty_row( ptbl , NULL ) , 0 );
	M_BREAK_STAT( nnc_set_static_int( ptbl , ( size_t )irow , ( size_t )icol++ , irow + 1 ) , 0 );
	//--------------------

	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->conversion_fxn = auto_refresh_seg_ttl_sz_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 );

	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->conversion_fxn = auto_refresh_remain2use_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 );

	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->conversion_fxn = auto_refresh_longTermInputLoad_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 );

	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->conversion_fxn = auto_refresh_longTermTcpOutLoad_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 );


	//--------------------
	irow++; icol = 0; M_BREAK_STAT( nnc_add_empty_row( ptbl , NULL ) , 0 );
	M_BREAK_STAT( nnc_set_static_int( ptbl , ( size_t )irow , ( size_t )icol++ , irow + 1 ) , 0 );
	//--------------------

	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->conversion_fxn = auto_refresh_longTermFaultyOutLoad_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 );

	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->conversion_fxn = auto_refresh_instantaneousInputLoad_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 );

	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->conversion_fxn = auto_refresh_rate_totally_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 );

	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->conversion_fxn = auto_refresh_TTF_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 );


	//--------------------
	irow++; icol = 0; M_BREAK_STAT( nnc_add_empty_row( ptbl , NULL ) , 0 );
	M_BREAK_STAT( nnc_set_static_int( ptbl , ( size_t )irow , ( size_t )icol++ , irow + 1 ) , 0 );
	//--------------------

	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->conversion_fxn = auto_refresh_fill_condition_stride_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 );

	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->conversion_fxn = auto_refresh_which_cond_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 );




	// --------------------

	M_BREAK_STAT( nnc_register_into_page_auto_refresh( ptbl , &_g->distributors.throttling_refresh_stat ) , 0 );

	BEGIN_SMPL
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
	status ret = segmgr_append( &_g->hdls.pkt_mgr.huge_fst_cache , buf , sz , &bNewSegment ); // store whole pakt + hdr into global buffer
	RANJE_ACT1( ret , errArg , NULL_ACT , MACRO_E( M_BREAK_STAT( ret , 0 ) ) );

	cwnd_rate_add_packet( &_g->hdls.pkt_mgr.longTermInputLoad , ( uint64_t )sz ); // calculate input rate
	inst_rate_add_packet( &_g->hdls.pkt_mgr.instantaneousInputLoad , ( uint64_t )sz );

	if ( bNewSegment )
	{
		MEMSET_ZERO_O( &_g->hdls.pkt_mgr.latest_huge_memory_time ); // try to send new packet cause time reset
	}

#ifdef ENABLE_USE_DBG_TAG
	if ( ret == errOK )
	{
		_defraged_udp++;
		_defraged_udp_sz += (long long)sz;
	}
#endif

	// TODO . add record to file if memory about to full
	
	BEGIN_SMPL
	M_END_RET
}

// 1 . try to send packet under tcp to destination
// TODO . multi thread entrance . be quite
_PRIVATE_FXN _CALLBACK_FXN status process_segment_itm( buffer data , size_t len , pass_p src_g )
{
	G * _g = ( G * )src_g;
	status err_sent = errCanceled;

	status ret_lock = errCanceled;
	LOCK_LINE( ( ret_lock = pthread_mutex_timedlock_rel( &_g->hdls.pkt_mgr.pm_lock , 1000 ) ) );
	if ( ret_lock == errTimeout )
	{
		return ret_lock;
	}
	
	xudp_hdr * pkt1 = ( xudp_hdr * )data;
	size_t sz_t = pkt1->metadata.payload_sz;
	WARNING( pkt1->metadata.version == TCP_XPKT_V1 );
	bool try_resolve_route = false;

	if ( pkt1->metadata.sent ) { err_sent = errOK; goto _exit_pt; }
	if ( _g->cmd.block_sending_1 ) { err_sent = errCanceled; goto _exit_pt; }

	time_t tnow = 0;
	tnow = time( NULL );
	sockfd fd = invalid_fd;
	void_p ab_tcp_p = NULL;
	AB * pb = NULL;

	// normal packet come and go. retried packet should just checked
	if ( pkt1->metadata.fault_registered ) // faulty item should not have too many attempt
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
	{
		if ( ab_tcp_p )
		{
			AB_tcp * ptcp = ( AB_tcp * )ab_tcp_p;
		}

		IMMORTAL_LPCSTR errString = NULL;
		uchar buf[MIN_SYSERR_BUF_SZ] = {0};
		err_sent = tcp_send_all( fd , data + pkt1->metadata.payload_offset , sz_t , 0 , SEND_TIMEOUT_ms , ACK_TIMEOUT_ms , RETRY_MECHANISM , &errString , ( buffer * )&buf ); // send is too heavy
		if ( errString )
		{
			struct in_addr addr = {0};
			addr.s_addr = pkt1->metadata.udp_hdr.dstIP;  // must already be in network byte order
			char str[ INET_ADDRSTRLEN ] = {0};
			inet_ntop( AF_INET , &addr , str , INET_ADDRSTRLEN );

		#ifdef ENABLE_LOGGING
			log_write( LOG_ERROR , "%d %s %s %s %s" , __LINE__ , errString , buf , pkt1->TCP_name , str );
		#endif
		}
		switch ( err_sent )
		{
			case errOK:
			{
			#ifdef ENABLE_USE_DBG_TAG
				_send_by_fast_dic++;
			#endif
				pkt1->metadata.sent = true;
				if ( ab_tcp_p )
				{
					AB_tcp * ptcp = ( AB_tcp * )ab_tcp_p;
					ptcp->this->last_access = time( NULL );
					pb = ptcp->owner_pb; // not safe at all . but for now is ok . TODO . fix this error prone potentially faulty part
				}

				if ( !pkt1->metadata.fault_registered )
				{
					cwnd_rate_add_packet( &_g->hdls.pkt_mgr.longTermTcpOutLoad , ( uint64_t )len );
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
					pb = ptcp->owner_pb; // not safe at all . but for now is ok . TODO . fix this error prone potentially faulty part
				}

				if ( !errString ) /*because before it logged*/
				{
				#ifdef ENABLE_LOGGING
					log_write( LOG_ERROR , "%d err:%s" , __LINE__ , internalErrorStr( err_sent ) );
				#endif
				}
				break;
			}
			default:
			{
				if ( !errString ) /*because before it logged*/
				{
				#ifdef ENABLE_LOGGING
					log_write( LOG_ERROR , "%d err:%s" , __LINE__ , internalErrorStr( err_sent ) );
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
	
	// TODO . take bounce between checking real connection
	if ( !pkt1->metadata.sent && !ab_tcp_p && _g->bridges.connected_tcp_out ) // slow method
	{
		pthread_mutex_lock( &_g->bridges.tcps_trd.mtx );
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
							if ( errString )
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
									_send_by_slow_seek++;
								#endif
									pkt1->metadata.sent = true;
									pb->tcps[ itcp ].last_access = time( NULL );
									if ( try_resolve_route )
									{
										try_resolve_route = false;
										dict_fst_put( &_g->hdls.pkt_mgr.map_tcp_socket , pkt1->TCP_name , pb->tcps[ itcp ].this->tcp_sockfd , ( void_p )pb->tcps[ itcp ].this , NULL , NULL , NULL );
									}

									if ( !pkt1->metadata.fault_registered ) // faulti item come from file so why should i conder them in calculation
									{
										cwnd_rate_add_packet( &_g->hdls.pkt_mgr.longTermTcpOutLoad , ( uint64_t )len );
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
										log_write( LOG_ERROR , "%d err:%s" , __LINE__ , internalErrorStr( err_sent ) );
									#endif
									}
									break;
								}
								default:
								{
									if ( !errString ) /*because before it logged*/
									{
									#ifdef ENABLE_LOGGING
										log_write( LOG_ERROR , "%d err:%s" , __LINE__ , internalErrorStr( err_sent ) );
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
	
	if ( !pkt1->metadata.sent && pkt1->metadata.retry )
	{
		pkt1->metadata.retry = false;
		pkt1->metadata.retried = true;
		err_sent = process_segment_itm( data , len , src_g ); // retry
		goto _exit_pt;
	}

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
_PRIVATE_FXN _CALLBACK_FXN status process_faulty_itm( buffer data , size_t len , pass_p src_g )
{
	G * _g = ( G * )src_g;
	status d_error = errCanceled;
	xudp_hdr * pkt1 = ( xudp_hdr * )data;
	size_t sz_t = pkt1->metadata.payload_sz;

	if ( pkt1->metadata.sent || pkt1->metadata.fault_registered )
	{
		return errOK;
	}
	
	_g->hdls.pkt_mgr.latest_memmap_time = pkt1->metadata.udp_hdr.tm; // wheter memmap packet is most important so first send them

	pkt1->metadata.fault_registered = 1;

#ifdef ENABLE_USE_DBG_TAG
	_mem_to_tcp_failure++;
#endif

#ifdef ENABLE_PERSISTENT_CACHE
	// try to seperate requestor and actually archiver. so i drop it on ground and coursed stinky person grab it
	d_error = distributor_publish_buffer_size( &_g->hdls.prst_csh.bcast_store_data , data , len , src_g );
#endif
	
	if ( d_error == errOK )
	{
		cwnd_rate_add_packet( &_g->hdls.pkt_mgr.longTermFaultyOutLoad , ( uint64_t )len );
	}

	// write it down on disk
	// later we can find logged that does not sent and find the bug
	
	return d_error;
}

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
	status d_error = process_segment_itm( buf , sz , src_g );
#ifdef ENABLE_USE_DBG_TAG
	if ( d_error == errOK )
	{
		_sucFromFile++;
	}
#endif
	return d_error;
}

// emptied buffer cache( huge_fst_cache ) then on failure go to persistent storage cache and get from it
// this fxn do empty segment by segment
_THREAD_FXN void_p process_filled_tcp_segment_proc( pass_p src_g )
{
	G * _g = ( G * )src_g;
	
#ifdef ENABLE_LOG_THREADS
	__attribute__( ( cleanup( thread_goes_out_of_scope ) ) ) pthread_t this_thread = pthread_self();
	distributor_publish_x3long( &_g->distributors.bcast_thread_startup , (long)this_thread , trdn_process_filled_tcp_segment_proc , (long)__FUNCTION__ , _g );
	
	/*retrieve track alive indicator*/
	pthread_mutex_lock( &_g->stat.nc_s_req.thread_list_mtx );
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
			pseg = segmgr_pop_filled_segment( &_g->hdls.pkt_mgr.huge_fst_cache , False , seg_trv_LIFO );
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
				if ( _g->cmd.block_sending_1 || ci_sgm_iter_items( pseg , process_segment_itm , src_g , true , _g->hdls.pkt_mgr.strides_packet_peek , tail_2_head ) != errOK ) // some fault detected
				{
					all_poped_send_suc = false;
					// if sending filled segment fail try to archive them
					ci_sgm_iter_items( pseg , process_faulty_itm , src_g , true , 1 , head_2_tail );
				}
				// then close segment
				if ( ( d_error = ci_sgm_mark_empty( &_g->hdls.pkt_mgr.huge_fst_cache , pseg ) ) == errEmpty ) // pop last emptied segment
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
	pthread_mutex_lock( &_g->stat.nc_s_req.thread_list_mtx );
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

		if ( !segmgr_cleanup_idle( &_g->hdls.pkt_mgr.huge_fst_cache , 5 ) ) // if there is no work to do clean unused segment
		{
			mng_basic_thread_sleep( _g , LOW_PRIORITY_THREAD );
		}
		else
		{
			mng_basic_thread_sleep( _g , NORMAL_PRIORITY_THREAD );
		}
	} while ( 1 );

#ifdef ENABLE_USE_DBG_TAG
	MARK_LINE();
#endif

	return NULL;
}

#ifndef release_last_unused_segment
#ifdef ENABLE_HALFFILL_SEGMENT
// check last packet of segment then if packet idle for long time then close segment
_PRIVATE_FXN _CALLBACK_FXN bool peek_decide_active_sgm( const buffer buf , size_t sz )
{
	if ( _g->cmd.cleanup_state <= getting_new_udp_stoped ) return true;
	xudp_hdr * pkt1 = ( xudp_hdr * )buf;
	WARNING( pkt1->metadata.version == TCP_XPKT_V1 );

	struct timeval tnow;
	gettimeofday( &tnow , NULL );
	double df_sec = timeval_diff_ms( &pkt1->metadata.udp_hdr.tm , &tnow ) / 1000;
	return df_sec > CFG().pkt_mgr_keep_idle_segment_sec;
}

// check if condition is true then set halffill segemtn as fill
_CALLBACK_FXN void release_halffill_segment( pass_p src_g , long v )
{
	G * _g = ( G * )src_g;
	if ( ci_sgm_peek_decide_active( &_g->hdls.pkt_mgr.huge_fst_cache , peek_decide_active_sgm ) )
	{
		//MEMSET_ZERO_O( &_g->hdls.pkt_mgr.latest_huge_memory_time ); // try to send new packet cause time reset
	}
}
#endif
#endif

int64 _seg_ttl_sz = 0;
double _remain2use = 0;
double _longTermInputLoad = 0;
double _longTermTcpOutLoad = 0;
double _longTermFaultyOutLoad = 0;
double _instantaneousInputLoad = 0;
double _rate_totally = 0;
double _TTF = 0;
int _fill_condition_stride = 0;
int _which_cond = 0;

// every one second
_CALLBACK_FXN void sampling_filled_segment_count( pass_p src_g )
{
	G * _g = ( G * )src_g;
	
	int64 seg_ttl_sz = ( int64 )( _g->hdls.pkt_mgr.huge_fst_cache.segment_total * _g->hdls.pkt_mgr.huge_fst_cache.default_seg_capacity );
	double remain2use = ( double )( CFG().pkt_mgr_allocation_allowed - seg_ttl_sz );
	double longTermInputLoad = cwnd_rate_get_bps( &_g->hdls.pkt_mgr.longTermInputLoad );
	double longTermTcpOutLoad = cwnd_rate_get_bps( &_g->hdls.pkt_mgr.longTermTcpOutLoad );
	double longTermFaultyOutLoad = cwnd_rate_get_bps( &_g->hdls.pkt_mgr.longTermFaultyOutLoad );
	double instantaneousInputLoad = ( double )inst_rate_get_last( &_g->hdls.pkt_mgr.instantaneousInputLoad , 10 /*TODO*/ );
	double coef = CFG().pkt_mgr_instantaneous_coefficient;
	double input_rate = ( coef * instantaneousInputLoad ) + ( ( 1 - coef ) * longTermInputLoad ); // partial part from instantaneous and complement from long term
	double rate_totally = MAX( input_rate - longTermTcpOutLoad - longTermFaultyOutLoad , 0 );
	int fill_condition_stride = 1;

	_seg_ttl_sz = seg_ttl_sz;
	_remain2use = remain2use;
	_longTermInputLoad = longTermInputLoad;
	_longTermTcpOutLoad = longTermTcpOutLoad;
	_longTermFaultyOutLoad = longTermFaultyOutLoad;
	_instantaneousInputLoad = instantaneousInputLoad;
	_rate_totally = rate_totally;


	if ( rate_totally > 0 )
	{
		double TTF = remain2use / rate_totally; /*time to fill*/
		_TTF = TTF;
		if ( TTF > CFG().pkt_mgr_TTF_nopressure_sec )
		{
			fill_condition_stride = 1;
			_which_cond = 1;
		}
		else if ( TTF > CFG().pkt_mgr_TTF_gentle_backpressure_sec )
		{
			fill_condition_stride = CFG().pkt_mgr_gentle_backpressure_stride;
			_which_cond = 2;
		}
		else if ( TTF > CFG().pkt_mgr_TTF_aggressive_sec )
		{
			fill_condition_stride = CFG().pkt_mgr_aggressive_persist_stride;
			_which_cond = 3;
		}
		else if ( TTF > CFG().pkt_mgr_TTF_emergency_drop_sec )
		{
			fill_condition_stride = CFG().pkt_mgr_emergency_drop_stride;
			_which_cond = 4;
		}
		else
		{
			//double BW_needed = _g->hdls.pkt_mgr.huge_fst_cache.current_bytes / TTF;
			//fill_condition_stride = ( int )ceil( BW_needed / longTermTcpOutLoad ); /**/
			fill_condition_stride = CFG().pkt_mgr_red_zone_stride;
			_which_cond = 5;
		}
		_fill_condition_stride = fill_condition_stride;
	}
	
	cbuf_m_advance( &_g->hdls.pkt_mgr.last_30_sec_seg_count , _g->hdls.pkt_mgr.huge_fst_cache.segment_total );
	size_t stride_basedon_segment_slope = ( size_t )MAX( cbuf_m_regression_slope_all( &_g->hdls.pkt_mgr.last_30_sec_seg_count ) , 1 );

	_g->hdls.pkt_mgr.strides_packet_peek = MAX( fill_condition_stride , stride_basedon_segment_slope ); // worst case scenario

#ifdef ENABLE_USE_DBG_TAG
	_regretion = _g->hdls.pkt_mgr.strides_packet_peek;
#endif
}

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
	segmgr_destroy( &pktmgr->huge_fst_cache );
#ifdef ENABLE_USE_DBG_TAG
	DBG_PT();
#endif
	pthread_mutex_destroy( &pktmgr->pm_lock );

	cwnd_rate_free( &pktmgr->longTermInputLoad );
	cwnd_rate_free( &pktmgr->longTermTcpOutLoad );
	cwnd_rate_free( &pktmgr->longTermFaultyOutLoad );
}
