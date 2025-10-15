#pragma once

typedef struct AB_handler_prerequisite
{

	struct s_pcap_udp_counter // one thread for send and receive
	{
		pthread_t trd_id;
		pcap_t * pcp_handle;
	} *p_pcap_udp_counter;

	struct s_krnl_udp_counter
	{
		pthread_t trd_id;
	} *p_krnl_udp_counter;

	struct s_one2one_krnl2krnl_SF // one thread for each direction with store & forward method . with kernel utils
	{
		pthread_t income_trd_id;
		pthread_t outgoing_trd_id;
	} *p_one2one_krnl2krnl_SF;

	struct s_one2one_pcap2krnl_SF // one thread for each direction with store & forward method
	{
		pthread_t income_trd_id;
		pthread_t outgoing_trd_id;
		pcap_t * pcp_handle;
	} *p_one2one_pcap2krnl_SF;

	struct s_one2many_pcap2krnl_SF // one thread for each direction with store & forward method
	{
		pthread_t income_trd_id;
		pthread_t outgoing_trd_id;
		dict_o_t dc_token_ring; // config may have multi rings and each one is for one grp
		pcap_t * pcp_handle;
	} *p_one2many_pcap2krnl_SF;

	struct s_many2one_pcap2krnl_SF_serialize // one thread for each direction with store & forward method
	{
		pthread_t income_trd_id;
		pthread_t outgoing_trd_id;
		dict_o_t dc_token_ring; // config may have multi rings and each one is for one grp
	} *p_many2one_pcap2krnl_SF_serialize;

} ex_preq;

typedef struct AB_thread // threads use to recv and send data
{
	struct AB_common_prerequisite // TODO . aware for cache line alignment
	{
		
		union
		{
			struct
			{
				bool stop_receiving; // command from outside to inside thread
				bool receive_stoped;
			};
			uchar pad1[64];
		};
		union
		{
			struct
			{
				bool stop_sending;
				bool send_stoped;
			};
			uchar pad2[ 64 ];
		};
		int thread_is_created;
		//int bridg_prerequisite_stabled; // because udp port may start after thread started . if all the condition is ready to bridge thread start

		cbuf_pked fast_wrt_cache; // ring buffer of input udp . why i use packed buffer . because each pcap has one ring and it consume lot of memory to keep pesimistic block( consider 8k for each pkt )
		defraged_udps_t defraged_udps;

		distributor_t fragmented_udp_packet_on_pcap_received_event; // used when raw socket worked
		distributor_t kernel_udp_payload_ready_event; // just complete payload pushed( i insist on payload concept not buffer litteraly )
		distributor_t defraged_pcap_udp_payload_event; // just payload poped( i insist on payload concept not buffer litteraly )
	} cmn;

	ex_preq t;

} ABtrd;

//

struct ActiveBridge;

typedef struct AB_udp_connection
{
	sockfd udp_sockfd;
	int udp_connection_established; // udp socket established
	int retry_to_connect_udp;
	udp_cfg_pak_t * __udp_cfg_pak; // link to passive cfg
	struct ActiveBridge * owner_pb; // upper struct

	//distributor_t change_state_dist; // not needed

} AB_udp;

typedef struct AB_tcp_connection
{
	sockfd tcp_sockfd;
	int tcp_connection_established; // tcp connection established
	
	//int retry_to_connect_tcp; // do retry to connect again
	int tcp_is_about_to_connect; // when tring to connect no more try should be attempt


	tcp_cfg_pak_t * __tcp_cfg_pak; // link to passive cfg
	struct ActiveBridge * owner_pb; // upper struct

	distributor_t change_state_dist;

} AB_tcp;

typedef struct ActiveBridge // protocol_bridge . each bridge define one or many input and output that recev data from that input
{
	// caution . make copy of allocation values
	brg_cfg_t cpy_cfg;  // copy of applied cfg . active protocol_bridge config . یک دسته کامل از ترد ها یک کانفیگ را اعمال می کند
	ABtrd trd;
	ABstat stat;

	AB_udp *udps;
	int udps_count;

	AB_tcp *tcps;
	int tcps_count;

	nnc_table * ab_stat_tbl;
	
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
	//int * do_close_thread;

	//int * bridg_prerequisite_stabled;
	distributor_t * buf_psh_distri;

	cbuf_pked * fast_wrt_cache; // buffer
	distributor_t * defrg_pcap_payload;
	dict_o_t * dc_token_ring;
	pcap_t ** pcp_handle; // address to pcap handler

} shrt_path;

void mk_shrt_path( _IN AB * pb , _RET_VAL_P shrt_path * hlpr );

_CALLBACK_FXN void stop_sending_by_bridge( pass_p src_g , long v );
