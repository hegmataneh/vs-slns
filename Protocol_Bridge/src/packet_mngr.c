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

GLOBAL_VAR extern G * _g;

_PRIVATE_FXN _CALLBACK_FXN bool always_say_is_filled_callback( const buffer buf , size_t sz )
{
	return true;
}

_PRIVATE_FXN _CALLBACK_FXN void cleanup_packet_mngr( pass_p src_g , long v )
{
	MARK_LINE();

	G * _g = ( G * )src_g;
	ci_sgm_peek_decide_active( &_g->hdls.pkt_mgr.huge_fst_cache , always_say_is_filled_callback );

	time_t tbegin = time( NULL );
	time_t tnow = tbegin;
	while ( !ci_sgm_is_empty( &_g->hdls.pkt_mgr.huge_fst_cache ) && ( ( tnow - tbegin ) < 60 ) )
	{
		tnow = time( NULL );
	}

	cleanup_pkt_mgr( &_g->hdls.pkt_mgr );

	MARK_LINE();
}

_CALLBACK_FXN _PRIVATE_FXN void pre_config_init_packet_mngr( void_p src_g )
{
	G * _g = ( G * )src_g;

	distributor_init( &_g->hdls.pkt_mgr.throttling_release_halffill_segment , 1 );
	dict_fst_create( &_g->hdls.pkt_mgr.map_tcp_socket , 0 );
	distributor_subscribe( &_g->hdls.pkt_mgr.throttling_release_halffill_segment , SUB_VOID , SUB_FXN( release_halffill_segment ) , _g );

	// register here to get quit cmd
	distributor_subscribe_withOrder( &_g->distributors.quit_interrupt_dist , SUB_LONG , SUB_FXN( cleanup_packet_mngr ) , _g , clean_packet_mngr );

	cbuf_m_init( &_g->hdls.pkt_mgr.last_60_sec_seg_count , 60 );
}

_CALLBACK_FXN _PRIVATE_FXN void post_config_init_packet_mngr( void_p src_g )
{
	INIT_BREAKABLE_FXN();
	G * _g = ( G * )src_g;

	segmgr_init( &_g->hdls.pkt_mgr.huge_fst_cache , ( size_t )_g->appcfg.g_cfg->c.c.pkt_mgr_segment_capacity , ( size_t )_g->appcfg.g_cfg->c.c.pkt_mgr_offsets_capacity , True );
	distributor_subscribe_withOrder( &_g->distributors.init_static_table , SUB_VOID , SUB_FXN( init_packetmgr_statistics ) , _g , packetmgr_statistics );
	
	pthread_mutex_init( &_g->hdls.pkt_mgr.pm_lock , NULL );

	MM_BREAK_IF( pthread_create( &_g->hdls.pkt_mgr.trd_tcp_sender , NULL , process_filled_tcp_segment_proc , ( pass_p )_g ) != PTHREAD_CREATE_OK , errCreation , 0 , "Failed to create tcp_sender thread" );
	//segmgr_init( &_g->hdls.pkt_mgr.sent_package_log , 3200000 , 100000 , True );
	M_BREAK_STAT( distributor_subscribe( &_g->hdls.prst_csh.pagestack_pakcets , SUB_DIRECT_ONE_CALL_BUFFER_SIZE , SUB_FXN( descharge_persistent_storage_data ) , _g ) , 0 );
	
	MM_BREAK_IF( pthread_create( &_g->hdls.pkt_mgr.trd_clean_unused_segment , NULL , cleanup_unused_segment_proc , ( pass_p )_g ) != PTHREAD_CREATE_OK , errCreation , 0 , "Failed to create tcp_sender thread" );

	M_BREAK_STAT( distributor_subscribe( &_g->distributors.throttling_refresh_stat , SUB_VOID , SUB_FXN( sampling_filled_segment_count ) , _g ) , 0 );

	BEGIN_SMPL
	M_V_END_RET
}

PRE_MAIN_INITIALIZATION( 105 )
_PRIVATE_FXN void pre_main_init_packet_mngr_component( void )
{
	distributor_subscribe( &_g->distributors.pre_configuration , SUB_VOID , SUB_FXN( pre_config_init_packet_mngr ) , _g );
	distributor_subscribe( &_g->distributors.post_config_stablished , SUB_VOID , SUB_FXN( post_config_init_packet_mngr ) , _g );
}

#ifndef statistics

