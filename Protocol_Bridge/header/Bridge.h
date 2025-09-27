#pragma once

typedef union AB_execution_tools
{

	struct s_pcap_udp_counter // one thread for send and receive
	{
		pthread_t trd_id;
		pcap_t * handle;
	} *p_pcap_udp_counter;

	struct s_krnl_udp_counter
	{
		pthread_t trd_id;
	} *p_krnl_udp_counter;

	struct s_one2one_krnl2krnl_SF // one thread for each direction with store & forward method . with kernel utils
	{
		pthread_t income_trd_id;
		pthread_t outgoing_trd_id;
		vcbuf_nb cbuf; // buffer
		distributor_t buffer_pop_distributor; // responsible for distribute udp packet to each type
	} *p_one2one_krnl2krnl_SF;

	struct s_one2one_pcap2krnl_SF // one thread for each direction with store & forward method
	{
		pthread_t income_trd_id;
		pthread_t outgoing_trd_id;
		vcbuf_nb cbuf; // buffer
		distributor_t buffer_pop_distributor; // responsible for distribute udp packet to each type
		pcap_t * handle;
	} *p_one2one_pcap2krnl_SF;

	struct s_one2many_pcap2krnl_SF // one thread for each direction with store & forward method
	{
		pthread_t income_trd_id;
		pthread_t outgoing_trd_id;

		vcbuf_nb cbuf; // buffer

		distributor_t buffer_pop_distributor; // responsible for distribute udp packet to each type
		dict_o_t dc_token_ring; // config may have multi rings and each one is for one grp

		pcap_t * handle;
	} *p_one2many_pcap2krnl_SF;

	struct s_many2one_pcap2krnl_SF_serialize // one thread for each direction with store & forward method
	{
		pthread_t income_trd_id;
		pthread_t outgoing_trd_id;

		vcbuf_nb cbuf; // buffer

		distributor_t buffer_pop_distributor; // responsible for distribute udp packet to each type
		dict_o_t dc_token_ring; // config may have multi rings and each one is for one grp

	} *p_many2one_pcap2krnl_SF_serialize;

} ex_tls;

typedef struct AB_thread // threads use to recv and send data
{
	struct s_bridges_thread_base
	{
		int thread_is_created;
		int do_close_thread; // command from outside to inside thread
		int bridg_prerequisite_stabled; // because udp port may start after thread started . if all the condition is ready to bridge thread start

		distributor_t buffer_push_distributor; // responsible for distribute udp packet to each type
	} base;

	ex_tls t;

} ABtrd;

//

struct ActiveBridge;

typedef struct AB_udp_connection
{
	sockfd udp_sockfd;
	int udp_connection_established; // udp socket established
	int retry_to_connect_udp;
	udp_cfg_pak * __udp_cfg_pak; // link to passive cfg
	struct ActiveBridge * owner_pb; // upper struct

} AB_udp;

typedef struct AB_tcp_connection
{
	sockfd tcp_sockfd;
	int tcp_connection_established; // tcp connection established
	int retry_to_connect_tcp;
	tcp_cfg_pak * __tcp_cfg_pak; // link to passive cfg
	struct ActiveBridge * owner_pb; // upper struct

} AB_tcp;

typedef struct ActiveBridge // protocol_bridge . each bridge define one or many input and output that recev data from that input
{
	// caution . make copy of allocation values
	Bcfg cpy_cfg;  // copy of applied cfg . active protocol_bridge config . یک دسته کامل از ترد ها یک کانفیگ را اعمال می کند
	ABtrd trd;
	ABstat stat;

	AB_udp *udps;
	int udps_count;

	AB_tcp *tcps;
	int tcps_count;

	nnc_table * pstat_tbl;
	
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

	int * bridg_prerequisite_stabled;
	distributor_t * buf_psh_distri;

	vcbuf_nb * cbuf; // buffer
	distributor_t * buf_pop_distr;
	dict_o_t * dc_token_ring;
	pcap_t ** handle;

} shrt_path;

void mk_shrt_path( _IN AB * pb , _RET_VAL_P shrt_path * hlpr );
