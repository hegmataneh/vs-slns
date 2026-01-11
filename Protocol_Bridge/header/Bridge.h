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
	struct /*AB_common_prerequisite*/
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
				bool stop_sending; /*try to stop fetching udp and sending*/
				bool send_stoped; /*fetching udp and send stoped*/
			};
			uchar pad2[ CACHE_LINE_SIZE ];
		};
		union
		{
			int thread_is_created;
			uchar pad3[ CACHE_LINE_SIZE ];
		};
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
	union
	{
		struct AB_tcp_connection * this; /*main connection has pointed to him self but other copy point to main one*/ /*every access to tco should wraped by this ptr because of sharing tcp connection between AB*/
		struct
		{
			void * this_mirror; /*main connection has pointed to him self but other copy point to main one*/
			union
			{
				bool main_instance; // maininstance of tcp conn
				long pad1;
			};
			sockfd tcp_sockfd;
			int tcp_connection_established; // tcp connection established
	
			union
			{
				struct
				{
					//int retry_to_connect_tcp; // do retry to connect again
					int tcp_is_about_to_connect; // when tring to connect no more try should be attempt
				};
				long pad2;
			};

			distributor_t bcast_change_state;
			timespec last_send_ts; // last time any pkt sent
			timespec last_action_ts; // action is more than just sending
		};
	};

	tcp_cfg_pak_t * __tcp_cfg_pak; // link to passive cfg
	struct ActiveBridge * owner_pb; // upper struct
	cr_in_wnd_t brdg_rate_ctrl_loadOnOutBridge;

} AB_tcp;

typedef struct ActiveBridge // protocol_bridge . each bridge define one or many input and output that recev data from that input
{
	// caution . make copy of allocation values
	brg_cfg_t cpy_cfg;  // copy of applied cfg . active protocol_bridge config . یک دسته کامل از ترد ها یک کانفیگ را اعمال می کند
	ABcomm comm; /*trd*/
	ABstat stat;

	AB_udp *udps;
	size_t udps_count;

	AB_tcp *tcps;
	size_t tcps_count;

#ifdef HAS_STATISTICSS
	nnc_table * ab_stat_tbl;
#endif

	time_t * pthread_alive_time; /*use in pcap loop*/
	
} AB;

typedef struct AB_holders
{
	SHARED_MEM dyn_mms_arr ABs; // AB . all the active protocol_bridge or Active bridges
	size_t connected_tcp_out; /*is there any output tcp exist. if there is so it could be possible to iterate throw memmaps*/

	struct
	{
		pthread_t trd_tcp_connection;
		pthread_mutex_t mtx; /*prevent multi creation*/
		bool bcreated; // thread is created
	} tcps_trd;
} ABhs;


typedef struct /*ActiveBridgeShortPathHelper*/ // every virtually inherit struct must have this vars same as this struct without any difference
{
	AB * pab;
	size_t * in_count;
	size_t * out_count;
	int * thread_is_created;

	cbuf_pked_t * raw_xudp_cache; // buffer
	dict_o_t * dc_token_ring;
	pcap_t ** pcp_handle; // address to pcap handler
	distributor_t * bcast_xudp_pkt; // xudp means my hdr + udp header and data

} shrt_pth_t;

void mk_shrt_path( _IN AB * pb , _RET_VAL_P shrt_pth_t * shrtcut );

_CALLBACK_FXN void try_stoping_sending_from_bridge( pass_p src_g , long v );

_CALLBACK_FXN void init_bridges_statistics( pass_p src_g );