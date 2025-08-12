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
		char version[ 64 ];
		int Major; //Indicates significant changes , potentially including incompatible API changes.
		int Minor; //Denotes new functionality added in a backwards - compatible manner.
		int Build; //Represents the specific build number of the software.
		int Revision_Patch; //Represents backwards - compatible bug fixes or minor updates.
	} ___temp_ver /* not usable just to prevent reallocation*/
		, * _ver; // app version
	int _version_changed; // act like bool . this var indicate just load new config

	// general
	Gcfg * _prev_cfg;
	Gcfg * _g_cfg;
	int _g_cfg_changed;

	// protocol_bridge 
	Bcfg * old_bdj_psv_cfg; // maybe later in some condition we need to rollback to prev config
	size_t _old_bdj_psv_cfg_count;

	Bcfg * _bdj_psv_cfg; // passive config
	size_t _bdj_psv_cfg_count;

	int _psvcfg_changed; // act like bool . something is changed
} Acfg;

_THREAD_FXN void * version_checker( void * app_data );
_THREAD_FXN void * config_loader( void * app_data );
_THREAD_FXN void * config_executer( void * app_data );
