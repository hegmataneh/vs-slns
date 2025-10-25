#pragma once

typedef struct AB_thread_action_handler
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
		//pthread_t outgoing_trd_id;
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


	struct s_one2many_krnl2krnl_SF // one thread for each direction with store & forward method
	{
		pthread_t income_trd_id;
		//pthread_t outgoing_trd_id;
		dict_o_t dc_token_ring; // config may have multi rings and each one is for one grp
	} *p_one2many_krnl2krnl_SF;

} trd_act_hdls;

typedef struct AB_communication // threads use to recv and send data
{
	struct /*AB_common_prerequisite*/ // TODO . aware for cache line alignment
	{
		union
		{
			struct
			{
				bool stop_receiving; // command from outside to inside thread
				bool receive_stoped;
			};
			uchar pad1[ CACHE_LINE_SIZE ];
		};
		union
		{
			struct
			{
				bool stop_sending;
				bool send_stoped;
			};
			uchar pad2[ CACHE_LINE_SIZE ];
		};
		int thread_is_created;
		//int bridg_prerequisite_stabled; // because udp port may start after thread started . if all the condition is ready to bridge thread start

		struct
		{
			cbuf_pked_t raw_xudp_cache; //fast_wrt_cache; // ring buffer of input udp . why i use packed buffer . because each pcap has one ring and it consume lot of memory to keep pesimistic block( consider 8k for each pkt )
			defraged_udps_t defraged_udps; // 
		};

		distributor_t bcast_pcap_udp_pkt; // used when raw socket worked
		distributor_t bcast_xudp_pkt; // just payload poped( i insist on payload concept not buffer litteraly )
	} preq; /*cmn,comn*/

	trd_act_hdls acts; /*t*/

} ABcomm;

//

struct ActiveBridge;

typedef struct AB_udp_connection
{
	sockfd udp_sockfd;
	int udp_connection_established; // udp socket established
	int retry_to_connect_udp;
	udp_cfg_pak_t * __udp_cfg_pak; // link to passive cfg
	struct ActiveBridge * owner_pb; // upper struct

	distributor_t bcast_change_state; // not needed

} AB_udp;

typedef struct AB_tcp_connection
{
	sockfd tcp_sockfd;
	int tcp_connection_established; // tcp connection established
	
	//int retry_to_connect_tcp; // do retry to connect again
	int tcp_is_about_to_connect; // when tring to connect no more try should be attempt

	time_t last_access; // last time any pkt sent

	tcp_cfg_pak_t * __tcp_cfg_pak; // link to passive cfg
	struct ActiveBridge * owner_pb; // upper struct

	distributor_t bcast_change_state;

} AB_tcp;

typedef struct ActiveBridge // protocol_bridge . each bridge define one or many input and output that recev data from that input
{
	// caution . make copy of allocation values
	brg_cfg_t cpy_cfg;  // copy of applied cfg . active protocol_bridge config . یک دسته کامل از ترد ها یک کانفیگ را اعمال می کند
	ABcomm comm; /*trd*/
	ABstat stat;

	AB_udp *udps;
	int udps_count;

	AB_tcp *tcps;
	int tcps_count;

	#ifdef HAS_STATISTICSS
	nnc_table * ab_stat_tbl;
	#endif
	
} AB;

// TODO . IPv6
typedef struct AB_holders
{
	dyn_mms_arr ABs; // all the active protocol_bridge or Active bridges

} ABhs;


typedef struct /*ActiveBridgeShortPathHelper*/ // every virtually inherit struct must have this vars same as this struct without any difference
{
	AB * pab;
	int * in_count;
	int * out_count;
	int * thread_is_created;

	cbuf_pked_t * raw_xudp_cache; // buffer
	dict_o_t * dc_token_ring;
	pcap_t ** pcp_handle; // address to pcap handler
	distributor_t * bcast_xudp_pkt; // xudp means my hdr + udp header and data

} shrt_pth_t;

void mk_shrt_path( _IN AB * pb , _RET_VAL_P shrt_pth_t * shrtcut );

_CALLBACK_FXN void stop_sending_by_bridge( pass_p src_g , long v );
