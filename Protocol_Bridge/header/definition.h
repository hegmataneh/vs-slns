#pragma once

#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wunused-variable"
//#pragma GCC diagnostic ignored "-Wcomment"


#define INPUT_MAX 256
#define BUFFER_SIZE MAX_PACKET_SIZE
#define CONFIG_ROOT_PATH "../../../../Protocol_Bridge/cfg"

//#define PARALLELISM_COUNT 1

typedef enum
{
	LOW_PRIORITY_THREAD = 1,
	NORMAL_PRIORITY_THREAD = 2,
	HI_PRIORITY_THREAD = 3,
	VLOW_PRIORITY_THREAD = 4
} etrd_priority;

#define DEFAULT_BUF_SIZE 2048

#define VERBOSE_MODE_DEFAULT 0
#define LOG_COOLDOWN_SEC 5
#define REFRESH_INTERVAL_SEC_DEFUALT 1
#define CLOSE_APP_VAR_DEFAULT 0

#define DEFAULT_VLOW_BASIC_THREAD_DELAY_NANOSEC 10000000000 /*10 sec*/

#define DEFAULT_LOW_BASIC_THREAD_DELAY_NANOSEC 3000000000 /*3 sec*/
#define DEFAULT_NORMAL_BASIC_THREAD_DELAY_NANOSEC 1000000000 /*1 sec*/
#define DEFAULT_HI_BASIC_THREAD_DELAY_NANOSEC 1000000 /*1 milsec*/

#define DEFAULT_INFINITE_LOOP_GUARD 1000

#define SNAP_LEN MAX_PACKET_SIZE/*1518*/  // max bytes per packet to capture

#define HI_FREQUENT_LOG_INTERVAL ( _g->appcfg.g_cfg ? CFG().log_cooldown_sec : LOG_COOLDOWN_SEC )

#define REFRESH_INTERVAL_SEC() ( _g->appcfg.g_cfg ? (uint)CFG().refresh_interval_sec : REFRESH_INTERVAL_SEC_DEFUALT )

#define GRACEFULLY_END_THREAD() ( _g->cmd.quit_first_level_thread_3 )

#define GRACEFULLY_END_NOLOSS_THREAD() ( _g->cmd.quit_noloss_data_thread_4 )

//#define CLOSE_APP_VAR() ( _g->cmd.quit_app_4 )

#define NUMBER_IN_SHORT_FORM() ( _g->appcfg.g_cfg ? CFG().number_in_short_form : 1 )

#define DOUBLE_PRECISION() ( _g->appcfg.g_cfg ? CFG().precision_of_double_in_short_form : 1 )


#define VLOW_THREAD_DEFAULT_DELAY_NANOSEC() DEFAULT_LOW_BASIC_THREAD_DELAY_NANOSEC
#define LOW_THREAD_DEFAULT_DELAY_NANOSEC() ( _g->appcfg.g_cfg ? CFG().low_priority_thread_cooldown_delay_nanosec : DEFAULT_LOW_BASIC_THREAD_DELAY_NANOSEC )
#define NORMAL_THREAD_DEFAULT_DELAY_NANOSEC() ( _g->appcfg.g_cfg ? CFG().normal_priority_thread_cooldown_delay_nanosec : DEFAULT_NORMAL_BASIC_THREAD_DELAY_NANOSEC )
#define HI_THREAD_DEFAULT_DELAY_NANOSEC() ( _g->appcfg.g_cfg ? CFG().hi_priority_thread_cooldown_delay_nanosec : DEFAULT_HI_BASIC_THREAD_DELAY_NANOSEC )

#define BAD_NETWORK_HANDSHAKE_TIMEOUT() ( _g->appcfg.g_cfg ? CFG().network_handshake_pessimistic_timeout_sec : DEFAULT_BAD_NETWORK_HANDSHAKE_TIMEOUT )

#define INFINITE_LOOP_GUARD() ( _g->appcfg.g_cfg ? CFG().infinite_loop_guard : DEFAULT_INFINITE_LOOP_GUARD )



#define STR_RoundRobin "RR"
#define STR_Replicate "Replicate"
#define STR_ONE_OUT "one_out"


#define DIST_BRIDGE_FAILURE() DO_WHILE( distributor_publish_str( &_g->distributors.bcast_pb_lvl_failure , __FUNCTION__ , ( pass_p )pb ) ) /*distribute error*/
#define DIST_APP_FAILURE() DO_WHILE( distributor_publish_str( &_g->distributors.bcast_app_lvl_failure , __FUNCTION__ , ( pass_p )_g ) ) /*distribute error in config reading*/

typedef  char CONFIG_SECTION_ITEM_VALUE  [DEFAULT_SFS_BUF_SZ];
typedef  CONFIG_SECTION_ITEM_VALUE  CFG_ITM;

