#define Uses_sleep
#define Uses_memcpy
#define Uses_TWD
#define Uses_pthread_t
#define Uses_json
#define Uses_config
#define Uses_helper

//#define DIRECT_ECHO_BUF _g->stat.last_command // just before include dep
#include <Protocol_Bridge.dep>

// TODO . exit gracefully by auto mechanism
// TODO . think about race condition
_THREAD_FXN void * version_checker( void * app_data )
{
	INIT_BREAKABLE_FXN();
	static TWD twd = { 0 };
	if ( twd.threadId == 0 )
	{
		twd.threadId = pthread_self();
		twd.cal = version_checker;
		twd.callback_arg = app_data;
	}
	if ( app_data == NULL )
	{
		return ( void * )&twd;
	}

	G * _g = ( G * )app_data;
	char buf[ 50 ] = { 0 };
	time_t prev_time = { 0 } , cur_time = { 0 };
	struct Config_ver temp_ver = { 0 };
	while ( 1 )
	{
		cur_time = time( NULL );

		if ( CLOSE_APP_VAR() ) break;

		if ( _g->appcfg._ver == NULL || difftime( cur_time , prev_time ) > 5 )
		{
			prev_time = cur_time;

			memset( buf , 0 , sizeof( buf ) );
			const char * config_ver_file_content = read_file( CONFIG_ROOT_PATH "/config_ver.txt" , ( char * )buf );
			MM_BREAK_IF( !config_ver_file_content , errGeneral , 0 , "cannot open and read version file" );

			result( json_element ) rs_config_ver = json_parse( config_ver_file_content );
			//free( ( void * )config_ver_file_content );
			MM_BREAK_IF( catch_error( &rs_config_ver , "config_ver" ) , errGeneral , 0 , "error in json_parse version file" );
			typed( json_element ) el_config_ver = result_unwrap( json_element )( &rs_config_ver );

			result( json_element ) ver = json_object_find( el_config_ver.value.as_object , "ver" );
			MM_BREAK_IF( catch_error( &ver , "ver" ) , errGeneral , 0 , "ver not found" );

			memset( &temp_ver , 0 , sizeof( temp_ver ) );

			strcpy( temp_ver.version , ( char * )ver.inner.value.value.as_string );
			char * perr = NULL;
			char * pver = ( char * )ver.inner.value.value.as_string;
			temp_ver.Major = ( int )strtol( strtok( pver , "." ) , &perr , 10 );			MM_BREAK_IF( !perr , errGeneral , 0 , "ver Major wrong" );
			temp_ver.Minor = ( int )strtol( strtok( NULL , "." ) , &perr , 10 );			MM_BREAK_IF( !perr , errGeneral , 0 , "ver Minor wrong" );
			temp_ver.Build = ( int )strtol( strtok( NULL , "." ) , &perr , 10 );			MM_BREAK_IF( !perr , errGeneral , 0 , "ver Build wrong" );
			temp_ver.Revision_Patch = ( int )strtol( strtok( NULL , "." ) , &perr , 10 );	MM_BREAK_IF( !perr , errGeneral , 0 , "ver Revision_Patch wrong" );
			json_free( &el_config_ver ); // string is user so must be free at the end

			if ( _g->appcfg._ver == NULL || strcmp( temp_ver.version , _g->appcfg._ver->version ) )
			{
				_g->appcfg._ver = ( struct Config_ver * )memcpy( &_g->appcfg.___temp_ver , &temp_ver , sizeof( temp_ver ) );
				_g->appcfg._version_changed = 1;
				//if ( IF_VERBOSE_MODE_CONDITION() )
				//{
				//	_ECHO( "version changed %s" , _g->appcfg._ver->version );
				//}
			}
		}
		sleep( 1 );
	}
	BEGIN_RET
		case 3:
	{
	}
		case 2:
		{
		}
		case 1: _g->stat.round_zero_set.syscal_err_count++;
			M_V_END_RET
				return VOID_RET;
}

