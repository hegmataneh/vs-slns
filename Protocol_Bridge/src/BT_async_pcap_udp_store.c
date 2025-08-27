#define Uses_stablish_pcap_udp_connection
#define Uses_signal
#define Uses_udphdr
#define Uses_errno
#define Uses_helper
#define Uses_Bridge
#define Uses_INIT_BREAKABLE_FXN
#include <Protocol_Bridge.dep>

pcap_t * pcap_udp_receiver_handle = NULL;

_PRIVATE_FXN void cleanup_and_exit_pcap_udp_receiver( int sig )
{
	if ( sig ) ( void )sig;
	if ( pcap_udp_receiver_handle )
	{
		pcap_breakloop( pcap_udp_receiver_handle ); // in case we're inside pcap_loop
		pcap_close( pcap_udp_receiver_handle );
	}
	exit( 0 );
}

_PRIVATE_FXN void handle_pcap_udp_receiver( u_char * src_pb , const struct pcap_pkthdr * hdr , const u_char * packet )
{
	AB * pb = ( AB * )src_pb;
	G * _g = pb->cpy_cfg.m.m.temp_data._g;
	ASSERT( pb->udps_count == 1 );
	AB_udp * udp = pb->udps; // caution . in this type of bridge udp conn must be just one

	const struct ip * ip_hdr;
	const struct udphdr * udp_hdr;
	const u_char * payload;

	int ip_header_len;
	int udp_header_len = sizeof( struct udphdr );
	int payload_len;

	//dump_buffer( ( const buffer )packet , hdr->len );

	//return;

	// Skip Ethernet header
	ip_hdr = ( struct ip * )( packet + SIZE_ETHERNET );
	ip_header_len = ip_hdr->ip_hl * 4;

	// UDP header follows IP header
	udp_hdr = ( struct udphdr * )( packet + SIZE_ETHERNET + ip_header_len );

	// Payload starts after UDP header
	payload = packet + SIZE_ETHERNET + ip_header_len + udp_header_len;
	payload_len = ntohs( udp_hdr->uh_ulen ) - udp_header_len;

	//payload_len = 1470; // HARD CODE . TODELETE

	if ( distributor_publish_buffer_int( &pb->trd.base.buffer_push_distributor , ( buffer )payload , payload_len ) != errOK ) return; // dist udp packet

	////if ( vcbuf_nb_push( &pb->trd.t.p_one2one_pcap2kernelDefaultStack_SF_thread->cbuf , ( const buffer )payload , payload_len ) != errOK ) return;

	////printf( " Payload (%d bytes): " , payload_len );
	////for ( int i = 0; i < payload_len; i++ )
	////{
	////	if ( payload[ i ] >= 32 && payload[ i ] <= 126 ) // printable ASCII
	////		putchar( payload[ i ] );
	////	else
	////		putchar( '.' );
	////}

	gettimeofday( &_g->stat.round_zero_set.t_end , NULL );

	_g->stat.round_zero_set.udp.total_udp_get_count++;
	_g->stat.round_zero_set.udp.total_udp_get_byte += 1;
	_g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count++;
	_g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes += 1;

	_g->stat.udp_get_data_alive_indicator++;

	udp->pk_tm.prev_access = udp->pk_tm.last_access;
	udp->pk_tm.last_access = time( NULL );
	if ( udp->pk_tm.prev_access > 0 )
	{
		udp->pk_tm.curr_packet_delay = difftime( udp->pk_tm.last_access , udp->pk_tm.prev_access );
		if ( udp->pk_tm.curr_packet_delay > udp->pk_tm.max_packet_delay )
		{
			udp->pk_tm.max_packet_delay = udp->pk_tm.curr_packet_delay;
			distributor_publish_int_double( &_g->stat.thresholds , MAX_UDP_PACKET_DELAY , udp->pk_tm.max_packet_delay );
		}
	}

}

_REGULAR_FXN status stablish_pcap_udp_connection( AB_udp * udp )
{
	INIT_BREAKABLE_FXN();
	AB * pb = udp->owner_pb;
	G * _g = pb->cpy_cfg.m.m.temp_data._g;

	char * dev = udp->__udp_cfg_pak->data.UDP_origin_interface;
	char errbuf[ PCAP_ERRBUF_SIZE ] = { 0 };
	struct bpf_program fp;
	//char filter_exp[] = "udp and port 1234";
	bpf_u_int32 net = 0 , mask = 0;
	pcap_t ** handle = &udp->handle;

	//MM_FMT_BREAK_IF( !( dev = pcap_lookupdev( errbuf ) ) , errGeneral , 0 , "Couldn't find default device: %s" , errbuf );
	MM_FMT_BREAK_IF( pcap_lookupnet( dev , &net , &mask , errbuf ) == -1 , errGeneral , 1 , "Warning: couldn't get netmask for device %s\n" , errbuf );

	// Open in promiscuous mode, snapshot length 65535, no timeout (0 means immediate)
	MM_FMT_BREAK_IF( !( *handle = pcap_open_live( dev , SNAP_LEN , 1 , 1000 , errbuf ) ) , errGeneral , 1 , "Couldn't open device %s: %s\n" , dev , errbuf );

	// Compile and apply filter
	MM_FMT_BREAK_IF( pcap_compile( *handle , &fp , udp->__udp_cfg_pak->data.UDP_origin_ports , 1 , mask ) == -1 , errGeneral , 2 , "Couldn't parse filter %s\n" , pcap_geterr( *handle ) );
	MM_FMT_BREAK_IF( pcap_setfilter( *handle , &fp ) == -1 , errGeneral , 3 , "Couldn't install filter %s\n" , pcap_geterr( *handle ) );

	pcap_freecode( &fp );

	pcap_udp_receiver_handle = *handle;
	signal( SIGINT , cleanup_and_exit_pcap_udp_receiver );
	signal( SIGTERM , cleanup_and_exit_pcap_udp_receiver );

	// set a large buffer (e.g., 10 MB)
	//MM_FMT_BREAK_IF( pcap_set_buffer_size( *handle , 1024 * 1024 ) != 0 , errGeneral , 2 , "failed to set buffer size %s\n" , pcap_geterr( *handle ) );

	if ( _g->stat.round_zero_set.t_begin.tv_sec == 0 && _g->stat.round_zero_set.t_begin.tv_usec == 0 )
	{
		gettimeofday( &_g->stat.round_zero_set.t_begin , NULL );
	}

	udp->udp_connection_established = 1;

	_g->stat.udp_connection_count++;
	_g->stat.total_retry_udp_connection_count++;

	// Capture indefinitely
	MM_FMT_BREAK_IF( pcap_loop( *handle , -1 , handle_pcap_udp_receiver , ( void_p )pb ) == -1 , errGeneral , 3 , "pcap_loop failed: %s\n" , pcap_geterr( *handle ) );

	BEGIN_RET
	case 4:
	{
		pcap_breakloop( *handle ); // in case we're inside pcap_loop
		pcap_close( *handle );
		break;
	}
	case 3:
	{
		pcap_freecode( &fp );
	}
	case 2:
	{
		pcap_close( *handle );
	}
	case 1:
	{
		_g->stat.round_zero_set.syscal_err_count++;
	}
	M_END_RET
}
