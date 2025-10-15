#define Uses_strings_ar

#define Uses_MEMSET_ZERO_O
#define Uses_ci_sgmgr_t
#define Uses_pthread_t
#define Uses_statistics
#define Uses_config
#define Uses_INIT_BREAKABLE_FXN
#define Uses_globals
#include <Protocol_Bridge.dep>

GLOBAL_VAR G * _g = NULL; // just one global var

/// <summary>
/// اینجوری می خواستم هر قسمت را یک کامپوننت بدانم و در نتیجه هر کسی مسئول ایجاد زیر ساختهای لازم برای خودش است
/// </summary>
PRE_MAIN_INITIALIZATION( 101 )
_PRIVATE_FXN void pre_main_top_prio_init( void )
{
	static G g = {0};
	_g = &g;
	// distribute initialization by the callback and throw components
	distributor_init( &_g->distributors.pre_configuration , 1 );
}

#ifdef Uses_MLEAK
extern mLeak_t __alc_hit[MLK_HASH_WIDTH][EACH_ADDR_COUNT];
#endif

_CALLBACK_FXN void * signal_thread( void * arg )
{
	sigset_t * set = arg;
	int sig;
	sigwait( set , &sig ); // Wait for Ctrl+C (SIGINT)
	printf( "\nSIGINT caught, stopping...\n" );
	_g->cmd.block_sending_1 = 1; // Signal all threads to stop
	_g->cmd.burst_waiting_2 = true;
	_g->cmd.quit_thread_3 = 1;
	distributor_publish_long( &_g->distributors.quit_interrupt_dist , sig , NULL );
	sub_destroy( &_g->distributors.quit_interrupt_dist );
	_g->cmd.quit_app_4 = 1;
	return NULL;
}

char __arrr[10000] = { 0 };
int __arrr_n = { 0 };

int main()
{
	INIT_BREAKABLE_FXN();

	#ifndef decently_shutdown 
	sigset_t set; // just in main fxn it is working and before everything
	pthread_t th_signal;

	// Block SIGINT in all threads (including main)
	sigemptyset( &set );
	sigaddset( &set , SIGINT );
	sigaddset( &set , SIGTERM );
	sigaddset( &set , SIGPIPE );
	pthread_sigmask( SIG_BLOCK , &set , NULL );

	// Start the signal handler thread
	pthread_create( &th_signal , NULL , signal_thread , &set );
	#endif

	distributor_publish_void( &_g->distributors.pre_configuration , NULL ); // pre config state

	// this thread keep everything alive and just check every thing just be at right position and do not do action with very risky consequenses and exception raiser
	MM_BREAK_IF( pthread_create( &_g->trds.trd_watchdog , NULL , watchdog_executer , ( pass_p )_g ) != PTHREAD_CREATE_OK , errCreation , 0 , "Failed to create watchdog thread" );
	pthread_create( &_g->trds.tid_stats , NULL , stats_thread , ( pass_p )_g );
	//pthread_create( &_g->trds.tid_input , NULL , input_thread , ( pass_p )_g );
	MM_BREAK_IF( pthread_create( &_g->trds.trd_version_checker , NULL , version_checker , ( pass_p )_g ) != PTHREAD_CREATE_OK , errCreation , 0 , "Failed to create version_checker thread" );
	MM_BREAK_IF( pthread_create( &_g->trds.trd_config_loader , NULL , config_loader , ( pass_p )_g ) != PTHREAD_CREATE_OK , errCreation , 0 , "Failed to create config_loader thread" );
	MM_BREAK_IF( pthread_create( &_g->trds.trd_config_executer , NULL , config_executer , ( pass_p )_g ) != PTHREAD_CREATE_OK , errCreation , 0 , "Failed to create config_executer thread" );

	M_BREAK_IF( pthread_join( _g->trds.trd_watchdog , NULL ) != PTHREAD_JOIN_OK , errGeneral , 0 );

	#ifdef Uses_MLEAK
	FILE * fl = fopen( "leak.txt" , "w+" );

	for ( int ii = 0 ; ii < MLK_HASH_WIDTH ; ii++ )
	{
		for ( int jj = 0 ; jj < EACH_ADDR_COUNT ; jj++ )
		{
			if ( __alc_hit[ ii ][ jj ].counter != 0 )
			{
				fprintf( fl , "%d %s\n" , __alc_hit[ ii ][ jj ].counter , __alc_hit[ ii ][ jj ].klstck.temp_buf );
			}
		}
	}
	fclose( fl );
	#endif

	//sleep(1); // for sanitizer to dump
	_exit( 0 );
	return 0;
	BEGIN_RET
	M_V_END_RET
	sleep( 1 );
	_exit( 0 );
	return 1;
}
