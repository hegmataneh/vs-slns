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

GLOBAL_VAR extern G * _g;

_CALLBACK_FXN void cleanup_stat( pass_p src_g , long v )
{
	G * _g = ( G * )src_g;

	#ifdef HAS_STATISTICSS
	nnc_destroy( &_g->stat.nc_h );
	mms_array_free( &_g->stat.nc_s_req.field_keeper );
	#endif

	sub_destroy( &_g->hdls.pkt_mgr.bcast_release_halffill_segment );
	
	MARK_LINE();
}

_CALLBACK_FXN _PRIVATE_FXN void pre_config_init_stat( void_p src_g )
{
	G * _g = ( G * )src_g;

	if ( _g->stat.aggregate_stat.t_begin.tv_sec == 0 && _g->stat.aggregate_stat.t_begin.tv_usec == 0 )
	{
		gettimeofday( &_g->stat.aggregate_stat.t_begin , NULL );
	}

	//pthread_mutex_init( &_g->stat.lock_data.lock , NULL );

	#ifdef HAS_STATISTICSS
	distributor_init_withOrder( &_g->distributors.init_static_table , 1 );
	#endif

	//init_tui( _g );
	 
	init_bypass_stdout( _g );

	#ifdef HAS_STATISTICSS
	distributor_init( &_g->distributors.throttling_refresh_stat , 1 );
	#endif

	distributor_subscribe_withOrder( &_g->distributors.bcast_quit , SUB_LONG , SUB_FXN( cleanup_stat ) , _g , clean_stat );
}

#ifdef HAS_STATISTICSS
_CALLBACK_FXN _PRIVATE_FXN void statistics_is_stabled_event( void_p src_g )
{
	G * _g = ( G * )src_g;
	pthread_create( &_g->trds.tid_stats , NULL , stats_thread , ( pass_p )_g );
}
#endif

_CALLBACK_FXN _PRIVATE_FXN void post_config_init_stat( void_p src_g )
{
	INIT_BREAKABLE_FXN();
	G * _g = ( G * )src_g;

	#ifdef HAS_STATISTICSS
	distributor_subscribe_withOrder( &_g->distributors.init_static_table , SUB_VOID , SUB_FXN( init_main_statistics ) , _g , main_statistics );
	distributor_subscribe_withOrder( &_g->distributors.init_static_table , SUB_VOID , SUB_FXN( statistics_is_stabled_event ) , _g , statistics_is_stabled );
	#endif

	BEGIN_SMPL
	M_V_END_RET
}