_CALLBACK_FXN PASSED_CSTR auto_refresh_segment_total_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	ci_sgmgr_t * huge_fst_cache = ( ci_sgmgr_t * )pcell->storage.bt.pass_data;
	sprintf( pcell->storage.tmpbuf , "%zu" , huge_fst_cache->segment_total );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR auto_refresh_filled_count_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	ci_sgmgr_t * huge_fst_cache = ( ci_sgmgr_t * )pcell->storage.bt.pass_data;
	sprintf( pcell->storage.tmpbuf , "%zu" , huge_fst_cache->filled_count );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR auto_refresh_total_items_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	ci_sgmgr_t * huge_fst_cache = ( ci_sgmgr_t * )pcell->storage.bt.pass_data;
	sprintf( pcell->storage.tmpbuf , "%zu" , huge_fst_cache->total_items );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR auto_refresh_total_bytes_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	ci_sgmgr_t * huge_fst_cache = ( ci_sgmgr_t * )pcell->storage.bt.pass_data;
	sprintf( pcell->storage.tmpbuf , "%zu" , huge_fst_cache->total_bytes );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

long long _suc = 0;
long long _fail = 0;
int _filled = 0;

_CALLBACK_FXN PASSED_CSTR auto_refresh_suc_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	ci_sgmgr_t * huge_fst_cache = ( ci_sgmgr_t * )pcell->storage.bt.pass_data;
	sprintf( pcell->storage.tmpbuf , "%lld" , _suc );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}
_CALLBACK_FXN PASSED_CSTR auto_refresh_fail_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	ci_sgmgr_t * huge_fst_cache = ( ci_sgmgr_t * )pcell->storage.bt.pass_data;
	sprintf( pcell->storage.tmpbuf , "%lld" , _fail );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}
_CALLBACK_FXN PASSED_CSTR auto_refresh_filled_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	ci_sgmgr_t * huge_fst_cache = ( ci_sgmgr_t * )pcell->storage.bt.pass_data;
	sprintf( pcell->storage.tmpbuf , "%d" , _filled );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN PASSED_CSTR auto_refresh_grow_segment_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	ci_sgmgr_t * huge_fst_cache = ( ci_sgmgr_t * )pcell->storage.bt.pass_data;
	sprintf( pcell->storage.tmpbuf , "%zu" , huge_fst_cache->newed_segments );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}
_CALLBACK_FXN PASSED_CSTR auto_refresh_decrese_segment_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	ci_sgmgr_t * huge_fst_cache = ( ci_sgmgr_t * )pcell->storage.bt.pass_data;
	sprintf( pcell->storage.tmpbuf , "%zu" , huge_fst_cache->released_segments );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}


_CALLBACK_FXN PASSED_CSTR auto_refresh_memory_time_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	G * _g = ( G * )pcell->storage.bt.pass_data;
	struct tm * tm_info = localtime( &_g->hdls.pkt_mgr.latest_huge_memory_time.tv_sec );
	strftime( pcell->storage.tmpbuf , sizeof(pcell->storage.tmpbuf) , "%Y-%m-%d %H:%M:%S" , tm_info );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}
_CALLBACK_FXN PASSED_CSTR auto_refresh_memmap_time_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	G * _g = ( G * )pcell->storage.bt.pass_data;
	struct tm * tm_info = localtime( &_g->hdls.pkt_mgr.latest_memmap_time.tv_sec );
	strftime( pcell->storage.tmpbuf , sizeof( pcell->storage.tmpbuf ) , "%Y-%m-%d %H:%M:%S" , tm_info );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}


_CALLBACK_FXN PASSED_CSTR auto_refresh_gateway_open_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	G * _g = ( G * )pcell->storage.bt.pass_data;
	sprintf( pcell->storage.tmpbuf , "%d" , _g->hdls.gateway.pagestack_gateway_open_val );
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

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


