#define Uses_WARNING
#define Uses_many_tcp_out_thread_proc
#define Uses_stablish_pcap_udp_connection
#define Uses_distributor_init
#define Uses_errno
#define Uses_globals
#define Uses_Bridge
#define Uses_INIT_BREAKABLE_FXN
#include <Protocol_Bridge.dep>


_CALLBACK_FXN void quit_interrupt_dist_one2many_pcap2krnl_SF( pass_p src_pb , long v )
{
	AB * pb = ( AB * )src_pb;
	
	for ( ; pb->trd.t.p_one2many_pcap2krnl_SF->pcp_handle ; sleep( 1 ) )
	{
		pcap_breakloop( pb->trd.t.p_one2many_pcap2krnl_SF->pcp_handle ); // in case we're inside pcap_loop
		// close really happened after loop closed
	}
}

//_PRIVATE_FXN _CALLBACK_FXN status buffer_push_one2many_pcap2krnl_SF( pass_p data , buffer buf , int payload_len )
//{
//	AB * pb = ( AB * )data;
//	return cbuf_pked_push( &pb->trd.cmn.fast_wrt_cache , buf , payload_len );
//}

_THREAD_FXN void_p proc_one2many_pcap2krnl_SF_udp_pcap( pass_p src_pb )
{
	INIT_BREAKABLE_FXN();
	AB * pb = ( AB * )src_pb;
	G * _g = pb->cpy_cfg.m.m.temp_data._pseudo_g;
	
	distributor_publish_long( &_g->distributors.thread_startup , pthread_self() , _g );
	__attribute__((cleanup(thread_goes_out_of_scope))) pthread_t trd_id = pthread_self();
	__arrr_n += sprintf( __arrr + __arrr_n , "\t\t\t\t\t\t\t%s started %lu\n" , __FUNCTION__ , trd_id );

	WARNING( pb->cpy_cfg.m.m.maintained.in_count == 1 );

	// TODO . implement muti input

	WARNING( pb->udps_count == 1 );

	

	// in addition to make shrt_path complete based on type and dependency is detached
	shrt_path pth; // 1 . we have simple pth here
	mk_shrt_path( pb , &pth ); // 2 . and fill it
	pth.pcp_handle = &pb->trd.t.p_one2many_pcap2krnl_SF->pcp_handle;
	pth.dc_token_ring = &pb->trd.t.p_one2many_pcap2krnl_SF->dc_token_ring;
	pth.fast_wrt_cache = &pb->trd.cmn.fast_wrt_cache;

	// register here to get quit cmd
	distributor_subscribe_withOrder( &_g->distributors.quit_interrupt_dist , SUB_LONG , SUB_FXN( quit_interrupt_dist_one2many_pcap2krnl_SF ) , pb , clean_input_connections );

	M_BREAK_STAT( stablish_pcap_udp_connection( pb , &pth ) , 1 );

	BEGIN_RET
	case 1:
	{
		DIST_BRIDGE_FAILURE();
	}
	M_V_END_RET
	return NULL;
}

_THREAD_FXN void_p proc_one2many_tcp_out( pass_p src_pb )
{
	//INIT_BREAKABLE_FXN();

	AB * pb = ( AB * )src_pb;
	G * _g = TO_G( pb->cpy_cfg.m.m.temp_data._pseudo_g );
	
	distributor_publish_long( &_g->distributors.thread_startup , pthread_self() , _g );
	__attribute__( ( cleanup( thread_goes_out_of_scope ) ) ) pthread_t trd_id = pthread_self();
	__arrr_n += sprintf( __arrr + __arrr_n , "\t\t\t\t\t\t\t%s started %lu\n" , __FUNCTION__ , trd_id );

	shrt_path pth;
	mk_shrt_path( pb , &pth );
	pth.dc_token_ring = &pb->trd.t.p_one2many_pcap2krnl_SF->dc_token_ring;
	pth.fast_wrt_cache = &pb->trd.cmn.fast_wrt_cache;
	pth.defrg_pcap_payload = &pb->trd.cmn.defraged_pcap_udp_payload_event;

	many_tcp_out_thread_proc( pb , &pth );

	return NULL;
}

