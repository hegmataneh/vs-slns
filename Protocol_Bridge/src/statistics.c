#define Uses_LOCK_LINE
#define Uses_INIT_BREAKABLE_FXN
#define Uses_STRLEN
#define Uses_sleep
#define Uses_setlocale
#define Uses_memset
#define Uses_TWD
#define Uses_pthread_t
#define Uses_ncurses
#define Uses_statistics
#define Uses_globals
#include <Protocol_Bridge.dep>

_GLOBAL_VAR _EXTERN G * _g;

_CALLBACK_FXN void cleanup_stat( pass_p src_g , long v )
{
	G * _g = ( G * )src_g;

#ifdef HAS_STATISTICSS
	nnc_destroy( &_g->stat.nc_h );

	mms_array_free( &_g->stat.nc_s_req.field_keeper );

	pthread_mutex_destroy( &_g->stat.nc_s_req.thread_list_mtx );
	mms_array_free( &_g->stat.nc_s_req.thread_list );
#endif

}

_CALLBACK_FXN _PRIVATE_FXN void state_pre_config_init_statistics( void_p src_g )
{
	INIT_BREAKABLE_FXN();
	G * _g = ( G * )src_g;

	if ( _g->stat.aggregate_stat.t_begin.tv_sec == 0 && _g->stat.aggregate_stat.t_begin.tv_usec == 0 )
	{
		gettimeofday( &_g->stat.aggregate_stat.t_begin , NULL );
	}

	//pthread_mutex_init( &_g->stat.lock_data.lock , NULL );

#ifdef HAS_STATISTICSS
	M_BREAK_STAT( distributor_init_withOrder_lock( &_g->distributors.init_static_table , 1 ) , 0 );

	M_BREAK_STAT( mms_array_init( &_g->stat.nc_s_req.thread_list , sizeof( thread_alive_indicator ) , 1 , GROW_STEP , 0 ) , 0 );
	MM_BREAK_IF( pthread_mutex_init( &_g->stat.nc_s_req.thread_list_mtx , NULL ) , errCreation , 0 , "mutex_init()" );
#endif

#ifdef ENABLE_BYPASS_STDOUT
	init_bypass_stdout( _g );
#endif

#ifdef HAS_STATISTICSS
	M_BREAK_STAT( distributor_init_withLock( &_g->distributors.throttling_refresh_stat , 1 ) , 0 );
#endif

	M_BREAK_STAT( distributor_subscribe_withOrder( &_g->distributors.bcast_quit , SUB_LONG , SUB_FXN( cleanup_stat ) , _g , clean_stat ) , 0 );
	
	BEGIN_RET
	default:
	{
		if ( d_error ) DIST_APP_FAILURE();
	}
	M_V_END_RET
}

#ifdef HAS_STATISTICSS
_CALLBACK_FXN _PRIVATE_FXN void event_statistics_is_stabled( void_p src_g )
{
	INIT_BREAKABLE_FXN();
	G * _g = ( G * )src_g;


#ifdef ENABLE_STAT_THREAD_PROC
	MM_BREAK_IF( pthread_create( &_g->trds.tid_stats , NULL , stats_thread , ( pass_p )_g ) != PTHREAD_CREATE_OK , errCreation , 0 , "failed stats_thread" );
#endif

	BEGIN_RET
	default:
	{
		if ( d_error ) DIST_APP_FAILURE();
	}
	M_V_END_RET
}
#endif

_CALLBACK_FXN _PRIVATE_FXN void state_post_config_init_statistics( void_p src_g )
{
	INIT_BREAKABLE_FXN();
	G * _g = ( G * )src_g;

#ifdef HAS_STATISTICSS
	M_BREAK_STAT( distributor_subscribe_withOrder( &_g->distributors.init_static_table , SUB_VOID , SUB_FXN( init_main_statistics ) , _g , app_overview ) , 0 );
	M_BREAK_STAT( distributor_subscribe_withOrder( &_g->distributors.init_static_table , SUB_VOID , SUB_FXN( event_statistics_is_stabled ) , _g , statistics_is_stabled ) , 0 );
	M_BREAK_STAT( distributor_subscribe_withOrder( &_g->distributors.init_static_table , SUB_VOID , SUB_FXN( thread_overviewing ) , _g , thread_overview ) , 0 );
#endif

	BEGIN_RET
	default:
	{
		if ( d_error ) DIST_APP_FAILURE();
	}
	M_V_END_RET
}

