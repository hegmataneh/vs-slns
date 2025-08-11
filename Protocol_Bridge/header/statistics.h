#pragma once

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

struct statistics_lock_data
{
	pthread_mutex_t lock;
};


struct BenchmarkRound_initable_memory // must be init with own function
{
	struct circbuf_t udp_stat_5_sec_count , udp_stat_5_sec_bytes;
	struct circbuf_t udp_stat_10_sec_count , udp_stat_10_sec_bytes;
	struct circbuf_t udp_stat_40_sec_count , udp_stat_40_sec_bytes;
	struct circbuf_t udp_stat_120_sec_count , udp_stat_120_sec_bytes;

	struct circbuf_t tcp_stat_5_sec_count , tcp_stat_5_sec_bytes;
	struct circbuf_t tcp_stat_10_sec_count , tcp_stat_10_sec_bytes;
	struct circbuf_t tcp_stat_40_sec_count , tcp_stat_40_sec_bytes;
	struct circbuf_t tcp_stat_120_sec_count , tcp_stat_120_sec_bytes;
};

struct BenchmarkRound_zero_init_memory // can be memset to zero all byte
{
	struct timeval t_begin , t_end; // begin and end of on iteration of benchmarking

	int continuously_unsuccessful_receive_error;
	int total_unsuccessful_receive_error;

	int continuously_unsuccessful_send_error;
	int total_unsuccessful_send_error;

	__int64u syscal_err_count;

	struct udp_stat_1_sec udp_1_sec;
	struct tcp_stat_1_sec tcp_1_sec;

	//struct udp_stat_10_sec stat_10_sec;
	//struct udp_stat_40_sec stat_40_sec;

	struct udp_stat udp;
	struct tcp_stat tcp;
};

typedef struct statistics
{
	// box & window
	int scr_height , scr_width;
	WINDOW * main_win;
	WINDOW * input_win;

	// cmd
	int pipefds[ 2 ]; // used for bypass stdout
	char last_command[ INPUT_MAX ];
	char input_buffer[ INPUT_MAX ];
	int last_line_meet;
	int alive_check_counter;
	struct statistics_lock_data lock_data;

	int udp_connection_count;
	int tcp_connection_count;
	int total_retry_udp_connection_count;
	int total_retry_tcp_connection_count;

	struct BenchmarkRound_zero_init_memory  round_zero_set;
	struct BenchmarkRound_initable_memory  round_init_set;
} St;

void reset_nonuse_stat();
void print_cell( WINDOW * win , int y , int x , int width , const char * text );
_THREAD_FXN void * stats_thread( void * pdata );