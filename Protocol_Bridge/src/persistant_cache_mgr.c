#define Uses_MARK_LINE
#define Uses_sleep
#define Uses_xudp_hdr
#define Uses_persistant_cache_mgr
#define Uses_MEMSET_ZERO_O
#define Uses_errno
#define Uses_globals
#define Uses_Bridge
#define Uses_INIT_BREAKABLE_FXN
#include <Protocol_Bridge.dep>

_GLOBAL_VAR _EXTERN G * _g;

_PRIVATE_FXN _CALLBACK_FXN void cleanup_persistant_cache_mngr( pass_p src_g , long v )
{
	G * _g = ( G * )src_g;

	sem_destroy( &_g->hdls.gateway.pagestack_gateway_open_sem );
	pg_stk_shutdown( &_g->hdls.prst_csh.page_stack );
	sub_destroy( &_g->hdls.prst_csh.bcast_store_data );
	sub_destroy( &_g->hdls.prst_csh.bcast_reroute_nopeer_pkt );

	sub_destroy( &_g->hdls.prst_csh.bcast_pagestacked_pkts );

#ifdef ENABLE_USE_DBG_TAG
	MARK_LINE();
#endif
}

_PRIVATE_FXN _CALLBACK_FXN void try_stop_sending_from_cach_mgr( pass_p src_g , long v )
{
	G * _g = ( G * )src_g;

	for
	(
		_g->hdls.gateway.pagestack_gateway_open_val = ( _g->hdls.gateway.pagestack_gateway_open_val >= gws_still_active ? gws_close : _g->hdls.gateway.pagestack_gateway_open_val ) ;
		_g->hdls.gateway.pagestack_gateway_open_val >= gws_still_active ;
		mng_basic_thread_sleep( _g , HI_PRIORITY_THREAD )
	)
	{
		sem_post( &_g->hdls.gateway.pagestack_gateway_open_sem );
	}

#ifdef ENABLE_USE_DBG_TAG
	MARK_LINE();
#endif
}

_CALLBACK_FXN _PRIVATE_FXN void pre_config_init_persistant_cache_mngr( void_p src_g )
{
	INIT_BREAKABLE_FXN();
	G * _g = ( G * )src_g;

	M_BREAK_STAT( distributor_init( &_g->hdls.prst_csh.bcast_store_data , 1 ) , 0 );
	M_BREAK_STAT( distributor_init( &_g->hdls.prst_csh.bcast_reroute_nopeer_pkt , 1 ) , 0 );
	
	M_BREAK_STAT( distributor_init( &_g->hdls.prst_csh.bcast_pagestacked_pkts , 1 ) , 0 );
	M_BREAK_STAT( distributor_subscribe( &_g->hdls.prst_csh.bcast_store_data , SUB_DIRECT_MULTICAST_CALL_BUFFER_SIZE , SUB_FXN( persistant_cache_mngr_store_data ) , _g ) , 0 );

	M_BREAK_STAT( distributor_subscribe( &_g->hdls.prst_csh.bcast_reroute_nopeer_pkt , SUB_DIRECT_MULTICAST_CALL_BUFFER_SIZE_LONG , SUB_FXN( persistant_cache_mngr_store_data_into_queue ) , _g ) , 0 );

	// register here to get quit cmd
	M_BREAK_STAT( distributor_subscribe_withOrder( &_g->distributors.bcast_quit , SUB_LONG , SUB_FXN( cleanup_persistant_cache_mngr ) , _g , clean_persistant_cache_mgr ) , 0 ); // when file write goes down
	M_BREAK_STAT( distributor_subscribe_withOrder( &_g->distributors.bcast_quit , SUB_LONG , SUB_FXN( try_stop_sending_from_cach_mgr ) , _g , stop_sending_from_cach_mgr ) , 0 ); // when fetch from file not need

	BEGIN_RET
	default:
	{
		if ( d_error ) DIST_APP_FAILURE();
	}
	M_V_END_RET
}

