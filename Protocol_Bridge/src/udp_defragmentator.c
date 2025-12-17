#define Uses_CFG
#define Uses_timeval_diff_ms
#define Uses_WARNING
#define Uses_udphdr
#define Uses_ether_header
#define Uses_errno
#define Uses_MEMSET_ZERO_O
#define Uses_AB
#define Uses_INIT_BREAKABLE_FXN
#include <Protocol_Bridge.dep>

#define MARKER_MEM MSB_MARKERS[ 4 ]

status init_udps_defragmentator( defraged_udps_t * frg )
{
	sem_init( &frg->gateway , 0 , 0 );

	//pthread_mutex_init( &frg->mtex , NULL ); // TEMP

	return errOK;
}

void finalize_udps_defragmentator( defraged_udps_t * frg )
{
	sem_destroy( &frg->gateway );
}

_GLOBAL_VAR _EXTERN long long _L1Cache_ipv4_entrance;

// called by producer. so it has to be as fast as possible . super fast
_CALLBACK_FXN status defragment_pcap_data( void_p src_pb , void_p src_hdr , void_p src_packet ) // memory come from kernel memory that pcap used
{
	AB * pb = ( AB * )src_pb;
	struct pcap_pkthdr * h = ( struct pcap_pkthdr * )src_hdr;
	u_char * frame_bytes = ( u_char * )src_packet;

	//dump_buffer( frame_bytes , h->caplen );

/*layer 2 Link (Ethernet)*/
	if ( h->caplen < SIZE_ETHERNET ) return errCanceled; // must have ethernet header
	uint16_t ethertype = ntohs( *( uint16_t * )( frame_bytes + 12 ) );
	if ( ethertype != 0x0800 ) return errCanceled; // only handle IPv4

/*layer 3 Internet (IP)*/
	buffer packet = frame_bytes + SIZE_ETHERNET; // skip ethernet header . ipv4 datagrams
	if ( h->caplen < 20 ) return errCanceled; // too short
	const uint8_t pkt_hdr_len_B = (uint8_t)(( packet[ 0 ] & 0x0F ) * 4); // ip header length in bytes . ip_hl
	if ( pkt_hdr_len_B < 20 || pkt_hdr_len_B > 60 || h->caplen < pkt_hdr_len_B ) return errCanceled; // invalid . no data
	
	uint16_t f_pkt_total_length_B = ntohs( *( uint16_t * )( packet + 2 ) ); // IP.total length . fragment size . TOCHECK is correct
	if ( f_pkt_total_length_B > ETHERNET_MTU ) return errCanceled;

	// Handle potential padding: use min of calculated frag_len and available captured data
	uint16_t avail_data_B = h->caplen - sizeof( struct ether_header ) - pkt_hdr_len_B;
	uint16_t frag_len_B = f_pkt_total_length_B - (uint16_t)pkt_hdr_len_B;
	frag_len_B = ( frag_len_B < avail_data_B ) ? frag_len_B : avail_data_B;  // Prevent buffer overrun

	// This addresses the buffer size issue: pcap may capture padded frames (e.g., Ethernet min 64 bytes),
	// but we use ip_len from the IP header to determine actual data size, not pkthdr->caplen.
	// If sent buffer was smaller, ip_len reflects the sent size, ignoring padding.

	/* Fragmentation info from IP header. */
	uint16_t frag_off_field = ntohs( *( uint16_t * )( packet + 6 ) );
	int more = ( frag_off_field & 0x2000 ) ? 1 : 0; // MF bit
	uint16_t frag_offset = ( frag_off_field & 0x1FFF ) * 8; // offset in bytes
	//if ( frag_offset > ETHERNET_MTU ) return errCanceled;

	/* Extract key fields for reassembly. */
	uint32_t src = *( uint32_t * )( packet + 12 ); // src ip
	uint32_t dst = *( uint32_t * )( packet + 16 ); // dst ip
	uint8_t proto = packet[ 9 ]; // proto used
	if ( proto != 17 ) return errCanceled; /*17 means udp in ipv4 layer*/

	_L1Cache_ipv4_entrance++;

/*layer 4 Transport (UDP)*/
	/* Pointer to payload (L4 header start). */
	buffer datagram = packet + pkt_hdr_len_B;
	uint16_t dg_hdr_pyload_len_B = 0;
	uint16_t udp_dg_len_B = 0;
	buffer udp_payload_buf = NULL;
	if ( !frag_offset )
	{
		dg_hdr_pyload_len_B = f_pkt_total_length_B > pkt_hdr_len_B ? ( f_pkt_total_length_B - pkt_hdr_len_B ) : 0;

		if ( dg_hdr_pyload_len_B < 1 || dg_hdr_pyload_len_B > ETHERNET_MTU ) return errCanceled;
		if ( !( dg_hdr_pyload_len_B >= UDP_LAYER_HDR_SZ_B ) ) return errCanceled; // UDP check

		udp_dg_len_B = ntohs( *( uint16_t * )( datagram + 4 ) )/* - sizeof(struct udphdr)*/;
		udp_payload_buf = packet + pkt_hdr_len_B + UDP_LAYER_HDR_SZ_B;
	}
	else
	{
		udp_payload_buf = packet + pkt_hdr_len_B;
	}
	
	dfrg_udp_metadata udp_spec/* = {0}*/ ; // just think about execution speed. because here we are at getter side of rign so we should get as much as possible
	//udp_spec.head_marker = GUARD_HEAD;
	//udp_spec.tail_marker = GUARD_TAIL;

	uint16_t udp_pkt_id = ntohs( *( uint16_t * )( packet + 4 ) ); // IP ID field -> upd pkt id
	
	udp_spec.mark_memory = MARKER_MEM; // to check memory validity
	udp_spec.hdr.srcIP = src;
	udp_spec.hdr.dstIP = dst;
	udp_spec.proto = proto;
	gettimeofday( &udp_spec.hdr.tm , NULL );

	/* Case 1: not fragmented => whole udp packet */
	if ( !frag_offset && !more )
	{
		uint16_t payload_len_B = udp_dg_len_B - sizeof(struct udphdr);

		//my_hdr.frag_offset = 0;
		//my_hdr.udp_pkt_id = 0;
		udp_spec.data_length_B = payload_len_B;
		udp_spec.data_progress_B = payload_len_B;

		//pthread_mutex_lock( &pb->comm.preq.defraged_udps.mtex );
		status d_error = cbuf_pked_push( &pb->comm.preq.raw_xudp_cache , ( buffer )&udp_spec , sizeof( udp_spec ) , sizeof( udp_spec ) , NULL , false );
		if ( d_error != errOK )
		{
			//pthread_mutex_unlock( &pb->comm.preq.defraged_udps.mtex );
			return d_error;
		}
		d_error = cbuf_pked_push( &pb->comm.preq.raw_xudp_cache , udp_payload_buf , udp_spec.data_progress_B , udp_spec.data_length_B , NULL , true );
		//pthread_mutex_unlock( &pb->comm.preq.defraged_udps.mtex );
		pb->comm.preq.defraged_udps.ids[ udp_pkt_id ].dirty = 1;
		pb->comm.preq.defraged_udps.ids[ udp_pkt_id ].filled = 1;
		return d_error;
	}

	/* Case 2: fragmented => first part of fragmented udp */
	if ( !frag_offset && more )
	{
		uint16_t payload_len_B = udp_dg_len_B - (uint16_t)sizeof(struct udphdr);
		uint16_t seg_payload_len_B = frag_len_B - (uint16_t)sizeof(struct udphdr);

		udp_spec.frag_offset = 0;
		udp_spec.hdr.udp_pkt_id = udp_pkt_id;
		udp_spec.data_length_B = payload_len_B;
		udp_spec.data_progress_B = seg_payload_len_B; // use this field at defrag step and it is sign that say udp received completly

		#ifdef CCH
		#undef CCH
		#endif
		#define CCH pb->comm.preq.defraged_udps.ids[ udp_spec.hdr.udp_pkt_id ]

		if ( CCH.last_pos != 0 )
		{
			pb->comm.preq.defraged_udps.mixed_up_udp++;
		}

		size_t hdr_addr , pyld_addr; // size in memory
		//pthread_mutex_lock( &pb->comm.preq.defraged_udps.mtex );
		status d_error = cbuf_pked_push( &pb->comm.preq.raw_xudp_cache , ( buffer )&udp_spec , sizeof( udp_spec ) , sizeof( udp_spec ) , &hdr_addr , false ); // first write header to buff
		if ( d_error != errOK )
		{
			//pthread_mutex_unlock( &pb->comm.preq.defraged_udps.mtex );
			return d_error;
		}
		d_error = cbuf_pked_push( &pb->comm.preq.raw_xudp_cache , udp_payload_buf , udp_spec.data_progress_B , udp_spec.data_length_B , &pyld_addr , true ); // for first one allocate more space then in read defrag by copyting them in their place
		//pthread_mutex_unlock( &pb->comm.preq.defraged_udps.mtex );
		if ( d_error == errOK )
		{
			// it is possible that second part received first after first part
			// some times in test one udp packet received continuously so i should be prepare for that
			if ( CCH.dirty )
			{
				if ( ( CCH.srcIP != udp_spec.hdr.srcIP || CCH.dstIP != udp_spec.hdr.dstIP ) )
				{
					if ( !CCH.done )
					{
						pb->comm.preq.defraged_udps.part_no_matched++;
					}
					MEMSET_ZERO_O( &CCH ); // maybe second part arrived first
				}
				else if ( CCH.done )
				{
					MEMSET_ZERO_O( &CCH ); // maybe second part arrived first
				}
				else if ( CCH.last_pos >= MAXIMUM_FRAGMENT_MADE )
				{
					if ( !CCH.done )
					{
						pb->comm.preq.defraged_udps.part_no_matched++;
					}
					MEMSET_ZERO_O( &CCH ); // maybe second part arrived first
				}
			}

			CCH.data_length_B = udp_spec.data_length_B;
			CCH.srcIP = udp_spec.hdr.srcIP;
			CCH.dstIP = udp_spec.hdr.dstIP;
			CCH.dirty = 1;
			// i prefer that first element keep first part of the udp so it has length and etc ( CCH.ring_addr[ 0 ] )
			CCH.ring_addr[ 0 ][ 0 ] = hdr_addr; // So, the ring handles translation to memory
			CCH.ring_addr[ 0 ][ 1 ] = pyld_addr;
			CCH.data_progress_B += (uint16_t)udp_spec.data_progress_B;

			// CCH.last_pos++; main always in last_pos-> 0 . main does not matter because copy to output
			if ( CCH.data_length_B && CCH.data_progress_B >= CCH.data_length_B )
			{
				CCH.filled = 1;
				sem_post( &pb->comm.preq.defraged_udps.gateway ); // some udp packet complete
			}
		}
		return d_error;
	}

	/* Case 3: fragmented => other part of fragmented udp */
	if ( frag_offset )
	{
		if ( frag_offset <= sizeof( struct udphdr ) )
		{
			return errGeneral;
		}

		frag_offset -= sizeof(struct udphdr);
		uint16_t seg_payload_len_B = frag_len_B /* - sizeof(struct udphdr)*/;

		if ( CCH.last_pos * ETHERNET_MTU + seg_payload_len_B > BUFFER_SIZE )
		{
			pb->comm.preq.defraged_udps.buffer_overload_error++;
			return errGeneral;
		}

		udp_spec.frag_offset = frag_offset;
		udp_spec.hdr.udp_pkt_id = udp_pkt_id;
		udp_spec.data_length_B = 0; // not usable in here . because this is middle or terminate packet
		udp_spec.data_progress_B = seg_payload_len_B; // be zero it means this is partial data udp

		size_t hdr_addr , pyld_addr;
		//pthread_mutex_lock( &pb->comm.preq.defraged_udps.mtex );
		status d_error = cbuf_pked_push( &pb->comm.preq.raw_xudp_cache , ( buffer )&udp_spec , sizeof( udp_spec ) , sizeof( udp_spec ) , &hdr_addr , false );
		if ( d_error != errOK )
		{
			//pthread_mutex_unlock( &pb->comm.preq.defraged_udps.mtex );
			return d_error;
		}
		d_error = cbuf_pked_push( &pb->comm.preq.raw_xudp_cache , udp_payload_buf , udp_spec.data_progress_B , udp_spec.data_progress_B , &pyld_addr , true ); // for first one allocate more space then in read defrag by copyting them in their place
		//pthread_mutex_unlock( &pb->comm.preq.defraged_udps.mtex );
		if ( d_error == errOK )
		{
			if ( CCH.dirty )
			{
				if ( ( CCH.srcIP != udp_spec.hdr.srcIP || CCH.dstIP != udp_spec.hdr.dstIP ) )
				{
					if ( !CCH.done )
					{
						pb->comm.preq.defraged_udps.part_no_matched++;
					}
					MEMSET_ZERO_O( &CCH ); // maybe second part arrived first
				}
				else if ( CCH.done )
				{
					MEMSET_ZERO_O( &CCH ); // maybe second part arrived first
				}
				else if ( CCH.last_pos >= MAXIMUM_FRAGMENT_MADE - 1 )
				{
					if ( !CCH.done )
					{
						pb->comm.preq.defraged_udps.part_no_matched++;
					}
					MEMSET_ZERO_O( &CCH ); // maybe second part arrived first
				}
			}

			//CCH.data_length_B = my_hdr.data_length_B;
			CCH.srcIP = udp_spec.hdr.srcIP;
			CCH.dstIP = udp_spec.hdr.dstIP;
			CCH.dirty = 1;
			CCH.ring_addr[ CCH.last_pos + 1 ][ 0 ] = hdr_addr; // So, the ring handles translation to memory
			CCH.ring_addr[ CCH.last_pos + 1 ][ 1 ] = pyld_addr;
			CCH.data_progress_B += udp_spec.data_progress_B;

			CCH.last_pos++; // extra segment from pos >=1
			if ( CCH.last_pos >= MAXIMUM_FRAGMENT_MADE )
			{
				CCH.last_pos = CCH.last_pos % MAXIMUM_FRAGMENT_MADE;
			}
			if ( CCH.data_length_B && CCH.data_progress_B >= CCH.data_length_B )
			{
				CCH.filled = 1;
				sem_post( &pb->comm.preq.defraged_udps.gateway ); // some udp packet complete
			}

			#undef CCH
		}
		return d_error;
	}
	return errCanceled;
}