// TODO . fix memory leak
// TODO . echo acceptible config one time to inform user
_THREAD_FXN void * config_loader( void * app_data )
{
	INIT_BREAKABLE_FXN();
	static TWD twd = { 0 };
	if ( twd.threadId == 0 )
	{
		twd.threadId = pthread_self();
		twd.cal = config_loader;
		twd.callback_arg = app_data;
	}
	if ( app_data == NULL )
	{
		return ( void * )&twd;
	}

	//	G * _g = ( G * )app_data;
	//	time_t prev_time , cur_time;
	//
	//	while ( !_g->appcfg._version_changed ) // load after version loaded
	//	{
	//		if ( CLOSE_APP_VAR() ) break;
	//		sleep( 1 );
	//	}
	//
	//	typed( json_element ) el_Protocol_Bridge_config;
	//
	//	while ( 1 )
	//	{
	//		cur_time = time( NULL );
	//		if ( CLOSE_APP_VAR() ) break;
	//		if ( _g->appcfg._general_config == NULL || _g->appcfg._version_changed /* || difftime(cur_time , prev_time) > 15 * 60*/ )
	//		{
	//			prev_time = prev_time; // to ignore warning
	//			prev_time = cur_time;
	//
	//			Gcfg temp_config = { 0 };
	//			Gcfg0 * pGeneralConfiguration = ( Gcfg0 * )&temp_config;
	//			Bcfg * pProtocol_Bridges = NULL;
	//			size_t Protocol_Bridges_count = 0;
	//			{
	//				const char * Protocol_Bridge_config_file_content = read_file( CONFIG_ROOT_PATH "/Protocol_Bridge_config.txt" , NULL );
	//				MM_BREAK_IF( !Protocol_Bridge_config_file_content , errGeneral , 0 , "cannot open config file" );
	//				
	//				result( json_element ) rs_Protocol_Bridge_config = json_parse( Protocol_Bridge_config_file_content );
	//				free( ( void * )Protocol_Bridge_config_file_content );
	//				MM_BREAK_IF( catch_error( &rs_Protocol_Bridge_config , "Protocol_Bridge_config" ) , errGeneral , 0 , "cannot parse config file" );
	//				el_Protocol_Bridge_config = result_unwrap( json_element )( &rs_Protocol_Bridge_config );
	//
	//				/*configurations*/
	//				if ( _g->appcfg._ver->Major >= 1 ) // first version of config file structure
	//				{
	//					result( json_element ) re_configurations = json_object_find( el_Protocol_Bridge_config.value.as_object , "configurations" );
	//					MM_BREAK_IF( catch_error( &re_configurations , "configurations" ) , errGeneral , 0 , "configurations" );
	//					typed( json_element ) el_configurations = result_unwrap( json_element )( &re_configurations );
	//
	//#define CFG_ELEM_STR( name ) \
	//						result( json_element ) re_##name = json_object_find( el_configurations.value.as_object , #name );\
	//						M_BREAK_IF( catch_error( &re_##name , #name ) , errGeneral , 0 );\
	//						typed( json_element ) el_##name = result_unwrap( json_element )( &re_##name );\
	//						NEWSTR( pGeneralConfiguration->name , el_##name.value.as_string , 0 );
	//#define CFG_ELEM_I( name ) \
	//						result( json_element ) re_##name = json_object_find( el_configurations.value.as_object , #name );\
	//						M_BREAK_IF( catch_error( &re_##name , #name ) , errGeneral , 0 );\
	//						typed( json_element ) el_##name = result_unwrap( json_element )( &re_##name );\
	//						pGeneralConfiguration->name = (int)el_##name.value.as_number.value.as_long;
	//
	//					CFG_ELEM_STR( create_date );
	//					CFG_ELEM_STR( modify_date );
	//					CFG_ELEM_STR( config_name );
	//					CFG_ELEM_STR( config_tags );
	//					CFG_ELEM_STR( description );
	//					CFG_ELEM_STR( log_level );
	//					CFG_ELEM_STR( log_file );
	//					CFG_ELEM_I( enable );
	//					CFG_ELEM_I( shutdown );
	//					CFG_ELEM_I( watchdog_enabled );
	//					CFG_ELEM_I( load_prev_config );
	//					CFG_ELEM_I( dump_current_config );
	//					CFG_ELEM_I( dump_prev_config );
	//					CFG_ELEM_I( time_out_sec );
	//					CFG_ELEM_I( verbose_mode );
	//					CFG_ELEM_I( hi_frequent_log_interval_sec );
	//					CFG_ELEM_I( refresh_variable_from_scratch );
	//					CFG_ELEM_I( stat_referesh_interval_sec );
	//					CFG_ELEM_STR( thread_handler_type );
	//					
	//					CFG_ELEM_I( synchronization_min_wait );
	//					CFG_ELEM_I( synchronization_max_roundup );
	//					CFG_ELEM_I( show_line_hit );
	//					CFG_ELEM_I( retry_unexpected_wait_for_sock );
	//					CFG_ELEM_I( number_in_short_form );
	//					
	//					
	//					
	//					
	//#undef CFG_ELEM_I
	//#undef CFG_ELEM_STR
	//				}
	//
	//				/*Protocol_Bridges*/
	//				{
	//					result( json_element ) re_Protocol_Bridges = json_object_find( el_Protocol_Bridge_config.value.as_object , "Protocol_Bridges" );
	//					MM_BREAK_IF( catch_error( &re_Protocol_Bridges , "Protocol_Bridges" ) , errGeneral , 0 , "Protocol_Bridges" );
	//					typed( json_element ) el_Protocol_Bridges = result_unwrap( json_element )( &re_Protocol_Bridges );
	//
	//					MM_BREAK_IF( ( Protocol_Bridges_count = el_Protocol_Bridges.value.as_object->count ) < 1 , errGeneral , 0 , "Protocol_Bridges must be not zero" );
	//
	//					M_BREAK_IF( !( pProtocol_Bridges = NEWBUF( Bcfg , el_Protocol_Bridges.value.as_object->count ) ) , errMemoryLow , 0 );
	//					MEMSET_ZERO( pProtocol_Bridges , Bcfg , el_Protocol_Bridges.value.as_object->count );
	//					
	//
	//					for ( int i = 0 ; i < el_Protocol_Bridges.value.as_object->count ; i++ )
	//					{
	//						((struct protocol_bridge_cfg_0 *)(pProtocol_Bridges + i))->temp_data._g = ( void * )_g;
	//
	//						char output_Protocol_Bridge_name[ 32 ];
	//						memset( output_Protocol_Bridge_name , 0 , sizeof( output_Protocol_Bridge_name ) );
	//						sprintf( output_Protocol_Bridge_name , "bridge%d" , i + 1 );
	//
	//						result( json_element ) re_output_Protocol_Bridge = json_object_find( el_Protocol_Bridges.value.as_object , output_Protocol_Bridge_name );
	//						M_BREAK_IF( catch_error( &re_output_Protocol_Bridge , output_Protocol_Bridge_name ) , errGeneral , 0 );
	//						typed( json_element ) el_output_Protocol_Bridge = result_unwrap( json_element )( &re_output_Protocol_Bridge );
	//
	//						#define CFG_ELEM_STR( name ) \
	//							result( json_element ) re_##name = json_object_find( el_output_Protocol_Bridge.value.as_object , #name );\
	//							M_BREAK_IF( catch_error( &re_##name , #name ) , errGeneral , 0 );\
	//							typed( json_element ) el_##name = result_unwrap( json_element )( &re_##name );\
	//							strcpy(((struct protocol_bridge_cfg_0 *)(pProtocol_Bridges + i))->name , el_##name.value.as_string );
	//
	//						#define CFG_ID_ELEM_STR( name ) \
	//							result( json_element ) re_##name = json_object_find( el_output_Protocol_Bridge.value.as_object , #name );\
	//							M_BREAK_IF( catch_error( &re_##name , #name ) , errGeneral , 0 );\
	//							typed( json_element ) el_##name = result_unwrap( json_element )( &re_##name );\
	//							strcpy(((struct protocol_bridge_cfg_0 *)(pProtocol_Bridges + i))->id.name , el_##name.value.as_string );
	//
	//						#define CFG_ELEM_I_maintained( name ) \
	//							result( json_element ) re_##name = json_object_find( el_output_Protocol_Bridge.value.as_object , #name );\
	//							M_BREAK_IF( catch_error( &re_##name , #name ) , errGeneral , 0 );\
	//							typed( json_element ) el_##name = result_unwrap( json_element )( &re_##name );\
	//							((struct protocol_bridge_cfg_0 *)(pProtocol_Bridges + i))->maintained.name = (int)el_##name.value.as_number.value.as_long;
	//
	//						#define CFG_ELEM_I_momentary( name ) \
	//							result( json_element ) re_##name = json_object_find( el_output_Protocol_Bridge.value.as_object , #name );\
	//							M_BREAK_IF( catch_error( &re_##name , #name ) , errGeneral , 0 );\
	//							typed( json_element ) el_##name = result_unwrap( json_element )( &re_##name );\
	//							((struct protocol_bridge_cfg_0 *)(pProtocol_Bridges + i))->momentary.name = (int)el_##name.value.as_number.value.as_long;
	//
	//						#define CFG_ID_ELEM_I( name ) \
	//							result( json_element ) re_##name = json_object_find( el_output_Protocol_Bridge.value.as_object , #name );\
	//							M_BREAK_IF( catch_error( &re_##name , #name ) , errGeneral , 0 );\
	//							typed( json_element ) el_##name = result_unwrap( json_element )( &re_##name );\
	//							((struct protocol_bridge_cfg_0 *)(pProtocol_Bridges + i))->id.name = (int)el_##name.value.as_number.value.as_long;
	//
	//						strcpy( ( ( struct protocol_bridge_cfg_0 * )( pProtocol_Bridges + i ) )->id.protocol_bridge_name , output_Protocol_Bridge_name );
	//						CFG_ID_ELEM_STR( UDP_origin_ip );
	//						CFG_ID_ELEM_I( UDP_origin_port );
	//						CFG_ID_ELEM_STR( UDP_origin_interface );
	//
	//						CFG_ID_ELEM_STR( TCP_destination_ip );
	//						CFG_ID_ELEM_I( TCP_destination_port );
	//						CFG_ID_ELEM_STR( TCP_destination_interface );
	//
	//						CFG_ELEM_I_maintained( enable );
	//						CFG_ELEM_I_momentary( reset_connections );
	//						
	//
	//
	//						#undef CFG_ID_ELEM_I
	//						#undef CFG_ELEM_I
	//						#undef CFG_ID_ELEM_STR
	//						#undef CFG_ELEM_STR
	//					}
	//
	//					json_free( &el_Protocol_Bridges );
	//				}
	//			}
	//
	//			int initial_general_config = 0;
	//			if ( _g->appcfg._general_config == NULL ) // TODO . make assignemnt atomic
	//			{
	//				M_BREAK_IF( !( _g->appcfg._general_config = malloc( sizeof( struct Global_Config ) ) ) , errMemoryLow , 0 );
	//
	//				memset( _g->appcfg._general_config , 0 , sizeof( struct Global_Config ) );
	//				memcpy( _g->appcfg._general_config , &temp_config , sizeof( temp_config ) );
	//				memset( &temp_config , 0 , sizeof( temp_config ) ); // copy to global variable and then zero to not free strings
	//				initial_general_config = 1;
	//				_g->appcfg._general_config_changed = 1;
	//			}
	//			if ( !initial_general_config )
	//			{
	//				DAC( _g->appcfg._prev_general_config );
	//
	//				_g->appcfg._prev_general_config = _g->appcfg._general_config;
	//				_g->appcfg._general_config = NULL;
	//
	//				M_BREAK_IF( !( _g->appcfg._general_config = malloc( sizeof( struct Global_Config ) ) ) , errMemoryLow , 0 );
	//				memset( _g->appcfg._general_config , 0 , sizeof( struct Global_Config ) );
	//				memcpy( _g->appcfg._general_config , &temp_config , sizeof( temp_config ) );
	//				memset( &temp_config , 0 , sizeof( temp_config ) ); // copy to global variable and then zero to not free strings
	//
	//				if ( _g->appcfg._prev_general_config != NULL && _g->appcfg._general_config != NULL )
	//				{
	//					_g->appcfg._general_config_changed |= !!strcmp( _g->appcfg._general_config->c.c.create_date , _g->appcfg._prev_general_config->c.c.create_date );
	//					_g->appcfg._general_config_changed |= !!strcmp( _g->appcfg._general_config->c.c.modify_date , _g->appcfg._prev_general_config->c.c.modify_date );
	//					_g->appcfg._general_config_changed |= !!strcmp( _g->appcfg._general_config->c.c.config_name , _g->appcfg._prev_general_config->c.c.config_name );
	//					_g->appcfg._general_config_changed |= !!strcmp( _g->appcfg._general_config->c.c.config_tags , _g->appcfg._prev_general_config->c.c.config_tags );
	//					_g->appcfg._general_config_changed |= !!strcmp( _g->appcfg._general_config->c.c.description , _g->appcfg._prev_general_config->c.c.description );
	//					_g->appcfg._general_config_changed |= !!strcmp( _g->appcfg._general_config->c.c.log_level , _g->appcfg._prev_general_config->c.c.log_level );
	//					_g->appcfg._general_config_changed |= !!strcmp( _g->appcfg._general_config->c.c.log_file , _g->appcfg._prev_general_config->c.c.log_file );
	//
	//					_g->appcfg._general_config_changed |= !( _g->appcfg._general_config->c.c.enable == _g->appcfg._prev_general_config->c.c.enable );
	//					_g->appcfg._general_config_changed |= !( _g->appcfg._general_config->c.c.shutdown == _g->appcfg._prev_general_config->c.c.shutdown );
	//					_g->appcfg._general_config_changed |= !( _g->appcfg._general_config->c.c.watchdog_enabled == _g->appcfg._prev_general_config->c.c.watchdog_enabled );
	//					_g->appcfg._general_config_changed |= !( _g->appcfg._general_config->c.c.load_prev_config == _g->appcfg._prev_general_config->c.c.load_prev_config );
	//					_g->appcfg._general_config_changed |= !( _g->appcfg._general_config->c.c.dump_current_config == _g->appcfg._prev_general_config->c.c.dump_current_config );
	//					_g->appcfg._general_config_changed |= !( _g->appcfg._general_config->c.c.dump_prev_config == _g->appcfg._prev_general_config->c.c.dump_prev_config );
	//					_g->appcfg._general_config_changed |= !( _g->appcfg._general_config->c.c.time_out_sec == _g->appcfg._prev_general_config->c.c.time_out_sec );
	//
	//					_g->appcfg._general_config_changed |= !( _g->appcfg._general_config->c.c.verbose_mode == _g->appcfg._prev_general_config->c.c.verbose_mode );
	//					_g->appcfg._general_config_changed |= !( _g->appcfg._general_config->c.c.hi_frequent_log_interval_sec == _g->appcfg._prev_general_config->c.c.hi_frequent_log_interval_sec );
	//					_g->appcfg._general_config_changed |= !( _g->appcfg._general_config->c.c.refresh_variable_from_scratch == _g->appcfg._prev_general_config->c.c.refresh_variable_from_scratch );
	//					_g->appcfg._general_config_changed |= !( _g->appcfg._general_config->c.c.stat_referesh_interval_sec == _g->appcfg._prev_general_config->c.c.stat_referesh_interval_sec );
	//					_g->appcfg._general_config_changed |= !!strcmp( _g->appcfg._general_config->c.c.thread_handler_type , _g->appcfg._prev_general_config->c.c.thread_handler_type );
	//
	//					_g->appcfg._general_config_changed |= !( _g->appcfg._general_config->c.c.synchronization_min_wait == _g->appcfg._prev_general_config->c.c.synchronization_min_wait );
	//					_g->appcfg._general_config_changed |= !( _g->appcfg._general_config->c.c.synchronization_max_roundup == _g->appcfg._prev_general_config->c.c.synchronization_max_roundup );
	//					_g->appcfg._general_config_changed |= !( _g->appcfg._general_config->c.c.show_line_hit == _g->appcfg._prev_general_config->c.c.show_line_hit );
	//					_g->appcfg._general_config_changed |= !( _g->appcfg._general_config->c.c.retry_unexpected_wait_for_sock == _g->appcfg._prev_general_config->c.c.retry_unexpected_wait_for_sock );
	//					_g->appcfg._general_config_changed |= !( _g->appcfg._general_config->c.c.number_in_short_form == _g->appcfg._prev_general_config->c.c.number_in_short_form );
	//					
	//					
	//					
	//				}
	//			}
	//			if ( strcmp( _g->appcfg._general_config->c.c.thread_handler_type , "buttleneck" ) == 0 )
	//			{
	//				_g->appcfg._general_config->c.c.atht = buttleneck;
	//			}
	//			else if ( strcmp( _g->appcfg._general_config->c.c.thread_handler_type , "bidirection" ) == 0 )
	//			{
	//				_g->appcfg._general_config->c.c.atht = bidirection;
	//			}
	//			else if ( strcmp( _g->appcfg._general_config->c.c.thread_handler_type , "justIncoming" ) == 0 )
	//			{
	//				_g->appcfg._general_config->c.c.atht = justIncoming;
	//			}
	//
	//			if ( _g->appcfg._general_config_changed )
	//			{
	//				if ( IF_VERBOSE_MODE_CONDITION() )
	//				{
	//					_ECHO( "general config changed" );
	//				}
	//			}
	//
	//			if ( _g->appcfg._pprotocol_bridge_psvcfg )
	//			{
	//				DAC( _g->appcfg._pprev_protocol_bridge_psvcfg );
	//				_g->appcfg._prev_protocol_bridge_psvcfg_count = 0;
	//				_g->appcfg._pprev_protocol_bridge_psvcfg = _g->appcfg._pprotocol_bridge_psvcfg;
	//				_g->appcfg._prev_protocol_bridge_psvcfg_count = _g->appcfg._protocol_bridge_psvcfg_count;
	//			}
	//			_g->appcfg._pprotocol_bridge_psvcfg = pProtocol_Bridges;
	//			_g->appcfg._protocol_bridge_psvcfg_count = Protocol_Bridges_count;
	//			pProtocol_Bridges = NULL; // to not delete intentionally
	//			Protocol_Bridges_count = 0;
	//
	//			if ( _g->appcfg._pprev_protocol_bridge_psvcfg == NULL && _g->appcfg._pprotocol_bridge_psvcfg )
	//			{
	//				// all new ones
	//				_g->appcfg._psvcfg_changed = 1; // ham koli set mishavad change rokh dad
	//				for ( int i = 0 ; i < _g->appcfg._protocol_bridge_psvcfg_count ; i++ )
	//				{
	//					_g->appcfg._pprotocol_bridge_psvcfg[ i ].m.m.temp_data.pcfg_changed = 1; // ham joz e set mishavad
	//				}
	//			}
	//			else if ( _g->appcfg._pprev_protocol_bridge_psvcfg && _g->appcfg._pprotocol_bridge_psvcfg )
	//			{
	//				// from old perspective
	//				for ( int i = 0 ; i < _g->appcfg._prev_protocol_bridge_psvcfg_count ; i++ )
	//				{
	//					int prev_exist = 0;
	//					for ( int j = 0 ; j < _g->appcfg._protocol_bridge_psvcfg_count ; j++ )
	//					{
	//						if ( memcmp( &_g->appcfg._pprev_protocol_bridge_psvcfg[ i ].m.m.id , &_g->appcfg._pprotocol_bridge_psvcfg[ j ].m.m.id , sizeof( struct protocol_bridge_cfg_id ) ) == 0 )
	//						{
	//							prev_exist = 1;
	//							if ( memcmp( &_g->appcfg._pprev_protocol_bridge_psvcfg[ i ].m.m.maintained , &_g->appcfg._pprotocol_bridge_psvcfg[ j ].m.m.maintained , sizeof( struct protocol_bridge_maintained_parameter ) ) )
	//							{
	//								_g->appcfg._psvcfg_changed = 1;
	//								_g->appcfg._pprotocol_bridge_psvcfg[ j ].m.m.temp_data.pcfg_changed = 1;
	//							}
	//							break;
	//						}
	//					}
	//					if ( !prev_exist )
	//					{
	//						_g->appcfg._psvcfg_changed = 1;
	//						_g->appcfg._pprev_protocol_bridge_psvcfg[ i ].m.m.temp_data.pcfg_changed = 1;
	//						break;
	//					}
	//				}
	//				// from new perspective
	//				for ( int j = 0 ; j < _g->appcfg._protocol_bridge_psvcfg_count ; j++ )
	//				{
	//					int new_exist = 0;
	//					for ( int i = 0 ; i < _g->appcfg._prev_protocol_bridge_psvcfg_count ; i++ )
	//					{
	//						if ( memcmp( &_g->appcfg._pprev_protocol_bridge_psvcfg[ i ].m.m.id , &_g->appcfg._pprotocol_bridge_psvcfg[ j ].m.m.id , sizeof( struct protocol_bridge_cfg_id ) ) == 0 )
	//						{
	//							new_exist = 1;
	//							if ( memcmp( &_g->appcfg._pprev_protocol_bridge_psvcfg[ i ].m.m.maintained , &_g->appcfg._pprotocol_bridge_psvcfg[ j ].m.m.maintained , sizeof( struct protocol_bridge_maintained_parameter ) ) )
	//							{
	//								_g->appcfg._psvcfg_changed = 1;
	//								_g->appcfg._pprotocol_bridge_psvcfg[ j ].m.m.temp_data.pcfg_changed = 1;
	//							}
	//							break;
	//						}
	//					}
	//					if ( !new_exist )
	//					{
	//						_g->appcfg._psvcfg_changed = 1;
	//						_g->appcfg._pprotocol_bridge_psvcfg[ j ].m.m.temp_data.pcfg_changed = 1;
	//						break;
	//					}
	//				}
	//			}
	//
	//			_g->appcfg._version_changed = 0;
	//
	//			if ( _g->appcfg._psvcfg_changed && IF_VERBOSE_MODE_CONDITION() )
	//			{
	//				_ECHO( "protocol_bridge config changed" );
	//			}
	//		}
	//		sleep( 2 );
	//	}
	//
	//	BEGIN_RET
	//		case 1: { break; }
	//	M_V_END_RET

	return NULL;
}

