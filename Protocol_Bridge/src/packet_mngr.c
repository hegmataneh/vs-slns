#define Uses_packet_mngr
#define Uses_MEMSET_ZERO_O
#define Uses_errno
#define Uses_helper
#define Uses_Bridge
#define Uses_INIT_BREAKABLE_FXN

#include <Protocol_Bridge.dep>

_CALLBACK_FXN status operation_on_tcp_packet( pass_p data , buffer buf , int sz )
{
	INIT_BREAKABLE_FXN();

	AB_tcp * tcp = ( AB_tcp * )data;
	AB * pb = tcp->owner_pb;
	G * _g = ( G * )tcp->owner_pb->cpy_cfg.m.m.temp_data._g;

	rdy_pkt1 * pkt1 = ( rdy_pkt1 * )buf;
	uint8_t ttt = TCP_PACKET_VERSION;
	
	ASSERT( pkt1->version == ttt );
	size_t hdr_sz = sizeof( pkt1->version ) + strlen( pkt1->TCP_name ) + sizeof( EOS );
	buffer pkt_data = buf + hdr_sz;
	size_t pkt_data_sz = sz - hdr_sz;
	//status ret = segmgr_append( &_g->aggr_inp_pkt , pkt_data , pkt_data_sz );
	//RANJE_ACT1( ret , errArg , NULL_ACT , MACRO_E( M_BREAK_STAT( ret , 0 ) ) );


	// TODO . add into memory
	// TODO . add log if necessary
	// TODO . add record to file if memory about to full
	// TODO . some how save record to send them later to spec destination

	//size_t sz_t = sz;
	//status d_error = sendall( tcp->tcp_sockfd , buf , &sz_t ); // send is to heavy so it must send where it can

	//if ( d_error == errOK && sz > 0 )
	//{
	//	pb->stat.round_zero_set.tcp.total_tcp_put_count++;
	//	pb->stat.round_zero_set.tcp.total_tcp_put_byte += sz;
	//	pb->stat.round_zero_set.tcp_1_sec.calc_throughput_tcp_put_count++;
	//	pb->stat.round_zero_set.tcp_1_sec.calc_throughput_tcp_put_bytes += sz;
	//	pb->stat.round_zero_set.tcp_send_data_alive_indicator++;
	//}
	
	BEGIN_SMPL
	M_END_RET
}

