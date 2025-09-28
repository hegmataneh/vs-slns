#define Uses_MEMSET_ZERO_O
#define Uses_ci_sgmgr_t
#define Uses_pthread_t
#define Uses_statistics
#define Uses_config
#define Uses_INIT_BREAKABLE_FXN
#define Uses_helper
#include <Protocol_Bridge.dep>

G * _g = NULL; // just one global var

/// <summary>
/// اینجوری می خواستم هر قسمت را یک کامپوننت بدانم و در نتیجه هر کسی مسئول ایجاد زیر ساختهای لازم برای خودش است
/// </summary>
__attribute__( ( constructor( 101 ) ) )
_PRIVATE_FXN void pre_main_top_prio_init( void )
{
	static G g = {0};
	_g = &g;
	// distribute initialization by the callback and throw components
	distributor_init( &_g->distributors.pre_configuration , 1 );
}

int main()
{
	INIT_BREAKABLE_FXN();

	distributor_publish_void( &_g->distributors.pre_configuration , NULL ); // pre config state

	// this thread keep everything alive and just check every thing just be at right position and do not do action with very risky consequenses and exception raiser
	MM_BREAK_IF( pthread_create( &_g->trds.trd_watchdog , NULL , watchdog_executer , ( pass_p )_g ) != PTHREAD_CREATE_OK , errCreation , 0 , "Failed to create watchdog thread" );
	pthread_create( &_g->trds.tid_stats , NULL , stats_thread , ( pass_p )_g );
	//pthread_create( &_g->trds.tid_input , NULL , input_thread , ( pass_p )_g );
	MM_BREAK_IF( pthread_create( &_g->trds.trd_version_checker , NULL , version_checker , ( pass_p )_g ) != PTHREAD_CREATE_OK , errCreation , 0 , "Failed to create version_checker thread" );
	MM_BREAK_IF( pthread_create( &_g->trds.trd_config_loader , NULL , config_loader , ( pass_p )_g ) != PTHREAD_CREATE_OK , errCreation , 0 , "Failed to create config_loader thread" );
	MM_BREAK_IF( pthread_create( &_g->trds.trd_config_executer , NULL , config_executer , ( pass_p )_g ) != PTHREAD_CREATE_OK , errCreation , 0 , "Failed to create config_executer thread" );

	M_BREAK_IF( pthread_join( _g->trds.trd_watchdog , NULL ) != PTHREAD_JOIN_OK , errGeneral , 0 );


	return 0;
	BEGIN_RET
	M_V_END_RET
	return 1;
}
