// xdp_udp_counter.c
#include <linux/bpf.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <bpf/bpf_helpers.h>
#include <netinet/in.h>

#define TARGET_PORT 1234

struct
{
	__uint( type , BPF_MAP_TYPE_ARRAY );
	__uint( max_entries , 1 );
	__type( key , __u32 );
	__type( value , __u64 );
} udp_counter SEC( ".maps" );

SEC( "xdp" )
int count_udp_packets( struct xdp_md * ctx )
{
	void * data_end = ( void * )( long )ctx->data_end;
	void * data = ( void * )( long )ctx->data;

	struct ethhdr * eth = data;
	if ( ( void * )( eth + 1 ) > data_end )
		return XDP_PASS;

	if ( eth->h_proto != __constant_htons( ETH_P_IP ) )
		return XDP_PASS;

	struct iphdr * ip = ( void * )( eth + 1 );
	if ( ( void * )( ip + 1 ) > data_end )
		return XDP_PASS;

	if ( ip->protocol != IPPROTO_UDP )
		return XDP_PASS;

	int ip_header_len = ip->ihl * 4;
	struct udphdr * udp = ( void * )ip + ip_header_len;
	if ( ( void * )( udp + 1 ) > data_end )
		return XDP_PASS;

	if ( __constant_ntohs( udp->dest ) == TARGET_PORT )
	{
		__u32 key = 0;
		__u64 * counter = bpf_map_lookup_elem( &udp_counter , &key );
		if ( counter )
			__sync_fetch_and_add( counter , 1 );
	}

	return XDP_PASS;
}

char _license[] SEC( "license" ) = "GPL";
