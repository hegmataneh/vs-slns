#pragma once

//// ready packet

typedef struct udp_packet_header
{
	struct timeval tm;
	union
	{
		uint16_t udp_pkt_id;
		PAD(8);
	};
	//uint32_t srcIP , dstIP; // use as udp id
} rdy_udp_hdr_t;

enum e_pkt_state // : unsigned char // does not support in old linux
{
	ps_on_L1Cache = 0,
	ps_on_L2_RAM ,
	ps_original_remain_packet_on_ram , /*remain packet on mem when replicate goes into file*/
	ps_replicated_one_on_pub_file , /*when pakcet copy to file it has replicated so origin and replicate one has seperate state*/
	ps_remain_packet_on_pub_file ,
	ps_replicated_one_on_priv_file ,
	ps_sent

};
typedef enum e_pkt_state pkt_state;


typedef struct /*ready_2_send_packet_v1*/
{
	// everything that need for postpond writing, must store here

	struct
	{
		rdy_udp_hdr_t udp_hdr; // header from udp gatherer
		size_t payload_sz;

		union
		{
			struct
			{
				uint8_t version;
				uint8_t TCP_name_size;
				uint8_t payload_offset; // offset to pkt payload . max 256 metadata sz
				uint8 cool_down_attempt; // it is very wiered that two attempt to send is near each other
			};
			PAD(8);
		};

		pkt_state prev_state;
		pkt_state state;

		uint64 tcp_name_key_hash;
		uint64 tcp_name_uniq_id;

	} metadata;

	union /*64 byte long*/
	{
		CHAR TCP_name[ 1 ]; // variable length name
		//DATAB pkt[1]; // after TCP_name data come
	};

} xudp_hdr; //rdy_pkt1;

#define TCP_XPKT_V1 MSB_MARKERS[3] /*version also check memory correctness as much as possible*/
#define TCP_XPKT_V2 MSB_MARKERS[2]
// ...

#if defined Uses_packet_mngr_prerequisite || !defined __COMPILING

typedef enum about_to_fill_pressure
{
	prss_no_pressure ,
	prss_gentle_pressure ,
	prss_aggressive_pressure ,
	prss_emergency_pressure ,
	prss_red_zone_pressure ,

	prss_skip_section , /*donot use this directly to assign or right of operation*/
	prss_skip_input_by_memory_fulled ,
	prss_skip_input_by_hard_fulled
} prss_e;

typedef struct packet_mngr_prerequisite
{
	kv_table_t map_tcp_socket; // keep mapping between tcp & id
	ci_sgmgr_t harbor_memory; // second huge buffer for storing all otput pkts. this buffer can extend to maximum ram size
	pthread_t trd_tcp_sender; // get filled segment and send them

	//cbuf_metr last_n_peek_total_seg_count; // peek segment count every one second
	//cbuf_metr last_n_peek_filled_seg_count; // peek segment count every one second
	
	size_t sampling_sent_packet_stride; // take step and prevent segment burst
	size_t TTF_sampling_sent_packet_stride;
	//size_t passable_sampling_rate_stride;

	STAT_FLD timeval latest_huge_memory_time; // last successfull packet sent
	STAT_FLD timeval latest_memmap_time; // last stored in memmap

	pthread_mutex_t pm_lock;		/* protect against reentrance of threads(pkt_mgr,persist_mgr) */

	struct /*they are about huge mem not memmap so does not calc out from file*/
	{
		cr_in_wnd_t ram_ctrl_loadOnInput; /*just one thread work with this. change this if multi output used*/ /*make out two because of thread safe*/
		cr_in_wnd_t ram_ctrl_instantaneousInputLoad;

		cr_in_wnd_t ram_ctrl_loadOnStorage;
		cr_in_wnd_t ram_ctrl_loadOnOutBridge;
		instBps_t ram_ctrl_instantaneousloadOnOutBridge;
		
		//cr_in_wnd_t ram_ctrl_instantaneousloadOnStorage;

		prss_e sampling_threshold_stage;
	};

} pkt_mgr_t;


_CALLBACK_FXN status fast_ring_2_huge_ring( pass_p data , buffer buf , size_t sz );

_THREAD_FXN void_p process_filled_tcp_segment_proc( pass_p src_g );

_THREAD_FXN void_p proc_and_release_halffill_segment( pass_p src_g );

#ifdef ENABLE_PERSISTENT_CACHE
_CALLBACK_FXN status discharge_persistent_storage_data( pass_p data , buffer buf , size_t sz );
#endif

void cleanup_pkt_mgr( pkt_mgr_t * pktmgr );

_CALLBACK_FXN void init_packetmgr_statistics( pass_p src_g );

#ifdef ENABLE_CLEAN_UNUSED_SEGMENT
_THREAD_FXN void_p cleanup_unused_idle_segment_proc( pass_p src_g );
#endif

#ifdef ENABLE_STORE_OLD_FILLED_LOW_PROBABLE_SENDABLE_SEGMENT
_THREAD_FXN void_p evacuate_long_time_sediment_segment_proc( pass_p src_g );
#endif

#ifdef ENABLE_RELEASE_HALFFILL_UNUSED_SEGMENT
_CALLBACK_FXN void try_release_halffill_segment( pass_p src_g , long v );
#endif

_CALLBACK_FXN void sampling_filled_segment_count( pass_p src_g );

#endif