_CALLBACK_FXN void init_packetmgr_statistics( pass_p src_g )
{
	INIT_BREAKABLE_FXN();
	G * _g = ( G * )src_g;

	nnc_table * ptbl = NULL;
	M_BREAK_STAT( nnc_add_table( &_g->stat.nc_h , "huge_fst_cache" , &ptbl ) , 0 );

	// col
	M_BREAK_STAT( nnc_add_column( ptbl , "" , "" , 0 ) , 0 );
	M_BREAK_STAT( nnc_add_column( ptbl , "" , "" , 20 ) , 0 );
	M_BREAK_STAT( nnc_add_column( ptbl , "" , "" , 0 ) , 0 );
	M_BREAK_STAT( nnc_add_column( ptbl , "" , "" , 20 ) , 0 );

	int irow = -1;
	int icol = 0;


	//------------------------------
	irow++; icol = 0;
	M_BREAK_STAT( nnc_add_empty_row( ptbl , NULL ) , 0 );
	
	// segment_total
	M_BREAK_STAT( nnc_set_static_text( ptbl , irow , icol++ , "segment_total" ) , 0 );
	// 
	nnc_cell_content * pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = &_g->hdls.pkt_mgr.huge_fst_cache;
	pcell->conversion_fxn = auto_refresh_segment_total_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , irow , icol++ , pcell ) , 0 );

	// filled_count
	M_BREAK_STAT( nnc_set_static_text( ptbl , irow , icol++ , "filled_count" ) , 0 );
	// 
	pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = &_g->hdls.pkt_mgr.huge_fst_cache;
	pcell->conversion_fxn = auto_refresh_filled_count_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , irow , icol++ , pcell ) , 0 );

	//------------------------------
	irow++; icol = 0;
	M_BREAK_STAT( nnc_add_empty_row( ptbl , NULL ) , 0 );
	
	// total_items
	M_BREAK_STAT( nnc_set_static_text( ptbl , irow , icol++ , "total_items" ) , 0 );
	// 
	pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = &_g->hdls.pkt_mgr.huge_fst_cache;
	pcell->conversion_fxn = auto_refresh_total_items_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , irow , icol++ , pcell ) , 0 );

	// total_bytes
	M_BREAK_STAT( nnc_set_static_text( ptbl , irow , icol++ , "total_bytes" ) , 0 );
	// 
	pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = &_g->hdls.pkt_mgr.huge_fst_cache;
	pcell->conversion_fxn = auto_refresh_total_bytes_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , irow , icol++ , pcell ) , 0 );

	//------------------------------
	irow++; icol = 0;
	M_BREAK_STAT( nnc_add_empty_row( ptbl , NULL ) , 0 );

	// _suc
	M_BREAK_STAT( nnc_set_static_text( ptbl , irow , icol++ , "_suc" ) , 0 );
	// 
	pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = &_g->hdls.pkt_mgr.huge_fst_cache;
	pcell->conversion_fxn = auto_refresh_suc_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , irow , icol++ , pcell ) , 0 );


	// _fail
	M_BREAK_STAT( nnc_set_static_text( ptbl , irow , icol++ , "_fail" ) , 0 );
	// 
	pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = &_g->hdls.pkt_mgr.huge_fst_cache;
	pcell->conversion_fxn = auto_refresh_fail_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , irow , icol++ , pcell ) , 0 );

	//------------------------------
	irow++; icol = 0;
	M_BREAK_STAT( nnc_add_empty_row( ptbl , NULL ) , 0 );

	// _filled
	M_BREAK_STAT( nnc_set_static_text( ptbl , irow , icol++ , "_filled" ) , 0 );
	// 
	pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = &_g->hdls.pkt_mgr.huge_fst_cache;
	pcell->conversion_fxn = auto_refresh_filled_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , irow , icol++ , pcell ) , 0 );


	// gateway_open
	M_BREAK_STAT( nnc_set_static_text( ptbl , irow , icol++ , "gateway_open" ) , 0 );
	// 
	pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = _g;
	pcell->conversion_fxn = auto_refresh_gateway_open_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , irow , icol++ , pcell ) , 0 );

	//------------------------------
	irow++; icol = 0;
	M_BREAK_STAT( nnc_add_empty_row( ptbl , NULL ) , 0 );

	// grow_segment
	M_BREAK_STAT( nnc_set_static_text( ptbl , irow , icol++ , "grow_segment" ) , 0 );
	// 
	pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = &_g->hdls.pkt_mgr.huge_fst_cache;
	pcell->conversion_fxn = auto_refresh_grow_segment_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , irow , icol++ , pcell ) , 0 );
	
	// decrese_segment
	M_BREAK_STAT( nnc_set_static_text( ptbl , irow , icol++ , "decrese_segment" ) , 0 );
	// 
	pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = &_g->hdls.pkt_mgr.huge_fst_cache;
	pcell->conversion_fxn = auto_refresh_decrese_segment_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , irow , icol++ , pcell ) , 0 );


	//------------------------------
	irow++; icol = 0;
	M_BREAK_STAT( nnc_add_empty_row( ptbl , NULL ) , 0 );

	// memory_time
	M_BREAK_STAT( nnc_set_static_text( ptbl , irow , icol++ , "memory_time" ) , 0 );
	// 
	pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = _g;
	pcell->conversion_fxn = auto_refresh_memory_time_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , irow , icol++ , pcell ) , 0 );

	// memmap_time
	M_BREAK_STAT( nnc_set_static_text( ptbl , irow , icol++ , "memmap_time" ) , 0 );
	// 
	pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = _g;
	pcell->conversion_fxn = auto_refresh_memmap_time_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , irow , icol++ , pcell ) , 0 );

	//------------------------------
	irow++; icol = 0;
	M_BREAK_STAT( nnc_add_empty_row( ptbl , NULL ) , 0 );


	// files
	M_BREAK_STAT( nnc_set_static_text( ptbl , irow , icol++ , "files" ) , 0 );
	// 
	pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = _g;
	pcell->conversion_fxn = auto_refresh_files_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , irow , icol++ , pcell ) , 0 );


	//------------------------------
	irow++; icol = 0;
	M_BREAK_STAT( nnc_add_empty_row( ptbl , NULL ) , 0 );

	// memmap_items
	M_BREAK_STAT( nnc_set_static_text( ptbl , irow , icol++ , "memmap_items" ) , 0 );
	// 
	pcell = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
	pcell->storage.bt.pass_data = _g;
	pcell->conversion_fxn = auto_refresh_memmap_items_cell;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , irow , icol++ , pcell ) , 0 );





	M_BREAK_STAT( nnc_register_into_page_auto_refresh( ptbl , &_g->distributors.throttling_refresh_stat ) , 0 );

	BEGIN_SMPL
	M_V_END_RET
}