// TODO . aware of concurrency in config read and act on it
_THREAD_FXN void * config_executer( void * app_data )
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
		return ( void * )&twd;
	}
	G * _g = ( G * )app_data;

	while ( !_g->appcfg._protocol_bridge_psvcfg_count ) // load after any config loaded
	{
		if ( CLOSE_APP_VAR() ) break;
		sleep( 1 );
	}

	//// TODO . change behavior by fresh config
	////if ( _g->appcfg._general_config->c.c.atht == buttleneck )
	////{
	////	_g->bridges.bottleneck_thread = NEW( struct bridges_bottleneck_thread );
	////	MEMSET_ZERO( _g->bridges.bottleneck_thread , struct bridges_bottleneck_thread , 1 );
	////	pthread_mutex_init( &_g->bridges.thread_base.creation_thread_race_cond , NULL );
	////	pthread_mutex_init( &_g->bridges.thread_base.start_working_race_cond , NULL );
	////}
	////else if ( _g->appcfg._general_config->c.c.atht == bidirection )
	////{
	////	_g->bridges.bidirection_thread = NEW( struct bridges_bidirection_thread );
	////	memset( &_g->bridges.bidirection_thread->mem , 0 , sizeof( struct bridges_bidirection_thread_zero_init_memory ) );
	////	queue_init( &_g->bridges.bidirection_thread->queue );
	////	pthread_mutex_init( &_g->bridges.thread_base.start_working_race_cond , NULL );
	////}
	////else if ( _g->appcfg._general_config->c.c.atht == justIncoming )
	////{
	////	_g->bridges.justIncoming_thread = NEW( struct bridges_justIncoming_thread );
	////	MEMSET_ZERO( _g->bridges.justIncoming_thread , struct bridges_justIncoming_thread , 1 );
	////	pthread_mutex_init( &_g->bridges.thread_base.creation_thread_race_cond , NULL );
	////	pthread_mutex_init( &_g->bridges.thread_base.start_working_race_cond , NULL );
	////}

	while ( 1 )
	{
		if ( CLOSE_APP_VAR() )
		{
			for ( int j = 0 ; j < _g->appcfg._protocol_bridge_psvcfg_count ; j++ )
			{
				_g->appcfg._pprotocol_bridge_psvcfg[ j ].m.m.maintained.enable = 0;
				apply_protocol_bridge_new_cfg_changes( _g , &_g->appcfg._pprotocol_bridge_psvcfg[ j ] , &_g->appcfg._pprotocol_bridge_psvcfg[ j ] );
			}
			break;
		}
		if ( _g->appcfg._general_config_changed )
		{
			_g->appcfg._general_config_changed = 0; // for now . TODO later
		}

		if ( _g->appcfg._psvcfg_changed )
		{
			for ( int i = 0 ; i < _g->appcfg._prev_protocol_bridge_psvcfg_count ; i++ )
			{
				int exist = 0;
				for ( int j = 0 ; j < _g->appcfg._protocol_bridge_psvcfg_count ; j++ )
				{
					if ( exist ) break;
					if ( memcmp( &_g->appcfg._pprev_protocol_bridge_psvcfg[ i ].m.m.id , &_g->appcfg._pprotocol_bridge_psvcfg[ j ].m.m.id , sizeof( _g->appcfg._pprev_protocol_bridge_psvcfg[ i ].m.m.id ) ) == 0 && _g->appcfg._pprotocol_bridge_psvcfg[ j ].m.m.temp_data.pcfg_changed )
					{
						// existed cfg changed
						apply_protocol_bridge_new_cfg_changes( _g , &_g->appcfg._pprev_protocol_bridge_psvcfg[ i ] , &_g->appcfg._pprotocol_bridge_psvcfg[ j ] );
						exist = 1;
					}
				}
				if ( !exist )
				{
					// remove removed one
					remove_protocol_bridge( _g , &_g->appcfg._pprev_protocol_bridge_psvcfg[ i ] );
				}
			}
			for ( int i = 0 ; i < _g->appcfg._protocol_bridge_psvcfg_count ; i++ )
			{
				int exist = 0;
				for ( int j = 0 ; j < _g->appcfg._prev_protocol_bridge_psvcfg_count ; j++ )
				{
					if ( memcmp( &_g->appcfg._pprotocol_bridge_psvcfg[ i ].m.m.id , &_g->appcfg._pprev_protocol_bridge_psvcfg[ j ].m.m.id , sizeof( _g->appcfg._pprotocol_bridge_psvcfg[ i ].m.m.id ) ) == 0 )
					{
						exist = 1;
						break;
					}
				}
				if ( !exist )
				{
					// start new cfg
					add_new_protocol_bridge( _g , &_g->appcfg._pprotocol_bridge_psvcfg[ i ] );
				}
			}
			_g->appcfg._psvcfg_changed = 0; // changes applied
		}
		sleep( 5 );
	}
	return NULL;
}


