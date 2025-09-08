#include <cstdio>
#include <pthread.h>
//#include <threads.h>

int thread_func( void * arg )
{
	printf( "Hello from thread!\n" );
	return 0;
}

void * proc_pcap_udp_counter( void * src_pb )
{
	return NULL;
}

int main()
{
	pthread_t trd_id;
	

	pthread_create( &trd_id , NULL , proc_pcap_udp_counter , NULL );

	return 0;
}