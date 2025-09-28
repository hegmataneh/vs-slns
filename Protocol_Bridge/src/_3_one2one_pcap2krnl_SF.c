#define Uses_many_tcp_out_thread_proc
#define Uses_distributor_init
#define Uses_stablish_pcap_udp_connection
#define Uses_errno
#define Uses_helper
#define Uses_Bridge
#define Uses_INIT_BREAKABLE_FXN
#include <Protocol_Bridge.dep>


void quit_interrupt_dist_one2one_pcap2krnl_SF( pass_p src_pb , int v )
{
	AB * pb = ( AB * )src_pb;
	if ( pb->trd.t.p_one2one_pcap2krnl_SF->handle )
	{
		pcap_breakloop( pb->trd.t.p_one2one_pcap2krnl_SF->handle ); // in case we're inside pcap_loop
		pcap_close( pb->trd.t.p_one2one_pcap2krnl_SF->handle );
	}
}

// gather udp into pcap ring
_PRIVATE_FXN _CALLBACK_FXN status buffer_push_one2one_pcap2krnl_SF( pass_p data , buffer buf , int payload_len )
{
	AB * pb = ( AB * )data;
	return vcbuf_nb_push( &pb->trd.t.p_one2one_pcap2krnl_SF->cbuf , buf , payload_len );
}

_THREAD_FXN void_p proc_one2one_pcap2krnl_SF_udp_pcap( pass_p src_pb )
{
	INIT_BREAKABLE_FXN();
	AB * pb = ( AB * )src_pb;
	G * _g = pb->cpy_cfg.m.m.temp_data._pseudo_g;

	WARNING( pb->cpy_cfg.m.m.maintained.in_count == 1 );

	// TODO . implement muti input

	WARNING( pb->udps_count == 1 );

	M_BREAK_STAT( distributor_init( &pb->trd.base.buffer_push_distributor , 1 ) , 1 );
	M_BREAK_STAT( distributor_subscribe( &pb->trd.base.buffer_push_distributor , SUB_DIRECT_ONE_CALL_BUFFER_INT ,
		SUB_FXN( buffer_push_one2one_pcap2krnl_SF ) , src_pb ) , 1 );
	
	// in addition to make shrt_path complete based on type and dependency is detached
	shrt_path pth; // 1 . we have simple pth here
	mk_shrt_path( pb , &pth ); // 2 . and fill it
	pth.handle = &pb->trd.t.p_one2one_pcap2krnl_SF->handle;

	// register here to get quit cmd
	distributor_subscribe( &_g->distributors.quit_interrupt_dist , SUB_INT , SUB_FXN( quit_interrupt_dist_one2one_pcap2krnl_SF ) , pb );
	
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
	//G * _g = ( G * )pb->cpy_cfg.m.m.temp_data._g;

	shrt_path pth;
	mk_shrt_path( pb , &pth );
	pth.cbuf = &pb->trd.t.p_one2one_pcap2krnl_SF->cbuf;
	pth.buf_pop_distr = &pb->trd.t.p_one2one_pcap2krnl_SF->buffer_pop_distributor;

	many_tcp_out_thread_proc( pb , &pth );

	return NULL;
}