_CALLBACK_FXN _PRIVATE_FXN void post_config_init_persistant_cache_mngr( void_p src_g )
{
	INIT_BREAKABLE_FXN();
	G * _g = ( G * )src_g;

	_g->hdls.gateway.pagestack_gateway_open_val = gws_close;
	sem_init( &_g->hdls.gateway.pagestack_gateway_open_sem , 0 , 0 );

#ifdef ENABLE_USE_DBG_TAG
	MARK_LINE();
#endif

#ifdef ENABLE_PERSISTENT_CACHE
	M_BREAK_STAT( pg_stk_init( &_g->hdls.prst_csh.page_stack , "./" , _g ) , 0 );
	MM_BREAK_IF( pthread_create( &_g->hdls.prst_csh.trd_pagestack_discharger , NULL , discharge_persistant_cache_proc , ( pass_p )_g ) != PTHREAD_CREATE_OK , errCreation , 0 , "Failed to create tcp_sender thread" );
#endif

	BEGIN_RET
	default:
	{
		if ( d_error ) DIST_APP_FAILURE();
	}
	M_V_END_RET
}

PRE_MAIN_INITIALIZATION( PRE_MAIN_INIT_PERSISTANT_CACHE_MNGR )
_PRIVATE_FXN void pre_main_init_persistant_cache_mngr_component( void )
{
	INIT_BREAKABLE_FXN();

	M_BREAK_STAT( distributor_subscribe( &_g->distributors.bcast_pre_cfg , SUB_VOID , SUB_FXN( pre_config_init_persistant_cache_mngr ) , _g ) , 0 );
	M_BREAK_STAT( distributor_subscribe_withOrder( &_g->distributors.bcast_post_cfg , SUB_VOID , SUB_FXN( post_config_init_persistant_cache_mngr ) , _g , post_config_order_persistant_cache_mngr ) , 0 );

	BEGIN_RET
	default:
	{
		if ( d_error ) DIST_APP_FAILURE();
	}
	M_V_END_RET
}

_CALLBACK_FXN status persistant_cache_mngr_store_data( pass_p src_g , buffer src_xudp_hdr , size_t sz )
{
#ifdef ENABLE_PERSISTENT_CACHE
	G * _g = ( G * )src_g;
	xudp_hdr * pkt1 = ( xudp_hdr * )src_xudp_hdr;

	status d_error = pg_stk_store_at_stack( &_g->hdls.prst_csh.page_stack , src_xudp_hdr , sz );
	return d_error;
#else
	return errOK;
#endif
}

_CALLBACK_FXN status persistant_cache_mngr_store_data_into_queue( pass_p src_g , buffer src_xudp_hdr , size_t sz , long ex_data ) // call from thread discharge_persistent
{
#ifdef ENABLE_PERSISTENT_CACHE
	G * _g = ( G * )src_g;
	xudp_hdr * pkt1 = ( xudp_hdr * )src_xudp_hdr;

	status d_error = pg_stk_store_at_queue( &_g->hdls.prst_csh.page_stack , src_xudp_hdr , sz , ( LPCSTR )ex_data );
	return d_error;
#else
	return errOK;
#endif
}

