#pragma once

typedef struct app_cmd
{
	int quit_app;
} Acmd;

typedef struct global_distributor
{
	distributor_t pb_err_dist; // occure in AB
	distributor_t ground_err_dist; // anywhere else

	distributor_t pb_udp_connected_dist; // dispatch tcp connection state to increase counter
	distributor_t pb_udp_disconnected_dist;

	distributor_t pb_tcp_connected_dist; // dispatch tcp connection state to increase counter
	distributor_t pb_tcp_disconnected_dist;

	distributor_t quit_interrupt_dist; // quit interrupt dispatch to all pcap loop

} g_dst;

typedef struct global_handles
{
	//pcap_t * pcap_udp_counter_handle;
	kv_table_t map_tcp_socket; // keep mapping between tcp &id
} g_hdl;

typedef struct global_buffer_handles
{
	ci_sgmgr_t aggr_inp_pkt;
} g_bufs;

typedef struct global_thread_handles
{
	pthread_t trd_watchdog;
	pthread_t tid_stats , tid_input;
	pthread_t trd_version_checker;
	pthread_t trd_config_loader;
	pthread_t trd_config_executer;

	pthread_t trd_tcp_sender; // get filled segment and send them
} g_trds;


typedef struct App_Data
{
	Acfg appcfg;
	ABhs bridges;
	St stat;

	nnc_req nnc;

	Acmd cmd;
	g_dst distrbtor;
	g_hdl hdls;
	g_bufs bufs;
	g_trds trds;
} G;

_THREAD_FXN void_p stdout_bypass_thread( pass_p src_g );
_THREAD_FXN void_p sync_thread( pass_p src_g ); // pause app until moment other app exist
_THREAD_FXN void_p input_thread( pass_p src_g );
_THREAD_FXN void_p connect_udps_proc( pass_p src_pb );
_THREAD_FXN void_p thread_tcp_connection_proc( pass_p src_pb );
_THREAD_FXN void_p watchdog_executer( pass_p src_g );

void init_bypass_stdout( G * _g );
void M_showMsg( LPCSTR msg );

void init_notcursor( G * _g );
void init_tui( G * _g );

void pre_config_init( G * _g );
void post_config_init( G * _g );

//int _connect_tcp( AB * pb );
status connect_one_tcp( AB_tcp * tcp );


// because of recursive dependency declration come here
void apply_new_protocol_bridge_config( G * _g , AB * pb , Bcfg * new_ccfg );
void stop_protocol_bridge( G * _g , AB * pb );
void apply_protocol_bridge_new_cfg_changes( G * _g , Bcfg * prev_pcfg , Bcfg * new_ccfg );
void remove_protocol_bridge( G * _g , Bcfg * pcfg );
void add_new_protocol_bridge( G * _g , Bcfg * new_ccfg );

// because of recursive dependency declration come here
void draw_table( G * _g );

void mng_basic_thread_sleep( G * _g , int priority );

_REGULAR_FXN void compile_udps_config_for_pcap_filter( _IN AB * abs , _RET_VAL_P int * clusterd_cnt , _NEW_OUT_P strings * interface_filter , _NEW_OUT_P strings * port_filter );