PRE_MAIN_INITIALIZATION( 103 )
_PRIVATE_FXN void pre_main_init_stat_component( void )
{
	distributor_subscribe( &_g->distributors.bcast_pre_cfg , SUB_VOID , SUB_FXN( pre_config_init_stat ) , _g );
	distributor_subscribe( &_g->distributors.bcast_post_cfg , SUB_VOID , SUB_FXN( post_config_init_stat ) , _g );
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
	M_BREAK_STAT( nnc_add_table( &_g->stat.nc_h , "P.B. overview" , &_g->stat.nc_s_req.pgeneral_tbl ) , 0 );

	nnc_table * ptbl = _g->stat.nc_s_req.pgeneral_tbl;
	// col
	M_BREAK_STAT( nnc_add_column( ptbl , "" , "" , 0 ) , 0 );
	M_BREAK_STAT( nnc_add_column( ptbl , "" , "" , 0 ) , 0 );
	M_BREAK_STAT( nnc_add_column( ptbl , "" , "" , 0 ) , 0 );
	M_BREAK_STAT( nnc_add_column( ptbl , "" , "" , 0 ) , 0 );
	M_BREAK_STAT( nnc_add_column( ptbl , "" , "" , 0 ) , 0 );
	M_BREAK_STAT( nnc_add_column( ptbl , "" , "" , 10 ) , 0 );

	// one new row
	M_BREAK_STAT( nnc_add_empty_row( ptbl , NULL ) , 0 );
	M_BREAK_STAT( nnc_add_empty_row( ptbl , NULL ) , 0 );
	M_BREAK_STAT( nnc_add_empty_row( ptbl , NULL ) , 0 );
	M_BREAK_STAT( nnc_add_empty_row( ptbl , NULL ) , 0 );
	M_BREAK_STAT( nnc_add_empty_row( ptbl , NULL ) , 0 );

	int irow = -1;

	irow++;
	//// time title
	M_BREAK_STAT( nnc_set_static_text( ptbl , (size_t)irow , 0 , "time" ) , 0 );
	// time cell
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&_g->stat.nc_s_req.ov_time_cell ) , 0 );
	_g->stat.nc_s_req.ov_time_cell->storage.bt.pass_data = _g;
	_g->stat.nc_s_req.ov_time_cell->conversion_fxn = ov_cell_time_2_str;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , (size_t)irow , 1 , _g->stat.nc_s_req.ov_time_cell ) , 0 );
	//// ver title
	M_BREAK_STAT( nnc_set_static_text( ptbl , 0 , 2 , "ver" ) , 0 );
	// ver cell
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&_g->stat.nc_s_req.ov_ver_cell ) , 0 );
	_g->stat.nc_s_req.ov_ver_cell->storage.bt.pass_data = _g;
	_g->stat.nc_s_req.ov_ver_cell->conversion_fxn = ov_cell_version_2_str;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , 0 , 3 , _g->stat.nc_s_req.ov_ver_cell ) , 0 );
	//// elapse time title
	M_BREAK_STAT( nnc_set_static_text( ptbl , 0 , 4 , "elapse" ) , 0 );
	// elapse time cell
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&_g->stat.nc_s_req.ov_elapse_cell ) , 0 );
	_g->stat.nc_s_req.ov_elapse_cell->storage.bt.pass_data = _g;
	_g->stat.nc_s_req.ov_elapse_cell->conversion_fxn = ov_time_elapse_2_str;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , 0 , 5 , _g->stat.nc_s_req.ov_elapse_cell ) , 0 );

	irow++;
	//// sys fault coun
	M_BREAK_STAT( nnc_set_static_text( ptbl , (size_t)irow , 0 , "fault" ) , 0 );
	// UDP conn cell
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&_g->stat.nc_s_req.ov_fault_cell ) , 0 );
	_g->stat.nc_s_req.ov_fault_cell->storage.bt.pass_data = _g;
	_g->stat.nc_s_req.ov_fault_cell->conversion_fxn = ov_fault_2_str;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , (size_t)irow , 1 , _g->stat.nc_s_req.ov_fault_cell ) , 0 );

	irow++;
	//// UDP conn title
	M_BREAK_STAT( nnc_set_static_text( ptbl , (size_t)irow , 0 , "UDP conn" ) , 0 );
	// UDP conn cell
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&_g->stat.nc_s_req.ov_UDP_conn_cell ) , 0 );
	_g->stat.nc_s_req.ov_UDP_conn_cell->storage.bt.pass_data = _g;
	_g->stat.nc_s_req.ov_UDP_conn_cell->conversion_fxn = ov_UDP_conn_2_str;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , (size_t)irow , 1 , _g->stat.nc_s_req.ov_UDP_conn_cell ) , 0 );
	//// TCP conn title
	M_BREAK_STAT( nnc_set_static_text( ptbl , (size_t)irow , 2 , "TCP conn" ) , 0 );
	// TCP conn cell
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&_g->stat.nc_s_req.ov_TCP_conn_cell ) , 0 );
	_g->stat.nc_s_req.ov_TCP_conn_cell->storage.bt.pass_data = _g;
	_g->stat.nc_s_req.ov_TCP_conn_cell->conversion_fxn = ov_TCP_conn_2_str;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , (size_t)irow , 3 , _g->stat.nc_s_req.ov_TCP_conn_cell ) , 0 );

	irow++;
	//// UDP retry conn title
	M_BREAK_STAT( nnc_set_static_text( ptbl , (size_t)irow , 0 , "UDP retry" ) , 0 );
	// UDP conn cell
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&_g->stat.nc_s_req.ov_UDP_retry_conn_cell ) , 0 );
	_g->stat.nc_s_req.ov_UDP_retry_conn_cell->storage.bt.pass_data = _g;
	_g->stat.nc_s_req.ov_UDP_retry_conn_cell->conversion_fxn = ov_UDP_retry_2_str;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , (size_t)irow , 1 , _g->stat.nc_s_req.ov_UDP_retry_conn_cell ) , 0 );
	//// TCP retry conn title
	M_BREAK_STAT( nnc_set_static_text( ptbl , (size_t)irow , 2 , "TCP retry" ) , 0 );
	// TCP conn cell
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&_g->stat.nc_s_req.ov_TCP_retry_conn_cell ) , 0 );
	_g->stat.nc_s_req.ov_TCP_retry_conn_cell->storage.bt.pass_data = _g;
	_g->stat.nc_s_req.ov_TCP_retry_conn_cell->conversion_fxn = ov_TCP_retry_2_str;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , (size_t)irow , 3 , _g->stat.nc_s_req.ov_TCP_retry_conn_cell ) , 0 );

	irow++;
	//// UDP retry conn title
	M_BREAK_STAT( nnc_set_static_text( ptbl , (size_t)irow , 0 , "thread cnt" ) , 0 );
	// UDP conn cell
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->stat.nc_s_req.field_keeper , ( void ** )&_g->stat.nc_s_req.ov_thread_cnt_cell ) , 0 );
	_g->stat.nc_s_req.ov_thread_cnt_cell->storage.bt.pass_data = _g;
	_g->stat.nc_s_req.ov_thread_cnt_cell->conversion_fxn = ov_thread_cnt_2_str;
	M_BREAK_STAT( nnc_set_outer_cell( ptbl , (size_t)irow , 1 , _g->stat.nc_s_req.ov_thread_cnt_cell ) , 0 );
	#endif

	BEGIN_RET
	M_V_END_RET
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

