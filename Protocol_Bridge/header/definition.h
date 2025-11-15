#pragma once

#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wunused-variable"
//#pragma GCC diagnostic ignored "-Wcomment"


#define INPUT_MAX 256
#define BUFFER_SIZE MAX_PACKET_SIZE
#define CONFIG_ROOT_PATH "../../../../Protocol_Bridge/cfg"

//#define PARALLELISM_COUNT 1

#define LOW_PRIORITY_THREAD 1
#define NORMAL_PRIORITY_THREAD 2
#define HI_PRIORITY_THREAD 3
#define VLOW_PRIORITY_THREAD 4

#define DEFAULT_BUF_SIZE 2048

#define PREALLOCAION_SIZE 4

#define VERBOSE_MODE_DEFAULT 0
#define HI_FREQUENT_LOG_INTERVAL_SEC_DEFAULT 5
#define STAT_REFERESH_INTERVAL_SEC_DEFUALT 1
#define CLOSE_APP_VAR_DEFAULT 0

#define DEFAULT_VLOW_BASIC_THREAD_DELAY_NANOSEC 10000000000 /*10 sec*/

#define DEFAULT_LOW_BASIC_THREAD_DELAY_NANOSEC 3000000000 /*3 sec*/
#define DEFAULT_NORMAL_BASIC_THREAD_DELAY_NANOSEC 1000000000 /*1 sec*/
#define DEFAULT_HI_BASIC_THREAD_DELAY_NANOSEC 1000000 /*1 milsec*/

#define SNAP_LEN MAX_PACKET_SIZE/*1518*/  // max bytes per packet to capture

#define HI_FREQUENT_LOG_INTERVAL ( _g->appcfg.g_cfg ? _g->appcfg.g_cfg->c.c.hi_frequent_log_interval_sec : HI_FREQUENT_LOG_INTERVAL_SEC_DEFAULT )

#define STAT_REFERESH_INTERVAL_SEC() ( _g->appcfg.g_cfg ? (uint)_g->appcfg.g_cfg->c.c.stat_referesh_interval_sec : STAT_REFERESH_INTERVAL_SEC_DEFUALT )

#define GRACEFULLY_END_THREAD() ( _g->cmd.quit_first_level_thread_3 )

#define GRACEFULLY_END_NOLOSS_THREAD() ( _g->cmd.quit_noloss_data_thread_4 )

//#define CLOSE_APP_VAR() ( _g->cmd.quit_app_4 )

// TODO . maybe in middle of config change bug appear ad app unexpectedly quit but that sit is very rare
#define RETRY_UNEXPECTED_WAIT_FOR_SOCK() ( _g->appcfg.g_cfg ? _g->appcfg.g_cfg->c.c.retry_unexpected_wait_for_sock : 3 )

#define NUMBER_IN_SHORT_FORM() ( _g->appcfg.g_cfg ? _g->appcfg.g_cfg->c.c.number_in_short_form : 1 )

#define VLOW_THREAD_DEFAULT_DELAY_NANOSEC() DEFAULT_LOW_BASIC_THREAD_DELAY_NANOSEC
#define LOW_THREAD_DEFAULT_DELAY_NANOSEC() ( _g->appcfg.g_cfg ? _g->appcfg.g_cfg->c.c.default_low_basic_thread_delay_nanosec : DEFAULT_LOW_BASIC_THREAD_DELAY_NANOSEC )
#define NORMAL_THREAD_DEFAULT_DELAY_NANOSEC() ( _g->appcfg.g_cfg ? _g->appcfg.g_cfg->c.c.default_normal_basic_thread_delay_nanosec : DEFAULT_NORMAL_BASIC_THREAD_DELAY_NANOSEC )
#define HI_THREAD_DEFAULT_DELAY_NANOSEC() ( _g->appcfg.g_cfg ? _g->appcfg.g_cfg->c.c.default_hi_basic_thread_delay_nanosec : DEFAULT_HI_BASIC_THREAD_DELAY_NANOSEC )


#define STR_RoundRobin "RR"
#define STR_Replicate "Replicate"
#define STR_ONE_OUT "one_out"


