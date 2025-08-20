#define Uses_signal
#define Uses_udphdr
#define Uses_errno
#define Uses_helper
#define Uses_Bridge
#define Uses_INIT_BREAKABLE_FXN
#include <Protocol_Bridge.dep>

pcap_t * pcap_udp_receiver_handle = NULL;

void cleanup_and_exit_pcap_udp_receiver( int sig )
{
	if ( sig ) ( void )sig;
	if ( pcap_udp_receiver_handle )
	{
		pcap_breakloop( pcap_udp_receiver_handle ); // in case we're inside pcap_loop
		pcap_close( pcap_udp_receiver_handle );
	}
	exit( 0 );
}

void handle_pcap_udp_receiver( u_char * src_pb , const struct pcap_pkthdr * hdr , const u_char * packet )
{
	AB * pb = ( AB * )src_pb;
	G * _g = pb->cpy_cfg.m.m.temp_data._g;

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

	if ( vcbuf_nb_push( &pb->trd.t.p_one2one_pcap2kernelDefaultStack_SF_thread->cbuf , ( const buffer )payload , payload_len ) != errOK ) return;

	//printf( " Payload (%d bytes): " , payload_len );
	//for ( int i = 0; i < payload_len; i++ )
	//{
	//	if ( payload[ i ] >= 32 && payload[ i ] <= 126 ) // printable ASCII
	//		putchar( payload[ i ] );
	//	else
	//		putchar( '.' );
	//}

	gettimeofday( &_g->stat.round_zero_set.t_end , NULL );

	_g->stat.round_zero_set.udp.total_udp_get_count++;
	_g->stat.round_zero_set.udp.total_udp_get_byte += 1;
	_g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count++;
	_g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes += 1;

}

void stablish_pcap_udp_connection( AB_udp * udp )
{
	INIT_BREAKABLE_FXN();
	AB * pb = udp->owner_pb;
	G * _g = pb->cpy_cfg.m.m.temp_data._g;

	char * dev = udp->__udp_cfg->UDP_origin_interface;
	char errbuf[ PCAP_ERRBUF_SIZE ] = { 0 };
	struct bpf_program fp;
	//char filter_exp[] = "udp and port 1234";
	bpf_u_int32 net = 0 , mask = 0;
	pcap_t ** handle = &udp->handle;

	//MM_FMT_BREAK_IF( !( dev = pcap_lookupdev( errbuf ) ) , errGeneral , 0 , "Couldn't find default device: %s" , errbuf );
	MM_FMT_BREAK_IF( pcap_lookupnet( dev , &net , &mask , errbuf ) == -1 , errGeneral , 1 , "Warning: couldn't get netmask for device %s\n" , errbuf );

	// Open in promiscuous mode, snapshot length 65535, no timeout (0 means immediate)
	MM_FMT_BREAK_IF( !( *handle = pcap_open_live( dev , SNAP_LEN, 1, 1000, errbuf ) ) , errGeneral , 1 , "Couldn't open device %s: %s\n" , dev , errbuf );

	// Compile and apply filter
	MM_FMT_BREAK_IF( pcap_compile( *handle , &fp , udp->__udp_cfg->UDP_origin_ports , 1 , mask ) == -1 , errGeneral , 2 , "Couldn't parse filter %s\n" , pcap_geterr( *handle ) );
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
	//return NULL;
}

_THREAD_FXN void_p pcap_udp_income_thread_proc( void_p src_pb )
{
	INIT_BREAKABLE_FXN();
	AB * pb = ( AB * )src_pb;
	G * _g = pb->cpy_cfg.m.m.temp_data._g;

	ASSERT( pb->cpy_cfg.m.m.maintained.in_count == 1 );

	// TODO . implement muti input

	ASSERT( pb->udps_count == 1 );

	stablish_pcap_udp_connection( pb->udps );

	BREAK_OK( 4 );

	BEGIN_RET
	case 1:
	{
		_g->stat.round_zero_set.syscal_err_count++;
	}
	M_V_END_RET
	return NULL;
}

