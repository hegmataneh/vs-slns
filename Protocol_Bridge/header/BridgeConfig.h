#pragma once

typedef struct udp_conn_cfg_data
{
	CFG_ITM group;
	CFG_ITM group_type;
	CFG_ITM connection_type;
	CFG_ITM connection_param;

	struct
	{
		CFG_ITM UDP_origin_ip;
		CFG_ITM UDP_destination_ip;
		CFG_ITM UDP_origin_ports; // singular port or ports ranje
		CFG_ITM UDP_origin_interface;
	} core;
	int enable;
	int reset_connection;

} udp_cfg_t;

typedef struct tcp_conn_cfg_data
{
	CFG_ITM group;
	CFG_ITM group_type;
	CFG_ITM connection_type;
	CFG_ITM connection_param;

	struct
	{
		CFG_ITM TCP_destination_ip;
		CFG_ITM TCP_destination_ports; // singular port or ports ranje
		CFG_ITM TCP_destination_interface;

	} core;
	int enable;
	int reset_connection;
	CFG_ITM post_action;
	int send_gap_nsec;
	int send_throughput_window_sz;
	int send_throughput_limit_Bps;

	char elastic_username[DEFAULT_SFS_BUF_SZ];
	char elastic_pass[DEFAULT_SFS_BUF_SZ];
	char elastic_http_agt_CAuth_path[PATH_MAX];
	char elastic_doc_index_name[DEFAULT_MFS_BUF_SZ];
	char elastic_insertion_protocol[DEFAULT_BT_BUF_SZ];
	char elastic_insertion_cmd[DEFAULT_SFS_BUF_SZ];

} tcp_cfg_t;

typedef struct bridge_cfg_input_part
{
	CFG_ITM name;
	udp_cfg_t data;

} udp_cfg_pak_t;

typedef struct bridge_cfg_output_part
{
	CFG_ITM name;
	tcp_cfg_t data;

} tcp_cfg_pak_t;

typedef struct bridge_cfg_0
{
	struct bridge_cfg_id
	{
		CFG_ITM bridge_name;
		CFG_ITM short_name;

		CFG_ITM out_type;
		CFG_ITM thread_handler_act;

	} id; // protocol_bridge_cfg_id . must be uniq for each bridge

	struct bridge_cascade_maintable_parameter // options that stays in position
	{
		udp_cfg_pak_t * in;
		size_t in_count;

		tcp_cfg_pak_t * out;
		size_t out_count;
	} cas_maintained;

	struct bridge_solid_maintained_parameter // options that stays in position
	{
		int enable;
		int hide;
		int issue_list_timeout_sec;
		int issue_list_count;

	} sol_maintained;

	struct bridge_temp_data
	{
		void_p _pseudo_g; // just point to the main g . just because double source dependencies it define as void_p
		int pcfg_changed; // in passive cfg and active cfg that in alive protocol_bridge, in both it means something changed
		Boolean delayed_validation; // when it is True it means structure copying complete

	} temp_data;

} Bcfg0; // protocol_bridge_cfg

#define TO_G( voidp ) (( G* )voidp )

typedef struct bridge_cfg_n
{
	Bcfg0 m; // first member , n - 1

} Bcfgn;

typedef struct bridge_cfg // finalizer . protocol_bridge_cfg
{
	Bcfgn m; // be first member

} brg_cfg_t;

void copy_bridge_cfg( brg_cfg_t * dst , brg_cfg_t * src );

int Bcfg0_id_equlity( Bcfg0 * left , Bcfg0 * right );
int Bcfg_id_equlity( brg_cfg_t * left , brg_cfg_t * right );

int bridge_cfg0_data_equlity( Bcfg0 * left , Bcfg0 * right );
int bridge_cfg_data_equlity( brg_cfg_t * left , brg_cfg_t * right );

bool tcp_core_config_equlity( tcp_cfg_pak_t * cfg1 , tcp_cfg_pak_t * cfg2 );

void cleaup_brg_cfg( brg_cfg_t * brg );
