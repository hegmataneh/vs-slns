#define Uses_STR_SAME
#define Uses_definition
#define Uses_MEMCPY
#define Uses_malloc
#define Uses_BridgeConfig
#define Uses_INIT_BREAKABLE_FXN
#include <Protocol_Bridge.dep>

void copy_bridge_cfg( brg_cfg_t * dst , brg_cfg_t * src )
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

	dst->m.m.temp_data.delayed_validation = True;

	BEGIN_SMPL
	M_V_END_RET
}

int Bcfg0_id_equlity( Bcfg0 * left , Bcfg0 * right )
{
	return MEMCMP( &left->id , &right->id ) == 0;
}

int Bcfg_id_equlity( brg_cfg_t * left , brg_cfg_t * right )
{
	return Bcfg0_id_equlity( ( Bcfg0 * )left , ( Bcfg0 * )right );
}

int bridge_cfg0_data_equlity( Bcfg0 * left , Bcfg0 * right )
{
	if ( left->maintained.enable != right->maintained.enable ) return 0;
	if ( left->maintained.hide != right->maintained.hide ) return 0;
	if ( left->maintained.in_count != right->maintained.in_count ) return 0;
	if ( left->maintained.out_count != right->maintained.out_count ) return 0;

	for ( int ilin = 0 ; ilin < left->maintained.in_count ; ilin++ )
	{
		int exist = 0;
		for ( int irin = 0 ; irin < right->maintained.in_count ; irin++ )
		{
			if ( STR_SAME( ( left->maintained.in + ilin )->name , ( right->maintained.in + irin )->name ) )
			{
				exist = 1;
				if ( MEMCMP( &( left->maintained.in + ilin )->data , &( right->maintained.in + irin )->data ) != 0 ) return 0;
				break;
			}
		}
		if ( !exist )
		{
			return 0;
		}
	}

	for ( int irin = 0 ; irin < right->maintained.in_count ; irin++ )
	{
		int exist = 0;
		for ( int ilin = 0 ; ilin < left->maintained.in_count ; ilin++ )
		{
			if ( STR_SAME( ( right->maintained.in + ilin )->name , ( left->maintained.in + irin )->name ) )
			{
				exist = 1;
				if ( MEMCMP( &( right->maintained.in + irin )->data , &( left->maintained.in + ilin )->data ) != 0 ) return 0;
				break;
			}
		}
		if ( !exist )
		{
			return 0;
		}
	}

	// TOCHECK later 14040528
	for ( int ilin = 0 ; ilin < left->maintained.out_count ; ilin++ )
	{
		int exist = 0;
		for ( int irin = 0 ; irin < right->maintained.out_count ; irin++ )
		{
			if ( STR_SAME( ( left->maintained.out + ilin )->name , ( right->maintained.out + irin )->name ) )
			{
				exist = 1;
				if ( MEMCMP( &( left->maintained.out + ilin )->data , &( right->maintained.out + irin )->data ) != 0 ) return 0;
				break;
			}
		}
		if ( !exist )
		{
			return 0;
		}
	}

	for ( int irin = 0 ; irin < right->maintained.out_count ; irin++ )
	{
		int exist = 0;
		for ( int ilin = 0 ; ilin < left->maintained.out_count ; ilin++ )
		{
			if ( STR_SAME( ( right->maintained.out + ilin )->name , ( left->maintained.out + irin )->name ) )
			{
				exist = 1;
				if ( MEMCMP( &( right->maintained.out + irin )->data , &( left->maintained.out + ilin )->data ) != 0 ) return 0;
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

int bridge_cfg_data_equlity( brg_cfg_t * left , brg_cfg_t * right )
{
	return bridge_cfg0_data_equlity( ( Bcfg0 * )left , ( Bcfg0 * )right );
}

void cleaup_brg_cfg( brg_cfg_t * brg )
{
	DAC( brg->m.m.maintained.in );
	DAC( brg->m.m.maintained.out );
	MEMSET_ZERO_O( brg );
}