_THREAD_FXN void_p one_tcp_out_thread_proc( void_p src_pb )
{
	INIT_BREAKABLE_FXN();
	//	static TWD twd = { 0 };
	//	if ( twd.threadId == 0 )
	//	{
	//		twd.threadId = pthread_self();
	//		twd.cal = outgoing_thread_proc; // self function address
	//		twd.callback_arg = src_pb;
	//	}
	//	if ( src_pb == NULL )
	//	{
	//		return ( void_p )&twd;
	//	}
	//
	AB * pb = ( AB * )src_pb;
	G * _g = ( G * )pb->cpy_cfg.m.m.temp_data._g;

	time_t tnow = 0;
	char buffer[ BUFFER_SIZE ]; // Define a buffer to store received data
	size_t sz;
	//ssize_t snd_ret;

	int output_tcp_socket_error_tolerance_count = 0; // restart socket after many error accur

	while ( !pb->trd.base.bridg_prerequisite_stabled )
	{
		if ( CLOSE_APP_VAR() ) break;
		mng_basic_thread_sleep( _g , HI_PRIORITY_THREAD );
	}

	while ( 1 )
	{

	//	//pthread_mutex_lock( &_g->sync.mutex );
	//	//while ( _g->sync.lock_in_progress )
	//	//{
	//	//	////struct timespec ts = { 0, 10L };
	//	//	////thrd_slep( &ts , NULL );
	//	//	pthread_cond_wait( &_g->sync.cond , &_g->sync.mutex );
	//	//}
	//	//pthread_mutex_unlock( &_g->sync.mutex );
	//	//if ( _g->sync.reset_static_after_lock )
	//	//{
	//	//	_g->sync.reset_static_after_lock = 0;
	//	//	memset( &_g->stat.round , 0 , sizeof( _g->stat.round ) );
	//	//}

		if ( pb->trd.base.do_close_thread )
		{
			break;
		}

		tnow = time( NULL );
		// tcp
		if ( difftime( tnow , _g->stat.round_zero_set.tcp_1_sec.t_tcp_throughput ) >= 1.0 )
		{
			if ( _g->stat.round_zero_set.tcp_1_sec.t_tcp_throughput > 0 )
			{
				cbuf_m_advance( &_g->stat.round_init_set.tcp_stat_5_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
				cbuf_m_advance( &_g->stat.round_init_set.tcp_stat_5_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );

				cbuf_m_advance( &_g->stat.round_init_set.tcp_stat_10_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
				cbuf_m_advance( &_g->stat.round_init_set.tcp_stat_10_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );

				cbuf_m_advance( &_g->stat.round_init_set.tcp_stat_40_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
				cbuf_m_advance( &_g->stat.round_init_set.tcp_stat_40_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );

				cbuf_m_advance( &_g->stat.round_init_set.tcp_stat_120_sec_count , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_count );
				cbuf_m_advance( &_g->stat.round_init_set.tcp_stat_120_sec_bytes , _g->stat.round_zero_set.udp_1_sec.calc_throughput_udp_get_bytes );

				//_g->stat.round.tcp_1_sec.tcp_put_count_throughput = _g->stat.round.tcp_1_sec.calc_throughput_tcp_put_count;
				//_g->stat.round.tcp_1_sec.tcp_put_byte_throughput = _g->stat.round.tcp_1_sec.calc_throughput_tcp_put_bytes;
			}
			_g->stat.round_zero_set.tcp_1_sec.t_tcp_throughput = tnow;
			_g->stat.round_zero_set.tcp_1_sec.calc_throughput_tcp_put_count = 0;
			_g->stat.round_zero_set.tcp_1_sec.calc_throughput_tcp_put_bytes = 0;
		}

		while( vcbuf_nb_pop( &pb->trd.t.p_one2one_pcap2kernelDefaultStack_SF_thread->cbuf , buffer , &sz , 60/*timeout*/) == errOK )
		{
			if ( pb->tcps_count && pb->tcps->tcp_connection_established )
			{
				if ( sendall( pb->tcps->tcp_sockfd , buffer , &sz ) != errOK )
				{
					_g->stat.round_zero_set.continuously_unsuccessful_send_error++;
					_g->stat.round_zero_set.total_unsuccessful_send_error++;

					if ( ++output_tcp_socket_error_tolerance_count > RETRY_UNEXPECTED_WAIT_FOR_SOCK() )
					{
						output_tcp_socket_error_tolerance_count = 0;
						if ( pb->tcps_count && pb->tcps->tcp_connection_established )
						{
							pb->tcps->retry_to_connect_tcp = 1;
						}
					}
					continue;
				}
				_g->stat.round_zero_set.continuously_unsuccessful_send_error = 0;
				if ( sz > 0 )
				{
					_g->stat.round_zero_set.tcp.total_tcp_put_count++;
					_g->stat.round_zero_set.tcp.total_tcp_put_byte += sz;
					_g->stat.round_zero_set.tcp_1_sec.calc_throughput_tcp_put_count++;
					_g->stat.round_zero_set.tcp_1_sec.calc_throughput_tcp_put_bytes += sz;
					//_g->stat.round.tcp_10_sec.calc_throughput_tcp_put_count++;
					//_g->stat.round.tcp_10_sec.calc_throughput_tcp_put_bytes += snd_ret;
					//_g->stat.round.tcp_40_sec.calc_throughput_tcp_put_count++;
					//_g->stat.round.tcp_40_sec.calc_throughput_tcp_put_bytes += snd_ret;
				}
			}
		}

	}

	BREAK_OK( 0 ); // to just ignore gcc warning

	BEGIN_RET
	case 3: ;
	case 2: ;
	case 1:
	{
		//_close_socket( &src_pb->tcp_sockfd );
		_g->stat.round_zero_set.syscal_err_count++;
	}
	M_V_END_RET
	return NULL;
}
