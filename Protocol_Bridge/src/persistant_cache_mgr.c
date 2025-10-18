#define Uses_rdy_pkt1
#define Uses_persistant_cache_mgr
#define Uses_MEMSET_ZERO_O
#define Uses_errno
#define Uses_globals
#define Uses_Bridge
#define Uses_INIT_BREAKABLE_FXN
#include <Protocol_Bridge.dep>

GLOBAL_VAR extern G * _g;

_PRIVATE_FXN _CALLBACK_FXN void cleanup_persistant_cache_mngr( pass_p src_g , long v )
{
	G * _g = ( G * )src_g;

	sem_destroy( &_g->hdls.prst_csh.pagestack_gateway_open_sem );
	pg_stk_shutdown( &_g->hdls.prst_csh.page_stack );
	sub_destroy( &_g->hdls.prst_csh.store_data );
	sub_destroy( &_g->hdls.prst_csh.pagestack_gateway_status );
	sub_destroy( &_g->hdls.prst_csh.pagestack_pakcets );

	MARK_LINE();
}

_PRIVATE_FXN _CALLBACK_FXN void cleanup_persistant_cache_mngr_fetch( pass_p src_g , long v )
{
	G * _g = ( G * )src_g;

	for ( _g->hdls.prst_csh.pagestack_gateway_open_val = ( _g->hdls.prst_csh.pagestack_gateway_open_val >= 0 ? 0 : _g->hdls.prst_csh.pagestack_gateway_open_val ) ; _g->hdls.prst_csh.pagestack_gateway_open_val >= 0 ; sleep( 1 ) )
	{
		sem_post( &_g->hdls.prst_csh.pagestack_gateway_open_sem );
	}

	MARK_LINE();
}

_CALLBACK_FXN _PRIVATE_FXN void pre_config_init_persistant_cache_mngr( void_p src_g )
{
	G * _g = ( G * )src_g;

	distributor_init( &_g->hdls.prst_csh.store_data , 1 );
	distributor_init( &_g->hdls.prst_csh.pagestack_gateway_status , 1 );

	distributor_init( &_g->hdls.prst_csh.pagestack_pakcets , 1 );
	
	distributor_subscribe( &_g->hdls.prst_csh.store_data , SUB_DIRECT_MULTICAST_CALL_BUFFER_SIZE , SUB_FXN( persistant_cache_mngr_store_data ) , _g );
	distributor_subscribe( &_g->hdls.prst_csh.pagestack_gateway_status , SUB_LONG , SUB_FXN( persistant_cache_mngr_control_pg_gateway ) , _g );

	// register here to get quit cmd
	distributor_subscribe_withOrder( &_g->distributors.quit_interrupt_dist , SUB_LONG , SUB_FXN( cleanup_persistant_cache_mngr ) , _g , clean_persistant_cache_mgr ); // when file write goes down

	distributor_subscribe_withOrder( &_g->distributors.quit_interrupt_dist , SUB_LONG , SUB_FXN( cleanup_persistant_cache_mngr_fetch ) , _g , clean_try_post_packet ); // when fetch from file not need
}

_CALLBACK_FXN _PRIVATE_FXN void post_config_init_persistant_cache_mngr( void_p src_g )
{
	INIT_BREAKABLE_FXN();
	G * _g = ( G * )src_g;

	sem_init( &_g->hdls.prst_csh.pagestack_gateway_open_sem , 0 , 0 );
	_g->hdls.prst_csh.pagestack_gateway_open_val = 0;

	M_BREAK_STAT( pg_stk_init( &_g->hdls.prst_csh.page_stack , "./" , _g ) , 0 );
	MM_BREAK_IF( pthread_create( &_g->hdls.prst_csh.trd_pagestack , NULL , discharge_persistant_cache_proc , ( pass_p )_g ) != PTHREAD_CREATE_OK , errCreation , 0 , "Failed to create tcp_sender thread" );

	BEGIN_SMPL
	M_V_END_RET
}

PRE_MAIN_INITIALIZATION( 106 )
_PRIVATE_FXN void pre_main_init_persistant_cache_mngr_component( void )
{
	distributor_subscribe( &_g->distributors.pre_configuration , SUB_VOID , SUB_FXN( pre_config_init_persistant_cache_mngr ) , _g );
	distributor_subscribe( &_g->distributors.post_config_stablished , SUB_VOID , SUB_FXN( post_config_init_persistant_cache_mngr ) , _g );
}