PRE_MAIN_INITIALIZATION( PRE_MAIN_INIT_STATISTICS )
_PRIVATE_FXN void pre_main_init_stat_component( void )
{
	INIT_BREAKABLE_FXN();

	M_BREAK_STAT( distributor_subscribe( &_g->distributors.bcast_pre_cfg , SUB_VOID , SUB_FXN( state_pre_config_init_statistics ) , _g ) , 0 );
	M_BREAK_STAT( distributor_subscribe_withOrder( &_g->distributors.bcast_post_cfg , SUB_VOID , SUB_FXN( state_post_config_init_statistics ) , _g , post_config_order_statistics ) , 0 );

	BEGIN_RET
	default:
	{
		if ( d_error ) DIST_APP_FAILURE();
	}
	M_V_END_RET
}

void reset_nonuse_stat()
{
	//MEMSET( &_g->stat.round_zero_set , 0 , sizeof( _g->stat.round_zero_set ) );

	//cbuf_m_reset( &_g->stat.round_init_set.udp_stat_5_sec_count );
	//cbuf_m_reset( &_g->stat.round_init_set.udp_stat_10_sec_count );
	//cbuf_m_reset( &_g->stat.round_init_set.udp_stat_40_sec_count );
	//cbuf_m_reset( &_g->stat.round_init_set.udp_stat_120_sec_count );

	//cbuf_m_reset( &_g->stat.round_init_set.udp_stat_5_sec_bytes );
	//cbuf_m_reset( &_g->stat.round_init_set.udp_stat_10_sec_bytes );
	//cbuf_m_reset( &_g->stat.round_init_set.udp_stat_40_sec_bytes );
	//cbuf_m_reset( &_g->stat.round_init_set.udp_stat_120_sec_bytes );

	//cbuf_m_reset( &_g->stat.round_init_set.tcp_stat_5_sec_count );
	//cbuf_m_reset( &_g->stat.round_init_set.tcp_stat_10_sec_count );
	//cbuf_m_reset( &_g->stat.round_init_set.tcp_stat_40_sec_count );
	//cbuf_m_reset( &_g->stat.round_init_set.tcp_stat_120_sec_count );

	//cbuf_m_reset( &_g->stat.round_init_set.tcp_stat_5_sec_bytes );
	//cbuf_m_reset( &_g->stat.round_init_set.tcp_stat_10_sec_bytes );
	//cbuf_m_reset( &_g->stat.round_init_set.tcp_stat_40_sec_bytes );
	//cbuf_m_reset( &_g->stat.round_init_set.tcp_stat_120_sec_bytes );
}

