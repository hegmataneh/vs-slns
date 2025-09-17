#define Uses_FREE_DOUBLE_PTR
#define Uses_udphdr
#define Uses_errno
#define Uses_helper
#define Uses_Bridge
#define Uses_INIT_BREAKABLE_FXN
#include <Protocol_Bridge.dep>

_PRIVATE_FXN void handle_pcap_udp_receiver( u_char * src_pb , const struct pcap_pkthdr * hdr , const u_char * packet )
{
	AB * pb = ( AB * )src_pb;
	//G * _g = pb->cpy_cfg.m.m.temp_data._g;
	//AB_udp * udp = pb->udps; // caution . in this type of bridge udp conn must be just one

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

	//payload_len = 1; // HARD CODE . TODELETE

	if ( distributor_publish_buffer_int( &pb->trd.base.buffer_push_distributor , ( buffer )payload , payload_len , NULL ) != errOK ) return; // dist udp packet

	////if ( vcbuf_nb_push( &pb->trd.t.p_one2one_pcap2kernelDefaultStack_SF_thread->cbuf , ( const buffer )payload , payload_len ) != errOK ) return;

	////printf( " Payload (%d bytes): " , payload_len );
	////for ( int i = 0; i < payload_len; i++ )
	////{
	////	if ( payload[ i ] >= 32 && payload[ i ] <= 126 ) // printable ASCII
	////		putchar( payload[ i ] );
	////	else
	////		putchar( '.' );
	////}

	gettimeofday( &pb->stat.round_zero_set.t_end , NULL );

	pb->stat.round_zero_set.udp.total_udp_get_count++;
	pb->stat.round_zero_set.udp.total_udp_get_byte += payload_len;
	pb->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count++;
	pb->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes += payload_len;

	pb->stat.round_zero_set.udp_get_data_alive_indicator++;

}

_REGULAR_FXN status stablish_pcap_udp_connection( AB * pb , shrt_path * pth )
{
	INIT_BREAKABLE_FXN();
	G * _g = pb->cpy_cfg.m.m.temp_data._g;

	char errbuf[ PCAP_ERRBUF_SIZE ] = { 0 };
	struct bpf_program fp;
	bpf_u_int32 net = 0 , mask = 0;
	pcap_t *handle;

	// TODO . handle multiple interface

	int clusterd_cnt;
	strings interface_filter = NULL;
	strings port_filter = NULL;
	compile_udps_config_for_pcap_filter( pb , &clusterd_cnt , &interface_filter , &port_filter );

	WARNING( clusterd_cnt == 1 );

	//MM_FMT_BREAK_IF( !( dev = pcap_lookupdev( errbuf ) ) , errDevice , 0 , "Couldn't find default device: %s" , errbuf );
	MM_FMT_BREAK_IF( pcap_lookupnet( interface_filter[ 0 ] , &net , &mask , errbuf) == -1 , errDevice , 1 , "use correct interface %s\n" , errbuf);

	// Open in promiscuous mode, snapshot length 65535, no timeout (0 means immediate)
	MM_FMT_BREAK_IF( !( handle = pcap_open_live( interface_filter[ 0 ] , SNAP_LEN , 1 , 1000 , errbuf) ) , errDevice , 1 , "exe by pcap prmit usr %s\n" , interface_filter[0] , errbuf);

	// Compile and apply filter
	MM_FMT_BREAK_IF( pcap_compile( handle , &fp , port_filter[ 0 ] , 1 , mask) == -1 , errDevice , 2 , "Couldn't parse filter %s\n" , pcap_geterr(handle));
	MM_FMT_BREAK_IF( pcap_setfilter( handle , &fp ) == -1 , errDevice , 3 , "Couldn't install filter %s\n" , pcap_geterr( handle ) );

	FREE_DOUBLE_PTR( interface_filter , clusterd_cnt );
	FREE_DOUBLE_PTR( port_filter , clusterd_cnt );

	pcap_freecode( &fp );
	WARNING( pth->handle );
	if ( pth->handle )
	{
		*pth->handle = handle;
	}

	// set a large buffer (e.g., 10 MB)
	//MM_FMT_BREAK_IF( pcap_set_buffer_size( handle , 1024 * 1024 ) != 0 , errDevice , 2 , "failed to set buffer size %s\n" , pcap_geterr( handle ) );

	if ( pb->stat.round_zero_set.t_begin.tv_sec == 0 && pb->stat.round_zero_set.t_begin.tv_usec == 0 )
	{
		gettimeofday( &pb->stat.round_zero_set.t_begin , NULL );
	}
	for ( int iinp = 0 ; iinp < pb->udps_count ; iinp++ )
	{
		pb->udps[ iinp ].udp_connection_established = 1;
		distributor_publish_int( &_g->distrbtor.pb_udp_connected_dist , 0 , ( pass_p )pb );
	}

	// Capture indefinitely
	MM_FMT_BREAK_IF( pcap_loop( handle , -1 , handle_pcap_udp_receiver , ( pass_p )pb ) == -1 , errDevice , 3 , "pcap_loop failed: %s\n" , pcap_geterr( handle ) );

	BEGIN_RET
	// TODO . FREE_DOUBLE_PTR( interface_filter , clusterd_cnt );
	// FREE_DOUBLE_PTR( port_filter , clusterd_cnt );
	case 4:
	{
		pcap_breakloop( handle ); // in case we're inside pcap_loop
		pcap_close( handle );
		break;
	}
	case 3:
	{
		pcap_freecode( &fp );
	}
	case 2:
	{
		pcap_close( handle );
	}
	case 1:
	{
		DIST_ERR();
	}
	M_END_RET
}
