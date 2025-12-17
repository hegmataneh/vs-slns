#pragma once

//// ready packet

typedef struct udp_packet_header
{
	struct timeval tm;
	uint32_t srcIP , dstIP; // use as udp id
	uint16_t udp_pkt_id;
	bool logged_2_mem;
	bool log_double_checked;
} rdy_udp_hdr_t;

typedef struct /*ready_2_send_packet_v1*/
{
	// everything that need for postpond writing, must store here

	struct
	{
		rdy_udp_hdr_t udp_hdr; // header from udp gatherer

		struct
		{
			uint8_t version;
			bool sent;
			uint8_t TCP_name_size;
			uint8_t payload_offset; // offset to pkt payload . max 256 metadata sz
			size_t payload_sz;

			bool retry; // do retry
			bool retried;
			bool is_faulti;
			uint8 cool_down_attempt; // it is very wiered that two attempt to send is near each other

			union
			{
				struct
				{
					bool filed; // send to file
					bool is_remapped; // mapped into exclusive file
				};
				size_t pad1;
			};

			uint64 tcp_name_key_hash;
			uint64 tcp_name_uniq_id;
		};

	} metadata;

	union
	{
		CHAR TCP_name[ 1 ]; // variable length name
		//DATAB pkt[1]; // after TCP_name data come
	};

} xudp_hdr; //rdy_pkt1;

#define TCP_XPKT_V1 MSB_MARKERS[3] /*version also check memory correctness as much as possible*/
#define TCP_XPKT_V2 MSB_MARKERS[2]
// ...

//// log
//typedef struct packet_mgr_write_ahead_log
//{
//	uint32_t srcIP , dstIP;
//	struct timeval tm;
//	uint16_t udp_pkt_id;
//} pkt_wal_t;

#if defined Uses_packet_mngr_prerequisite || !defined __COMPILING

typedef struct packet_mngr_prerequisite
{
#ifdef ENABLE_HALFFILL_SEGMENT
	distributor_t bcast_release_halffill_segment; //throttling_release_halffill_segment; // check if condition is true then set halffill segemtn as fill
#endif
	kv_table_t map_tcp_socket; // keep mapping between tcp & id
	ci_sgmgr_t harbor_memory; // second huge buffer for storing all otput pkts. this buffer can extend to maximum ram size
	pthread_t trd_tcp_sender; // get filled segment and send them

	cbuf_metr last_n_peek_total_seg_count; // peek segment count every one second
	cbuf_metr last_n_peek_filled_seg_count; // peek segment count every one second
	//cbuf_metr input_rates , output_rates; /*these cr_in_wnd_t keep track of bandwidth on IO*/
	
	size_t sampling_sent_packet_stride; // take step and prevent segment burst
	//size_t hysteresis_evacuate_segments_countdown; // take step and prevent segment burst . depend on prev state

	//ci_sgmgr_t sent_package_log;

	STAT_FLD timeval latest_huge_memory_time; // last successfull packet sent
	STAT_FLD timeval latest_memmap_time; // last stored in memmap

	pthread_mutex_t pm_lock;		/* protect against reentrance of threads(pkt_mgr,persist_mgr) */

	/*they are about huge mem not memmap so does not calc out from file*/
	cr_in_wnd_t longTermInputLoad /*just one thread work with this. change this if multi output used*/; /*make out two because of thread safe*/
	//cr_in_wnd_t longTermTcpOutLoad;
	cr_in_wnd_t longTermToStorageOutLoad;
	cr_in_wnd_t longTermOutLoad;
	instBps_t instantaneousInputLoad; /*just input may have a peak*/
	/*~*/

} pkt_mgr_t;

_CALLBACK_FXN status fast_ring_2_huge_ring( pass_p data , buffer buf , size_t sz );

_THREAD_FXN void_p process_filled_tcp_segment_proc( pass_p src_g );

#ifdef ENABLE_HALFFILL_SEGMENT
_CALLBACK_FXN void release_halffill_segment( pass_p src_g , long v );
#endif

_CALLBACK_FXN status discharge_persistent_storage_data( pass_p data , buffer buf , size_t sz );

void cleanup_pkt_mgr( pkt_mgr_t * pktmgr );

_CALLBACK_FXN void init_packetmgr_statistics( pass_p src_g );

_THREAD_FXN void_p cleanup_unused_segment_proc( pass_p src_g );

_THREAD_FXN void_p evacuate_old_segment_proc( pass_p src_g );

_CALLBACK_FXN void sampling_filled_segment_count( pass_p src_g );

#endif
