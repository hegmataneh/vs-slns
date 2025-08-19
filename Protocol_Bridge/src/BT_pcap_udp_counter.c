#define Uses_signal
#define Uses_errno
#define Uses_helper
#define Uses_Bridge
#define Uses_INIT_BREAKABLE_FXN
#define Uses_pcap
#include <Protocol_Bridge.dep>

pcap_t * pcap_udp_counter_handle = NULL;

void cleanup_and_exit_pcap_udp_counter( int sig )
{
	if ( sig ) ( void )sig;
	if ( pcap_udp_counter_handle )
	{
		pcap_breakloop( pcap_udp_counter_handle ); // in case we're inside pcap_loop
		pcap_close( pcap_udp_counter_handle );
	}
	exit( 0 );
}

void handle_pcap_udp_counter( u_char * src_pb , const struct pcap_pkthdr * hdr , const u_char * packet )
{
	AB * pb = ( AB * )src_pb;
	G * _g = pb->cpy_cfg.m.m.temp_data._g;

	( void )hdr;
	( void )packet;

	gettimeofday( &_g->stat.round_zero_set.t_end , NULL );

	_g->stat.round_zero_set.udp.total_udp_get_count++;
	_g->stat.round_zero_set.udp.total_udp_get_byte += 1;
	_g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count++;
	_g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes += 1;

}

_THREAD_FXN void_p pcap_udp_counter_thread_proc( void_p src_pb )
{
	INIT_BREAKABLE_FXN();
	//static TWD twd = { 0 }; // static not allowed on shared fxn
	//if ( twd.threadId == 0 )
	//{
	//	twd.threadId = pthread_self();
	//	twd.cal = udp_counter_thread_proc; // self function address
	//	twd.callback_arg = src_pb;
	//}
	//if ( src_pb == NULL )
	//{
	//	return ( void_p )&twd;
	//}

	//AB_udp * pAB_udp = ( AB_udp * )src_AB_udp;
	AB * pb = ( AB * )src_pb;
	G * _g = pb->cpy_cfg.m.m.temp_data._g;

	ASSERT( pb->cpy_cfg.m.m.maintained.in_count == 1 );
	char * dev = pb->cpy_cfg.m.m.maintained.in->data.UDP_origin_interface;
	char errbuf[ PCAP_ERRBUF_SIZE ] = { 0 };
	struct bpf_program fp;
	//char filter_exp[] = "udp and port 1234";
	bpf_u_int32 net = 0 , mask = 0;
	pcap_t ** handle = &pb->trd.t.p_pcap_udp_counter_thread->handle;

	//MM_BREAK_IF( !( dev = pcap_lookupdev( errbuf ) ) , errGeneral , 0 , "Couldn't find default device: %s" , errbuf );
	MM_FMT_BREAK_IF( pcap_lookupnet( dev , &net , &mask , errbuf ) == -1 , errGeneral , 1 , "Warning: couldn't get netmask for device %s\n" , errbuf );

	// Open in promiscuous mode, snapshot length 65535, no timeout (0 means immediate)
	MM_FMT_BREAK_IF( !( *handle = pcap_open_live( dev , 65535 , 1 , 1000 , errbuf ) ) , errGeneral , 1 , "Couldn't open device %s: %s\n" , dev , errbuf );

	// Compile and apply filter
	MM_FMT_BREAK_IF( pcap_compile( *handle , &fp , pb->cpy_cfg.m.m.maintained.in->data.UDP_origin_ports , 1 , mask ) == -1 , errGeneral , 2 , "Couldn't parse filter %s\n" , pcap_geterr( *handle ) );
	MM_FMT_BREAK_IF( pcap_setfilter( *handle , &fp ) == -1 , errGeneral , 3 , "Couldn't install filter %s\n" , pcap_geterr( *handle ) );

	pcap_freecode( &fp );

	pcap_udp_counter_handle = *handle;
	signal( SIGINT , cleanup_and_exit_pcap_udp_counter );
	signal( SIGTERM , cleanup_and_exit_pcap_udp_counter );

	// set a large buffer (e.g., 10 MB)
	//MM_FMT_BREAK_IF( pcap_set_buffer_size( *handle , 1024 * 1024 ) != 0 , errGeneral , 2 , "failed to set buffer size %s\n" , pcap_geterr( *handle ) );

	if ( _g->stat.round_zero_set.t_begin.tv_sec == 0 && _g->stat.round_zero_set.t_begin.tv_usec == 0 )
	{
		gettimeofday( &_g->stat.round_zero_set.t_begin , NULL );
	}

	_g->stat.udp_connection_count++;
	_g->stat.total_retry_udp_connection_count++;

	// Capture indefinitely
	MM_FMT_BREAK_IF( pcap_loop( *handle , -1 , handle_pcap_udp_counter , src_pb ) == -1 , errGeneral , 3 , "pcap_loop failed: %s\n" , pcap_geterr( *handle ) );

	BREAK_OK( 4 );

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
	M_V_END_RET

	return NULL;
}
