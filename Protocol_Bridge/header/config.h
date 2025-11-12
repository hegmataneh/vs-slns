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

	int enable;
	int shutdown;
	int watchdog_enabled;
	int load_prev_config;
	int dump_current_config;
	int dump_prev_config;
	int time_out_sec;

	int verbose_mode;
	int hi_frequent_log_interval_sec;
	int refresh_variable_from_scratch;
	int stat_referesh_interval_sec;

	int synchronization_min_wait;
	int synchronization_max_roundup;
	int show_line_hit;
	int retry_unexpected_wait_for_sock;
	int number_in_short_form;
	const char * NetworkStack_FilterType;
	int64 default_low_basic_thread_delay_nanosec;
	int64 default_normal_basic_thread_delay_nanosec;
	int64 default_hi_basic_thread_delay_nanosec;

	int64 pkt_mgr_segment_capacity;
	int64 pkt_mgr_offsets_capacity;

	int pkt_mgr_maximum_keep_unfinished_segment_sec;

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

_THREAD_FXN void_p version_checker( pass_p src_g );
_THREAD_FXN void_p config_loader( pass_p src_g );
_THREAD_FXN void_p config_executer( pass_p src_g );
