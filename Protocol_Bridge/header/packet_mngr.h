#pragma once

typedef struct packet_mngr_prerequisite
{
	distributor_t bcast_release_halffill_segment; //throttling_release_halffill_segment; // check if condition is true then set halffill segemtn as fill
	kv_table_t map_tcp_socket; // keep mapping between tcp & id
	ci_sgmgr_t huge_fst_cache; // second huge buffer for after each pcap fast buffer. this buffer can extend to maximum ram size
	pthread_t trd_tcp_sender; // get filled segment and send them

	pthread_t trd_clean_unused_segment; // to clean long time unused free segment

	cbuf_metr last_30_sec_seg_count; // peek segment count every one second
	volatile size_t strides_packet_peek; // take step and prevent segment burst

	//ci_sgmgr_t sent_package_log;

	timeval latest_huge_memory_time; // last successfull packet sent
	timeval latest_memmap_time; // last stored in memmap

	pthread_mutex_t pm_lock;		/* protect against reentrance of threads(pkt_mgr,persist_mgr) */

} pkt_mgr_t;

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
	// TODO . every thing that need for postpond writing must store here

	struct
	{
		rdy_udp_hdr_t udp_hdr; // header from udp gatherer

		struct
		{
			uint8_t version;
			bool sent;
			uint8_t TCP_name_size;
			uint8_t payload_offset; // offset to pkt payload . max 256 metadata sz

			bool retry; // do retry
			bool retried;
			bool fault_registered;
			uint8 cool_down_attempt; // it is very wiered that two attempt to send is near each other
			
			uint64 tcp_name_key_hash;
			uint64 tcp_name_uniq_id;
		};

	} metadata;

	union
	{
		CHAR TCP_name[1]; // variable length name
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


_CALLBACK_FXN status fast_ring_2_huge_ring( pass_p data , buffer buf , size_t sz );

_THREAD_FXN void_p process_filled_tcp_segment_proc( pass_p src_g );

_CALLBACK_FXN void release_halffill_segment( pass_p src_g );

_CALLBACK_FXN status descharge_persistent_storage_data( pass_p data , buffer buf , size_t sz );

void cleanup_pkt_mgr( pkt_mgr_t * pktmgr );

_CALLBACK_FXN void init_packetmgr_statistics( pass_p src_g );

_THREAD_FXN void_p cleanup_unused_segment_proc( pass_p src_g );

_CALLBACK_FXN void sampling_filled_segment_count( pass_p src_g );