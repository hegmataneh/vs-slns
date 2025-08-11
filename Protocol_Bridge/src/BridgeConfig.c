#define Uses_memcpy
#define Uses_BridgeConfig

#define Uses_INIT_BREAKABLE_FXN
//#define DIRECT_ECHO_BUF _g->stat.last_command // just before include dep
#include <Protocol_Bridge.dep>

void copy_bridge_cfg( Bcfg * dst , Bcfg * src )
{
	INIT_BREAKABLE_FXN();

	memcpy( &dst->m.m.id , &src->m.m.id , sizeof( dst->m.m.id ) );
	memcpy( &dst->m.m.momentary , &src->m.m.momentary , sizeof( dst->m.m.momentary ) );
	memcpy( &dst->m.m.temp_data , &src->m.m.temp_data , sizeof( dst->m.m.temp_data ) );

	DAC( dst->m.m.maintained.in );
	M_BREAK_IF( !( dst->m.m.maintained.in = malloc( ( size_t )sizeof( struct bridge_cfg_input_part ) * ( size_t )src->m.m.maintained.in_count ) ) , errMemoryLow , 0 );
	memset( dst->m.m.maintained.in , 0 , ( size_t )sizeof( struct bridge_cfg_input_part ) * ( size_t )src->m.m.maintained.in_count );
	memcpy( dst->m.m.maintained.in , &src->m.m.maintained.in , ( size_t )sizeof( struct bridge_cfg_input_part ) * ( size_t )src->m.m.maintained.in_count );
	dst->m.m.maintained.in_count = src->m.m.maintained.in_count;

	DAC( dst->m.m.maintained.out );
	M_BREAK_IF( !( dst->m.m.maintained.out = malloc( ( size_t )sizeof( struct bridge_cfg_output_part ) * ( size_t )src->m.m.maintained.out_count ) ) , errMemoryLow , 0 );
	memset( dst->m.m.maintained.out , 0 , ( size_t )sizeof( struct bridge_cfg_output_part ) * ( size_t )src->m.m.maintained.out_count );
	memcpy( dst->m.m.maintained.out , &src->m.m.maintained.in , ( size_t )sizeof( struct bridge_cfg_output_part ) * ( size_t )src->m.m.maintained.out_count );
	dst->m.m.maintained.out_count = src->m.m.maintained.out_count;

	BEGIN_RET
	case 1:
	{
	}
	M_V_END_RET
}