void apply_new_protocol_bridge_config( G * _g , AB * pb , Bcfg * new_ccfg )
{
	//INIT_BREAKABLE_FXN();

	//if ( !new_ccfg->m.m.maintained.enable )
	//{
	//	for ( int i = 0 ; i < pb->pb_trds_masks_count ; i++ )
	//	{
	//		if ( pb->pb_trds_masks[ i ] )
	//		{
	//			pb->pb_trds->alc_thread->do_close_thread = 1;
	//		}
	//	}
	//	return;
	//}

	// make extra space for new one
	//if ( pb->pb_trds_masks_count < PARALLELISM_COUNT )
	//{
	//	int old_thread_count = pb->pb_trds_masks_count;
	//	int diff_count = ( PARALLELISM_COUNT - pb->pb_trds_masks_count );
	//	int new_thread_count = old_thread_count + diff_count;

	//	M_BREAK_IF( ( pb->pb_trds_masks = REALLOC( pb->pb_trds_masks , new_thread_count * sizeof( int ) ) ) == REALLOC_ERR , errMemoryLow , 0 );
	//	MEMSET_ZERO( pb->pb_trds_masks + old_thread_count , int , diff_count );

	//	M_BREAK_IF( ( pb->pb_trds = REALLOC( pb->pb_trds , new_thread_count * sizeof( struct protocol_bridge_thread_holder ) ) ) == REALLOC_ERR , errMemoryLow , 0 );
	//	MEMSET_ZERO( pb->pb_trds + old_thread_count , struct protocol_bridge_thread_holder , diff_count );

	//	pb->pb_trds_masks_count = new_thread_count;
	//}

	// create extra needed thread and socket for each of them
	//int valid_thread_count = 0;
	//for ( int i = 0 ; i < pb->pb_trds_masks_count ; i++ )
	//{
	//	if ( pb->pb_trds_masks[ i ] ) valid_thread_count++;
	//}
	//// must add new thread to active wave
	//if ( valid_thread_count < PARALLELISM_COUNT )
	//{
	//	// run new one
	//	int diff_new_thread_count = PARALLELISM_COUNT - valid_thread_count;

	//	while ( diff_new_thread_count > 0 )
	//	{
	//		for ( int i = 0 ; i < pb->pb_trds_masks_count ; i++ )
	//		{
	//			if ( !pb->pb_trds_masks[ i ] )
	//			{
	//				// 2. set mask
	//				pb->pb_trds_masks[ i ] = 1; // order is matter

	//				DAC( pb->pb_trds[ i ].alc_thread );
	//				M_BREAK_IF( ( pb->pb_trds[ i ].alc_thread = NEW( struct protocol_bridge_thread ) ) == NEW_ERR , errMemoryLow , 0 );
	//				MEMSET_ZERO( pb->pb_trds[ i ].alc_thread , struct protocol_bridge_thread , 1 );
	//				pb->pb_trds[ i ].alc_thread->pb = pb;

	//				// 3. create thread also
	//				MM_BREAK_IF( pthread_create( &pb->pb_trds[ i ].alc_thread->trd_id , NULL , protocol_bridge_runner , ( void * )pb ) != PTHREAD_CREATE_OK , errGeneral , 0 , "thread creation failed" );
	//				
	//				// 4. etc
	//				diff_new_thread_count--;
	//				break;
	//			}
	//		}
	//	}
	//}

	// retire extra thread
	//if ( valid_thread_count > PARALLELISM_COUNT )
	//{
	//	// stop extra
	//	int extra_count = valid_thread_count - PARALLELISM_COUNT;
	//	while ( extra_count > 0 )
	//	{
	//		for ( int i = pb->pb_trds_masks_count - 1 ; i >= 0 ; i-- ) // az yah hazf kon
	//		{
	//			if ( pb->pb_trds_masks[ i ] && pb->pb_trds[ i ].alc_thread->do_close_thread == 0 )
	//			{
	//				pb->pb_trds[ i ].alc_thread->do_close_thread = 1;

	//				//if ( pthread_cancel( pwave->pb_trds[ i ].trd_id ) == 0 )
	//				//{
	//				//	pwave->pb_trds[ i ].alc_thread->trd_id = 0;
	//				//	_close_socket( &pwave->pb_trds[ i ].socket_id );
	//					
	//					extra_count--;
	//				//}
	//				break;
	//			}
	//		}
	//	}
	//}


	//// when we arrive at this point we sure that somethings is changed
	//memcpy( &pb->ccfg , new_ccfg , sizeof( Bcfg ) );
	//new_ccfg->m.m.temp_data.pcfg_changed = 0;
	//// Set every threads that their state must change
	//for ( int i = 0 ; i < pb->pb_trds_masks_count ; i++ )
	//{
	//	if ( pb->pb_trds_masks[ i ] )
	//	{
	//		pb->pb_trds[ i ].alc_thread->base_config_change_applied = 1;
	//	}
	//}

	// TODO . complete reverse on error

	//BEGIN_RET
	//	case 3:
	//{
	//}
	//	case 2:
	//	{
	//	}
	//	case 1:
	//	{
	//		_g->stat.round_zero_set.syscal_err_count++;
	//	}
	//	M_V_END_RET
}

