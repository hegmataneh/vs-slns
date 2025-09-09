#define Uses_pthread_t
#define Uses_statistics
#define Uses_config
#define Uses_INIT_BREAKABLE_FXN
#define Uses_helper
#include <Protocol_Bridge.dep>

G * _g; // just one global var

int main()
{
	INIT_BREAKABLE_FXN();
	G g = { 0 };
	_g = &g;

	init( _g );

	pthread_t tid_stats , tid_input;
	pthread_create( &tid_stats , NULL , stats_thread , ( pass_p )_g );
	pthread_create( &tid_input , NULL , input_thread , ( pass_p )_g );

	pthread_t trd_version_checker;
	MM_BREAK_IF( pthread_create( &trd_version_checker , NULL , version_checker , ( pass_p )_g ) != PTHREAD_CREATE_OK , errCreation , 0 , "Failed to create thread" );

	pthread_t trd_config_loader;
	MM_BREAK_IF( pthread_create( &trd_config_loader , NULL , config_loader , ( pass_p )_g ) != PTHREAD_CREATE_OK , errCreation , 0 , "Failed to create thread" );

	pthread_t trd_config_executer;
	MM_BREAK_IF( pthread_create( &trd_config_executer , NULL , config_executer , ( pass_p )_g ) != PTHREAD_CREATE_OK , errCreation , 0 , "Failed to create thread" );

	pthread_t trd_watchdog; // this thread keep everything alive and just check every thing just be at right position and do not do action with very risky consequenses and exception raiser
	MM_BREAK_IF( pthread_create( &trd_watchdog , NULL , watchdog_executer , ( pass_p )_g ) != PTHREAD_CREATE_OK , errCreation , 0 , "Failed to create thread" );

	M_BREAK_IF( pthread_join( trd_watchdog , NULL ) != PTHREAD_JOIN_OK , errGeneral , 0 );


	return 0;
	BEGIN_RET
	M_V_END_RET
	return 1;
}

