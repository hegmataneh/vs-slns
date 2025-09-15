#include <cstdio>
#include <pthread.h>
//#include <threads.h>
#include <unistd.h>

int thread_func( void * arg )
{
	printf( "Hello from thread!\n" );
	return 0;
}

void * proc_pcap_udp_counter( void * src_pb )
{

	int i = 1;
	int j = 0;

	int k = i / j;
	(void)k;

	return NULL;
}

int main()
{
	pthread_t trd_id;
	

	pthread_create( &trd_id , NULL , proc_pcap_udp_counter , NULL );


	sleep( 1000000 );

	return 0;
}