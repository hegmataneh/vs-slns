#define Uses_MARK_START_THREAD
#define Uses_stablish_pcap_udp_connection
#define Uses_distributor_init
#define Uses_errno
#define Uses_globals
#define Uses_Bridge
#define Uses_INIT_BREAKABLE_FXN
#include <Protocol_Bridge.dep>

_CALLBACK_FXN void quit_interrupt_dist_push_many2many_pcap_krnl_SF( pass_p src_pb , long v )
{
	AB * pb = ( AB * )src_pb;
	//if ( pb->comm.acts.p_many2one_pcap2krnl_SF_serialize->handle )
	//{
	//	pcap_breakloop( pb->comm.acts.p_many2one_pcap2krnl_SF_serialize->handle ); // in case we're inside pcap_loop
	//	pcap_close( pb->comm.acts.p_many2one_pcap2krnl_SF_serialize->handle );
	//}
}

//_PRIVATE_FXN _CALLBACK_FXN status buffer_push_many2many_pcap_krnl_SF( pass_p data , buffer buf , int payload_len )
//{
//	AB * pb = ( AB * )data;
//	return cbuf_pked_push( &pb->comm.preq.raw_xudp_cache , buf , payload_len );
//}

_THREAD_FXN void_p proc_many2many_pcap_krnl_SF( pass_p src_pb )
{
	// TODO . generate rules of input then create suit threads to input that rule
	INIT_BREAKABLE_FXN();
	AB * pb = ( AB * )src_pb;
	G * _g = pb->cpy_cfg.m.m.temp_data._pseudo_g;
	
#ifdef ENABLE_LOG_THREADS
	__attribute__( ( cleanup( thread_goes_out_of_scope ) ) ) pthread_t this_thread = pthread_self();
	distributor_publish_x3long( &_g->distributors.bcast_thread_startup , (long)this_thread , trdn_proc_many2many_pcap_krnl_SF , (long)__FUNCTION__ , _g );
	
	/*retrieve track alive indicator*/
	pthread_mutex_lock( &_g->stat.nc_s_req.thread_list_mtx );
	time_t * pthis_thread_alive_time = NULL;
	for ( size_t idx = 0 ; idx < _g->stat.nc_s_req.thread_list.count ; idx++ )
	{
		thread_alive_indicator * pthread_ind = NULL;
		if ( mms_array_get_s( &_g->stat.nc_s_req.thread_list , idx , ( void ** )&pthread_ind ) == errOK && pthread_ind->thread_id == this_thread )
		{
			pthis_thread_alive_time = &pthread_ind->alive_time;
		}
	}
	pthread_mutex_unlock( &_g->stat.nc_s_req.thread_list_mtx );
#ifdef ENABLE_USE_DBG_TAG
	MARK_START_THREAD();
#endif
#endif

	//WARNING( pb->cpy_cfg.m.m.maintained.in_count == 1 );

	

	// in addition to make shrt_path complete based on type and dependency is detached
	shrt_pth_t shrtcut; // 1 . we have simple pth here
	mk_shrt_path( pb , &shrtcut ); // 2 . and fill it
	//shrtcut.handle = &pb->comm.acts.p_many2one_pcap2krnl_SF_serialize->handle;
	shrtcut.dc_token_ring = &pb->comm.acts.p_many2one_pcap2krnl_SF_serialize->dc_token_ring;
	shrtcut.raw_xudp_cache = &pb->comm.preq.raw_xudp_cache;

	// register here to get quit cmd
	distributor_subscribe_withOrder( &_g->distributors.bcast_quit , SUB_LONG , SUB_FXN( quit_interrupt_dist_push_many2many_pcap_krnl_SF ) , pb , stop_input_udp );

#ifdef ENABLE_USE_DBG_TAG
	MARK_LINE();
#endif

	if ( pthis_thread_alive_time )
	{
		*pthis_thread_alive_time = time( NULL );
		pb->pthread_alive_time = pthis_thread_alive_time;
	}

	M_BREAK_STAT( stablish_pcap_udp_connection( pb , &shrtcut ) , 1 );

	BEGIN_RET
	case 1:
	{
		DIST_BRIDGE_FAILURE();
	}
	M_V_END_RET
	return NULL;
}
