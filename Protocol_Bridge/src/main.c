#define Uses_pthread_t
#define Uses_statistics
#define Uses_config
#define Uses_INIT_BREAKABLE_FXN
#define Uses_helper

//#define DIRECT_ECHO_BUF _g->stat.last_command // just before include dep
#include <Protocol_Bridge.dep>


//int __FXN_HIT[FXN_HIT_COUNT][PC_COUNT] = {0}; // max size is about number of code lines
//static int _pc = 1; // step of each call globally

G * __g;
extern vcbuf_nb * ppp;

int main()
{
	INIT_BREAKABLE_FXN();
	G g = { 0 };
	G * _g = &g;
	__g = _g;

	init( _g );

	ppp = NULL;

	pthread_t tid_stats , tid_input;
	pthread_create( &tid_stats , NULL , stats_thread , ( void_p )_g );
	pthread_create( &tid_input , NULL , input_thread , ( void_p )_g );

	pthread_t trd_version_checker;
	MM_BREAK_IF( pthread_create( &trd_version_checker , NULL , version_checker , ( void_p )_g ) != PTHREAD_CREATE_OK , errGeneral , 0 , "Failed to create thread" );

	pthread_t trd_config_loader;
	MM_BREAK_IF( pthread_create( &trd_config_loader , NULL , config_loader , ( void_p )_g ) != PTHREAD_CREATE_OK , errGeneral , 0 , "Failed to create thread" );

	pthread_t trd_config_executer;
	MM_BREAK_IF( pthread_create( &trd_config_executer , NULL , config_executer , ( void_p )_g ) != PTHREAD_CREATE_OK , errGeneral , 0 , "Failed to create thread" );

	pthread_t trd_watchdog; // this thread keep everything alive and just check every thing just be at right position and do not do action with very risky consequenses and exception raiser
	MM_BREAK_IF( pthread_create( &trd_watchdog , NULL , watchdog_executer , ( void_p )_g ) != PTHREAD_CREATE_OK , errGeneral , 0 , "Failed to create thread" );

	M_BREAK_IF( pthread_join( trd_watchdog , NULL ) != PTHREAD_JOIN_OK , errGeneral , 0 );


	return 0;
	BEGIN_RET
		case 2: {}
		case 1: {}
		case 0: __g->stat.round_zero_set.syscal_err_count++;
	M_V_END_RET
	return 1;
}

