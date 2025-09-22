#define Uses_STRLEN
#define Uses_sleep
#define Uses_setlocale
#define Uses_memset
#define Uses_TWD
#define Uses_pthread_t
#define Uses_ncurses
#define Uses_statistics
#define Uses_helper
#define Uses_vcbuf_nb
#include <Protocol_Bridge.dep>

extern G * _g;

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

// Centered cell printing
void print_cell( WINDOW * win , int y , int x , int width , LPCSTR text )
{
	size_t len = STRLEN( text );
	int pad = ( width - ( int )len ) / 2;
	if ( pad < 0 ) pad = 0;
	mvwprintw( win , y , x + pad , "%s" , text );
}

extern size_t * ptotal_items;	  // temp
extern size_t * ptotal_bytes;	  // temp
extern size_t * psegment_total;	  // temp
extern size_t * pfilled_count;	  // temp
extern size_t * perr_full;

#define MAIN_STAT()  _g->stat
#define MAIN_WIN  MAIN_STAT().main_win

#define _FORMAT_SHRTFRM( baaf , NPP , val , decimal_precision , unit ) ( NUMBER_IN_SHORT_FORM() ? \
		format_pps( baaf , sizeof(baaf) , val , decimal_precision , unit ) :\
		__snprintf( baaf , sizeof(baaf) , "%llu" , val ) )