#endif

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
	if ( bNewSegment )
	{
		MEMSET_ZERO_O( &_g->hdls.pkt_mgr.latest_huge_memory_time ); // try to send new packet cause time reset
	}

	// TODO . add record to file if memory about to full
	
	BEGIN_SMPL
	M_END_RET
}

// 1 . try to send packet under tcp to destination
// TODO . multi thread entrance . be quite
_PRIVATE_FXN _CALLBACK_FXN status process_segment_itm( buffer data , size_t len , pass_p src_g )
{
	G * _g = ( G * )src_g;
	
	pthread_mutex_lock( &_g->hdls.pkt_mgr.pm_lock );

	status d_error = errCanceled;
	rdy_pkt1 * pkt1 = ( rdy_pkt1 * )data;
	size_t sz_t = len - pkt1->metadata.payload_offset;
	WARNING( pkt1->metadata.version == TCP_PACKET_V1 );
	bool try_resolve_route = false;

	if ( pkt1->metadata.sent ) { d_error = errOK; goto _exit_pt; }
	if ( _g->cmd.block_sending_1 ) { d_error = errCanceled; goto _exit_pt; }

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
			{ d_error = errTooManyAttempt; goto _exit_pt; } // cannot send and too many attempt to send
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
		d_error = tcp_send_all( fd , data + pkt1->metadata.payload_offset , sz_t , 0 , 0 ); // send is too heavy
		switch ( d_error )
		{
			case errOK:
			{

				_suc++;

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
	// TODO . take bounce between checking real connection
	if ( !pkt1->metadata.sent && !ab_tcp_p ) // slow method
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
		d_error = process_segment_itm( data , len , src_g ); // retry
		goto _exit_pt;
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
		//if ( !pkt1->metadata.udp_hdr.logged_2_mem )
		//{
		//	if ( segmgr_append( &_g->hdls.pkt_mgr.sent_package_log , &pkt1->metadata.udp_hdr , sizeof( pkt1->metadata.udp_hdr ) ) == errOK )
		//	{
		//		pkt1->metadata.udp_hdr.logged_2_mem = true;
		//	}
		//}
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

	//d_error = errOK;
_exit_pt:
	pthread_mutex_unlock( &_g->hdls.pkt_mgr.pm_lock );
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
	
	_g->hdls.pkt_mgr.latest_memmap_time = pkt1->metadata.udp_hdr.tm; // wheter memmap packet is most important so first send them

	pkt1->metadata.fault_registered = 1;

	_fail++;

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
	//if ( buf == NULL && !sz ) // when no data in memmap then time reset to zero and actual send data if wait then release and do his work
	//{
	//	if ( _g->hdls.pkt_mgr.latest_memmap_time.tv_sec )
	//	{
	//		MEMSET_ZERO_O( &_g->hdls.pkt_mgr.latest_memmap_time ); // store failed packet reset value
	//	}
	//	return errOK;
	//}
	return process_segment_itm( buf , sz , src_g );
}

// emptied buffer cache( huge_fst_cache ) then on failure go to persistent storage cache and get from it
// this fxn do empty segment by segment
_THREAD_FXN void_p process_filled_tcp_segment_proc( pass_p src_g )
{
	G * _g = ( G * )src_g;
	distributor_publish_long( &_g->distributors.thread_startup , pthread_self() , _g );
	__attribute__( ( cleanup( thread_goes_out_of_scope ) ) ) pthread_t trd_id = pthread_self();
	MARK_START_THREAD();

	ci_sgm_t * pseg = NULL;
	
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
		if ( GRACEFULLY_END_THREAD() ) break;

		if
		(
			!_g->hdls.pkt_mgr.latest_huge_memory_time.tv_sec ||
			!_g->hdls.pkt_mgr.latest_memmap_time.tv_sec ||
			timeval_compare( &_g->hdls.pkt_mgr.latest_huge_memory_time , &_g->hdls.pkt_mgr.latest_memmap_time ) >= 0
		)
		{
			pseg = segmgr_pop_filled_segment( &_g->hdls.pkt_mgr.huge_fst_cache , False , seg_trv_LIFO );
			if ( pseg ) // poped on memory packets
			{
				cpu_unburne.idx = ( cpu_unburne.idx + 1 ) % sizeof( cpu_unburne.arr );
				cpu_unburne.arr.bytes[ cpu_unburne.idx ] = 1;
				_g->hdls.gateway.pagestack_gateway_open_val = gws_close;

				// try to send from mem under tcp to dst
				if ( ci_sgm_iter_items( pseg , process_segment_itm , src_g , true , _g->hdls.pkt_mgr.strides_packet_peek , tail_2_head ) != errOK ) // some fault detected
				{
					// if sending filled segment fail try to archive them
					ci_sgm_iter_items( pseg , process_faulty_itm , src_g , true , 1 , head_2_tail );
				}
				// then close segment
				ci_sgm_mark_empty( &_g->hdls.pkt_mgr.huge_fst_cache , pseg ); // pop last emptied segment
			}
			else // there is no packet in memory so fetch persisted packets
			{
				cpu_unburne.idx = ( cpu_unburne.idx + 1 ) % sizeof( cpu_unburne.arr );
				cpu_unburne.arr.bytes[ cpu_unburne.idx ] = 0;
				
				gateway_open_stat tmp_old = _g->hdls.gateway.pagestack_gateway_open_val;
				if ( !tmp_old )
				{
					_g->hdls.gateway.pagestack_gateway_open_val = gws_open;
					sem_post( &_g->hdls.gateway.pagestack_gateway_open_sem );
				}
			}
		}
		else
		{
			cpu_unburne.idx = ( cpu_unburne.idx + 1 ) % sizeof( cpu_unburne.arr );
			cpu_unburne.arr.bytes[ cpu_unburne.idx ] = 0;

			//if ( cpu_unburne.arr.big_data.a || cpu_unburne.arr.big_data.b )
			//{
			//	MEMSET_ZERO_O( &cpu_unburne );
			//}
			gateway_open_stat tmp_old = _g->hdls.gateway.pagestack_gateway_open_val;
			if ( !tmp_old )
			{
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
	_g->hdls.gateway.pagestack_gateway_open_val = gws_close;

	MARK_LINE();

	return NULL;
}

_THREAD_FXN void_p cleanup_unused_segment_proc( pass_p src_g )
{
	G * _g = ( G * )src_g;
	distributor_publish_long( &_g->distributors.thread_startup , pthread_self() , _g );
	__attribute__( ( cleanup( thread_goes_out_of_scope ) ) ) pthread_t trd_id = pthread_self();
	MARK_START_THREAD();

	do
	{
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

	MARK_LINE();

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

// check if condition is true then set halffill segemtn as fill
_CALLBACK_FXN void release_halffill_segment( pass_p src_g )
{
	G * _g = ( G * )src_g;
	if ( ci_sgm_peek_decide_active( &_g->hdls.pkt_mgr.huge_fst_cache , peek_decide_active_sgm ) )
	{
		//MEMSET_ZERO_O( &_g->hdls.pkt_mgr.latest_huge_memory_time ); // try to send new packet cause time reset
	}
}

_CALLBACK_FXN void sampling_filled_segment_count( pass_p src_g )
{
	G * _g = ( G * )src_g;
	cbuf_m_advance( &_g->hdls.pkt_mgr.last_60_sec_seg_count , _g->hdls.pkt_mgr.huge_fst_cache.filled_count );
	_filled = MAX( cbuf_m_regression_slope_all( &_g->hdls.pkt_mgr.last_60_sec_seg_count ) , 1 );
	_g->hdls.pkt_mgr.strides_packet_peek = ( size_t )_filled;
}

void cleanup_pkt_mgr( pkt_mgr_t * pktmgr )
{
	sub_destroy( &pktmgr->throttling_release_halffill_segment );
	dict_fst_destroy( &pktmgr->map_tcp_socket );
	segmgr_destroy( &pktmgr->huge_fst_cache );
}
