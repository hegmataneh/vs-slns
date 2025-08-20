#define Uses_sleep
#define Uses_setlocale
#define Uses_memset
#define Uses_TWD
#define Uses_pthread_t
#define Uses_ncurses
#define Uses_statistics
#define Uses_helper
#define Uses_vcbuf_nb

//#define DIRECT_ECHO_BUF _g->stat.last_command // just before include dep
#include <Protocol_Bridge.dep>

extern G * __g;

extern vcbuf_nb * ppp;

void reset_nonuse_stat()
{
	G * _g = __g;
	memset( &_g->stat.round_zero_set , 0 , sizeof( _g->stat.round_zero_set ) );

	cbuf_m_reset( &_g->stat.round_init_set.udp_stat_5_sec_count );
	cbuf_m_reset( &_g->stat.round_init_set.udp_stat_10_sec_count );
	cbuf_m_reset( &_g->stat.round_init_set.udp_stat_40_sec_count );
	cbuf_m_reset( &_g->stat.round_init_set.udp_stat_120_sec_count );

	cbuf_m_reset( &_g->stat.round_init_set.udp_stat_5_sec_bytes );
	cbuf_m_reset( &_g->stat.round_init_set.udp_stat_10_sec_bytes );
	cbuf_m_reset( &_g->stat.round_init_set.udp_stat_40_sec_bytes );
	cbuf_m_reset( &_g->stat.round_init_set.udp_stat_120_sec_bytes );

	cbuf_m_reset( &_g->stat.round_init_set.tcp_stat_5_sec_count );
	cbuf_m_reset( &_g->stat.round_init_set.tcp_stat_10_sec_count );
	cbuf_m_reset( &_g->stat.round_init_set.tcp_stat_40_sec_count );
	cbuf_m_reset( &_g->stat.round_init_set.tcp_stat_120_sec_count );

	cbuf_m_reset( &_g->stat.round_init_set.tcp_stat_5_sec_bytes );
	cbuf_m_reset( &_g->stat.round_init_set.tcp_stat_10_sec_bytes );
	cbuf_m_reset( &_g->stat.round_init_set.tcp_stat_40_sec_bytes );
	cbuf_m_reset( &_g->stat.round_init_set.tcp_stat_120_sec_bytes );
}

// Centered cell printing
void print_cell( WINDOW * win , int y , int x , int width , const char * text )
{
	size_t len = strlen( text );
	int pad = ( width - ( int )len ) / 2;
	if ( pad < 0 ) pad = 0;
	mvwprintw( win , y , x + pad , "%s" , text );
}

#define MAIN_STAT()  _g->stat
#define MAIN_WIN  MAIN_STAT().main_win