_THREAD_FXN void_p stats_thread( pass_p src_g )
{
	G * _g = ( G * )src_g;
	
	distributor_publish_long( &_g->distributors.bcast_thread_startup , (long)pthread_self() , _g );
	__attribute__( ( cleanup( thread_goes_out_of_scope ) ) ) pthread_t trd_id = pthread_self();
	MARK_START_THREAD();
	
	#ifdef HAS_STATISTICSS
	distributor_subscribe( &_g->distributors.throttling_refresh_stat , SUB_VOID , SUB_FXN( g_every_ticking_refresh ) , _g );
	_g->distributors.throttling_refresh_stat.iteration_dir = tail_2_head; // first order issued then applied
	#endif

	int tmp_debounce_release_segment = 0;

	while ( 1 )
	{
		if ( GRACEFULLY_END_THREAD() ) break; // keep track changes until app is down

		#ifdef HAS_STATISTICSS
		// distribute statistic referesh pulse
		distributor_publish_void( &_g->distributors.throttling_refresh_stat , SUBSCRIBER_PROVIDED/*each subscriber set what it need*/ );
		#endif

		if ( !( tmp_debounce_release_segment++ % 5 ) )
		{
			// distribute segment management pulse
			distributor_publish_void( &_g->hdls.pkt_mgr.bcast_release_halffill_segment , SUBSCRIBER_PROVIDED/*each subscriber set what it need*/ ); // check if condition is true then set halffill segemtn as fill
		}

		//pthread_mutex_lock( &_g->stat.lock_data.lock );
		//werase( _g->stat.main_win );
		//box( _g->stat.main_win , 0 , 0 );
		//draw_table( _g );
		//wrefresh( _g->stat.main_win );
		//pthread_mutex_unlock( &_g->stat.lock_data.lock );

		sleep( STAT_REFERESH_INTERVAL_SEC() ); // OK 14040526
		//sleep( 3 /*STAT_REFERESH_INTERVAL_SEC()*/ ); // OK 14040526
	}
	return NULL;
}

