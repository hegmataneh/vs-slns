#define Uses_sleep
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
}

#if defined Uses_MemLEAK || !defined __COMPILING
	#ifdef ENABLE_USE_INTERNAL_C_STATISTIC
		GLOBAL_VAR extern mLeak_t __alc_hit[MLK_HASH_WIDTH][EACH_ADDR_COUNT];
	#endif
#endif

_CALLBACK_FXN void inmem_seg_cleaned_up( pass_p src_g , long v )
{
	G * _g = ( G * )src_g;
	_g->cmd.quit_noloss_data_thread_4 = 1;
#ifdef ENABLE_USE_INTERNAL_C_STATISTIC
	MARK_LINE();
#endif
}

_PRIVATE_FXN _CALLBACK_FXN void ignore_cleanup( pass_p src_g , long v )
{
	G * _g = ( G * )src_g;
	_g->cmd.quit_app_4 = 1;
#ifdef ENABLE_USE_INTERNAL_C_STATISTIC
	MARK_LINE();
#endif
}

_CALLBACK_FXN void * signal_thread( void * arg )
{
	sigset_t * set = arg;
	int sig;
	sigwait( set , &sig ); // Wait for Ctrl+C (SIGINT)
	printf( "\nSIGINT caught, stopping...\n" );
	_g->cmd.block_sending_1 = 1; // Signal all threads to stop
	_g->cmd.burst_waiting_2 = true;
	_g->cmd.quit_first_level_thread_3 = 1;

	
	distributor_subscribe_withOrder( &_g->distributors.bcast_quit , SUB_LONG , SUB_FXN( ignore_cleanup ) , _g , more_cleanup_are_ignorable );
	distributor_subscribe_withOrder( &_g->distributors.bcast_quit , SUB_LONG , SUB_FXN( inmem_seg_cleaned_up ) , _g , inmem_seg_cleaned );

	distributor_publish_long( &_g->distributors.bcast_quit , sig , SUBSCRIBER_PROVIDED );
	sub_destroy( &_g->distributors.bcast_quit );
	_g->cmd.quit_app_4 = 1;
	return NULL;
}

#ifdef ENABLE_USE_INTERNAL_C_STATISTIC
	GLOBAL_VAR char __arrr[ 100000 ] = { 0 };
	GLOBAL_VAR int __arrr_n = { 0 };
#endif

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
	//sigaddset( &set , SIGPIPE );
	pthread_sigmask( SIG_BLOCK , &set , NULL );

	// Start the signal handler thread
	pthread_create( &th_signal , NULL , signal_thread , &set );
	#endif

	distributor_publish_void( &_g->distributors.bcast_pre_cfg , SUBSCRIBER_PROVIDED ); // pre config state

	// this thread keep everything alive and just check every thing just be at right position and do not do action with very risky consequenses and exception raiser
	MM_BREAK_IF( pthread_create( &_g->trds.trd_watchdog , NULL , watchdog_executer , ( pass_p )_g ) != PTHREAD_CREATE_OK , errCreation , 0 , "Failed to create watchdog thread" );

	//pthread_create( &_g->trds.tid_input , NULL , input_thread , ( pass_p )_g );
	MM_BREAK_IF( pthread_create( &_g->trds.trd_version_checker , NULL , version_checker , ( pass_p )_g ) != PTHREAD_CREATE_OK , errCreation , 0 , "Failed to create version_checker thread" );
	MM_BREAK_IF( pthread_create( &_g->trds.trd_config_loader , NULL , config_loader , ( pass_p )_g ) != PTHREAD_CREATE_OK , errCreation , 0 , "Failed to create config_loader thread" );
	MM_BREAK_IF( pthread_create( &_g->trds.trd_config_executer , NULL , config_executer , ( pass_p )_g ) != PTHREAD_CREATE_OK , errCreation , 0 , "Failed to create config_executer thread" );

	pthread_join( _g->trds.trd_watchdog , NULL );

	//#ifdef Uses_MemLEAK
	//FILE * fl = fopen( "leak.txt" , "w+" );
	//for ( int ii = 0 ; ii < MLK_HASH_WIDTH ; ii++ )
	//{
	//	for ( int jj = 0 ; jj < EACH_ADDR_COUNT ; jj++ )
	//	{
	//		if ( __alc_hit[ ii ][ jj ].counter != 0 )
	//		{
	//			fprintf( fl , "%d %s (%lu)\n" , __alc_hit[ ii ][ jj ].counter , __alc_hit[ ii ][ jj ].klstck.temp_buf , __alc_hit[ ii ][ jj ].size );
	//		}
	//	}
	//}
	//fclose( fl );
	//#endif


	//sleep(1); // for sanitizer to dump
	_exit( 0 );
	return 0;
	BEGIN_RET
	M_V_END_RET
	sleep( 1 );
	_exit( 0 );
	return 1;
}
