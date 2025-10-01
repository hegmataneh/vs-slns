#pragma once

#if defined Uses_udp_defragmentator || !defined __COMPILING

#define MAXIMUM_FRAGMENT_MADE 5

typedef struct cache_udp_part_fgms // aligned for boost up
{
	volatile size_t ring_addr[MAXIMUM_FRAGMENT_MADE][2]; /*hdr addr + data addr*/
	uint8_t last_pos;
	uint8_t dirty;
	uint16_t data_length;
	uint32_t srcIP , dstIP; // Composite Key
	volatile uint16_t data_progress; // update imadiatly
	uint16_t done;
} udp_fgms;

typedef struct cache_udps_fgms
{
	udp_fgms ids[G_USHORT_MAX];
	sem_t gateway;
} udps_fgms;

typedef struct gather_defragmentated_udp // aligned for boost up
{
	uchar mark_memory; // check memory correctness
	uint8_t proto; // protocole that use . for now it is udp
	uint16_t frag_offset; // fragmented udp offset
	uint32_t srcIP , dstIP; // use as udp id
	uint16_t udp_pkt_id;

	uint16_t data_length; // total length . just for main
	uint16_t data_progress; // main and extra
	uint16_t pad1;
	struct timeval tm;
} dfrg_udp;

status init_udps_fgms( udps_fgms * fgms );

_CALLBACK_FXN status defragment_pcap_data( void_p src_pb , void_p src_hdr , void_p src_packet );

status poped_defraged_packet( void_p src_pb , buffer out_buf , size_t * out_len );

#endif

