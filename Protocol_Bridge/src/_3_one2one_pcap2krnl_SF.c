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


_CALLBACK_FXN void quit_interrupt_dist_one2one_pcap2krnl_SF( pass_p src_pb , long v )
{
	AB * pb = ( AB * )src_pb;
	for ( ; pb->trd.t.p_one2one_pcap2krnl_SF->pcp_handle ; sleep( pb->trd.t.p_one2one_pcap2krnl_SF->pcp_handle ? 1 : 0 ) )
	{
		pcap_breakloop( pb->trd.t.p_one2one_pcap2krnl_SF->pcp_handle ); // in case we're inside pcap_loop
		// close really happened after loop closed
	}
}

// gather udp into pcap ring
//_PRIVATE_FXN _CALLBACK_FXN status buffer_push_one2one_pcap2krnl_SF( pass_p data , buffer buf , int payload_len )
//{
//	AB * pb = ( AB * )data;
//	return cbuf_pked_push( &pb->trd.cmn.fast_wrt_cache , buf , payload_len );
//}

_THREAD_FXN void_p proc_one2one_pcap2krnl_SF_udp_pcap( pass_p src_pb )
{
	INIT_BREAKABLE_FXN();
	AB * pb = ( AB * )src_pb;
	G * _g = pb->cpy_cfg.m.m.temp_data._pseudo_g;
	
	distributor_publish_long( &_g->distributors.thread_startup , pthread_self() , _g );
	__attribute__( ( cleanup( thread_goes_out_of_scope ) ) ) pthread_t trd_id = pthread_self();
	MARK_START_THREAD();

	WARNING( pb->cpy_cfg.m.m.maintained.in_count == 1 );

	// TODO . implement muti input

	WARNING( pb->udps_count == 1 );

	M_BREAK_STAT( distributor_init( &pb->trd.cmn.fragmented_udp_packet_on_pcap_received_event , 1 ) , 1 );
	M_BREAK_STAT( distributor_subscribe( &pb->trd.cmn.fragmented_udp_packet_on_pcap_received_event , SUB_DIRECT_ONE_CALL_3VOIDP ,
		SUB_FXN( defragment_pcap_data ) , src_pb ) , 1 );

	
	
	// in addition to make shrt_path complete based on type and dependency is detached
	shrt_path pth; // 1 . we have simple pth here
	mk_shrt_path( pb , &pth ); // 2 . and fill it
	pth.pcp_handle = &pb->trd.t.p_one2one_pcap2krnl_SF->pcp_handle;

	// register here to get quit cmd
	distributor_subscribe_withOrder( &_g->distributors.quit_interrupt_dist , SUB_LONG , SUB_FXN( quit_interrupt_dist_one2one_pcap2krnl_SF ) , pb , clean_input_connections );

	// call general 
	M_BREAK_STAT( stablish_pcap_udp_connection( pb , &pth ) , 1 );

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
	
	distributor_publish_long( &_g->distributors.thread_startup , pthread_self() , _g );
	__attribute__( ( cleanup( thread_goes_out_of_scope ) ) ) pthread_t trd_id = pthread_self();
	MARK_START_THREAD();

	shrt_path pth;
	mk_shrt_path( pb , &pth );
	pth.fast_wrt_cache = &pb->trd.cmn.fast_wrt_cache;
	pth.defrg_pcap_payload = &pb->trd.cmn.defraged_pcap_udp_payload_event;

	many_tcp_out_thread_proc( pb , &pth );
	MARK_LINE();
	return NULL;
}

