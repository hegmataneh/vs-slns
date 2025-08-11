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

_THREAD_FXN void * stdout_bypass_thread( void * pdata );
void init_bypass_stdout( G * _g );
void M_showMsg( const char * msg );
void init( G * _g );
_THREAD_FXN void * sync_thread( void * pdata ); // pause app until moment other app exist
_THREAD_FXN void * input_thread( void * pdata );
_THREAD_FXN void * thread_udp_connection_proc( void * src_pb );
int _connect_tcp( AB * pb );
_THREAD_FXN void * thread_tcp_connection_proc( void * src_pb );

// because of recursive dependency declration come here
void apply_new_protocol_bridge_config( G * _g , AB * pb , Bcfg * new_ccfg );
void stop_protocol_bridge( G * _g , AB * pb );
void apply_protocol_bridge_new_cfg_changes( G * _g , Bcfg * prev_pcfg , Bcfg * new_ccfg );
void remove_protocol_bridge( G * _g , Bcfg * pcfg );
void add_new_protocol_bridge( G * _g , Bcfg * new_ccfg );

// because of recursive dependency declration come here
void draw_table( G * _g );
void init_windows( G * _g );
