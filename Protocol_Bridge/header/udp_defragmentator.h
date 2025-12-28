#pragma once

#if defined Uses_udp_defragmentator || !defined __COMPILING

#define MAXIMUM_FRAGMENT_MADE 5 /*mr mohammad masoumi said that in wort situation there is five part for udp*/

typedef struct udp_single_part // aligned for boost up
{
	size_t ring_addr[MAXIMUM_FRAGMENT_MADE][2]; /*hdr addr + data addr*/
	
	uint8_t last_pos;		// last udp part pos . this var use to fill udp part hdr
	char pad1[3];
	uint16_t data_length_B;
	uint16_t data_progress_B;	// update imadiatly . no cache, direct memory access
		
	union
	{
		struct timeval tm; // time of first arrival
		uint64_t dirty;
	};
	//uint32_t srcIP;		// Composite Key
	//uint32_t dstIP;		// Composite Key
	//uint8_t dirty;	// if there is packet with same udp id already exist		
	//uint8_t filled;	// use when progress exact with length
	//uint8_t done;		// udp process and sent done

} udp_part; // each recieved udp packet id

#define UDP_IDS_COUNT 65536
#define WORD_BITS 64

typedef struct pkt_lock_s
{
	_Atomic uint_fast8_t lock;
	char pad[ 63 ];
} pkt_lock_t;

typedef struct defragmented_udp_pcaket
{
	udp_part ids[ UDP_IDS_COUNT ]; // limited buffer for storing header of udps and replace old one from top with new one arrived
	pkt_lock_t pktlcks[ UDP_IDS_COUNT ];

	sem_t gateway;

	ulong ipv4_bad_structure; /*bad packet format*/
	ulong kernel_error; /*allocation error*/
	ulong bad_buffer_err; /*buffer not contain good data*/
	ulong unordered_ipv4; /*some part of udp arrived not in order*/
	ulong defragmentation_corrupted; /*packet cannot defraged*/

	ulong L1Cache_ipv4s;

	//ulong part_no_matched; // statistics
	//ulong buffer_overload_error; // statistics . there is offset value in udp packet that point to out of bound ranjes
	//ulong packet_no_aggregate; // there is udp packet not completed

} defraged_udps_t;

typedef struct gather_defragmentated_udp_metadata // aligned for boost up
{
	rdy_udp_hdr_t hdr;

	//struct
	//{
		uchar mark_memory; // sign that help to check memory validity and correctness
		uint8_t proto; // protocole that use . for now it is udp
		uint16_t frag_offset; // fragmented udp offset
		uint16_t data_length_B; // total length . just for main
		uint16_t data_progress_B; // main and extra
	//};

} dfrg_udp_metadata;

status init_udps_defragmentator( defraged_udps_t * fgms );
void finalize_udps_defragmentator( defraged_udps_t * fgms );

_CALLBACK_FXN status defragment_pcap_data( void_p src_pb , void_p src_hdr , void_p src_packet );

status poped_defraged_packet( void_p src_pb , OUTalc buffer out_buf , size_t out_buf_sz , OUTx size_t * out_len_B , OUTalc rdy_udp_hdr_t * out_hdr );

#endif

