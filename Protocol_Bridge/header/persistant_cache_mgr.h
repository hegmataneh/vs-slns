#pragma once

typedef struct persistant_cache_prerequisite
{
	distributor_t store_data; // used as face for time when we need send one buffer to persist storage
	page_stack_t page_stack; // manager for persistent storage
	pthread_t trd_pagestack; // thread who responsible for discharge persist storage data and deliver to packer manager

	distributor_t pagestack_gateway_status; // open clost page stack discharge
	sem_t pagestack_gateway_open_sem; // prevent cpu burne
	int pagestack_gateway_open_val; // assist gateway status
	distributor_t pagestack_pakcets; // every one who need data sub here

} prst_csh_t; // place who responsible for manage persistent storage for packet manager

/*callback*/
_CALLBACK_FXN status persistant_cache_mngr_store_data( pass_p data , buffer buf , size_t sz );
_THREAD_FXN void_p discharge_persistant_cache_proc( pass_p src_g );
_CALLBACK_FXN void persistant_cache_mngr_control_pg_gateway( pass_p src_g , int open_close );