_CALLBACK_FXN void init_main_statistics( pass_p src_g )
{
	INIT_BREAKABLE_FXN();

#ifdef HAS_STATISTICSS
	M_BREAK_STAT( nnc_begin_init_mode( &_g->stat.nc_h ) , 0 );
	M_BREAK_STAT( mms_array_init( &_g->stat.nc_s_req.field_keeper , sizeof( nnc_cell_content ) , 1 , GROW_STEP , 0 ) , 0 ); // some place to store field in one place and prevent realease mutiple field sorage
#endif

	//M_BREAK_STAT( dict_fst_create( &_g->stat.nc_s_req.map_flds , 256 ) , 0 );

#ifdef HAS_STATISTICSS

	nnc_lock_for_changes( &_g->stat.nc_h );

	M_BREAK_STAT( nnc_add_table( &_g->stat.nc_h , "P.B. overview" , &_g->stat.nc_s_req.pgeneral_tbl ) , 0 );

	nnc_table * ptbl = _g->stat.nc_s_req.pgeneral_tbl;
	// col
	M_BREAK_STAT( nnc_add_column( ptbl , ""  , "" , 0 ) , 0 );
	M_BREAK_STAT( nnc_add_column( ptbl , "A" , "" , 0 ) , 0 );
	M_BREAK_STAT( nnc_add_column( ptbl , "B" , "" , 0 ) , 0 );
	M_BREAK_STAT( nnc_add_column( ptbl , "C" , "" , 0 ) , 0 );
	M_BREAK_STAT( nnc_add_column( ptbl , "D" , "" , 0 ) , 0 );
	M_BREAK_STAT( nnc_add_column( ptbl , "E" , "" , 0 ) , 0 );
	M_BREAK_STAT( nnc_add_column( ptbl , "F" , "" , 10 ) , 0 );

	int irow = -1;
	int icol = 0; 
	nnc_cell_content * pcell = NULL;


	//--->>>
	irow++; icol = 0; M_BREAK_STAT( nnc_add_empty_row( ptbl , NULL ) , 0 );
	M_BREAK_STAT( nnc_set_static_int( ptbl , (size_t)irow , ( size_t )icol++ , irow + 1 ) , 0 );
	//---<<<
	// ver title
	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "ver" ) , 0 );
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&_g->stat.nc_s_req.ov_ver_cell ) , 0 );
	_g->stat.nc_s_req.ov_ver_cell->storage.bt.pass_data = _g;
	_g->stat.nc_s_req.ov_ver_cell->conversion_fxn = ov_cell_version_2_str;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , _g->stat.nc_s_req.ov_ver_cell ) , 0 );
	
	// time title
	M_BREAK_STAT( nnc_set_static_text( ptbl , (size_t)irow , ( size_t )icol++ , "time" ) , 0 );
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&_g->stat.nc_s_req.ov_time_cell ) , 0 );
	_g->stat.nc_s_req.ov_time_cell->storage.bt.pass_data = _g;
	_g->stat.nc_s_req.ov_time_cell->conversion_fxn = ov_cell_time_2_str;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , (size_t)irow , ( size_t )icol++ , _g->stat.nc_s_req.ov_time_cell ) , 0 );
	
	// elapse time title
	M_BREAK_STAT( nnc_set_static_text( ptbl , (size_t)irow , ( size_t )icol++ , "elapse" ) , 0 );
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&_g->stat.nc_s_req.ov_elapse_cell ) , 0 );
	_g->stat.nc_s_req.ov_elapse_cell->storage.bt.pass_data = _g;
	_g->stat.nc_s_req.ov_elapse_cell->conversion_fxn = ov_time_elapse_2_str;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , (size_t)irow , ( size_t )icol++ , _g->stat.nc_s_req.ov_elapse_cell ) , 0 );


	//--->>>
	irow++; icol = 0; M_BREAK_STAT( nnc_add_empty_row( ptbl , NULL ) , 0 );
	M_BREAK_STAT( nnc_set_static_int( ptbl , (size_t)irow , ( size_t )icol++ , irow + 1 ) , 0 );
	//---<<<
	// sys fault coun
	M_BREAK_STAT( nnc_set_static_text( ptbl , (size_t)irow , ( size_t )icol++ , "fault" ) , 0 );
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&_g->stat.nc_s_req.ov_fault_cell ) , 0 );
	_g->stat.nc_s_req.ov_fault_cell->storage.bt.pass_data = _g;
	_g->stat.nc_s_req.ov_fault_cell->conversion_fxn = ov_fault_2_str;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , (size_t)irow , ( size_t )icol++ , _g->stat.nc_s_req.ov_fault_cell ) , 0 );

	// UDP retry conn title
	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "thread cnt" ) , 0 );
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&_g->stat.nc_s_req.ov_thread_cnt_cell ) , 0 );
	_g->stat.nc_s_req.ov_thread_cnt_cell->storage.bt.pass_data = _g;
	_g->stat.nc_s_req.ov_thread_cnt_cell->conversion_fxn = ov_thread_cnt_2_str;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , _g->stat.nc_s_req.ov_thread_cnt_cell ) , 0 );

	//--->>>
	irow++; icol = 0; M_BREAK_STAT( nnc_add_empty_row( ptbl , NULL ) , 0 );
	M_BREAK_STAT( nnc_set_static_int( ptbl , (size_t)irow , ( size_t )icol++ , irow + 1 ) , 0 );
	//---<<<
	// UDP conn title
	M_BREAK_STAT( nnc_set_static_text( ptbl , (size_t)irow , ( size_t )icol++ , "UDP conn" ) , 0 );
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&_g->stat.nc_s_req.ov_UDP_conn_cell ) , 0 );
	_g->stat.nc_s_req.ov_UDP_conn_cell->storage.bt.pass_data = _g;
	_g->stat.nc_s_req.ov_UDP_conn_cell->conversion_fxn = ov_UDP_conn_2_str;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , (size_t)irow , ( size_t )icol++ , _g->stat.nc_s_req.ov_UDP_conn_cell ) , 0 );
	
	// UDP retry conn title
	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "UDP retry" ) , 0 );
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&_g->stat.nc_s_req.ov_UDP_retry_conn_cell ) , 0 );
	_g->stat.nc_s_req.ov_UDP_retry_conn_cell->storage.bt.pass_data = _g;
	_g->stat.nc_s_req.ov_UDP_retry_conn_cell->conversion_fxn = ov_UDP_retry_2_str;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , _g->stat.nc_s_req.ov_UDP_retry_conn_cell ) , 0 );


	//--->>>
	irow++; icol = 0; M_BREAK_STAT( nnc_add_empty_row( ptbl , NULL ) , 0 );
	M_BREAK_STAT( nnc_set_static_int( ptbl , (size_t)irow , ( size_t )icol++ , irow + 1 ) , 0 );
	//---<<<
	// TCP conn title
	M_BREAK_STAT( nnc_set_static_text( ptbl , ( size_t )irow , ( size_t )icol++ , "TCP conn" ) , 0 );
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&_g->stat.nc_s_req.ov_TCP_conn_cell ) , 0 );
	_g->stat.nc_s_req.ov_TCP_conn_cell->storage.bt.pass_data = _g;
	_g->stat.nc_s_req.ov_TCP_conn_cell->conversion_fxn = ov_TCP_conn_2_str;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , _g->stat.nc_s_req.ov_TCP_conn_cell ) , 0 );

	// TCP retry conn title
	M_BREAK_STAT( nnc_set_static_text( ptbl , (size_t)irow , ( size_t )icol++ , "TCP retry" ) , 0 );
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&_g->stat.nc_s_req.ov_TCP_retry_conn_cell ) , 0 );
	_g->stat.nc_s_req.ov_TCP_retry_conn_cell->storage.bt.pass_data = _g;
	_g->stat.nc_s_req.ov_TCP_retry_conn_cell->conversion_fxn = ov_TCP_retry_2_str;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , (size_t)irow , ( size_t )icol++ , _g->stat.nc_s_req.ov_TCP_retry_conn_cell ) , 0 );

