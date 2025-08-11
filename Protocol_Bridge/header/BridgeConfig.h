#pragma once

typedef struct bridge_cfg_0
{
	struct bridge_cfg_id
	{
		char bridge_name[ 64 ];

		char out_type[ 64 ];
		char thread_handler_act[ 64 ];
	} id; // protocol_bridge_cfg_id . must be uniq for each bridge

	struct bridge_maintained_parameter // options that stays in position
	{
		struct bridge_cfg_input_part
		{
			char group[ 64 ];
			char group_type[ 64 ];
			char UDP_origin_ip[ 64 ];
			int UDP_origin_port;
			char UDP_origin_interface[ 64 ];
			int enable;
			int reset_connection;
		} *in;
		int in_count;

		struct bridge_cfg_output_part
		{
			char group[ 64 ];
			char group_type[ 64 ];
			char TCP_destination_ip[ 64 ];
			int TCP_destination_port;
			char TCP_destination_interface[ 64 ];
			int enable;
			int reset_connection;
		} *out;
		int out_count;

		int enable;
	} maintained;

	struct bridge_momentary_parameter // popup automatically
	{
		int reset_connections;
	} momentary;

	struct bridge_temp_data
	{
		void * _g; // just point to the main g
		int pcfg_changed; // in passive cfg and active cfg that in alive protocol_bridge, in both it means something changed
	} temp_data;

} Bcfg0; // protocol_bridge_cfg

typedef struct bridge_cfg_n
{
	Bcfg0 m; // first member , n - 1
} Bcfgn;

typedef struct bridge_cfg // finalizer . protocol_bridge_cfg
{
	Bcfgn m; // be first member
} Bcfg;

void copy_bridge_cfg( Bcfg * dst , Bcfg * src );
