#define Uses_proc_krnl_udp_capture
#define Uses_pthread_create
#define Uses_proc_many2many_pcap_NetStack_SF
#define Uses_proc_one2many_pcap2NetStack_SF_udp_pcap
#define Uses_proc_NetStack_udp_counter
#define Uses_proc_pcap_udp_counter
#define Uses_proc_one2one_pcap2NetStack_SF_udp_pcap
#define Uses_MALLOC_AR
#define Uses_INIT_BREAKABLE_FXN
#define Uses_pthread_t
#define Uses_Bridge
#define Uses_helper
#include <Protocol_Bridge.dep>

/// <summary>
/// init active udp tcp structure by the bridge config
/// </summary>
/// <param name="_g"></param>
/// <param name="pb"></param>
void init_ActiveBridge( G * _g , AB * pb )
{
	INIT_BREAKABLE_FXN();

	if ( pb->cpy_cfg.m.m.maintained.in_count > 0 )
	{
		M_BREAK_IF( !( pb->udps = MALLOC_AR( pb->udps , pb->cpy_cfg.m.m.maintained.in_count ) ) , errMemoryLow , 1 );
		MEMSET_ZERO( pb->udps , pb->cpy_cfg.m.m.maintained.in_count );
		pb->udps_count = pb->cpy_cfg.m.m.maintained.in_count; // caution . count is always set after main ptr initialized

		for ( int iudp = 0 ; iudp < pb->udps_count ; iudp++ )
		{
			pb->udps[ iudp ].owner_pb = pb;
			pb->udps[ iudp ].__udp_cfg_pak = pb->cpy_cfg.m.m.maintained.in + iudp;
		}
	}
	if ( pb->cpy_cfg.m.m.maintained.out_count > 0 )
	{
		M_BREAK_IF( !( pb->tcps = MALLOC_AR( pb->tcps , pb->cpy_cfg.m.m.maintained.out_count ) ) , errMemoryLow , 1 );
		MEMSET_ZERO( pb->tcps , pb->cpy_cfg.m.m.maintained.out_count );
		pb->tcps_count = pb->cpy_cfg.m.m.maintained.out_count;

		for ( int itcp = 0 ; itcp < pb->tcps_count ; itcp++ )
		{
			pb->tcps[ itcp ].owner_pb = pb;
			pb->tcps[ itcp ].__tcp_cfg_pak = pb->cpy_cfg.m.m.maintained.out + itcp;
		}
	}

	BEGIN_RET // TODO . complete reverse on error
	case 1:
	{
		DIST_ERR();
	}
	M_V_END_RET
}

void mk_shrt_path( _IN AB * pb , _RET_VAL_P shrt_path * hlpr )
{
	MEMSET_ZERO_O( hlpr );

	WARNING( pb && hlpr );
	hlpr->pab = pb;
	hlpr->in_count = &pb->cpy_cfg.m.m.maintained.in_count;
	hlpr->out_count = &pb->cpy_cfg.m.m.maintained.out_count;
	hlpr->thread_is_created = &pb->trd.base.thread_is_created;
	hlpr->do_close_thread = &pb->trd.base.do_close_thread;
	//hlpr->creation_thread_race_cond = &pb->trd.base.creation_thread_race_cond;
	hlpr->bridg_prerequisite_stabled = &pb->trd.base.bridg_prerequisite_stabled;
	//hlpr->buf_psh_distri = &pb->trd.base.buffer_push_distributor;
}