#endif

	BEGIN_RET
	M_V_END_RET

	nnc_release_lock( &_g->stat.nc_h );
}

_CALLBACK_FXN PASSED_CSTR auto_refresh_thread_aliveness_cell( pass_p src_pcell )
{
	nnc_cell_content * pcell = ( nnc_cell_content * )src_pcell;
	G * _g = ( G * )pcell->storage.bt.pass_data;
	
	thread_alive_indicator * pthread_ind = NULL;
	THREAD_LOCK_LINE( pthread_mutex_lock( &_g->stat.nc_s_req.thread_list_mtx ) );
	if ( mms_array_get_s( &_g->stat.nc_s_req.thread_list , (size_t)pcell->storage.bt.j , ( void ** )&pthread_ind ) == errOK ) /*try find one thread based on position*/
	{
		int t = 1 + 2;
	}
	pthread_mutex_unlock( &_g->stat.nc_s_req.thread_list_mtx );
	if ( pthread_ind && pthread_ind->alive_time )
	{
		int n = snprintf( pcell->storage.tmpbuf , sizeof( pcell->storage.tmpbuf ) , "%s" , pthread_ind->thread_name );
		struct tm * tm_info = localtime( &pthread_ind->alive_time );
		strftime( ( ( char * )pcell->storage.tmpbuf ) + n , sizeof( pcell->storage.tmpbuf ) - (size_t)n , " %H:%M:%S" , tm_info );
	}
	else
	{
		pcell->storage.tmpbuf[ 0 ] = 0;
	}
	return ( PASSED_CSTR )pcell->storage.tmpbuf;
}

