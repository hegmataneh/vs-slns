#define Uses_LOCK_LINE
#define Uses__MK_MSG
#define Uses_MARK_START_THREAD
#define Uses_strcasecmp
#define Uses_WARNING
#define Uses_STR_DIFF
#define Uses_MEMSET
#define Uses_INIT_BREAKABLE_FXN
#define Uses_TWD
#define Uses_pthread_t
#define Uses_json
#define Uses_config
#define Uses_globals
#include <Protocol_Bridge.dep>

_GLOBAL_VAR _EXTERN G * _g;

_PRIVATE_FXN _CALLBACK_FXN void cleanup_config( pass_p src_g , long v )
{
	G * _g = ( G * )src_g;
	if ( _g->appcfg.prev_cfg )
	{
		DAC( _g->appcfg.prev_cfg->c.c.create_date );
		DAC( _g->appcfg.prev_cfg->c.c.modify_date );
		DAC( _g->appcfg.prev_cfg->c.c.config_name );
		DAC( _g->appcfg.prev_cfg->c.c.config_tags );
		DAC( _g->appcfg.prev_cfg->c.c.description );
		DAC( _g->appcfg.prev_cfg->c.c.log_level );
		DAC( _g->appcfg.prev_cfg->c.c.log_file );
		
		DAC( _g->appcfg.prev_cfg );
	}
	if ( _g->appcfg.g_cfg )
	{
		DAC( CFG().create_date );
		DAC( CFG().modify_date );
		DAC( CFG().config_name );
		DAC( CFG().config_tags );
		DAC( CFG().description );
		DAC( CFG().log_level );
		DAC( CFG().log_file );
		
		DAC( _g->appcfg.g_cfg );
	}

	if ( _g->appcfg.old_bdj_psv_cfg )
	{
		for ( size_t idx = 0 ; idx < _g->appcfg.old_bdj_psv_cfg_count ; idx++ )
		{
			DAC( _g->appcfg.old_bdj_psv_cfg->m.m.maintained.in );
			DAC( _g->appcfg.old_bdj_psv_cfg->m.m.maintained.out );
		}
		DAC( _g->appcfg.old_bdj_psv_cfg );
	}

	if ( _g->appcfg.bdj_psv_cfg )
	{
		for ( size_t idx = 0 ; idx < _g->appcfg.bdj_psv_cfg_count ; idx++ )
		{
			//DAC( _g->appcfg.bdj_psv_cfg->m.m.maintained.in );
			//DAC( _g->appcfg.bdj_psv_cfg->m.m.maintained.out );
		}
		DAC( _g->appcfg.bdj_psv_cfg );
	}
}

_CALLBACK_FXN _PRIVATE_FXN void state_pre_config_init_config( void_p src_g )
{
	INIT_BREAKABLE_FXN();
	G * _g = ( G * )src_g;

	M_BREAK_STAT( distributor_subscribe_withOrder( &_g->distributors.bcast_quit , SUB_LONG , SUB_FXN( cleanup_config ) , _g , clean_config ) , 0 );
	M_BREAK_STAT( distributor_subscribe_withOrder( &_g->distributors.bcast_quit , SUB_LONG , SUB_FXN( try_stoping_sending_from_bridge ) , _g , stop_sending_from_bridge ) , 0 );

	BEGIN_RET
	default:
	{
		if ( d_error ) DIST_APP_FAILURE();
	}
	M_V_END_RET
}

_CALLBACK_FXN _PRIVATE_FXN void post_config_completed( void_p src_g )
{
	G * _g = ( G * )src_g;
	_g->distributors.bafter_post_cfg_called = true;
}

_CALLBACK_FXN _PRIVATE_FXN void event_program_is_stabled_cfg( void_p src_g )
{
	INIT_BREAKABLE_FXN();
	G * _g = ( G * )src_g;

	M_BREAK_STAT( distributor_subscribe_withOrder( &_g->distributors.bcast_post_cfg , SUB_VOID , SUB_FXN( post_config_completed ) , _g , post_config_order_last_call ) , 0 ); // just before publish but call last
	M_BREAK_STAT( distributor_publish_void( &_g->distributors.bcast_post_cfg , SUBSCRIBER_PROVIDED ) , 0 );

#ifdef HAS_STATISTICSS
	M_BREAK_STAT( distributor_publish_void( &_g->distributors.init_static_table , SUBSCRIBER_PROVIDED ) , 0 );
#endif

	MM_BREAK_IF( pthread_mutex_init( &_g->appcfg.cfg_mtx , NULL ) , errCreation , 0 , "mutex_init()" );
	_g->appcfg.cfg_mtx_protector = false;

	BEGIN_RET
	default:
	{
		if ( d_error ) DIST_APP_FAILURE();
	}
	M_V_END_RET
}

PRE_MAIN_INITIALIZATION( PRE_MAIN_INIT_CONFIG )
_PRIVATE_FXN void pre_main_init_config_component( void )
{
	INIT_BREAKABLE_FXN();

	// distribute initialization by the callback and throw components
	M_BREAK_STAT( distributor_init( &_g->distributors.bcast_pre_cfg , 1 ) , 0 );
	M_BREAK_STAT( distributor_subscribe( &_g->distributors.bcast_pre_cfg , SUB_VOID , SUB_FXN( state_pre_config_init_config ) , _g ) , 0 );

	M_BREAK_STAT( distributor_init_withOrder( &_g->distributors.bcast_post_cfg , 1 ) , 0 );

	M_BREAK_STAT( distributor_init_withOrder( &_g->distributors.bcast_program_stabled , 1 ) , 0 );
	M_BREAK_STAT( distributor_subscribe_withOrder( &_g->distributors.bcast_program_stabled , SUB_VOID , SUB_FXN( event_program_is_stabled_cfg ) , _g , config_stablity ) , 0 );

	BEGIN_RET
	default:
	{
		if ( d_error ) DIST_APP_FAILURE();
	}
	M_V_END_RET
}