// get udp packet payload plus extra hdr that not usabale . called in slower mode side of ring( consumer )
status poped_defraged_packet( void_p src_pb , OUTcpy buffer out_buf , OUTx size_t * out_len_B , OUTcpy rdy_udp_hdr_t * out_hdr )
{
	AB * pb = ( AB * )src_pb;
	G * _g = TO_G( pb->cpy_cfg.m.m.temp_data._pseudo_g );
	
	status d_error;
	dfrg_udp_metadata tmp_hdr; // top udp packet part in head of buffer
	size_t hdr_sz = 0;
	if ( ( d_error = cbuf_pked_pop( &pb->comm.preq.raw_xudp_cache , &tmp_hdr , sizeof( tmp_hdr ) , &hdr_sz , (long)CFG().time_out_sec , true ) ) != errOK )
	{
		if ( d_error != errTimeout )
		{
			return d_error;
		}
		return d_error;
	}

	if ( tmp_hdr.mark_memory != MARKER_MEM ) // very error prone but fast
	{
		goto _ignore;
	}

	#ifdef CCH
	#undef CCH
	#endif
	#define CCH pb->comm.preq.defraged_udps.ids[ tmp_hdr.hdr.udp_pkt_id ]

	if ( CCH.done )
	{
		goto _ignore;
	}

	if ( tmp_hdr.data_length_B && tmp_hdr.data_progress_B == tmp_hdr.data_length_B ) // it is completed
	{
		if ( out_hdr ) MEMCPY( out_hdr , &tmp_hdr.hdr );
		d_error = cbuf_pked_pop( &pb->comm.preq.raw_xudp_cache , out_buf , 0/*no exp*/ , out_len_B , (long)CFG().time_out_sec , false ); // most of the packet get here na dis normal
		if( d_error ) return d_error;
		goto _update_stat;
	}

	// until this line complete and simple packet sent . now we have hdr that says next buf block situation
	// now we continue with this basic assumption that udp packet fragmented part may arrive without any order
	// there is addinional assumption that udp generator at least make 5 udp fragment so it can be used in dictionary policy to make it fast
	// we know whick udp packet code to pop up

	WARNING( tmp_hdr.data_progress_B ); // it is not possible not no receive any payload

	uint16_t tmp_data_length_B = CCH.data_length_B; // 14040922 . just for debug reason
	uint16_t tmp_data_progress_B = CCH.data_progress_B; // 14040922 . just for debug reason
	uint8_t tmp_last_pos = CCH.last_pos; // 14040922 . just for debug reason

	while( CCH.data_length_B && CCH.data_progress_B < CCH.data_length_B )
	{
		struct timespec ts;
		clock_gettime( CLOCK_REALTIME , &ts );
		ts.tv_nsec += CFG().fragment_udp_retention_time_msec * 1000000; // 5 mili
		if ( sem_timedwait( &pb->comm.preq.defraged_udps.gateway , &ts ) < 0 ) // wait for open signal . decrements the semaphore . if zero wait
		{
			if ( errno == ETIMEDOUT )
			{
				break;
			}
		}

		if ( CCH.data_progress_B >= CCH.data_length_B ) break;

		struct timeval tnow;
		gettimeofday( &tnow , NULL );
		if ( timeval_diff_ms( &tmp_hdr.hdr.tm , &tnow ) > ( double )CFG().fragment_udp_retention_time_msec )
		{
			break;
		}
	}
	if ( !CCH.data_length_B || CCH.data_progress_B < CCH.data_length_B || !CCH.filled )
	{
		if ( !CCH.done )
		{
			pb->comm.preq.defraged_udps.packet_no_aggregate++;
		}
		
		goto _ignore;
	}

	// until we pop no one can push another item if buf is full
	// first find main packet that has free space
	// we have assumption that cycle of udp packet id take too long time and it is possible to process udp before next equl id ruined it
	// now time for dirty stuff and do ugly thing with ring buffer . just for performance

	//pthread_mutex_lock( &pb->comm.preq.defraged_udps.mtex );
	for ( uint8_t ipos = 0 ; ipos <= CCH.last_pos ; ipos++ )
	{
		dfrg_udp_metadata segm;
		// is bad mem . should not be proc because first one in grg peak . first one is main so others must be not
		if ( cbuf_pked_blindcopy( &pb->comm.preq.raw_xudp_cache , &segm , CCH.ring_addr[ ipos ][ 0 ] ) != errOK
			|| segm.mark_memory != MARKER_MEM || !segm.data_progress_B ) // hdr
		{
			goto _ignore;
		}

		if ( ipos == 0 )
		{
			if ( out_len_B ) *out_len_B = segm.data_length_B;
			if ( out_hdr ) MEMCPY( out_hdr , &segm.hdr );
		}
		
		// sag to rohehet agar kar nakoni . var gorbeh be jamalet agar kar koni
		cbuf_pked_blindcopy( &pb->comm.preq.raw_xudp_cache , out_buf + segm.frag_offset , CCH.ring_addr[ ipos ][ 1 ] ); // data
	}

	CCH.done = 1; // to just later access ignored and do nothing
	CCH.last_pos = 0; // 14040922 . just to be insure next get packet placed on correct position

	cbuf_pked_pop( &pb->comm.preq.raw_xudp_cache , NULL , 0 , NULL , 1 , false ); // just ignore first occurance of fregmented part
	#undef CCH

	goto _update_stat;

_ignore:
	cbuf_pked_pop( &pb->comm.preq.raw_xudp_cache , NULL , 0 , NULL , 1/*timeout*/ , false ); // ignore one pkt
	return errGeneral;

_update_stat:
	gettimeofday( &pb->stat.round_zero_set.t_end , NULL );

	pb->stat.round_zero_set.udp.total_udp_get_count++;
	pb->stat.round_zero_set.udp.total_udp_get_byte += *out_len_B;
	pb->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count++;
	pb->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes += *out_len_B;

	pb->stat.round_zero_set.udp_get_data_alive_indicator++;

	time_t tnow = 0;
	tnow = time( NULL );
	// udp
	if ( difftime( tnow , pb->stat.round_zero_set.udp_1_sec.t_udp_throughput ) >= 1.0 )
	{
		#ifdef ENABLE_THROUGHPUT_MEASURE
		if ( pb->stat.round_zero_set.udp_1_sec.t_udp_throughput > 0 )
		{
			cbuf_m_advance( &pb->stat.round_init_set.udp_stat_5_sec_count , pb->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
			cbuf_m_advance( &pb->stat.round_init_set.udp_stat_5_sec_bytes , pb->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );

			cbuf_m_advance( &pb->stat.round_init_set.udp_stat_10_sec_count , pb->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
			cbuf_m_advance( &pb->stat.round_init_set.udp_stat_10_sec_bytes , pb->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );

			cbuf_m_advance( &pb->stat.round_init_set.udp_stat_40_sec_count , pb->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
			cbuf_m_advance( &pb->stat.round_init_set.udp_stat_40_sec_bytes , pb->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );
		}
		#endif
		pb->stat.round_zero_set.udp_1_sec.t_udp_throughput = tnow;
		pb->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count = 0;
		pb->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes = 0;
	}

	return errOK;
}
