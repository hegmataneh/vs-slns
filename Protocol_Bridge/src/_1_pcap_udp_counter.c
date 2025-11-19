#define Uses_MARK_START_THREAD
#define Uses_WARNING
#define Uses_FREE_DOUBLE_PTR
#define Uses_errno
#define Uses_globals
#define Uses_Bridge
#define Uses_INIT_BREAKABLE_FXN
#define Uses_pcap
#include <Protocol_Bridge.dep>

_CALLBACK_FXN void quit_interrupt_dist_pcap_udp_counter( pass_p src_pb , long v )
{
	AB * pb = ( AB * )src_pb;
	if ( pb->comm.acts.p_pcap_udp_counter->pcp_handle )
	{
		pcap_breakloop( pb->comm.acts.p_pcap_udp_counter->pcp_handle ); // in case we're inside pcap_loop
		pcap_close( pb->comm.acts.p_pcap_udp_counter->pcp_handle );
	}
}

_CALLBACK_FXN void handle_pcap_udp_counter( u_char * src_pb , const struct pcap_pkthdr * hdr , const u_char * packet )
{
	AB * pb = ( AB * )src_pb;
	G * _g = pb->cpy_cfg.m.m.temp_data._pseudo_g;

	( void )hdr;
	( void )packet;

	if ( pb->comm.preq.stop_receiving )
	{
		pb->comm.preq.receive_stoped = true;
	}
	else
	{
		gettimeofday( &pb->stat.round_zero_set.t_end , NULL );
		pb->stat.round_zero_set.udp.total_udp_get_count++;
		pb->stat.round_zero_set.udp.total_udp_get_byte += 1; // TODO . actual byte
		pb->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count++;
		pb->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes += 1;
		pb->stat.round_zero_set.udp_get_data_alive_indicator++;
	}
}

_THREAD_FXN void_p proc_pcap_udp_counter( pass_p src_pb )
{
	INIT_BREAKABLE_FXN();
	AB * pb = ( AB * )src_pb;
	G * _g = pb->cpy_cfg.m.m.temp_data._pseudo_g;
	
#ifdef ENABLE_LOG_THREADS
	distributor_publish_x3long( &_g->distributors.bcast_thread_startup , (long)pthread_self() , trdn_proc_pcap_udp_counter , NP , _g );
	__attribute__( ( cleanup( thread_goes_out_of_scope ) ) ) pthread_t trd_id = pthread_self();
#ifdef ENABLE_USE_DBG_TAG
	MARK_START_THREAD();
#endif
#endif

	//WARNING( pb->cpy_cfg.m.m.maintained.in_count == 1 );
	//char * dev = pb->cpy_cfg.m.m.maintained.in->data.UDP_origin_interface;
	char errbuf[ PCAP_ERRBUF_SIZE ] = { 0 };
	struct bpf_program fp;
	bpf_u_int32 net = 0 , mask = 0;
	pb->comm.acts.p_pcap_udp_counter->pcp_handle = NULL;

	int clusterd_cnt;
	strings interface_filter = NULL;
	strings port_filter = NULL;
	compile_udps_config_for_pcap_filter( pb , &clusterd_cnt , &interface_filter , &port_filter );

	WARNING( clusterd_cnt == 1 );

	//MM_BREAK_IF( !( dev = pcap_lookupdev( errbuf ) ) , errDevice , 0 , "Couldn't find default device: %s" , errbuf );
	MM_FMT_BREAK_IF( pcap_lookupnet( interface_filter[ 0 ] , &net , &mask , errbuf ) == -1 , errDevice , 1 , "use correct interface %s\n" , errbuf );

	// Open in promiscuous mode, snapshot length 65535, no timeout (0 means immediate)
	MM_FMT_BREAK_IF( !( pb->comm.acts.p_pcap_udp_counter->pcp_handle = pcap_open_live( interface_filter[ 0 ] , SNAP_LEN , 1 , 1000 , errbuf ) ) , errDevice , 1 , "exe by pcap prmit usr %s: %s\n" , interface_filter[ 0 ] , errbuf );

	int fd = pcap_get_selectable_fd( pb->comm.acts.p_pcap_udp_counter->pcp_handle );
	int busy_poll_time = 50;  // microseconds per syscall spin budget
	M_BREAK_IF( setsockopt( fd , SOL_SOCKET , SO_BUSY_POLL , &busy_poll_time , sizeof( busy_poll_time ) ) < 0 , errSocket , 2 );

	//const char * filter = pb->cpy_cfg.m.m.maintained.in->data.UDP_origin_ports;

	// Compile and apply filter
	MM_FMT_BREAK_IF( pcap_compile( pb->comm.acts.p_pcap_udp_counter->pcp_handle , &fp , port_filter[ 0 ] , 1 , mask ) == -1 , errDevice , 2 , "Couldn't parse filter %s\n" , pcap_geterr( pb->comm.acts.p_pcap_udp_counter->pcp_handle ) );
	MM_FMT_BREAK_IF( pcap_setfilter( pb->comm.acts.p_pcap_udp_counter->pcp_handle , &fp ) == -1 , errDevice , 3 , "Couldn't install filter %s\n" , pcap_geterr( pb->comm.acts.p_pcap_udp_counter->pcp_handle ) );

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

	distributor_publish_long( &_g->distributors.bcast_pb_udp_connected , 0 , ( pass_p )pb );
	
	distributor_subscribe_withOrder( &_g->distributors.bcast_quit , SUB_LONG , SUB_FXN( quit_interrupt_dist_pcap_udp_counter ) , pb , stop_input_udp );

#ifdef ENABLE_USE_DBG_TAG
	MARK_LINE();
#endif

	// Capture indefinitely
	MM_FMT_BREAK_IF( pcap_loop( pb->comm.acts.p_pcap_udp_counter->pcp_handle , -1 , handle_pcap_udp_counter , src_pb ) == -1 , errDevice , 3 , "pcap_loop failed: %s\n" , pcap_geterr( pb->comm.acts.p_pcap_udp_counter->pcp_handle ) );
#ifdef ENABLE_USE_DBG_TAG
	MARK_LINE();
#endif
	BREAK_OK( 4 ); // clean every thing

	BEGIN_RET
	case 4:
	{
		pcap_breakloop( pb->comm.acts.p_pcap_udp_counter->pcp_handle ); // in case we're inside pcap_loop
		pcap_close( pb->comm.acts.p_pcap_udp_counter->pcp_handle );
		pb->comm.acts.p_pcap_udp_counter->pcp_handle = NULL;
		break;
	}
	case 3:
	{
		pcap_freecode( &fp );
	}
	case 2:
	{
		pcap_close( pb->comm.acts.p_pcap_udp_counter->pcp_handle );
		pb->comm.acts.p_pcap_udp_counter->pcp_handle = NULL;
	}
	case 1:
	{
		DIST_BRIDGE_FAILURE();
	}
	M_V_END_RET
	pb->comm.preq.receive_stoped = true;
	return NULL;
}