_THREAD_FXN void_p version_checker( pass_p src_g )
{
	INIT_BREAKABLE_FXN();
	G * _g = ( G * )src_g;
	
#ifdef ENABLE_LOG_THREADS
	__attribute__( ( cleanup( thread_goes_out_of_scope ) ) ) pthread_t this_thread = pthread_self();
	distributor_publish_x3long( &_g->distributors.bcast_thread_startup , (long)this_thread , trdn_version_checker , (long)__FUNCTION__ , _g );
	
	/*retrieve track alive indicator*/
	THREAD_LOCK_LINE( pthread_mutex_lock( &_g->stat.nc_s_req.thread_list_mtx ) );
	time_t * pthis_thread_alive_time = NULL;
	for ( size_t idx = 0 ; idx < _g->stat.nc_s_req.thread_list.count ; idx++ )
	{
		thread_alive_indicator * pthread_ind = NULL;
		if ( mms_array_get_s( &_g->stat.nc_s_req.thread_list , idx , ( void ** )&pthread_ind ) == errOK && pthread_ind->thread_id == this_thread )
		{
			pthis_thread_alive_time = &pthread_ind->alive_time;
		}
	}
	pthread_mutex_unlock( &_g->stat.nc_s_req.thread_list_mtx );

#ifdef ENABLE_USE_DBG_TAG
	MARK_START_THREAD();
#endif
#endif

	char buf[ 50 ] = { 0 };
	time_t prev_time = { 0 } , cur_time = { 0 };
	struct Config_ver temp_ver = { 0 };
	while ( 1 )
	{
		cur_time = time( NULL );
		if ( pthis_thread_alive_time ) *pthis_thread_alive_time = cur_time;

		if ( GRACEFULLY_END_THREAD() ) break;

		if ( _g->appcfg.ver == NULL || difftime( cur_time , prev_time ) > 5 )
		{
			prev_time = cur_time;

			MEMSET( buf , 0 , sizeof( buf ) );
			const char * config_ver_file_content = read_file( CONFIG_ROOT_PATH "/config_ver.txt" , ( char * )buf , NULL );
			MM_BREAK_IF( !config_ver_file_content , errNotFound , 0 , "cannot open and read version file" );

			result( json_element ) rs_config_ver = json_parse( config_ver_file_content );
			//free( ( void_p )config_ver_file_content );
			MM_BREAK_IF( catch_error( &rs_config_ver , "config_ver" , 1 ) , errNotFound , 0 , "error in json_parse version file" );
			typed( json_element ) el_config_ver = result_unwrap( json_element )( &rs_config_ver );

			result( json_element ) ver = json_object_find( el_config_ver.value.as_object , "ver" );
			MM_BREAK_IF( catch_error( &ver , "ver" , 1 ) , errNotFound , 0 , "ver not found" );

			MEMSET( &temp_ver , 0 , sizeof( temp_ver ) );

			strcpy( temp_ver.version , ( char * )ver.inner.value.value.as_string );
			char * perr = NULL;
			char * pver = ( char * )ver.inner.value.value.as_string;
			temp_ver.Major = ( int )strtol( strtok( pver , "." ) , &perr , 10 );			MM_BREAK_IF( !perr , errGeneral , 0 , "ver Major wrong" );
			temp_ver.Minor = ( int )strtol( strtok( NULL , "." ) , &perr , 10 );			MM_BREAK_IF( !perr , errGeneral , 0 , "ver Minor wrong" );
			temp_ver.Build = ( int )strtol( strtok( NULL , "." ) , &perr , 10 );			MM_BREAK_IF( !perr , errGeneral , 0 , "ver Build wrong" );
			temp_ver.Revision_Patch = ( int )strtol( strtok( NULL , "." ) , &perr , 10 );	MM_BREAK_IF( !perr , errGeneral , 0 , "ver Revision_Patch wrong" );
			json_free( &el_config_ver ); // string is user so must be free at the end

			if ( _g->appcfg.ver == NULL || STR_DIFF( temp_ver.version , _g->appcfg.ver->version ) )
			{
				_g->appcfg.ver = ( struct Config_ver * )MEMCPY_OR( &_g->appcfg.temp_ver , &temp_ver , sizeof( temp_ver ) );
				_g->appcfg.version_changed = 1;
				//if ( IF_VERBOSE_MODE_CONDITION() )
				//{
				//	_ECHO( "version changed %s" , _g->appcfg._ver->version );
				//}
			}
		}
		mng_basic_thread_sleep( _g , NORMAL_PRIORITY_THREAD );
	}
	BEGIN_RET
		case 1: if ( d_error ) DIST_APP_FAILURE();
	M_V_END_RET

	return VOID_RET;
}

