#pragma once

typedef struct app_cmd
{
	int quit_app;
} Acmd;

typedef struct App_Data
{
	Acfg appcfg;
	ABhs bridges;
	St stat;
	Acmd cmd;
} G;

_THREAD_FXN void_p stdout_bypass_thread( void_p pdata );
_THREAD_FXN void_p sync_thread( void_p pdata ); // pause app until moment other app exist
_THREAD_FXN void_p input_thread( void_p pdata );
_THREAD_FXN void_p connect_udps_proc( void_p src_pb );
_THREAD_FXN void_p thread_tcp_connection_proc( void_p src_pb );
_THREAD_FXN void_p watchdog_executer( void_p src_g );

void init_bypass_stdout( G * _g );
void M_showMsg( const char * msg );
void init( G * _g );

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
void init_windows( G * _g );

void mng_basic_thread_sleep( G * _g , int priority );