#define _FORMAT_SHRTFRM_SNPRINTF_BY_TYPE( buf , val , decimal_precision , unit_s , prefix_string ) \
    ({_Generic((val), \
        int:        __snprintf( buf , sizeof(buf) , "%s%d%s%s" , ""prefix_string"" , val , *unit_s ? " " : "", unit_s ) , \
        long:       __snprintf( buf , sizeof(buf) , "%s%ld%s%s" , ""prefix_string"" , val , *unit_s ? " " : "", unit_s ) , \
        long long:  __snprintf( buf , sizeof(buf) , "%s%lld%s%s" , ""prefix_string"" , val , *unit_s ? " " : "", unit_s ) , \
        unsigned long long: __snprintf( buf , sizeof(buf) , "%s%llu%s%s" , ""prefix_string"" , val , *unit_s ? " " : "", unit_s ) , \
        double:     __snprintf( buf , sizeof(buf) , "%s%.2f%s%s" , ""prefix_string"" , val , *unit_s ? " " : "", unit_s ) , \
        default:    __snprintf( buf , sizeof(buf) , "%s%.2f%s%s" , ""prefix_string"" , (double)val , *unit_s ? " " : "", unit_s ) /* fallback */ \
	); })

#define _FORMAT_SHRTFRM( baaf , NPP , val , decimal_precision , unit_s , prefix_string ) \
		( NUMBER_IN_SHORT_FORM() ? /*make cell string in short form or long*/ \
		format_pps_double( baaf , sizeof(baaf) , (double)val , decimal_precision , ""unit_s"" , ""prefix_string"" ) :\
		_FORMAT_SHRTFRM_SNPRINTF_BY_TYPE( baaf , val , decimal_precision , ""unit_s"" , ""prefix_string"" )/*;*/ \
		)


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

enum postcfg_order
{
	post_config_order_last_call , /*in this step set post config is called to true*/
	post_config_order_bridges ,
	post_config_order_persistant_cache_mngr ,
	post_config_order_packet_mngr ,
	post_config_order_statistics ,
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
	bridge_overview ,
	thread_overview ,
	packetmgr_overview ,
	app_overview ,
};

enum thread_used
{
	trdn_version_checker ,
	trdn_config_loader ,
	trdn_config_executer ,
	trdn_connect_udps_proc ,
	trdn_thread_tcp_connection_proc ,
	trdn_watchdog_executer ,
	trdn_stdout_bypass_thread ,
	trdn_sync_thread ,
	trdn_input_thread ,
	trdn_process_filled_tcp_segment_proc ,
	trdn_cleanup_unused_segment_proc ,
	trdn_evacuate_old_segment_proc ,
	trdn_discharge_persistant_cache_proc ,
	trdn_stats_thread ,
	trdn_proc_pcap_udp_counter ,
	trdn_proc_krnl_udp_counter ,
	trdn_proc_one2one_pcap2krnl_SF_udp_pcap ,
	trdn_proc_one2one_pcap2krnl_SF_tcp_out ,
	trdn_proc_one2many_pcap2krnl_SF_udp_pcap ,
	trdn_proc_one2many_tcp_out ,
	trdn_proc_many2many_pcap_krnl_SF ,
	trdn_proc_one2one_krnl_udp_store ,
	trdn_proc_many2many_krnl_udp_store ,
};

#ifndef control_app_segment_in_deep

	#define HAS_STATISTICSS /*be defined 14040930*/

	#define ENABLE_COMMUNICATION /*be defined 14040930*/

	#define ENABLE_PERSISTENT_CACHE /*be defined 14040930*/

	#define ENABLE_FILLED_TCP_SEGMENT_PROC /*be defined 14040930*/

	#define ENABLE_CLEAN_UNUSED_SEGMENT /*be defined 14040930*/

	#define ENABLE_ABSOLETE_OLD_SEGMENT /*be defined 14040930*/

	#define ENABLE_GATHER_STATIC /*be defined 14040930*/

	// /*comment by default*/ #define SEND_DIRECTLY_ARRIVE_UDP /*comment by default*/

	#define ENABLE_VERBOSE_FAULT /*be defined 14040930*/

	#define ENABLE_THROUGHPUT_MEASURE /*be defined 14040930*/

	#define ENABLE_HALFFILL_SEGMENT /*be defined 14040930*/

	#define ENABLE_LOCK_ON_CONFIG // TODO

	#define ENABLE_BRIDGE_THREAD_CREATION /*be defined 14040930*/

	#define ENABLE_TCP_OUT_PROC /*be defined 14040930*/

	#define ENABLE_PCAP_LOOP_PREQ /*pcap and prerequisit that needed*/

	//#define ENABLE_BYPASS_STDOUT // be comment

	#define ENABLE_KEEPALIVE_CHAOTIC /*be defined 14040930*/

	#define ENABLE_LOG_THREADS /*always defined*/

	#define ENABLE_MEMMAP_SYNC_FOR_EACH_WRITE /*be defined 14040930*/

	#define ENABLE_ON_PCAP_TCP_OUT /*be defined 14040930*/

	#define ENABLE_STAT_THREAD_PROC /*be defined 14040930*/

	#define ENALBE_LOCK_IN_UDP_DEFRAGMENTATOR /*be defined 14040930*/

	#define ENABLE_REMAP_UNSEDABLE_PACKET /*be defined 14040930*/

	#define ENALBE_PASS_DEFRAGED_PACKET_INTO_L2 /*be defined 1404100*/

#endif