// Drawing the full table
void draw_table( G * _g )
{
	char * header_border = "+----------+--------------------------------------------------------------------------------+";

	int cell_w = STRLEN( header_border ) / 2;
	int start_x = 2;
	int y = 1;

	// Top border
	mvwprintw( MAIN_WIN , y++ , start_x , header_border );

	char buf[ 640 ];
	char buf2[ 64 ];
	char buf3[ 64 ];
	struct timespec now;
	clock_gettime( CLOCK_REALTIME , &now );
	format_clock_time( &now , buf , sizeof( buf ) );
	snprintf( buf2 , sizeof( buf2 ) , "PB Metric-%s" , buf );

	// Header
	wattron( MAIN_WIN , COLOR_PAIR( 1 ) );
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , buf2 );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , "Value" );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );
	wattroff( MAIN_WIN , COLOR_PAIR( 1 ) );

	// Header border
	mvwprintw( MAIN_WIN , y++ , start_x , header_border );

	// Data rows
	wattron( MAIN_WIN , COLOR_PAIR( 2 ) );
	
	setlocale(LC_NUMERIC, "en_US.UTF-8");

	//
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "ver." );
	snprintf( buf , sizeof( buf ) , "%s" , _g->appcfg.ver->version );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );

	//
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "alive" );
	snprintf( buf , sizeof( buf ) , "%d%.*s" , MAIN_STAT().last_line_meet , MAIN_STAT().alive_check_counter , "-+-+-+-+-+-+-+-+" );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );

	//////////////
	
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "UDP conn" );

	//snprintf( buf , sizeof( buf ) , "%d Σ%d %-*.*s" , MAIN_STAT().udp_connection_count , MAIN_STAT().total_retry_udp_connection_count , 10 , ( MAIN_STAT().udp_get_data_alive_indicator % 10 ) , "<<<<<<<<<<" );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );

	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "TCP conn" );
	//snprintf( buf , sizeof( buf ) , "%d Σ%d %-*.*s" , MAIN_STAT().tcp_connection_count , MAIN_STAT().total_retry_tcp_connection_count , 10 , ( MAIN_STAT().tcp_send_data_alive_indicator % 10 ) , ">>>>>>>>>>" );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );

	mvwprintw( MAIN_WIN , y , start_x , "|" );
	
	//print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "inp failure" );
	//snprintf( buf , sizeof( buf ) , "v%d Σv%d" , MAIN_STAT().round_zero_set.continuously_unsuccessful_receive_error , MAIN_STAT().round_zero_set.total_unsuccessful_receive_error);

	//print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "full head tail" );
	//snprintf( buf , sizeof( buf ) , "%d %d %d" , ppp ? ppp->err_full : 0 , ppp ? ppp->head : 0 , ppp ? ppp->tail : 0 );
	
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );

	mvwprintw( MAIN_WIN , y , start_x , "|" );
	
	//print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "out failure" );
	//snprintf( buf , sizeof( buf ) , "^%d Σ^%d" , MAIN_STAT().round_zero_set.continuously_unsuccessful_send_error , MAIN_STAT().round_zero_set.total_unsuccessful_send_error );

	//print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "max packet delay udp tcp" );
	//snprintf( buf , sizeof( buf ) , "%.0f %.0f" , MAIN_STAT().max_udp_packet_delay , MAIN_STAT().max_tcp_packet_delay );

	//mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	//print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	//mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );

	mvwprintw( MAIN_WIN , y++ , start_x , header_border );

	#ifdef __USE_DBG_TOOLS
	if ( _g->appcfg.g_cfg && _g->appcfg.g_cfg->c.c.show_line_hit )
	{
		for ( int itu = 0 ; itu < MAX_TU ; itu++ )
		{
			for ( int iline = 0 ; iline < MAX_TU_LINES ; iline++ )
			{
				if ( __FXN_HIT[ itu ][ iline ][ 0 ] > 0 ) // this line hit
				{
					mvwprintw( MAIN_WIN , y , start_x , "|" );
					snprintf( buf , sizeof( buf ) , "%s %d" , __map_c2idx[ itu ] , iline ); // line
					print_cell( MAIN_WIN , y , start_x + 1 , cell_w , buf ); // line
					snprintf( buf , sizeof( buf ) , "%d " , __FXN_HIT[ itu ][ iline ][ 0 ] ); // hit count

					for ( int ibt = 1 ; ibt < BACKTRACK_COUNT ; ibt++ )
					{
						if ( __FXN_HIT[ itu ][ iline ][ ibt ] == 0 )
						{
							break;
						}
						snprintf( buf2 , sizeof( buf2 ) , ",%d" , __FXN_HIT[ itu ][ iline ][ ibt ] );
						strcat( buf , buf2 );
					}
			
					mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
					print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
					mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );
				}
			}
		}

		mvwprintw( MAIN_WIN , y++ , start_x , header_border );
	}
	#endif

	///////////
	

	

	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "syscal_err" );
	//snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof(buf2) , MAIN_STAT().round_zero_set.syscal_err_count , 2 , "" ) );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );

	//if ( _g->appcfg.g_cfg && _g->appcfg.g_cfg->c.c.atht == bidirection && _g->bridges.bidirection_thread )
	//{
	//	mvwprintw( MAIN_WIN , y , start_x , "|" );
	//	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "qu cnt " );
	//	snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , ( ubigint )_g->bridges.bidirection_thread->queue.count , 2 , "" ) );
	//	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	//	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	//	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );
	//}

	mvwprintw( MAIN_WIN , y++ , start_x , header_border );

	/*temp*/
	if ( ptotal_items )
	{
		mvwprintw( MAIN_WIN , y , start_x , "|" );
		print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "ttl pkt in global buff" );
		snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , *ptotal_items , 2 , "" ) );
		mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
		print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
		mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );
	}

	if ( ptotal_bytes )
	{
		mvwprintw( MAIN_WIN , y , start_x , "|" );
		print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "ttl pkt byte" );
		snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , *ptotal_bytes , 2 , "" ) );
		mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
		print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
		mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );
	}

	if ( psegment_total )
	{
		mvwprintw( MAIN_WIN , y , start_x , "|" );
		print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "segment_total" );
		snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , *psegment_total , 2 , "" ) );
		mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
		print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
		mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );
	}

	if ( pfilled_count )
	{
		mvwprintw( MAIN_WIN , y , start_x , "|" );
		print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "filled_count" );
		snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , *pfilled_count , 2 , "" ) );
		mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
		print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
		mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );
	}

	if ( perr_full )
	{
		mvwprintw( MAIN_WIN , y , start_x , "|" );
		print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "ring udp get loss" );
		snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , *perr_full , 2 , "" ) );
		mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
		print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
		mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );
	}

	

	/*temp*/


	//format_elapsed_time_with_millis( _g->stat.round_zero_set.t_begin , _g->stat.round_zero_set.t_end , buf2 , sizeof( buf2 ) , 1);
	//
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "itr duration" );
	snprintf( buf , sizeof( buf ) , "%s" , buf2 );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );

	//
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	
	
	if ( _g->bridges.ABhs_masks_count > 0 && _g->bridges.ABhs_masks[ 0 ] && _g->bridges.ABs[ 0 ].single_AB ) // this cnd is temp
	{
	
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "udp get" );
	snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , _g->bridges.ABs[ 0 ].single_AB->stat.round_zero_set.udp.total_udp_get_count , 2 , "" ) );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );
	//
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "udp get byte" );
	snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , _g->bridges.ABs[ 0 ].single_AB->stat.round_zero_set.udp.total_udp_get_byte , 2 , "B" ) );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );

	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "tcp put" );
	snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , _g->bridges.ABs[ 0 ].single_AB->stat.round_zero_set.tcp.total_tcp_put_count , 2 , "" ) );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );
	//
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "tcp put byte" );
	snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , _g->bridges.ABs[ 0 ].single_AB->stat.round_zero_set.tcp.total_tcp_put_byte , 2 , "B" ) );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );

	}

	//mvwprintw( MAIN_WIN , y , start_x , "|" );
	//print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "contnu unsuces slct udp" );
	////	snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , MAIN_STAT().round_zero_set.udp.continuously_unsuccessful_select_on_open_port_count , 2 , "" ) );
	//mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	//print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	//mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );

	#ifndef time_frame

	//// 5 sec
	//mvwprintw( MAIN_WIN , y , start_x , "|" );
	//print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "5s udp pps" );
	////snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , ( ubigint )cbuf_m_mean_all( &MAIN_STAT().round_init_set.udp_stat_5_sec_count ) , 4 , "" ) );
	//mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	//print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	//mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );
	////
	//mvwprintw( MAIN_WIN , y , start_x , "|" );
	//print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "5s udp bps" );
	////snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , ( ubigint )cbuf_m_mean_all( &MAIN_STAT().round_init_set.udp_stat_5_sec_bytes ) , 4 , "B" ) );
	//mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	//print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	//mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );

	//// 10 sec
	//mvwprintw( MAIN_WIN , y , start_x , "|" );
	//print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "10s udp pps" );
	////snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , ( ubigint )cbuf_m_mean_all( &MAIN_STAT().round_init_set.udp_stat_10_sec_count ) , 4 , "" ) );
	//mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	//print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	//mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );
	////
	//mvwprintw( MAIN_WIN , y , start_x , "|" );
	//print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "10s udp bps" );
	////snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , ( ubigint )cbuf_m_mean_all( &MAIN_STAT().round_init_set.udp_stat_10_sec_bytes ) , 4 , "B" ) );
	//mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	//print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	//mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );

	//// 40 sec
	//mvwprintw( MAIN_WIN , y , start_x , "|" );
	//print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "40s udp pps" );
	////snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , ( ubigint )cbuf_m_mean_all( &MAIN_STAT().round_init_set.udp_stat_40_sec_count ) , 4 , "" ) );
	//mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	//print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	//mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );
	////
	//mvwprintw( MAIN_WIN , y , start_x , "|" );
	//print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "40s udp bps" );
	////snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , ( ubigint )cbuf_m_mean_all( &MAIN_STAT().round_init_set.udp_stat_40_sec_bytes ) , 4 , "B" ) );
	//mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	//print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	//mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );

	//// 120 sec
	//mvwprintw( MAIN_WIN , y , start_x , "|" );
	//print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "120s udp pps" );
	////snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , ( ubigint )cbuf_m_mean_all( &MAIN_STAT().round_init_set.udp_stat_120_sec_count ) , 4 , "" ) );
	//mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	//print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	//mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );
	////
	//mvwprintw( MAIN_WIN , y , start_x , "|" );
	//print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "120s udp bps" );
	////snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , ( ubigint )cbuf_m_mean_all( &MAIN_STAT().round_init_set.udp_stat_120_sec_bytes ) , 4 , "B" ) );
	//mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	//print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	//mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );

	mvwprintw( MAIN_WIN , y++ , start_x , header_border );

	#endif


	#ifndef tcp

	//
	////mvwprintw( MAIN_WIN , y , start_x , "|" );
	////print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "tcp put" );
	//////snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , MAIN_STAT().round_zero_set.tcp.total_tcp_put_count , 2 , "" ) );
	////mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	////print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	////mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );
	//////
	////mvwprintw( MAIN_WIN , y , start_x , "|" );
	////print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "tcp put byte" );
	//////snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , MAIN_STAT().round_zero_set.tcp.total_tcp_put_byte , 2 , "B" ) );
	////mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	////print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	////mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );


	////// 5 sec
	////mvwprintw( MAIN_WIN , y , start_x , "|" );
	////print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "5s tcp pps" );
	//////snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , ( ubigint )cbuf_m_mean_all( &MAIN_STAT().round_init_set.tcp_stat_5_sec_count ) , 4 , "" ) );
	////mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	////print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	////mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );
	//////
	////mvwprintw( MAIN_WIN , y , start_x , "|" );
	////print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "5s tcp bps" );
	//////snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , ( ubigint )cbuf_m_mean_all( &MAIN_STAT().round_init_set.tcp_stat_5_sec_bytes ) , 4 , "B" ) );
	////mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	////print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	////mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );


	//////// 10 sec
	////mvwprintw( MAIN_WIN , y , start_x , "|" );
	////print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "10s tcp pps" );
	//////snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , ( ubigint )cbuf_m_mean_all( &MAIN_STAT().round_init_set.tcp_stat_10_sec_count ) , 4 , "" ) );
	////mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	////print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	////mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );
	//////
	////mvwprintw( MAIN_WIN , y , start_x , "|" );
	////print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "10s tcp bps" );
	//////snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , ( ubigint )cbuf_m_mean_all( &MAIN_STAT().round_init_set.tcp_stat_10_sec_bytes ) , 4 , "B" ) );
	////mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	////print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	////mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );

	//////// 40 sec
	////mvwprintw( MAIN_WIN , y , start_x , "|" );
	////print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "40s tcp pps" );
	//////snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , ( ubigint )cbuf_m_mean_all( &MAIN_STAT().round_init_set.tcp_stat_40_sec_count ) , 4 , "" ) );
	////mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	////print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	////mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );
	//////
	////mvwprintw( MAIN_WIN , y , start_x , "|" );
	////print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "40s tcp bps" );
	//////snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , ( ubigint )cbuf_m_mean_all( &MAIN_STAT().round_init_set.tcp_stat_40_sec_bytes ) , 4 , "B" ) );
	////mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	////print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	////mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );

	//////// 120 sec
	////mvwprintw( MAIN_WIN , y , start_x , "|" );
	////print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "120s tcp pps" );
	//////snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , ( ubigint )cbuf_m_mean_all( &MAIN_STAT().round_init_set.tcp_stat_120_sec_count ) , 4 , "" ) );
	////mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	////print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	////mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );
	//////
	////mvwprintw( MAIN_WIN , y , start_x , "|" );
	////print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "120s tcp bps" );
	//////snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , ( ubigint )cbuf_m_mean_all( &MAIN_STAT().round_init_set.tcp_stat_120_sec_bytes ) , 4 , "B" ) );
	////mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	////print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	////mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );

	#endif


	wattroff( MAIN_WIN , COLOR_PAIR( 2 ) );

	// Mid border
	mvwprintw( MAIN_WIN , y++ , start_x , header_border );

	// Last Command Row
	wattron( MAIN_WIN , COLOR_PAIR( 3 ) );
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "Last Cmd" );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , MAIN_STAT().last_command );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );
	wattroff( MAIN_WIN , COLOR_PAIR( 3 ) );

	// Bottom border
	mvwprintw( MAIN_WIN , y++ , start_x , header_border );
}

