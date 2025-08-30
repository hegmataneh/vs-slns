#pragma once

typedef union AB_execution_tools
{
	//struct s_bottleneck_thread // one thread for send and receive
	//{
	//	pthread_t trd_id;
	//} *bottleneck_thread;
	//
	//struct s_bidirection_thread // one thread for each direction
	//{
	//	struct bidirection_thread_zero_init_memory
	//	{
	//		pthread_t income_trd_id;
	//		pthread_t outgoing_trd_id;
	//	} mem;
	//
	//	struct PacketQueue queue;
	//} *bidirection_thread;

	struct s_udp_counter_thread // one thread for send and receive
	{
		pthread_t trd_id;
	} *p_udp_counter_thread;

	struct s_pcap_udp_counter_thread // one thread for send and receive
	{
		pthread_t trd_id;
		pcap_t * handle;
	} *p_pcap_udp_counter_thread;

	struct s_one2one_pcap2kernelDefaultStack_SF_thread // one thread for each direction with store & forward method
	{
		pthread_t income_trd_id;
		pthread_t outgoing_trd_id;

		vcbuf_nb cbuf; // buffer

	} *p_one2one_pcap2kernelDefaultStack_SF_thread;

	struct s_one2many_pcap2kernelDefaultStack_SF_Mix_RR_Replicate // one thread for each direction with store & forward method
	{
		pthread_t income_trd_id;
		pthread_t outgoing_trd_id;

		vcbuf_nb cbuf; // buffer

		distributor_t buffer_pop_distributor; // responsible for distribute udp packet to each type
		dict_o_t dc_token_ring; // config may have multi rings and each one is for one grp

	} *p_one2many_pcap2kernelDefaultStack_SF_Mix_RR_Replicate;

} ex_tls;

typedef struct AB_thread // threads use to recv and send data
{
	struct s_bridges_thread_base
	{
		int thread_is_created;
		int do_close_thread; // command from outside to inside thread
		pthread_mutex_t creation_thread_race_cond; // prevent multi bridge create thread concurrently

		int bridg_prerequisite_stabled; // because udp port may start after thread started . if all the condition is ready to bridge thread start
		//pthread_mutex_t do_all_prerequisite_stablished_race_cond; // prevent multi bridge create thread concurrently

		distributor_t buffer_push_distributor; // responsible for distribute udp packet to each type
	} base;

	ex_tls t;

} ABtrd;

//

typedef struct packet_timing
{
	time_t prev_access , last_access;
	double curr_packet_delay , max_packet_delay;
} pck_tm;

struct ActiveBridge;

typedef struct AB_udp_connection
{
	sockfd udp_sockfd;
	int udp_connection_established; // udp socket established
	int retry_to_connect_udp;
	udp_cfg_pak * __udp_cfg_pak; // link to passive cfg
	struct ActiveBridge * owner_pb; // upper struct
	pcap_t * handle;

	pck_tm pk_tm; // to calculate elapse time between two arrival or send data

} AB_udp;

typedef struct AB_tcp_connection
{
	sockfd tcp_sockfd;
	int tcp_connection_established; // tcp connection established
	int retry_to_connect_tcp;
	tcp_cfg_pak * __tcp_cfg_pak; // link to passive cfg
	struct ActiveBridge * owner_pb; // upper struct

	pck_tm pk_tm; // to calculate elapse time between two arrival or send data

} AB_tcp;

typedef struct ActiveBridge // protocol_bridge . each bridge define one or many input and output that recev data from that input
{
	// caution . make copy of allocation values
	struct bridge_cfg cpy_cfg;  // copy of applied cfg . active protocol_bridge config . یک دسته کامل از ترد ها یک کانفیگ را اعمال می کند

	AB_udp *udps;
	int udps_count;

	AB_tcp *tcps;
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


typedef struct ActiveBridgeShortPathHelper // every virtually inherit struct must have this vars same as this struct without any difference
{
	AB * pab;
	int * in_count;
	int * out_count;
	int * thread_is_created;
	int * do_close_thread;
	pthread_mutex_t * creation_thread_race_cond;
	int * bridg_prerequisite_stabled;
	distributor_t * buf_psh_distri;

} abhelp;

void mk_hlpr0( _IN AB * pb , _OUT abhelp * hlpr );

//_THREAD_FXN void_p bottleneck_thread_proc( void_p src_g );
//_THREAD_FXN void_p income_thread_proc( void_p src_g );
//_THREAD_FXN void_p protocol_bridge_runner( void_p src_pb );
