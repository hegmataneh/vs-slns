#pragma once

typedef struct app_cmd
{
	int64 block_sending_1; // for nicely termination there should be flag that say no more sending
	volatile bool burst_waiting_2;
	int64 quit_first_level_thread_3;
	int64 quit_noloss_data_thread_4;
	int64 quit_app_4;
} Acmd;

typedef struct global_distributor
{
	distributor_t bcast_pre_cfg; //pre_configuration;
	distributor_t bcast_post_cfg; //post_config_stablished;
	distributor_t bcast_program_stabled; // stabled after config loaded and first bridges determind
	distributor_t bcast_thread_startup; //thread_startup; // every thread start up declare himself by this
	distributor_t bcast_app_lvl_failure; //app_lvl_failure_dist;
	distributor_t bcast_pb_lvl_failure; //pb_lvl_failure_dist;
	distributor_t bcast_pb_udp_connected; //pb_udp_connected_dist; // dispatch tcp connection state to increase counter
	distributor_t bcast_pb_udp_disconnected; //pb_udp_disconnected_dist;
	distributor_t bcast_pb_tcp_connected; //pb_tcp_connected_dist; // dispatch tcp connection state to increase counter
	distributor_t bcast_pb_tcp_disconnected; //pb_tcp_disconnected_dist;
	distributor_t bcast_quit; // quit_interrupt_dist; // quit interrupt dispatch to all pcap loop

	#ifdef HAS_STATISTICSS
	distributor_t throttling_refresh_stat; // refresh stat intervally
	distributor_t init_static_table; // table that is static with content . for now without AB
	#endif
} g_dst;

typedef enum
{
	gws_die = -1 ,
	gws_close = 0,
	gws_open = 1
} gateway_open_stat;

typedef struct global_handles
{
	struct
	{
		pkt_mgr_t pkt_mgr; // packet manager . receive from pcap ring and add to huge double circular linked list
		prst_csh_t prst_csh; // persistent cache manager . 
		struct
		{
			sem_t pagestack_gateway_open_sem; // prevent cpu burne
			gateway_open_stat pagestack_gateway_open_val; // assist gateway status . also -1 means close persistent mngr
		} gateway;
	};

	struct
	{
		pthread_mutex_t thread_close_mtx; // work with registered_thread
		dyn_arr registered_thread;
	};

} g_hdl;

typedef struct global_thread_handles
{
	pthread_t trd_watchdog;
	pthread_t tid_stats , tid_input;
	pthread_t trd_version_checker;
	pthread_t trd_config_loader;
	pthread_t trd_config_executer;
} g_trds;


typedef struct App_Data
{
	Acfg appcfg;
	ABhs bridges;
	Stt stat;

	struct
	{
		Acmd cmd;
		g_dst distributors;
		g_hdl hdls; // holders
		g_trds trds; // threads
	};
} G;

_THREAD_FXN void_p stdout_bypass_thread( pass_p src_g );
_THREAD_FXN void_p sync_thread( pass_p src_g ); // pause app until moment other app exist
_THREAD_FXN void_p input_thread( pass_p src_g );
_THREAD_FXN void_p connect_udps_proc( pass_p src_pb );
_THREAD_FXN void_p thread_tcp_connection_proc( pass_p src_pb );
_THREAD_FXN void_p watchdog_executer( pass_p src_g );

void init_bypass_stdout( G * _g );
void M_showMsg( LPCSTR msg );

////int _connect_tcp( AB * pb );
////status connect_one_tcp( AB_tcp * tcp );
_REGULAR_FXN void compile_udps_config_for_pcap_filter( _IN AB * abs , _RET_VAL_P int * clusterd_cnt , _NEW_OUT_P strings * interface_filter , _NEW_OUT_P strings * port_filter );

// because of recursive dependency declration come here
void apply_new_protocol_bridge_config( G * _g , AB * pb , brg_cfg_t * new_ccfg );
void stop_protocol_bridge( G * _g , AB * pb );
void apply_protocol_bridge_new_cfg_changes( G * _g , brg_cfg_t * prev_pcfg , brg_cfg_t * new_ccfg );
void remove_protocol_bridge( G * _g , brg_cfg_t * pcfg );
void add_new_protocol_bridge( G * _g , brg_cfg_t * new_ccfg );

void mng_basic_thread_sleep( G * _g , int priority );