// Drawing the full table
void draw_table( G * _g )
{
	char * header_border = "+----------+--------------------------------------------------------------------------------+";

	int cell_w = strlen( header_border ) / 2;
	int start_x = 2;
	int y = 1;

	// Top border
	mvwprintw( MAIN_WIN , y++ , start_x , header_border );

	char buf[ 640 ];
	char buf2[ 64 ];
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
	snprintf( buf , sizeof( buf ) , "%d Σ%d" , MAIN_STAT().udp_connection_count , MAIN_STAT().total_retry_udp_connection_count );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );

	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "TCP conn" );
	snprintf( buf , sizeof( buf ) , "%d Σ%d" , MAIN_STAT().tcp_connection_count , MAIN_STAT().total_retry_tcp_connection_count );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );

	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "inp failure" );
	
	snprintf( buf , sizeof( buf ) , "%d %d %d" , ppp ? ppp->err_full : 0 , ppp ? ppp->head : 0 , ppp ? ppp->tail : 0 );

	//snprintf( buf , sizeof( buf ) , "v%d Σv%d" , MAIN_STAT().round_zero_set.continuously_unsuccessful_receive_error , MAIN_STAT().round_zero_set.total_unsuccessful_receive_error);
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );

	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "out failure" );
	snprintf( buf , sizeof( buf ) , "^%d Σ^%d" , MAIN_STAT().round_zero_set.continuously_unsuccessful_send_error , MAIN_STAT().round_zero_set.total_unsuccessful_send_error );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );

	mvwprintw( MAIN_WIN , y++ , start_x , header_border );

	//if ( _g->appcfg.g_cfg && _g->appcfg.g_cfg->c.c.show_line_hit )
	//{
	//	for ( int i = 0 ; i < FXN_HIT_COUNT ; i++ )
	//	{
	//		if ( __FXN_HIT[ i ][0] > 0 )
	//		{
	//			mvwprintw( MAIN_WIN , y , start_x , "|" );
	//			snprintf( buf , sizeof( buf ) , "%d" , i ); // line
	//			print_cell( MAIN_WIN , y , start_x + 1 , cell_w , buf ); // line
	//			snprintf( buf , sizeof( buf ) , "%d " , __FXN_HIT[ i ][0] ); // hit count

	//			for ( int k = 1 ; k < PC_COUNT ; k++ )
	//			{
	//				if ( __FXN_HIT[ i ][ k ] == 0 )
	//				{
	//					break;
	//				}
	//				snprintf( buf2 , sizeof( buf2 ) , ",%d" , __FXN_HIT[ i ][ k ] );
	//				strcat( buf , buf2 );
	//			}
	//		
	//			mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	//			print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	//			mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );
	//		}
	//	}

	//	mvwprintw( MAIN_WIN , y++ , start_x , header_border );
	//}

	///////////

	#define _FORMAT_SHRTFRM( baaf , NPP , val , decimal_precision , unit ) ( NUMBER_IN_SHORT_FORM() ? \
		format_pps( baaf , sizeof(baaf) , val , decimal_precision , unit ) :\
		__snprintf( baaf , sizeof(baaf) , "%llu" , val ) )

	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "syscal_err" );
	snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof(buf2) , MAIN_STAT().round_zero_set.syscal_err_count , 2 , "" ) );
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

	format_elapsed_time_with_millis( _g->stat.round_zero_set.t_begin , _g->stat.round_zero_set.t_end , buf2 , sizeof( buf2 ) , 1);
	//
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "itr duration" );
	snprintf( buf , sizeof( buf ) , "%s" , buf2 );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );

	//
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "udp get" );
	snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , MAIN_STAT().round_zero_set.udp.total_udp_get_count , 2 , "" ) );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );
	//
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "udp get byte" );
	snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , MAIN_STAT().round_zero_set.udp.total_udp_get_byte , 2 , "B" ) );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );

	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "contnu unsuces slct udp" );
	snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , MAIN_STAT().round_zero_set.udp.continuously_unsuccessful_select_on_open_port_count , 2 , "" ) );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );

	#ifndef time_frame

	// 5 sec
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "5s udp pps" );
	snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , ( ubigint )cbuf_m_mean_all( &MAIN_STAT().round_init_set.udp_stat_5_sec_count ) , 4 , "" ) );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );
	//
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "5s udp bps" );
	snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , ( ubigint )cbuf_m_mean_all( &MAIN_STAT().round_init_set.udp_stat_5_sec_bytes ) , 4 , "B" ) );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );

	// 10 sec
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "10s udp pps" );
	snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , ( ubigint )cbuf_m_mean_all( &MAIN_STAT().round_init_set.udp_stat_10_sec_count ) , 4 , "" ) );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );
	//
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "10s udp bps" );
	snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , ( ubigint )cbuf_m_mean_all( &MAIN_STAT().round_init_set.udp_stat_10_sec_bytes ) , 4 , "B" ) );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );

	// 40 sec
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "40s udp pps" );
	snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , ( ubigint )cbuf_m_mean_all( &MAIN_STAT().round_init_set.udp_stat_40_sec_count ) , 4 , "" ) );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );
	//
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "40s udp bps" );
	snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , ( ubigint )cbuf_m_mean_all( &MAIN_STAT().round_init_set.udp_stat_40_sec_bytes ) , 4 , "B" ) );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );

	// 120 sec
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "120s udp pps" );
	snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , ( ubigint )cbuf_m_mean_all( &MAIN_STAT().round_init_set.udp_stat_120_sec_count ) , 4 , "" ) );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );
	//
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "120s udp bps" );
	snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , ( ubigint )cbuf_m_mean_all( &MAIN_STAT().round_init_set.udp_stat_120_sec_bytes ) , 4 , "B" ) );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );

	mvwprintw( MAIN_WIN , y++ , start_x , header_border );

	#endif


	#ifndef tcp

	//
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "tcp put" );
	snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , MAIN_STAT().round_zero_set.tcp.total_tcp_put_count , 2 , "" ) );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );
	//
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "tcp put byte" );
	snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , MAIN_STAT().round_zero_set.tcp.total_tcp_put_byte , 2 , "B" ) );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );


	// 5 sec
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "5s tcp pps" );
	snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , ( ubigint )cbuf_m_mean_all( &MAIN_STAT().round_init_set.tcp_stat_5_sec_count ) , 4 , "" ) );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );
	//
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "5s tcp bps" );
	snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , ( ubigint )cbuf_m_mean_all( &MAIN_STAT().round_init_set.tcp_stat_5_sec_bytes ) , 4 , "B" ) );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );


	//// 10 sec
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "10s tcp pps" );
	snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , ( ubigint )cbuf_m_mean_all( &MAIN_STAT().round_init_set.tcp_stat_10_sec_count ) , 4 , "" ) );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );
	//
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "10s tcp bps" );
	snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , ( ubigint )cbuf_m_mean_all( &MAIN_STAT().round_init_set.tcp_stat_10_sec_bytes ) , 4 , "B" ) );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );

	//// 40 sec
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "40s tcp pps" );
	snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , ( ubigint )cbuf_m_mean_all( &MAIN_STAT().round_init_set.tcp_stat_40_sec_count ) , 4 , "" ) );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );
	//
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "40s tcp bps" );
	snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , ( ubigint )cbuf_m_mean_all( &MAIN_STAT().round_init_set.tcp_stat_40_sec_bytes ) , 4 , "B" ) );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );

	//// 120 sec
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "120s tcp pps" );
	snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , ( ubigint )cbuf_m_mean_all( &MAIN_STAT().round_init_set.tcp_stat_120_sec_count ) , 4 , "" ) );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );
	//
	mvwprintw( MAIN_WIN , y , start_x , "|" );
	print_cell( MAIN_WIN , y , start_x + 1 , cell_w , "120s tcp bps" );
	snprintf( buf , sizeof( buf ) , "%s" , _FORMAT_SHRTFRM( buf2 , sizeof( buf2 ) , ( ubigint )cbuf_m_mean_all( &MAIN_STAT().round_init_set.tcp_stat_120_sec_bytes ) , 4 , "B" ) );
	mvwprintw( MAIN_WIN , y , start_x + cell_w + 1 , "|" );
	print_cell( MAIN_WIN , y , start_x + cell_w + 2 , cell_w , buf );
	mvwprintw( MAIN_WIN , y++ , start_x + 2 * cell_w + 2 , "|" );

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

_THREAD_FXN void_p stats_thread( void_p src_g )
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

	while ( 1 )
	{
		//if ( CLOSE_APP_VAR() ) break; // keep track changes until app is down

		pthread_mutex_lock( &_g->stat.lock_data.lock );

		werase( _g->stat.main_win );
		box( _g->stat.main_win , 0 , 0 );
		draw_table( _g );
		wrefresh( _g->stat.main_win );

		pthread_mutex_unlock( &_g->stat.lock_data.lock );

		sleep( STAT_REFERESH_INTERVAL_SEC() ); // OK 14040526
	}
	return NULL;
}

void init_windows( G * _g )
{
	//int maxy, maxx;
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

