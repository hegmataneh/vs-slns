#define Uses_ci_sgmgr_t
#define Uses_pthread_t
#define Uses_statistics
#define Uses_config
#define Uses_INIT_BREAKABLE_FXN
#define Uses_helper
#include <Protocol_Bridge.dep>

G * _g; // just one global var

status print_item_cb( const void * data , size_t len , void * ud )
{
	( void )ud;
	printf( " item(len=%zu): " , len );
	return errOK;
}

int main()
{
	INIT_BREAKABLE_FXN();
	G g = { 0 };
	_g = &g;

	pre_config_init( _g );

	// this thread keep everything alive and just check every thing just be at right position and do not do action with very risky consequenses and exception raiser
	MM_BREAK_IF( pthread_create( &_g->trds.trd_watchdog , NULL , watchdog_executer , ( pass_p )_g ) != PTHREAD_CREATE_OK , errCreation , 0 , "Failed to create watchdog thread" );
	pthread_create( &_g->trds.tid_stats , NULL , stats_thread , ( pass_p )_g );
	pthread_create( &_g->trds.tid_input , NULL , input_thread , ( pass_p )_g );
	MM_BREAK_IF( pthread_create( &_g->trds.trd_version_checker , NULL , version_checker , ( pass_p )_g ) != PTHREAD_CREATE_OK , errCreation , 0 , "Failed to create version_checker thread" );
	MM_BREAK_IF( pthread_create( &_g->trds.trd_config_loader , NULL , config_loader , ( pass_p )_g ) != PTHREAD_CREATE_OK , errCreation , 0 , "Failed to create config_loader thread" );
	MM_BREAK_IF( pthread_create( &_g->trds.trd_config_executer , NULL , config_executer , ( pass_p )_g ) != PTHREAD_CREATE_OK , errCreation , 0 , "Failed to create config_executer thread" );

	M_BREAK_IF( pthread_join( _g->trds.trd_watchdog , NULL ) != PTHREAD_JOIN_OK , errGeneral , 0 );


	return 0;
	BEGIN_RET
	M_V_END_RET
	return 1;
}

