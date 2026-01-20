#define Uses_ceil
#define Uses_LOCK_LINE
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

_GLOBAL_VAR _EXTERN G * _g;

#define MARKER_MEM MSB_MARKERS[ 4 ]

status init_udps_defragmentator( defraged_udps_t * frg )
{
	sem_init( &frg->gateway , 0 , 0 );

	return errOK;
}

void finalize_udps_defragmentator( defraged_udps_t * frg )
{
	sem_destroy( &frg->gateway );
}

//extern void * __file_map;
//extern int last_pos;

//_PRIVATE_FXN inline void cpu_relax( void )
//{
//#if defined(__x86_64__) || defined(__i386__)
//	__asm__ __volatile__( "pause" );
//#elif defined(__aarch64__) || defined(__arm__)
//	__asm__ __volatile__( "yield" );
//#else
//	__asm__ __volatile__( "" ::: "memory" );
//#endif
//}

#define DEFRAGED_UDPS() pb->comm.preq.defraged_udps

//static inline bool try_acquire_lock( _Atomic uint_fast8_t * lock )
//{
//	uint_fast8_t expected = 0;
//	return atomic_compare_exchange_strong_explicit(
//		lock , &expected , 1 ,
//		memory_order_acquire ,
//		memory_order_relaxed
//	);
//}

//_PRIVATE_FXN void __unlock( AB * pb , uint16_t id )
//{
//	//atomic_store_explicit( &DEFRAGED_UDPS().pktlcks[ id ].lock , 0 , memory_order_release );
//}
//
//_PRIVATE_FXN void __lock( AB * pb , uint16_t id )
//{
//	////CIRCUIT_BREAKER long break_cuit_1 = 0;
//	//while ( !try_acquire_lock( &DEFRAGED_UDPS().pktlcks[ id ].lock ) )
//	//{
//	//	//break_cuit_1++;
//	//	cpu_relax();
//	//	//if ( break_cuit_1 > 100000000 )
//	//	{
//	//		/*struct timespec ts;
//	//		ts.tv_sec = 0;   // seconds
//	//		ts.tv_nsec = 1000000L;  // 1 mili
//	//		thrd_sleep( &ts , NULL );*/
//
//	//		//__unlock( pb , id );
//	//	}
//	//}
//}

#define __unlock( a , b )
#define __lock( a , b )

