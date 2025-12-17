#pragma once

typedef struct persistant_cache_prerequisite
{
	distributor_t bcast_store_data; //store_data; // used as face for time when we need send one buffer to persist storage
	distributor_t bcast_reroute_nopeer_pkt;
	page_stack_t page_stack; // manager for persistent storage
	pthread_t trd_pagestack_discharger; // thread who responsible for discharge persist storage data and deliver to packer manager

	//sem_t pagestack_gateway_open_sem; // prevent cpu burne
	//int pagestack_gateway_open_val; // assist gateway status . also -1 means close persistent mngr
	int cool_down_attempt_onEmpty;
	distributor_t bcast_pagestacked_pkts; //pagestack_pakcets; // every one who need data sub here
	
} prst_csh_t; // place who responsible for manage persistent storage for packet manager

/*callback*/
_CALLBACK_FXN status persistant_cache_mngr_store_data( pass_p data , buffer buf , size_t sz );
_CALLBACK_FXN status persistant_cache_mngr_store_data_into_queue( pass_p src_g , buffer src_xudp_hdr , size_t sz , long ex_data );
_THREAD_FXN void_p discharge_persistant_cache_proc( pass_p src_g );
