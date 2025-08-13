#pragma once

#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wsign-conversion"

#pragma GCC diagnostic ignored "-Wcomment"


#define INPUT_MAX 256
#define BUFFER_SIZE MAX_PACKET_SIZE
#define CONFIG_ROOT_PATH "/root/my_projects/home-config/protocol_Bridge"

//#define PARALLELISM_COUNT 1

#define BUF_SIZE 2048

#define PREALLOCAION_SIZE 10

#define VERBOSE_MODE_DEFAULT 0
#define HI_FREQUENT_LOG_INTERVAL_SEC_DEFAULT 5
#define STAT_REFERESH_INTERVAL_SEC_DEFUALT 1
#define CLOSE_APP_VAR_DEFAULT 0

#define HI_FREQUENT_LOG_INTERVAL ( _g->appcfg.g_cfg ? _g->appcfg.g_cfg->c.c.hi_frequent_log_interval_sec : HI_FREQUENT_LOG_INTERVAL_SEC_DEFAULT )

#define STAT_REFERESH_INTERVAL_SEC() ( _g->appcfg.g_cfg ? _g->appcfg.g_cfg->c.c.stat_referesh_interval_sec : STAT_REFERESH_INTERVAL_SEC_DEFUALT )
#define CLOSE_APP_VAR() ( _g->cmd.quit_app )

// TODO . maybe in middle of config change bug appear ad app unexpectedly quit but that sit is very rare
#define RETRY_UNEXPECTED_WAIT_FOR_SOCK() ( _g->appcfg.g_cfg ? _g->appcfg.g_cfg->c.c.retry_unexpected_wait_for_sock : 3 )


#define NUMBER_IN_SHORT_FORM() ( _g->appcfg.g_cfg ? _g->appcfg.g_cfg->c.c.number_in_short_form : 1 )



//#define FXN_HIT_COUNT 5000
//#define PC_COUNT 10 // first for hit count and last alwayz zero

//#define SYS_ALIVE_CHECK() do {\
//	int __function_line = __LINE__; _g->stat.last_line_meet = __LINE__; _g->stat.alive_check_counter = ( _g->stat.alive_check_counter + 1 ) % 10; __FXN_HIT[__function_line][0]++; static int pc = 0;/*each line hit*/ if ( pc <= PC_COUNT-1 ) __FXN_HIT[__function_line][1+pc++] = _pc++; \
//	} while(0)



