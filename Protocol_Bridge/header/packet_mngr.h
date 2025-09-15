#pragma once

typedef struct ready_2_send_packet_v1
{
	// TODO . every thing that need for postpond writing must store here
	uint8_t version;
	CHAR TCP_name[1];
	//DATAB * pkt;

} rdy_pkt1;

#define TCP_PACKET_VERSION 1

_CALLBACK_FXN status operation_on_tcp_packet( pass_p data , buffer buf , int sz );

//#define SEGMENT_SIZE (64 * 1024 * 1024) // 64 MB, tunable
////#define MAX_SEGMENTS_IN_RAM 4          // tunable per RAM
//
//typedef struct
//{
//	uint64_t base_offset;     // offset of first byte in this segment
//	size_t   write_pos;       // bytes written in segment
//	size_t   capacity;        // SEGMENT_SIZE
//	buffer   buffer;          // pointer to contiguous memory
//	int		 sealed;          // no further writes
//	int		 on_disk;         // already flushed to disk
//	char	 filename[ 256 ]; // disk filename if flushed
//} segment_t;
//
//typedef struct
//{
//	segment_t segments[ MAX_SEGMENTS_IN_RAM ]; // ring
//	size_t    head_idx;   // index for next write segment
//	size_t    tail_idx;   // oldest sealed segment
//	uint64_t  next_offset; // next log_offset to assign
//	pthread_mutex_t lock;  // for simple multi-producer; prefer lock-free/sharded
//} log_store_t;
//

