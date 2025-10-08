#define Uses_rdy_pkt1
#define Uses_persistant_cache_mgr
#define Uses_MEMSET_ZERO_O
#define Uses_errno
#define Uses_helper
#define Uses_Bridge
#define Uses_INIT_BREAKABLE_FXN

#include <Protocol_Bridge.dep>

GLOBAL_VAR extern G * _g;

_CALLBACK_FXN _PRIVATE_FXN void pre_config_init_persistant_cache_mngr( void_p src_g )
{
	G * _g = ( G * )src_g;

	distributor_init( &_g->hdls.prst_csh.store_data , 1 );
	distributor_init( &_g->hdls.prst_csh.pagestack_gateway_status , 1 );

	distributor_init( &_g->hdls.prst_csh.pagestack_pakcets , 1 );
	
	distributor_subscribe( &_g->hdls.prst_csh.store_data , SUB_DIRECT_MULTICAST_CALL_BUFFER_SIZE , SUB_FXN( persistant_cache_mngr_store_data ) , _g );
	distributor_subscribe( &_g->hdls.prst_csh.pagestack_gateway_status , SUB_INT , SUB_FXN( persistant_cache_mngr_control_pg_gateway ) , _g );
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

//sem_destroy( &vc->gateway );

__attribute__( ( constructor( 106 ) ) )
static void pre_main_init_persistant_cache_mngr_component( void )
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

_CALLBACK_FXN void persistant_cache_mngr_control_pg_gateway( pass_p src_g , int open_close )
{
	G * _g = ( G * )src_g;
	if ( open_close && !_g->hdls.prst_csh.pagestack_gateway_open_val )
	{
		sem_post( &_g->hdls.prst_csh.pagestack_gateway_open_sem );
	}
	if ( !open_close )
	{
		while ( sem_trywait( &_g->hdls.prst_csh.pagestack_gateway_open_sem ) == 0 );
	}
	_g->hdls.prst_csh.pagestack_gateway_open_val = open_close;
}

_CALLBACK_FXN _PRIVATE_FXN pgstk_cmd persistant_cache_mngr_relay_packet( void_p src_g , void_p data , size_t len )
{
	G * _g = ( G * )src_g;
	if ( CLOSE_APP_VAR() ) return pgstk_not_send__stop_sending;
	if ( distributor_publish_buffer_size( &_g->hdls.prst_csh.pagestack_pakcets , data , len , NULL ) != errOK )
	{
		return _g->hdls.prst_csh.pagestack_gateway_open_val ? pgstk_not_send__continue_sending : pgstk_not_send__stop_sending;
	}
	else
	{
		return _g->hdls.prst_csh.pagestack_gateway_open_val ? pgstk_sended__continue_sending : pgstk_sended__stop_sending;
	}
}

_THREAD_FXN void_p discharge_persistant_cache_proc( pass_p src_g )
{
	G * _g = ( G * )src_g;
	do
	{
		if ( CLOSE_APP_VAR() ) break;
		sem_wait( &_g->hdls.prst_csh.pagestack_gateway_open_sem ); // wait until gate open
		while ( _g->hdls.prst_csh.pagestack_gateway_open_val ) // do discharge untill open
		{
			if ( pg_stk_try_to_pop_latest( &_g->hdls.prst_csh.page_stack , persistant_cache_mngr_relay_packet ) == errEmpty ) // actually this fxn has loop and discharge it self
			{
				while ( sem_trywait( &_g->hdls.prst_csh.pagestack_gateway_open_sem ) == 0 ); // reset semaphore
				_g->hdls.prst_csh.pagestack_gateway_open_val = 0;
				continue;
			}
		}
	}
	while( 1 );
	return NULL;
}
