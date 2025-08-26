#define Uses_TWD
#define Uses_pthread_t
#define Uses_json
#define Uses_config
#define Uses_helper
#define Uses_memset

//#define DIRECT_ECHO_BUF _g->stat.last_command // just before include dep
#include <Protocol_Bridge.dep>

// TODO . exit gracefully by auto mechanism
// TODO . think about race condition
_THREAD_FXN void_p version_checker( void_p src_g )
{
	INIT_BREAKABLE_FXN();
	static TWD twd = { 0 };
	if ( twd.threadId == 0 )
	{
		twd.threadId = pthread_self();
		twd.cal = version_checker;
		twd.callback_arg = src_g;
	}
	if ( src_g == NULL )
	{
		return ( void_p )&twd;
	}

	G * _g = ( G * )src_g;
	char buf[ 50 ] = { 0 };
	time_t prev_time = { 0 } , cur_time = { 0 };
	struct Config_ver temp_ver = { 0 };
	while ( 1 )
	{
		cur_time = time( NULL );

		if ( CLOSE_APP_VAR() ) break;

		if ( _g->appcfg.ver == NULL || difftime( cur_time , prev_time ) > 5 )
		{
			prev_time = cur_time;

			memset( buf , 0 , sizeof( buf ) );
			const char * config_ver_file_content = read_file( CONFIG_ROOT_PATH "/config_ver.txt" , ( char * )buf );
			MM_BREAK_IF( !config_ver_file_content , errGeneral , 0 , "cannot open and read version file" );

			result( json_element ) rs_config_ver = json_parse( config_ver_file_content );
			//free( ( void_p )config_ver_file_content );
			MM_BREAK_IF( catch_error( &rs_config_ver , "config_ver" , 1 ) , errGeneral , 0 , "error in json_parse version file" );
			typed( json_element ) el_config_ver = result_unwrap( json_element )( &rs_config_ver );

			result( json_element ) ver = json_object_find( el_config_ver.value.as_object , "ver" );
			MM_BREAK_IF( catch_error( &ver , "ver" , 1 ) , errGeneral , 0 , "ver not found" );

			memset( &temp_ver , 0 , sizeof( temp_ver ) );

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
				_g->appcfg.ver = ( struct Config_ver * )memcpy( &_g->appcfg.temp_ver , &temp_ver , sizeof( temp_ver ) );
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
		case 3: ;
		case 2: ;
		case 1: _g->stat.round_zero_set.syscal_err_count++;
	M_V_END_RET
	return VOID_RET;
}

// TODO . fix memory leak
// TODO . echo acceptible config one time to inform user
_THREAD_FXN void_p config_loader( void_p src_g )
{
	INIT_BREAKABLE_FXN();
	static TWD twd = { 0 };
	if ( twd.threadId == 0 )
	{
		twd.threadId = pthread_self();
		twd.cal = config_loader;
		twd.callback_arg = src_g;
	}
	if ( src_g == NULL )
	{
		return ( void_p )&twd;
	}

	G * _g = ( G * )src_g;
	//time_t prev_time , cur_time;
	
	while ( !_g->appcfg.version_changed ) // load after version loaded
	{
		if ( CLOSE_APP_VAR() ) break;
		mng_basic_thread_sleep( _g , HI_PRIORITY_THREAD );
	}
	
	typed( json_element ) el_Protocol_Bridge_config;
	
	while ( 1 )
	{
		//cur_time = time( NULL );
		if ( CLOSE_APP_VAR() ) break;
		if ( _g->appcfg.g_cfg == NULL || _g->appcfg.version_changed /* || difftime(cur_time , prev_time) > 15 * 60*/ )
		{
			//prev_time = prev_time; // to ignore warning
			//prev_time = cur_time;
	
			Gcfg temp_config = { 0 };
			Gcfg0 * pGeneralConfiguration = ( Gcfg0 * )&temp_config;
			Bcfg * pProtocol_Bridges = NULL;
			size_t Protocol_Bridges_count = 0;
			{
				const char * Protocol_Bridge_config_file_content = read_file( CONFIG_ROOT_PATH "/Protocol_Bridge_config.txt" , NULL );
				MM_BREAK_IF( !Protocol_Bridge_config_file_content , errGeneral , 0 , "cannot open config file" );
					
				result( json_element ) rs_Protocol_Bridge_config = json_parse( Protocol_Bridge_config_file_content );
				free( ( void_p )Protocol_Bridge_config_file_content );
				MM_BREAK_IF( catch_error( &rs_Protocol_Bridge_config , "Protocol_Bridge_config" , 1 ) , errGeneral , 0 , "cannot parse config file" );
				el_Protocol_Bridge_config = result_unwrap( json_element )( &rs_Protocol_Bridge_config );
	
				// load general configurations
				if ( _g->appcfg.ver->Major >= 1 ) // first version of config file structure
				{
					result( json_element ) re_configurations = json_object_find( el_Protocol_Bridge_config.value.as_object , "configurations" );
					MM_BREAK_IF( catch_error( &re_configurations , "configurations" , 1 ) , errGeneral , 0 , "configurations" );
					typed( json_element ) el_configurations = result_unwrap( json_element )( &re_configurations );
	
					#define CFG_ELEM_STR( name )																			/**/\
						result( json_element ) re_##name = json_object_find( el_configurations.value.as_object , #name );	/**/\
						M_BREAK_IF( catch_error( &re_##name , #name , 1 ) , errGeneral , 0 );									/**/\
						typed( json_element ) el_##name = result_unwrap( json_element )( &re_##name );						/**/\
						NEWSTR( pGeneralConfiguration->name , el_##name.value.as_string , 0 );								/**/
					#define CFG_ELEM_I( name )																				/**/\
						result( json_element ) re_##name = json_object_find( el_configurations.value.as_object , #name );	/**/\
						M_BREAK_IF( catch_error( &re_##name , #name , 1 ) , errGeneral , 0 );									/**/\
						typed( json_element ) el_##name = result_unwrap( json_element )( &re_##name );						/**/\
						pGeneralConfiguration->name = (int)el_##name.value.as_number.value.as_long;							/**/
					#define CFG_ELEM_I64( name )																			/**/\
						result( json_element ) re_##name = json_object_find( el_configurations.value.as_object , #name );	/**/\
						M_BREAK_IF( catch_error( &re_##name , #name , 1 ) , errGeneral , 0 );									/**/\
						typed( json_element ) el_##name = result_unwrap( json_element )( &re_##name );						/**/\
						pGeneralConfiguration->name = (int64)el_##name.value.as_number.value.as_long;						/**/

					CFG_ELEM_STR( create_date );																			/**/\
					CFG_ELEM_STR( modify_date );																			/**/\
					CFG_ELEM_STR( config_name );																			/**/\
					CFG_ELEM_STR( config_tags );																			/**/\
					CFG_ELEM_STR( description );																			/**/\
					CFG_ELEM_STR( log_level );																				/**/\
					CFG_ELEM_STR( log_file );																				/**/\
					CFG_ELEM_I( enable );																					/**/\
					CFG_ELEM_I( shutdown );																					/**/\
					CFG_ELEM_I( watchdog_enabled );																			/**/\
					CFG_ELEM_I( load_prev_config );																			/**/\
					CFG_ELEM_I( dump_current_config );																		/**/\
					CFG_ELEM_I( dump_prev_config );																			/**/\
					CFG_ELEM_I( time_out_sec );																				/**/\
					CFG_ELEM_I( verbose_mode );																				/**/\
					CFG_ELEM_I( hi_frequent_log_interval_sec );																/**/\
					CFG_ELEM_I( refresh_variable_from_scratch );															/**/\
					CFG_ELEM_I( stat_referesh_interval_sec );																/**/\


					CFG_ELEM_I( synchronization_min_wait );																	/**/\
					CFG_ELEM_I( synchronization_max_roundup );																/**/\
					CFG_ELEM_I( show_line_hit );																			/**/\
					CFG_ELEM_I( retry_unexpected_wait_for_sock );															/**/\
					CFG_ELEM_I( number_in_short_form );																		/**/\
					CFG_ELEM_STR( NetworkStack_FilterType );																/**/\
					
					CFG_ELEM_I64( default_low_basic_thread_delay_nanosec );													/**/\
					CFG_ELEM_I64( default_normal_basic_thread_delay_nanosec );												/**/\
					CFG_ELEM_I64( default_hi_basic_thread_delay_nanosec );													/**/
					

					#undef CFG_ELEM_I
					#undef CFG_ELEM_STR
				}
	
				// load Protocol_Bridges
				{
					result( json_element ) re_Protocol_Bridges = json_object_find( el_Protocol_Bridge_config.value.as_object , "Protocol_Bridges" );
					MM_BREAK_IF( catch_error( &re_Protocol_Bridges , "Protocol_Bridges" , 1 ) , errGeneral , 0 , "Protocol_Bridges" );
					typed( json_element ) el_Protocol_Bridges = result_unwrap( json_element )( &re_Protocol_Bridges );
	
					MM_BREAK_IF( ( Protocol_Bridges_count = el_Protocol_Bridges.value.as_object->count ) < 1 , errGeneral , 0 , "Protocol_Bridges must be not zero" );
	
					M_BREAK_IF( !( pProtocol_Bridges = MALLOC_AR( pProtocol_Bridges , el_Protocol_Bridges.value.as_object->count ) ) , errMemoryLow , 0 );
					MEMSET_ZERO( pProtocol_Bridges , el_Protocol_Bridges.value.as_object->count );
						
					for ( int i = 0 ; i < el_Protocol_Bridges.value.as_object->count ; i++ ) // each bridge
					{
						((Bcfg0 *)(pProtocol_Bridges + i))->temp_data._g = ( void_p )_g; // each config must know global settings
						const char * output_Protocol_Bridge_name = ( *( el_Protocol_Bridges.value.as_object->entries + i ) )->key;
	
						result( json_element ) re_each_bridge = json_object_find( el_Protocol_Bridges.value.as_object , output_Protocol_Bridge_name );
						M_BREAK_IF( catch_error( &re_each_bridge , output_Protocol_Bridge_name , 1 ) , errGeneral , 0 );
						typed( json_element ) el_each_bridge = result_unwrap( json_element )( &re_each_bridge );
	
						strcpy( ( ( Bcfg0 * )( pProtocol_Bridges + i ) )->id.bridge_name , output_Protocol_Bridge_name );
						
						#define CFG_ELEM_STR( part , name )																	/**/\
						result( json_element ) re_##name = json_object_find( el_each_bridge.value.as_object , #name );		/**/\
						M_BREAK_IF( catch_error( &re_##name , #name , 1 ) , errGeneral , 0 );									/**/\
						typed( json_element ) el_##name = result_unwrap( json_element )( &re_##name );						/**/\
						strcpy(((Bcfg0 *)(pProtocol_Bridges + i))->part.name , el_##name.value.as_string );					/**/
	
						#define CFG_ELEM_I( part , name )																		/**/\
						result( json_element ) re_##name = json_object_find( el_each_bridge.value.as_object , #name );			/**/\
						M_BREAK_IF( catch_error( &re_##name , #name , 1 ) , errGeneral , 0 );										/**/\
						typed( json_element ) el_##name = result_unwrap( json_element )( &re_##name );							/**/\
						((Bcfg0 *)(pProtocol_Bridges + i))->part.name = (int)el_##name.value.as_number.value.as_long;			/**/
	
						CFG_ELEM_STR( id , out_type );
						CFG_ELEM_STR( id , thread_handler_act );
						CFG_ELEM_I( maintained , enable );
						
						// bridge inputs
						result( json_element ) re_inputs = json_object_find( el_each_bridge.value.as_object , "inputs" );
						MM_BREAK_IF( catch_error( &re_inputs , "inputs" , 1 ) , errGeneral , 0 , "inputs" );
						typed( json_element ) el_inputs = result_unwrap( json_element )( &re_inputs );
												
						((Bcfg0 *)(pProtocol_Bridges + i))->maintained.in_count = el_inputs.value.as_object->count;
						
						M_BREAK_IF( !( ((Bcfg0 *)(pProtocol_Bridges + i))->maintained.in = MALLOC_AR( ((Bcfg0 *)(pProtocol_Bridges + i))->maintained.in , ((Bcfg0 *)(pProtocol_Bridges + i))->maintained.in_count ) ) , errMemoryLow , 0 );
						MEMSET_ZERO( ((Bcfg0 *)(pProtocol_Bridges + i))->maintained.in , ((Bcfg0 *)(pProtocol_Bridges + i))->maintained.in_count );
						for ( int j = 0 ; j < ((Bcfg0 *)(pProtocol_Bridges + i))->maintained.in_count ; j++ )
						{
							const char * output_input_name = ( *( el_inputs.value.as_object->entries + j ) )->key;

							strcpy( ( ((Bcfg0 *)(pProtocol_Bridges + i))->maintained.in + j )->name , output_input_name );

							result( json_element ) re_inp = json_object_find( el_inputs.value.as_object , output_input_name );
							M_BREAK_IF( catch_error( &re_inp , output_input_name , 1 ) , errGeneral , 0 );
							typed( json_element ) el_inp = result_unwrap( json_element )( &re_inp );

							#define IN_CFG_ELEM_STR( elem , namee )																		/**/\
							result( json_element ) re_##namee = json_object_find( elem.value.as_object , #namee );						/**/\
							M_BREAK_IF( catch_error( &re_##namee , #namee , 1 ) , errGeneral , 0 );											/**/\
							typed( json_element ) el_##namee = result_unwrap( json_element )( &re_##namee );								/**/\
							strcpy( ( ((Bcfg0 *)(pProtocol_Bridges + i))->maintained.in + j )->data.namee , el_##namee.value.as_string );					/**/

							#define IN_CFG_ELEM_I( elem , namee )																				/**/\
							result( json_element ) re_##namee = json_object_find( elem.value.as_object , #namee );								/**/\
							M_BREAK_IF( catch_error( &re_##namee , #namee , 1 ) , errGeneral , 0 );													/**/\
							typed( json_element ) el_##namee = result_unwrap( json_element )( &re_##namee );										/**/\
							( ((Bcfg0 *)(pProtocol_Bridges + i))->maintained.in + j )->data.namee = (int)el_##namee.value.as_number.value.as_long;					/**/

							IN_CFG_ELEM_STR( el_inp , group );
							IN_CFG_ELEM_STR( el_inp , group_type );
							IN_CFG_ELEM_STR( el_inp , UDP_origin_ip );
							IN_CFG_ELEM_STR( el_inp , UDP_origin_interface );

							IN_CFG_ELEM_STR( el_inp , UDP_origin_ports );
							IN_CFG_ELEM_I( el_inp , enable );
							IN_CFG_ELEM_I( el_inp , reset_connection );

							#undef IN_CFG_ELEM_STR
							#undef IN_CFG_ELEM_I
						}
						
						// bridge outputs
						result( json_element ) re_outputs = json_object_find( el_each_bridge.value.as_object , "outputs" );
						if ( !catch_error( &re_outputs , "outputs" , 0 ) )
						{
							typed( json_element ) el_outputs = result_unwrap( json_element )( &re_outputs );

							( ( Bcfg0 * )( pProtocol_Bridges + i ) )->maintained.out_count = el_outputs.value.as_object->count;

							M_BREAK_IF( !( ( ( Bcfg0 * )( pProtocol_Bridges + i ) )->maintained.out = MALLOC_AR( ( ( Bcfg0 * )( pProtocol_Bridges + i ) )->maintained.out , ( ( Bcfg0 * )( pProtocol_Bridges + i ) )->maintained.out_count ) ) , errMemoryLow , 0 );
							MEMSET_ZERO( ( ( Bcfg0 * )( pProtocol_Bridges + i ) )->maintained.out , ( ( Bcfg0 * )( pProtocol_Bridges + i ) )->maintained.out_count );
							for ( int j = 0 ; j < ( ( Bcfg0 * )( pProtocol_Bridges + i ) )->maintained.out_count ; j++ )
							{
								const char * output_outputs_name = ( *( el_outputs.value.as_object->entries + j ) )->key;

								strcpy( ( ((Bcfg0 *)(pProtocol_Bridges + i))->maintained.out + j )->name , output_outputs_name );

								result( json_element ) re_out = json_object_find( el_outputs.value.as_object , output_outputs_name );
								M_BREAK_IF( catch_error( &re_out , output_outputs_name , 1 ) , errGeneral , 0 );
								typed( json_element ) el_out = result_unwrap( json_element )( &re_out );

								#define IN_CFG_ELEM_STR( elem , namee )																		/**/\
								result( json_element ) re_##namee = json_object_find( elem.value.as_object , #namee );						/**/\
								M_BREAK_IF( catch_error( &re_##namee , #namee , 1 ) , errGeneral , 0 );											/**/\
								typed( json_element ) el_##namee = result_unwrap( json_element )( &re_##namee );								/**/\
								strcpy( ( ((Bcfg0 *)(pProtocol_Bridges + i))->maintained.out + j )->data.namee , el_##namee.value.as_string );		/**/

								#define IN_CFG_ELEM_I( elem , namee )																				/**/\
								result( json_element ) re_##namee = json_object_find( elem.value.as_object , #namee );								/**/\
								M_BREAK_IF( catch_error( &re_##namee , #namee , 1 ) , errGeneral , 0 );													/**/\
								typed( json_element ) el_##namee = result_unwrap( json_element )( &re_##namee );										/**/\
								( ((Bcfg0 *)(pProtocol_Bridges + i))->maintained.out + j )->data.namee = (int)el_##namee.value.as_number.value.as_long;	/**/

								IN_CFG_ELEM_STR( el_out , group );
								IN_CFG_ELEM_STR( el_out , group_type );
								IN_CFG_ELEM_STR( el_out , TCP_destination_ip );
								IN_CFG_ELEM_STR( el_out , TCP_destination_interface );

								IN_CFG_ELEM_STR( el_out , TCP_destination_ports );
								IN_CFG_ELEM_I( el_out , enable );
								IN_CFG_ELEM_I( el_out , reset_connection );

								#undef IN_CFG_ELEM_STR
								#undef IN_CFG_ELEM_I
							}
						}
	
						#undef CFG_ELEM_I
						#undef CFG_ELEM_STR
					}
	
					json_free( &el_Protocol_Bridges );
				}
			}
	
			int initial_general_config = 0;
			// 
			if ( _g->appcfg.g_cfg == NULL ) // TODO . make assignemnt atomic
			{
				M_BREAK_IF( !( _g->appcfg.g_cfg = malloc( sizeof( struct Global_Config ) ) ) , errMemoryLow , 0 );
	
				MEMSET_ZERO( _g->appcfg.g_cfg , 1 );
				memcpy( _g->appcfg.g_cfg , &temp_config , sizeof( temp_config ) );
				memset( &temp_config , 0 , sizeof( temp_config ) ); // copy to global variable and then zero to not free strings
				initial_general_config = 1;
				_g->appcfg.g_cfg_changed = 1;
			}
			if ( !initial_general_config )
			{
				// compare general config
				DAC( _g->appcfg.prev_cfg );
	
				_g->appcfg.prev_cfg = _g->appcfg.g_cfg;
				_g->appcfg.g_cfg = NULL;
	
				M_BREAK_IF( !( _g->appcfg.g_cfg = malloc( sizeof( struct Global_Config ) ) ) , errMemoryLow , 0 );
				memset( _g->appcfg.g_cfg , 0 , sizeof( struct Global_Config ) );
				memcpy( _g->appcfg.g_cfg , &temp_config , sizeof( temp_config ) );
				memset( &temp_config , 0 , sizeof( temp_config ) ); // copy to global variable and then zero to not free strings
	
				if ( _g->appcfg.prev_cfg != NULL && _g->appcfg.g_cfg != NULL )
				{
					_g->appcfg.g_cfg_changed |= !STR_SAME( _g->appcfg.g_cfg->c.c.create_date , _g->appcfg.prev_cfg->c.c.create_date );
					_g->appcfg.g_cfg_changed |= !STR_SAME( _g->appcfg.g_cfg->c.c.modify_date , _g->appcfg.prev_cfg->c.c.modify_date );
					_g->appcfg.g_cfg_changed |= !STR_SAME( _g->appcfg.g_cfg->c.c.config_name , _g->appcfg.prev_cfg->c.c.config_name );
					_g->appcfg.g_cfg_changed |= !STR_SAME( _g->appcfg.g_cfg->c.c.config_tags , _g->appcfg.prev_cfg->c.c.config_tags );
					_g->appcfg.g_cfg_changed |= !STR_SAME( _g->appcfg.g_cfg->c.c.description , _g->appcfg.prev_cfg->c.c.description );
					_g->appcfg.g_cfg_changed |= !STR_SAME( _g->appcfg.g_cfg->c.c.log_level , _g->appcfg.prev_cfg->c.c.log_level );
					_g->appcfg.g_cfg_changed |= !STR_SAME( _g->appcfg.g_cfg->c.c.log_file , _g->appcfg.prev_cfg->c.c.log_file );

					_g->appcfg.g_cfg_changed |= !( _g->appcfg.g_cfg->c.c.enable == _g->appcfg.prev_cfg->c.c.enable );
					_g->appcfg.g_cfg_changed |= !( _g->appcfg.g_cfg->c.c.shutdown == _g->appcfg.prev_cfg->c.c.shutdown );
					_g->appcfg.g_cfg_changed |= !( _g->appcfg.g_cfg->c.c.watchdog_enabled == _g->appcfg.prev_cfg->c.c.watchdog_enabled );
					_g->appcfg.g_cfg_changed |= !( _g->appcfg.g_cfg->c.c.load_prev_config == _g->appcfg.prev_cfg->c.c.load_prev_config );
					_g->appcfg.g_cfg_changed |= !( _g->appcfg.g_cfg->c.c.dump_current_config == _g->appcfg.prev_cfg->c.c.dump_current_config );
					_g->appcfg.g_cfg_changed |= !( _g->appcfg.g_cfg->c.c.dump_prev_config == _g->appcfg.prev_cfg->c.c.dump_prev_config );
					_g->appcfg.g_cfg_changed |= !( _g->appcfg.g_cfg->c.c.time_out_sec == _g->appcfg.prev_cfg->c.c.time_out_sec );

					_g->appcfg.g_cfg_changed |= !( _g->appcfg.g_cfg->c.c.verbose_mode == _g->appcfg.prev_cfg->c.c.verbose_mode );
					_g->appcfg.g_cfg_changed |= !( _g->appcfg.g_cfg->c.c.hi_frequent_log_interval_sec == _g->appcfg.prev_cfg->c.c.hi_frequent_log_interval_sec );
					_g->appcfg.g_cfg_changed |= !( _g->appcfg.g_cfg->c.c.refresh_variable_from_scratch == _g->appcfg.prev_cfg->c.c.refresh_variable_from_scratch );
					_g->appcfg.g_cfg_changed |= !( _g->appcfg.g_cfg->c.c.stat_referesh_interval_sec == _g->appcfg.prev_cfg->c.c.stat_referesh_interval_sec );
					//_g->appcfg.g_cfg_changed |= !STR_SAME( _g->appcfg.g_cfg->c.c.thread_handler_type , _g->appcfg.prev_cfg->c.c.thread_handler_type );
	
					_g->appcfg.g_cfg_changed |= !( _g->appcfg.g_cfg->c.c.synchronization_min_wait == _g->appcfg.prev_cfg->c.c.synchronization_min_wait );
					_g->appcfg.g_cfg_changed |= !( _g->appcfg.g_cfg->c.c.synchronization_max_roundup == _g->appcfg.prev_cfg->c.c.synchronization_max_roundup );
					_g->appcfg.g_cfg_changed |= !( _g->appcfg.g_cfg->c.c.show_line_hit == _g->appcfg.prev_cfg->c.c.show_line_hit );
					_g->appcfg.g_cfg_changed |= !( _g->appcfg.g_cfg->c.c.retry_unexpected_wait_for_sock == _g->appcfg.prev_cfg->c.c.retry_unexpected_wait_for_sock );
					_g->appcfg.g_cfg_changed |= !( _g->appcfg.g_cfg->c.c.number_in_short_form == _g->appcfg.prev_cfg->c.c.number_in_short_form );

					_g->appcfg.g_cfg_changed |= !STR_SAME( _g->appcfg.g_cfg->c.c.NetworkStack_FilterType , _g->appcfg.prev_cfg->c.c.NetworkStack_FilterType );
					
					_g->appcfg.g_cfg_changed |= !( _g->appcfg.g_cfg->c.c.default_low_basic_thread_delay_nanosec == _g->appcfg.prev_cfg->c.c.default_low_basic_thread_delay_nanosec );
					_g->appcfg.g_cfg_changed |= !( _g->appcfg.g_cfg->c.c.default_normal_basic_thread_delay_nanosec == _g->appcfg.prev_cfg->c.c.default_normal_basic_thread_delay_nanosec );
					_g->appcfg.g_cfg_changed |= !( _g->appcfg.g_cfg->c.c.default_hi_basic_thread_delay_nanosec == _g->appcfg.prev_cfg->c.c.default_hi_basic_thread_delay_nanosec );
						
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
				for ( int i = 0 ; i < _g->appcfg.bdj_psv_cfg_count ; i++ )
				{
					_g->appcfg.bdj_psv_cfg[ i ].m.m.temp_data.pcfg_changed = 1; // ham joz e set mishavad
				}
			}
			else if ( _g->appcfg.old_bdj_psv_cfg && _g->appcfg.bdj_psv_cfg ) // prev !null & curr !null
			{
				// from old perspective
				for ( int i = 0 ; i < _g->appcfg.old_bdj_psv_cfg_count ; i++ )
				{
					int prev_exist = 0;
					for ( int j = 0 ; j < _g->appcfg.bdj_psv_cfg_count ; j++ )
					{
						if ( Bcfg_id_equlity( &_g->appcfg.old_bdj_psv_cfg[ i ] , &_g->appcfg.bdj_psv_cfg[ j ] ) )
						{
							prev_exist = 1;
							if ( !bridge_cfg_data_equlity( &_g->appcfg.old_bdj_psv_cfg[ i ] , &_g->appcfg.bdj_psv_cfg[ j ] ) )
							{
								_g->appcfg.psv_cfg_changed = 1;
								_g->appcfg.bdj_psv_cfg[ j ].m.m.temp_data.pcfg_changed = 1;
							}
							break;
						}
					}
					if ( !prev_exist )
					{
						_g->appcfg.psv_cfg_changed = 1;
						_g->appcfg.old_bdj_psv_cfg[ i ].m.m.temp_data.pcfg_changed = 1;
						break;
					}
				}
				// from new perspective
				for ( int j = 0 ; j < _g->appcfg.bdj_psv_cfg_count ; j++ )
				{
					int new_exist = 0;
					for ( int i = 0 ; i < _g->appcfg.old_bdj_psv_cfg_count ; i++ )
					{
						if ( Bcfg_id_equlity( &_g->appcfg.old_bdj_psv_cfg[ i ] , &_g->appcfg.bdj_psv_cfg[ j ] ) )
						{
							new_exist = 1;
							if ( !bridge_cfg_data_equlity( &_g->appcfg.old_bdj_psv_cfg[ i ] , &_g->appcfg.bdj_psv_cfg[ j ] ) )
							{
								_g->appcfg.psv_cfg_changed = 1;
								_g->appcfg.bdj_psv_cfg[ j ].m.m.temp_data.pcfg_changed = 1;
							}
							break;
						}
					}
					if ( !new_exist )
					{
						_g->appcfg.psv_cfg_changed = 1;
						_g->appcfg.bdj_psv_cfg[ j ].m.m.temp_data.pcfg_changed = 1;
						break;
					}
				}
			}
	
			_g->appcfg.version_changed = 0;
	
			//if ( _g->appcfg._psv_cfg_changed && IF_VERBOSE_MODE_CONDITION() )
			//{
			//	_ECHO( "protocol_bridge config changed" );
			//}
		}
		mng_basic_thread_sleep( _g , LOW_PRIORITY_THREAD );
	}
	
	BEGIN_SMPL
	M_V_END_RET

	return NULL;
}

// TODO . aware of concurrency in config read and act on it
_THREAD_FXN void_p config_executer( void_p app_data )
{
	INIT_BREAKABLE_FXN();
	static TWD twd = { 0 };
	if ( twd.threadId == 0 )
	{
		twd.threadId = pthread_self();
		twd.cal = config_executer;
		twd.callback_arg = app_data;
	}
	if ( app_data == NULL )
	{
		return ( void_p )&twd;
	}
	G * _g = ( G * )app_data;

	while ( !_g->appcfg.bdj_psv_cfg_count ) // load after any config loaded
	{
		if ( CLOSE_APP_VAR() ) break;
		mng_basic_thread_sleep( _g , HI_PRIORITY_THREAD );
	}

	while ( 1 )
	{
		if ( CLOSE_APP_VAR() )
		{
			for ( int j = 0 ; j < _g->appcfg.bdj_psv_cfg_count ; j++ )
			{
				_g->appcfg.bdj_psv_cfg[ j ].m.m.maintained.enable = 0;
				apply_protocol_bridge_new_cfg_changes( _g , &_g->appcfg.bdj_psv_cfg[ j ] , &_g->appcfg.bdj_psv_cfg[ j ] );
			}
			break;
		}
		if ( _g->appcfg.g_cfg_changed )
		{
			_g->appcfg.g_cfg_changed = 0; // for now . TODO later
		}

		if ( _g->appcfg.psv_cfg_changed )
		{
			for ( int i = 0 ; i < _g->appcfg.old_bdj_psv_cfg_count ; i++ )
			{
				int exist = 0;
				for ( int j = 0 ; j < _g->appcfg.bdj_psv_cfg_count ; j++ )
				{
					if ( exist ) break;
					if ( Bcfg_id_equlity( &_g->appcfg.old_bdj_psv_cfg[ i ] , &_g->appcfg.bdj_psv_cfg[ j ] ) && _g->appcfg.bdj_psv_cfg[ j ].m.m.temp_data.pcfg_changed )
					{
						// existed cfg changed
						apply_protocol_bridge_new_cfg_changes( _g , &_g->appcfg.old_bdj_psv_cfg[ i ] , &_g->appcfg.bdj_psv_cfg[ j ] );
						exist = 1;
					}
				}
				if ( !exist )
				{
					// remove removed one
					remove_protocol_bridge( _g , &_g->appcfg.old_bdj_psv_cfg[ i ] );
				}
			}
			for ( int i = 0 ; i < _g->appcfg.bdj_psv_cfg_count ; i++ )
			{
				int exist = 0;
				for ( int j = 0 ; j < _g->appcfg.old_bdj_psv_cfg_count ; j++ )
				{
					if ( Bcfg_id_equlity( &_g->appcfg.bdj_psv_cfg[ i ] , &_g->appcfg.old_bdj_psv_cfg[ j ] ) )
					{
						exist = 1;
						break;
					}
				}
				if ( !exist )
				{
					// start new cfg
					add_new_protocol_bridge( _g , &_g->appcfg.bdj_psv_cfg[ i ] );
				}
			}
			_g->appcfg.psv_cfg_changed = 0; // changes applied
		}
		mng_basic_thread_sleep( _g , LOW_PRIORITY_THREAD );
	}
	return NULL;
}

void stop_protocol_bridge( G * _g , AB * pb )
{
	////INIT_BREAKABLE_FXN();

	pb->cpy_cfg.m.m.maintained.enable = 0;
	pb->cpy_cfg.m.m.temp_data.pcfg_changed = 1;
	apply_new_protocol_bridge_config( _g , pb , &pb->cpy_cfg );
	//// TODO
}

void apply_protocol_bridge_new_cfg_changes( G * _g , Bcfg * prev_pcfg , Bcfg * new_ccfg )
{
	//INIT_BREAKABLE_FXN();

	for ( int i = 0 ; i < _g->bridges.ABhs_masks_count ; i++ )
	{
		if ( _g->bridges.ABhs_masks[ i ] )
		{
			if ( Bcfg_id_equlity( &_g->bridges.ABs[ i ].single_AB->cpy_cfg , prev_pcfg ) )
			{
				apply_new_protocol_bridge_config( _g , _g->bridges.ABs[ i ].single_AB , new_ccfg );
			}
		}
	}
}

void remove_protocol_bridge( G * _g , Bcfg * pcfg )
{
	////INIT_BREAKABLE_FXN();

	for ( int i = 0 ; i < _g->bridges.ABhs_masks_count ; i++ )
	{
		if ( _g->bridges.ABhs_masks[ i ] )
		{
			if ( Bcfg_id_equlity( &_g->bridges.ABs[ i ].single_AB->cpy_cfg , pcfg ) )
			{
				stop_protocol_bridge( _g , _g->bridges.ABs[ i ].single_AB );
				_g->bridges.ABhs_masks[ i ] = 0;
				DAC( _g->bridges.ABs[ i ].single_AB );
			}
		}
	}
}

void add_new_protocol_bridge( G * _g , Bcfg * new_ccfg )
{
	INIT_BREAKABLE_FXN();

	int new_ccfg_placement_index = -1;
	while ( new_ccfg_placement_index < 0 ) // try to find one place for new wave
	{
		for ( int i = 0 ; i < _g->bridges.ABhs_masks_count ; i++ )
		{
			if ( !_g->bridges.ABhs_masks[ i ] )
			{
				new_ccfg_placement_index = i;
				break;
			}
		}
		if ( new_ccfg_placement_index < 0 )
		{
			int old_ABhs_masks_count = _g->bridges.ABhs_masks_count;
			int new_ABhs_masks_count = old_ABhs_masks_count + PREALLOCAION_SIZE;

			M_BREAK_IF( ( _g->bridges.ABhs_masks = REALLOC( _g->bridges.ABhs_masks , new_ABhs_masks_count * sizeof( int ) ) ) == REALLOC_ERR , errMemoryLow , 2 );
			MEMSET_ZERO( _g->bridges.ABhs_masks + old_ABhs_masks_count , PREALLOCAION_SIZE );

			M_BREAK_IF( ( _g->bridges.ABs = REALLOC( _g->bridges.ABs , new_ABhs_masks_count * sizeof( ABh ) ) ) == REALLOC_ERR , errMemoryLow , 1 );
			MEMSET_ZERO( _g->bridges.ABs + old_ABhs_masks_count , PREALLOCAION_SIZE );

			_g->bridges.ABhs_masks_count = new_ABhs_masks_count;
		}
	}

	ASSERT( _g->bridges.ABs[ new_ccfg_placement_index ].single_AB == NULL );
	M_BREAK_IF( ( _g->bridges.ABs[ new_ccfg_placement_index ].single_AB = MALLOC_ONE( _g->bridges.ABs[ new_ccfg_placement_index ].single_AB ) ) == NEW_ERR , errMemoryLow , 0 );
	MEMSET_ZERO_O( _g->bridges.ABs[ new_ccfg_placement_index ].single_AB );
	_g->bridges.ABhs_masks[ new_ccfg_placement_index ] = 1;
	copy_bridge_cfg( &_g->bridges.ABs[ new_ccfg_placement_index ].single_AB->cpy_cfg , new_ccfg );

	apply_protocol_bridge_new_cfg_changes( _g , new_ccfg , new_ccfg );

	BEGIN_RET
		case 3: DAC( _g->bridges.ABs );
		case 2: DAC( _g->bridges.ABhs_masks );
		case 1: _g->stat.round_zero_set.syscal_err_count++;
	M_V_END_RET
} // TODO . return value