_THREAD_FXN void_p stats_thread( pass_p src_g )
{
	static TWD twd = { 0 };
	if ( twd.threadId == 0 )
	{
		twd.threadId = pthread_self();
		twd.cal = stats_thread; // self function address
		twd.callback_arg = src_g;
	}
	if ( src_g == NULL )
	{
		return ( void_p )&twd;
	}
	G * _g = ( G * )src_g;

	nnc_begin_render_mode( &_g->nnc );

	while ( 1 )
	{
		if ( CLOSE_APP_VAR() ) break; // keep track changes until app is down

		couninue_loop_callback( &_g->nnc );

		//pthread_mutex_lock( &_g->stat.lock_data.lock );

		//werase( _g->stat.main_win );
		//box( _g->stat.main_win , 0 , 0 );
		//draw_table( _g );
		//wrefresh( _g->stat.main_win );

		//pthread_mutex_unlock( &_g->stat.lock_data.lock );

		sleep( STAT_REFERESH_INTERVAL_SEC() ); // OK 14040526
	}
	return NULL;
}

void init_ncursor()
{
	initscr();
	start_color();
	cbreak();
	noecho();
	curs_set( 1 );

	init_pair( 1 , COLOR_WHITE , COLOR_BLUE );   // Header
	init_pair( 2 , COLOR_GREEN , COLOR_BLACK );  // Data
	init_pair( 3 , COLOR_YELLOW , COLOR_BLACK ); // Last Command
}

void init_notcursor( G * _g )
{
	nnc_begin_init_mode( &_g->nnc );
}

void init_tui( G * _g )
{
	init_notcursor( _g );

	return;

	init_ncursor( _g );

	getmaxyx( stdscr , _g->stat.scr_height , _g->stat.scr_width );

	// Calculate window sizes (60% for cells, 40% for input)
	int stats_height = _g->stat.scr_height - 3;
	int input_height = 3;

	// Create or replace windows
	if ( _g->stat.main_win ) delwin( _g->stat.main_win );
	if ( _g->stat.input_win ) delwin( _g->stat.input_win );

	_g->stat.main_win = newwin( stats_height , _g->stat.scr_width , 0 , 0 );
	_g->stat.input_win = newwin( input_height , _g->stat.scr_width , stats_height , 0 );

	// Enable scrolling and keypad for input window
	scrollok( _g->stat.main_win , TRUE );
	keypad( _g->stat.input_win , TRUE );

	// Set box borders
	box( _g->stat.main_win , 0 , 0 );
	box( _g->stat.input_win , 0 , 0 );

	// Refresh windows
	wrefresh( _g->stat.main_win );
	wrefresh( _g->stat.input_win );
}

