#pragma once

#ifndef AB_base_stat

struct udp_stat_1_sec
{
	time_t t_udp_throughput;

	__int64u calc_throughput_udp_get_count;
	__int64u calc_throughput_udp_get_bytes;

	//__int64u udp_get_count_throughput;
	//__int64u udp_get_byte_throughput;
};

struct udp_stat
{
	__int64u total_udp_get_count;
	__int64u total_udp_get_byte;

	__int64u continuously_unsuccessful_select_on_open_port_count; // that canot get find data
};

struct tcp_stat_1_sec
{
	time_t t_tcp_throughput;

	__int64u calc_throughput_tcp_put_count;
	__int64u calc_throughput_tcp_put_bytes;

	//__int64u tcp_put_count_throughput;
	//__int64u tcp_put_byte_throughput;
};

struct tcp_stat
{
	__int64u total_tcp_put_count;
	__int64u total_tcp_put_byte;
};

#ifdef ENABLE_THROUGHPUT_MEASURE
struct BenchmarkRound_initable_memory // must be init with own function
{
	cbuf_metr udp_stat_5_sec_count , udp_stat_5_sec_bytes;
	cbuf_metr udp_stat_10_sec_count , udp_stat_10_sec_bytes;
	cbuf_metr udp_stat_40_sec_count , udp_stat_40_sec_bytes;

	cbuf_metr tcp_stat_5_sec_count , tcp_stat_5_sec_bytes;
	cbuf_metr tcp_stat_10_sec_count , tcp_stat_10_sec_bytes;
	cbuf_metr tcp_stat_40_sec_count , tcp_stat_40_sec_bytes;
};
#endif

struct BenchmarkRound_zero_init_memory // can be memset to zero all byte
{
	struct timeval t_begin , t_end; // begin and end of on iteration of benchmarking

	int continuously_unsuccessful_receive_error;
	int total_unsuccessful_receive_error;

	int continuously_unsuccessful_send_error;
	int total_unsuccessful_send_error;

	__int64u pb_fault_count;

	struct udp_stat_1_sec udp_1_sec;
	struct tcp_stat_1_sec tcp_1_sec;

	struct udp_stat udp;
	struct tcp_stat tcp;

	int udp_connection_count;
	int tcp_connection_count;
	int total_retry_udp_connection_count;
	int total_retry_tcp_connection_count;

	int udp_get_data_alive_indicator;
	int tcp_send_data_alive_indicator;
};

typedef struct s_bridge_stat
{
	struct BenchmarkRound_zero_init_memory  round_zero_set;
	#ifdef ENABLE_THROUGHPUT_MEASURE
	struct BenchmarkRound_initable_memory  round_init_set;
	#endif

	#ifdef HAS_STATISTICSS
	nnc_cell_content * pb_elapse_cell;

	nnc_cell_content * pb_fault_cell;
	nnc_cell_content * pb_fst_cash_lost;
	nnc_cell_content * pb_UDP_conn_cell;
	nnc_cell_content * pb_TCP_conn_cell;
	nnc_cell_content * pb_UDP_retry_conn_cell;
	nnc_cell_content * pb_TCP_retry_conn_cell;

	nnc_cell_content * pb_total_udp_get_count_cell;
	nnc_cell_content * pb_total_udp_get_byte_cell;
	nnc_cell_content * pb_total_tcp_put_count_cell;
	nnc_cell_content * pb_total_tcp_put_byte_cell;

	#ifdef ENABLE_THROUGHPUT_MEASURE
	nnc_cell_content * pb_5s_udp_pps;
	nnc_cell_content * pb_5s_udp_bps;
	nnc_cell_content * pb_10s_udp_pps;
	nnc_cell_content * pb_10s_udp_bps;
	nnc_cell_content * pb_40s_udp_pps;
	nnc_cell_content * pb_40s_udp_bps;

	nnc_cell_content * pb_5s_tcp_pps;
	nnc_cell_content * pb_5s_tcp_bps;
	nnc_cell_content * pb_10s_tcp_pps;
	nnc_cell_content * pb_10s_tcp_bps;
	nnc_cell_content * pb_40s_tcp_pps;
	nnc_cell_content * pb_40s_tcp_bps;
	#endif
	#endif

} ABstat;

#endif

#ifndef main_section

//struct statistics_lock_data
//{
//	pthread_mutex_t lock;
//};

typedef struct // can be memset to zero all byte
{
	struct timeval t_begin /* , t_end*/; // begin and end of on iteration of benchmarking
	__int64u app_fault_count;

	int total_udp_connection_count;
	int total_tcp_connection_count;
	int total_retry_udp_connection_count;
	int total_retry_tcp_connection_count;
} stat_zero_init_memory;

typedef struct
{
	IMMORTAL_LPCSTR thread_name; /*i use __FUNCTION__*/
	time_t alive_time;
	pthread_t thread_id;
} thread_alive_indicator;

typedef struct notcurses_stat_req
{
#ifdef HAS_STATISTICSS
	nnc_table * pgeneral_tbl; // general overview page. add gere for additional field addition

	SHARED_MEM dyn_mms_arr field_keeper; // nnc_cell_content . one block keep array of field that dynamically changed. prevent memory fragment
	//kv_table_t map_flds; // make access to field faster

	// fastest way to access important cell
	nnc_cell_content * ov_time_cell; // shortcut access to most frequently cell
	nnc_cell_content * ov_ver_cell;
	nnc_cell_content * ov_elapse_cell;
	nnc_cell_content * ov_fault_cell;
	nnc_cell_content * ov_UDP_conn_cell;
	nnc_cell_content * ov_TCP_conn_cell;
	nnc_cell_content * ov_UDP_retry_conn_cell;
	nnc_cell_content * ov_TCP_retry_conn_cell;
	nnc_cell_content * ov_thread_cnt_cell;

	struct
	{
		SHARED_MEM dyn_mms_arr thread_list; // thread_alive_indicator . use to check they are alive
		pthread_mutex_t thread_list_mtx; /*use just on thread_list*/
	};

#endif
} n_s_req;

typedef struct statistics
{
	//// box & window
	//int scr_height , scr_width;
	//WINDOW * main_win;
	//WINDOW * input_win;
	
	#ifdef HAS_STATISTICSS
	SHARED_MEM nnc_req nc_h; // notcurses handle
	n_s_req nc_s_req; // data viewed in not curses
	#endif

	stat_zero_init_memory aggregate_stat;

	// cmd
	int pipefds[ 2 ]; // used for bypass stdout
	//char last_command[ INPUT_MAX ];
	char input_buffer[ INPUT_MAX ];
	int last_line_meet;
	int alive_check_counter;
	//struct statistics_lock_data lock_data;

} Stt;

//void reset_nonuse_stat();
//void print_cell( WINDOW * win , int y , int x , int width , LPCSTR text );
_THREAD_FXN void_p stats_thread( pass_p pdata );

//void init_ncursor();

_CALLBACK_FXN void init_main_statistics( pass_p src_g );
_CALLBACK_FXN void thread_overviewing( pass_p src_g );

#endif
