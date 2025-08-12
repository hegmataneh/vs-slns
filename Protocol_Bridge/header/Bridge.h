#pragma once

////struct protocol_bridge_thread
////{
////	pthread_t trd_id;
////	int base_config_change_applied;
////	int do_close_thread; // command from outside to inside thread
////
////	AB *pb; // point to upper wave
////};
////int under_listen_udp_sockets_group_changed;
////struct protocol_bridge_thread_holder
////{
////	struct protocol_bridge_thread * alc_thread; // one
////};

//

typedef struct AB_thread // threads use to recv and send data
{
	struct bridges_thread_base
	{
		int thread_is_created;
		int do_close_thread; // command from outside to inside thread
		pthread_mutex_t creation_thread_race_cond; // prevent multi bridge create thread concurrently

		int start_working; // because udp port may start after thread started
		pthread_mutex_t start_working_race_cond; // prevent multi bridge create thread concurrently
	} base;

	struct bottleneck_thread // one thread for send and receive
	{
		pthread_t trd_id;
	} *bottleneck_thread;

	struct bidirection_thread // one thread for each direction
	{
		struct bidirection_thread_zero_init_memory
		{
			pthread_t income_trd_id;
			pthread_t outgoing_trd_id;
		} mem;

		struct PacketQueue queue;
	} *bidirection_thread;

	struct justIncoming_thread // one thread for send and receive
	{
		pthread_t trd_id;
	} *justIncoming_thread;

} ABtrd;

//

typedef struct ActiveBridge // protocol_bridge . each bridge define one or many input and output that recev data from that input
{
	// caution . make copy of allocation values
	struct bridge_cfg ccfg;  // copy of applied cfg . active protocol_bridge config . یک دسته کامل از ترد ها یک کانفیگ را اعمال می کند

	struct AB_udps
	{
		int udp_sockfd;
		int udp_connection_established; // udp socket established
		int retry_to_connect_udp;
	} *udps;
	int udps_count;

	struct AB_tcps
	{
		int tcp_sockfd;
		int tcp_connection_established; // tcp connection established
		int retry_to_connect_tcp;
	} *tcps;
	int tcps_count;

	ABtrd trd;
	
} AB;

typedef struct AB_holder // every elemnt at the reallocation array must have holder because reallocate change data values but with holder just pointer addr change
{
	AB * single_AB; // single allocated
} ABh;

// TODO . IPv6
typedef struct AB_holders
{
	ABh * ABs; // all the active protocol_bridge or Active bridges
	int * ABhs_masks; // protocol_bridge masks
	size_t ABhs_masks_count; // mask count
} ABhs;


_THREAD_FXN void * bottleneck_thread_proc( void * src_g );
_THREAD_FXN void * income_thread_proc( void * src_g );
_THREAD_FXN void * outgoing_thread_proc( void * src_g );
_THREAD_FXN void * justIncoming_thread_proc( void * src_g );
_THREAD_FXN void * protocol_bridge_runner( void * src_pb );
