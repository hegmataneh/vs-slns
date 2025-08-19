#pragma once

_THREAD_FXN void_p pcap_udp_income_thread_proc( void_p src_pb );
_THREAD_FXN void_p one_tcp_out_thread_proc( void_p src_pb );

void clb_init( void_p src_ABtrd );
void clb_destroy( void_p src_ABtrd );
void clb_lock( void_p src_ABtrd , int callerid );
void clb_unlock( void_p src_ABtrd , int callerid );

