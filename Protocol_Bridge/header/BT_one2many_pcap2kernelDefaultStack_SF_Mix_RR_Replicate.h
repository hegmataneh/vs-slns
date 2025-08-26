#pragma once

typedef struct ActiveBridgeShortPathHelper_Mix
{
	/*at the top same as ActiveBridgeShortPathHelper*/
	AB * pab;
	int * in_count;
	int * out_count;
	int * thread_is_created;
	int * do_close_thread;
	pthread_mutex_t * creation_thread_race_cond;
	int * bridg_prerequisite_stabled;
	distributor_t * buf_psh_distri;
	/*at the top*/

	pthread_t * income_trd_id;
	pthread_t * outgoing_trd_id;
	vcbuf_nb * cbuf; // buffer
	distributor_t * buf_pop_distr;
	dict_o_t * dc_token_ring;
} mix_helper;

void mk_hlpr1( _IN AB * pb , _OUT mix_helper * hlpr );

_THREAD_FXN void_p one2many_pcap2kernelDefaultStack_SF_Mix_RR_Replicate_pcap_udp_income_thread_proc( void_p src_pb );
_THREAD_FXN void_p one2many_pcap2kernelDefaultStack_SF_Mix_RR_Replicate_many_tcp_out_thread_proc( void_p src_pb );