_CALLBACK_FXN void quit_interrupt( int sig );
_CALLBACK_FXN void app_err_dist( pass_p src_g , LPCSTR msg );
_CALLBACK_FXN void pb_err_dist( pass_p src_pb , LPCSTR msg );
_CALLBACK_FXN void udp_connected( pass_p src_pb , long v );
_CALLBACK_FXN void udp_disconnected( pass_p src_pb , long v );
_CALLBACK_FXN void tcp_connected( pass_p src_AB_tcp , long fd );
_CALLBACK_FXN void tcp_disconnected( pass_p src_pb , long v );

_CALLBACK_FXN void thread_goes_out_of_scope( void *ptr );

#ifndef update_cell_section

// because of recursive dependency declration come here
void draw_table( G * _g );

_CALLBACK_FXN PASSED_CSTR ov_cell_time_2_str( pass_p src_pcell );
_CALLBACK_FXN PASSED_CSTR ov_cell_version_2_str( pass_p src_pcell );
_CALLBACK_FXN PASSED_CSTR ov_UDP_conn_2_str( pass_p src_pcell );
_CALLBACK_FXN PASSED_CSTR ov_TCP_conn_2_str( pass_p src_pcell );
_CALLBACK_FXN PASSED_CSTR ov_UDP_retry_2_str( pass_p src_pcell );
_CALLBACK_FXN PASSED_CSTR ov_TCP_retry_2_str( pass_p src_pcell );
_CALLBACK_FXN PASSED_CSTR ov_fault_2_str( pass_p src_pcell );
_CALLBACK_FXN PASSED_CSTR ov_time_elapse_2_str( pass_p src_pcell );

_CALLBACK_FXN PASSED_CSTR ov_thread_cnt_2_str( pass_p src_pcell );

_CALLBACK_FXN PASSED_CSTR pb_time_elapse_2_str( pass_p src_pcell );
_CALLBACK_FXN PASSED_CSTR pb_fault_2_str( pass_p src_pcell );
_CALLBACK_FXN PASSED_CSTR pb_fst_cash_lost_2_str( pass_p src_pcell );
_CALLBACK_FXN PASSED_CSTR pb_UDP_conn_2_str( pass_p src_pcell );
_CALLBACK_FXN PASSED_CSTR pb_TCP_conn_2_str( pass_p src_pcell );
_CALLBACK_FXN PASSED_CSTR pb_UDP_retry_2_str( pass_p src_pcell );
_CALLBACK_FXN PASSED_CSTR pb_TCP_retry_2_str( pass_p src_pcell );

_CALLBACK_FXN PASSED_CSTR pb_UDP_get_count_2_str( pass_p src_pcell );
_CALLBACK_FXN PASSED_CSTR pb_UDP_get_byte_2_str( pass_p src_pcell );
_CALLBACK_FXN PASSED_CSTR pb_TCP_put_count_2_str( pass_p src_pcell );
_CALLBACK_FXN PASSED_CSTR pb_TCP_put_byte_2_str( pass_p src_pcell );

#ifdef ENABLE_THROUGHPUT_MEASURE
_CALLBACK_FXN PASSED_CSTR pb_5s_udp_pps_2_str( pass_p src_pcell );
_CALLBACK_FXN PASSED_CSTR pb_5s_udp_bps_2_str( pass_p src_pcell );
_CALLBACK_FXN PASSED_CSTR pb_10s_udp_pps_2_str( pass_p src_pcell );
_CALLBACK_FXN PASSED_CSTR pb_10s_udp_bps_2_str( pass_p src_pcell );
_CALLBACK_FXN PASSED_CSTR pb_40s_udp_pps_2_str( pass_p src_pcell );
_CALLBACK_FXN PASSED_CSTR pb_40s_udp_bps_2_str( pass_p src_pcell );

_CALLBACK_FXN PASSED_CSTR pb_5s_tcp_pps_2_str( pass_p src_pcell );
_CALLBACK_FXN PASSED_CSTR pb_5s_tcp_bps_2_str( pass_p src_pcell );
_CALLBACK_FXN PASSED_CSTR pb_10s_tcp_pps_2_str( pass_p src_pcell );
_CALLBACK_FXN PASSED_CSTR pb_10s_tcp_bps_2_str( pass_p src_pcell );
_CALLBACK_FXN PASSED_CSTR pb_40s_tcp_pps_2_str( pass_p src_pcell );
_CALLBACK_FXN PASSED_CSTR pb_40s_tcp_bps_2_str( pass_p src_pcell );

#endif
#endif
