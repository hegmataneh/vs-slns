#define Uses_MARK_START_THREAD
#define Uses_sleep
#define Uses_WARNING
#define Uses_defragment_pcap_data
#define Uses_many_tcp_out_thread_proc
#define Uses_distributor_init
#define Uses_stablish_pcap_udp_connection
#define Uses_errno
#define Uses_globals
#define Uses_Bridge
#define Uses_INIT_BREAKABLE_FXN
#include <Protocol_Bridge.dep>

_GLOBAL_VAR _EXTERN G * _g;

_CALLBACK_FXN void quit_interrupt_dist_one2one_pcap2krnl_SF( pass_p src_pb , long v )
{
	AB * pb = ( AB * )src_pb;
	CIRCUIT_BREAKER long break_cuit = 0;
	for ( ; pb->comm.acts.p_one2one_pcap2krnl_SF->pcp_handle && break_cuit < 1000 ; mng_basic_thread_sleep( _g , HI_PRIORITY_THREAD ) , break_cuit++ )
	{
		pcap_breakloop( pb->comm.acts.p_one2one_pcap2krnl_SF->pcp_handle ); // in case we're inside pcap_loop
		// close really happened after loop closed
	}
	if ( pb->comm.acts.p_one2one_pcap2krnl_SF->pcp_handle )
	{
		char errbuf[ PCAP_ERRBUF_SIZE ] = { 0 };
		pcap_setnonblock(pb->comm.acts.p_one2one_pcap2krnl_SF->pcp_handle, 1, errbuf);
		pcap_breakloop( pb->comm.acts.p_one2one_pcap2krnl_SF->pcp_handle ); // in case we're inside pcap_loop
	}
}

// gather udp into pcap ring
//_PRIVATE_FXN _CALLBACK_FXN status buffer_push_one2one_pcap2krnl_SF( pass_p data , buffer buf , int payload_len )
//{
//	AB * pb = ( AB * )data;
//	return cbuf_pked_push( &pb->comm.preq.raw_xudp_cache , buf , payload_len );
//}

_THREAD_FXN void_p proc_one2one_pcap2krnl_SF_udp_pcap( pass_p src_pb )
{
	INIT_BREAKABLE_FXN();
	AB * pb = ( AB * )src_pb;
	G * _g = pb->cpy_cfg.m.m.temp_data._pseudo_g;
	

#ifdef ENABLE_LOG_THREADS
	distributor_publish_x3long( &_g->distributors.bcast_thread_startup , (long)pthread_self() , trdn_proc_one2one_pcap2krnl_SF_udp_pcap , NP , _g );
	__attribute__( ( cleanup( thread_goes_out_of_scope ) ) ) pthread_t trd_id = pthread_self();
#ifdef ENABLE_USE_DBG_TAG
	MARK_START_THREAD();
#endif
#endif

	WARNING( pb->cpy_cfg.m.m.maintained.in_count == 1 );

	// TODO . implement muti input

	WARNING( pb->udps_count == 1 );

	M_BREAK_STAT( distributor_init( &pb->comm.preq.bcast_pcap_udp_pkt , 1 ) , 1 );
	M_BREAK_STAT( distributor_subscribe( &pb->comm.preq.bcast_pcap_udp_pkt , SUB_DIRECT_ONE_CALL_3VOIDP ,
		SUB_FXN( defragment_pcap_data ) , src_pb ) , 1 );

	// in addition to make shrt_path complete based on type and dependency is detached
	shrt_pth_t shrtcut; // 1 . we have simple pth here
	mk_shrt_path( pb , &shrtcut ); // 2 . and fill it
	shrtcut.pcp_handle = &pb->comm.acts.p_one2one_pcap2krnl_SF->pcp_handle;

	// register here to get quit cmd
	distributor_subscribe_withOrder( &_g->distributors.bcast_quit , SUB_LONG , SUB_FXN( quit_interrupt_dist_one2one_pcap2krnl_SF ) , pb , stop_input_udp );

	// call general 
	M_BREAK_STAT( stablish_pcap_udp_connection( pb , &shrtcut ) , 1 );

	BEGIN_RET
	case 1:
	{
		DIST_BRIDGE_FAILURE();
	}
	M_V_END_RET
	return NULL;
}

_THREAD_FXN void_p proc_one2one_pcap2krnl_SF_tcp_out( pass_p src_pb )
{
	INIT_BREAKABLE_FXN();

	AB * pb = ( AB * )src_pb;
	G * _g = TO_G( pb->cpy_cfg.m.m.temp_data._pseudo_g );
	
#ifdef ENABLE_LOG_THREADS
	distributor_publish_x3long( &_g->distributors.bcast_thread_startup , (long)pthread_self() , trdn_proc_one2one_pcap2krnl_SF_tcp_out , NP , _g );
	__attribute__( ( cleanup( thread_goes_out_of_scope ) ) ) pthread_t trd_id = pthread_self();
#ifdef ENABLE_USE_DBG_TAG
	MARK_START_THREAD();
#endif
#endif

	shrt_pth_t shrtcut;
	mk_shrt_path( pb , &shrtcut );
	shrtcut.raw_xudp_cache = &pb->comm.preq.raw_xudp_cache;
	shrtcut.bcast_xudp_pkt = &pb->comm.preq.bcast_xudp_pkt;

	many_tcp_out_thread_proc( pb , &shrtcut );
#ifdef ENABLE_USE_DBG_TAG
	MARK_LINE();
#endif
	return NULL;
}