//void init_ncursor()
//{
//	initscr();
//	start_color();
//	cbreak();
//	noecho();
//	curs_set( 1 );
//
//	init_pair( 1 , COLOR_WHITE , COLOR_BLUE );   // Header
//	init_pair( 2 , COLOR_GREEN , COLOR_BLACK );  // Data
//	init_pair( 3 , COLOR_YELLOW , COLOR_BLACK ); // Last Command
//}
//void init_tui( G * _g )
//{
//	//init_ncursor( _g );
//
//	//getmaxyx( stdscr , _g->stat.scr_height , _g->stat.scr_width );
//
//	//// Calculate window sizes (60% for cells, 40% for input)
//	//int stats_height = _g->stat.scr_height - 3;
//	//int input_height = 3;
//
//	//// Create or replace windows
//	//if ( _g->stat.main_win ) delwin( _g->stat.main_win );
//	//if ( _g->stat.input_win ) delwin( _g->stat.input_win );
//
//	//_g->stat.main_win = newwin( stats_height , _g->stat.scr_width , 0 , 0 );
//	//_g->stat.input_win = newwin( input_height , _g->stat.scr_width , stats_height , 0 );
//
//	//// Enable scrolling and keypad for input window
//	//scrollok( _g->stat.main_win , TRUE );
//	//keypad( _g->stat.input_win , TRUE );
//
//	//// Set box borders
//	//box( _g->stat.main_win , 0 , 0 );
//	//box( _g->stat.input_win , 0 , 0 );
//
//	//// Refresh windows
//	//wrefresh( _g->stat.main_win );
//	//wrefresh( _g->stat.input_win );
//}
//// Centered cell printing
//void print_cell( WINDOW * win , int y , int x , int width , LPCSTR text )
//{
//	size_t len = STRLEN( text );
//	int pad = ( width - ( int )len ) / 2;
//	if ( pad < 0 ) pad = 0;
//	mvwprintw( win , y , x + pad , "%s" , text );
//}
//// Drawing the full table
//void draw_table( G * _g )
//{
//	char * header_border = "+----------+--------------------------------------------------------------------------------+";
//
//	int cell_w = STRLEN( header_border ) / 2;
//	int start_x = 2;
//	int y = 1;
//
//	// Top border
//	mvwprintw( MAIN_WIN , y++ , start_x , header_border );
//
//	char buf[ 640 ];
//	char buf2[ 64 ];
//	char buf3[ 64 ];
//
//	//
//	mvwprintw( MAIN_WIN , y , start_x , "|" );
//	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "alive" );
//	snprintf( buf , sizeof( buf ) , "%d%.*s" , MAIN_STAT().last_line_meet , MAIN_STAT().alive_check_counter , "-+-+-+-+-+-+-+-+" );
//	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
//	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
//	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );
//
//	//////////////
//	
//	//print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "inp failure" );
//	//snprintf( buf , sizeof( buf ) , "v%d Σv%d" , MAIN_STAT().round_zero_set.continuously_unsuccessful_receive_error , MAIN_STAT().round_zero_set.total_unsuccessful_receive_error);
//
//	//print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "full head tail" );
//	//snprintf( buf , sizeof( buf ) , "%d %d %d" , ppp ? ppp->err_full : 0 , ppp ? ppp->head : 0 , ppp ? ppp->tail : 0 );
//	
//	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
//	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
//	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );
//
//	mvwprintw( MAIN_WIN , y , start_x , "|" );
//	
//	//print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "out failure" );
//	//snprintf( buf , sizeof( buf ) , "^%d Σ^%d" , MAIN_STAT().round_zero_set.continuously_unsuccessful_send_error , MAIN_STAT().round_zero_set.total_unsuccessful_send_error );
//
//	//print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "max packet delay udp tcp" );
//	//snprintf( buf , sizeof( buf ) , "%.0f %.0f" , MAIN_STAT().max_udp_packet_delay , MAIN_STAT().max_tcp_packet_delay );
//
//	//mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
//	//print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
//	//mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );
//
//	mvwprintw( MAIN_WIN , y++ , start_x , header_border );
//
//	#ifdef __USE_DBG_TOOLS
//	if ( _g->appcfg.g_cfg && _g->appcfg.g_cfg->c.c.show_line_hit )
//	{
//		for ( int itu = 0 ; itu < MAX_TU ; itu++ )
//		{
//			for ( int iline = 0 ; iline < MAX_TU_LINES ; iline++ )
//			{
//				if ( __FXN_HIT[ itu ][ iline ][ 0 ] > 0 ) // this line hit
//				{
//					mvwprintw( MAIN_WIN , y , start_x , "|" );
//					snprintf( buf , sizeof( buf ) , "%s %d" , __map_c2idx[ itu ] , iline ); // line
//					print_cell( MAIN_WIN , y , start_x + 1 , cell_w , buf ); // line
//					snprintf( buf , sizeof( buf ) , "%d " , __FXN_HIT[ itu ][ iline ][ 0 ] ); // hit count
//
//					for ( int ibt = 1 ; ibt < BACKTRACK_COUNT ; ibt++ )
//					{
//						if ( __FXN_HIT[ itu ][ iline ][ ibt ] == 0 )
//						{
//							break;
//						}
//						snprintf( buf2 , sizeof( buf2 ) , ",%d" , __FXN_HIT[ itu ][ iline ][ ibt ] );
//						strcat( buf , buf2 );
//					}
//			
//					mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
//					print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
//					mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );
//				}
//			}
//		}
//
//		mvwprintw( MAIN_WIN , y++ , start_x , header_border );
//	}
//	#endif
//
//	///////////
//	
//
//	
//
//	mvwprintw( MAIN_WIN , y , start_x , "|" );
//	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "syscal_err" );
//	//snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof(buf2) , MAIN_STAT().round_zero_set.syscal_err_count , 2 , "" ) );
//	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
//	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
//	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );
//
//	//if ( _g->appcfg.g_cfg && _g->appcfg.g_cfg->c.c.atht == bidirection && _g->bridges.bidirection_thread )
//	//{
//	//	mvwprintw( MAIN_WIN , y , start_x , "|" );
//	//	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "qu cnt " );
//	//	snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , ( ubigint )_g->bridges.bidirection_thread->queue.count , 2 , "" ) );
//	//	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
//	//	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
//	//	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );
//	//}
//
//	mvwprintw( MAIN_WIN , y++ , start_x , header_border );
//
//
//
//	wattroff( MAIN_WIN , COLOR_PAIR( 2 ) );
//
//	// Mid border
//	mvwprintw( MAIN_WIN , y++ , start_x , header_border );
//
//	// Last Command Row
//	wattron( MAIN_WIN , COLOR_PAIR( 3 ) );
//	mvwprintw( MAIN_WIN , y , start_x , "|" );
//	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "Last Cmd" );
//	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
//	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , MAIN_STAT().last_command );
//	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );
//	wattroff( MAIN_WIN , COLOR_PAIR( 3 ) );
//
//	// Bottom border
//	mvwprintw( MAIN_WIN , y++ , start_x , header_border );
//}