_CALLBACK_FXN status persistant_cache_mngr_store_data( pass_p src_g , buffer src_rdy_pkt1 , size_t sz )
{
	G * _g = ( G * )src_g;
	rdy_pkt1 * pkt1 = ( rdy_pkt1 * )src_rdy_pkt1;

	return pg_stk_store( &_g->hdls.prst_csh.page_stack , src_rdy_pkt1 , sz );
}

_CALLBACK_FXN void persistant_cache_mngr_control_pg_gateway( pass_p src_g , long open_close )
{
	G * _g = ( G * )src_g;
	
	if ( open_close )
	{
		int sval = -1;
		int * psval = NULL;
		if ( !sem_getvalue( &_g->hdls.prst_csh.pagestack_gateway_open_sem , &sval ) )
		{
			psval = &sval;
		}
		if ( !psval || (*psval) < 1 )
		{
			sem_post( &_g->hdls.prst_csh.pagestack_gateway_open_sem );
		}
	}
	else
	{
		while ( sem_trywait( &_g->hdls.prst_csh.pagestack_gateway_open_sem ) == 0 );
	}
	if ( _g->hdls.prst_csh.pagestack_gateway_open_val >= 0 )
	{
		_g->hdls.prst_csh.pagestack_gateway_open_val = open_close;
	}
}

_CALLBACK_FXN _PRIVATE_FXN pgstk_cmd persistant_cache_mngr_relay_packet( void_p src_g , void_p data , size_t len )
{
	G * _g = ( G * )src_g;

	if ( _g->cmd.block_sending_1 ) return pgstk_not_send__stop_sending;
	status ret = distributor_publish_buffer_size( &_g->hdls.prst_csh.pagestack_pakcets , data , len , NULL );
	switch ( ret )
	{
		case errOK:
		{
			return _g->hdls.prst_csh.pagestack_gateway_open_val ? pgstk_sended__continue_sending : pgstk_sended__stop_sending;
		}
		case errTooManyAttempt:
		{
			return pgstk_not_send__continue_sending_with_delay;
		}
		default:
		{
			return _g->hdls.prst_csh.pagestack_gateway_open_val ? pgstk_not_send__continue_sending : pgstk_not_send__stop_sending;
			break;
		}
	}
}

_THREAD_FXN void_p discharge_persistant_cache_proc( pass_p src_g )
{
	G * _g = ( G * )src_g;
	
	distributor_publish_long( &_g->distributors.thread_startup , pthread_self() , _g );
	__attribute__( ( cleanup( thread_goes_out_of_scope ) ) ) pthread_t trd_id = pthread_self();
	MARK_START_THREAD();

	status ret;
	do
	{
		if ( GRACEFULLY_END_THREAD() ) break;
		sem_wait( &_g->hdls.prst_csh.pagestack_gateway_open_sem ); // wait until gate open
		while ( _g->hdls.prst_csh.pagestack_gateway_open_val > 0 ) // do discharge untill open
		{
			ret = pg_stk_try_to_pop_latest( &_g->hdls.prst_csh.page_stack , persistant_cache_mngr_relay_packet );
			time_t tnow = time( NULL );
			switch ( ret )
			{
				case errEmpty: // actually this fxn has loop and discharge it self
				{
					while ( sem_trywait( &_g->hdls.prst_csh.pagestack_gateway_open_sem ) == 0 ); // reset semaphore
					_g->hdls.prst_csh.pagestack_gateway_open_val = 0;
					if ( _g->hdls.prst_csh.cool_down_attempt_onEmpty && _g->hdls.prst_csh.cool_down_attempt_onEmpty == *( int * )&tnow ) // in one second we should not attempt . and this check and possiblity is rare in not too many attempt situation
					{
						mng_basic_thread_sleep( _g , NORMAL_PRIORITY_THREAD );
					}
					else
					{
						tnow = time( NULL );
						_g->hdls.prst_csh.cool_down_attempt_onEmpty = *( int * )&tnow;
					}
					continue;
				}
				case errTooManyAttempt: // let get the cpu some air
				{
					mng_basic_thread_sleep( _g , NORMAL_PRIORITY_THREAD );
					break;
				}
			}
		}
	}
	while( 1 );
	_g->hdls.prst_csh.pagestack_gateway_open_val = -1;

	MARK_LINE();

	return NULL;
}