#define DIST_BRIDGE_FAILURE() DO_WHILE( distributor_publish_str( &_g->distributors.bcast_pb_lvl_failure , __FUNCTION__ , ( pass_p )pb ) ) /*distribute error*/
#define DIST_APP_FAILURE() DO_WHILE( distributor_publish_str( &_g->distributors.bcast_app_lvl_failure , __FUNCTION__ , ( pass_p )_g ) ) /*distribute error in config reading*/

typedef  char CONFIG_SECTION_ITEM_VALUE  [64];
typedef  CONFIG_SECTION_ITEM_VALUE  CFG_ITM;

#define _FORMAT_SHRTFRM( baaf , NPP , val , decimal_precision , unit_s ) ( NUMBER_IN_SHORT_FORM() ? /*make cell string in short form or long*/ \
		format_pps( baaf , sizeof(baaf) , val , decimal_precision , unit_s ) :\
		__snprintf( baaf , sizeof(baaf) , "%llu%s%s" , val , *unit_s ? " " : "", unit_s ) )


enum pre_main_priority_order /*top down startup priority*/
{
	pre_main_init_main = 101,
	pre_main_init_config = 102,
	pre_main_init_statistics = 103,
	pre_main_init_globals = 104,
	pre_main_init_bridges = 105,
	pre_main_init_packet_mngr = 106,
	pre_main_init_persistant_cache_mngr = 107,
};

#define PRE_MAIN_INIT_MAIN pre_main_init_main
#define PRE_MAIN_INIT_CONFIG pre_main_init_config
#define PRE_MAIN_INIT_STATISTICS pre_main_init_statistics
#define PRE_MAIN_INIT_GLOBALS pre_main_init_globals
#define PRE_MAIN_INIT_BRIDGES pre_main_init_bridges
#define PRE_MAIN_INIT_PACKET_MNGR pre_main_init_packet_mngr
#define PRE_MAIN_INIT_PERSISTANT_CACHE_MNGR pre_main_init_persistant_cache_mngr

enum program_stablity_bcast_order /*bottom up termination priority*/
{
	tcp_thread_trigger , // tcp thread start after config completely loaded
	config_stablity ,
};

typedef enum cleanup_priority_order /*bottom up termination priority*/
{
	clean_globals ,
	clean_globals_shared_var ,
	clean_config ,
	clean_stat ,
	
	more_cleanup_are_ignorable ,
	
	clean_persistant_cache_mgr ,
	clean_packet_mngr , /*most have higher priority than persistant_cache_mgr*/
	clean_threads , /*wait until all thread go away*/
	inmem_seg_cleaned ,
	cleaned_segments_state = inmem_seg_cleaned, /*do not use this*/
	
	wait_until_no_more_unsaved_packet ,

	// all segments closed

	getting_new_udp_stoped , // i put after stop_sending_from_cach_mgr . so no more oacket put into huge cache or Mem2
	stop_sending_from_cach_mgr , // before thread goes down

	// each pb send_stoped = 1
	// each pb stop_sending = 1

	stop_sending_from_bridge ,
	
	// receive_stoped = true
	// stop_receiving = true
	
	bridge_insure_input_bus_stoped ,
	stop_input_udp , // close connection for no more input data

	_begin_cleanup_item ,
} cleanup_priority_order;

enum stat_init_priority_order /*bottom up termination priority*/
{
	statistics_is_stabled , // at last call like event
	bridge_statistics ,
	packetmgr_statistics ,
	main_statistics
};

#ifndef control_app_segment_in_deep

#define HAS_STATISTICSS

#define ENABLE_COMMUNICATION

#define ENABLE_PERSISTENT_CACHE

#define ENABLE_FILLED_TCP_SEGMENT_PROC

#define ENABLE_CLEAN_UNUSED_SEGMENT

#define ENABLE_GATHER_STATIC

// /*comment by default*/ #define SEND_DIRECTLY_ARRIVE_UDP /*comment by default*/

#define ENABLE_VERBOSE_FAULT

#define ENABLE_THROUGHPUT_MEASURE

#define ENABLE_HALFFILL_SEGMENT

#define ENABLE_LOCK_ON_CONFIG // TODO

#define ENABLE_BRIDGE_THREAD_CREATION

#define ENABLE_TCP_OUT_PROC

#define ENABLE_PCAP_LOOP_PREQ /*pcap and prerequisit that needed*/

#define ENABLE_BYPASS_STDOUT

#define ENABLE_KEEPALIVE_CHAOTIC

#endif