void stop_protocol_bridge( G * _g , AB * pb )
{
	////INIT_BREAKABLE_FXN();

	pb->ccfg.m.m.maintained.enable = 0;
	pb->ccfg.m.m.temp_data.pcfg_changed = 1;
	apply_new_protocol_bridge_config( _g , pb , &pb->ccfg );
	//// TODO
}

void apply_protocol_bridge_new_cfg_changes( G * _g , Bcfg * prev_pcfg , Bcfg * new_ccfg )
{
	//INIT_BREAKABLE_FXN();

	for ( int i = 0 ; i < _g->bridges.ABhs_masks_count ; i++ )
	{
		if ( _g->bridges.ABhs_masks[ i ] )
		{
			if ( memcmp( &_g->bridges.ABhs[ i ].single_AB->ccfg.m.m.id , &prev_pcfg->m.m.id , sizeof( _g->bridges.ABhs[ i ].single_AB->ccfg.m.m.id ) ) == 0 )
			{
				apply_new_protocol_bridge_config( _g , _g->bridges.ABhs[ i ].single_AB , new_ccfg );
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
			if ( memcmp( &_g->bridges.ABhs[ i ].single_AB->ccfg.m.m.id , &pcfg->m.m.id , sizeof( _g->bridges.ABhs[ i ].single_AB->ccfg.m.m.id ) ) == 0 )
			{
				stop_protocol_bridge( _g , _g->bridges.ABhs[ i ].single_AB );
				_g->bridges.ABhs_masks[ i ] = 0;
				DAC( _g->bridges.ABhs[ i ].single_AB );
			}
		}
	}
}

#define PREALLOCAION_SIZE 10

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
			MEMSET_ZERO( _g->bridges.ABhs_masks + old_ABhs_masks_count , int , PREALLOCAION_SIZE );

			M_BREAK_IF( ( _g->bridges.ABhs = REALLOC( _g->bridges.ABhs , new_ABhs_masks_count * sizeof( ABh ) ) ) == REALLOC_ERR , errMemoryLow , 1 );
			MEMSET_ZERO( _g->bridges.ABhs + old_ABhs_masks_count , ABh , PREALLOCAION_SIZE );

			_g->bridges.ABhs_masks_count = new_ABhs_masks_count;
		}
	}

	ASSERT( _g->bridges.ABhs[ new_ccfg_placement_index ].single_AB == NULL );
	M_BREAK_IF( ( _g->bridges.ABhs[ new_ccfg_placement_index ].single_AB = NEW( AB ) ) == NEW_ERR , errMemoryLow , 0 );
	MEMSET_ZERO( _g->bridges.ABhs[ new_ccfg_placement_index ].single_AB , AB , 1 );
	_g->bridges.ABhs_masks[ new_ccfg_placement_index ] = 1;
	copy_bridge_cfg( &_g->bridges.ABhs[ new_ccfg_placement_index ].single_AB->ccfg , new_ccfg );

	apply_protocol_bridge_new_cfg_changes( _g , new_ccfg , new_ccfg );

	BEGIN_RET
		case 3: DAC( _g->bridges.ABhs );
		case 2: DAC( _g->bridges.ABhs_masks );
		case 1: _g->stat.round_zero_set.syscal_err_count++;
			M_V_END_RET
} // TODO . return value

