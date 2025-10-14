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

GLOBAL_VAR extern G * _g;

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
		DAC( _g->appcfg.prev_cfg->c.c.NetworkStack_FilterType );
		DAC( _g->appcfg.prev_cfg );
	}
	if ( _g->appcfg.g_cfg )
	{
		DAC( _g->appcfg.g_cfg->c.c.create_date );
		DAC( _g->appcfg.g_cfg->c.c.modify_date );
		DAC( _g->appcfg.g_cfg->c.c.config_name );
		DAC( _g->appcfg.g_cfg->c.c.config_tags );
		DAC( _g->appcfg.g_cfg->c.c.description );
		DAC( _g->appcfg.g_cfg->c.c.log_level );
		DAC( _g->appcfg.g_cfg->c.c.log_file );
		DAC( _g->appcfg.g_cfg->c.c.NetworkStack_FilterType );
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
			DAC( _g->appcfg.bdj_psv_cfg->m.m.maintained.in );
			DAC( _g->appcfg.bdj_psv_cfg->m.m.maintained.out );
		}
		DAC( _g->appcfg.bdj_psv_cfg );
	}
}

_CALLBACK_FXN _PRIVATE_FXN void pre_config_init_config( void_p src_g )
{
	G * _g = ( G * )src_g;
	
	distributor_subscribe_withOrder( &_g->distributors.quit_interrupt_dist , SUB_LONG , SUB_FXN( cleanup_config ) , _g , clean_config );

	distributor_subscribe_withOrder( &_g->distributors.quit_interrupt_dist , SUB_LONG , SUB_FXN( cleanup_bridges ) , _g , clean_bridges );
}

__attribute__( ( constructor( 102 ) ) )
_PRIVATE_FXN void pre_main_init_config_component( void )
{
	distributor_subscribe( &_g->distributors.pre_configuration , SUB_VOID , SUB_FXN( pre_config_init_config ) , _g );
	distributor_init( &_g->distributors.post_config_stablished , 1 );
}

