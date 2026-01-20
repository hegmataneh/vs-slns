#pragma once

typedef struct Global_Config_0
{
	const char * create_date; // "2025-06-21 20:43:00"
	const char * modify_date;
	const char * config_name;
	const char * config_tags;
	const char * description;
	const char * log_level; // no , error , warn , verbose,
	const char * log_file;

	union
	{
		struct
		{
			int load_prev_config;
			int dump_current_config;
			int dump_prev_config;
			int socket_def_timeout_sec;
			int verbose_mode;
			int log_cooldown_sec;
			int stat_refresh_interval_sec;
			int number_in_short_form;
			int precision_of_double_in_short_form;
			int64 low_priority_thread_cooldown_delay_nanosec;
			int64 normal_priority_thread_cooldown_delay_nanosec;
			int64 hi_priority_thread_cooldown_delay_nanosec;
			int64 very_hi_priority_thread_cooldown_delay_nanosec;
			int64 L2_segment_capacity;
			int64 L2_segment_offsets_cnt_base;
			int L2_idle_active_segment_expiration_timeout_sec;
			int64 L2_allowed_ram_allocation;
			float instantaneous_input_load_coefficient;
			int TTF_no_backpressure_threshold_sec;
			int TTF_gentle_backpressure_threshold_sec;
			int TTF_aggressive_backpressure_threshold_sec;
			int TTF_emergency_drop_backpressure_threshold_sec;
			int TTF_skip_input_threshold_sec;
			int TTF_gentle_backpressure_stride;
			int TTF_aggressive_backpressure_stride;
			int TTF_emergency_drop_backpressure_stride;
			int TTF_red_zone_stride;
			int L2_wait_until_cleaning_up_sec;
			int L1_cache_sz_byte;
			int network_handshake_pessimistic_timeout_sec;
			int retry_on_idle_tcp_conn_after_timeout_sec;
			int L2_flood_evaluator_sample_need_count;
			int L2_long_term_evaluator_sample_need_count;
			int L2_old_udp_holdon_timeout_sec;
			int L2_unused_segment_holdon_time_sec;
			int udp_id_valid_until_timeout_msec;
			int ipv4_reassembly_timeout_msec;
			int infinite_loop_guard;
			int L2_idle_active_iteration_sec;
			int max_saved_file_size_threshold_MB;
			int L1_2_L2_byte_copy_latency_nsec;

			// everything add here most be copy down there

		} cfg_change_pck; /*config that are solid*/

		struct
		{
			int load_prev_config;
			int dump_current_config;
			int dump_prev_config;
			int socket_def_timeout_sec;
			int verbose_mode;
			int log_cooldown_sec;
			int stat_refresh_interval_sec;
			int number_in_short_form;
			int precision_of_double_in_short_form;
			int64 low_priority_thread_cooldown_delay_nanosec;
			int64 normal_priority_thread_cooldown_delay_nanosec;
			int64 hi_priority_thread_cooldown_delay_nanosec;
			int64 very_hi_priority_thread_cooldown_delay_nanosec;
			int64 L2_segment_capacity;
			int64 L2_segment_offsets_cnt_base;
			int L2_idle_active_segment_expiration_timeout_sec;
			int64 L2_allowed_ram_allocation;
			float instantaneous_input_load_coefficient;
			int TTF_no_backpressure_threshold_sec;
			int TTF_gentle_backpressure_threshold_sec;
			int TTF_aggressive_backpressure_threshold_sec;
			int TTF_emergency_drop_backpressure_threshold_sec;
			int TTF_skip_input_threshold_sec;
			int TTF_gentle_backpressure_stride;
			int TTF_aggressive_backpressure_stride;
			int TTF_emergency_drop_backpressure_stride;
			int TTF_red_zone_stride;
			int L2_wait_until_cleaning_up_sec;
			int L1_cache_sz_byte;
			int network_handshake_pessimistic_timeout_sec;
			int retry_on_idle_tcp_conn_after_timeout_sec;
			int L2_flood_evaluator_sample_need_count;
			int L2_long_term_evaluator_sample_need_count;
			int L2_old_udp_holdon_timeout_sec;
			int L2_unused_segment_holdon_time_sec;
			int udp_id_valid_until_timeout_msec;
			int ipv4_reassembly_timeout_msec;
			int infinite_loop_guard;
			int L2_idle_active_iteration_sec;
			int max_saved_file_size_threshold_MB;
			int L1_2_L2_byte_copy_latency_nsec;

			// everything add here most be copy up here
		};
	};

} Gcfg0;

typedef struct Global_Config_n
{
	Gcfg0 c; // first member , n - 1
} Gcfgn;

typedef struct Global_Config // finalizer
{
	Gcfgn c; // first member
	// ...
} Gcfg;

typedef struct App_Config // global config
{
	struct Config_ver
	{
		CFG_ITM version;
		int Major; //Indicates significant changes , potentially including incompatible API changes.
		int Minor; //Denotes new functionality added in a backwards - compatible manner.
		int Build; //Represents the specific build number of the software.
		int Revision_Patch; //Represents backwards - compatible bug fixes or minor updates.
	} temp_ver /* not usable just to prevent reallocation*/ , * ver; // app version
	int version_changed; // act like Boolean . this var indicate just load new config

	// general
	Gcfg * prev_cfg;
	Gcfg * g_cfg;
	int g_cfg_changed;

	// protocol_bridge 
	brg_cfg_t * old_bdj_psv_cfg; // maybe later in some condition we need to rollback to prev config
	size_t old_bdj_psv_cfg_count;

	brg_cfg_t * bdj_psv_cfg; // passive config
	size_t bdj_psv_cfg_count;

	int psv_cfg_changed; // somewhere in cfg something was changed
	pthread_mutex_t cfg_mtx;
	bool cfg_mtx_protector;

	bool already_main_cfg_stablished;

} Acfg;

#define CFG() ( _g->appcfg.g_cfg->c.c )

_THREAD_FXN void_p version_checker( pass_p src_g );
_THREAD_FXN void_p config_loader( pass_p src_g );
_THREAD_FXN void_p config_executer( pass_p src_g );