void apply_new_protocol_bridge_config( G * _g , AB * pb , Bcfg * new_ccfg )
{
	INIT_BREAKABLE_FXN();

	if ( !new_ccfg->m.m.maintained.enable ) return; // think more about this option maybe at better place should place it

	//if ( !new_ccfg->m.m.maintained.enable )
	//{
	//	for ( int i = 0 ; i < pb->pb_trds_masks_count ; i++ )
	//	{
	//		if ( pb->pb_trds_masks[ i ] )
	//		{
	//			pb->pb_trds->alc_thread->do_close_thread = 1;
	//		}
	//	}
	//	return;
	//}

	// when we arrive at this point we sure that somethings is changed
	new_ccfg->m.m.temp_data.pcfg_changed = 0; // say to config that change applied to bridge
	
	// each thread action switched here

	if ( iSTR_SAME( pb->cpy_cfg.m.m.id.thread_handler_act , "pcap_udp_counter" ) )
	{
		if ( !pb->trd.t.p_pcap_udp_counter )
		{
			init_ActiveBridge( _g , pb );

			M_BREAK_IF( !( pb->trd.t.p_pcap_udp_counter = MALLOC_ONE( pb->trd.t.p_pcap_udp_counter ) ) , errMemoryLow , 1 );
			MEMSET_ZERO_O( pb->trd.t.p_pcap_udp_counter );
			//pthread_mutex_init( &pb->trd.base.creation_thread_race_cond , NULL );
			//pthread_mutex_init( &pb->trd.base.do_all_prerequisite_stablished_race_cond , NULL );
			//pthread_mutex_lock( &pb->trd.base.creation_thread_race_cond );
			if ( !pb->trd.base.thread_is_created )
			{
				MM_BREAK_IF( pthread_create( &pb->trd.t.p_pcap_udp_counter->trd_id , NULL ,
					proc_pcap_udp_counter , pb ) != PTHREAD_CREATE_OK , errCreation , 0 , "thread creation failed" );
				pb->trd.base.thread_is_created = 1;
			}
			//pthread_mutex_unlock( &pb->trd.base.creation_thread_race_cond );
		}
	}

	else if ( iSTR_SAME( pb->cpy_cfg.m.m.id.thread_handler_act , "NetStack_udp_counter" ) )
	{
		if ( !pb->trd.t.p_NetStack_udp_counter )
		{
			init_ActiveBridge( _g , pb );

			M_BREAK_IF( !( pb->trd.t.p_NetStack_udp_counter = MALLOC_ONE( pb->trd.t.p_NetStack_udp_counter ) ) , errMemoryLow , 1 );
			MEMSET_ZERO_O( pb->trd.t.p_NetStack_udp_counter );
			//pthread_mutex_init( &pb->trd.base.creation_thread_race_cond , NULL );
			//pthread_mutex_init( &pb->trd.base.do_all_prerequisite_stablished_race_cond , NULL );
			//pthread_mutex_lock( &pb->trd.base.creation_thread_race_cond );
			if ( !pb->trd.base.thread_is_created )
			{
				MM_BREAK_IF( pthread_create( &pb->trd.t.p_NetStack_udp_counter->trd_id , NULL ,
					proc_NetStack_udp_counter , pb ) != PTHREAD_CREATE_OK , errCreation , 0 , "thread creation failed" );
				pb->trd.base.thread_is_created = 1;
			}
			//pthread_mutex_unlock( &pb->trd.base.creation_thread_race_cond );

			pthread_t trd_udp_connection;
			MM_BREAK_IF( pthread_create( &trd_udp_connection , NULL , connect_udps_proc , pb ) != PTHREAD_CREATE_OK , errCreation , 0 , "thread creation failed" );
		}
	}

	else if ( iSTR_SAME( pb->cpy_cfg.m.m.id.thread_handler_act , "one2one_krnl2krnl_SF" ) )
	{
		if ( !pb->trd.t.p_one2one_krnl2krnl_SF )
		{
			init_ActiveBridge( _g , pb );

			M_BREAK_IF( !( pb->trd.t.p_one2one_krnl2krnl_SF = CALLOC_ONE( pb->trd.t.p_one2one_krnl2krnl_SF ) ) , errMemoryLow , 1 );
			//pthread_mutex_init( &pb->trd.base.creation_thread_race_cond , NULL );
			//pthread_mutex_init( &pb->trd.base.do_all_prerequisite_stablished_race_cond , NULL );
			//pthread_mutex_lock( &pb->trd.base.creation_thread_race_cond );

			// TODO . this size come from config and each packet size and release as soon as possible to prevent lost
			M_BREAK_STAT( vcbuf_nb_init( &pb->trd.t.p_one2one_krnl2krnl_SF->cbuf , 100000 , MAX_PACKET_SIZE ) , 1 );

			if ( !pb->trd.base.thread_is_created )
			{
				MM_BREAK_IF( pthread_create( &pb->trd.t.p_one2one_krnl2krnl_SF->income_trd_id , NULL ,
					proc_one2one_krnl_udp_store , pb ) != PTHREAD_CREATE_OK , errCreation , 0 , "thread creation failed" );
				MM_BREAK_IF( pthread_create( &pb->trd.t.p_one2one_krnl2krnl_SF->outgoing_trd_id , NULL ,
					proc_one2one_krnl_tcp_forward , pb ) != PTHREAD_CREATE_OK , errCreation , 0 , "thread creation failed" );
				pb->trd.base.thread_is_created = 1;
			}
			//pthread_mutex_unlock( &pb->trd.base.creation_thread_race_cond );

			pthread_t trd_udp_connection;
			MM_BREAK_IF( pthread_create( &trd_udp_connection , NULL , connect_udps_proc , pb ) != PTHREAD_CREATE_OK , errCreation , 0 , "thread creation failed" );
			pthread_t trd_tcp_connection;
			MM_BREAK_IF( pthread_create( &trd_tcp_connection , NULL , thread_tcp_connection_proc , pb ) != PTHREAD_CREATE_OK , errCreation , 0 , "thread creation failed" );
		}
	}

	else if ( iSTR_SAME( pb->cpy_cfg.m.m.id.thread_handler_act , "one2one_pcap2NetStack_SF" ) )
	{
		if ( !pb->trd.t.p_one2one_pcap2NetStack_SF )
		{
			init_ActiveBridge( _g , pb );

			M_BREAK_IF( !( pb->trd.t.p_one2one_pcap2NetStack_SF = MALLOC_ONE( pb->trd.t.p_one2one_pcap2NetStack_SF ) ) , errMemoryLow , 1 );
			MEMSET_ZERO_O( pb->trd.t.p_one2one_pcap2NetStack_SF );
			//pthread_mutex_init( &pb->trd.base.creation_thread_race_cond , NULL );
			//pthread_mutex_init( &pb->trd.base.do_all_prerequisite_stablished_race_cond , NULL );

			// TODO . this size come from config and each packet size and release as soon as possible to prevent lost
			M_BREAK_STAT( vcbuf_nb_init( &pb->trd.t.p_one2one_pcap2NetStack_SF->cbuf , 524288 , MAX_PACKET_SIZE ) , 1 );

			//pthread_mutex_lock( &pb->trd.base.creation_thread_race_cond );
			if ( !pb->trd.base.thread_is_created )
			{
				MM_BREAK_IF( pthread_create( &pb->trd.t.p_one2one_pcap2NetStack_SF->income_trd_id , NULL ,
					proc_one2one_pcap2NetStack_SF_udp_pcap , pb ) != PTHREAD_CREATE_OK , errCreation , 0 , "thread creation failed" );
				//MM_BREAK_IF( pthread_create( &pb->trd.t.p_one2one_pcap2NetStack_SF->outgoing_trd_id , NULL ,
				//	proc_one2one_pcap2NetStack_SF_tcp_out , pb ) != PTHREAD_CREATE_OK , errCreation , 0 , "thread creation failed" );
				pb->trd.base.thread_is_created = 1;
			}
			//pthread_mutex_unlock( &pb->trd.base.creation_thread_race_cond );

			pthread_t trd_tcp_connection;
			MM_BREAK_IF( pthread_create( &trd_tcp_connection , NULL , thread_tcp_connection_proc , pb ) != PTHREAD_CREATE_OK , errCreation , 0 , "thread creation failed" );
		}
	}

	else if ( iSTR_SAME( pb->cpy_cfg.m.m.id.thread_handler_act , "one2many_pcap2NetStack_SF" ) )
	{
		if ( !pb->trd.t.p_one2many_pcap2NetStack_SF )
		{
			init_ActiveBridge( _g , pb );

			M_BREAK_IF( !( pb->trd.t.p_one2many_pcap2NetStack_SF = MALLOC_ONE( pb->trd.t.p_one2many_pcap2NetStack_SF ) ) , errMemoryLow , 1 );
			MEMSET_ZERO_O( pb->trd.t.p_one2many_pcap2NetStack_SF );
			//pthread_mutex_init( &pb->trd.base.creation_thread_race_cond , NULL );
			//pthread_mutex_init( &pb->trd.base.do_all_prerequisite_stablished_race_cond , NULL );
			
			M_BREAK_STAT( vcbuf_nb_init( &pb->trd.t.p_one2many_pcap2NetStack_SF->cbuf , 524288 , MAX_PACKET_SIZE ) , 1 );

			//pthread_mutex_lock( &pb->trd.base.creation_thread_race_cond );
			if ( !pb->trd.base.thread_is_created )
			{
				MM_BREAK_IF( pthread_create( &pb->trd.t.p_one2many_pcap2NetStack_SF->income_trd_id , NULL ,
					proc_one2many_pcap2NetStack_SF_udp_pcap , pb ) != PTHREAD_CREATE_OK , errCreation , 0 , "thread creation failed" );
				MM_BREAK_IF( pthread_create( &pb->trd.t.p_one2many_pcap2NetStack_SF->outgoing_trd_id , NULL ,
					proc_one2many_tcp_out , pb ) != PTHREAD_CREATE_OK , errCreation , 0 , "thread creation failed" );
				pb->trd.base.thread_is_created = 1;
			}
			//pthread_mutex_unlock( &pb->trd.base.creation_thread_race_cond );

			pthread_t trd_tcp_connection;
			MM_BREAK_IF( pthread_create( &trd_tcp_connection , NULL , thread_tcp_connection_proc , pb ) != PTHREAD_CREATE_OK , errCreation , 0 , "thread creation failed" );
		}
	}

	else if ( iSTR_SAME( pb->cpy_cfg.m.m.id.thread_handler_act , "many2one_pcap2kernelDefaultStack_S&F_serialize" ) )
	{
		if ( !pb->trd.t.p_many2one_pcap2kernelDefaultStack_SF_serialize )
		{
			init_ActiveBridge( _g , pb );
			M_BREAK_IF( !( pb->trd.t.p_many2one_pcap2kernelDefaultStack_SF_serialize = MALLOC_ONE( pb->trd.t.p_many2one_pcap2kernelDefaultStack_SF_serialize ) ) , errMemoryLow , 1 );
			MEMSET_ZERO_O( pb->trd.t.p_many2one_pcap2kernelDefaultStack_SF_serialize );
	//		//pthread_mutex_init( &pb->trd.base.creation_thread_race_cond , NULL );
	//		//pthread_mutex_init( &pb->trd.base.do_all_prerequisite_stablished_race_cond , NULL );
	//		// TODO . buff size came from config
			M_BREAK_STAT( vcbuf_nb_init( &pb->trd.t.p_many2one_pcap2kernelDefaultStack_SF_serialize->cbuf , 524288 , MAX_PACKET_SIZE ) , 1 );
	//		//pthread_mutex_lock( &pb->trd.base.creation_thread_race_cond );
			if ( !pb->trd.base.thread_is_created )
			{
				MM_BREAK_IF( pthread_create( &pb->trd.t.p_many2one_pcap2kernelDefaultStack_SF_serialize->income_trd_id , NULL ,
					proc_many2many_pcap_NetStack_SF , pb ) != PTHREAD_CREATE_OK , errCreation , 0 , "thread creation failed" );
				//MM_BREAK_IF( pthread_create( &pb->trd.t.p_many2one_pcap2kernelDefaultStack_SF_serialize->outgoing_trd_id , NULL ,
				//	 , pb ) != PTHREAD_CREATE_OK , errCreation , 0 , "thread creation failed" );
				pb->trd.base.thread_is_created = 1;
			}
	//		//pthread_mutex_unlock( &pb->trd.base.creation_thread_race_cond );
			pthread_t trd_tcp_connection;
			MM_BREAK_IF( pthread_create( &trd_tcp_connection , NULL , thread_tcp_connection_proc , pb ) != PTHREAD_CREATE_OK , errCreation , 0 , "thread creation failed" );
		}
	}


	BEGIN_RET // TODO . complete reverse on error
	case 1:
	{
		DIST_ERR();
	}
	M_V_END_RET
}

