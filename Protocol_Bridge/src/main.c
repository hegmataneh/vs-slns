#define Uses_log_init
#define Uses_MARK_LINE
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

_GLOBAL_VAR G * _g = NULL; // just one global var

/// <summary>
/// اینجوری می خواستم هر قسمت را یک کامپوننت بدانم و در نتیجه هر کسی مسئول ایجاد زیر ساختهای لازم برای خودش است
/// </summary>
PRE_MAIN_INITIALIZATION( PRE_MAIN_INIT_MAIN )
_PRIVATE_FXN void pre_main_top_prio_init( void )
{
	static G g = {0};
	_g = &g;

	#ifdef ENABLE_LOGGING
	char meta_path[ MAX_PATH ] = {0};
	time_t tnow = time( NULL );
	snprintf( meta_path , sizeof( meta_path ) , "./log%ld.txt" , tnow );
	log_init( meta_path , true );
	#endif
}

#if defined Uses_MemLEAK || !defined __COMPILING
	#ifdef ENABLE_USE_DBG_TAG
		_GLOBAL_VAR _EXTERN mLeak_t __alc_hit[MLK_HASH_WIDTH][EACH_ADDR_COUNT];
	#endif
#endif

_CALLBACK_FXN void inmem_seg_cleaned_up( pass_p src_g , long v )
{
	G * _g = ( G * )src_g;
	_g->cmd.quit_noloss_data_thread_4 = 1;
	_g->cmd.cleanup_state = cleaned_segments_state;
}

_PRIVATE_FXN _CALLBACK_FXN void ignore_cleanup( pass_p src_g , long v )
{
	G * _g = ( G * )src_g;
	_g->cmd.quit_app_4 = 1;
}

_CALLBACK_FXN void * signal_thread( void * arg )
{
	sigset_t * set = arg;
	int sig;
	sigwait( set , &sig ); // Wait for Ctrl+C (SIGINT)
	
#ifdef ENABLE_USE_DBG_TAG
	MARK_LINE();
#endif
	
	printf( "\nSIGINT caught, stopping...\n" );
	_g->cmd.cleanup_state = _begin_cleanup_item;
	_g->cmd.block_sending_1 = true;
	_g->cmd.burst_waiting_2 = true;
	_g->cmd.quit_first_level_thread_3 = true;

	distributor_subscribe_withOrder( &_g->distributors.bcast_quit , SUB_LONG , SUB_FXN( ignore_cleanup ) , _g , more_cleanup_are_ignorable );
	distributor_subscribe_withOrder( &_g->distributors.bcast_quit , SUB_LONG , SUB_FXN( inmem_seg_cleaned_up ) , _g , inmem_seg_cleaned );

	distributor_publish_long( &_g->distributors.bcast_quit , NP , SUBSCRIBER_PROVIDED );
	sub_destroy( &_g->distributors.bcast_quit );
	_g->cmd.quit_app_4 = 1;
	return NULL;
}

#ifdef ENABLE_USE_DBG_TAG
	_GLOBAL_VAR _EXTERN char __arrr[ 100000 ];
	_GLOBAL_VAR _EXTERN int TMP_DUMP_BUFF_N;
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
	
#ifdef ENABLE_USE_DBG_TAG
	MARK_LINE();
#endif
	
	MM_BREAK_IF( pthread_create( &_g->trds.trd_config_loader , NULL , config_loader , ( pass_p )_g ) != PTHREAD_CREATE_OK , errCreation , 0 , "Failed to create config_loader thread" );
	
#ifdef ENABLE_USE_DBG_TAG
	MARK_LINE();
#endif
	
	MM_BREAK_IF( pthread_create( &_g->trds.trd_config_executer , NULL , config_executer , ( pass_p )_g ) != PTHREAD_CREATE_OK , errCreation , 0 , "Failed to create config_executer thread" );

#ifdef ENABLE_USE_DBG_TAG
	MARK_LINE();
#endif

	init_UI( _g );


	pthread_join( _g->trds.trd_watchdog , NULL );

#ifdef ENABLE_USE_DBG_TAG
	MARK_LINE();
#endif

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

	#ifdef ENABLE_LOGGING
		log_close();
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