// called by producer. so it has to be as fast as possible . super fast
_CALLBACK_FXN status defragment_pcap_data( void_p src_pb , void_p src_hdr , void_p src_packet ) // memory come from kernel memory that pcap used
{
	AB * pb = ( AB * )src_pb;
	struct pcap_pkthdr * h = ( struct pcap_pkthdr * )src_hdr;
	u_char * frame_bytes = ( u_char * )src_packet;

	//dump_buffer( frame_bytes , h->caplen );

/*layer 2 Link (Ethernet)*/
	if ( h->caplen < SIZE_ETHERNET )
	{
		DEFRAGED_UDPS().ipv4_bad_structure++; /*error situation*/
		return errCanceled; // must have ethernet header
	}
	uint16_t ethertype = ntohs( *( uint16_t * )( frame_bytes + 12 ) );
	PAD( sizeof( void * ) - sizeof( uint16_t ) );
	if ( ethertype != 0x0800 )
	{
		DEFRAGED_UDPS().ipv4_bad_structure++; /*error situation*/
		return errCanceled; // only handle IPv4
	}

/*layer 3 Internet (IP)*/
	buffer packet = frame_bytes + SIZE_ETHERNET; // skip ethernet header . ipv4 datagrams
	if ( h->caplen < 20 )
	{
		DEFRAGED_UDPS().ipv4_bad_structure++; /*error situation*/
		return errCanceled; // too short
	}
	const uint8_t pkt_hdr_len_B = (uint8_t)(( packet[ 0 ] & 0x0F ) * 4); // ip header length in bytes . ip_hl
	PAD( sizeof( void * ) - sizeof( uint8_t ) );
	if ( pkt_hdr_len_B < 20 || pkt_hdr_len_B > 60 || h->caplen < pkt_hdr_len_B )
	{
		DEFRAGED_UDPS().ipv4_bad_structure++; /*error situation*/
		return errCanceled; // invalid . no data
	}
	
	uint16_t f_pkt_total_length_B = ntohs( *( uint16_t * )( packet + 2 ) ); // IP.total length . fragment size . TOCHECK is correct
	if ( f_pkt_total_length_B > ETHERNET_MTU )
	{
		DEFRAGED_UDPS().ipv4_bad_structure++; /*error situation*/
		return errCanceled;
	}

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
	//uint32_t src = *( uint32_t * )( packet + 12 ); // src ip
	//uint32_t dst = *( uint32_t * )( packet + 16 ); // dst ip
	uint8_t proto = packet[ 9 ]; // proto used
	PAD( sizeof( int ) - sizeof( uint16_t ) );
	if ( proto != 17 )
	{
		DEFRAGED_UDPS().ipv4_bad_structure++; /*error situation*/
		return errCanceled; /*17 means udp in ipv4 layer*/
	}
	STACK_ALIGNED_CHKPT( 14041015 )

	DEFRAGED_UDPS().L1Cache_ipv4s++;

/*layer 4 Transport (UDP)*/
	/* Pointer to payload (L4 header start). */
	buffer datagram = packet + pkt_hdr_len_B;
	uint16_t dg_hdr_pyload_len_B = 0;
	uint16_t udp_dg_len_B = 0;
	PAD( sizeof(buffer) - sizeof(uint16_t) - sizeof(uint16_t) );
	buffer udp_payload_buf = NULL;
	if ( !frag_offset )
	{
		dg_hdr_pyload_len_B = f_pkt_total_length_B > pkt_hdr_len_B ? ( f_pkt_total_length_B - pkt_hdr_len_B ) : 0;

		if ( dg_hdr_pyload_len_B < 1 || dg_hdr_pyload_len_B > ETHERNET_MTU )
		{
			DEFRAGED_UDPS().ipv4_bad_structure++; /*error situation*/
			return errCanceled;
		}
		if ( !( dg_hdr_pyload_len_B >= UDP_LAYER_HDR_SZ_B ) )
		{
			DEFRAGED_UDPS().ipv4_bad_structure++; /*error situation*/
			return errCanceled; // UDP check
		}
		udp_dg_len_B = ntohs( *( uint16_t * )( datagram + 4 ) )/* - sizeof(struct udphdr)*/;
		udp_payload_buf = packet + pkt_hdr_len_B + UDP_LAYER_HDR_SZ_B;
	}
	else
	{
		udp_payload_buf = packet + pkt_hdr_len_B;
	}
	
	dfrg_udp_metadata udp_spec /*= {0}*/ ; // just think about execution speed. because here we are at getter side of rign so we should get as much as possible
	udp_spec.hdr.udp_pkt_id = ntohs( *( uint16_t * )( packet + 4 ) ); // IP ID field -> upd pkt id

	DEFRG_LOCK_LINE( __lock( pb , udp_spec.hdr.udp_pkt_id ) );

	//last_pos += sprintf( ( char * )__file_map + last_pos , "(%d)" , (int)udp_pkt_id );

	udp_spec.mark_memory = MARKER_MEM; // to check memory validity
	//udp_spec.hdr.srcIP = src;
	//udp_spec.hdr.dstIP = dst;
	udp_spec.proto = proto;
	//gettimeofday( &udp_spec.hdr.tm , NULL );
	udp_spec.hdr.tm = h->ts;

	/* Case 1: not fragmented => whole udp packet */
	if ( !frag_offset && !more )
	{
		if ( DEFRAGED_UDPS().ids[ udp_spec.hdr.udp_pkt_id ].dirty )
		{
			DEFRAGED_UDPS().no_dfrg_id_overlaped++;
		}
		//memset( &DEFRAGED_UDPS().ids[ udp_spec.hdr.udp_pkt_id ] , 0 , sizeof( DEFRAGED_UDPS().ids[ udp_spec.hdr.udp_pkt_id ] ) ); /*too heavy operation and not necesity for every situation*/
		MEMSET_ZERO_O( &DEFRAGED_UDPS().ids[ udp_spec.hdr.udp_pkt_id ].ring_addr );
		DEFRAGED_UDPS().ids[ udp_spec.hdr.udp_pkt_id ].last_pos = 0;
		DEFRAGED_UDPS().ids[ udp_spec.hdr.udp_pkt_id ].tm = udp_spec.hdr.tm;
		//my_hdr.frag_offset = 0;
		udp_spec.data_progress_B = udp_spec.data_length_B = udp_dg_len_B - sizeof(struct udphdr);
		DEFRAGED_UDPS().ids[ udp_spec.hdr.udp_pkt_id ].fragment_delay_msec = CFG().ipv4_reassembly_timeout_msec;

		status d_error = cbuf_pked_push( &pb->comm.preq.raw_xudp_cache , ( buffer )&udp_spec , sizeof( udp_spec ) , sizeof( udp_spec ) , NULL , false );
		if ( d_error != errOK )
		{
			MEMSET_ZERO_O( &DEFRAGED_UDPS().ids[ udp_spec.hdr.udp_pkt_id ] );
			__unlock( pb , udp_spec.hdr.udp_pkt_id );
			DEFRAGED_UDPS().kernel_error++;
			return d_error;
		}
		d_error = cbuf_pked_push( &pb->comm.preq.raw_xudp_cache , udp_payload_buf , udp_spec.data_progress_B , udp_spec.data_length_B , NULL , true );
		if ( d_error != errOK )
		{
			MEMSET_ZERO_O( &DEFRAGED_UDPS().ids[ udp_spec.hdr.udp_pkt_id ] );
			__unlock( pb , udp_spec.hdr.udp_pkt_id );
			DEFRAGED_UDPS().kernel_error++;
			return d_error;
		}
		DEFRAGED_UDPS().ids[ udp_spec.hdr.udp_pkt_id ].data_length_B = DEFRAGED_UDPS().ids[ udp_spec.hdr.udp_pkt_id ].data_progress_B = udp_spec.data_length_B;
		__unlock( pb , udp_spec.hdr.udp_pkt_id );
		DEFRAGED_UDPS().L1Cache_cached_ipv4s++;
		sem_post( &DEFRAGED_UDPS().gateway ); // some udp packet complete
		return d_error;
	}

	/*
	in fast buffer it assumes that first udp fragment specify whole udp packet size and it allocate that
	then arrival of next part copy on empty blocked that previuosly allocated. so if first part receive first
	then correct allocation should happened. but if next part arrived sooner that first part then i should allocate whole packet in pessimistic condition.
	*/

	/* Case 2: fragmented => first part of fragmented udp */
	if ( !frag_offset && more )
	{
		//uint16_t payload_len_B = udp_dg_len_B - (uint16_t)sizeof(struct udphdr);
		//uint16_t seg_payload_len_B = frag_len_B - (uint16_t)sizeof(struct udphdr);

		udp_spec.frag_offset = 0;
		udp_spec.data_length_B = udp_dg_len_B - (uint16_t)sizeof(struct udphdr);
		udp_spec.data_progress_B = frag_len_B - (uint16_t)sizeof(struct udphdr); // use this field at defrag step and it is sign that say udp received completly

		#ifdef CCH
		#undef CCH
		#endif
		#define CCH DEFRAGED_UDPS().ids[ udp_spec.hdr.udp_pkt_id ]

		if ( CCH.last_pos != 0 )
		{
			DEFRAGED_UDPS().unordered_ipv4++;
		}

		/* first of all set time for ip id */
		if ( CCH.dirty )
		{
			// it is possible that second part received first after first part
			// some times in test one udp packet received continuously so i should be prepare for that
			if ( timeval_diff_ms( &udp_spec.hdr.tm , &CCH.tm ) > CFG().udp_id_valid_until_timeout_msec )
			{
				DEFRAGED_UDPS().no_dfrg_id_timeout++;
				MEMSET_ZERO_O( &CCH );
				//last_pos += sprintf( ( char * )__file_map + last_pos , "(MEMSET_ZERO_O %d)" , (int)udp_pkt_id );
			}
			if ( CCH.last_pos >= MAXIMUM_FRAGMENT_MADE )
			{
				DEFRAGED_UDPS().no_dfrg_max_part_pos_exced++;
				MEMSET_ZERO_O( &CCH ); // maybe second part arrived first
				//last_pos += sprintf( ( char * )__file_map + last_pos , "(MEMSET_ZERO_O %d)" , (int)udp_pkt_id );
			}
			if ( CCH.data_length_B && CCH.data_progress_B > CCH.data_length_B )
			{
				DEFRAGED_UDPS().no_dfrg_pylod_sz_exced++;
				MEMSET_ZERO_O( &CCH );
			}
		}
		if ( !CCH.dirty ) /*very important*/
		{
			CCH.tm = udp_spec.hdr.tm;
		}

		CCH.fragment_delay_msec = MAX( 1 , ( uint16_t )ceil( 1.0 * ( SIZE_ETHERNET + sizeof( struct udphdr ) + udp_spec.data_length_B ) / ( ETHERNET_MTU - IP_header ) ) ) * CFG().ipv4_reassembly_timeout_msec; // ceil( (IP + UDP + payload) / (MTU - IP_header) )

		size_t hdr_addr , pyld_addr; // size in memory
		status d_error = cbuf_pked_push( &pb->comm.preq.raw_xudp_cache , ( buffer )&udp_spec , sizeof( udp_spec ) , sizeof( udp_spec ) , &hdr_addr , false ); // first write header to buff
		if ( d_error != errOK )
		{
			MEMSET_ZERO_O( &CCH );
			__unlock( pb , udp_spec.hdr.udp_pkt_id );
			DEFRAGED_UDPS().kernel_error++;
			return d_error;
		}
		// for increasing speed of writing it should be just copy into buffer then at reading read and copy on their offset
		d_error = cbuf_pked_push( &pb->comm.preq.raw_xudp_cache , udp_payload_buf , udp_spec.data_progress_B , udp_spec.data_progress_B , &pyld_addr , true ); // for first one allocate more space then in read defrag by copyting them in their place
		if ( d_error != errOK )
		{
			MEMSET_ZERO_O( &CCH );
			__unlock( pb , udp_spec.hdr.udp_pkt_id );
			DEFRAGED_UDPS().kernel_error++;
			return d_error;
		}
		else
		{
			DEFRAGED_UDPS().L1Cache_cached_ipv4s++;

			CCH.data_length_B = udp_spec.data_length_B;
			//CCH.srcIP = udp_spec.hdr.srcIP;
			//CCH.dstIP = udp_spec.hdr.dstIP;
			// i prefer that first udp part keep at first index ( CCH.ring_addr[ 0 ] )
			CCH.ring_addr[ 0 ][ 0 ] = hdr_addr; // So, the ring handles translation to memory
			CCH.ring_addr[ 0 ][ 1 ] = pyld_addr;

			CCH.data_progress_B += (uint16_t)udp_spec.data_progress_B; // maybe firt part arrive later that second part

			__unlock( pb , udp_spec.hdr.udp_pkt_id );

			// CCH.last_pos++; main always in last_pos-> 0 . main does not matter because copy to output
			if ( CCH.data_length_B && CCH.data_progress_B == CCH.data_length_B )
			{
				sem_post( &DEFRAGED_UDPS().gateway ); // some udp packet complete
			}
		}
		return d_error;
	}

	/* Case 3: fragmented => other part of fragmented udp */
	if ( frag_offset )
	{
		if ( frag_offset <= sizeof( struct udphdr ) )
		{
			MEMSET_ZERO_O( &CCH );
			__unlock( pb , udp_spec.hdr.udp_pkt_id );
			DEFRAGED_UDPS().ipv4_bad_structure++;
			return errGeneral;
		}

		udp_spec.frag_offset = frag_offset - sizeof(struct udphdr);
		//frag_offset -= sizeof(struct udphdr);
		//uint16_t seg_payload_len_B = frag_len_B /* - sizeof(struct udphdr)*/;
		udp_spec.data_progress_B = frag_len_B /* - sizeof(struct udphdr)*/; // be zero it means this is partial data udp

		if ( CCH.last_pos * ETHERNET_MTU + udp_spec.data_progress_B > BUFFER_SIZE )
		{
			MEMSET_ZERO_O( &CCH );
			DEFRAGED_UDPS().ipv4_bad_structure++;
			__unlock( pb , udp_spec.hdr.udp_pkt_id );
			return errGeneral;
		}

		/* first of all set time for ip id */
		if ( CCH.dirty )
		{
			// it is possible that second part received first after first part
			// some times in test one udp packet received continuously so i should be prepare for that
			if ( timeval_diff_ms( &udp_spec.hdr.tm , &CCH.tm ) > CFG().udp_id_valid_until_timeout_msec )
			{
				DEFRAGED_UDPS().no_dfrg_id_timeout++;
				MEMSET_ZERO_O( &CCH );
				//last_pos += sprintf( ( char * )__file_map + last_pos , "(MEMSET_ZERO_O %d)" , (int)udp_pkt_id );
			}
			if ( CCH.last_pos >= MAXIMUM_FRAGMENT_MADE )
			{
				DEFRAGED_UDPS().no_dfrg_max_part_pos_exced++;
				MEMSET_ZERO_O( &CCH ); // maybe second part arrived first
				//last_pos += sprintf( ( char * )__file_map + last_pos , "(MEMSET_ZERO_O %d)" , (int)udp_pkt_id );
			}
			if ( CCH.data_length_B && CCH.data_progress_B > CCH.data_length_B )
			{
				DEFRAGED_UDPS().no_dfrg_pylod_sz_exced++;
				MEMSET_ZERO_O( &CCH );
			}
		}
		if ( !CCH.dirty ) /*very important*/
		{
			CCH.tm = udp_spec.hdr.tm;
		}

		if ( !CCH.fragment_delay_msec )
		{
			CCH.fragment_delay_msec = CFG().udp_id_valid_until_timeout_msec; // ceil( (IP + UDP + payload) / (MTU - IP_header) )
		}

		//udp_spec.frag_offset = frag_offset;
		udp_spec.data_length_B = 0; // not usable in here . because this is middle or terminate packet
		//udp_spec.data_progress_B = seg_payload_len_B; // be zero it means this is partial data udp
		size_t hdr_addr , pyld_addr;
		status d_error = cbuf_pked_push( &pb->comm.preq.raw_xudp_cache , ( buffer )&udp_spec , sizeof( udp_spec ) , sizeof( udp_spec ) , &hdr_addr , false );
		if ( d_error != errOK )
		{
			MEMSET_ZERO_O( &CCH );
			__unlock( pb , udp_spec.hdr.udp_pkt_id );
			DEFRAGED_UDPS().kernel_error++;
			return d_error;
		}
		d_error = cbuf_pked_push( &pb->comm.preq.raw_xudp_cache , udp_payload_buf , udp_spec.data_progress_B , udp_spec.data_progress_B , &pyld_addr , true ); // for first one allocate more space then in read defrag by copyting them in their place
		if ( d_error != errOK )
		{
			MEMSET_ZERO_O( &CCH );
			__unlock( pb , udp_spec.hdr.udp_pkt_id );
			DEFRAGED_UDPS().kernel_error++;
			return d_error;
		}
		else
		{
			DEFRAGED_UDPS().L1Cache_cached_ipv4s++;

			//CCH.data_length_B = my_hdr.data_length_B;
			//CCH.srcIP = udp_spec.hdr.srcIP;
			//CCH.dstIP = udp_spec.hdr.dstIP;
			CCH.ring_addr[ CCH.last_pos + 1 ][ 0 ] = hdr_addr; // So, the ring handles translation to memory
			CCH.ring_addr[ CCH.last_pos + 1 ][ 1 ] = pyld_addr;
			CCH.data_progress_B += udp_spec.data_progress_B;
			CCH.last_pos++; // extra segment from pos >=1

			__unlock( pb , udp_spec.hdr.udp_pkt_id );

			if ( CCH.data_length_B && CCH.data_progress_B == CCH.data_length_B )
			{
				sem_post( &DEFRAGED_UDPS().gateway ); // some udp packet complete
			}
			#undef CCH
		}
		return d_error;
	}
	__unlock( pb , udp_spec.hdr.udp_pkt_id );
	return errCanceled;
}