_CALLBACK_FXN void thread_overviewing( pass_p src_g )
{
	INIT_BREAKABLE_FXN();

	nnc_lock_for_changes( &_g->stat.nc_h );

#ifdef HAS_STATISTICSS


	M_BREAK_STAT( nnc_add_table( &_g->stat.nc_h , "threads" , &_g->stat.nc_s_req.pgeneral_tbl ) , 0 );

	nnc_table * ptbl = _g->stat.nc_s_req.pgeneral_tbl;
	// col
	M_BREAK_STAT( nnc_add_column( ptbl , "" , "" , 0 ) , 0 );
	M_BREAK_STAT( nnc_add_column( ptbl , "A" , "" , 0 ) , 0 );
	M_BREAK_STAT( nnc_add_column( ptbl , "B" , "" , 0 ) , 0 );
	M_BREAK_STAT( nnc_add_column( ptbl , "C" , "" , 0 ) , 0 );

	int irow = -1;
	int icol = 0;
	int cell = 0;
	nnc_cell_content * pcell = NULL;

	for ( int bunch = 0 ; bunch < 30 / 3 ; bunch++ )
	{
		//--->>>
		irow++; icol = 0; M_BREAK_STAT( nnc_add_empty_row( ptbl , NULL ) , 0 );
		M_BREAK_STAT( nnc_set_static_int( ptbl , ( size_t )irow , ( size_t )icol++ , irow + 1 ) , 0 );
		//---<<<
	
		for ( int bunch_item = 0 ; bunch_item < 3 ; bunch_item++ )
		{
			M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&pcell ) , 0 );
			pcell->storage.bt.pass_data = _g;
			pcell->storage.bt.j = cell++;
			pcell->conversion_fxn = auto_refresh_thread_aliveness_cell;
			M_BREAK_STAT( nnc_set_outer_cell( ptbl , ( size_t )irow , ( size_t )icol++ , pcell ) , 0 );
		}
	}
	
	M_BREAK_STAT( nnc_register_into_page_auto_refresh( ptbl , &_g->distributors.throttling_refresh_stat ) , 0 );

#endif

	BEGIN_RET
	M_V_END_RET

	nnc_release_lock( &_g->stat.nc_h );
}

// call at the end of every refresh must implied and refresh view
_CALLBACK_FXN void g_every_ticking_refresh( pass_p src_g )
{
	G * _g = ( G * )src_g;

#ifdef HAS_STATISTICSS
	nnc_cell_triggered( _g->stat.nc_s_req.ov_time_cell );
	nnc_cell_triggered( _g->stat.nc_s_req.ov_elapse_cell );

	continue_loop_callback( &_g->stat.nc_h );
#endif
}

/*
it must be always use lock free mechanism in this fxn because screen update also locked
*/
_THREAD_FXN void_p stats_thread( pass_p src_g )
{
	G * _g = ( G * )src_g;
	
#ifdef ENABLE_LOG_THREADS
	__attribute__( ( cleanup( thread_goes_out_of_scope ) ) ) pthread_t this_thread = pthread_self();
	distributor_publish_x3long( &_g->distributors.bcast_thread_startup , (long)this_thread , trdn_stats_thread , (long)__FUNCTION__ , _g );
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
	
#ifdef HAS_STATISTICSS
	distributor_subscribe( &_g->distributors.throttling_refresh_stat , SUB_VOID , SUB_FXN( g_every_ticking_refresh ) , _g );
	_g->distributors.throttling_refresh_stat.iteration_dir = tail_2_head; // first order issued then applied
#endif

	while ( 1 )
	{
		if ( pthis_thread_alive_time ) *pthis_thread_alive_time = time( NULL );
		if ( GRACEFULLY_END_NOLOSS_THREAD() ) break; // keep track changes until app is down

	#ifdef HAS_STATISTICSS
		// distribute statistic referesh pulse
		distributor_publish_void( &_g->distributors.throttling_refresh_stat , SUBSCRIBER_PROVIDED/*each subscriber set what it need*/ );
	#endif

		//pthread_mutex_lock( &_g->stat.lock_data.lock );
		//werase( _g->stat.main_win );
		//box( _g->stat.main_win , 0 , 0 );
		//draw_table( _g );
		//wrefresh( _g->stat.main_win );
		//pthread_mutex_unlock( &_g->stat.lock_data.lock );

		sleep( REFRESH_INTERVAL_SEC() ); // OK 14040526
	}
	return NULL;
}

// afer main config stablished then its time to broadcast it to every where meanwhile notcureses init him self and other stuff that suppose to happened
void init_UI( G * _g ) /*just to call by main thread*/
{
	CIRCUIT_BREAKER long break_cuit = 0;
	for ( ; !_g->appcfg.already_main_cfg_stablished && break_cuit < INFINITE_LOOP_GUARD() ; mng_basic_thread_sleep( _g , HI_PRIORITY_THREAD ) , break_cuit++ );
	if ( _g->appcfg.already_main_cfg_stablished && _g->distributors.bcast_program_stabled.grps.count )
	{
		distributor_publish_void( &_g->distributors.bcast_program_stabled , SUBSCRIBER_PROVIDED );
		sub_destroy( &_g->distributors.bcast_program_stabled ); // just one time anounce stablity
	}
}
