#define Uses_iSTR_DIFF
#define Uses_STR_DIFF
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
	MEMCPY( &dst->m.m.sol_maintained , &src->m.m.sol_maintained );

	DAC( dst->m.m.cas_maintained.in );
	M_BREAK_IF( !( dst->m.m.cas_maintained.in = MALLOC_AR( dst->m.m.cas_maintained.in , src->m.m.cas_maintained.in_count ) ) , errMemoryLow , 0 );
	MEMSET_ZERO( dst->m.m.cas_maintained.in , src->m.m.cas_maintained.in_count );
	MEMCPY_AR( dst->m.m.cas_maintained.in , src->m.m.cas_maintained.in , src->m.m.cas_maintained.in_count );
	dst->m.m.cas_maintained.in_count = src->m.m.cas_maintained.in_count;

	DAC( dst->m.m.cas_maintained.out );
	M_BREAK_IF( !( dst->m.m.cas_maintained.out = MALLOC_AR( dst->m.m.cas_maintained.out , src->m.m.cas_maintained.out_count ) ) , errMemoryLow , 0 );
	MEMSET_ZERO( dst->m.m.cas_maintained.out , src->m.m.cas_maintained.out_count );
	MEMCPY_AR( dst->m.m.cas_maintained.out , src->m.m.cas_maintained.out , src->m.m.cas_maintained.out_count );
	dst->m.m.cas_maintained.out_count = src->m.m.cas_maintained.out_count;

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
	if ( MEMCMP( &left->sol_maintained , &right->sol_maintained ) ) return 0;
	if ( left->cas_maintained.in_count != right->cas_maintained.in_count ) return 0;
	if ( left->cas_maintained.out_count != right->cas_maintained.out_count ) return 0;

	for ( size_t ilin = 0 ; ilin < left->cas_maintained.in_count ; ilin++ )
	{
		int exist = 0;
		for ( size_t irin = 0 ; irin < right->cas_maintained.in_count ; irin++ )
		{
			if ( STR_SAME( ( left->cas_maintained.in + ilin )->name , ( right->cas_maintained.in + irin )->name ) )
			{
				exist = 1;
				if ( MEMCMP( &( left->cas_maintained.in + ilin )->data , &( right->cas_maintained.in + irin )->data ) != 0 ) return 0;
				break;
			}
		}
		if ( !exist )
		{
			return 0;
		}
	}

	for ( size_t irin = 0 ; irin < right->cas_maintained.in_count ; irin++ )
	{
		int exist = 0;
		for ( size_t ilin = 0 ; ilin < left->cas_maintained.in_count ; ilin++ )
		{
			if ( STR_SAME( ( right->cas_maintained.in + ilin )->name , ( left->cas_maintained.in + irin )->name ) )
			{
				exist = 1;
				if ( MEMCMP( &( right->cas_maintained.in + irin )->data , &( left->cas_maintained.in + ilin )->data ) != 0 ) return 0;
				break;
			}
		}
		if ( !exist )
		{
			return 0;
		}
	}

	// TOCHECK later 14040528
	for ( size_t ilin = 0 ; ilin < left->cas_maintained.out_count ; ilin++ )
	{
		int exist = 0;
		for ( size_t irin = 0 ; irin < right->cas_maintained.out_count ; irin++ )
		{
			if ( STR_SAME( ( left->cas_maintained.out + ilin )->name , ( right->cas_maintained.out + irin )->name ) )
			{
				exist = 1;
				if ( MEMCMP( &( left->cas_maintained.out + ilin )->data , &( right->cas_maintained.out + irin )->data ) != 0 ) return 0;
				break;
			}
		}
		if ( !exist )
		{
			return 0;
		}
	}

	for ( size_t irin = 0 ; irin < right->cas_maintained.out_count ; irin++ )
	{
		int exist = 0;
		for ( size_t ilin = 0 ; ilin < left->cas_maintained.out_count ; ilin++ )
		{
			if ( STR_SAME( ( right->cas_maintained.out + ilin )->name , ( left->cas_maintained.out + irin )->name ) )
			{
				exist = 1;
				if ( MEMCMP( &( right->cas_maintained.out + irin )->data , &( left->cas_maintained.out + ilin )->data ) != 0 ) return 0;
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
	DAC( brg->m.m.cas_maintained.in );
	DAC( brg->m.m.cas_maintained.out );
	MEMSET_ZERO_O( brg );
}

bool tcp_core_config_equlity( tcp_cfg_pak_t * cfg1 , tcp_cfg_pak_t * cfg2 )
{
	if ( !cfg1 || !cfg2 ) return false;
	if ( STR_DIFF( cfg1->data.core.TCP_destination_ip , cfg2->data.core.TCP_destination_ip ) ) return false;
	if ( STR_DIFF( cfg1->data.core.TCP_destination_ports , cfg2->data.core.TCP_destination_ports ) ) return false;
	if ( iSTR_DIFF( cfg1->data.core.TCP_destination_interface , cfg2->data.core.TCP_destination_interface ) ) return false;
	return true;
}