// get udp packet payload plus extra hdr that not usabale . called in slower mode side of ring( consumer )
status poped_defraged_packet( void_p src_pb , OUTcpy buffer out_buf , size_t out_buf_sz , OUTx size_t * out_len_B , OUTcpy rdy_udp_hdr_t * out_hdr )
{
	AB * pb = ( AB * )src_pb;
	G * _g = TO_G( pb->cpy_cfg.m.m.temp_data._pseudo_g );
	
	status d_error;
	dfrg_udp_metadata tmp_hdr; // top udp packet part in head of buffer
	size_t hdr_sz = 0;

	/*read header(dfrg_udp_metadata) from buffer*/
	if ( ( d_error = cbuf_pked_pop( &pb->comm.preq.raw_xudp_cache , &tmp_hdr , sizeof( tmp_hdr ) , &hdr_sz , (long)CFG().socket_def_timeout_sec , true ) ) != errOK )
	{
		if ( d_error != errTimeout )
		{
			DEFRAGED_UDPS().kernel_error++;
			return d_error;
		}
		// timeout
		return d_error;
	}

	if ( tmp_hdr.mark_memory != MARKER_MEM ) // very error prone but fast
	{
		DEFRAGED_UDPS().bad_buffer_err++;
		goto _ignore;
	}

	#ifdef CCH
	#undef CCH
	#endif
	#define CCH DEFRAGED_UDPS().ids[ tmp_hdr.hdr.udp_pkt_id ]

	DEFRG_LOCK_LINE( __lock( pb , tmp_hdr.hdr.udp_pkt_id ) );

	/*because of the main part, other related parts cleaned so they should ignored*/
	if ( !CCH.dirty )
	{
		__unlock( pb , tmp_hdr.hdr.udp_pkt_id );
		// TODO . check with counter
		goto _ignore;
	}

	/*one small and complete packet(less than MTU) fetched*/
	if ( tmp_hdr.data_length_B && tmp_hdr.data_progress_B == tmp_hdr.data_length_B ) // one complete packet
	{
		//__lock( pb , tmp_hdr.hdr.udp_pkt_id );

		if ( !CCH.data_length_B || CCH.data_progress_B != CCH.data_length_B )
		{
			//__unlock( pb , tmp_hdr.hdr.udp_pkt_id ); // temporarily . make safe zone for writer

			/*try to make some delay that cause other part of the packet arrived*/
			while ( !CCH.data_length_B || CCH.data_progress_B != CCH.data_length_B )
			{
				//if ( CCH.data_length_B && CCH.data_progress_B >= CCH.data_length_B ) break;

				struct timespec ts;
				if ( !clock_gettime( CLOCK_REALTIME , &ts ) )
				{
					long long nsec = ts.tv_nsec;
					nsec += CCH.fragment_delay_msec * 1000000;
					ts.tv_sec += nsec / 1000000000;
					ts.tv_nsec += nsec % 1000000000; // 100 is good mili

					if ( sem_timedwait( &DEFRAGED_UDPS().gateway , &ts ) < 0 ) // wait for open signal . decrements the semaphore . if zero wait
					{
						if ( errno == ETIMEDOUT )
						{
							break;
						}
					}
				}

				if ( CCH.data_length_B && CCH.data_progress_B >= CCH.data_length_B ) break;

				struct timeval tnow;
				if ( !gettimeofday( &tnow , NULL ) )
				{
					if ( timeval_diff_ms( &tmp_hdr.hdr.tm , &tnow ) > ( double )CCH.fragment_delay_msec )
					{
						break;
					}
				}
			}

			//__lock( pb , tmp_hdr.hdr.udp_pkt_id );
		}

		/*if every part of packet not recieved then reset block of them*/
		if ( !CCH.data_length_B || CCH.data_progress_B != CCH.data_length_B )
		{
			if ( !CCH.data_length_B )
			{
				DEFRAGED_UDPS().no_dfrg_data_length_zero++;
			}
			else
			{
				//if ( CCH.dirty )
				{
					if ( CCH.data_progress_B < CCH.data_length_B )
					{
						DEFRAGED_UDPS().no_dfrg_less_pylod++;
					}
					else if ( CCH.data_progress_B == CCH.data_length_B )
					{
						DEFRAGED_UDPS().no_dfrg_eq_pylod++;
					}
					else if ( CCH.data_progress_B > CCH.data_length_B )
					{
						DEFRAGED_UDPS().no_dfrg_more_pylod++;
					}
				}
			}
			MEMSET_ZERO_O( &CCH );
			__unlock( pb , tmp_hdr.hdr.udp_pkt_id );
			goto _ignore;
		}


		if ( out_hdr ) MEMCPY( out_hdr , &tmp_hdr.hdr );
		d_error = cbuf_pked_pop( &pb->comm.preq.raw_xudp_cache , out_buf , 0/*no exp*/ , out_len_B , (long)CFG().socket_def_timeout_sec , false ); // most of the packet get here na dis normal
		if ( d_error )
		{
			if ( d_error != errTimeout )
			{
				DEFRAGED_UDPS().kernel_error++;
			}
			DEFRAGED_UDPS().bad_buffer_err++;
			MEMSET_ZERO_O( &CCH );
			__unlock( pb , tmp_hdr.hdr.udp_pkt_id );
			return d_error;
		}
		
		DEFRAGED_UDPS().L1Cache_tried_to_defraged_ipv4++;
		MEMSET_ZERO_O( &CCH );
		__unlock( pb , tmp_hdr.hdr.udp_pkt_id );
		goto _update_stat;
	}

	// until this line simple small packet sent . now we have hdr that says next buf block situation
	// now we continue with this basic assumption that udp packet fragmented part may arrive without any order
	// there is addinional assumption that udp generator at least make 5 udp fragment so it can be used in dictionary policy to make it fast
	// we know whick udp packet code to pop up

	/*
	* sometimes its possible that last packet comes first
	*/

	//WARNING( tmp_hdr.data_progress_B ); // it is not possible not no receive any payload

	if ( CCH.dirty /* && CCH.data_length_B && CCH.data_progress_B != CCH.data_length_B*/ )
	{
		if ( !CCH.data_length_B || CCH.data_progress_B != CCH.data_length_B )
		{
			__unlock( pb , tmp_hdr.hdr.udp_pkt_id ); // temporarily . make safe zone for writer

			/*try to make some delay that cause other part of the packet arrived*/
			while( !CCH.data_length_B || CCH.data_progress_B != CCH.data_length_B )
			{
				//if ( CCH.data_length_B && CCH.data_progress_B >= CCH.data_length_B ) break;

				struct timespec ts;
				if ( !clock_gettime( CLOCK_REALTIME , &ts ) )
				{
					long long nsec = ts.tv_nsec;
					nsec += CCH.fragment_delay_msec * 1000000;
					ts.tv_sec += nsec / 1000000000;
					ts.tv_nsec += nsec % 1000000000; // 100 is good mili

					if ( sem_timedwait( &DEFRAGED_UDPS().gateway , &ts ) < 0 ) // wait for open signal . decrements the semaphore . if zero wait
					{
						if ( errno == ETIMEDOUT )
						{
							break;
						}
					}
				}

				if ( CCH.data_length_B && CCH.data_progress_B >= CCH.data_length_B ) break;

				struct timeval tnow;
				if ( !gettimeofday( &tnow , NULL ) )
				{
					if ( timeval_diff_ms( &tmp_hdr.hdr.tm , &tnow ) > ( double )CCH.fragment_delay_msec )
					{
						break;
					}
				}
			}

			__lock( pb , tmp_hdr.hdr.udp_pkt_id );
		}
	}

	/*now it is time to make decisive and final decision so luck and prevent write make change to packet*/
	//DEFRG_LOCK_LINE( __lock( pb , tmp_hdr.hdr.udp_pkt_id ) );

	/*if every part of packet not recieved then reset block of them*/
	if ( !CCH.data_length_B || CCH.data_progress_B != CCH.data_length_B )
	{
		if ( !CCH.data_length_B )
		{
			DEFRAGED_UDPS().no_dfrg_data_length_zero++;
		}
		else
		{
			//if ( CCH.dirty )
			{
				if ( CCH.data_progress_B < CCH.data_length_B )
				{
					DEFRAGED_UDPS().no_dfrg_less_pylod++;
				}
				else if ( CCH.data_progress_B == CCH.data_length_B )
				{
					DEFRAGED_UDPS().no_dfrg_eq_pylod++;
				}
				else if ( CCH.data_progress_B > CCH.data_length_B )
				{
					DEFRAGED_UDPS().no_dfrg_more_pylod++;
				}
			}
		}
		MEMSET_ZERO_O( &CCH );
		__unlock( pb , tmp_hdr.hdr.udp_pkt_id );
		goto _ignore;
	}

	// until we pop, no one can push another item if buf is full
	// first find main packet that has free space
	// we have assumption that cycle of udp packet id take too long and it is possible to process udp before next equl id ruined it
	// so there is now time for dirty stuff and ugly thing implied on ring buffer . just for performance

	for ( uint8_t ipos = 0 ; ipos <= CCH.last_pos ; ipos++ )
	{
		dfrg_udp_metadata segm;
		// is bad mem . should not be proc because first one in grg peak . first one is main so others must be not
		if ( ( d_error = cbuf_pked_blindcopy( &pb->comm.preq.raw_xudp_cache , &segm , sizeof( segm ) , CCH.ring_addr[ ipos ][ 0 ] ) ) != errOK
			|| segm.mark_memory != MARKER_MEM || !segm.data_progress_B ) // hdr
		{
			DEFRAGED_UDPS().bad_buffer_err++;
			MEMSET_ZERO_O( &CCH );
			__unlock( pb , tmp_hdr.hdr.udp_pkt_id );
			goto _ignore;
		}

		if ( ipos == 0 )
		{
			if ( out_len_B ) *out_len_B = segm.data_length_B;
			if ( out_hdr ) MEMCPY( out_hdr , &segm.hdr );
		}

		if ( segm.frag_offset > out_buf_sz )
		{
			DEFRAGED_UDPS().bad_buffer_err++;
			MEMSET_ZERO_O( &CCH );
			__unlock( pb , tmp_hdr.hdr.udp_pkt_id );
			goto _ignore;
		}
		
		if ( ( d_error = cbuf_pked_blindcopy( &pb->comm.preq.raw_xudp_cache , out_buf + segm.frag_offset , out_buf_sz - segm.frag_offset , CCH.ring_addr[ ipos ][ 1 ] ) ) != errOK ) // data
		{
			DEFRAGED_UDPS().bad_buffer_err++;
			MEMSET_ZERO_O( &CCH );
			__unlock( pb , tmp_hdr.hdr.udp_pkt_id );
			goto _ignore;
		}
		
		DEFRAGED_UDPS().L1Cache_tried_to_defraged_ipv4++;
	}

	MEMSET_ZERO_O( &CCH );
	__unlock( pb , tmp_hdr.hdr.udp_pkt_id );
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
