#pragma once

#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wsign-conversion"

#pragma GCC diagnostic ignored "-Wcomment"


#define INPUT_MAX 256
#define BUFFER_SIZE MAX_PACKET_SIZE
#define CONFIG_ROOT_PATH "../../../../Protocol_Bridge/cfg"

//#define PARALLELISM_COUNT 1

#define LOW_PRIORITY_THREAD 1
#define NORMAL_PRIORITY_THREAD 2
#define HI_PRIORITY_THREAD 3

#define DEFAULT_BUF_SIZE 2048

#define PREALLOCAION_SIZE 4

#define VERBOSE_MODE_DEFAULT 0
#define HI_FREQUENT_LOG_INTERVAL_SEC_DEFAULT 5
#define STAT_REFERESH_INTERVAL_SEC_DEFUALT 1
#define CLOSE_APP_VAR_DEFAULT 0

#define DEFAULT_LOW_BASIC_THREAD_DELAY_NANOSEC 3000000000
#define DEFAULT_NORMAL_BASIC_THREAD_DELAY_NANOSEC 1000000000
#define DEFAULT_HI_BASIC_THREAD_DELAY_NANOSEC 1000000

#define SNAP_LEN MAX_PACKET_SIZE/*1518*/  // max bytes per packet to capture

#define HI_FREQUENT_LOG_INTERVAL ( _g->appcfg.g_cfg ? _g->appcfg.g_cfg->c.c.hi_frequent_log_interval_sec : HI_FREQUENT_LOG_INTERVAL_SEC_DEFAULT )

#define STAT_REFERESH_INTERVAL_SEC() ( _g->appcfg.g_cfg ? _g->appcfg.g_cfg->c.c.stat_referesh_interval_sec : STAT_REFERESH_INTERVAL_SEC_DEFUALT )

#define GRACEFULLY_END_THREAD() ( _g->cmd.quit_thread_3 )

//#define CLOSE_APP_VAR() ( _g->cmd.quit_app_4 )

// TODO . maybe in middle of config change bug appear ad app unexpectedly quit but that sit is very rare
#define RETRY_UNEXPECTED_WAIT_FOR_SOCK() ( _g->appcfg.g_cfg ? _g->appcfg.g_cfg->c.c.retry_unexpected_wait_for_sock : 3 )


#define NUMBER_IN_SHORT_FORM() ( _g->appcfg.g_cfg ? _g->appcfg.g_cfg->c.c.number_in_short_form : 1 )

#define LOW_THREAD_DEFAULT_DELAY_NANOSEC() ( _g->appcfg.g_cfg ? _g->appcfg.g_cfg->c.c.default_low_basic_thread_delay_nanosec : DEFAULT_LOW_BASIC_THREAD_DELAY_NANOSEC )
#define NORMAL_THREAD_DEFAULT_DELAY_NANOSEC() ( _g->appcfg.g_cfg ? _g->appcfg.g_cfg->c.c.default_normal_basic_thread_delay_nanosec : DEFAULT_NORMAL_BASIC_THREAD_DELAY_NANOSEC )
#define HI_THREAD_DEFAULT_DELAY_NANOSEC() ( _g->appcfg.g_cfg ? _g->appcfg.g_cfg->c.c.default_hi_basic_thread_delay_nanosec : DEFAULT_HI_BASIC_THREAD_DELAY_NANOSEC )


#define STR_RoundRobin "RR"
#define STR_Replicate "Replicate"
#define STR_ONE_OUT "one_out"


#define DIST_BRIDGE_FAILURE() DO_WHILE( distributor_publish_str( &_g->distributors.pb_lvl_failure_dist , __FUNCTION__ , ( pass_p )pb ) ) /*distribute error*/
#define DIST_APP_FAILURE() DO_WHILE( distributor_publish_str( &_g->distributors.app_lvl_failure_dist , __FUNCTION__ , ( pass_p )_g ) ) /*distribute error in config reading*/

typedef  char CONFIG_SECTION_ITEM_VALUE  [64];
typedef  CONFIG_SECTION_ITEM_VALUE  CFG_ITM;

#define _FORMAT_SHRTFRM( baaf , NPP , val , decimal_precision , unit ) ( NUMBER_IN_SHORT_FORM() ? /*make cell string in short form or long*/ \
		format_pps( baaf , sizeof(baaf) , val , decimal_precision , unit ) :\
		__snprintf( baaf , sizeof(baaf) , "%llu" , val ) )


extern char __arrr[ 10000 ];
extern int __arrr_n;

enum cleanup_priority_order /*ascending termination priority*/
{
	clean_globals ,
	clean_config ,

	clean_persistant_cache_mgr ,
	clean_packet_mngr , /*most have higher priority than persistant_cache_mgr*/

	clean_threads , /*wait until all thread go away*/

	clean_try_post_packet , // before thread goes down

	clean_bridges ,

	clean_connections , // close connection for no more data
};

