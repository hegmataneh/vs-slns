#pragma once

typedef struct udp_conn_cfg_data
{
	CFG_ITM group;
	CFG_ITM group_type;
	CFG_ITM UDP_origin_ip;
	CFG_ITM UDP_origin_ports; // singular port or ports ranje
	CFG_ITM UDP_origin_interface;
	int enable;
	int reset_connection;
} udp_cfg;

typedef struct tcp_conn_cfg_data
{
	CFG_ITM group;
	CFG_ITM group_type;
	CFG_ITM TCP_destination_ip;
	CFG_ITM TCP_destination_ports; // singular port or ports ranje
	CFG_ITM TCP_destination_interface;
	int enable;
	int reset_connection;
} tcp_cfg;

typedef struct bridge_cfg_input_part
{
	CFG_ITM name;
	udp_cfg data;

} udp_cfg_pak;

typedef struct bridge_cfg_output_part
{
	CFG_ITM name;
	tcp_cfg data;

} tcp_cfg_pak;

typedef struct bridge_cfg_0
{
	struct bridge_cfg_id
	{
		CFG_ITM bridge_name;

		CFG_ITM out_type;
		CFG_ITM thread_handler_act;
	} id; // protocol_bridge_cfg_id . must be uniq for each bridge

	struct bridge_maintained_parameter // options that stays in position
	{
		udp_cfg_pak *in;
		int in_count;

		tcp_cfg_pak *out;
		int out_count;

		int enable;
		int hide;
	} maintained;

	struct bridge_temp_data
	{
		void_p _g; // just point to the main g . just because double source dependencies it define as void_p
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
} Bcfg;

void copy_bridge_cfg( Bcfg * dst , Bcfg * src );

int Bcfg0_id_equlity( Bcfg0 * left , Bcfg0 * right );
int Bcfg_id_equlity( Bcfg * left , Bcfg * right );

int bridge_cfg0_data_equlity( Bcfg0 * left , Bcfg0 * right );
int bridge_cfg_data_equlity( Bcfg * left , Bcfg * right );