_CALLBACK_FXN _PRIVATE_FXN pgstk_cmd persistant_cache_mngr_relay_packet( void_p src_g , void_p data , size_t len )
{
	G * _g = ( G * )src_g;

	if ( _g->cmd.block_sending_1 ) return pgstk_not_send__stop_sending;
	status ret = distributor_publish_buffer_size( &_g->hdls.prst_csh.bcast_pagestacked_pkts , data , len , SUBSCRIBER_PROVIDED );
	switch ( ret )
	{
		case errOK:
		{
			switch ( _g->hdls.gateway.pagestack_gateway_open_val )
			{
				case gws_close: return pgstk_sended__stop_sending;
				case gws_open: return pgstk_sended__continue_sending;
				default : return pgstk_not_send__stop_sending;
			}
		}
		case errMapped:
		{
			switch ( _g->hdls.gateway.pagestack_gateway_open_val )
			{
				case gws_close: return pgstk_remapped__stop_sending;
				case gws_open: return pgstk_remapped__continue_sending;
				default: return pgstk_remapped__stop_sending;
			}
		}
		case errTooManyAttempt:
		{
			return pgstk_not_send__continue_sending_onTooManyAttempt;
		}
		case errPeerClosed:
		case errNoPeer:
		{
			return pgstk_not_send__not_any_peer;
		}
		case errButContinue: // if mapped item not send just delayed memmap and continue other memmaps
		{
			switch ( _g->hdls.gateway.pagestack_gateway_open_val )
			{
				default:
				case gws_close: return pgstk_not_send__stop_sending_delayedMemmap;
				case gws_open: return pgstk_not_send__continue_sending;
			}
			return pgstk_not_send__continue_sending;
		}
		default:
		{
			switch ( _g->hdls.gateway.pagestack_gateway_open_val )
			{
				case gws_close: return pgstk_not_send__stop_sending;
				case gws_open: return pgstk_not_send__continue_sending;
				default: return pgstk_not_send__stop_sending;
			}
		}
	}
}

#ifdef ENABLE_USE_DBG_TAG
_GLOBAL_VAR _EXTERN long long _open_gate_cnt;
_GLOBAL_VAR _EXTERN long long _close_gate_cnt;
#endif

_THREAD_FXN void_p discharge_persistant_cache_proc( pass_p src_g )
{
	G * _g = ( G * )src_g;
	
#ifdef ENABLE_LOG_THREADS
	__attribute__( ( cleanup( thread_goes_out_of_scope ) ) ) pthread_t this_thread = pthread_self();
	distributor_publish_x3long( &_g->distributors.bcast_thread_startup , (long)this_thread , trdn_discharge_persistant_cache_proc , (long)__FUNCTION__ , _g );
	
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

	status ret;
	do
	{
		if ( pthis_thread_alive_time ) *pthis_thread_alive_time = time( NULL );
		if ( GRACEFULLY_END_THREAD() ) break;
		sem_wait( &_g->hdls.gateway.pagestack_gateway_open_sem ); // wait until gate open
		while ( _g->hdls.gateway.pagestack_gateway_open_val == gws_open ) // do discharge untill open
		{
			ret = pg_stk_try_to_pop_latest( &_g->hdls.prst_csh.page_stack , persistant_cache_mngr_relay_packet );
			time_t tnow = time( NULL );
			switch ( ret )
			{
				case errEmpty: // actually this fxn has loop and discharge it self
				{
				#ifdef ENABLE_USE_DBG_TA
					_close_gate_cnt++;
				#endif
					_g->hdls.gateway.pagestack_gateway_open_val = gws_close;
					if ( _g->hdls.prst_csh.cool_down_attempt_onEmpty && _g->hdls.prst_csh.cool_down_attempt_onEmpty == *( int * )&tnow ) // in one second we should not attempt . and this check and possiblity is rare in not too many attempt situation
					{
						mng_basic_thread_sleep( _g , NORMAL_PRIORITY_THREAD );
					}
					else
					{
						tnow = time( NULL );
						_g->hdls.prst_csh.cool_down_attempt_onEmpty = *( int * )&tnow;
						distributor_publish_buffer_size( &_g->hdls.prst_csh.bcast_pagestacked_pkts , NULL , 0 , SUBSCRIBER_PROVIDED );
					}
					continue;
				}
				case errTooManyAttempt: // let get the cpu some air
				{
					distributor_publish_buffer_size( &_g->hdls.prst_csh.bcast_pagestacked_pkts , NULL , 0 , SUBSCRIBER_PROVIDED );
					mng_basic_thread_sleep( _g , NORMAL_PRIORITY_THREAD );
					break;
				}
			}
		}
	}
	while( 1 );
	_g->hdls.gateway.pagestack_gateway_open_val = gws_die;

#ifdef ENABLE_USE_DBG_TAG
	MARK_LINE();
#endif

	return NULL;
}
