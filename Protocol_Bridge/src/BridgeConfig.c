#define Uses_memcpy
#define Uses_malloc
#define Uses_BridgeConfig

#define Uses_INIT_BREAKABLE_FXN
//#define DIRECT_ECHO_BUF _g->stat.last_command // just before include dep
#include <Protocol_Bridge.dep>

void copy_bridge_cfg( Bcfg * dst , Bcfg * src )
{
	INIT_BREAKABLE_FXN();

	MEMCPY( &dst->m.m.id , &src->m.m.id );
	MEMCPY( &dst->m.m.temp_data , &src->m.m.temp_data );

	DAC( dst->m.m.maintained.in );
	M_BREAK_IF( !( dst->m.m.maintained.in = MALLOC_AR( dst->m.m.maintained.in , src->m.m.maintained.in_count ) ) , errMemoryLow , 0 );
	MEMSET_ZERO( dst->m.m.maintained.in , src->m.m.maintained.in_count );
	MEMCPY_AR( dst->m.m.maintained.in , src->m.m.maintained.in , src->m.m.maintained.in_count );
	dst->m.m.maintained.in_count = src->m.m.maintained.in_count;

	DAC( dst->m.m.maintained.out );
	M_BREAK_IF( !( dst->m.m.maintained.out = MALLOC_AR( dst->m.m.maintained.out , src->m.m.maintained.out_count ) ) , errMemoryLow , 0 );
	MEMSET_ZERO( dst->m.m.maintained.out , src->m.m.maintained.out_count );
	MEMCPY_AR( dst->m.m.maintained.out , src->m.m.maintained.out , src->m.m.maintained.out_count );
	dst->m.m.maintained.out_count = src->m.m.maintained.out_count;

	BEGIN_RET
	case 1: ;
	M_V_END_RET
}

int Bcfg0_id_equlity( Bcfg0 * left , Bcfg0 * right )
{
	return MEMCMP( &left->id , &right->id ) == 0;
}

int Bcfg_id_equlity( Bcfg * left , Bcfg * right )
{
	return Bcfg0_id_equlity( ( Bcfg0 * )left , ( Bcfg0 * )right );
}

int bridge_cfg0_data_equlity( Bcfg0 * left , Bcfg0 * right )
{
	if ( left->maintained.enable != right->maintained.enable ) return 0;
	if ( left->maintained.in_count != right->maintained.in_count ) return 0;
	if ( left->maintained.out_count != right->maintained.out_count ) return 0;

	for ( int i = 0 ; i < left->maintained.in_count ; i++ )
	{
		int exist = 0;
		for ( int j = 0 ; j < right->maintained.in_count ; j++ )
		{
			if ( STR_SAME( ( left->maintained.in + i )->name , ( right->maintained.in + j )->name ) )
			{
				exist = 1;
				if ( MEMCMP( &( left->maintained.in + i )->data , &( right->maintained.in + j )->data ) != 0 ) return 0;
				break;
			}
		}
		if ( !exist )
		{
			return 0;
		}
	}

	for ( int j = 0 ; j < right->maintained.in_count ; j++ )
	{
		int exist = 0;
		for ( int i = 0 ; i < left->maintained.in_count ; i++ )
		{
			if ( STR_SAME( ( right->maintained.in + i )->name , ( left->maintained.in + j )->name ) )
			{
				exist = 1;
				if ( MEMCMP( &( right->maintained.in + j )->data , &( left->maintained.in + i )->data ) != 0 ) return 0;
				break;
			}
		}
		if ( !exist )
		{
			return 0;
		}
	}

	// TOCHECK later 14040528
	for ( int i = 0 ; i < left->maintained.out_count ; i++ )
	{
		int exist = 0;
		for ( int j = 0 ; j < right->maintained.out_count ; j++ )
		{
			if ( STR_SAME( ( left->maintained.out + i )->name , ( right->maintained.out + j )->name ) )
			{
				exist = 1;
				if ( MEMCMP( &( left->maintained.out + i )->data , &( right->maintained.out + j )->data ) != 0 ) return 0;
				break;
			}
		}
		if ( !exist )
		{
			return 0;
		}
	}

	for ( int j = 0 ; j < right->maintained.out_count ; j++ )
	{
		int exist = 0;
		for ( int i = 0 ; i < left->maintained.out_count ; i++ )
		{
			if ( STR_SAME( ( right->maintained.out + i )->name , ( left->maintained.out + j )->name ) )
			{
				exist = 1;
				if ( MEMCMP( &( right->maintained.out + j )->data , &( left->maintained.out + i )->data ) != 0 ) return 0;
				break;
			}
		}
		if ( !exist )
		{
			return 0;
		}
	}

	return 1;
}

int bridge_cfg_data_equlity( Bcfg * left , Bcfg * right )
{
	return bridge_cfg0_data_equlity( ( Bcfg0 * )left , ( Bcfg0 * )right );
}
