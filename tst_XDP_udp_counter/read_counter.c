// read_counter.c
#include <stdio.h>
#include <bpf/libbpf.h>
#include <bpf/bpf.h>
#include <unistd.h>
#include <fcntl.h>

#define MAP_PATH "/sys/fs/bpf/tc/globals/udp_counter"

int main()
{
	int map_fd = bpf_obj_get( MAP_PATH );
	if ( map_fd < 0 )
	{
		perror( "bpf_obj_get" );
		return 1;
	}

	__u32 key = 0;
	__u64 value;

	while ( 1 )
	{
		if ( bpf_map_lookup_elem( map_fd , &key , &value ) == 0 )
		{
			printf( "UDP packets to port 1234: %llu\n" , value );
		}
		else
		{
			perror( "bpf_map_lookup_elem" );
		}
		sleep( 1 );
	}

	return 0;
}
