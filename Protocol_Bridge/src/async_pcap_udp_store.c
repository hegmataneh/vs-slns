#define Uses_MARK_LINE
#define Uses_WARNING
#define Uses_FREE_DOUBLE_PTR
#define Uses_udphdr
#define Uses_errno
#define Uses_globals
#define Uses_Bridge
#define Uses_INIT_BREAKABLE_FXN
#include <Protocol_Bridge.dep>

_PRIVATE_FXN _CALLBACK_FXN void handle_pcap_udp_receiver( u_char * src_pb , const struct pcap_pkthdr * hdr , const u_char * packet )
{
	AB * pb = ( AB * )src_pb;
	if ( pb->comm.preq.stop_receiving )
	{
		pb->comm.preq.receive_stoped = true;
	}
	else
	{
		if ( distributor_publish_onedirectcall_3voidp( &pb->comm.preq.bcast_pcap_udp_pkt , ( void_p )src_pb , ( void_p )hdr , ( void_p )packet ) != errOK ) return; // dist udp packet
	}
}

_REGULAR_FXN status stablish_pcap_udp_connection( AB * pb , shrt_pth_t * shrtcut )
{
	INIT_BREAKABLE_FXN();
	G * _g = TO_G( pb->cpy_cfg.m.m.temp_data._pseudo_g );

	char errbuf[ PCAP_ERRBUF_SIZE ] = { 0 };
	struct bpf_program fp;
	bpf_u_int32 net = 0 , mask = 0;

	// TODO . handle multiple interface

	#ifdef ENABLE_PCAP_LOOP_PREQ
	int clusterd_cnt;
	strings interface_filter = NULL;
	strings port_filter = NULL;
	compile_udps_config_for_pcap_filter( pb , &clusterd_cnt , &interface_filter , &port_filter );

	WARNING( clusterd_cnt == 1 );

	//MM_FMT_BREAK_IF( !( dev = pcap_lookupdev( errbuf ) ) , errDevice , 0 , "Couldn't find default device: %s" , errbuf );
	MM_FMT_BREAK_IF( pcap_lookupnet( interface_filter[ 0 ] , &net , &mask , errbuf) == -1 , errDevice , 1 , "use correct interface %s\n" , errbuf);

	// Open in promiscuous mode, snapshot length 65535, no timeout (0 means immediate)
	MM_FMT_BREAK_IF( !( *shrtcut->pcp_handle = pcap_open_live( interface_filter[ 0 ] , 65535, 1, 1000 , errbuf) ) , errDevice , 1 , "exe by pcap prmit usr %s\n" , interface_filter[0] , errbuf);

	// Compile and apply filter
	MM_FMT_BREAK_IF( pcap_compile( *shrtcut->pcp_handle , &fp , port_filter[ 0 ] , 1 , mask) == -1 , errDevice , 2 , "Couldn't parse filter %s\n" , pcap_geterr(*shrtcut->pcp_handle));
	MM_FMT_BREAK_IF( pcap_setfilter( *shrtcut->pcp_handle , &fp ) == -1 , errDevice , 3 , "Couldn't install filter %s\n" , pcap_geterr( *shrtcut->pcp_handle ) );

	FREE_DOUBLE_PTR( interface_filter , clusterd_cnt );
	FREE_DOUBLE_PTR( port_filter , clusterd_cnt );

	pcap_freecode( &fp );

	// set a large buffer (e.g., 10 MB)
	//MM_FMT_BREAK_IF( pcap_set_buffer_size( handle , 1024 * 1024 ) != 0 , errDevice , 2 , "failed to set buffer size %s\n" , pcap_geterr( handle ) );

	if ( pb->stat.round_zero_set.t_begin.tv_sec == 0 && pb->stat.round_zero_set.t_begin.tv_usec == 0 )
	{
		gettimeofday( &pb->stat.round_zero_set.t_begin , NULL );
		gettimeofday( &pb->stat.round_zero_set.t_end , NULL );
	}
	for ( int iinp = 0 ; iinp < pb->udps_count ; iinp++ )
	{
		pb->udps[ iinp ].udp_connection_established = 1;
		distributor_publish_long( &_g->distributors.bcast_pb_udp_connected , 0 , ( pass_p )pb );
	}

	#ifdef ENABLE_USE_DBG_TAG
		MARK_LINE();
	#endif

	// Capture indefinitely
	MM_FMT_BREAK_IF( pcap_loop( *shrtcut->pcp_handle , -1 , handle_pcap_udp_receiver , ( pass_p )pb ) == -1 , errDevice , 3 , "pcap_loop failed: %s\n" , pcap_geterr( *shrtcut->pcp_handle ) );

	pcap_close( *shrtcut->pcp_handle );
	#endif
	*shrtcut->pcp_handle = NULL; // closed successfully
	pb->comm.preq.receive_stoped = true;
	
#ifdef ENABLE_USE_DBG_TAG
	MARK_LINE();
#endif

	BEGIN_RET
	// TODO . FREE_DOUBLE_PTR( interface_filter , clusterd_cnt );
	// FREE_DOUBLE_PTR( port_filter , clusterd_cnt );
	case 4:
	{
		pcap_breakloop( *shrtcut->pcp_handle ); // in case we're inside pcap_loop
		pcap_close( *shrtcut->pcp_handle );
		break;
	}
	case 3:
	{
		pcap_freecode( &fp );
	}
	case 2:
	{
		pcap_close( *shrtcut->pcp_handle );
	}
	case 1:
	{
		DIST_BRIDGE_FAILURE();
	}
	M_END_RET
}