// TODO . think about race condition
_THREAD_FXN void_p version_checker( pass_p src_g )
{
	INIT_BREAKABLE_FXN();
	G * _g = ( G * )src_g;
	
	distributor_publish_long( &_g->distributors.thread_startup , pthread_self() , _g );
	__attribute__( ( cleanup( thread_goes_out_of_scope ) ) ) pthread_t trd_id = pthread_self();
	__arrr_n += sprintf( __arrr + __arrr_n , "\t\t\t\t\t\t\t%s started %lu\n" , __FUNCTION__ , trd_id );

	char buf[ 50 ] = { 0 };
	time_t prev_time = { 0 } , cur_time = { 0 };
	struct Config_ver temp_ver = { 0 };
	while ( 1 )
	{
		cur_time = time( NULL );

		if ( GRACEFULLY_END_THREAD() ) break;

		if ( _g->appcfg.ver == NULL || difftime( cur_time , prev_time ) > 5 )
		{
			prev_time = cur_time;

			MEMSET( buf , 0 , sizeof( buf ) );
			const char * config_ver_file_content = read_file( CONFIG_ROOT_PATH "/config_ver.txt" , ( char * )buf );
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
		case 1: DIST_APP_FAILURE();
	M_V_END_RET
	return VOID_RET;
}

// TODO . echo acceptible config one time to inform user
_THREAD_FXN void_p config_loader( pass_p src_g )
{
	INIT_BREAKABLE_FXN();
	G * _g = ( G * )src_g;
	
	distributor_publish_long( &_g->distributors.thread_startup , pthread_self() , _g );
	__attribute__( ( cleanup( thread_goes_out_of_scope ) ) ) pthread_t trd_id = pthread_self();
	__arrr_n += sprintf( __arrr + __arrr_n , "\t\t\t\t\t\t\t%s started %lu\n" , __FUNCTION__ , trd_id );
	
	while ( !_g->appcfg.version_changed ) // load after version loaded
	{
		if ( GRACEFULLY_END_THREAD() ) break;
		mng_basic_thread_sleep( _g , HI_PRIORITY_THREAD );
	}
	
	typed( json_element ) el_Protocol_Bridge_config;
	
	while ( 1 )
	{
		if ( GRACEFULLY_END_THREAD() ) break;
		if ( _g->appcfg.g_cfg == NULL || _g->appcfg.version_changed )
		{
			Gcfg temp_config = { 0 };
			Gcfg0 * pGeneralConfiguration = ( Gcfg0 * )&temp_config;
			brg_cfg_t * pProtocol_Bridges = NULL;
			size_t Protocol_Bridges_count = 0;
			{
				const char * Protocol_Bridge_config_file_content = read_file( CONFIG_ROOT_PATH "/Protocol_Bridge_config.txt" , NULL );
				MM_BREAK_IF( !Protocol_Bridge_config_file_content , errNotFound , 0 , "cannot open config file" );
					
				result( json_element ) rs_Protocol_Bridge_config = json_parse( Protocol_Bridge_config_file_content );
				FREE( ( void_p )Protocol_Bridge_config_file_content );
				MM_BREAK_IF( catch_error( &rs_Protocol_Bridge_config , "Protocol_Bridge_config" , 1 ) , errGeneral , 0 , "cannot parse config file" );
				el_Protocol_Bridge_config = result_unwrap( json_element )( &rs_Protocol_Bridge_config );
	
				// load general configurations
				if ( _g->appcfg.ver->Major >= 1 ) // first version of config file structure
				{
					result( json_element ) re_configurations = json_object_find( el_Protocol_Bridge_config.value.as_object , "configurations" );
					MM_BREAK_IF( catch_error( &re_configurations , "configurations" , 1 ) , errNotFound , 0 , "configurations" );
					typed( json_element ) el_configurations = result_unwrap( json_element )( &re_configurations );
	
					#define CFG_ELEM_STR( name )																			/**/\
						result( json_element ) re_##name = json_object_find( el_configurations.value.as_object , #name );	/**/\
						M_BREAK_IF( catch_error( &re_##name , #name , 1 ) , errNotFound , 0 );								/**/\
						typed( json_element ) el_##name = result_unwrap( json_element )( &re_##name );						/**/\
						NEWSTR( pGeneralConfiguration->name , el_##name.value.as_string , 0 );								/**/
					#define CFG_ELEM_I( name )																				/**/\
						result( json_element ) re_##name = json_object_find( el_configurations.value.as_object , #name );	/**/\
						M_BREAK_IF( catch_error( &re_##name , #name , 1 ) , errNotFound , 0 );								/**/\
						typed( json_element ) el_##name = result_unwrap( json_element )( &re_##name );						/**/\
						pGeneralConfiguration->name = (int)el_##name.value.as_number.value.as_long;							/**/
					#define CFG_ELEM_I64( name )																			/**/\
						result( json_element ) re_##name = json_object_find( el_configurations.value.as_object , #name );	/**/\
						M_BREAK_IF( catch_error( &re_##name , #name , 1 ) , errNotFound , 0 );								/**/\
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
					
					CFG_ELEM_I64( pkt_mgr_segment_capacity );																/**/
					CFG_ELEM_I64( pkt_mgr_offsets_capacity );																/**/

					CFG_ELEM_I( pkt_mgr_maximum_keep_unfinished_segment_sec );												/**/
					
					#undef CFG_ELEM_I
					#undef CFG_ELEM_STR
				}
	
				// load Protocol_Bridges
				{
					result( json_element ) re_Protocol_Bridges = json_object_find( el_Protocol_Bridge_config.value.as_object , "Protocol_Bridges" );
					MM_BREAK_IF( catch_error( &re_Protocol_Bridges , "Protocol_Bridges" , 1 ) , errNotFound , 0 , "Protocol_Bridges" );
					typed( json_element ) el_Protocol_Bridges = result_unwrap( json_element )( &re_Protocol_Bridges );
	
					//MM_BREAK_IF( ( Protocol_Bridges_count = el_Protocol_Bridges.value.as_object->count ) < 1 , errGeneral , 0 , "Protocol_Bridges must be not zero" );
	
					M_BREAK_IF( !( pProtocol_Bridges = MALLOC_AR( pProtocol_Bridges , el_Protocol_Bridges.value.as_object->count ) ) , errMemoryLow , 0 );
					MEMSET_ZERO( pProtocol_Bridges , el_Protocol_Bridges.value.as_object->count );
					
					int iactual_section = 0;
					for ( int icnf_section = 0 ; icnf_section < el_Protocol_Bridges.value.as_object->count ; icnf_section++ ) // each bridge
					{
						((Bcfg0 *)(pProtocol_Bridges + iactual_section))->temp_data._pseudo_g = ( void_p )_g; // each config must know global settings
						const char * output_Protocol_Bridge_name = ( *( el_Protocol_Bridges.value.as_object->entries + icnf_section ) )->key;
	
						result( json_element ) re_each_bridge = json_object_find( el_Protocol_Bridges.value.as_object , output_Protocol_Bridge_name );
						M_BREAK_IF( catch_error( &re_each_bridge , output_Protocol_Bridge_name , 1 ) , errNotFound , 0 );
						typed( json_element ) el_each_bridge = result_unwrap( json_element )( &re_each_bridge );
	
						#define CFG_ELEM_STR( part , name )																					/**/\
						result( json_element ) re_##name = json_object_find( el_each_bridge.value.as_object , #name );						/**/\
						M_BREAK_IF( catch_error( &re_##name , #name , 1 ) , errNotFound , 0 );												/**/\
						typed( json_element ) el_##name = result_unwrap( json_element )( &re_##name );										/**/\
						strcpy(((Bcfg0 *)(pProtocol_Bridges + iactual_section))->part.name , el_##name.value.as_string );					/**/
	
						#define CFG_ELEM_I( part , name )																					/**/\
						result( json_element ) re_##name = json_object_find( el_each_bridge.value.as_object , #name );						/**/\
						M_BREAK_IF( catch_error( &re_##name , #name , 1 ) , errNotFound , 0 );												/**/\
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
						result( json_element ) re_inputs = json_object_find( el_each_bridge.value.as_object , "inputs" );
						MM_BREAK_IF( catch_error( &re_inputs , "inputs" , 1 ) , errNotFound , 0 , "inputs" );
						typed( json_element ) el_inputs = result_unwrap( json_element )( &re_inputs );
												
						((Bcfg0 *)(pProtocol_Bridges + iactual_section))->maintained.in_count = el_inputs.value.as_object->count;
						
						M_BREAK_IF( !( ((Bcfg0 *)(pProtocol_Bridges + iactual_section))->maintained.in = MALLOC_AR( ((Bcfg0 *)(pProtocol_Bridges + iactual_section))->maintained.in , ((Bcfg0 *)(pProtocol_Bridges + iactual_section))->maintained.in_count ) ) , errMemoryLow , 0 );
						MEMSET_ZERO( ((Bcfg0 *)(pProtocol_Bridges + iactual_section))->maintained.in , ((Bcfg0 *)(pProtocol_Bridges + iactual_section))->maintained.in_count );
						for ( int iin = 0 ; iin < ((Bcfg0 *)(pProtocol_Bridges + iactual_section))->maintained.in_count ; iin++ )
						{
							const char * output_input_name = ( *( el_inputs.value.as_object->entries + iin ) )->key;

							strcpy( ( ((Bcfg0 *)(pProtocol_Bridges + iactual_section))->maintained.in + iin )->name , output_input_name );

							result( json_element ) re_inp = json_object_find( el_inputs.value.as_object , output_input_name );
							M_BREAK_IF( catch_error( &re_inp , output_input_name , 1 ) , errNotFound , 0 );
							typed( json_element ) el_inp = result_unwrap( json_element )( &re_inp );

							#define IN_CFG_ELEM_STR( elem , namee )																		/**/\
							result( json_element ) re_##namee = json_object_find( elem.value.as_object , #namee );						/**/\
							M_BREAK_IF( catch_error( &re_##namee , #namee , 1 ) , errNotFound , 0 );											/**/\
							typed( json_element ) el_##namee = result_unwrap( json_element )( &re_##namee );								/**/\
							strcpy( ( ((Bcfg0 *)(pProtocol_Bridges + iactual_section))->maintained.in + iin )->data.namee , el_##namee.value.as_string );					/**/

							#define IN_CFG_ELEM_I( elem , namee )																				/**/\
							result( json_element ) re_##namee = json_object_find( elem.value.as_object , #namee );								/**/\
							M_BREAK_IF( catch_error( &re_##namee , #namee , 1 ) , errNotFound , 0 );													/**/\
							typed( json_element ) el_##namee = result_unwrap( json_element )( &re_##namee );										/**/\
							( ((Bcfg0 *)(pProtocol_Bridges + iactual_section))->maintained.in + iin )->data.namee = (int)el_##namee.value.as_number.value.as_long;					/**/

							IN_CFG_ELEM_STR( el_inp , group );
							IN_CFG_ELEM_STR( el_inp , group_type );
							IN_CFG_ELEM_STR( el_inp , UDP_origin_ip );
							IN_CFG_ELEM_STR( el_inp , UDP_destination_ip );
							
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

							( ( Bcfg0 * )( pProtocol_Bridges + iactual_section ) )->maintained.out_count = el_outputs.value.as_object->count;

							M_BREAK_IF( !( ( ( Bcfg0 * )( pProtocol_Bridges + iactual_section ) )->maintained.out = MALLOC_AR( ( ( Bcfg0 * )( pProtocol_Bridges + iactual_section ) )->maintained.out , ( ( Bcfg0 * )( pProtocol_Bridges + iactual_section ) )->maintained.out_count ) ) , errMemoryLow , 0 );
							MEMSET_ZERO( ( ( Bcfg0 * )( pProtocol_Bridges + iactual_section ) )->maintained.out , ( ( Bcfg0 * )( pProtocol_Bridges + iactual_section ) )->maintained.out_count );
							for ( int iout = 0 ; iout < ( ( Bcfg0 * )( pProtocol_Bridges + iactual_section ) )->maintained.out_count ; iout++ )
							{
								const char * output_outputs_name = ( *( el_outputs.value.as_object->entries + iout ) )->key;

								strcpy( ( ((Bcfg0 *)(pProtocol_Bridges + iactual_section ))->maintained.out + iout )->name , output_outputs_name );

								result( json_element ) re_out = json_object_find( el_outputs.value.as_object , output_outputs_name );
								M_BREAK_IF( catch_error( &re_out , output_outputs_name , 1 ) , errNotFound , 0 );
								typed( json_element ) el_out = result_unwrap( json_element )( &re_out );

								#define IN_CFG_ELEM_STR( elem , namee )																		/**/\
								result( json_element ) re_##namee = json_object_find( elem.value.as_object , #namee );						/**/\
								M_BREAK_IF( catch_error( &re_##namee , #namee , 1 ) , errNotFound , 0 );											/**/\
								typed( json_element ) el_##namee = result_unwrap( json_element )( &re_##namee );								/**/\
								strcpy( ( ((Bcfg0 *)(pProtocol_Bridges + iactual_section ))->maintained.out + iout )->data.namee , el_##namee.value.as_string );		/**/

								#define IN_CFG_ELEM_I( elem , namee )																				/**/\
								result( json_element ) re_##namee = json_object_find( elem.value.as_object , #namee );								/**/\
								M_BREAK_IF( catch_error( &re_##namee , #namee , 1 ) , errNotFound , 0 );													/**/\
								typed( json_element ) el_##namee = result_unwrap( json_element )( &re_##namee );										/**/\
								( ((Bcfg0 *)(pProtocol_Bridges + iactual_section ))->maintained.out + iout )->data.namee = (int)el_##namee.value.as_number.value.as_long;	/**/

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

						iactual_section++;
					}
					Protocol_Bridges_count = iactual_section;
					WARNING( Protocol_Bridges_count );
	
					json_free( &el_Protocol_Bridges );
				}
			}
	
			int initial_general_config = 0;
			// 
			if ( _g->appcfg.g_cfg == NULL ) // TODO . make assignemnt atomic
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
					
					_g->appcfg.g_cfg_changed |= !( _g->appcfg.g_cfg->c.c.pkt_mgr_segment_capacity == _g->appcfg.prev_cfg->c.c.pkt_mgr_segment_capacity );
					_g->appcfg.g_cfg_changed |= !( _g->appcfg.g_cfg->c.c.pkt_mgr_offsets_capacity == _g->appcfg.prev_cfg->c.c.pkt_mgr_offsets_capacity );

					_g->appcfg.g_cfg_changed |= !( _g->appcfg.g_cfg->c.c.pkt_mgr_maximum_keep_unfinished_segment_sec == _g->appcfg.prev_cfg->c.c.pkt_mgr_maximum_keep_unfinished_segment_sec );

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
_THREAD_FXN void_p config_executer( pass_p src_g )
{
	INIT_BREAKABLE_FXN();
	G * _g = ( G * )src_g;
	
	distributor_publish_long( &_g->distributors.thread_startup , pthread_self() , _g );
	__attribute__( ( cleanup( thread_goes_out_of_scope ) ) ) pthread_t trd_id = pthread_self();
	__arrr_n += sprintf( __arrr + __arrr_n , "\t\t\t\t\t\t\t%s started %lu\n" , __FUNCTION__ , trd_id );

	while ( !_g->appcfg.bdj_psv_cfg_count ) // load after any config loaded
	{
		if ( GRACEFULLY_END_THREAD() ) break;
		mng_basic_thread_sleep( _g , HI_PRIORITY_THREAD );
	}

	distributor_publish_void( &_g->distributors.post_config_stablished , NULL );

	while ( 1 )
	{
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
			_g->appcfg.g_cfg_changed = 0; // for now . TODO later
		}

		if ( _g->appcfg.psv_cfg_changed )
		{
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
	for ( int imsk = 0 ; imsk < _g->bridges.ABhs_masks_count ; imsk++ )
	{
		if ( _g->bridges.ABhs_masks[ imsk ] )
		{
			if ( Bcfg_id_equlity( &_g->bridges.ABs[ imsk ].single_AB->cpy_cfg , prev_pcfg ) )
			{
				apply_new_protocol_bridge_config( _g , _g->bridges.ABs[ imsk ].single_AB , new_ccfg );
			}
		}
	}
}

void remove_protocol_bridge( G * _g , brg_cfg_t * pcfg )
{
	for ( int imsk = 0 ; imsk < _g->bridges.ABhs_masks_count ; imsk++ )
	{
		if ( _g->bridges.ABhs_masks[ imsk ] )
		{
			if ( Bcfg_id_equlity( &_g->bridges.ABs[ imsk ].single_AB->cpy_cfg , pcfg ) )
			{
				stop_protocol_bridge( _g , _g->bridges.ABs[ imsk ].single_AB );
				_g->bridges.ABhs_masks[ imsk ] = 0;
				DAC( _g->bridges.ABs[ imsk ].single_AB );
			}
		}
	}
}

void add_new_protocol_bridge( G * _g , brg_cfg_t * new_ccfg )
{
	INIT_BREAKABLE_FXN();

	int new_ccfg_placement_index = -1;
	while ( new_ccfg_placement_index < 0 ) // try to find one place for new wave
	{
		for ( int imsk = 0 ; imsk < _g->bridges.ABhs_masks_count ; imsk++ )
		{
			if ( !_g->bridges.ABhs_masks[ imsk ] )
			{
				new_ccfg_placement_index = imsk;
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

	WARNING( _g->bridges.ABs[ new_ccfg_placement_index ].single_AB == NULL );
	M_BREAK_IF( ( _g->bridges.ABs[ new_ccfg_placement_index ].single_AB = MALLOC_ONE( _g->bridges.ABs[ new_ccfg_placement_index ].single_AB ) ) == NEW_ERR , errMemoryLow , 0 );
	MEMSET_ZERO_O( _g->bridges.ABs[ new_ccfg_placement_index ].single_AB );
	_g->bridges.ABhs_masks[ new_ccfg_placement_index ] = 1;
	copy_bridge_cfg( &_g->bridges.ABs[ new_ccfg_placement_index ].single_AB->cpy_cfg , new_ccfg );

	apply_protocol_bridge_new_cfg_changes( _g , new_ccfg , new_ccfg );

	BEGIN_RET
		case 3: DAC( _g->bridges.ABs );
		case 2: DAC( _g->bridges.ABhs_masks );
		case 1: DIST_APP_FAILURE();
	M_V_END_RET
}