_THREAD_FXN void_p config_loader( pass_p src_g )
{
	INIT_BREAKABLE_FXN();
	G * _g = ( G * )src_g;
	char tmp_arr[ DEFAULT_MFS_BUF_SZ ] = "";
	
#ifdef ENABLE_LOG_THREADS
	__attribute__( ( cleanup( thread_goes_out_of_scope ) ) ) pthread_t this_thread = pthread_self();
	distributor_publish_x3long( &_g->distributors.bcast_thread_startup , (long)this_thread , trdn_config_loader , (long)__FUNCTION__ , _g );
	
	/*retrieve track alive indicator*/
	THREAD_LOCK_LINE( pthread_mutex_lock( &_g->stat.nc_s_req.thread_list_mtx ) );
	time_t * pthis_thread_alive_time = NULL;
	for ( size_t idx = 0 ; idx < _g->stat.nc_s_req.thread_list.count ; idx++ )
	{
		thread_alive_indicator * pthread_ind = NULL;
		if ( mms_array_get_s( &_g->stat.nc_s_req.thread_list , idx , ( void ** )&pthread_ind ) == errOK && pthread_ind->thread_id == this_thread )
		{
			pthis_thread_alive_time = &pthread_ind->alive_time;
		}
	}
	pthread_mutex_unlock( &_g->stat.nc_s_req.thread_list_mtx );
#ifdef ENABLE_USE_DBG_TAG
	MARK_START_THREAD();
#endif
#endif
	
	while ( !_g->appcfg.version_changed ) // load after version loaded
	{
		if ( pthis_thread_alive_time ) *pthis_thread_alive_time = time( NULL );
		if ( GRACEFULLY_END_THREAD() ) break;
		mng_basic_thread_sleep( _g , HI_PRIORITY_THREAD );
	}
	
	typed( json_element ) el_Protocol_Bridge_config;
	
	while ( 1 )
	{
		if ( pthis_thread_alive_time ) *pthis_thread_alive_time = time( NULL );
		if ( GRACEFULLY_END_THREAD() ) break;
		if ( _g->appcfg.g_cfg == NULL || _g->appcfg.version_changed )
		{
			CFG_LOCK_LINE( pthread_mutex_lock( &_g->appcfg.cfg_mtx ) );
			_g->appcfg.cfg_mtx_protector = true;

			Gcfg temp_config = { 0 };
			Gcfg0 * pGeneralConfiguration = ( Gcfg0 * )&temp_config;
			brg_cfg_t * pProtocol_Bridges = NULL;
			size_t Protocol_Bridges_count = 0;
			{
				const char * Protocol_Bridge_config_file_content = read_file( CONFIG_ROOT_PATH "/Protocol_Bridge_config.txt" , NULL , NULL );
				MM_BREAK_IF( !Protocol_Bridge_config_file_content , errNotFound , 0 , _MK_MSG( tmp_arr , "cannot open %s" , CONFIG_ROOT_PATH "/Protocol_Bridge_config.txt" ) );
					
				result( json_element ) rs_Protocol_Bridge_config = json_parse( Protocol_Bridge_config_file_content );
				FREE( ( void_p )Protocol_Bridge_config_file_content );
				MM_BREAK_IF( catch_error( &rs_Protocol_Bridge_config , "Protocol_Bridge_config" , 1 ) , errGeneral , 0 , _MK_MSG( tmp_arr , "cannot parse %s" , CONFIG_ROOT_PATH "/Protocol_Bridge_config.txt" ) );
				el_Protocol_Bridge_config = result_unwrap( json_element )( &rs_Protocol_Bridge_config );
	
				// load general configurations
				if ( _g->appcfg.ver->Major >= 1 ) // first version of config file structure
				{
					result( json_element ) re_configurations = json_object_find( el_Protocol_Bridge_config.value.as_object , "CONFIGURATIONS" );
					MM_BREAK_IF( catch_error( &re_configurations , "CONFIGURATIONS" , 1 ) , errNotFound , 0 , "CONFIGURATIONS tag not found" );
					typed( json_element ) el_configurations = result_unwrap( json_element )( &re_configurations );
	
					#define CFG_ELEM_STR( name )																			/**/\
						result( json_element ) re_##name = json_object_find( el_configurations.value.as_object , #name );	/**/\
						MM_BREAK_IF( catch_error( &re_##name , #name , 1 ) , errNotFound , 0 , "@configurations " #name " not found" );								/**/\
						typed( json_element ) el_##name = result_unwrap( json_element )( &re_##name );						/**/\
						NEWSTR( pGeneralConfiguration->name , el_##name.value.as_string , 0 );								/**/
					#define CFG_ELEM_I( name )																				/**/\
						result( json_element ) re_##name = json_object_find( el_configurations.value.as_object , #name );	/**/\
						MM_BREAK_IF( catch_error( &re_##name , #name , 1 ) , errNotFound , 0 , "@configurations " #name " not found" );								/**/\
						typed( json_element ) el_##name = result_unwrap( json_element )( &re_##name );						/**/\
						pGeneralConfiguration->name = (int)el_##name.value.as_number.value.as_long;							/**/
					#define CFG_ELEM_I64( name )																			/**/\
						result( json_element ) re_##name = json_object_find( el_configurations.value.as_object , #name );	/**/\
						MM_BREAK_IF( catch_error( &re_##name , #name , 1 ) , errNotFound , 0 , "@configurations " #name " not found" );								/**/\
						typed( json_element ) el_##name = result_unwrap( json_element )( &re_##name );						/**/\
						pGeneralConfiguration->name = (int64)el_##name.value.as_number.value.as_long;						/**/
					#define CFG_ELEM_F( name )																				/**/\
						result( json_element ) re_##name = json_object_find( el_configurations.value.as_object , #name );	/**/\
						MM_BREAK_IF( catch_error( &re_##name , #name , 1 ) , errNotFound , 0 , "@configurations " #name " not found" );								/**/\
						typed( json_element ) el_##name = result_unwrap( json_element )( &re_##name );						/**/\
						pGeneralConfiguration->name = (float)el_##name.value.as_number.value.as_double;						/**/

					CFG_ELEM_STR( create_date );																			/**/\
					CFG_ELEM_STR( modify_date );																			/**/\
					CFG_ELEM_STR( config_name );																			/**/\
					CFG_ELEM_STR( config_tags );																			/**/\
					CFG_ELEM_STR( description );																			/**/\
					CFG_ELEM_STR( log_level );																				/**/\
					CFG_ELEM_STR( log_file );																				/**/\
					
					CFG_ELEM_I( load_prev_config );																			/**/\
					CFG_ELEM_I( dump_current_config );																		/**/\
					CFG_ELEM_I( dump_prev_config );																			/**/\
					CFG_ELEM_I( time_out_sec );																				/**/\
					CFG_ELEM_I( verbose_mode );																				/**/\
					CFG_ELEM_I( log_cooldown_sec );																			/**/\
					CFG_ELEM_I( refresh_interval_sec );																		/**/\
					CFG_ELEM_I( number_in_short_form );																		/**/\
					CFG_ELEM_I( precision_of_double_in_short_form );														/**/\
					CFG_ELEM_I64( low_priority_thread_cooldown_delay_nanosec );												/**/\
					CFG_ELEM_I64( normal_priority_thread_cooldown_delay_nanosec );											/**/\
					CFG_ELEM_I64( hi_priority_thread_cooldown_delay_nanosec );												/**/\
					CFG_ELEM_I64( very_hi_priority_thread_cooldown_delay_nanosec );												/**/\
					CFG_ELEM_I64( harbor_mem_segment_capacity );															/**/\
					CFG_ELEM_I64( harbor_mem_segment_offsets_cnt_base );													/**/\
					CFG_ELEM_I( idle_active_harbor_mem_segment_timeout_sec );												/**/\
					CFG_ELEM_I64( harbor_mem_max_allowed_allocation );														/**/\
					CFG_ELEM_F( instantaneous_input_load_coefficient );														/**/\
					CFG_ELEM_I( TTF_no_backpressure_threshold_sec );														/**/\
					CFG_ELEM_I( TTF_gentle_backpressure_threshold_sec );													/**/\
					CFG_ELEM_I( TTF_aggressive_backpressure_threshold_sec );												/**/\
					CFG_ELEM_I( TTF_emergency_drop_backpressure_threshold_sec );											/**/\
					CFG_ELEM_I( TTF_skip_input_threshold_sec );																/**/\
					CFG_ELEM_I( TTF_gentle_backpressure_stride );															/**/\
					CFG_ELEM_I( TTF_aggressive_backpressure_stride );														/**/\
					CFG_ELEM_I( TTF_emergency_drop_backpressure_stride );													/**/\
					CFG_ELEM_I( TTF_red_zone_stride );																		/**/\
					CFG_ELEM_I( wait_at_cleanup_until_unsaved_packet_stored_sec );											/**/\
					CFG_ELEM_I( raw_udp_cache_sz_byte );																	/**/\
					CFG_ELEM_I( network_handshake_pessimistic_timeout_sec );												/**/\
					CFG_ELEM_I( tcp_connection_idle_timeout_sec );															/**/\
					CFG_ELEM_I( harbor_mem_flood_detection_sample_count );													/**/\
					CFG_ELEM_I( long_term_throughput_smoothing_samples );													/**/\
					CFG_ELEM_I( in_memory_udp_hold_time_sec );																/**/\
					CFG_ELEM_I( unused_memory_block_hold_time_sec );														/**/\
					CFG_ELEM_I( instant_load_influence_window_time_sec );													/**/\
					CFG_ELEM_I( udp_id_keeping_timeout_msec );																/**/\
					CFG_ELEM_I( each_udp_part_reassembly_timeout_msec );													/**/\
					CFG_ELEM_I( infinite_loop_guard );																		/**/\
					CFG_ELEM_I( harbor_mem_segment_check_idle_active_each_n_sec );											/**/\
					CFG_ELEM_I( max_saved_file_size_threshold_MB );															/**/
					

					#undef CFG_ELEM_I
					#undef CFG_ELEM_STR
					#undef CFG_ELEM_F
					#undef CFG_ELEM_I64
				}
	
				// load Protocol_Bridges
				{
					result( json_element ) re_Protocol_Bridges = json_object_find( el_Protocol_Bridge_config.value.as_object , "PROTOCOL_BRIDGES" );
					MM_BREAK_IF( catch_error( &re_Protocol_Bridges , "PROTOCOL_BRIDGES" , 1 ) , errNotFound , 0 , "tag PROTOCOL_BRIDGES not found" );
					typed( json_element ) el_Protocol_Bridges = result_unwrap( json_element )( &re_Protocol_Bridges );
	
					//MM_BREAK_IF( ( Protocol_Bridges_count = el_Protocol_Bridges.value.as_object->count ) < 1 , errGeneral , 0 , "Protocol_Bridges must be not zero" );
	
					M_BREAK_IF( !( pProtocol_Bridges = MALLOC_AR( pProtocol_Bridges , el_Protocol_Bridges.value.as_object->count ) ) , errMemoryLow , 0 );
					MEMSET_ZERO( pProtocol_Bridges , el_Protocol_Bridges.value.as_object->count );
					
					size_t iactual_section = 0;
					for ( int icnf_section = 0 ; icnf_section < el_Protocol_Bridges.value.as_object->count ; icnf_section++ ) // each bridge
					{
						((Bcfg0 *)(pProtocol_Bridges + iactual_section))->temp_data._pseudo_g = ( void_p )_g; // each config must know global settings
						const char * output_Protocol_Bridge_name = ( *( el_Protocol_Bridges.value.as_object->entries + icnf_section ) )->key;
	
						result( json_element ) re_each_bridge = json_object_find( el_Protocol_Bridges.value.as_object , output_Protocol_Bridge_name );
						MM_BREAK_IF( catch_error( &re_each_bridge , output_Protocol_Bridge_name , 1 ) , errNotFound , 0 , _MK_MSG( tmp_arr , "Protocol_Bridges>%s not found" , output_Protocol_Bridge_name ) );
						typed( json_element ) el_each_bridge = result_unwrap( json_element )( &re_each_bridge );
	
						#define CFG_ELEM_STR( part , name )																					/**/\
						result( json_element ) re_##name = json_object_find( el_each_bridge.value.as_object , #name );						/**/\
						MM_BREAK_IF( catch_error( &re_##name , #name , 1 ) , errNotFound , 0 , _MK_MSG( tmp_arr , "%s> " #name " not found" , output_Protocol_Bridge_name ) );	/**/\
						typed( json_element ) el_##name = result_unwrap( json_element )( &re_##name );										/**/\
						strcpy(((Bcfg0 *)(pProtocol_Bridges + iactual_section))->part.name , el_##name.value.as_string );					/**/
	
						#define CFG_ELEM_I( part , name )																					/**/\
						result( json_element ) re_##name = json_object_find( el_each_bridge.value.as_object , #name );						/**/\
						MM_BREAK_IF( catch_error( &re_##name , #name , 1 ) , errNotFound , 0 , _MK_MSG( tmp_arr , "%s> " #name " not found" , output_Protocol_Bridge_name ) );	/**/\
						typed( json_element ) el_##name = result_unwrap( json_element )( &re_##name );										/**/\
						((Bcfg0 *)(pProtocol_Bridges + iactual_section))->part.name = (int)el_##name.value.as_number.value.as_long;			/**/
	
						CFG_ELEM_STR( id , short_name );
						CFG_ELEM_STR( id , out_type );
						CFG_ELEM_STR( id , thread_handler_act );
						CFG_ELEM_I( maintained , enable );
						CFG_ELEM_I( maintained , hide );

						if ( (((Bcfg0 *)(pProtocol_Bridges + iactual_section))->maintained.hide) )
						{ continue; }
						
						// bridge inputs
						result( json_element ) re_inputs = json_object_find( el_each_bridge.value.as_object , "INPUTS" );
						MM_BREAK_IF( catch_error( &re_inputs , "INPUTS" , 1 ) , errNotFound , 0 , _MK_MSG( tmp_arr , "Protocol_Bridges>%s>INPUTS not found" , output_Protocol_Bridge_name ) );
						typed( json_element ) el_inputs = result_unwrap( json_element )( &re_inputs );
												
						((Bcfg0 *)(pProtocol_Bridges + iactual_section))->maintained.in_count = el_inputs.value.as_object->count;
						
						M_BREAK_IF( !( ((Bcfg0 *)(pProtocol_Bridges + iactual_section))->maintained.in = MALLOC_AR( ((Bcfg0 *)(pProtocol_Bridges + iactual_section))->maintained.in , ((Bcfg0 *)(pProtocol_Bridges + iactual_section))->maintained.in_count ) ) , errMemoryLow , 0 );
						MEMSET_ZERO( ((Bcfg0 *)(pProtocol_Bridges + iactual_section))->maintained.in , ((Bcfg0 *)(pProtocol_Bridges + iactual_section))->maintained.in_count );
						for ( size_t iin = 0 ; iin < ((Bcfg0 *)(pProtocol_Bridges + iactual_section))->maintained.in_count ; iin++ )
						{
							const char * output_input_name = ( *( el_inputs.value.as_object->entries + iin ) )->key;
							strcpy( ( ((Bcfg0 *)(pProtocol_Bridges + iactual_section))->maintained.in + iin )->name , output_input_name );

							result( json_element ) re_inp = json_object_find( el_inputs.value.as_object , output_input_name );
							MM_BREAK_IF( catch_error( &re_inp , output_input_name , 1 ) , errNotFound , 0 , _MK_MSG( tmp_arr , "Protocol_Bridges>%s>inputs>%s not found" , output_Protocol_Bridge_name , output_input_name ) );
							typed( json_element ) el_inp = result_unwrap( json_element )( &re_inp );

							#define IN_CFG_ELEM_STR( elem , namee )																		/**/\
							result( json_element ) re_##namee = json_object_find( elem.value.as_object , #namee );						/**/\
							MM_BREAK_IF( catch_error( &re_##namee , #namee , 1 ) , errNotFound , 0 , _MK_MSG( tmp_arr , "Protocol_Bridges>%s>inputs>%s>" #namee " not found" , output_Protocol_Bridge_name , output_input_name ) );	 /**/\
							typed( json_element ) el_##namee = result_unwrap( json_element )( &re_##namee );								/**/\
							strcpy( ( ((Bcfg0 *)(pProtocol_Bridges + iactual_section))->maintained.in + iin )->data.namee , el_##namee.value.as_string );					/**/

							#define IN_CORE_CFG_ELEM_STR( elem , namee )																		/**/\
							result( json_element ) re_##namee = json_object_find( elem.value.as_object , #namee );						/**/\
							MM_BREAK_IF( catch_error( &re_##namee , #namee , 1 ) , errNotFound , 0 , _MK_MSG( tmp_arr , "Protocol_Bridges>%s>inputs>%s>" #namee " not found" , output_Protocol_Bridge_name , output_input_name ) );											/**/\
							typed( json_element ) el_##namee = result_unwrap( json_element )( &re_##namee );								/**/\
							strcpy( ( ((Bcfg0 *)(pProtocol_Bridges + iactual_section))->maintained.in + iin )->data.core.namee , el_##namee.value.as_string );					/**/

							#define IN_CFG_ELEM_I( elem , namee )																				/**/\
							result( json_element ) re_##namee = json_object_find( elem.value.as_object , #namee );								/**/\
							MM_BREAK_IF( catch_error( &re_##namee , #namee , 1 ) , errNotFound , 0 , _MK_MSG( tmp_arr , "Protocol_Bridges>%s>inputs>%s>" #namee " not found" , output_Protocol_Bridge_name , output_input_name ) );													/**/\
							typed( json_element ) el_##namee = result_unwrap( json_element )( &re_##namee );										/**/\
							( ((Bcfg0 *)(pProtocol_Bridges + iactual_section))->maintained.in + iin )->data.namee = (int)el_##namee.value.as_number.value.as_long;					/**/

							IN_CFG_ELEM_STR( el_inp , group );
							IN_CFG_ELEM_STR( el_inp , group_type );
							IN_CORE_CFG_ELEM_STR( el_inp , UDP_origin_ip );
							IN_CORE_CFG_ELEM_STR( el_inp , UDP_destination_ip );
							
							IN_CORE_CFG_ELEM_STR( el_inp , UDP_origin_interface );

							IN_CORE_CFG_ELEM_STR( el_inp , UDP_origin_ports );
							IN_CFG_ELEM_I( el_inp , enable );
							IN_CFG_ELEM_I( el_inp , reset_connection );

							#undef IN_CFG_ELEM_STR
							#undef IN_CFG_ELEM_I
						}
						
						// bridge outputs
						result( json_element ) re_outputs = json_object_find( el_each_bridge.value.as_object , "OUTPUTS" );
						if ( !catch_error( &re_outputs , "OUTPUTS" , 0 ) )
						{
							typed( json_element ) el_outputs = result_unwrap( json_element )( &re_outputs );
							( ( Bcfg0 * )( pProtocol_Bridges + iactual_section ) )->maintained.out_count = el_outputs.value.as_object->count;

							M_BREAK_IF( !( ( ( Bcfg0 * )( pProtocol_Bridges + iactual_section ) )->maintained.out = MALLOC_AR( ( ( Bcfg0 * )( pProtocol_Bridges + iactual_section ) )->maintained.out , ( ( Bcfg0 * )( pProtocol_Bridges + iactual_section ) )->maintained.out_count ) ) , errMemoryLow , 0 );
							MEMSET_ZERO( ( ( Bcfg0 * )( pProtocol_Bridges + iactual_section ) )->maintained.out , ( ( Bcfg0 * )( pProtocol_Bridges + iactual_section ) )->maintained.out_count );
							for ( int iout = 0 ; iout < ( ( Bcfg0 * )( pProtocol_Bridges + iactual_section ) )->maintained.out_count ; iout++ )
							{
								const char * output_outputs_name = ( *( el_outputs.value.as_object->entries + iout ) )->key;
								strcpy( ( ((Bcfg0 *)(pProtocol_Bridges + iactual_section ))->maintained.out + iout )->name , output_outputs_name );

								result( json_element ) re_out = json_object_find( el_outputs.value.as_object , output_outputs_name );
								MM_BREAK_IF( catch_error( &re_out , output_outputs_name , 1 ) , errNotFound , 0 , _MK_MSG( tmp_arr , "Protocol_Bridges>%s>outputs>%s not found" , output_Protocol_Bridge_name , output_outputs_name ) );
								typed( json_element ) el_out = result_unwrap( json_element )( &re_out );

								#define IN_CFG_ELEM_STR( elem , namee )																		/**/\
								result( json_element ) re_##namee = json_object_find( elem.value.as_object , #namee );						/**/\
								MM_BREAK_IF( catch_error( &re_##namee , #namee , 1 ) , errNotFound , 0 , _MK_MSG( tmp_arr , "Protocol_Bridges>%s>outputs>%s>" #namee " not found" , output_Protocol_Bridge_name , output_outputs_name ) );	/**/\
								typed( json_element ) el_##namee = result_unwrap( json_element )( &re_##namee );								/**/\
								strcpy( ( ((Bcfg0 *)(pProtocol_Bridges + iactual_section ))->maintained.out + iout )->data.namee , el_##namee.value.as_string );		/**/

								#define IN_CFG_CORE_ELEM_STR( elem , namee )																		/**/\
								result( json_element ) re_##namee = json_object_find( elem.value.as_object , #namee );						/**/\
								MM_BREAK_IF( catch_error( &re_##namee , #namee , 1 ) , errNotFound , 0 , _MK_MSG( tmp_arr , "Protocol_Bridges>%s>outputs>%s>" #namee " not found" , output_Protocol_Bridge_name , output_outputs_name ) );	/**/\
								typed( json_element ) el_##namee = result_unwrap( json_element )( &re_##namee );								/**/\
								strcpy( ( ((Bcfg0 *)(pProtocol_Bridges + iactual_section ))->maintained.out + iout )->data.core.namee , el_##namee.value.as_string );		/**/

								#define IN_CFG_ELEM_I( elem , namee )																				/**/\
								result( json_element ) re_##namee = json_object_find( elem.value.as_object , #namee );								/**/\
								MM_BREAK_IF( catch_error( &re_##namee , #namee , 1 ) , errNotFound , 0 , _MK_MSG( tmp_arr , "Protocol_Bridges>%s>outputs>%s>" #namee " not found" , output_Protocol_Bridge_name , output_outputs_name ) );	/**/\
								typed( json_element ) el_##namee = result_unwrap( json_element )( &re_##namee );										/**/\
								( ((Bcfg0 *)(pProtocol_Bridges + iactual_section ))->maintained.out + iout )->data.namee = (int)el_##namee.value.as_number.value.as_long;	/**/

								IN_CFG_ELEM_STR( el_out , group );
								IN_CFG_ELEM_STR( el_out , group_type );
								IN_CFG_CORE_ELEM_STR( el_out , TCP_destination_ip );
								IN_CFG_CORE_ELEM_STR( el_out , TCP_destination_interface );

								IN_CFG_CORE_ELEM_STR( el_out , TCP_destination_ports );
								IN_CFG_ELEM_I( el_out , enable );
								IN_CFG_ELEM_I( el_out , reset_connection );

								IN_CFG_ELEM_STR( el_out , post_action );
								IN_CFG_ELEM_I( el_out , send_througput_limit_Bps );
								IN_CFG_ELEM_I( el_out , send_gap_nsec );


								#undef IN_CFG_ELEM_STR
								#undef IN_CFG_ELEM_I
							}
						}
	
						#undef CFG_ELEM_I
						#undef CFG_ELEM_STR

						iactual_section++;
					}
					Protocol_Bridges_count = iactual_section;
					WARNING( Protocol_Bridges_count );
	
					json_free( &el_Protocol_Bridges );
				}
			}
	
			int initial_general_config = 0;
			// 
			if ( _g->appcfg.g_cfg == NULL )
			{
				M_BREAK_IF( !( _g->appcfg.g_cfg = MALLOC( sizeof( struct Global_Config ) ) ) , errMemoryLow , 0 );
	
				MEMSET_ZERO( _g->appcfg.g_cfg , 1 );
				MEMCPY_OR( _g->appcfg.g_cfg , &temp_config , sizeof( temp_config ) );
				MEMSET( &temp_config , 0 , sizeof( temp_config ) ); // copy to global variable and then zero to not free strings
				initial_general_config = 1;
				_g->appcfg.g_cfg_changed = 1;
			}
			if ( !initial_general_config )
			{
				// compare general config
				DAC( _g->appcfg.prev_cfg );
	
				_g->appcfg.prev_cfg = _g->appcfg.g_cfg;
				_g->appcfg.g_cfg = NULL;
	
				M_BREAK_IF( !( _g->appcfg.g_cfg = MALLOC( sizeof( struct Global_Config ) ) ) , errMemoryLow , 0 );
				MEMSET( _g->appcfg.g_cfg , 0 , sizeof( struct Global_Config ) );
				MEMCPY_OR( _g->appcfg.g_cfg , &temp_config , sizeof( temp_config ) );
				MEMSET( &temp_config , 0 , sizeof( temp_config ) ); // copy to global variable and then zero to not free strings
	
				if ( _g->appcfg.prev_cfg != NULL && _g->appcfg.g_cfg != NULL )
				{
					_g->appcfg.g_cfg_changed |= !STR_SAME( CFG().create_date , _g->appcfg.prev_cfg->c.c.create_date );
					_g->appcfg.g_cfg_changed |= !STR_SAME( CFG().modify_date , _g->appcfg.prev_cfg->c.c.modify_date );
					_g->appcfg.g_cfg_changed |= !STR_SAME( CFG().config_name , _g->appcfg.prev_cfg->c.c.config_name );
					_g->appcfg.g_cfg_changed |= !STR_SAME( CFG().config_tags , _g->appcfg.prev_cfg->c.c.config_tags );
					_g->appcfg.g_cfg_changed |= !STR_SAME( CFG().description , _g->appcfg.prev_cfg->c.c.description );
					_g->appcfg.g_cfg_changed |= !STR_SAME( CFG().log_level , _g->appcfg.prev_cfg->c.c.log_level );
					_g->appcfg.g_cfg_changed |= !STR_SAME( CFG().log_file , _g->appcfg.prev_cfg->c.c.log_file );
					
					_g->appcfg.g_cfg_changed |= !!( memcmp( &CFG().cfg_change_pck , &_g->appcfg.prev_cfg->c.c.cfg_change_pck , sizeof( _g->appcfg.prev_cfg->c.c.cfg_change_pck ) ) );
				}
			}
	
			if ( _g->appcfg.g_cfg_changed )
			{
				//if ( IF_VERBOSE_MODE_CONDITION() )
				//{
				//	_ECHO( "general config changed" );
				//}
			}
	
			if ( _g->appcfg.bdj_psv_cfg )
			{
				DAC( _g->appcfg.old_bdj_psv_cfg );
				_g->appcfg.old_bdj_psv_cfg_count = 0;
				_g->appcfg.old_bdj_psv_cfg = _g->appcfg.bdj_psv_cfg;
				_g->appcfg.old_bdj_psv_cfg_count = _g->appcfg.bdj_psv_cfg_count;
			}
			_g->appcfg.bdj_psv_cfg = pProtocol_Bridges;
			_g->appcfg.bdj_psv_cfg_count = Protocol_Bridges_count;
			pProtocol_Bridges = NULL; // to not delete intentionally
			Protocol_Bridges_count = 0;
	
			// TEST CASE. some slightly change in config cause equality control tested
			if ( _g->appcfg.old_bdj_psv_cfg == NULL && _g->appcfg.bdj_psv_cfg ) // prev null & curr !null
			{
				// all new ones
				_g->appcfg.psv_cfg_changed = 1; // ham koli set mishavad change rokh dad
				for ( int icfg = 0 ; icfg < _g->appcfg.bdj_psv_cfg_count ; icfg++ )
				{
					_g->appcfg.bdj_psv_cfg[ icfg ].m.m.temp_data.pcfg_changed = 1; // ham joz e set mishavad
				}
			}
			else if ( _g->appcfg.old_bdj_psv_cfg && _g->appcfg.bdj_psv_cfg ) // prev !null & curr !null
			{
				// from old perspective
				for ( int ioldcfg = 0 ; ioldcfg < _g->appcfg.old_bdj_psv_cfg_count ; ioldcfg++ )
				{
					int prev_exist = 0;
					for ( int icfg = 0 ; icfg < _g->appcfg.bdj_psv_cfg_count ; icfg++ )
					{
						if ( Bcfg_id_equlity( &_g->appcfg.old_bdj_psv_cfg[ ioldcfg ] , &_g->appcfg.bdj_psv_cfg[ icfg ] ) )
						{
							prev_exist = 1;
							if ( !bridge_cfg_data_equlity( &_g->appcfg.old_bdj_psv_cfg[ ioldcfg ] , &_g->appcfg.bdj_psv_cfg[ icfg ] ) )
							{
								_g->appcfg.psv_cfg_changed = 1;
								_g->appcfg.bdj_psv_cfg[ icfg ].m.m.temp_data.pcfg_changed = 1;
							}
							break;
						}
					}
					if ( !prev_exist )
					{
						_g->appcfg.psv_cfg_changed = 1;
						_g->appcfg.old_bdj_psv_cfg[ ioldcfg ].m.m.temp_data.pcfg_changed = 1;
						break;
					}
				}
				// from new perspective
				for ( int icfg = 0 ; icfg < _g->appcfg.bdj_psv_cfg_count ; icfg++ )
				{
					int new_exist = 0;
					for ( int ioldcfg = 0 ; ioldcfg < _g->appcfg.old_bdj_psv_cfg_count ; ioldcfg++ )
					{
						if ( Bcfg_id_equlity( &_g->appcfg.old_bdj_psv_cfg[ ioldcfg ] , &_g->appcfg.bdj_psv_cfg[ icfg ] ) )
						{
							new_exist = 1;
							if ( !bridge_cfg_data_equlity( &_g->appcfg.old_bdj_psv_cfg[ ioldcfg ] , &_g->appcfg.bdj_psv_cfg[ icfg ] ) )
							{
								_g->appcfg.psv_cfg_changed = 1;
								_g->appcfg.bdj_psv_cfg[ icfg ].m.m.temp_data.pcfg_changed = 1;
							}
							break;
						}
					}
					if ( !new_exist )
					{
						_g->appcfg.psv_cfg_changed = 1;
						_g->appcfg.bdj_psv_cfg[ icfg ].m.m.temp_data.pcfg_changed = 1;
						break;
					}
				}
			}
	
			WARNING( _g->appcfg.bdj_psv_cfg_count );

			_g->appcfg.version_changed = 0;
	
			_g->appcfg.cfg_mtx_protector = false;
			pthread_mutex_unlock( &_g->appcfg.cfg_mtx );

			//if ( _g->appcfg._psv_cfg_changed && IF_VERBOSE_MODE_CONDITION() )
			//{
			//	_ECHO( "protocol_bridge config changed" );
			//}
		}
		mng_basic_thread_sleep( _g , LOW_PRIORITY_THREAD );
	}
	
	BEGIN_RET
	default:
	{
		if ( d_error )
		{
			DIST_APP_FAILURE();
			fprintf( stderr , EACH_FXN_MSG_HOLDER );
		}
	}
	M_V_END_RET

	if ( _g->appcfg.cfg_mtx_protector )
	{
		pthread_mutex_unlock( &_g->appcfg.cfg_mtx );
	}

	return NULL;
}

// aware of concurrency in config read and act on it
_THREAD_FXN void_p config_executer( pass_p src_g )
{
	INIT_BREAKABLE_FXN();
	G * _g = ( G * )src_g;
	
#ifdef ENABLE_LOG_THREADS
	__attribute__( ( cleanup( thread_goes_out_of_scope ) ) ) pthread_t this_thread = pthread_self();
	distributor_publish_x3long( &_g->distributors.bcast_thread_startup , (long)this_thread , trdn_config_executer , (long)__FUNCTION__ , _g );
	
	/*retrieve track alive indicator*/
	THREAD_LOCK_LINE( pthread_mutex_lock( &_g->stat.nc_s_req.thread_list_mtx ) );
	time_t * pthis_thread_alive_time = NULL;
	for ( size_t idx = 0 ; idx < _g->stat.nc_s_req.thread_list.count ; idx++ )
	{
		thread_alive_indicator * pthread_ind = NULL;
		if ( mms_array_get_s( &_g->stat.nc_s_req.thread_list , idx , ( void ** )&pthread_ind ) == errOK && pthread_ind->thread_id == this_thread )
		{
			pthis_thread_alive_time = &pthread_ind->alive_time;
		}
	}
	pthread_mutex_unlock( &_g->stat.nc_s_req.thread_list_mtx );
#ifdef ENABLE_USE_DBG_TAG
	MARK_START_THREAD();
#endif
#endif

	while ( !_g->appcfg.bdj_psv_cfg_count ) // load after any config loaded
	{
		if ( pthis_thread_alive_time ) *pthis_thread_alive_time = time( NULL );
		if ( GRACEFULLY_END_THREAD() ) break;
		mng_basic_thread_sleep( _g , HI_PRIORITY_THREAD );
	}

	while ( 1 )
	{
		if ( pthis_thread_alive_time ) *pthis_thread_alive_time = time( NULL );
		if ( GRACEFULLY_END_THREAD() )
		{
			//for ( int icfg = 0 ; icfg < _g->appcfg.bdj_psv_cfg_count ; icfg++ )
			//{
			//	_g->appcfg.bdj_psv_cfg[ icfg ].m.m.maintained.enable = 0;
			//	apply_protocol_bridge_new_cfg_changes( _g , &_g->appcfg.bdj_psv_cfg[ icfg ] , &_g->appcfg.bdj_psv_cfg[ icfg ] );
			//}
			break;
		}
		if ( _g->appcfg.g_cfg_changed )
		{
			_g->appcfg.g_cfg_changed = 0;
		}

		if ( _g->appcfg.psv_cfg_changed )
		{
			CFG_LOCK_LINE( pthread_mutex_lock( &_g->appcfg.cfg_mtx ) );

			for ( int ioldcfg = 0 ; ioldcfg < _g->appcfg.old_bdj_psv_cfg_count ; ioldcfg++ )
			{
				int exist = 0;
				for ( int icfg = 0 ; icfg < _g->appcfg.bdj_psv_cfg_count ; icfg++ )
				{
					if ( exist ) break;
					if ( Bcfg_id_equlity( &_g->appcfg.old_bdj_psv_cfg[ ioldcfg ] , &_g->appcfg.bdj_psv_cfg[ icfg ] ) && _g->appcfg.bdj_psv_cfg[ icfg ].m.m.temp_data.pcfg_changed )
					{
						// existed cfg changed
						apply_protocol_bridge_new_cfg_changes( _g , &_g->appcfg.old_bdj_psv_cfg[ ioldcfg ] , &_g->appcfg.bdj_psv_cfg[ icfg ] );
						exist = 1;
					}
				}
				if ( !exist )
				{
					// remove removed one
					remove_protocol_bridge( _g , &_g->appcfg.old_bdj_psv_cfg[ ioldcfg ] );
				}
			}
			for ( int icfg = 0 ; icfg < _g->appcfg.bdj_psv_cfg_count ; icfg++ )
			{
				int exist = 0;
				for ( int ioldcfg = 0 ; ioldcfg < _g->appcfg.old_bdj_psv_cfg_count ; ioldcfg++ )
				{
					if ( Bcfg_id_equlity( &_g->appcfg.bdj_psv_cfg[ icfg ] , &_g->appcfg.old_bdj_psv_cfg[ ioldcfg ] ) )
					{
						exist = 1;
						break;
					}
				}
				if ( !exist )
				{
					// start new cfg
					add_new_protocol_bridge( _g , &_g->appcfg.bdj_psv_cfg[ icfg ] );

				}
			}
			_g->appcfg.psv_cfg_changed = 0; // changes applied

			pthread_mutex_unlock( &_g->appcfg.cfg_mtx );

			_g->appcfg.already_main_cfg_stablished = true;

			//break; // TEMP
		}
		mng_basic_thread_sleep( _g , LOW_PRIORITY_THREAD );
	}
	return NULL;
}

void stop_protocol_bridge( G * _g , AB * pb )
{
	pb->cpy_cfg.m.m.maintained.enable = 0;
	pb->cpy_cfg.m.m.temp_data.pcfg_changed = 1;
	apply_new_protocol_bridge_config( _g , pb , &pb->cpy_cfg );
}

void apply_protocol_bridge_new_cfg_changes( G * _g , brg_cfg_t * prev_pcfg , brg_cfg_t * new_ccfg )
{
	for ( size_t iab = 0 ; iab < _g->bridges.ABs.count ; iab++ )
	{
		AB * pb = NULL;
		if ( mms_array_get_s( &_g->bridges.ABs , iab , (void**)&pb ) == errOK )
		{
			if ( Bcfg_id_equlity( &pb->cpy_cfg , prev_pcfg ) )
			{
				apply_new_protocol_bridge_config( _g , pb , new_ccfg );
			}
		}
	}
}

void remove_protocol_bridge( G * _g , brg_cfg_t * pcfg )
{
	for ( size_t iab = _g->bridges.ABs.count - 1 ; _g->bridges.ABs.count && iab >= 0 ; iab-- )
	{
		AB * pb = NULL;
		if ( mms_array_get_s( &_g->bridges.ABs , iab , ( void ** )&pb ) == errOK )
		{
			if ( Bcfg_id_equlity( &pb->cpy_cfg , pcfg ) )
			{
				stop_protocol_bridge( _g , pb );
				mms_array_delete( &_g->bridges.ABs , iab );
			}
		}
		if ( iab < 1 ) break;
	}
}

void add_new_protocol_bridge( G * _g , brg_cfg_t * new_ccfg )
{
	INIT_BREAKABLE_FXN();

	AB * pb = NULL;
	M_BREAK_STAT( mms_array_get_one_available_unoccopied_item( &_g->bridges.ABs , (void**)&pb ) , 0 );
	copy_bridge_cfg( &pb->cpy_cfg , new_ccfg );
	apply_protocol_bridge_new_cfg_changes( _g , new_ccfg , new_ccfg );

	BEGIN_RET
	default:
	{
		if ( d_error ) DIST_APP_FAILURE();
	}
	M_V_END_RET
}
